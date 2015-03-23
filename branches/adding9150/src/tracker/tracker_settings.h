#ifndef SETTINGS_H
#define SETTINGS_H

// this is the structure we save into the
// non-volatile section of the nRF24LE1
typedef struct
{
	uint8_t		is_empty;		// 0xff if empty, 0x00 if used

	uint8_t		is_calibrated;
	
	int16_t		gyro_bias[3];
	int16_t		accel_bias[3];

	/*
	uint8_t		rf_power;		// vRF_PWR_M18DBM
								// vRF_PWR_M12DBM
								// vRF_PWR_M6DBM
								// vRF_PWR_0DBM
								*/
} tracker_settings_t;

const tracker_settings_t __xdata * get_tracker_settings(void);
void save_tracker_settings(tracker_settings_t* pNewSettings);

// these are for testing
//void list_settings(void);
//void test_settings(void);

#endif	// SETTINGS_H
