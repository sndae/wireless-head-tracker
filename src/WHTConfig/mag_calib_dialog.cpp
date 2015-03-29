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
	_dongle(dngl)
{}

void MagCalibDialog::OnInit()
{
	SetIcon(_icon_small);
	SetIcon(_icon_large);

	_is_dragging = false;
	_is_collecting_raw_samples = false;
	_is_ellipsoid_fit_valid = false;

	_lbl_num_points.SetHandle(GetCtrl(IDC_LBL_NUM_POINTS));
	_btn_load_raw_points.SetHandle(GetCtrl(IDC_BTN_LOAD_RAW_POINTS));
	_btn_save_raw_points.SetHandle(GetCtrl(IDC_BTN_SAVE_RAW_POINTS));
	_btn_start_sampling.SetHandle(GetCtrl(IDC_BTN_START_SAMPLING));
	_btn_stop_sampling.SetHandle(GetCtrl(IDC_BTN_STOP_SAMPLING));
	_btn_clear_all_points.SetHandle(GetCtrl(IDC_BTN_CLEAR_ALL_POINTS));
	_btn_clear_calibrated_points.SetHandle(GetCtrl(IDC_BTN_CLEAR_CALIBRATED_POINTS));
	_btn_calc_calibration.SetHandle(GetCtrl(IDC_BTN_CALC_CALIBRATION));
	_btn_save_calibration.SetHandle(GetCtrl(IDC_BTN_SAVE_CALIBRATION));

	_d3d_window.SetHandle(GetCtrl(IDC_D3D));

	RefreshControls();
	
	// set the device adapter name in the title
	SetText("Magnetometer calibration on " + _d3d.GetAdapterName());

	// remember the size of the D3D window relative to the size of the window
	RECT dlgrect, dxrect;
	GetRect(dlgrect);
	_d3d_window.GetRect(dxrect);

	_d3d_width_diff = dlgrect.right - dlgrect.left - dxrect.right + dxrect.left;
	_d3d_height_diff = dlgrect.bottom - dlgrect.top - dxrect.bottom + dxrect.top;

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

	// init d3d if we're not minimized
	if (wParam != SIZE_MINIMIZED)
	{
		_d3d_device.Init(_d3d, _d3d_window);

		Init3D();
	}
}

