#ifndef MPU_H
#define MPU_H

void mpu_init(void);
bool dmp_read_fifo(mpu_packet_t* pckt, uint8_t* more);

void mpu_calibrate_bias(void);

void mpu_get_temperature(int16_t* result);

#endif	// MPU_H
