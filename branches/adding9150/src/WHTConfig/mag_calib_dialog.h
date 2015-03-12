#pragma once

class MagCalibDialog: public Dialog
{
private:
	Window		_d3d_window;

	Icon		_icon_large;
	Icon		_icon_small;

	Direct3D	_d3d;
	DeviceD3D	_d3d_device;
	Camera		_camera;

	CoordSys				_coord_sys;		// the coordinate axes
	std::vector<MagPoint>	_mags;			// the magnetometer measurement points

	int			_last_x, _last_y;			// used to calculate mouse movement delta
	bool		_is_dragging;

	WHTDongle&	_dongle;

	void UpdateD3DSize();

public:

	MagCalibDialog(WHTDongle& dngl);

	virtual ~MagCalibDialog()		{}

	virtual void OnInit();
	virtual void OnDestroy();
	virtual void OnSize(int width, int height, WPARAM wParam);
	virtual void OnException(const std::wstring& str);
	virtual void OnTimer(int timerID);

	// view transformation
	virtual void OnLButtonDown(int x, int y, WPARAM wParam);
	virtual void OnLButtonUp(int x, int y, WPARAM wParam);
	virtual void OnMouseMove(int x, int y, WPARAM wParam);
	virtual void OnMouseWheel(int x, int y, int delta, WPARAM wParam);

	void Init3D();
	void Render();
};
