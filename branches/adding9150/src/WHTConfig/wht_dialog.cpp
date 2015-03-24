#include "stdafx.h"

#include "resource.h"

#include "../dongle/reports.h"
#include "my_utils.h"
#include "wht_dongle.h"
#include "my_win.h"
#include "d3d.h"
#include "d3d_objects.h"
#include "mag_calib_dialog.h"
#include "wht_dialog.h"

// status bar "parts"
#define STATBAR_RF_STATUS	0
#define STATBAR_VOLTAGE		1
#define STATBAR_TEMPERATURE	2
#define STATBAR_VERSION		3

WHTDialog::WHTDialog()
:	_icon_large(IDI_ICON, true),
	_icon_small(IDI_ICON, false),
	_autoConnect(true),
	_isConfigChanged(false),
	_isTrackerFound(false),
	_ignoreNotifications(false),
	_readCalibrationCnt(0),
	_mag_calig_dlg(_dongle)
{}

void WHTDialog::OnInit()
{
	SetIcon(_icon_small);
	SetIcon(_icon_large);

	// setup our controls
	_cmb_axis_response.SetHandle(GetCtrl(IDC_CMB_AXIS_RESPONSE));
	_cmb_autocenter.SetHandle(GetCtrl(IDC_CMB_AUTOCENTER));

	_prg_axis_x.SetHandle(GetCtrl(IDC_PRG_AXIS_X));
	_prg_axis_y.SetHandle(GetCtrl(IDC_PRG_AXIS_Y));
	_prg_axis_z.SetHandle(GetCtrl(IDC_PRG_AXIS_Z));

	_lbl_axis_num_x.SetHandle(GetCtrl(IDC_LBL_AXIS_NUM_X));
	_lbl_axis_num_y.SetHandle(GetCtrl(IDC_LBL_AXIS_NUM_Y));
	_lbl_axis_num_z.SetHandle(GetCtrl(IDC_LBL_AXIS_NUM_Z));

	_edt_fact_x.SetHandle(GetCtrl(IDC_EDT_FACT_X));
	_edt_fact_y.SetHandle(GetCtrl(IDC_EDT_FACT_Y));
	_edt_fact_z.SetHandle(GetCtrl(IDC_EDT_FACT_Z));

	_lbl_new_drift_comp.SetHandle(GetCtrl(IDC_LBL_NEW_DRIFT_COMP));
	_lbl_applied_drift_comp.SetHandle(GetCtrl(IDC_LBL_APPLIED_DRIFT_COMP));
	_lbl_packets_sum.SetHandle(GetCtrl(IDC_LBL_PACKETS_SUM));
	_lbl_calib_status.SetHandle(GetCtrl(IDC_LBL_CALIB_STATUS));

	_lbl_gyro_bias_x.SetHandle(GetCtrl(IDC_LBL_GYRO_BIAS_X));
	_lbl_gyro_bias_y.SetHandle(GetCtrl(IDC_LBL_GYRO_BIAS_Y));
	_lbl_gyro_bias_z.SetHandle(GetCtrl(IDC_LBL_GYRO_BIAS_Z));

	_lbl_accel_bias_x.SetHandle(GetCtrl(IDC_LBL_ACCEL_BIAS_X));
	_lbl_accel_bias_y.SetHandle(GetCtrl(IDC_LBL_ACCEL_BIAS_Y));
	_lbl_accel_bias_z.SetHandle(GetCtrl(IDC_LBL_ACCEL_BIAS_Z));

	_btn_connect.SetHandle(GetCtrl(IDC_BTN_CONNECT));
	_btn_calibrate.SetHandle(GetCtrl(IDC_BTN_CALIBRATE));
	_btn_reset_drift_comp.SetHandle(GetCtrl(IDC_BTN_RESET_DRIFT_COMP));
	_btn_save_drift_comp.SetHandle(GetCtrl(IDC_BTN_SAVE_DRIFT_COMP));
	_btn_plus.SetHandle(GetCtrl(IDC_BTN_PLUS));
	_btn_minus.SetHandle(GetCtrl(IDC_BTN_MINUS));
	_btn_save_axes_setup.SetHandle(GetCtrl(IDC_BTN_SAVE_AXES_SETUP));
	_btn_mag_calibration.SetHandle(GetCtrl(IDC_BTN_MAG_CALIBRATION));

	_status_bar.SetHandle(GetCtrl(IDC_STATUS_BAR));

	// setup the progress bar ranges
	_prg_axis_x.SetRange(0, 0xffff);
	_prg_axis_y.SetRange(0, 0xffff);
	_prg_axis_z.SetRange(0, 0xffff);

	// init the combo boxes
	_cmb_axis_response.AddString(L"Exponential");
	_cmb_axis_response.AddString(L"Linear");

	_cmb_autocenter.AddString(L"Off");
	_cmb_autocenter.AddString(L"Light");
	_cmb_autocenter.AddString(L"Medium");
	_cmb_autocenter.AddString(L"Heavy");

	// disable the controls since we're not connected to the dongle yet
	ChangeConnectedStateUI();

	// setup the status bar parts
	const int NUM_PARTS = 4;
	int parts[NUM_PARTS];
	parts[0] = 110;
	parts[1] = 230;
	parts[2] = 350;
	parts[3] = -1;

	_status_bar.SetParts(parts, NUM_PARTS);

	_status_bar.SetPartText(STATBAR_VERSION, WIDEN(__DATE__) L" " WIDEN(__TIME__));

	// start the timer
	::SetTimer(_hWnd, 1, 100, NULL);	// 100ms which is 10 Hz
}

