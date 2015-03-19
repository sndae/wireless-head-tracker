#pragma once

#include "Point.h"
#include "EllipsoidFit.h"

class MagCalibDialog: public Dialog
{
private:
	Button		_btn_clear_points;
	Button		_btn_reset_camera;

	Window		_lbl_num_points;
	Window		_lbl_num_samples;

	Window		_d3d_window;

	Icon		_icon_large;
	Icon		_icon_small;

	Direct3D	_d3d;
	DeviceD3D	_d3d_device;
	Camera		_camera;

	CoordSys			_coord_sys;		// the coordinate axes
	EllipsoidAxes		_ellipsoid_axes;

	int							_num_samples;	// total samples received
	std::set<Point<int16_t>>	_mag_set;		// used for avoiding duplicates
	std::vector<MagPoint>		_mags;			// the magnetometer measurement points

	EllipsoidFit				_ellipsoid_fit;
	bool						_is_valid;		// true if the members of _ellipsoid_fit are valid

	int			_last_x, _last_y;			// used to calculate mouse movement delta
	bool		_is_dragging;

	WHTDongle&	_dongle;

	void UpdateD3DSize();

	void ClearSamples();
	void SaveData();
	void LoadData();
	void CalcEllipsoidFit();

public:

	MagCalibDialog(WHTDongle& dngl);

	virtual ~MagCalibDialog()		{}

	virtual void OnInit();
	virtual void OnDestroy();
	virtual void OnSize(int width, int height, WPARAM wParam);
	virtual void OnException(const std::wstring& str);
	virtual void OnTimer(int timerID);
	virtual void OnControl(int ctrlID, int notifyID, HWND hWndCtrl);

	// view transformation
	virtual void OnLButtonDown(int x, int y, WPARAM wParam);
	virtual void OnLButtonUp(int x, int y, WPARAM wParam);
	virtual void OnMouseMove(int x, int y, WPARAM wParam);
	virtual void OnMouseWheel(int x, int y, int delta, WPARAM wParam);

	void Init3D();
	void Render();
};
