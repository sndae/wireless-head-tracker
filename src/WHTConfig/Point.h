#pragma once

struct Point
{
	int16_t		x, y, z;

	Point()
		: x(0), y(0), z(0)
	{}

	Point(int16_t nx, int16_t ny, int16_t nz)
		: x(nx), y(ny), z(nz)
	{}

	bool operator < (const Point& lhs) const
	{
		return x == lhs.x ? (y == lhs.y ? z < lhs.z : y < lhs.y) : x < lhs.x;
	}
};

