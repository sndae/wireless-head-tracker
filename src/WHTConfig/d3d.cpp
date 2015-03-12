#include "stdafx.h"
#pragma hdrstop

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#include "my_utils.h"
#include "my_win.h"
#include "d3d.h"
#include "d3d_objects.h"

// cross product of two vectors
D3DXVECTOR3 operator * (const D3DXVECTOR3& v1, const D3DXVECTOR3& v2)
{
	D3DXVECTOR3 ret_val;
	D3DXVec3Cross(&ret_val, &v1, &v2);
	return ret_val;
}

#define DEFINE_HERROR(err)		case err:	srslt = L#err; break;

void ThrowD3DExceptionFileLine(HRESULT rslt, const std::wstring& m, const wchar_t* file, const int line_no)
{
	const wchar_t* srslt = L"<unknown>";

	switch (rslt)
	{
	DEFINE_HERROR(D3DERR_WRONGTEXTUREFORMAT)
	DEFINE_HERROR(D3DERR_UNSUPPORTEDCOLOROPERATION)
	DEFINE_HERROR(D3DERR_UNSUPPORTEDCOLORARG)
	DEFINE_HERROR(D3DERR_UNSUPPORTEDALPHAOPERATION)
	DEFINE_HERROR(D3DERR_UNSUPPORTEDALPHAARG)
	DEFINE_HERROR(D3DERR_TOOMANYOPERATIONS)
	DEFINE_HERROR(D3DERR_CONFLICTINGTEXTUREFILTER)
	DEFINE_HERROR(D3DERR_UNSUPPORTEDFACTORVALUE)
	DEFINE_HERROR(D3DERR_CONFLICTINGRENDERSTATE)
	DEFINE_HERROR(D3DERR_UNSUPPORTEDTEXTUREFILTER)
	DEFINE_HERROR(D3DERR_CONFLICTINGTEXTUREPALETTE)
	DEFINE_HERROR(D3DERR_DRIVERINTERNALERROR)

	DEFINE_HERROR(D3DERR_NOTFOUND)
	DEFINE_HERROR(D3DERR_MOREDATA)
	DEFINE_HERROR(D3DERR_DEVICELOST)
	DEFINE_HERROR(D3DERR_DEVICENOTRESET)
	DEFINE_HERROR(D3DERR_NOTAVAILABLE)
	DEFINE_HERROR(D3DERR_OUTOFVIDEOMEMORY)
	DEFINE_HERROR(D3DERR_INVALIDDEVICE)
	DEFINE_HERROR(D3DERR_INVALIDCALL)
	DEFINE_HERROR(D3DERR_DRIVERINVALIDCALL)
	DEFINE_HERROR(D3DERR_WASSTILLDRAWING)
	DEFINE_HERROR(D3DOK_NOAUTOGEN)
	}

	debug(std::wstring(file) + L"(" + int2str(line_no) + L") : exception: " + m + L" rslt=" + srslt + L"\n");

	throw std::wstring(m + L"\n" + srslt);
}

Direct3D::Direct3D()
{
	// create the direct3D object
	_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (_pD3D == 0)
		ThrowD3DException(0, L"Direct3DCreate9() returned NULL");
}

std::string Direct3D::GetAdapterName() const
{
	HRESULT rslt;
	D3DADAPTER_IDENTIFIER9 identifier;

	rslt = _pD3D->GetAdapterIdentifier(0, 0, &identifier);
	if (FAILED(rslt))
		ThrowD3DException(rslt, L"GetAdapterIdentifier() failed!");

	/*
	// enumerate the adapters
	UINT adapter_cnt = _pD3D->GetAdapterCount();
	for (UINT c = 0; c < adapter_cnt; c++)
	{
		rslt = _pD3D->GetAdapterIdentifier(c, 0, &identifier);
		if (FAILED(rslt))
			ThrowD3DException(rslt, L"GetAdapterIdentifier() failed!!!\n");
	}
	*/

	return identifier.Description;
}

void Direct3D::GetAdapterDisplayMode(D3DDISPLAYMODE& d3ddm)
{
	HRESULT rslt = _pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm);
	if (FAILED(rslt))
		ThrowD3DException(rslt, L"GetAdapterDisplayMode() failed!");
}

