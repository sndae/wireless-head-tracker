#include "stdafx.h"
#pragma hdrstop

#include "resource.h"

#include "../dongle/reports.h"

#include "my_utils.h"
#include "my_win.h"
#include "d3d.h"
#include "d3d_objects.h"
#include "wht_dongle.h"
#include "mag_calib_dialog.h"

#include "EllipsoidFit.h"

MagCalibDialog::MagCalibDialog(WHTDongle& dngl)
:	_icon_large(IDI_MAGNET, true),
	_icon_small(IDI_MAGNET, false),
	_camera(_d3d_device),
	_is_dragging(false),
	_dongle(dngl),
	_is_ellipsoid_fit_valid(false)
{}

void MagCalibDialog::OnInit()
{
	SetIcon(_icon_small);
	SetIcon(_icon_large);

	_btn_clear_points.SetHandle(GetCtrl(IDC_BTN_CLEAR_POINTS));
	_btn_reset_camera.SetHandle(GetCtrl(IDC_BTN_RESET_CAMERA));

	_lbl_num_points.SetHandle(GetCtrl(IDC_LBL_NUM_POINTS));

	_d3d_window.SetHandle(GetCtrl(IDC_D3D));
	
	UpdateD3DSize();

	// init the Direct3D device
	_d3d_device.Init(_d3d, _d3d_window);
	Init3D();

	::SetTimer(GetHandle(), 1, 100, NULL);
}

void MagCalibDialog::OnException(const std::wstring& str)
{
	MsgBox(str, L"Exception", MB_OK | MB_ICONERROR);
}

void MagCalibDialog::OnDestroy()
{
	_hWnd = 0;

	ClearSamples();
}

void MagCalibDialog::ClearSamples()
{
	_mag_set.clear();

	// clear the vertex buffers
	_vb_mag_points[0].clear();
	_vb_mag_points[1].clear();

	// and default empty vertex buffers
	_vb_mag_points[0].push_back(VertexBuffer());
	_vb_mag_points[0].back().Alloc(_d3d_device, NUM_VERTICES_PER_VBUFFER);

	_vb_mag_points[1].push_back(VertexBuffer());
	_vb_mag_points[1].back().Alloc(_d3d_device, NUM_VERTICES_PER_VBUFFER);

	_is_ellipsoid_fit_valid = false;
}

void MagCalibDialog::OnSize(int width, int height, WPARAM wParam)
{
	WaitCursor cwc;

	UpdateD3DSize();

	// re-init Direct3D
	_vb_mag_points[0].clear();
	_vb_mag_points[1].clear();

	_vb_coord_sys.Release();
	_vb_ellipsoid_axes.Release();

	_d3d_device.Release();
	_d3d_device.Init(_d3d, _d3d_window);

	Init3D();
}