void MagCalibDialog::UpdateD3DSize()
{
	RECT dlgrect;
	int dlgw, dlgh, dxw, dxh;
	
	GetRect(dlgrect);
	dlgw = dlgrect.right - dlgrect.left;
	dlgh = dlgrect.bottom - dlgrect.top;

	dxw = dlgw - _d3d_width_diff;
	dxh = dlgh - _d3d_height_diff;

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

	// we need the aspect ratio of the window to calculate the perspective
	RECT dxrect;
	_d3d_window.GetRect(dxrect);

	// Use D3DX to create a left handed cartesian Field Of View transform
	D3DXMatrixPerspectiveFovLH(	&matProjection,
								D3DX_PI / 4,
								(float) (dxrect.right - dxrect.left) / (dxrect.bottom - dxrect.top),
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

void MagCalibDialog::RefreshControls()
{
	// check if the num points counter needs updating to avoid flickering, then update it
	std::wstring num_str(_lbl_num_points.GetText());
	if (num_str.empty()  ||  _wtoi(num_str.c_str()) != _mag_set.size())
		_lbl_num_points.SetText((int) _mag_set.size());

	_btn_calc_calibration.Enable(_mag_set.size() >= 500);

	if (_dongle.IsOpen())
	{
		_btn_start_sampling.Enable(!_is_collecting_raw_samples);
		_btn_stop_sampling.Enable(_is_collecting_raw_samples);

		_btn_save_calibration.Enable(_is_ellipsoid_fit_valid);
	} else {
		_btn_start_sampling.Enable(false);
		_btn_stop_sampling.Enable(false);
		_btn_save_calibration.Enable(false);
	}
}

void MagCalibDialog::OnTimer(int timerID)
{
	RefreshControls();

	if (!_dongle.IsOpen())
		return;

	// get raw the mag data
	if (_is_collecting_raw_samples)
	{
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
}

void MagCalibDialog::StartSampling()
{
	_is_collecting_raw_samples = true;
	RefreshControls();

	// we need to change the focus away from a disabled control,
	// or else we are not going to receive mousewheel notifications
	_btn_stop_sampling.SetFocus();
}

void MagCalibDialog::StopSampling()
{
	_is_collecting_raw_samples = false;
	RefreshControls();

	// we need to change the focus away from a disabled control,
	// or else we are not going to receive mousewheel notifications
	_btn_start_sampling.SetFocus();
}

void MagCalibDialog::OnControl(int ctrlID, int notifyID, HWND hWndCtrl)
{
	if (ctrlID == IDC_BTN_CLEAR_ALL_POINTS)
		ClearSamples();
	else if (ctrlID == IDC_BTN_RESET_CAMERA)
		_camera.Reset();
	else if (ctrlID == IDC_BTN_SAVE_RAW_POINTS)
		SaveData();
	else if (ctrlID == IDC_BTN_LOAD_RAW_POINTS)
		GenerateEllipsoid();		//LoadData();
	else if (ctrlID == IDC_BTN_CALC_CALIBRATION)
		CalcEllipsoidFit();
	else if (ctrlID == IDC_BTN_CLEAR_CALIBRATED_POINTS)
		ClearCalibPoints();
	else if (ctrlID == IDC_BTN_START_SAMPLING)
		StartSampling();
	else if (ctrlID == IDC_BTN_STOP_SAMPLING)
		StopSampling();
	else if (ctrlID == IDC_BTN_SAVE_CALIBRATION)
		SaveCalibCorrection();
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
	if (!_d3d_device.IsValid())
		return;

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
	OpenSaveFileDialog openFile;
	openFile.AddFilter(L"CSV file (*.csv)", L"*.csv");
	openFile.AddFilter(L"All files (*.*)", L"*.*");

	if (openFile.GetOpenFile(L"Load magnetometer samples", *this))
	{
		WaitCursor wc;

		SimpleFile f;

		if (f.Open(openFile.GetFullFileName(), false))
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

void MagCalibDialog::ClearCalibPoints()
{
	// and default empty vertex buffers
	_vb_mag_points[1].clear();
	_vb_mag_points[1].push_back(VertexBuffer());
	_vb_mag_points[1].back().Alloc(_d3d_device, NUM_VERTICES_PER_VBUFFER);
}

void MagCalibDialog::SaveCalibCorrection()
{
	// save the offsets and the matrix
	FeatRep_DongleSettings rep;

	// get the current settings
	rep.report_id = DONGLE_SETTINGS_REPORT_ID;
	_dongle.GetFeatureReport(rep);

	// set the offsets
	rep.mag_offset[0] = int16_t(_ellipsoid_fit.center.x + 0.5);
	rep.mag_offset[1] = int16_t(_ellipsoid_fit.center.y + 0.5);
	rep.mag_offset[2] = int16_t(_ellipsoid_fit.center.z + 0.5);

	// scale the matrix for fixed point calc and then save
	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
			rep.mag_matrix[i][j] = int16_t(_ellipsoid_fit.calibMatrix[i][j] * (1 << MAG_MATRIX_SCALE_BITS) + 0.5);
	}

	// output the calibration 
	debug(L"\t{" + int2str(rep.mag_offset[0]) + L"," + int2str(rep.mag_offset[1]) + L"," + int2str(rep.mag_offset[2]) + L"},\t\t// mag_offset");
	debug(L"\t{");
	for (int i = 0; i < 3; ++i)
		debug(L"\t\t{" + int2str(rep.mag_matrix[i][0]) + L"," + int2str(rep.mag_matrix[i][1]) + L"," + int2str(rep.mag_matrix[i][2]) + L"},");
	debug(L"\t}");

	// send it back to the dongle
	rep.report_id = DONGLE_SETTINGS_REPORT_ID;
	_dongle.SetFeatureReport(rep);
}

void MagCalibDialog::GenerateEllipsoid()
{
	srand(GetTickCount());

	ClearSamples();

	// make a scale/rotate/translate matrix
	D3DXMATRIX scale, rotate, translate, srt;
	D3DXMatrixScaling(&scale, 0.2f + float(rand()) / RAND_MAX * 2,
								0.2f + float(rand()) / RAND_MAX * 2,
								0.2f + float(rand()) / RAND_MAX * 2);
	D3DXMatrixRotationYawPitchRoll(&rotate, float(M_PI * rand() / RAND_MAX),
											float(M_PI * rand() / RAND_MAX),
											float(M_PI * rand() / RAND_MAX));
	D3DXMatrixTranslation(&translate,	float(rand() % 1000 - 500),
										float(rand() % 1000 - 500),
										float(rand() % 1000 - 500));

	srt = scale * rotate * translate;

	for (int i = 0; i < 5000; ++i)
	{
		// generate a random vector and normalize it, which places it on a sphere
		D3DXVECTOR3 v;
		v.x = float(rand() % 400 - 200);
		v.y = float(rand() % 400 - 200);
		v.z = float(rand() % 400 - 200);

		float m = sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
		v.x = v.x / m * 200;
		v.y = v.y / m * 200;
		v.z = v.z / m * 200;

		D3DXVECTOR4 vout;
		D3DXVec3Transform(&vout, &v, &srt);

		Point<int16_t> p(int16_t(vout.x), int16_t(vout.y), int16_t(vout.z));

		// add the vector
		if (_mag_set.insert(p).second)
		{
			// make the 3D represantations
			AddPoint(p, true);
		}
	}

	CalcEllipsoidFit();
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