bool Direct3D::CheckDeviceMultiSampleType(D3DFORMAT format, D3DMULTISAMPLE_TYPE multisample_type)
{
	return SUCCEEDED(_pD3D->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, format, TRUE, multisample_type, 0));
}

IDirect3DDevice9* Direct3D::CreateDevice(const Window& win, D3DPRESENT_PARAMETERS& d3d_pp)
{
	IDirect3DDevice9* ret_val;
	HRESULT rslt = _pD3D->CreateDevice(	D3DADAPTER_DEFAULT,
										D3DDEVTYPE_HAL,
										win.GetHandle(),
										D3DCREATE_HARDWARE_VERTEXPROCESSING,
										&d3d_pp,
										&ret_val);

	if (FAILED(rslt))
		ThrowD3DException(rslt, L"CreateDevice() failed!!!\n");

	return ret_val;
}

void DeviceD3D::Init(Direct3D& d3d, const Window& win)
{
	// PresentParams struct to hold info about the rendering method
	::ZeroMemory(&_d3d_pp, sizeof(_d3d_pp));

	// Get the settings for the current display mode. This gives us hints on how to setup our
	// Present Parameters struct
	D3DDISPLAYMODE d3ddm;
	d3d.GetAdapterDisplayMode(d3ddm);

	// the width & height of the back buffer in pixels
	RECT r;
	::GetWindowRect(win.GetHandle(), &r);
	_d3d_pp.BackBufferWidth = r.right - r.left;
	_d3d_pp.BackBufferHeight = r.bottom - r.top;
	
	// the format of the backbuffer is the same as our current desktop
	_d3d_pp.BackBufferFormat = d3ddm.Format;

	// handle to render target window
	_d3d_pp.Windowed = TRUE;
	_d3d_pp.hDeviceWindow = win.GetHandle();
	
	// number of back buffers
	_d3d_pp.BackBufferCount = 1;

	// swap method
	_d3d_pp.SwapEffect = D3DSWAPEFFECT_DISCARD;

	// try the best type of multisampling (anti-aliasing)
	if (d3d.CheckDeviceMultiSampleType(d3ddm.Format, D3DMULTISAMPLE_16_SAMPLES))
		_d3d_pp.MultiSampleType = D3DMULTISAMPLE_16_SAMPLES;
	else if (d3d.CheckDeviceMultiSampleType(d3ddm.Format, D3DMULTISAMPLE_8_SAMPLES))
		_d3d_pp.MultiSampleType = D3DMULTISAMPLE_8_SAMPLES;
	else if ((d3d.CheckDeviceMultiSampleType(d3ddm.Format, D3DMULTISAMPLE_4_SAMPLES)))
		_d3d_pp.MultiSampleType = D3DMULTISAMPLE_4_SAMPLES;
	else if ((d3d.CheckDeviceMultiSampleType(d3ddm.Format, D3DMULTISAMPLE_2_SAMPLES)))
		_d3d_pp.MultiSampleType = D3DMULTISAMPLE_2_SAMPLES;
	else
		_d3d_pp.MultiSampleType = D3DMULTISAMPLE_NONE;

	// Let D3D manage the depth buffer
	_d3d_pp.EnableAutoDepthStencil = TRUE;

	// Set the depth buffer format to 16bits
	_d3d_pp.AutoDepthStencilFormat = D3DFMT_D16;
	
	// Use default refresh rate
	_d3d_pp.FullScreen_RefreshRateInHz = 0;

	// present the information as fast as possible.
	_d3d_pp.PresentationInterval = 0;

	// Get a pointer to the IDirect3DDevice9 interface
	_pDevice = d3d.CreateDevice(win, _d3d_pp);

	// set the vector format
	HRESULT rslt = _pDevice->SetFVF(SimpleVertex::fvf_id);
	if (FAILED(rslt))		ThrowD3DException(rslt, L"SetFVF() failed");

	// default
	_lighting_enabled = false;
	_pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
}