void WHTDialog::OnControl(int ctrlID, int notifyID, HWND hWndCtrl)
{
	if (ctrlID == IDC_BTN_CALIBRATE)
	{
		FeatRep_Command rep;
		rep.report_id = COMMAND_REPORT_ID;
		rep.command = CMD_CALIBRATE;
		_dongle.SetFeatureReport(rep);

		// clear the calibration fields
		_lbl_calib_status.SetText(L"Calibrating...");
		_lbl_gyro_bias_x.ClearText();
		_lbl_gyro_bias_y.ClearText();
		_lbl_gyro_bias_z.ClearText();
		_lbl_accel_bias_x.ClearText();
		_lbl_accel_bias_y.ClearText();
		_lbl_accel_bias_z.ClearText();

		_readCalibrationCnt = 15;

	} else if (ctrlID == IDC_BTN_SAVE_AXES_SETUP) {
		
		SendConfigToDevice();
		_isConfigChanged = false;
		ChangeConnectedStateUI();

	} else if (ctrlID == IDC_BTN_CONNECT) {

		if (_dongle.IsOpen())
		{
			_dongle.Close();
			_isConfigChanged = false;
			_isTrackerFound = false;
			ChangeConnectedStateUI();
		
		} else {

			if (!ConnectDongle())
				MsgBox(L"Wireless head tracker dongle not found.\nCheck if the dongle is connected to USB and try again.", L"Error", MB_OK | MB_ICONERROR);
		}


	} else if (ctrlID == IDC_BTN_RESET_DRIFT_COMP) {
		
		FeatRep_Command rep;
		rep.report_id = COMMAND_REPORT_ID;
		rep.command = CMD_RECENTER;
		_dongle.SetFeatureReport(rep);

	} else if (ctrlID == IDC_BTN_SAVE_DRIFT_COMP) {

		FeatRep_Command rep;
		rep.report_id = COMMAND_REPORT_ID;
		rep.command = CMD_SAVE_DRIFT;
		_dongle.SetFeatureReport(rep);

		// read the new drift compensation value back
		FeatRep_DongleSettings repSettings;
		repSettings.report_id = DONGLE_SETTINGS_REPORT_ID;
		_dongle.GetFeatureReport(repSettings);

		_lbl_applied_drift_comp.SetText(repSettings.drift_per_1k / float(1024));

		// reset the drift calculation
		FeatRep_Command repReset;
		repReset.report_id = COMMAND_REPORT_ID;
		repReset.command = CMD_RECENTER;
		_dongle.SetFeatureReport(repReset);

	} else if (ctrlID == IDC_BTN_PLUS  ||  ctrlID == IDC_BTN_MINUS) {

		FeatRep_Command repReset;
		repReset.report_id = COMMAND_REPORT_ID;
		repReset.command = (ctrlID == IDC_BTN_PLUS ? CMD_INC_DRIFT_COMP : CMD_DEC_DRIFT_COMP);
		_dongle.SetFeatureReport(repReset);

		// read the new drift compensation value back
		FeatRep_DongleSettings repSettings;
		repSettings.report_id = DONGLE_SETTINGS_REPORT_ID;
		_dongle.GetFeatureReport(repSettings);

		_lbl_applied_drift_comp.SetText(repSettings.drift_per_1k / float(1024));

	} else if (ctrlID == IDC_EDT_FACT_X  ||  ctrlID == IDC_EDT_FACT_Y  ||  ctrlID == IDC_EDT_FACT_Z) {

		// if the text in the exit boxes has changed
		if (notifyID == EN_CHANGE  &&  	!_ignoreNotifications)
		{
			_isConfigChanged = true;
			ChangeConnectedStateUI();
		}

	} else if (ctrlID == IDC_CMB_AXIS_RESPONSE  ||  ctrlID == IDC_CMB_AUTOCENTER) {

		// if the selection of the combo box has changed
		if (notifyID == CBN_SELCHANGE  &&  	!_ignoreNotifications)
		{
			_isConfigChanged = true;
			ChangeConnectedStateUI();
		}

	/*} else if (ctrlID == IDC_CMB_RF_POWER) {

		// if the selection of the combo box has changed
		if (notifyID == CBN_SELCHANGE  &&  	!_ignoreNotifications)
		{
			_isPowerChanged = true;
			ChangeConnectedStateUI();
		}

	} else if (ctrlID == IDC_BTN_SAVE_RF_POWER) {
		
		FeatRep_Command repReset;
		repReset.report_id = COMMAND_REPORT_ID;

		switch (_cmb_rf_power.GetSelection())
		{
		case 0:		repReset.command = CMD_RF_PWR_LOWEST;		break;
		case 1:		repReset.command = CMD_RF_PWR_LOWER;		break;
		case 2:		repReset.command = CMD_RF_PWR_HIGHER;		break;
		case 3:		repReset.command = CMD_RF_PWR_HIGHEST;		break;
		}

		_dongle.SetFeatureReport(repReset);

		_isPowerChanged = false;
		ChangeConnectedStateUI();
		*/
	} else if (ctrlID == IDC_BTN_MAG_CALIBRATION) {
		
		if (_mag_calig_dlg.IsValid())
			_mag_calig_dlg.Show();
		else
			_mag_calig_dlg.CreateDlg(IDD_MAGNETOMETER_CALIB, 0);
	}
}

