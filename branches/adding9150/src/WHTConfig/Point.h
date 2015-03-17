#pragma once

template <class T>
struct Point
{
	T		x, y, z;

	Point()
		: x(0), y(0), z(0)
	{}

	Point(T nx, T ny, T nz)
		: x(nx), y(ny), z(nz)
	{}

	bool operator < (const Point& lhs) const
	{
		return x == lhs.x ? (y == lhs.y ? z < lhs.z : y < lhs.y) : x < lhs.x;
	}
};
