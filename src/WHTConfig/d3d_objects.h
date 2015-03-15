#pragma once

//
// the coordinate system
//

class CoordSys: public Object3D
{
public:
	void Build();
};

//
// the eigen vectors
//

class EllipsoidAxes: public Object3D
{
public:
	void Build();
};

//
// one magnetometer measurement
//

class MagPoint: public Object3D
{
public:
	int16_t		x, y, z;

	void Build(int16_t newx, int16_t newy, int16_t newz);
};
