#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include <compiler_mcs51.h>

#include <reg24le1.h>
#include <nrf24l.h>
#include <nrfutils.h>
#include <nrfdbg.h>

#include "i2c.h"
#include "rf_protocol.h"
#include "../dongle/reports.h"
#include "mpu_simple.h"
#include "sleeping.h"
#include "rf_head.h"
#include "tracker_settings.h"
#include "tracker.h"

void hw_init()
{
	uint8_t c;
	
	P0DIR = 0xf0;		// P0.0 P0.1 P0.2 are the LEDs and they are outputs
                        // P0.3 is the UART TX - output
						// P0.5 is the push button - input
						// P0.6 is the MPU interrupt pin - input

	P0CON = 0x55;		// turn on the pullup for the recenter button

	P1DIR = 0x00;
	
	while (1)
	{
		P12 = 0;
		P12 = 1;
		P12 = 0;
		P12 = 1;
		P12 = 0;
		P12 = 1;
		P12 = 0;
		P12 = 1;
		P12 = 0;
		P12 = 1;
		P12 = 0;
		P12 = 1;
		P12 = 0;
		P12 = 1;
		P12 = 0;
		P12 = 1;
		P12 = 0;
		P12 = 1;
		P12 = 0;
		P12 = 1;
		P12 = 0;
		P12 = 1;
		P12 = 0;
		P12 = 1;
		P12 = 0;
		P12 = 1;
		P12 = 0;
		P12 = 1;
		P12 = 0;
		P12 = 1;
		P12 = 0;
		P12 = 1;
		P12 = 0;
		P12 = 1;
		P12 = 0;
		P12 = 1;
		P12 = 0;
		P12 = 1;
		P12 = 0;
		P12 = 1;
		P12 = 0;
		P12 = 1;
		P12 = 0;
		P12 = 1;
		P12 = 0;
		P12 = 1;
		P12 = 0;
		P12 = 1;
		P12 = 0;
		P12 = 1;
		P12 = 0;
		P12 = 1;
		P12 = 0;
		P12 = 1;
		P12 = 0;
		P12 = 1;
		P12 = 0;
		P12 = 1;
	}
	
	// cycle the LEDs
	LED_RED		= 0;
	LED_YELLOW	= 0;
	LED_GREEN	= 0;
	
	for (c = 0; c < 3; ++c)
	{
		LED_RED		= 1;
		delay_ms(40);
		LED_RED		= 0;
		LED_YELLOW	= 1;
		delay_ms(40);
		LED_YELLOW	= 0;
		LED_GREEN	= 1;
		delay_ms(40);
		LED_GREEN	= 0;
		LED_YELLOW	= 1;
		delay_ms(40);
		LED_YELLOW	= 0;
	}
	
	dbgInit();
	i2c_init();

	dputs("init started");
	
	LED_YELLOW = 1;
	
	mpu_init();
	
	dbgFlush();
	
	rf_head_init();		// init the radio
	
	init_sleep();		// we need to wake up from RFIRQ

	LED_YELLOW = 0;

	dputs("init OK");
}

/*
// This is a test for a new kind of calibration function.
void test_bias(void)
{
	int32_t g[3];
	int32_t a[3];
	int16_t cnt = 0;
	uint8_t more;
	mpu_packet_t pckt;
	
	while (1)
	{
		if (cnt == 0)
		{
			g[0] = 0;
			g[1] = 0;
			g[2] = 0;

			a[0] = 0;
			a[1] = 0;
			a[2] = 0;
		}
		
		// wait for the interrupt
		while (MPU_IRQ)
			dbgPoll();
		while (!MPU_IRQ);

		dmp_read_fifo(&pckt, &more);
		
		a[0] += pckt.accel[0];
		a[1] += pckt.accel[1];
		a[2] += pckt.accel[2];

		g[0] += pckt.gyro[0];
		g[1] += pckt.gyro[1];
		g[2] += pckt.gyro[2];
		
		if (cnt == 200)
		{
			dprintf("%li   %li   %li\n", a[0] / cnt, a[1] / cnt, a[2] / cnt - 16384);
			cnt = 0;
		} else {
			++cnt;
		}
	}
}
*/

