#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <compiler_mcs51.h>

#include "nrfdbg.h"

#include "rf_protocol.h"
#include "proc_packet.h"
#include "math_cordic.h"
#include "mdu.h"
#include "mymath.h"
#include "reports.h"
#include "dongle_settings.h"

bool should_recenter = true;
int32_t sample_cnt;
int32_t yaw_drift;

void save_x_drift_comp(void)
{
	// get the current settings
	FeatRep_DongleSettings __xdata new_settings;
	memcpy(&new_settings, get_dongle_settings(), sizeof(FeatRep_DongleSettings));

	new_settings.drift_per_1k += (int16_t)((1024.0 * yaw_drift) / sample_cnt);

	save_dongle_settings(&new_settings);

	recenter();
}

void recenter(void)
{
	should_recenter = true;
}

float get_curr_x_drift_comp(void)
{
	if (sample_cnt > 0)
		return yaw_drift / (float)sample_cnt;

	return 0;
}

// convert the raw quaternions from the sensors into Euler angles
void quat2euler(int16_t* quat, int16_t* euler)
{
	int16_t qw, qx, qy, qz;
	int32_t qww, qxx, qyy, qzz;

	// calculate Yaw/Pitch/Roll

	// the CORDIC trig functions return angles in units already adjusted to the 16
	// bit signed integer range, so there's no need to scale the results

	qw = quat[0];
	qx = quat[1];
	qy = quat[2];
	qz = quat[3];

	qww = mul_16x16(qw, qw);	// these are MDU optimized 16 bit integer multiplications
	qxx = mul_16x16(qx, qx);
	qyy = mul_16x16(qy, qy);
	qzz = mul_16x16(qz, qz);

	euler[0] = -iatan2_cord(2 * (mul_16x16(qx, qy) + mul_16x16(qw, qz)), qww + qxx - qyy - qzz);
	euler[1] = -iasin_cord(-2 * (mul_16x16(qx, qz) - mul_16x16(qw, qy)));
	euler[2] =  iatan2_cord(2 * (mul_16x16(qy, qz) + mul_16x16(qw, qx)), qww - qxx - qyy + qzz);
}

#define SAMPLES_FOR_RECENTER	30

// calculates and applies recentering offsets
// returns false if we are in the process of calulating new offsets and the euler are not valid
bool do_offset(int16_t* euler)
{
	static int32_t offset[3];
	static bool is_offset_valid = false;

	// do we need to calculate a new offset
	if (should_recenter)
	{
		sample_cnt = -SAMPLES_FOR_RECENTER;
		yaw_drift = 0;
		should_recenter = false;
		is_offset_valid = false;

		// clear the sums
		memset(offset, 0, sizeof(offset));
	}

	sample_cnt++;

	// accumulate the samples
	if (!is_offset_valid)
	{
		offset[0] += euler[0];
		offset[1] += euler[1];
		offset[2] += euler[2];
	}

	// enough samples?
	if (sample_cnt == 0)
	{
		offset[0] /= SAMPLES_FOR_RECENTER;
		offset[1] /= SAMPLES_FOR_RECENTER;
		offset[2] /= SAMPLES_FOR_RECENTER;

		is_offset_valid = true;
	}

	// apply the offsets
	if (is_offset_valid)
	{
		// correct the euler angles
		euler[0] -= offset[0];
		euler[1] -= offset[1];
		euler[2] -= offset[2];
	}

	return is_offset_valid;
}

// applies the yaw drift
void do_drift(int16_t* euler, const FeatRep_DongleSettings __xdata * pSettings)
{
	int16_t compensate = (sample_cnt * pSettings->drift_per_1k) >> 10;

	euler[0] -= compensate;
}

void constrain_16bit(int32_t* val)
{
	if (*val < -32768)
		*val = -32768;
	else if (*val > 32767)
		*val = 32767;
}

// do the axis response
void do_response(int16_t* euler, const FeatRep_DongleSettings __xdata* pSettings)
{
	uint8_t i;

	if (pSettings->is_linear)
	{
		for (i = 0; i < 3; ++i)
		{
			int32_t new_val = mul_16x16(euler[i], pSettings->fact[i]);
			constrain_16bit(&new_val);
			euler[i] = new_val;
		}

		//dzlimit = 35.0 * pSettings->fact_x * pSettings->autocenter;
	} else {
		for (i = 0; i < 3; ++i)
		{
			// the floating arithemetic is only temporary
			// it will be replaced by custom axis responses

			float new_val = mul_16x16(euler[i], abs(euler[i]));
			new_val = new_val * pSettings->fact[i] / 8192.0;

			if (new_val < -32768)
				new_val = -32768;
			else if (new_val > 32767)
				new_val = 32767;
			
			euler[i] = (int16_t)new_val;
		}

		dprintf("%d %d %d\n", euler[0], euler[1], euler[2]);
		
		//dzlimit = 39.0 * pSettings->fact_x * pSettings->autocenter;
	}
}

bool process_packet(mpu_packet_t* pckt)
{
	const FeatRep_DongleSettings __xdata* pSettings = get_dongle_settings();
	int16_t euler[3];		// the resulting angles

	quat2euler(pckt->quat, euler);	// convert quaternions to euler angles

	if (pckt->flags & FLAG_RECENTER)
		recenter();

	// calc and apply the centering offset
	if (!do_offset(euler))
		return false;

	// apply the drift compensations
	do_drift(euler, pSettings);

	// save the current yaw angle after drift compensation
	yaw_drift = euler[0];

	// do the axis response transformations
	do_response(euler, pSettings);

	// copy the data into the USB report
	usb_joystick_report.x = euler[0];
	usb_joystick_report.y = euler[1];
	usb_joystick_report.z = euler[2];

	return true;
}
