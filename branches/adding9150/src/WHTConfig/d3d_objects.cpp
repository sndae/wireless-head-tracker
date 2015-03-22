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

void EllipsoidAxes::Build(const Point<double>& center, const double radii[3], const double evecs[3][3])
{
	_vertices.clear();

	// create the ellipsoid principal axes
	D3DXVECTOR3 vs(0, 0, 0);
	D3DXVECTOR3 vx((float) evecs[0][0], (float) evecs[0][1], (float) evecs[0][2]);
	D3DXVECTOR3 vy((float) evecs[1][0], (float) evecs[1][1], (float) evecs[1][2]);
	D3DXVECTOR3 vz((float) evecs[2][0], (float) evecs[2][1], (float) evecs[2][2]);

	vx = vx * (float) radii[0];
	vy = vy * (float) radii[1];
	vz = vz * (float) radii[2];

	_vertices.push_back(SimpleVertex(vs, D3DCOLOR_XRGB(255, 200, 200)));
	_vertices.push_back(SimpleVertex(vx, D3DCOLOR_XRGB(255, 200, 200)));

	_vertices.push_back(SimpleVertex(vs, D3DCOLOR_XRGB(200, 255, 200)));
	_vertices.push_back(SimpleVertex(vy, D3DCOLOR_XRGB(200, 255, 200)));

	_vertices.push_back(SimpleVertex(vs, D3DCOLOR_XRGB(200, 200, 255)));
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

	// point moved to center
	D3DXVECTOR3 vpoint(float(p.x - center.x), float(p.y - center.y), float(p.z - center.z));

	// the calibration matrix
	D3DXMATRIX calib;
	D3DXMatrixIdentity(&calib);
	calib._11 = (float) calibMatrix[0][0];
	calib._12 = (float) calibMatrix[0][1];
	calib._13 = (float) calibMatrix[0][2];

	calib._21 = (float) calibMatrix[1][0];
	calib._22 = (float) calibMatrix[1][1];
	calib._23 = (float) calibMatrix[1][2];

	calib._31 = (float) calibMatrix[2][0];
	calib._32 = (float) calibMatrix[2][1];
	calib._33 = (float) calibMatrix[2][2];

	// calc the calibrated vector length
	D3DXVECTOR4 vout;
	D3DXVec3Transform(&vout, &vpoint, &calib);
	double vlen = sqrt(vout.x*vout.x + vout.y*vout.y + vout.z*vout.z);
	D3DCOLOR col;
	const int BASE_COL = 0x60;
	if (vlen < CALIBRATED_SCALE)
	{
		size_t c = BASE_COL + int((CALIBRATED_SCALE - vlen)*1.5);
		if (c > 0xff)
			c = 0xff;

		col = D3DCOLOR_XRGB(c, BASE_COL, BASE_COL);
	} else {
		size_t c = BASE_COL + int((vlen - CALIBRATED_SCALE)*1.5);
		if (c > 0xff)
			c = 0xff;

		col = D3DCOLOR_XRGB(BASE_COL, BASE_COL, c);
	}

	// put the cube to its calibrated position
	BuildCube(_vertices, 6, 6, 6, col);

	D3DXMATRIX translate;
	D3DXMatrixTranslation(&translate, vout.x, vout.y, vout.z);

	std::for_each(_vertices.begin(), _vertices.end(), SimpleVertex::transform_t(translate));
}