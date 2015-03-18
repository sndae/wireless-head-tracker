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

#define d2r(deg)		float((deg) / 180.0 * M_PI)
#define r2d(rad)		float((rad) / M_PI * 180.0)

void EllipsoidAxes::Build(const Point<double>& center, const Point<double>& radii, const Point<double> eigen_vectors[3])
{
	_vertices.clear();

	D3DXMATRIX translate;
	D3DXMatrixTranslation(&translate, 1, 1, 1);

	/*
	D3DXVECTOR3 vs(  0,   0,   0);
	D3DXVECTOR3 vx(100,   0,   0);
	D3DXVECTOR3 vy(  0, 100,   0);
	D3DXVECTOR3 vz(  0,   0, 100);

	_vertices.push_back(SimpleVertex(vs, D3DCOLOR_XRGB(255, 200, 200)));
	_vertices.push_back(SimpleVertex(vx, D3DCOLOR_XRGB(255, 200, 200)));

	_vertices.push_back(SimpleVertex(vs, D3DCOLOR_XRGB(200, 255, 200)));
	_vertices.push_back(SimpleVertex(vy, D3DCOLOR_XRGB(200, 255, 200)));

	_vertices.push_back(SimpleVertex(vs, D3DCOLOR_XRGB(200, 200, 255)));
	_vertices.push_back(SimpleVertex(vz, D3DCOLOR_XRGB(200, 200, 255)));

	// do a rotation
	D3DXMATRIX rotx, roty, rotall;
	D3DXMatrixRotationX(&rotx, d2r(12));
	D3DXMatrixRotationY(&roty, d2r(34));
	rotall = rotx * roty;
	std::for_each(_vertices.begin(), _vertices.end(), SimpleVertex::transform_t(rotx));
	std::for_each(_vertices.begin(), _vertices.end(), SimpleVertex::transform_t(roty));

	float x = _vertices[1].pos.x;
	float y = _vertices[1].pos.y;
	float z = _vertices[1].pos.z;

	float theta = r2d(atan2(-z, y));
	float alpha = r2d(atan2(cos(theta) * y - sin(theta) * z, -x));
	*/

	/*
	debug(L"-----------------");
	debug(L"atan");

	float angleH = r2d(atan2(_vertices[3].pos.z, _vertices[3].pos.x));
	float angleV = r2d(atan2(_vertices[3].pos.y, _vertices[3].pos.z));

	debug(flt2str(angleH));
	debug(flt2str(angleV));

	debug(L"asin");

	angleH = r2d(asin(_vertices[3].pos.z / _vertices[3].pos.x));
	angleV = r2d(asin(_vertices[3].pos.y / _vertices[3].pos.z));

	debug(flt2str(angleH));
	debug(flt2str(angleV));
	*/

	//::PostQuitMessage(0);

	/*
	float angleXY_Y = (float) atan2(eigen_vectors[0].x, eigen_vectors[0].y);
	float angleZX_X = (float) atan2(eigen_vectors[0].z, eigen_vectors[0].x);

	// to avoid overlap with the coordinate axes
	D3DXMATRIX translate;
	D3DXMatrixTranslation(&translate, 0.5, 0.5, 0.5);
	
	std::for_each(_vertices.begin(), _vertices.end(), SimpleVertex::transform_t(translate));

	D3DXVECTOR3 vx((float) eigen_vectors[0].x, (float) eigen_vectors[0].y, (float) eigen_vectors[0].z);
	D3DXVECTOR3 vy((float) eigen_vectors[1].x, (float) eigen_vectors[1].y, (float) eigen_vectors[1].z);
	D3DXVECTOR3 vz((float) eigen_vectors[2].x, (float) eigen_vectors[2].y, (float) eigen_vectors[2].z);

	vx = vx * (float) radii.x;
	vy = vy * (float) radii.y;
	vz = vz * (float) radii.z;

	_vertices.push_back(SimpleVertex(-vx, D3DCOLOR_XRGB(255, 200, 200)));
	_vertices.push_back(SimpleVertex(vx, D3DCOLOR_XRGB(255, 200, 200)));

	_vertices.push_back(SimpleVertex(-vy, D3DCOLOR_XRGB(200, 255, 200)));
	_vertices.push_back(SimpleVertex(vy, D3DCOLOR_XRGB(200, 255, 200)));

	_vertices.push_back(SimpleVertex(-vz, D3DCOLOR_XRGB(200, 200, 255)));
	_vertices.push_back(SimpleVertex(vz, D3DCOLOR_XRGB(200, 200, 255)));

	float angleXY_Y = (float) atan2(eigen_vectors[0].x, eigen_vectors[0].y);
	float angleZX_X = (float) atan2(eigen_vectors[0].z, eigen_vectors[0].x);

	float angleY = angleXY_Y / M_PI * 180;
	float angleX = angleZX_X / M_PI * 180;

	SimpleVertex vs(0, 0, 0, D3DCOLOR_XRGB(255, 255, 0));
	SimpleVertex ve(0, 0, 100, D3DCOLOR_XRGB(255, 255, 0));

	D3DXMATRIX rotY, rotX;
	D3DXMatrixRotationY(&rotY, angleXY_Y);
	D3DXMatrixRotationY(&rotX, angleZX_X);
	SimpleVertex::transform_t ty(rotY);
	SimpleVertex::transform_t tx(rotX);
	ty(ve);
	tx(ve);

	_vertices.push_back(vs);
	_vertices.push_back(ve);
	*/

	//D3DXMATRIX translate;
	//D3DXMatrixTranslation(&translate, (float) center.x, (float) center.y, (float) center.z);
	//
	//std::for_each(_vertices.begin(), _vertices.end(), SimpleVertex::transform_t(translate));

	/*
	D3DXMATRIX rotx, roty, rotz;
	D3DXMatrixRotationX(&rotx, -angleX);
	D3DXMatrixRotationY(&roty, -angleY);
	D3DXMatrixRotationZ(&rotz, -angleZ);
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
