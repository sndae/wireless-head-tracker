#pragma once

#include "Point.h"
#include "EllipsoidFit.h"

class MagCalibDialog: public Dialog
{
private:
	Window		_lbl_num_points;
	Button		_btn_load_raw_points;
	Button		_btn_save_raw_points;
	Button		_btn_start_sampling;
	Button		_btn_stop_sampling;
	Button		_btn_clear_all_points;
	Button		_btn_clear_calibrated_points;
	Button		_btn_calc_calibration;
	Button		_btn_save_calibration;

	Window		_lbl_display_adapter;

	Window		_d3d_window;

	Icon		_icon_large;
	Icon		_icon_small;

	Direct3D	_d3d;
	DeviceD3D	_d3d_device;
	Camera		_camera;

	// difference between D3D window size and the size of the dialog window
	size_t		_d3d_width_diff;
	size_t		_d3d_height_diff;

	// how many cubes do we store in one vertex buffer?
	enum { NUM_VERTICES_PER_CUBE = 36};
	enum { NUM_VERTICES_PER_VBUFFER = 256 * NUM_VERTICES_PER_CUBE};

	// these are the vertex buffers for the three ttypes of objects
	std::vector<VertexBuffer>	_vb_mag_points[2];		// for the raw and calibrated mag measurement points
	VertexBuffer				_vb_coord_sys;			// for the lines - coordinate system and the ellipsoid axes
	VertexBuffer				_vb_ellipsoid_axes;		// for the lines - coordinate system and the ellipsoid axes

	// the D3D objects
	CoordSys					_coord_sys;			// the coordinate axes
	EllipsoidAxes				_ellipsoid_axes;

	std::set<Point<int16_t>>	_mag_set;			// store for the mag points

	EllipsoidFit	_ellipsoid_fit;
	bool			_is_ellipsoid_fit_valid;		// true if the members of _ellipsoid_fit are valid

	bool			_is_collecting_raw_samples;	// true if we're collecting raw samples

	int				_last_x, _last_y;				// used to calculate mouse movement delta
	bool			_is_dragging;

	WHTDongle&		_dongle;

	// adds the point to the set if needed, and creates a 3D cube with
	void AddPoint(const Point<int16_t>& p, const bool is_raw);
	void UnlockVertexBuffers();

	void UpdateD3DSize();

	void ClearSamples();
	void SaveData();
	void LoadData();
	void CalcEllipsoidFit();
	void ClearCalibPoints();
	void RefreshControls();

	void StartSampling();
	void StopSampling();

	// send the calibration values (center offset and the transformation matrix)
	void SaveCalibCorrection();

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
