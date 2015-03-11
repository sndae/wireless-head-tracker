#pragma once

// undefine to enable minimize to tray
#define MINIMIZE_TO_TRAY

class WHTDialog: public Dialog
{
private:
	HICON		_hIconSmall;
	HICON		_hIconBig;

	ComboBox	_cmb_axis_response;
	ComboBox	_cmb_autocenter;
	ComboBox	_cmb_rf_power;

	ProgressBar	_prg_axis_x;
	ProgressBar	_prg_axis_y;
	ProgressBar	_prg_axis_z;

	Window		_edt_fact_x;
	Window		_edt_fact_y;
	Window		_edt_fact_z;

	Window		_lbl_axis_num_x;
	Window		_lbl_axis_num_y;
	Window		_lbl_axis_num_z;

	Window		_lbl_gyro_bias_x;
	Window		_lbl_gyro_bias_y;
	Window		_lbl_gyro_bias_z;

	Window		_lbl_accel_bias_x;
	Window		_lbl_accel_bias_y;
	Window		_lbl_accel_bias_z;

	Window		_lbl_new_drift_comp;
	Window		_lbl_packets_sum;

	Window		_lbl_calib_status;

	StatusBar	_status_bar;

	WHTDevice	_device;

	bool		_isConfigChanged;
	bool		_autoConnect;
	bool		_isPowerChanged;
	bool		_ignoreNotifications;
	bool		_isTrackerFound;
	int			_readCalibrationCnt;

	void ReadConfigFromDevice();
	void ReadTrackerSettings();
	void SendConfigToDevice();

#ifdef MINIMIZE_TO_TRAY

	void CreateTrayIcon();
	void RemoveTrayIcon();
	virtual void OnTrayNotify(LPARAM lParam);
	void OnMinimize();

#endif

	bool ConnectDongle();
	void ChangeConnectedStateUI();

public:
	WHTDialog();
	virtual ~WHTDialog()		{}

	virtual void OnInit();
	
	virtual void OnDestroy()
	{
		::PostQuitMessage(0);
	}

	virtual void OnTimer(int timerID);
	virtual void OnControl(int ctrlID, int notifyID, HWND hWndCtrl);
	virtual void OnSysCommand(WPARAM wParam);
};