void DeviceD3D::EnableLight()
{
	if (!_lighting_enabled)
	{
		HRESULT rslt;

		rslt = _pDevice->SetRenderState(D3DRS_LIGHTING, true);
		if (FAILED(rslt))		ThrowD3DException(rslt, L"dev->SetRenderState(D3DRS_LIGHTING, true) failed");

		D3DLIGHT9 light;
		ZeroMemory(&light, sizeof(light));

		light.Type = D3DLIGHT_DIRECTIONAL;
		light.Diffuse.r = 155;
		light.Diffuse.g = 155;
		light.Diffuse.b = 155;
		light.Direction.x = -1.0f;
		light.Direction.y = -1.0f;
		light.Direction.z = 0.0f;

		rslt = _pDevice->SetLight(0, &light);	// set the light
		if (FAILED(rslt))		ThrowD3DException(rslt, L"dev->SetLight(0, &light) failed");

		rslt = _pDevice->LightEnable(0, true);	// enables the light
		if (FAILED(rslt))		ThrowD3DException(rslt, L"dev->LightEnable(0, true) failed");
	}

	_lighting_enabled = true;
}

void DeviceD3D::DisableLight()
{
	if (_lighting_enabled)
		_pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

	_lighting_enabled = false;
}

void DeviceD3D::SetView(const D3DXVECTOR3& camera_pos, const D3DXVECTOR3& look_at, const D3DXVECTOR3& up)
{
	// create a matrix to store our view transform
	D3DXMATRIX matView;
	ZeroMemory(&matView, sizeof(matView));

	// Use D3DX to create a Look At matrix from eye, lookat and up vectors.
	D3DXMatrixLookAtLH(&matView, &camera_pos, &look_at, &up);

	// Tell D3D to use our View matrix for the view transformation stage
	HRESULT rslt = _pDevice->SetTransform(D3DTS_VIEW, &matView);
	if (FAILED(rslt))		ThrowD3DException(rslt, L"Failed to set View Transform.");
}

DeviceD3D::~DeviceD3D()
{
	Release();
}

bool DeviceD3D::IsValid()
{
	// Test the current state of the device
	HRESULT rslt = _pDevice->TestCooperativeLevel();
	if (FAILED(rslt))
	{
		// if the device is lost, then return a false
		if (rslt == D3DERR_DEVICELOST)
			return false;

		// if the device is ready to be reset, then try it
		if (rslt == D3DERR_DEVICENOTRESET)
		{
			// reset the device
			rslt = _pDevice->Reset(&_d3d_pp);
			if (FAILED(rslt))		ThrowD3DException(rslt, L"Unable to reset device");
		}
	}

	return true;
}

void DeviceD3D::Clear()
{
	HRESULT rslt = _pDevice->Clear(0,0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0,0,55), 1.0f, 0);
	if (FAILED(rslt))		ThrowD3DException(rslt, L"dev->Clear() failed");
}

void DeviceD3D::BeginScene()
{
	HRESULT rslt = _pDevice->BeginScene();
	if (FAILED(rslt))		ThrowD3DException(rslt, L"BeginScene() failed");
}

void DeviceD3D::EndScene()
{
	HRESULT rslt = _pDevice->EndScene();
	if (FAILED(rslt))		ThrowD3DException(rslt, L"EndScene() failed");
}

void DeviceD3D::Present()
{
    HRESULT rslt = _pDevice->Present(NULL, NULL, NULL, NULL);
	if (FAILED(rslt))		ThrowD3DException(rslt, L"Present() failed");
}

void DeviceD3D::SetProjectionTransform(D3DXMATRIX &matProjection)
{
	HRESULT rslt = _pDevice->SetTransform(D3DTS_PROJECTION, &matProjection);
	if (FAILED(rslt))		ThrowD3DException(rslt, L"SetTransform(D3DTS_PROJECTION, ...) failed");
}

void DeviceD3D::SetCulling(const size_t cull_mode)
{
	HRESULT rslt = _pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	if (FAILED(rslt))		ThrowD3DException(rslt, L"SetRenderState(D3DRS_CULLMODE, ...) failed");
}

