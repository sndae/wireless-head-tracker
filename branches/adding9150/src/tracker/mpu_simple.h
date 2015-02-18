#ifndef MPU_H
#define MPU_H

void mpu_init(void);
bool dmp_read_fifo(mpu_packet_t* pckt);

int16_t mpu_read_temperature(void);
void mpu_read_compass(mpu_packet_t* pckt);

void mpu_calibrate_bias(void);

#endif	// MPU_H
