#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "nrfdbg.h"

#include "rf_protocol.h"
#include "proc_packet.h"
#include "math_cordic.h"
#include "mdu.h"
#include "reports.h"
#include "dongle_settings.h"

bool should_recenter = true;
int32_t sample_cnt;
int32_t yaw_value;

// these are indexes of the euler[] and center[] arrays
#define YAW		0
#define ROLL	1
#define PITCH	2

void save_x_drift_comp(void)
{
	// get the current settings
	FeatRep_DongleSettings __xdata new_settings;
	memcpy(&new_settings, get_dongle_settings(), sizeof(FeatRep_DongleSettings));

	// << 10 is identical to * 1024
	new_settings.drift_per_1k += (int16_t)((yaw_value << 10) / sample_cnt);

	save_dongle_settings(&new_settings);

	recenter();
}

void recenter(void)
{
	should_recenter = true;
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

	// x - yaw
	euler[YAW] = -iatan2_cord(2 * (mul_16x16(qx, qy) + mul_16x16(qw, qz)), qww + qxx - qyy - qzz);
	
	// y - roll
	euler[ROLL] = -iasin_cord(-2 * (mul_16x16(qx, qz) - mul_16x16(qw, qy)));
	
	// z - pitch
	euler[PITCH] =  iatan2_cord(2 * (mul_16x16(qy, qz) + mul_16x16(qw, qx)), qww - qxx - qyy + qzz);
}

#define SAMPLES_FOR_RECENTER	8

// calculates and applies recentering offsets
// returns false if we are in the process of calulating new offsets and the euler are not valid
bool do_center(int16_t* euler)
{
	static int16_t center[3];
	static int16_t wrap_bias[3];
	static bool is_center_valid = false;
	uint8_t i;

	// do we need to calculate a new center
	if (should_recenter)
	{
		sample_cnt = -SAMPLES_FOR_RECENTER;
		yaw_value = 0;
		should_recenter = false;
		is_center_valid = false;

		// remember the current angles to make a wrap bias
		// this fixes the problem with centering close to the -32768/32767 wrapping point
		memcpy(wrap_bias, euler, sizeof(wrap_bias));
		
		// clear the sums
		memset(center, 0, sizeof(center));
	}

	++sample_cnt;

	// accumulate the samples
	if (!is_center_valid)
		for (i = 0; i < 3; ++i)
			center[i] += euler[i] - wrap_bias[i];

	// enough samples?
	if (sample_cnt == 0)
	{
		// >> 3  if functionally identical to / 8, or / SAMPLES_FOR_RECENTER
		// but it's faster :)
		for (i = 0; i < 3; ++i)
			center[i] = (center[i] >> 3) + wrap_bias[i];

		is_center_valid = true;
	}

	// correct the euler angles
	if (is_center_valid)
		for (i = 0; i < 3; ++i)
			euler[i] -= center[i];

	return is_center_valid;
}

// applies the yaw drift
void do_drift(int16_t* euler, const FeatRep_DongleSettings __xdata * pSettings)
{
	int16_t compensate = (sample_cnt * pSettings->drift_per_1k) >> 10;

	euler[YAW] -= compensate;
}

// do the axis response
void do_response(int16_t* euler, const FeatRep_DongleSettings __xdata* pSettings)
{
	uint8_t i;

	if (!pSettings->is_linear)
	{
		for (i = 0; i < 3; ++i)
		{
			// this is only temporary. it will be replaced with
			// multi-point custom axis responses

			int32_t new_val = mul_16x16(euler[i], abs(euler[i]));
			euler[i] = (int16_t) (new_val >>= 13);		// / 8192
		}
	}
	
	// apply the axis factors
	for (i = 0; i < 3; ++i)
	{
		int32_t new_val = mul_16x16(euler[i], pSettings->fact[i]);

		if (new_val < -32768)
			euler[i] = -32768;
		else if (new_val > 32767)
			euler[i] = 32767;
		else
			euler[i] = (int16_t) new_val;
	}
}

int16_t calc_mag_heading(int16_t* mag, int16_t* euler)
{
	// this is a tilt compensated heading calculation. read more here:
	// http://www.st.com/web/en/resource/technical/document/application_note/CD00269797.pdf

	int32_t Xh, Yh;
	int16_t sinroll, cosroll;
	int16_t sinpitch, cospitch;
	int16_t sinroll_pitch;

	isincos_cord(euler[1], &cosroll, &sinroll);
	isincos_cord(euler[2], &cospitch, &sinpitch);
	
	Xh = mul_16x16(mag[0], cospitch) + mul_16x16(mag[2], sinpitch);
	
	Yh = mul_16x16(sinroll, sinpitch);
	sinroll_pitch = (Yh >> 16);
	
	Yh = mul_16x16(mag[0], sinroll_pitch) + mul_16x16(mag[1], cosroll) - mul_16x16(mag[2], sinroll_pitch);
	
	return -iatan2_cord(Yh, Xh);
}

// 10 seconds on 50Hz samples rate
#define CALIB_MAG_HEADING_OFFSET_SAMPLES	200

