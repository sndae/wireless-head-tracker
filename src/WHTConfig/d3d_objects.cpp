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
	_vertices.push_back(SimpleVertex(1000, 0, 0, D3DCOLOR_XRGB(230, 80, 80)));
	_vertices.push_back(SimpleVertex(-1000, 0, 0, D3DCOLOR_XRGB(60, 80, 80)));

	// y
	_vertices.push_back(SimpleVertex(0, 1000, 0, D3DCOLOR_XRGB(80, 230, 80)));
	_vertices.push_back(SimpleVertex(0, -1000, 0, D3DCOLOR_XRGB(80, 60, 80)));

	// z
	_vertices.push_back(SimpleVertex(0, 0, 1000, D3DCOLOR_XRGB(80, 80, 230)));
	_vertices.push_back(SimpleVertex(0, 0, -1000, D3DCOLOR_XRGB(80, 80, 60)));
}

void EllipsoidAxes::Build(const Point<double>& center, const double radii[3], double evecs[3][3])
{
	// create the ellipsoid principal axes
	D3DXVECTOR3 vx((float) evecs[0][0], (float) evecs[0][1], (float) evecs[0][2]);
	D3DXVECTOR3 vy((float) evecs[1][0], (float) evecs[1][1], (float) evecs[1][2]);
	D3DXVECTOR3 vz((float) evecs[2][0], (float) evecs[2][1], (float) evecs[2][2]);

	vx = vx * (float) radii[0];
	vy = vy * (float) radii[1];
	vz = vz * (float) radii[2];

	_vertices.push_back(SimpleVertex(-vx, D3DCOLOR_XRGB(255, 200, 200)));
	_vertices.push_back(SimpleVertex(vx, D3DCOLOR_XRGB(255, 200, 200)));

	_vertices.push_back(SimpleVertex(-vy, D3DCOLOR_XRGB(200, 255, 200)));
	_vertices.push_back(SimpleVertex(vy, D3DCOLOR_XRGB(200, 255, 200)));

	_vertices.push_back(SimpleVertex(-vz, D3DCOLOR_XRGB(200, 200, 255)));
	_vertices.push_back(SimpleVertex(vz, D3DCOLOR_XRGB(200, 200, 255)));

	// move them to the ellipsoid center
	D3DXMATRIX translate;
	D3DXMatrixTranslation(&translate, (float) center.x, (float) center.y, (float) center.z);
	
	std::for_each(_vertices.begin(), _vertices.end(), SimpleVertex::transform_t(translate));
}

void MagPoint::Build(const Point<int16_t>& p)
{
	_vertices.clear();

	BuildCube(_vertices, 0.8f, 0.8f, 0.8f, D3DCOLOR_XRGB(190, 190, 190));

	point = p;

	D3DXMATRIX translate;
	D3DXMatrixTranslation(&translate, point.x, point.y, point.z);

	std::for_each(_vertices.begin(), _vertices.end(), SimpleVertex::transform_t(translate));
}

void MagPoint::BuildCalibrated(const Point<int16_t>& p, const Point<double>& center, const double calibMatrix[3][3])
{
	_vertices.clear();

	BuildCube(_vertices, 0.8f, 0.8f, 0.8f, D3DCOLOR_XRGB(200, 80, 80));

	point = p;

	D3DXMATRIX translate;
	D3DXMatrixTranslation(&translate, point.x - (float) center.x, point.y - (float) center.y, point.z - (float) center.z);

	std::for_each(_vertices.begin(), _vertices.end(), SimpleVertex::transform_t(translate));

	D3DXMATRIX rotscale;
	D3DXMatrixIdentity(&rotscale);
	rotscale._11 = (float) calibMatrix[0][0];
	rotscale._12 = (float) calibMatrix[0][1];
	rotscale._13 = (float) calibMatrix[0][2];

	rotscale._21 = (float) calibMatrix[1][0];
	rotscale._22 = (float) calibMatrix[1][1];
	rotscale._23 = (float) calibMatrix[1][2];

	rotscale._31 = (float) calibMatrix[2][0];
	rotscale._32 = (float) calibMatrix[2][1];
	rotscale._33 = (float) calibMatrix[2][2];

	D3DXMATRIX scale100;
	D3DXMatrixScaling(&scale100, 500, 500, 500);

	D3DXMATRIX result = rotscale * scale100;

	std::for_each(_vertices.begin(), _vertices.end(), SimpleVertex::transform_t(result));
}