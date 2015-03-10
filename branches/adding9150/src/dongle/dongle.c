#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "reg24lu1.h"

#include "reports.h"

#include "../tracker/rf_protocol.h"
#include "../tracker/tracker_settings.h"
#include "rf_dngl.h"
#include "nrfutils.h"

#include "usb.h"
#include "usb_regs.h"
#include "hw_defs.h"

#include "nrfdbg.h"
#include "proc_packet.h"
#include "dongle_settings.h"

#define NUM_COUNTER_PACKETS	10
uint8_t total_packets[NUM_COUNTER_PACKETS];		// holds the total count of the last ~1 second of packets

uint16_t battery_voltage, temperature;

// the next two event handlers are called by the USB code in usb.c
void on_set_report(void)
{
	if (out0buf[0] == DONGLE_SETTINGS_REPORT_ID)
	{
		const FeatRep_DongleSettings __xdata * pSettings = get_dongle_settings();

		FeatRep_DongleSettings new_settings;
		memcpy(&new_settings, out0buf, sizeof(FeatRep_DongleSettings));
		new_settings.drift_per_1k = pSettings->drift_per_1k;
		
		// save the data structure we've just received
		save_dongle_settings(&new_settings);

	} else if (out0buf[0] == COMMAND_REPORT_ID) {

		uint8_t command = out0buf[1];
	
		if (command == CMD_CALIBRATE  ||  command >= CMD_RF_PWR_LOWEST && command <= CMD_RF_PWR_HIGHEST)
		{
			// tell the head tracker to execute command
			rf_dngl_queue_ack_payload(&command, 1);
			recenter();
		} else if (command == CMD_RECENTER) {
			recenter();
		} else if (command == CMD_SAVE_DRIFT) {
			save_x_drift_comp();
		} else if (command == CMD_INC_DRIFT_COMP  ||  command == CMD_DEC_DRIFT_COMP) {
		
			const FeatRep_DongleSettings __xdata * pSettings = get_dongle_settings();

			FeatRep_DongleSettings new_settings;
			memcpy(&new_settings, pSettings, sizeof(FeatRep_DongleSettings));
			if (command == CMD_INC_DRIFT_COMP)
				new_settings.drift_per_1k += 102;
			else
				new_settings.drift_per_1k -= 102;
			
			// save the data structure we've just received
			save_dongle_settings(&new_settings);
		}
	}
}

