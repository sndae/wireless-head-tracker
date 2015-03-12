#pragma once

//
// the coordinate system
//

class CoordSys: public Object3D
{
public:
	void Build(DeviceD3D& dev);
};

//
// one magnetometer measurement
//

class MagPoint: public Object3D
{
public:
	int16_t		x, y, z;

	void Build(DeviceD3D& dev, int16_t newx, int16_t newy, int16_t newz);
};