void WHTDialog::OnTimer(int timerID)
{
	if (_dongle.IsOpen())
	{
		// get the axis states
		hid_joystick_report_t rep;
		rep.report_id = JOYSTICK_REPORT_ID;
		_dongle.GetInputReport(rep);

		_prg_axis_x.SetPos(rep.x + 0x8000);
		_prg_axis_y.SetPos(rep.y + 0x8000);
		_prg_axis_z.SetPos(rep.z + 0x8000);

		_lbl_axis_num_x.SetText(rep.x);
		_lbl_axis_num_y.SetText(rep.y);
		_lbl_axis_num_z.SetText(rep.z);

		// get the current status
		FeatRep_Status repStatus;
		repStatus.report_id = STATUS_REPORT_ID;
		_dongle.GetFeatureReport(repStatus);

		std::wstring res;
		if (repStatus.num_packets >= 48)
			res = L"100";
		else
			res = int2str(repStatus.num_packets * 2);

		_status_bar.SetPartText(STATBAR_RF_STATUS, L"RF packets: " + res + L"%");

		if (repStatus.sample_cnt)
			_lbl_new_drift_comp.SetText(repStatus.yaw_value / (float)repStatus.sample_cnt);
		else
			_lbl_new_drift_comp.SetText(0);

		_lbl_packets_sum.SetText(int2str(repStatus.sample_cnt) + L" / " + int2str(repStatus.yaw_value));

		const int BUFF_SIZE = 256;
		wchar_t buff[BUFF_SIZE];
		swprintf_s(buff, BUFF_SIZE, L"Batt. voltage: %2.2fV", repStatus.battery_voltage / 100.0);
		_status_bar.SetPartText(STATBAR_VOLTAGE, buff);

		swprintf_s(buff, BUFF_SIZE, L"Temperature: %2.1f°C", repStatus.temperature / 10.0);
		_status_bar.SetPartText(STATBAR_TEMPERATURE, buff);

		// read the calibration if needed
		if (_readCalibrationCnt)
		{
			if (_readCalibrationCnt == 1)
			{
				if (repStatus.num_packets > 0)
				{
					ReadTrackerSettings();
					_readCalibrationCnt = 0;
				}
			} else {
				--_readCalibrationCnt;
			}

		} else if (!_isTrackerFound  &&  repStatus.num_packets >= 10) {

			// read the tracker settings if the dongle received data from the tracker
			ReadTrackerSettings();
		}


	} else if (_autoConnect) {

		ConnectDongle();
		_autoConnect = false;

	} else {

		_prg_axis_x.SetPos(0);
		_prg_axis_y.SetPos(0);
		_prg_axis_z.SetPos(0);

		_status_bar.SetPartText(STATBAR_RF_STATUS, L"Disconnected");
		_status_bar.SetPartText(STATBAR_VOLTAGE, L"Batt. voltage:");
		_status_bar.SetPartText(STATBAR_TEMPERATURE, L"Temperature:");
	}
}

