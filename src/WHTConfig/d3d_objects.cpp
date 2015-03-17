#include "stdafx.h"
#pragma hdrstop

#include "my_utils.h"
#include "my_win.h"
#include "d3d.h"
#include "d3d_objects.h"

void CoordSys::Build()
{
	_vertices.clear();

	// x
	_vertices.push_back(SimpleVertex(1000, 0, 0, D3DCOLOR_XRGB(150, 80, 80)));
	_vertices.push_back(SimpleVertex(-1000, 0, 0, D3DCOLOR_XRGB(150, 80, 80)));

	// y
	_vertices.push_back(SimpleVertex(0, 1000, 0, D3DCOLOR_XRGB(80, 150, 80)));
	_vertices.push_back(SimpleVertex(0, -1000, 0, D3DCOLOR_XRGB(80, 150, 80)));

	// z
	_vertices.push_back(SimpleVertex(0, 0, 1000, D3DCOLOR_XRGB(80, 80, 150)));
	_vertices.push_back(SimpleVertex(0, 0, -1000, D3DCOLOR_XRGB(80, 80, 150)));
}

void EllipsoidAxes::Build(const Point<double>& center, const Point<double>& radii, const Point<double> eigen_vectors[3])
{
	_vertices.clear();

	/*
	BuildCube(_vertices, 2, 2, 2, 0.2f * f1, -0.65f * f1, -0.74f * f1, D3DCOLOR_XRGB(250, 80, 80));
	BuildCube(_vertices, 2, 2, 2, 0.24f * f2, -0.7f * f2, 0.68f * f2, D3DCOLOR_XRGB(80, 250, 80));
	BuildCube(_vertices, 2, 2, 2, 0.95f * f3, 0.31f * f3, -0.02f * f3, D3DCOLOR_XRGB(80, 80, 250));
	*/
}

void MagPoint::Build(int16_t newx, int16_t newy, int16_t newz)
{
	_vertices.clear();

	BuildCube(_vertices, 0.8f, 0.8f, 0.8f, D3DCOLOR_XRGB(190, 190, 190));

	x = newx;
	y = newy;
	z = newz;

	D3DXMATRIX translate;
	D3DXMatrixTranslation(&translate, x, y, z);

	std::for_each(_vertices.begin(), _vertices.end(), SimpleVertex::transform_t(translate));
}
