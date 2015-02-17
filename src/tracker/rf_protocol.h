#ifndef RF_PROTOCOL_H
#define RF_PROTOCOL_H

#define NRF_ADDR_SIZE	5

// the channel number is hard-coded for the moment
#define CHANNEL_NUM		115

extern __code const uint8_t HeadAddr[NRF_ADDR_SIZE];
extern __code const uint8_t DongleAddr[NRF_ADDR_SIZE];

// this message is sent to the USB dongle over the radio
enum mpu_packet_flags
{
	FLAG_RECENTER			= 0x01,
	FLAG_VOLTAGE_VALID		= 0x02,
	FLAG_TEMPERATURE_VALID	= 0x04,
	FLAG_COMPASS_VALID		= 0x08,
};

typedef struct
{
	uint8_t		flags;			// bits defined in mpu_packet_flags
	int16_t		gyro[3];
	int16_t		accel[3];
	int16_t		quat[4];
	int16_t		voltage;		// in units of 100th of a Volt, so 289 means 2.89V
	int16_t		temperature;	// in units of 10th of a Celsius, so 289 means 28.9C
	int16_t		compass[3];
} mpu_packet_t;

#endif		// RF_PROTOCOL_H
