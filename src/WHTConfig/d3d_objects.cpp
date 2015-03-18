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

void EllipsoidAxes::Build(const Point<double>& center, const double radii[3], double evecs[3][3], double evals[3])
{
	int or[3];
		
	or[0] = 0;  // orientation vector, has info which radii is which
	for (int i=1; i<3; ++i) 
		if (fabs(evecs[0][or[0]]) < fabs(evecs[0][i]))
			or[0] = i;
	
	or[1] = 0;
	for(int i=1; i<3; ++i) 
		if (fabs(evecs[1][or[1]]) < fabs(evecs[1][i]))
			or[1] = i;

	or[2] = 0;
	for(int i=1; i<3; ++i) 
		if (fabs(evecs[2][or[2]]) < fabs(evecs[2][i]))
			or[2] = i;

	// get eigenvectors into array 3x3 in correct order
	double ev[3][3];
	double lambda[3];
	for(int i=0; i<3; ++i)
	{
		ev[or[0]][i] = evecs[0][i];
		ev[or[1]][i] = evecs[1][i];
		ev[or[2]][i] = evecs[2][i];
		lambda[or[i]] = radii[i]; // gets radii in correct order
	}

	// calculates W matrix
	double w[3][3];
	for(int i=0; i<3; ++i)
	{
		for(int j=0; j<3; ++j)
		{
			w[i][j] = 0;
			for(int k=0; k<3; ++k)
				w[i][j] += ev[k][i] * ev[k][j] / lambda[k];
		}
	}

	/*
	mx -= fp.center.toArray()[0]; // compensates for hard iron offset
	my -= fp.center.toArray()[1];
	mz -= fp.center.toArray()[2];
                    
	Cx = mx * w[0][0] + my * w[0][1] + mz * w[0][2];  // compensates for soft iron offset
	Cy = mx * w[1][0] + my * w[1][1] + mz * w[1][2];
	Cz = mx * w[2][0] + my * w[2][1] + mz * w[2][2];
	*/

	D3DXMATRIX eigen;
	D3DXMatrixIdentity(&eigen);
	eigen._11 = (float) w[0][0];
	eigen._12 = (float) w[0][1];
	eigen._13 = (float) w[0][2];

	eigen._21 = (float) w[1][0];
	eigen._22 = (float) w[1][1];
	eigen._23 = (float) w[1][2];

	eigen._31 = (float) w[2][0];
	eigen._32 = (float) w[2][1];
	eigen._33 = (float) w[2][2];

	/*
	// the matrix from eigenvectors
	D3DXMATRIX evecm, evecim;
	D3DXMatrixIdentity(&evecm);
	evecm._11 = (float) eigen_vectors[0].x;
	evecm._21 = (float) eigen_vectors[0].y;
	evecm._31 = (float) eigen_vectors[0].z;

	evecm._12 = (float) eigen_vectors[1].x;
	evecm._22 = (float) eigen_vectors[1].y;
	evecm._32 = (float) eigen_vectors[1].z;

	evecm._13 = (float) eigen_vectors[2].x;
	evecm._23 = (float) eigen_vectors[2].y;
	evecm._33 = (float) eigen_vectors[2].z;

	// the inverse of evecm
	D3DXMatrixInverse(&evecim, NULL, &evecm);

	// the matrix of eigenvalues
	D3DXMATRIX evalm;
	D3DXMatrixIdentity(&evalm);
	evalm._11 = (float) sqrt(1/eigen_values.x);
	evalm._22 = (float) sqrt(1/eigen_values.y);
	evalm._33 = (float) sqrt(1/eigen_values.z);

	//D3DXMATRIX sc;
	//D3DXMatrixScaling(&sc, 1000, 1000, 1000);

	D3DXMATRIX emres = evecm * evalm * evecim;
	*/


	/*
	_vertices.clear();

	double m[3][3];

	// make a matrix
	m[0][0] = eigen_vectors[0].x;	m[0][1] = eigen_vectors[1].x;	m[0][2] = eigen_vectors[2].x;
	m[1][0] = eigen_vectors[0].y;	m[1][1] = eigen_vectors[1].y;	m[1][2] = eigen_vectors[2].y;
	m[2][0] = eigen_vectors[0].z;	m[2][1] = eigen_vectors[1].z;	m[2][2] = eigen_vectors[2].z;

	D3DXMATRIX translate;
	D3DXMatrixTranslation(&translate, 1, 1, 1);
	*/

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

	//float angleXY_Y = (float) atan2(eigen_vectors[0].x, eigen_vectors[0].y);
	//float angleZX_X = (float) atan2(eigen_vectors[0].z, eigen_vectors[0].x);

	D3DXVECTOR3 vs(0, 0, 0);
	D3DXVECTOR3 vx((float) evecs[0][0], (float) evecs[0][1], (float) evecs[0][2]);
	D3DXVECTOR3 vy((float) evecs[1][0], (float) evecs[1][1], (float) evecs[1][2]);
	D3DXVECTOR3 vz((float) evecs[2][0], (float) evecs[2][1], (float) evecs[2][2]);

	vx = vx * 1000;
	vy = vy * 1000;
	vz = vz * 1000;

/*
	D3DXVECTOR3 vs(0, 0, 0);
	D3DXVECTOR3 vx(100, 0, 0);
	D3DXVECTOR3 vy(0, 100, 0);
	D3DXVECTOR3 vz(0, 0, 100);
	*/

	_vertices.push_back(SimpleVertex(vs, D3DCOLOR_XRGB(255, 200, 200)));
	_vertices.push_back(SimpleVertex(vx, D3DCOLOR_XRGB(255, 200, 200)));

	_vertices.push_back(SimpleVertex(vs, D3DCOLOR_XRGB(200, 255, 200)));
	_vertices.push_back(SimpleVertex(vy, D3DCOLOR_XRGB(200, 255, 200)));

	_vertices.push_back(SimpleVertex(vs, D3DCOLOR_XRGB(200, 200, 255)));
	_vertices.push_back(SimpleVertex(vz, D3DCOLOR_XRGB(200, 200, 255)));

	std::for_each(_vertices.begin(), _vertices.end(), SimpleVertex::transform_t(eigen));

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
