#include <stdint.h>

#ifdef MATH_CORDIC

#define PI 3.14159265358979323846F

#include "math_cordic.h"

#define CORDIC_NUM_BITS 	32
#define ASIN_GAIN			16384

float __code atanTable[CORDIC_NUM_BITS] = {
    7.8539816339744830962E-01F,
	4.6364760900080611621E-01F,
	2.4497866312686415417E-01F,
	1.2435499454676143503E-01F,
	6.2418809995957348474E-02F,
	3.1239833430268276254E-02F,
	1.5623728620476830803E-02F,
	7.8123410601011112965E-03F,
    3.9062301319669718276E-03F,
	1.9531225164788186851E-03F,
	9.7656218955931943040E-04F,
	4.8828121119489827547E-04F,
    2.4414062014936176402E-04F,
	1.2207031189367020424E-04F,
	6.1035156174208775022E-05F,
	3.0517578115526096862E-05F,
    1.5258789061315762107E-05F,
	7.6293945311019702634E-06F,
	3.8146972656064962829E-06F,
	1.9073486328101870354E-06F,
    9.5367431640596087942E-07F,
	4.7683715820308885993E-07F,
	2.3841857910155798249E-07F,
	1.1920928955078068531E-07F,
    5.9604644775390554414E-08F,
	2.9802322387695303677E-08F,
	1.4901161193847655147E-08F,
	7.4505805969238279871E-09F,
    3.7252902984619140453E-09F,
	1.8626451492309570291E-09F,
	9.3132257461547851536E-10F,
	4.6566128730773925778E-10F,
	};

float asin(float xf)
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

		xf *= 1.0F + powtwo * powtwo;

		powtwo *= 0.5;
    }

    return result;
}

float atan2(float y, float x)
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

/*
// this is a CORDIC algorithm with double iteration
float iasin_cord(int32_t xf)
{
    uint8_t cnt;
    float result = 0;
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

float iatan2_cord(int32_t y, int32_t x)
{
	float result;
	int32_t xi, yi;
	uint8_t cnt;

	if (x == 0)
		return 0;

	if (x < 0)
	{
		result = 180;
		x = -x;
		y = -y;
	} else {
		result = 0;
	}

	for (cnt = 0; cnt < CORDIC_NUM_BITS; cnt++)
	{
		xi = x >> cnt;
		yi = y >> cnt;

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
	}

	return result;
}
*/

#endif	// MATH_CORDIC