#include "stdafx.h"
#pragma hdrstop

#include "my_utils.h"
#include "my_win.h"
#include "d3d.h"
#include "d3d_objects.h"

void CoordSys::Build(DeviceD3D& dev)
{
	_vertices.clear();

	BuildCube(_vertices, 1000.0f, 0.5f, 0.5f);
	BuildCube(_vertices, 0.5f, 1000.0f, 0.5f);
	BuildCube(_vertices, 0.5f, 0.5f, 1000.0f);

	int vert_per_cube = _vertices.size() / 3;

	// set the color
	std::for_each(_vertices.begin() + vert_per_cube*0, _vertices.begin() + vert_per_cube*1, [] (SimpleVertex& v) { v.diffuse = D3DCOLOR_XRGB(150, 80, 80); } );
	std::for_each(_vertices.begin() + vert_per_cube*1, _vertices.begin() + vert_per_cube*2, [] (SimpleVertex& v) { v.diffuse = D3DCOLOR_XRGB(80, 150, 80); } );
	std::for_each(_vertices.begin() + vert_per_cube*2, _vertices.begin() + vert_per_cube*3, [] (SimpleVertex& v) { v.diffuse = D3DCOLOR_XRGB(80, 80, 150); } );
}

void MagPoint::Build(DeviceD3D& dev, int16_t newx, int16_t newy, int16_t newz)
{
	BuildCube(_vertices, 0.8f, 0.8f, 0.8f);

	x = newx;
	y = newy;
	z = newz;

	D3DXMATRIX translate;
	D3DXMatrixTranslation(&translate, newx, newy, newz);

	std::for_each(_vertices.begin(), _vertices.end(), SimpleVertex::transform_t(translate));

	// set the color
	std::for_each(_vertices.begin(), _vertices.end(), [] (SimpleVertex& v) { v.diffuse = D3DCOLOR_XRGB(190, 190, 190); });
}
