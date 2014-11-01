#include <stdint.h>

#include "math_cordic.h"

#define CORDIC_NUM_BITS 	14
#define ASIN_GAIN			0x10000000

// These are CORDIC implementations of the atan2 and asin functions.
// These take up about 2.5kb less flash than the standard C math library
// versions. The return units are not radians, they are scaled to match
// the range we need them in, which is -16383 to 16383.

int16_t __code atanTable[CORDIC_NUM_BITS] =
{
8192,	// 0		== atan(1) * (0x8000 / PI)
4836,   // 1		== atan(0.5) * (0x8000 / PI)
2555,   // 2		== atan(0.25) * (0x8000 / PI)
1297,   // 3		== atan(0.125) * (0x8000 / PI)
651,    // 4		== ...
326,    // 5
163,    // 6
81,     // 7
41,     // 8
20,     // 9
10,     // 10
5,      // 11
3,      // 12
1,      // 13
};     

// We are doing the double iteration variant of the CORDIC algorithm
// This is needed to keep the accuracy good enough for our purposes,
// but comes at a price - it's about 3 times slower than the math
// library implementation.
int16_t iasin_cord(int32_t xf)
{
    uint8_t cnt;
    int16_t result = 0;
    int32_t x = ASIN_GAIN;
	int32_t y = 0;
    int32_t xi, yi;

    for (cnt = 0; cnt < CORDIC_NUM_BITS; cnt++)
    {
		xi = x >> cnt;
		yi = y >> cnt;

        if (y < xf)
		{
			x -= yi;
			y += xi;

			xi = x >> cnt;
			yi = y >> cnt;

			x -= yi;
			y += xi;

			result += 2 * atanTable[cnt];
		} else {
			x += yi;
			y -= xi;

			xi = x >> cnt;
			yi = y >> cnt;

			x += yi;
			y -= xi;

			result -= 2 * atanTable[cnt];
		}

		xf += xf >> (cnt << 1);
    }

    return result;
}

int16_t iatan2_cord(int32_t x, int32_t y)
{
	int16_t result;
	int32_t xi, yi;
	uint8_t cnt;

	if (y < 0)
	{
		result = 0x7fff;

		if (x == 0)
			return result;

		y = -y;
		x = -x;
	} else {
		result = 0;
	}

	for (cnt = 0; cnt < CORDIC_NUM_BITS; cnt++)
	{
		yi = y >> cnt;
		xi = x >> cnt;

		if (x < 0)
		{
			y -= xi;
			x += yi;

			result -= atanTable[cnt];
		} else {
			y += xi;
			x -= yi;

			result += atanTable[cnt];
		}
	}

	return result;
}

// These are floating point version of the CORDIC atan2 and asin functions.
// They are not needed for our project, but I'll keep them here for a while just in case.

/*
#define PI 3.14159265358979323846F

float __code atanTable[] =
{
    7.8539816339744830962E-01f,		//  0
	4.6364760900080611621E-01f,		//  1
	2.4497866312686415417E-01f,		//  2
	1.2435499454676143503E-01f,		//  3
	6.2418809995957348474E-02f,		//  4
	3.1239833430268276254E-02f,		//  5
	1.5623728620476830803E-02f,		//  6
	7.8123410601011112965E-03f,		//  7
    3.9062301319669718276E-03f,		//  8
	1.9531225164788186851E-03f,		//  9
	1.9531225164788186851E-03f,		// 10
	9.7656218955931943040E-04f,		// 11
	4.8828121119489827547E-04f,		// 12
    2.4414062014936176402E-04f,		// 13
	1.2207031189367020424E-04f,		// 14
	6.1035156174208775022E-05f,		// 15
	3.0517578115526096862E-05f,		// 16
    1.5258789061315762107E-05F,		// 17
	7.6293945311019702634E-06F,		// 18
	3.8146972656064962829E-06F,		// 19
	1.9073486328101870354E-06F,		// 20
    9.5367431640596087942E-07F,		// 21
	4.7683715820308885993E-07F,		// 22
	2.3841857910155798249E-07F,		// 23
	1.1920928955078068531E-07F,		// 24
    5.9604644775390554414E-08F,		// 25
	2.9802322387695303677E-08F,		// 26
	1.4901161193847655147E-08F,		// 27
	7.4505805969238279871E-09F,		// 28
    3.7252902984619140453E-09F,		// 29
	1.8626451492309570291E-09F,		// 30
	9.3132257461547851536E-10F,		// 31
};

float asin_cordic(float xf)
{
    uint8_t cnt;
    float result = 0;
    float x = 1;
	float y = 0;
	float powtwo = 1;
    float xi, yi;

    for (cnt = 0; cnt < CORDIC_NUM_BITS; cnt++)
    {
		xi = x * powtwo;
		yi = y * powtwo;

        if (y < xf)
		{
			x -= yi;
			y += xi;

			xi = x * powtwo;
			yi = y * powtwo;

			x -= yi;
			y += xi;

			result += 2 * atanTable[cnt];
		} else {
			x += yi;
			y -= xi;

			xi = x * powtwo;
			yi = y * powtwo;

			x += yi;
			y -= xi;

			result -= 2 * atanTable[cnt];
		}

		xf *= 1.0 + powtwo * powtwo;

		powtwo *= 0.5;
    }

    return result;
}

float atan2_cordic(float y, float x)
{
	float result;
	float xi, yi;
	float powtwo = 1;
	uint8_t cnt;

	if (x == 0)
		return 0;

	if (x < 0)
	{
		result = PI;
		x = -x;
		y = -y;
	} else {
		result = 0;
	}

	for (cnt = 0; cnt < CORDIC_NUM_BITS; cnt++)
	{
		xi = x * powtwo;
		yi = y * powtwo;

		if (y < 0)
		{
			x -= yi;
			y += xi;

			result -= atanTable[cnt];
		} else {
			x += yi;
			y -= xi;

			result += atanTable[cnt];
		}

		powtwo *= 0.5;
	}

	return result;
}
*/
