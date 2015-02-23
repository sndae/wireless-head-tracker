// These are tests for the cordic trigonometric functions in ../dongle/math_cordic.c

// we need this so that the STD library defines M_PI
#define _USE_MATH_DEFINES

#include <stdio.h>
#include <math.h>
#include <stdint.h>

extern "C"
{
#include "../dongle/math_cordic.h"
}

// this creates the atanTable needed in math_cordic.c
void create_atan_table()
{
	float val = 1;
	for (int i = 0; i < CORDIC_TABLE_SIZE; i++)
	{
		double atanVal = atanf(val) * (ANGLE_PI / M_PI);
		int atanValInt = int(atanVal + 0.5);

		printf("%d,\t// %i\tatan(%g) * (CORDIC_RANGE / M_PI)\n", atanValInt, i, val);
		val /= 2;
	}
}

void calc_cordic_gain()
{
	double atanarg = 1;
	double cordic_gain = 1;
	for (int i = 0; i < CORDIC_TABLE_SIZE; i++)
	{
		cordic_gain *= cos(atan(atanarg));
		atanarg /= 2;
	}

	printf("CORDIC_GAIN = %18.16f\n", cordic_gain);
}

void test_sincos()
{
	int32_t s, c;

	for (int32_t angle = -ANGLE_PI; angle <= ANGLE_PI; angle += ANGLE_PI/45)
	{
		isincos_cord(angle, &c, &s);

		float rad_angle = float((double)angle / ANGLE_PI * M_PI);
		int32_t sc = int32_t(sinf(rad_angle) * ANGLE_PI + .5);
		int32_t cc = int32_t(cosf(rad_angle) * ANGLE_PI + .5);

		printf("a=%4d sc=%6i s=%6i cc=%6i c=%6i sdif=%6i cdif=%6i\n", angle * 180 / ANGLE_PI, s, sc, c, cc, s-sc, c-cc);
	}
}

#define ATAN_RANGE		100000

void test_atan()
{
	for (int32_t x = -ATAN_RANGE; x < ATAN_RANGE; x += ATAN_RANGE / 100)
	{
		for (int32_t y = -ATAN_RANGE; y < ATAN_RANGE; y += ATAN_RANGE / 100)
		{
			int32_t res = int32_t(atan2((double) x, (double) y) / M_PI * ANGLE_PI + .5);
			int32_t resc = iatan2_cord(x, y);

			printf("math=%8i  cord=%8i  diff=%6i\n", res, resc, resc - res);
		}
	}
}

void main()
{
	test_atan();
}
