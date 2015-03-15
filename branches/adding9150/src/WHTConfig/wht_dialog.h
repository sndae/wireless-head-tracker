#pragma once

// undefine to enable minimize to tray
//#define MINIMIZE_TO_TRAY

class WHTDialog: public Dialog
{
private:
	Icon		_icon_large;
	Icon		_icon_small;

	ComboBox	_cmb_axis_response;
	ComboBox	_cmb_autocenter;
	ComboBox	_cmb_rf_power;

	ProgressBar	_prg_axis_x;
	ProgressBar	_prg_axis_y;
	ProgressBar	_prg_axis_z;

	EditBox		_edt_fact_x;
	EditBox		_edt_fact_y;
	EditBox		_edt_fact_z;

	Label		_lbl_axis_num_x;
	Label		_lbl_axis_num_y;
	Label		_lbl_axis_num_z;

	Label		_lbl_gyro_bias_x;
	Label		_lbl_gyro_bias_y;
	Label		_lbl_gyro_bias_z;

	Label		_lbl_accel_bias_x;
	Label		_lbl_accel_bias_y;
	Label		_lbl_accel_bias_z;

	Label		_lbl_new_drift_comp;
	Label		_lbl_applied_drift_comp;
	Label		_lbl_packets_sum;

	Label		_lbl_calib_status;

	Button		_btn_connect;
	Button		_btn_calibrate;
	Button		_btn_reset_drift_comp;
	Button		_btn_save_drift_comp;
	Button		_btn_plus;
	Button		_btn_minus;
	Button		_btn_save_rf_power;
	Button		_btn_save_axes_setup;
	Button		_btn_mag_calibration;

	StatusBar	_status_bar;

	WHTDongle	_dongle;

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
	virtual bool OnSysCommand(WPARAM wParam);
#endif

	bool ConnectDongle();
	void ChangeConnectedStateUI();

	virtual void OnTimer(int timerID);
	virtual void OnControl(int ctrlID, int notifyID, HWND hWndCtrl);

	// the magnetometer calibration dialog
	MagCalibDialog		_mag_calig_dlg;

public:
	WHTDialog();
	virtual ~WHTDialog()		{}

	virtual void OnInit();
	
	virtual void OnDestroy()
	{
		::PostQuitMessage(0);
	}

	virtual void OnException(const std::wstring& str);

	MagCalibDialog& GetMagCompDialog()
	{
		return _mag_calig_dlg;
	}
};
