#ifndef MATH_CORDIC_H
#define MATH_CORDIC_H

#define CORDIC_NUM_BITS 	14
#define CORDIC_RANGE		0x7fff

void isincos_cord(int32_t angle, int32_t* rcos, int32_t* rsin);
int16_t iatan2_cord(int32_t y, int32_t x);
int16_t iasin_cord(int32_t x);

#endif	// MATH_CORDIC_H