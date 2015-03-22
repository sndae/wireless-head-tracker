#pragma once

#define ThrowD3DException(rslt, msg)		ThrowD3DExceptionFileLine(rslt, msg, WIDEN(__FILE__), __LINE__)

//
// the direct3D object
//

class Direct3D
{
private:
	IDirect3D9*		_pD3D;

public:
	Direct3D();

	~Direct3D()
	{
		// shutdown Direct3D
		_pD3D->Release();
	}

	std::string GetAdapterName() const;
	void GetAdapterDisplayMode(D3DDISPLAYMODE& d3ddm);
	bool CheckDeviceMultiSampleType(D3DFORMAT format, D3DMULTISAMPLE_TYPE multisample_type);
	IDirect3DDevice9* CreateDevice(const Window& wnd, D3DPRESENT_PARAMETERS& d3d_pp);
};

//
// device
//

class VertexBuffer;

class DeviceD3D
{
private:
	IDirect3DDevice9*		_pDevice;

	D3DPRESENT_PARAMETERS	_d3d_pp;
	
	bool					_lighting_enabled;

public:
	DeviceD3D()
		: _pDevice(0)
	{}

	~DeviceD3D();

	void Release()
	{
		if (_pDevice != 0)
			_pDevice->Release();

		_pDevice = 0;
	}
	
	void Init(Direct3D& d3d, const Window& d3d_win);
	void EnableLight();
	void DisableLight();
	void SetView(const D3DXVECTOR3& camera_pos, const D3DXVECTOR3& look_at, const D3DXVECTOR3& up);
	bool IsValid();
	void Clear();

	void BeginScene();
	void EndScene();
	void Present();

	void SetProjectionTransform(D3DXMATRIX &matProjection);
	void GetViewport(D3DVIEWPORT9& viewport);
	void SetViewport(D3DVIEWPORT9& viewport);

	void SetCulling(const size_t cull_mode);

	IDirect3DSurface9* GetBackBuffer();
	IDirect3DVertexBuffer9* CreateVertexBuffer(const int vcount);
	void DrawVertices(VertexBuffer& vbuff, D3DPRIMITIVETYPE primitive_type);
};


//
// the only vertex format we'll be using
//

struct SimpleVertex
{
	enum {fvf_id = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_NORMAL};

	D3DXVECTOR3	pos;
	D3DXVECTOR3	normal;
	D3DCOLOR	diffuse;

	SimpleVertex()
	{}

	SimpleVertex(float x, float y, float z, D3DCOLOR col)
	{
		pos.x = x;
		pos.y = y;
		pos.z = z;
		diffuse = col;
	}

	SimpleVertex(float x, float y, float z)
	{
		pos.x = x;
		pos.y = y;
		pos.z = z;
	}

	SimpleVertex(const D3DXVECTOR3& p, D3DCOLOR col)
	{
		pos = p;
		diffuse = col;
	}

	// used to transform the vertex with STL (for_each)
	struct transform_t
	{
		D3DXMATRIX		transform_matrix;
		D3DXVECTOR4		out;

		transform_t(const D3DXMATRIX tm) : transform_matrix(tm)		{}

		void operator () (SimpleVertex& v)
		{
			D3DXVec3Transform(&out, &v.pos, &transform_matrix);

			v.pos.x = out.x;
			v.pos.y = out.y;
			v.pos.z = out.z;
		}
	};
};

//
// base class for the objects we'll have in the scene
//

class Object3D
{
protected:
	// this holds the vertices for the object
	std::vector<SimpleVertex>	_vertices;

public:

	void clear()
	{
		_vertices.clear();
	}

	const std::vector<SimpleVertex>& GetVertices() const
	{
		return _vertices;
	}
};



//
// the vertex buffer -- every CD3DObject has one embedded
//

class VertexBuffer
{
	friend class DeviceD3D;
private:
	// interface pointer
	IDirect3DVertexBuffer9*		_pvb;

	// while the buffer is locked, this is the pointer to the first locked vertex
	SimpleVertex*				_pVertex;

	size_t	_vsize;
	size_t	_vcapacity;

public:
	VertexBuffer();
	VertexBuffer(const VertexBuffer& c);

	~VertexBuffer()
	{
		Release();
	}

	bool IsEmpty() const
	{
		return _pvb == 0  ||  _vsize == 0;
	}

	bool IsLocked() const
	{
		return _pVertex != 0;
	}

	void Clear()
	{
		_vsize = 0;
	}

	void Alloc(DeviceD3D& dev, const size_t vcount);
	void Lock();
	void Unlock();

	size_t Size() const			{ return _vsize; }
	size_t Capacity() const		{ return _vcapacity; }

	void Release();

	bool AddObject(const Object3D& obj);
};


//
// the camera class
//

class Camera
{
private:
	D3DXVECTOR3		_up;
	D3DXVECTOR3		_camera_pos;

	float			_rotY;		// the current camera rotation and scale
	float			_rotX;
	float			_scale;

	DeviceD3D&		_dev;

	void CalcCamera();

public:
	Camera(DeviceD3D& d);

	void Reset();

	// sets the view transformation on the device
	void RefreshPos();

	void SetRotation(float deltaY, float deltaX);
	void Zoom(const int zoom);
};

// builds a cube of given dimensions around (0,0,0)
void BuildCube(std::vector<SimpleVertex>& v, float Width, float Height, float Depth, D3DCOLOR col);

// builds a cube of given dimensions at given coordinates
void BuildCube(std::vector<SimpleVertex>& v, float Width, float Height, float Depth, float x, float y, float z, D3DCOLOR col);

// returns the angle between 2 NORMALIZED vectors
inline float GetAngle(const D3DXVECTOR3& v1, const D3DXVECTOR3& v2)
{
	return acos(D3DXVec3Dot(&v1, &v2));
}
