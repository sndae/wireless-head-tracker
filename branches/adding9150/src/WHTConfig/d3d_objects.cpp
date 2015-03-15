#include "stdafx.h"
#pragma hdrstop

#include "my_utils.h"
#include "my_win.h"
#include "d3d.h"
#include "d3d_objects.h"

void CoordSys::Build()
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

void EllipsoidAxes::Build()
{
	_vertices.clear();

/*
Ellipsoid
Center: {-74.66; -6.6; -6.95}
Radii: {169.67; 174.49; 189.48}
Eigenvalues: [3.4737246304197866E-5, 3.284374642192236E-5, 2.7853729936035815E-5]
Eigenvector 0: {0.2; -0.65; -0.74}
Eigenvector 1: {0.24; -0.7; 0.68}
Eigenvector 2: {0.95; 0.31; -0.02}
*/

	/*
	float x_angle, y_angle, z_angle;
	D3DXVECTOR3 cv, ev;

	cv.x = 1;
	cv.y = 0;
	cv.z = 0;

	ev.x = 0.2f;
	ev.y = -0.65f;
	ev.z = -0.74f;

	x_angle = GetAngle(cv, ev);
	*/

	const float f1 = 169.67f;
	const float f2 = 174.49f;
	const float f3 = 189.48f;

	BuildCube(_vertices, 2, 2, 2, 0.2f * f1, -0.65f * f1, -0.74f * f1);
	BuildCube(_vertices, 2, 2, 2, 0.24f * f2, -0.7f * f2, 0.68f * f2);
	BuildCube(_vertices, 2, 2, 2, 0.95f * f3, 0.31f * f3, -0.02f * f3);

	int vert_per_cube = _vertices.size() / 3;

	// set the color
	std::for_each(_vertices.begin() + vert_per_cube*0, _vertices.begin() + vert_per_cube*1, [] (SimpleVertex& v) { v.diffuse = D3DCOLOR_XRGB(250, 80, 80); } );
	std::for_each(_vertices.begin() + vert_per_cube*1, _vertices.begin() + vert_per_cube*2, [] (SimpleVertex& v) { v.diffuse = D3DCOLOR_XRGB(80, 250, 80); } );
	std::for_each(_vertices.begin() + vert_per_cube*2, _vertices.begin() + vert_per_cube*3, [] (SimpleVertex& v) { v.diffuse = D3DCOLOR_XRGB(80, 80, 250); } );
}

void MagPoint::Build(int16_t newx, int16_t newy, int16_t newz)
{
	_vertices.clear();

	BuildCube(_vertices, 0.8f, 0.8f, 0.8f);

	x = newx;
	y = newy;
	z = newz;

	/*
	const float xoff = -74.66f;
	const float yoff = -6.6f;
	const float zoff = -6.95f;
	*/

	D3DXMATRIX translate;
	D3DXMatrixTranslation(&translate, x, y, z);

	std::for_each(_vertices.begin(), _vertices.end(), SimpleVertex::transform_t(translate));

	// set the color
	std::for_each(_vertices.begin(), _vertices.end(), [] (SimpleVertex& v) { v.diffuse = D3DCOLOR_XRGB(190, 190, 190); });
}