// returns the battery voltage in 10mV units
// for instance: get_battery_voltage() returning 278 equals a voltage of 2.78V
uint16_t get_battery_voltage(void)
{
	uint16_t res;
	
	// config the ADC

	ADCCON3 = 0b01100000;	// 8 bits, right justify
	ADCCON2 = 0b00000111;	// longest input acquisition window
							// 6us ADC power-down delay

	// start
	ADCCON1 = 0b10111000;	// power up ADC
							// 1/3 VDD
							// internal 1.2V reference
			
	// wait for the conversion to finish
	delay_us(3);	// wait for busy bit to stabilize
	while (ADCCON1 & 0x40)
		;
		
	res = ADCDATL;

	return (res * 72) / 51;
}

#define LED_PCKT_TOTAL		150
#define LED_PCKT_LED_ON		2

#define VOLTAGE_READ_EVERY		50
#define TEMPERATURE_READ_EVERY	20

int main(void)
{
	uint8_t more, ack;
	uint8_t rf_pckt_ok = 0, rf_pckt_lost = 0;
	uint8_t voltage_counter = 0, temperature_counter = 0;
	
	bool read_result;
	mpu_packet_t pckt;

	hw_init();
	
	for (;;)
	{
		pckt.flags = 0;		// reset the flags

		// get the battery voltage every VOLTAGE_READ_EVERY iterations
		if (++voltage_counter == VOLTAGE_READ_EVERY)
		{
			pckt.flags |= FLAG_VOLTAGE_VALID;
			pckt.voltage = get_battery_voltage();
			
			voltage_counter = 0;
		}
		
		// same as with the voltage, send the temperature
		if (++temperature_counter == TEMPERATURE_READ_EVERY)
		{
			pckt.flags |= FLAG_TEMPERATURE_VALID;
			mpu_get_temperature(&pckt.temperature);
			
			temperature_counter = 0;
		}
		
		// Waits for the interrupt from the MPU-6050.
		// Instead of polling, I should put the MCU to sleep and then have it awaken by the MPU-6050.
		// However, I have not succeeded in the making the wakeup work reliably.
		while (MPU_IRQ)
			dbgPoll();
		while (!MPU_IRQ)
			;
			
		do {
			// read all the packets in the MPU fifo
			do {
				read_result = dmp_read_fifo(&pckt, &more);
			} while (more);
			
			if (read_result)
			{
				pckt.flags |= (RECENTER_BTN == 0 ? FLAG_RECENTER : 0);
				
				// send the message
				if (rf_head_send_message(&pckt, sizeof(pckt)))
					++rf_pckt_ok;
				else
					++rf_pckt_lost;

				// update the LEDs
				if (rf_pckt_lost + rf_pckt_ok == LED_PCKT_TOTAL)
				{
					if (rf_pckt_ok > rf_pckt_lost)
						LED_GREEN = 1;
					else
						LED_RED = 1;
						
				} else if (rf_pckt_lost + rf_pckt_ok == LED_PCKT_TOTAL + LED_PCKT_LED_ON) {
					LED_RED = 0;
					LED_GREEN = 0;

					rf_pckt_ok = rf_pckt_lost = 0;
				}

				// check for an ACK payload
				if (rf_head_read_ack_payload(&ack, 1))
				{
					if (ack == CMD_CALIBRATE)
					{
						mpu_calibrate_bias();
						
					} else if (ack == CMD_READ_TRACKER_SETTINGS) {
					
						rf_head_send_message(get_tracker_settings(), sizeof(tracker_settings_t));

					} else if (ack >= CMD_RF_PWR_LOWEST  &&  ack <= CMD_RF_PWR_HIGHEST) {
					
						tracker_settings_t new_settings;
						memcpy(&new_settings, get_tracker_settings(), sizeof(tracker_settings_t));
						new_settings.rf_power = ack;
						
						save_tracker_settings(&new_settings);
					}
				}
			}
			
		} while (more);
	}
}
