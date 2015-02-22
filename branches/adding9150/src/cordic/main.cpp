#define _USE_MATH_DEFINES		// we need this for M_PI

#include <stdio.h>
#include <math.h>
#include <stdint.h>

extern "C" {
#include "../dongle/math_cordic.h"
}

void create_atan_table()
{
	float val = 1;
	for (int i = 0; i < CORDIC_NUM_BITS; i++)
	{
		double atanVal = atanf(val) * (CORDIC_RANGE / M_PI);
		int atanValInt = int(atanVal + 0.5);

		printf("%d,\t// %i\tatan(%g) * (CORDIC_RANGE / M_PI)\n", atanValInt, i, val);
		val /= 2;
	}
}

void test_sincos()
{
	int32_t s, c;

	for (int32_t angle = 119; angle < 180; angle++)
	{
		isincos_cord(int32_t(angle * 182.03888 + .5), &c, &s);

		int32_t sc = int32_t(sinf(angle / (float) 180.0 * (float) M_PI) * CORDIC_RANGE + .5);
		int32_t cc = int32_t(cosf(angle / (float) 180.0 * (float) M_PI) * CORDIC_RANGE + .5);

		printf("sincord=%6i sin=%6i coscord=%6i cos=%6i sindif=%6i cosdif=%6i\n", s, sc, c, cc, s-sc, c-cc);
	}
}

int main()
{
	test_sincos();
	return 0;
}
