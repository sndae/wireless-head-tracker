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
private:
	int16_t		_x, _y, _z;

public:
	void Build(DeviceD3D& dev, int16_t x, int16_t y, int16_t z);
};