void on_get_report(void)
{
	// This requests the HID report we defined with the HID report descriptor.
	// This is usually requested over EP1 IN, but can be requested over EP0 too.
	if (usbReqHidGetSetReport.reportID == JOYSTICK_REPORT_ID)
	{
		memcpy(in0buf, &usb_joystick_report, sizeof(usb_joystick_report));

		// send the data on it's way
		in0bc = sizeof(usb_joystick_report);

	} else if (usbReqHidGetSetReport.reportID == DONGLE_SETTINGS_REPORT_ID) {

		// copy the data into the buffer
		memcpy(in0buf, get_dongle_settings(), sizeof(FeatRep_DongleSettings));
		in0buf[0] = DONGLE_SETTINGS_REPORT_ID;

		// send the data on it's way
		in0bc = sizeof(FeatRep_DongleSettings);

	} else if (usbReqHidGetSetReport.reportID == TRACKER_SETTINGS_REPORT_ID) {

		// tell the head tracker to send us the calibration data
		uint8_t ack_payload = CMD_READ_TRACKER_SETTINGS;
		uint8_t pckt_cnt, bytes_rcvd;
		tracker_settings_t tracker_settings;
		FeatRep_TrackerSettings __xdata * pReport = (FeatRep_TrackerSettings __xdata *) in0buf;
		
		rf_dngl_queue_ack_payload(&ack_payload, 1);

		pReport->report_id = CMD_READ_TRACKER_SETTINGS;
		pReport->has_tracker_responded = 0;
		
		// now wait for that data
		for (pckt_cnt = 0; pckt_cnt < 5; ++pckt_cnt)
		{
			bytes_rcvd = rf_dngl_recv(&tracker_settings, sizeof(tracker_settings));
			if (bytes_rcvd == sizeof(tracker_settings))
			{
				pReport->has_tracker_responded = 1;
				pReport->is_calibrated = tracker_settings.is_calibrated;
				pReport->gyro_bias[0] = tracker_settings.gyro_bias[0];
				pReport->gyro_bias[1] = tracker_settings.gyro_bias[1];
				pReport->gyro_bias[2] = tracker_settings.gyro_bias[2];
				pReport->accel_bias[0] = tracker_settings.accel_bias[0];
				pReport->accel_bias[1] = tracker_settings.accel_bias[1];
				pReport->accel_bias[2] = tracker_settings.accel_bias[2];
				pReport->rf_power = tracker_settings.rf_power;

				break;
			}
			
			delay_ms(20);
		}

		// send the data
		in0bc = sizeof(FeatRep_TrackerSettings);
		
	} else if (usbReqHidGetSetReport.reportID == STATUS_REPORT_ID) {
	
		FeatRep_Status __xdata * pResult = (FeatRep_Status __xdata *) in0buf;
		uint8_t c;
		uint16_t total = 0;
		
		pResult->report_id = STATUS_REPORT_ID;
		
		for (c = 0; c < NUM_COUNTER_PACKETS; ++c)
			total += total_packets[c];

		pResult->num_packets = total;
		pResult->sample_cnt = sample_cnt > 0 ? sample_cnt : 0;
		pResult->yaw_value = yaw_value;
		
		pResult->battery_voltage = battery_voltage;
		pResult->temperature = temperature;
		
		// send the data
		in0bc = sizeof(FeatRep_Status);

	} else if (usbReqHidGetSetReport.reportID == MAG_RAW_DATA_REPORT_ID) {
	
		// copy the raw data to the USB buffer
		mag_data_samples.report_id = MAG_RAW_DATA_REPORT_ID;
		memcpy(in0buf, &mag_data_samples, sizeof(mag_data_samples));

		// send the data on it's way
		in0bc = sizeof(mag_data_samples);
		
		mag_data_samples.num_samples = 0;
	}
}

void main(void)
{
	bool joystick_report_ready = false;
	__xdata mpu_packet_t packet;
	uint8_t last_timer_capture;
	uint8_t total_packets_ndx;
	uint8_t curr_packets;
	
	P0DIR = 0x00;	// all outputs
	P0ALT = 0x00;	// all GPIO default behavior
	P0 = 0;			// all low
	
	// timer init
	T2CON =	0b10000001;		// start 1/24 timer
	CCEN =	0b11000000;		// capture on write to CCL3
	last_timer_capture = 0;
	
	memset(total_packets, 0, sizeof(total_packets));
	curr_packets = total_packets_ndx = 0;
	
	mag_data_samples.num_samples = 0;

	LED_off();
	
	dbgInit();
	dputs("\nI live...");

	usbInit();

	rf_dngl_init();

	reset_joystick_report();
	
	for (;;)
	{
		usbPoll();	// handles USB events
		dbgPoll();	// send chars from the UART TX buffer
		
		// check the timer
		CCL3 = 1;	// capture CCH3 and check for overflow
		if (last_timer_capture > CCH3)
		{
			total_packets[total_packets_ndx] = curr_packets;
			
			total_packets_ndx++;
			if (total_packets_ndx == NUM_COUNTER_PACKETS)
				total_packets_ndx = 0;

			curr_packets = 0;
		}
		last_timer_capture = CCH3;

		// try to read the recv buffer, then process the received data
		if (rf_dngl_recv(&packet, sizeof packet) == sizeof packet)
		{
			joystick_report_ready |= process_packet(&packet);
			
			if (packet.flags & FLAG_VOLTAGE_VALID)
				battery_voltage = packet.voltage;

			temperature = packet.temperature;

			curr_packets++;
			
			LED_on();
		} else {
			LED_off();
		}

		// send the report if the endpoint is not busy
		if ((in1cs & 0x02) == 0   &&   (joystick_report_ready  ||  usbHasIdleElapsed()))
		{
			// copy the joystick report into the endpoint buffer
			memcpy(in1buf, &usb_joystick_report, sizeof(usb_joystick_report));

			// send the data on it's way
			in1bc = sizeof(usb_joystick_report);
			
			joystick_report_ready = false;
		}
	}
}
