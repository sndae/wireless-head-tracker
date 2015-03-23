#ifndef PROC_PACKET_H
#define PROC_PACKET_H

bool process_packet(__xdata tracker_readings_packet_t* pckt);

// drift compensation configuration
void save_x_drift_comp(void);

// recenter requested
void recenter(void);

extern int32_t sample_cnt;
extern int32_t yaw_value;

extern FeatRep_RawMagSamples raw_mag_samples;

#endif	// PROC_PACKET_H