IDirect3DSurface9* DeviceD3D::GetBackBuffer()
{
	LPDIRECT3DSURFACE9 pbs;
	HRESULT rslt = _pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pbs);
	if (FAILED(rslt))
		ThrowD3DException(rslt, L"GetBackBuffer() failed !!!");

	return pbs;
}

void DeviceD3D::GetViewport(D3DVIEWPORT9& viewport)
{
	HRESULT rslt = _pDevice->GetViewport(&viewport);
	if (FAILED(rslt))		ThrowD3DException(rslt, L"GetViewport() failed");
}

void DeviceD3D::SetViewport(D3DVIEWPORT9& viewport)
{
	HRESULT rslt = _pDevice->SetViewport(&viewport);
	if (FAILED(rslt))		ThrowD3DException(rslt, L"SetViewport() failed");
}

IDirect3DVertexBuffer9* DeviceD3D::CreateVertexBuffer(const int vcount)
{
	IDirect3DVertexBuffer9* ret_val;

	if (_pDevice == 0)
		ThrowD3DException(0, L"Device not initilized while creating VertexBuffer");

	HRESULT rslt = _pDevice->CreateVertexBuffer(	vcount * sizeof(SimpleVertex),
													D3DUSAGE_WRITEONLY,
													SimpleVertex::fvf_id,
													D3DPOOL_MANAGED,
													&ret_val,
													0);

	if (FAILED(rslt))
		ThrowD3DException(rslt, L"CreateVertexBuffer() failed");

	return ret_val;
}

void Object3D::Render(DeviceD3D& dev)
{
	HRESULT rslt;

	if (!_vertices.empty())
	{
		if (_vertex_buffer.IsEmpty())
			MakeVertexBuffer(dev);

		rslt = dev._pDevice->SetStreamSource(0, _vertex_buffer._pvb, 0, sizeof(SimpleVertex));
		if (FAILED(rslt))		ThrowD3DException(rslt, L"SetStreamSource() failed");

		rslt = dev._pDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, _vertices.size() / 3);
		if (FAILED(rslt))		ThrowD3DException(rslt, L"DrawPrimitive() failed");
	}
}

void Object3D::MakeVertexBuffer(DeviceD3D& dev)
{
	// fill the normals
	std::vector<SimpleVertex>::iterator first = _vertices.begin();
	std::vector<SimpleVertex>::iterator last = _vertices.end();

	assert((_vertices.size() % 3) == 0);

	while (first != last)
	{
		D3DXVECTOR3 normal((first[2].pos - first[1].pos) * (first[0].pos - first[1].pos));
		first[0].normal = first[1].normal = first[2].normal = normal;

		first += 3;
	}
	
	// alloc the vertex buffer
	_vertex_buffer.Alloc(dev, _vertices.size());

	// copy
	memcpy(_vertex_buffer.Lock(), &_vertices.front(), sizeof(SimpleVertex) * _vertices.size());

	// give the block back to D3D
	_vertex_buffer.Unlock();
}

VertexBuffer::VertexBuffer()
:	_pvb(0),
	_vertex_count(0)
{}

VertexBuffer::VertexBuffer(const VertexBuffer& c)
:	_pvb(c._pvb),
	_vertex_count(c._vertex_count)
{
	if (_pvb)
		_pvb->AddRef();
}

void VertexBuffer::Alloc(DeviceD3D& dev, const int vcount)
{
	assert(vcount != 0);

	_vertex_count = vcount;

	// release the buffer if we already have one
	if (_pvb)
	{
		int ref_cnt = _pvb->Release();
		assert(ref_cnt == 0);
	}

	_pvb = dev.CreateVertexBuffer(vcount);
}

void VertexBuffer::Release()
{
	if (_pvb)
		_pvb->Release();

	_pvb = 0;
}

char* VertexBuffer::Lock()
{
	LPVOID ret_val;
	HRESULT rslt = _pvb->Lock(0, _vertex_count * sizeof(SimpleVertex), &ret_val, 0);
	if (FAILED(rslt))		ThrowD3DException(rslt, L"dev->Lock() failed");
		
	return (char*) ret_val;
}

