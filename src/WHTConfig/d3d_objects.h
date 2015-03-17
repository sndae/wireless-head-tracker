#pragma once

#include "Point.h"

//
// the coordinate system
//

class CoordSys : public Object3D
{
public:
	CoordSys()
	{
		_vertex_buffer.SetPrimitiveType(D3DPT_LINELIST);
	}

	void Build();
};

//
// the eigen vectors
//

class EllipsoidAxes: public Object3D
{
public:
	EllipsoidAxes()
	{
		_vertex_buffer.SetPrimitiveType(D3DPT_LINELIST);
	}

	void Build(const Point<double>& center, const Point<double>& radii, const Point<double> eigen_vectors[3]);
};

//
// one magnetometer measurement
//

class MagPoint: public Object3D, Point<int16_t>
{
public:
	int16_t		x, y, z;

	void Build(int16_t newx, int16_t newy, int16_t newz);
};
