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

void EllipsoidAxes::Build(const Point<double>& center, const Point<double>& radii, const Point<double> eigen_vectors[3])
{
	_vertices.clear();

	D3DXVECTOR3 vx((float) eigen_vectors[0].x, (float) eigen_vectors[0].y, (float) eigen_vectors[0].z);
	D3DXVECTOR3 vy((float) eigen_vectors[1].x, (float) eigen_vectors[1].y, (float) eigen_vectors[1].z);
	D3DXVECTOR3 vz((float) eigen_vectors[2].x, (float) eigen_vectors[2].y, (float) eigen_vectors[2].z);

	float angleX = GetAngle(vx, D3DXVECTOR3(1, 0, 0));
	float angleY = GetAngle(vy, D3DXVECTOR3(0, 1, 0));
	float angleZ = GetAngle(vz, D3DXVECTOR3(0, 0, 1));

	vx = vx * (float) radii.x;
	vy = vy * (float) radii.y;
	vz = vz * (float) radii.z;

	_vertices.push_back(SimpleVertex(-vx, D3DCOLOR_XRGB(255, 200, 200)));
	_vertices.push_back(SimpleVertex(vx, D3DCOLOR_XRGB(255, 200, 200)));

	_vertices.push_back(SimpleVertex(-vy, D3DCOLOR_XRGB(200, 255, 200)));
	_vertices.push_back(SimpleVertex(vy, D3DCOLOR_XRGB(200, 255, 200)));

	_vertices.push_back(SimpleVertex(-vz, D3DCOLOR_XRGB(200, 200, 255)));
	_vertices.push_back(SimpleVertex(vz, D3DCOLOR_XRGB(200, 200, 255)));

	D3DXMATRIX translate;
	D3DXMatrixTranslation(&translate, (float) center.x, (float) center.y, (float) center.z);

	std::for_each(_vertices.begin(), _vertices.end(), SimpleVertex::transform_t(translate));

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