void WHTDialog::OnException(const std::wstring& str)
{
	_dongle.Close();			// close the HID device
	_isConfigChanged = false;	// we're haven't changed anything since we're close
	ChangeConnectedStateUI();	// disable controls change the UI

	MsgBox(str, L"Exception", MB_OK | MB_ICONERROR);
}

#ifdef MINIMIZE_TO_TRAY

bool WHTDialog::OnSysCommand(WPARAM wParam)
{
	if (wParam == SC_MINIMIZE)
	{
		// minimize to tray
		CreateTrayIcon();
		Hide();

		return true;
	}

	return false;
}

void WHTDialog::CreateTrayIcon()
{
	NOTIFYICONDATA nid;
 
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = _hWnd;
	nid.uID = 100;
	nid.uVersion = NOTIFYICON_VERSION;
	nid.uCallbackMessage = WM_TRAYNOTIFY;
	nid.hIcon = _hIconSmall;
	wcscpy_s(nid.szTip, L"Wireless Head Tracker");
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
 
	Shell_NotifyIcon(NIM_ADD, &nid);
}

void WHTDialog::OnTrayNotify(LPARAM lParam)
{
	if (lParam == WM_LBUTTONDOWN)
	{
		Show();
		RemoveTrayIcon();
	}
}

void WHTDialog::RemoveTrayIcon()
{
	NOTIFYICONDATA nid;
 
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = _hWnd;
	nid.uID = 100;
	nid.uVersion = NOTIFYICON_VERSION;
	nid.uCallbackMessage = WM_TRAYNOTIFY;
	nid.hIcon = _hIconSmall;
	wcscpy_s(nid.szTip, L"Wireless Head Tracker");
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
 
	Shell_NotifyIcon(NIM_DELETE, &nid);
}

