#ifndef RF_PROTOCOL_H
#define RF_PROTOCOL_H

// we use the full, 5 byte address length
#define NRF_ADDR_SIZE	5

// define RF data rate
#define NRF_DATA_RATE	vRF_DR_1MBPS

// the channel number is hard-coded for the moment
#define CHANNEL_NUM		115

// these are the addresses
extern __code const uint8_t HeadAddr[NRF_ADDR_SIZE];
extern __code const uint8_t DongleAddr[NRF_ADDR_SIZE];

// this message is sent to the USB dongle over the radio
enum mpu_packet_flags
{
	FLAG_RECENTER		= 0x01,
	FLAG_VOLTAGE_VALID	= 0x02,
	FLAG_MAG_VALID		= 0x04,
};

typedef struct
{
	uint8_t		flags;			// bits defined in mpu_packet_flags
	uint16_t	pckt_cnt;		// packet counter - used for session
	int16_t		quat[4];
	int16_t		mag[3];			// the magnetometer readings
	int16_t		voltage;		// in units of 100th of a Volt, so 289 means 2.89V
	int16_t		temperature;	// in units of 10th of a Celsius, so 289 means 28.9C
} tracker_readings_packet_t;

#endif		// RF_PROTOCOL_H
