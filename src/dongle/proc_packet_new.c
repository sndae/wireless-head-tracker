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
	
	// set the new value
	new_settings.x_drift_comp += get_curr_x_drift_comp();
	
	save_dongle_settings(&new_settings);
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

float constrain_flt(float val)
{
	if (val < -16383.0)
		return -16384.0;
		
	if (val > 16383.0)
		return 16383.0;
		
	return val;
}

int32_t constrain_16bit(int32_t val)
{
	if (val < -32768)
		return -32768;
		
	if (val > 32767)
		return 32767;
		
	return val;
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
		yaw_drift = offset[0] - euler[0];
		
		euler[0] -= offset[0];
		euler[1] -= offset[1];
		euler[2] -= offset[2];
	}
	
	return is_offset_valid;
}

bool process_packet(mpu_packet_t* pckt)
{
	int16_t euler[3];

	const FeatRep_DongleSettings __xdata * pSettings = get_dongle_settings();

	quat2euler(pckt->quat, euler);	// convert quaternions to euler angles

	if (pckt->flags & FLAG_RECENTER)
		recenter();
	
	// do_offet
	if (!do_offset(euler))
		return false;
		
	usb_joystick_report.x = euler[0];
	usb_joystick_report.y = euler[1];
	usb_joystick_report.z = euler[2];

	if (dbgEmpty())
		dprintf("%6d %6d %6d\n", euler[0], euler[1], euler[2]);

	return true;
}
