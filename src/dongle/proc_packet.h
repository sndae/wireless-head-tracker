#ifndef PROC_PACKET_H
#define PROC_PACKET_H

bool process_packet(mpu_packet_t* pckt);

// drift compensation configuration
void save_x_drift_comp(void);

// recenter initiated from the PC
void recenter(void);

extern int32_t sample_cnt;
extern int32_t yaw_value;

extern FeatRep_MagRawData mag_data_samples;

#endif	// PROC_PACKET_H