void VertexBuffer::Unlock()
{
	HRESULT rslt = _pvb->Unlock();

	if (FAILED(rslt))		ThrowD3DException(rslt, L"dev->CreateVertexBuffer() failed");
}

Camera::Camera(DeviceD3D& d)
:	_dev(d)
{
	Reset();
}

void Camera::Reset()
{
	_rotX = 45;
	_rotY = 135;
	_scale = 1;

	CalcCamera();
}

void Camera::RefreshPos()
{
	D3DXVECTOR3 look_at(0, 0, 0);
	_dev.SetView(_camera_pos, look_at, _up);
}

void Camera::CalcCamera()
{
	D3DXVECTOR3 cpos(0, 0, -550);

	D3DXMATRIX mroty, mrotx, mscale, mfinal;
	D3DXMatrixRotationY(&mroty, float(_rotY/180*3.1416926));
	D3DXMatrixRotationX(&mrotx, float(_rotX/180*3.1416926));
	D3DXMatrixScaling(&mscale, _scale, _scale, _scale);

	mfinal = mrotx * mroty * mscale;

	D3DXVECTOR4	out;
	D3DXVec3Transform(&out, &cpos, &mfinal);

	_camera_pos.x = out.x;
	_camera_pos.y = out.y;
	_camera_pos.z = out.z;

	D3DXVECTOR3 up(0,1,0);
	mfinal = mrotx * mroty;
	D3DXVec3Transform(&out, &up, &mfinal);

	_up.x = out.x;
	_up.y = out.y;
	_up.z = out.z;
}

void Camera::SetRotation(float deltaY, float deltaX)
{
	_rotY += deltaY / 3;
	_rotX += deltaX / 3;

	CalcCamera();
}

void Camera::Zoom(const int zoom)
{
	_scale *= float(zoom > 0 ? 0.9 : 1.1);

	CalcCamera();
}

void BuildCube(std::vector<SimpleVertex>& v, float Width, float Height, float Depth, float x, float y, float z)
{
	D3DVECTOR p[8] = {	{ x,       y+Height,        z},
						{ x+Width, y+Height,        z},
						{ x,              y,        z},
						{ x+Width,        y,        z},
						{ x,       y+Height,  z+Depth},
						{ x+Width, y+Height,  z+Depth},
						{ x,              y,  z+Depth},
						{ x+Width,        y,  z+Depth}};

	//v.reserve(v.size() + 36);
	v.insert(v.end(), 36, SimpleVertex());

	std::vector<SimpleVertex>::iterator vp(v.end() - 36);

	// front
	vp[ 0].pos=p[0];		vp[ 1].pos=p[1];		vp[ 2].pos=p[2];
	vp[ 3].pos=p[2];		vp[ 4].pos=p[1];		vp[ 5].pos=p[3];
	// back
	vp[ 6].pos=p[5];		vp[ 7].pos=p[4];		vp[ 8].pos=p[7];
	vp[ 9].pos=p[7];		vp[10].pos=p[4];		vp[11].pos=p[6];
	// top
	vp[12].pos=p[4];		vp[13].pos=p[5];		vp[14].pos=p[0];
	vp[15].pos=p[0];		vp[16].pos=p[5];		vp[17].pos=p[1];
	// bottom
	vp[18].pos=p[2];		vp[19].pos=p[3];		vp[20].pos=p[6];
	vp[21].pos=p[6];		vp[22].pos=p[3];		vp[23].pos=p[7];
	// left
	vp[24].pos=p[4];		vp[25].pos=p[0];		vp[26].pos=p[6];
	vp[27].pos=p[6];		vp[28].pos=p[0];		vp[29].pos=p[2];
	// right
	vp[30].pos=p[1];		vp[31].pos=p[5];		vp[32].pos=p[3];
	vp[33].pos=p[3];		vp[34].pos=p[5];		vp[35].pos=p[7];
}

void BuildCube(std::vector<SimpleVertex>& v, float Width, float Height, float Depth)
{
	float x = -Width/2;
	float y = -Height/2;
	float z = -Depth/2;

	BuildCube(v, Width, Height, Depth, x, y, z);
}