#endif	// MINIMIZE_TO_TRAY

void WHTDialog::ReadConfigFromDevice()
{
	FeatRep_DongleSettings rep;
	rep.report_id = DONGLE_SETTINGS_REPORT_ID;
	_dongle.GetFeatureReport(rep);

	_lbl_applied_drift_comp.SetText(rep.drift_per_1k / float(1024));

	_ignoreNotifications = true;

	_cmb_axis_response.SetSelection(rep.is_linear ? 1 : 0);
	_cmb_autocenter.SetSelection(rep.autocenter);

	_edt_fact_x.SetText(rep.fact[0]);
	_edt_fact_y.SetText(rep.fact[1]);
	_edt_fact_z.SetText(rep.fact[2]);

	_isConfigChanged = false;

	_ignoreNotifications = false;
}

void WHTDialog::ReadTrackerSettings()
{
	_lbl_calib_status.ClearText();
	_lbl_gyro_bias_x.ClearText();
	_lbl_gyro_bias_y.ClearText();
	_lbl_gyro_bias_z.ClearText();
	_lbl_accel_bias_x.ClearText();
	_lbl_accel_bias_y.ClearText();
	_lbl_accel_bias_z.ClearText();

	FeatRep_TrackerSettings rep;
	rep.report_id = TRACKER_SETTINGS_REPORT_ID;
	_dongle.GetFeatureReport(rep);

	if (rep.has_tracker_responded == 0)
	{
		_lbl_calib_status.SetText(L"Tracker not found");
		_isTrackerFound = false;
	} else {
		_lbl_calib_status.SetText(rep.is_calibrated ? L"Calibrated" : L"Not calibrated");

		_lbl_gyro_bias_x.SetText(rep.gyro_bias[0]);
		_lbl_gyro_bias_y.SetText(rep.gyro_bias[1]);
		_lbl_gyro_bias_z.SetText(rep.gyro_bias[2]);

		_lbl_accel_bias_x.SetText(rep.accel_bias[0]);
		_lbl_accel_bias_y.SetText(rep.accel_bias[1]);
		_lbl_accel_bias_z.SetText(rep.accel_bias[2]);

		_isTrackerFound = true;
	}
}

void WHTDialog::SendConfigToDevice()
{
	FeatRep_DongleSettings rep;

	// get the current settings
	_dongle.GetFeatureReport(rep);

	// change the values we are interested in
	rep.is_linear = _cmb_axis_response.GetSelection();
	rep.autocenter = _cmb_autocenter.GetSelection();

	rep.fact[0] = _edt_fact_x.GetInt();
	rep.fact[1] = _edt_fact_y.GetInt();
	rep.fact[2] = _edt_fact_z.GetInt();

	// send it back to the dongle
	rep.report_id = DONGLE_SETTINGS_REPORT_ID;
	_dongle.SetFeatureReport(rep);
}

bool WHTDialog::ConnectDongle()
{
	if (_dongle.Open())
	{
		ReadConfigFromDevice();
		ReadTrackerSettings();
		ChangeConnectedStateUI();

		return true;
	}

	return false;
}

void WHTDialog::ChangeConnectedStateUI()
{
	bool is_connected = _dongle.IsOpen();

	_btn_connect.SetText(is_connected ? L"Disconnect" : L"Connect");

	_btn_calibrate.Enable(is_connected);
	_btn_minus.Enable(is_connected);
	_btn_plus.Enable(is_connected);
	_btn_reset_drift_comp.Enable(is_connected);
	_btn_save_drift_comp.Enable(is_connected);
	_cmb_axis_response.Enable(is_connected);
	_cmb_autocenter.Enable(is_connected);
	_edt_fact_x.Enable(is_connected);
	_edt_fact_y.Enable(is_connected);
	_edt_fact_z.Enable(is_connected);

	//_btn_mag_calibration.Enable(is_connected);

	_btn_save_axes_setup.Enable(is_connected  &&  _isConfigChanged);
}
