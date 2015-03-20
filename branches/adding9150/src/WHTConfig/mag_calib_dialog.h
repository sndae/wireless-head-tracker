#pragma once

#include "Point.h"
#include "EllipsoidFit.h"

class MagCalibDialog: public Dialog
{
private:
	Button		_btn_clear_points;
	Button		_btn_reset_camera;

	Window		_lbl_num_points;

	Window		_d3d_window;

	Icon		_icon_large;
	Icon		_icon_small;

	Direct3D	_d3d;
	DeviceD3D	_d3d_device;
	Camera		_camera;

	// how many cubes do we store in one vertex buffer?
	enum { NUM_VERTICES_PER_CUBE = 36};
	enum { NUM_VERTICES_PER_VBUFFER = 256 * NUM_VERTICES_PER_CUBE};

	// these are the vertex buffers for the three ttypes of objects
	std::vector<VertexBuffer>	_vb_mag_points[2];		// for the raw and calibrated mag measurement points
	VertexBuffer				_line_vertex_buffer;	// for the lines - coordinate system and the ellipsoid axes

	// the D3D objects
	CoordSys					_coord_sys;		// the coordinate axes
	EllipsoidAxes				_ellipsoid_axes;

	int							_num_samples;	// total samples received
	std::set<Point<int16_t>>	_mag_set;		// used for avoiding duplicates

	EllipsoidFit				_ellipsoid_fit;
	bool						_is_valid;		// true if the members of _ellipsoid_fit are valid

	int			_last_x, _last_y;				// used to calculate mouse movement delta
	bool		_is_dragging;

	WHTDongle&	_dongle;

	// adds the point to the set if needed, and creates a 3D cube with
	void AddPoint(const Point<int16_t>& p, const bool is_raw);

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
