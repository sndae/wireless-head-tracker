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

	void Build(const Point<double>& center, const double radii[3], double evecs[3][3]);
};

//
// one magnetometer measurement
//

class MagPoint: public Object3D
{
public:
	Point<int16_t>	point;

	void Build(const Point<int16_t>& p);
	void BuildCalibrated(const Point<int16_t>& p, const Point<double>& center, const double calibMatrix[3][3]);
};
