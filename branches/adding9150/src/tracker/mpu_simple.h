#ifndef MPU_H
#define MPU_H

typedef struct
{
	uint8_t		has_mag;		// != 0 if mag[] is valid
	int16_t		quat[4];
	int16_t		accel[3];
	int16_t		gyro[3];
	int16_t		mag[3];			// the magnetometer readings
} mpu_readings_t;

void mpu_init(void);
bool dmp_read_fifo(mpu_readings_t* mpu_rd);

int16_t mpu_read_temperature(void);
void mpu_read_compass(mpu_readings_t* mpu_rd);

void mpu_calibrate_bias(void);

#endif	// MPU_H