void MagCalibDialog::UpdateD3DSize()
{
	RECT dlgrect;
	int dlgw, dlgh, dxw, dxh;
	
	GetRect(dlgrect);
	dlgw = dlgrect.right - dlgrect.left;
	dlgh = dlgrect.bottom - dlgrect.top;

	dxw = dlgw - 30;
	dxh = dlgh - 87;

	// set the D3D window size
	::SetWindowPos(_d3d_window.GetHandle(), HWND_TOP, 0, 0, dxw, dxh, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
}

void MagCalibDialog::UnlockVertexBuffers()
{
	if (_vb_mag_points[0].back().IsLocked())
		_vb_mag_points[0].back().Unlock();

	if (_vb_mag_points[1].back().IsLocked())
		_vb_mag_points[1].back().Unlock();

	if (_vb_coord_sys.IsLocked())
		_vb_coord_sys.Unlock();

	if (_vb_ellipsoid_axes.IsLocked())
		_vb_ellipsoid_axes.Unlock();
}

void MagCalibDialog::Render()
{
    // return if the device is not ready
    if (!_d3d_device.IsValid())
		return;

	// unlock all the locked vertex buffers
	UnlockVertexBuffers();

    // Clear the back buffer
	_d3d_device.Clear();

	_d3d_device.BeginScene();

	// should we use light?
	//if (UseLighting())
	//	_d3d_device.EnableLight();
	//else
	//	_d3d_device.DisableLight();

	// set the current view transformation
	_camera.RefreshPos();

	// render our line based objects (the coordinate axes and the ellipsoid axes)
	_d3d_device.DrawVertices(_vb_coord_sys, D3DPT_LINELIST);

	if (_is_ellipsoid_fit_valid)
		_d3d_device.DrawVertices(_vb_ellipsoid_axes, D3DPT_LINELIST);

	// render our triangle based objects

	// the raw mag
	std::for_each(_vb_mag_points[0].begin(), _vb_mag_points[0].end(), [&] (VertexBuffer& vb) { _d3d_device.DrawVertices(vb, D3DPT_TRIANGLELIST); } );

	// the calibrated mag
	std::for_each(_vb_mag_points[1].begin(), _vb_mag_points[1].end(), [&] (VertexBuffer& vb) { _d3d_device.DrawVertices(vb, D3DPT_TRIANGLELIST); } );

	_d3d_device.EndScene();

    // present the back buffer to the display adapter to be drawn
    _d3d_device.Present();
}

void MagCalibDialog::Init3D()
{
	// Create a matrix to store our Projection transform. Null all the fields.
	D3DXMATRIX matProjection;
	ZeroMemory(&matProjection, sizeof(matProjection));

	RECT r;
	GetRect(r);

	// Use D3DX to create a left handed cartesian Field Of View transform
	D3DXMatrixPerspectiveFovLH(	&matProjection,
								D3DX_PI / 4,
								(float) (r.right - r.left) / (r.bottom - r.top),
								1.0,
								100000.0);
	
	// Tell D3D to use our Projection matrix for the projection transformation stage
	_d3d_device.SetProjectionTransform(matProjection);

	_d3d_device.SetCulling(D3DCULL_NONE);
	_d3d_device.DisableLight();

	// and 3D points for the cubes
	_vb_mag_points[0].push_back(VertexBuffer());
	_vb_mag_points[0].back().Alloc(_d3d_device, NUM_VERTICES_PER_VBUFFER);

	_vb_mag_points[1].push_back(VertexBuffer());
	_vb_mag_points[1].back().Alloc(_d3d_device, NUM_VERTICES_PER_VBUFFER);

	// re-create the raw mag points
	std::for_each(_mag_set.begin(), _mag_set.end(), [&] (const Point<int16_t>& p) { AddPoint(p, true); } );

	// and the calibrated points too
	if (_is_ellipsoid_fit_valid)
		std::for_each(_mag_set.begin(), _mag_set.end(), [&] (const Point<int16_t>& p) { AddPoint(p, false); } );

	// add the coordinate system to the vertex buffer
	_coord_sys.Build();
	_vb_coord_sys.Alloc(_d3d_device, 32);
	_vb_coord_sys.Lock();
	_vb_coord_sys.AddObject(_coord_sys);
	_vb_coord_sys.Unlock();

	// create the vertex buffers ellipsoid axes
	_vb_ellipsoid_axes.Alloc(_d3d_device, 32);
	if (_is_ellipsoid_fit_valid)
	{
		_vb_ellipsoid_axes.Lock();
		_vb_ellipsoid_axes.AddObject(_ellipsoid_axes);
		_vb_ellipsoid_axes.Unlock();
	}
}

void MagCalibDialog::OnTimer(int timerID)
{
	// update the counters
	_lbl_num_points.SetText((int) _mag_set.size());

	if (!_dongle.IsOpen())
		return;

	// get raw the mag data
	FeatRep_RawMagSamples repMagSamples;
	repMagSamples.report_id = RAW_MAG_REPORT_ID;
	_dongle.GetFeatureReport(repMagSamples);

	for (int i = 0; i < repMagSamples.num_samples; ++i)
	{
		Point<int16_t> mps(repMagSamples.mag[i].x, repMagSamples.mag[i].y, repMagSamples.mag[i].z);

		if (_mag_set.find(mps) == _mag_set.end())
		{
			_mag_set.insert(mps);

			AddPoint(mps, true);
			if (_is_ellipsoid_fit_valid)
				AddPoint(mps, false);
		}
	}
}

void MagCalibDialog::OnControl(int ctrlID, int notifyID, HWND hWndCtrl)
{
	if (ctrlID == IDC_BTN_CLEAR_POINTS)
		ClearSamples();
	else if (ctrlID == IDC_BTN_RESET_CAMERA)
		_camera.Reset();
	else if (ctrlID == IDC_BTN_SAVE)
		SaveData();
	else if (ctrlID == IDC_BTN_LOAD)
		LoadData();
	else if (ctrlID == IDC_BTN_CALC)
		CalcEllipsoidFit();
}

void MagCalibDialog::OnLButtonDown(int x, int y, WPARAM wParam)
{
	// TODO: proper mouse capture
	_is_dragging = true;
	_last_x = x;
	_last_y = y;
}

void MagCalibDialog::OnLButtonUp(int x, int y, WPARAM wParam)
{
	_is_dragging = false;
}

void MagCalibDialog::OnMouseMove(int x, int y, WPARAM wParam)
{
	if (_is_dragging)
	{
		_camera.SetRotation(float(x - _last_x), float(y - _last_y));

		_last_x = x;
		_last_y = y;
	}
}

void MagCalibDialog::OnMouseWheel(int x, int y, int delta, WPARAM wParam)
{
	_camera.Zoom(delta);
}

void MagCalibDialog::SaveData()
{
	OpenSaveFileDialog saveFile;
	saveFile.AddFilter(L"CSV file (*.csv)", L"*.csv");
	saveFile.AddFilter(L"All files (*.*)", L"*.*");
	saveFile.SetDefaultFileName(L"magdata.csv");

	if (saveFile.GetSaveFile(L"Save magnetometer samples", *this))
	{
		SimpleFile f;

		if (f.Open(saveFile.GetFullFileName(), true, true))
		{
			char line[128];
			for (std::set<Point<int16_t>>::iterator mi(_mag_set.begin()); mi != _mag_set.end(); ++mi)
			{
				sprintf_s(line, sizeof(line), "%i,%i,%i\n", mi->x, mi->y, mi->z);
				f.Write(line, strlen(line));
			}

		} else {
			MsgBox(L"Unable to open file " + saveFile.GetFullFileName(), L"Error", MB_OK | MB_ICONERROR);
		}
	}
}

// splits a string using the provided delimiter
void split_record(const std::string& in_str, std::vector<std::string>& out_vector, const char delim)
{
	out_vector.clear();

	std::string::const_iterator riter(in_str.begin());
	std::string field;
	while (riter != in_str.end())
	{
		if (*riter == delim)
		{
			out_vector.push_back(field);
			field.clear();
		} else {
			field += *riter;
		}

		++riter;
	}

	out_vector.push_back(field);
}

void MagCalibDialog::AddPoint(const Point<int16_t>& p, bool is_raw)
{
	const size_t ndx = is_raw ? 0 : 1;

	// enough space in the last vertex buffer?
	if (_vb_mag_points[ndx].back().Capacity() - _vb_mag_points[ndx].back().Size() < NUM_VERTICES_PER_CUBE)
	{
		// unlock the last buffer and make a new one
		if (_vb_mag_points[ndx].back().IsLocked())
			_vb_mag_points[ndx].back().Unlock();

		_vb_mag_points[ndx].push_back(VertexBuffer());
		_vb_mag_points[ndx].back().Alloc(_d3d_device, NUM_VERTICES_PER_VBUFFER);
	}

	if (!_vb_mag_points[ndx].back().IsLocked())
		_vb_mag_points[ndx].back().Lock();

	MagPoint mp;
	if (is_raw)
		mp.Build(p);
	else
		mp.BuildCalibrated(p, _ellipsoid_fit.center, _ellipsoid_fit.calibMatrix);

	_vb_mag_points[ndx].back().AddObject(mp);
}

void MagCalibDialog::LoadData()
{
/*
#if _DEBUG
	std::wstring fname(L"C:\\my_opensource\\wht_adding9150\\src\\WHTConfig\\samples\\tracker2.csv");
	{
		WaitCursor wc;
		SimpleFile f;
		if (f.Open(fname, false))
#else
*/
	OpenSaveFileDialog openFile;
	openFile.AddFilter(L"CSV file (*.csv)", L"*.csv");
	openFile.AddFilter(L"All files (*.*)", L"*.*");

	if (openFile.GetOpenFile(L"Load magnetometer samples", *this))
	{
		WaitCursor wc;

		SimpleFile f;

		if (f.Open(openFile.GetFullFileName(), false))
//#endif
		{
			// ellipsoid fit becomes invalid
			_is_ellipsoid_fit_valid = false;

			// read the entire file into a string (yeah, nasty, i know...)
			const int BUFF_SIZE = 1000;
			char buff[BUFF_SIZE];
			DWORD bytes_read;
			std::string file_str;
			do {
				bytes_read = f.Read(buff, BUFF_SIZE - 1);
				buff[bytes_read] = '\0';
				file_str += buff;
			} while (bytes_read == BUFF_SIZE - 1);

			ClearSamples();

			// parse and handle the lines, make points
			std::vector<std::string> lines, record;
			split_record(file_str, lines, '\n');

			for (std::vector<std::string>::iterator li(lines.begin()); li != lines.end(); ++li)
			{
				split_record(*li, record, ',');
				if (record.size() == 3)
				{
					Point<int16_t> mps(atoi(record[0].c_str()), atoi(record[1].c_str()), atoi(record[2].c_str()));

					if (_mag_set.find(mps) == _mag_set.end())
					{
						_mag_set.insert(mps);
						AddPoint(mps, true);
					}
				}
			}
		}
	}
}

void MagCalibDialog::CalcEllipsoidFit()
{
	if (_mag_set.size() < 500)
	{
		MsgBox(L"Please record more points.", L"Error", MB_OK | MB_ICONERROR);
		return;
	}

	WaitCursor wc;

	// try to make an ellipsoid out of the points
	_ellipsoid_fit.fitEllipsoid(_mag_set);
	_ellipsoid_fit.calcCalibMatrix(MagPoint::CALIBRATED_SCALE);

	debug(L"-----------------------");

	debug(int2str(int(_ellipsoid_fit.center.x)) + L"," + int2str(int(_ellipsoid_fit.center.y)) + L"," + int2str(int(_ellipsoid_fit.center.z)));

	// scale the matrix for fixed point calc
	std::wstring line;
	for (int i = 0; i < 3; ++i)
	{
		line.clear();

		for (int j = 0; j < 3; ++j)
		{
			if (j)
				line += L",";

			line += int2str(int(_ellipsoid_fit.calibMatrix[i][j] * (1 << MAG_MATRIX_SCALE_BITS)));
		}

		debug(line);
	}

	// draw the ellipsoid axes
	_ellipsoid_axes.Build(_ellipsoid_fit.center, _ellipsoid_fit.radii, _ellipsoid_fit.evecs);

	_vb_ellipsoid_axes.Lock();
	_vb_ellipsoid_axes.Clear();
	_vb_ellipsoid_axes.AddObject(_ellipsoid_axes);
	_vb_ellipsoid_axes.Unlock();

	// clear the calibrated vertex buffer
	_vb_mag_points[1].clear();
	_vb_mag_points[1].push_back(VertexBuffer());
	_vb_mag_points[1].back().Alloc(_d3d_device, NUM_VERTICES_PER_VBUFFER);

	// re-draw the calibrated points
	for (std::set<Point<int16_t>>::const_iterator piter(_mag_set.begin()); piter != _mag_set.end(); ++piter)
		AddPoint(*piter, false);

	_is_ellipsoid_fit_valid = true;
}