void do_mag(int16_t* mag, int16_t* euler)
{
	static int16_t consec_count = 0;
	static bool behind = false;
	static int32_t mag_heading_offset = 0;
	static uint16_t mag_head_off_cnt = 0;
	static int16_t mag_correction = 0;
	
	int16_t mag_heading, mag_delta;

	/*
	{
		static int16_t magmin[3] = {32767, 32767, 32767};
		static int16_t magmax[3] = {-32768, -32768, -32768};
		uint8_t i;
		bool changed = false;
		
		for (i = 0; i < 3; ++i)
		{
			if (magmin[i] > mag[i])		magmin[i] = mag[i], changed = true;
			if (magmax[i] < mag[i])		magmax[i] = mag[i], changed = true;
		}

		if (changed)
		{
			dputs("--------------");
			for (i = 0; i < 3; ++i)
				dprintf("%d  %5d  %5d  %5d\n", i, magmin[i], magmax[i], (magmin[i] + magmax[i]) / 2);
		}
	}
	*/
	
	mag[0] -= -20;		// hard-coded for now
	mag[1] -= -14;
	mag[2] -= -105;
	
	// apply the correction
	euler[YAW] -= mag_correction;
	
	mag_heading = calc_mag_heading(mag, euler);
	mag_delta = mag_heading - euler[YAW];

	// have we accumulated enough samples for a magnetic heading offset?
	if (mag_head_off_cnt < CALIB_MAG_HEADING_OFFSET_SAMPLES)
	{
		++mag_head_off_cnt;

		mag_heading_offset += mag_delta;
		
		if (mag_head_off_cnt == CALIB_MAG_HEADING_OFFSET_SAMPLES)
		{
			mag_heading_offset /= CALIB_MAG_HEADING_OFFSET_SAMPLES;
			dprintf("mag_heading_offset=%ld\n", mag_heading_offset);
		} else {
			return;
		}
	}
	
	mag_delta -= mag_heading_offset;	// apply the heading offset
	
	// EDtracker quote:
	//
	// Mag is so noisy on 9150 we just ask 'is Mag ahead or behind DMP'
	// and keep a count of consecutive behinds or aheads and use this 
	// to adjust our DMP heading and also tweak the DMP drift
	if (mag_delta > 0)
	{
		if (behind)
			--consec_count;
		else
			consec_count = 0;
		
		behind = true;
	} else {
		if (!behind)
			++consec_count;
		else
			consec_count = 0;
		
		behind = false;
	}

	mag_correction += (int16_t) (mul_16x16(consec_count, abs(consec_count)) / 80);

	if (dbgEmpty())
		dprintf("delta=%6d  corr=%6d  conscnt=%4d\n", mag_delta, mag_correction, consec_count);
	
	//euler[0] = ;
	euler[1] = mag_heading;
	euler[2] = mag_delta;
	
	// Also tweak the overall drift compensation.
	// DMP still suffers from 'warm-up' issues and this helps greatly.
	//xDriftComp = xDriftComp + (float)(consecCount) * 0.00001;
}

#define SAMPLES_FOR_AUTOCENTER_BITS		6

void do_auto_center(int16_t* euler, uint8_t autocenter)
{
	static int16_t ticks_in_zone = 0;
	static int16_t last_yaw = 0;
	static int32_t sum_yaw = 0;
	static int16_t autocenter_correction = 0;
	
	int16_t yaw_limit = autocenter * 300;

	// have we had a reset?
	if (sample_cnt == 0)
		autocenter_correction = 0;
		
	// apply the current auto-centering
	euler[YAW] += autocenter_correction;
	
	if (abs(euler[YAW]) < yaw_limit					// if we're looking ahead, give or take
			&&  abs(euler[YAW] - last_yaw) < 300	// and not moving
			&&  abs(euler[PITCH]) < 1000)			// and pitch is levelish
	{
		// start counting
		++ticks_in_zone;
		sum_yaw += euler[YAW];
	} else {
		// stop counting
		ticks_in_zone = 0;
		sum_yaw = 0;
	}
	
	last_yaw = euler[YAW];

	// if we stayed looking ahead-ish long enough then adjust yaw offset
	if (ticks_in_zone >= (1<<SAMPLES_FOR_AUTOCENTER_BITS))
	{
		// NB this currently causes a small but visible jump in the
		// view. Useful for debugging!
		autocenter_correction -= sum_yaw >> SAMPLES_FOR_AUTOCENTER_BITS;
		ticks_in_zone = 0;
		sum_yaw = 0;
	}
}

bool process_packet(mpu_packet_t* pckt)
{
	// we're getting the settings pointer here and pass it on to the functions below
	// because get_dongle_settings() isn't really a trivial function
	const FeatRep_DongleSettings __xdata* pSettings = get_dongle_settings();
	
	int16_t euler[3];		// the resulting angles

	quat2euler(pckt->quat, euler);	// convert quaternions to euler angles

	if (pckt->flags & FLAG_RECENTER)
		recenter();

	// magnetometer
	//if (pckt->flags & FLAG_COMPASS_VALID)
	//	do_mag(pckt->compass, euler);

	// calc and/or apply the centering offset
	if (!do_center(euler))
		return false;
	
	// apply the drift compensations
	do_drift(euler, pSettings);

	// save the current yaw angle after drift compensation and before auto-centering
	yaw_value = euler[YAW];

	// do the auto-centering if required
	if (pSettings->autocenter)
		do_auto_center(euler, pSettings->autocenter);

	// do the axis response transformations
	do_response(euler, pSettings);

	// copy the data into the USB report
	usb_joystick_report.x = euler[YAW];
	usb_joystick_report.y = euler[ROLL];
	usb_joystick_report.z = euler[PITCH];

	return true;
}
