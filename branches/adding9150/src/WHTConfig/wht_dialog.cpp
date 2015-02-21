#include "stdafx.h"

#include "resource.h"
#include "../dongle/reports.h"
#include "myutils.h"
#include "wht_device.h"
#include "wht_dialog.h"

#ifdef MINIMIZE_TO_TRAY
# define WM_TRAYNOTIFY		(WM_APP+1)
#endif

#define WIDEN2(x)		L ## x
#define WIDEN(x)		WIDEN2(x)

// status bar "parts"
#define STATBAR_RF_STATUS	0
#define STATBAR_VOLTAGE		1
#define STATBAR_TEMPERATURE	2
#define STATBAR_VERSION		3

BOOL CALLBACK WHTDialog::MyDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	WHTDialog* pDlg = reinterpret_cast<WHTDialog*> (GetWindowLong(hDlg, GWL_USERDATA));

	if (message == WM_INITDIALOG)
	{
		new WHTDialog(hDlg);
		return TRUE;
	}
	
	if (message == WM_DESTROY  &&  pDlg)
	{
		delete pDlg;
		pDlg = NULL;
	}

	if (pDlg)
		return pDlg->OnMessage(message, wParam, lParam);

	return FALSE;
}

WHTDialog::WHTDialog(HWND hDlg)
:	hDialog(hDlg),
	autoConnect(true),
	isConfigChanged(false),
	isPowerChanged(false),
	isTrackerFound(false),
	ignoreNotifications(false),
	readCalibrationCnt(0)
{
	// first save the dialog pointer to user data
	SetWindowLong(hDialog, GWL_USERDATA, reinterpret_cast<LONG>(this));

	// create the icons
	hIconBig = (HICON) LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, 48, 48, LR_SHARED);
	SendMessage(hDialog, WM_SETICON, ICON_BIG, (LPARAM) hIconBig);

	hIconSmall = (HICON) LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, 16, 16, LR_SHARED);
	SendMessage(hDialog, WM_SETICON, ICON_SMALL, (LPARAM) hIconSmall);

	// setup the progress bar ranges
	SendMessage(GetCtrl(IDC_PRG_AXIS_X), PBM_SETRANGE, 0, MAKELPARAM(0, 0xffff));
	SendMessage(GetCtrl(IDC_PRG_AXIS_Y), PBM_SETRANGE, 0, MAKELPARAM(0, 0xffff));
	SendMessage(GetCtrl(IDC_PRG_AXIS_Z), PBM_SETRANGE, 0, MAKELPARAM(0, 0xffff));

	// start the timer
	SetTimer(hDialog, 1, 100, NULL);	// 100ms which is 10 Hz

	// init the the axis response combo box
	AddComboString(IDC_CMB_AXIS_RESPONSE, L"Exponential");
	AddComboString(IDC_CMB_AXIS_RESPONSE, L"Linear");
	//SetComboSelection(IDC_CMB_AXIS_RESPONSE, 0);

	AddComboString(IDC_CMB_AUTOCENTER, L"Off");
	AddComboString(IDC_CMB_AUTOCENTER, L"Light");
	AddComboString(IDC_CMB_AUTOCENTER, L"Medium");
	AddComboString(IDC_CMB_AUTOCENTER, L"Heavy");
	//SetComboSelection(IDC_CMB_AUTOCENTER, 0);

	AddComboString(IDC_CMB_RF_POWER, L"Lowest");
	AddComboString(IDC_CMB_RF_POWER, L"Lower");
	AddComboString(IDC_CMB_RF_POWER, L"Higher");
	AddComboString(IDC_CMB_RF_POWER, L"Highest");

	// disable the controls since we're not connected to the dongle yet
	ChangeConnectedStateUI();

	// setup the status bar
	InitStatusbar();

	SetStatusbarText(STATBAR_VERSION, WIDEN(__DATE__) L" " WIDEN(__TIME__));
}

WHTDialog::~WHTDialog()
{
	// clear the dialog pointer
	SetWindowLong(hDialog, GWL_USERDATA, 0);
}

BOOL WHTDialog::OnMessage(int message, WPARAM wParam, LPARAM lParam)
{
	try {
		switch (message)
		{
		case WM_COMMAND:

			OnCommand(LOWORD(wParam), HIWORD(wParam));
			return FALSE;

		case WM_TIMER:

			OnTimer();
			return TRUE;

#ifdef MINIMIZE_TO_TRAY

		case WM_SYSCOMMAND:

			if (wParam == SC_MINIMIZE)
			{
				// minimize to tray
				OnMinimize();
				return TRUE;
			}

			return FALSE;

		case WM_TRAYNOTIFY:

			OnTrayNotify(lParam);
			return TRUE;

#endif

		case WM_CLOSE:

			EndDialog(hDialog, 0);
			return TRUE;
		}

	} catch (std::wstring& e) {

		device.Close();				// close the HID device
		isConfigChanged = false;	// we're haven't changed anything since we're close
		ChangeConnectedStateUI();	// disable controls change the UI

		MessageBox(hDialog, e.c_str(), L"Exception", MB_OK | MB_ICONERROR);
	}

	return FALSE;
}

void WHTDialog::OnCommand(int ctrl_id, int notification)
{
	if (ctrl_id == IDC_BTN_CALIBRATE)
	{
		FeatRep_Command rep;
		rep.report_id = COMMAND_REPORT_ID;
		rep.command = CMD_CALIBRATE;
		device.SetFeatureReport(rep);

		// clear the calibration fields
		SetCtrlText(IDC_LBL_CALIB_STATUS, L"Calibrating...");
		ClearCtrlText(IDC_LBL_GYRO_BIAS_X);
		ClearCtrlText(IDC_LBL_GYRO_BIAS_Y);
		ClearCtrlText(IDC_LBL_GYRO_BIAS_Z);
		ClearCtrlText(IDC_LBL_ACCEL_BIAS_X);
		ClearCtrlText(IDC_LBL_ACCEL_BIAS_Y);
		ClearCtrlText(IDC_LBL_ACCEL_BIAS_Z);

		readCalibrationCnt = 15;

	} else if (ctrl_id == IDC_BTN_SAVE_AXES_SETUP) {
		
		SendConfigToDevice();
		isConfigChanged = false;
		ChangeConnectedStateUI();

	} else if (ctrl_id == IDC_BTN_CONNECT) {

		if (device.IsOpen())
		{
			device.Close();
			isConfigChanged = false;
			isPowerChanged = false;
			isTrackerFound = false;
			ChangeConnectedStateUI();
		
		} else {

			if (!ConnectDongle())
				MessageBox(hDialog, L"Wireless head tracker dongle not found.", L"Error", MB_OK | MB_ICONERROR);
		}


	} else if (ctrl_id == IDC_BTN_RESET_DRIFT_COMP) {
		
		FeatRep_Command rep;
		rep.report_id = COMMAND_REPORT_ID;
		rep.command = CMD_RECENTER;
		device.SetFeatureReport(rep);

	} else if (ctrl_id == IDC_BTN_SAVE_DRIFT_COMP) {

		FeatRep_Command rep;
		rep.report_id = COMMAND_REPORT_ID;
		rep.command = CMD_SAVE_DRIFT;
		device.SetFeatureReport(rep);

		// read the new drift compensation value back
		FeatRep_DongleSettings repSettings;
		repSettings.report_id = DONGLE_SETTINGS_REPORT_ID;
		device.GetFeatureReport(repSettings);

		SetCtrlText(IDC_LBL_APPLIED_DRIFT_COMP, flt2str(repSettings.drift_per_1k / float(1024)));

		// reset the drift calculation
		FeatRep_Command repReset;
		repReset.report_id = COMMAND_REPORT_ID;
		repReset.command = CMD_RECENTER;
		device.SetFeatureReport(repReset);

	} else if (ctrl_id == IDC_BTN_PLUS  ||  ctrl_id == IDC_BTN_MINUS) {

		FeatRep_Command repReset;
		repReset.report_id = COMMAND_REPORT_ID;
		repReset.command = (ctrl_id == IDC_BTN_PLUS ? CMD_INC_DRIFT_COMP : CMD_DEC_DRIFT_COMP);
		device.SetFeatureReport(repReset);

		// read the new drift compensation value back
		FeatRep_DongleSettings repSettings;
		repSettings.report_id = DONGLE_SETTINGS_REPORT_ID;
		device.GetFeatureReport(repSettings);

		SetCtrlText(IDC_LBL_APPLIED_DRIFT_COMP, flt2str(repSettings.drift_per_1k / float(1024)));

	} else if (ctrl_id == IDC_EDT_FACT_X  ||  ctrl_id == IDC_EDT_FACT_Y  ||  ctrl_id == IDC_EDT_FACT_Z) {

		// if the text in the exit boxes has changed
		if (notification == EN_CHANGE  &&  	!ignoreNotifications)
		{
			isConfigChanged = true;
			ChangeConnectedStateUI();
		}

	} else if (ctrl_id == IDC_CMB_AXIS_RESPONSE  ||  ctrl_id == IDC_CMB_AUTOCENTER) {

		// if the selection of the combo box has changed
		if (notification == CBN_SELCHANGE  &&  	!ignoreNotifications)
		{
			isConfigChanged = true;
			ChangeConnectedStateUI();
		}

	} else if (ctrl_id == IDC_CMB_RF_POWER) {

		// if the selection of the combo box has changed
		if (notification == CBN_SELCHANGE  &&  	!ignoreNotifications)
		{
			isPowerChanged = true;
			ChangeConnectedStateUI();
		}

	} else if (ctrl_id == IDC_BTN_SAVE_RF_POWER) {
		
		FeatRep_Command repReset;
		repReset.report_id = COMMAND_REPORT_ID;

		switch (GetComboSelection(IDC_CMB_RF_POWER))
		{
		case 0:		repReset.command = CMD_RF_PWR_LOWEST;		break;
		case 1:		repReset.command = CMD_RF_PWR_LOWER;		break;
		case 2:		repReset.command = CMD_RF_PWR_HIGHER;		break;
		case 3:		repReset.command = CMD_RF_PWR_HIGHEST;		break;
		}

		device.SetFeatureReport(repReset);

		isPowerChanged = false;
		ChangeConnectedStateUI();
	}
}

void WHTDialog::OnTimer()
{
	if (device.IsOpen())
	{
		// get the axis states
		hid_joystick_report_t rep;
		rep.report_id = JOYSTICK_REPORT_ID;
		device.GetInputReport(rep);

		SendMessage(GetCtrl(IDC_PRG_AXIS_X), PBM_SETPOS, (WPARAM) rep.x + 0x8000, 0);
		SendMessage(GetCtrl(IDC_PRG_AXIS_Y), PBM_SETPOS, (WPARAM) rep.y + 0x8000, 0);
		SendMessage(GetCtrl(IDC_PRG_AXIS_Z), PBM_SETPOS, (WPARAM) rep.z + 0x8000, 0);

		SetCtrlTextInt(IDC_LBL_AXIS_NUM_X, rep.x);
		SetCtrlTextInt(IDC_LBL_AXIS_NUM_Y, rep.y);
		SetCtrlTextInt(IDC_LBL_AXIS_NUM_Z, rep.z);

		// get the current status
		FeatRep_Status repStatus;
		repStatus.report_id = STATUS_REPORT_ID;
		device.GetFeatureReport(repStatus);

		std::wstring res;
		if (repStatus.num_packets >= 48)
			res = L"100";
		else
			res = int2str(repStatus.num_packets * 2);

		SetStatusbarText(STATBAR_RF_STATUS, L"RF packets: " + res + L"%");

		SetCtrlText(IDC_LBL_NEW_DRIFT_COMP, flt2str(repStatus.new_drift_comp));
		SetCtrlText(IDC_LBL_PACKETS_SUM, int2str(repStatus.sample_cnt) + L" / " + int2str(repStatus.yaw_value));

		const int BUFF_SIZE = 256;
		wchar_t buff[BUFF_SIZE];
		swprintf_s(buff, BUFF_SIZE, L"Batt. voltage: %2.2fV", repStatus.battery_voltage / 100.0);
		SetStatusbarText(STATBAR_VOLTAGE, buff);

		swprintf_s(buff, BUFF_SIZE, L"Temperature: %2.1f°C", repStatus.temperature / 10.0);
		SetStatusbarText(STATBAR_TEMPERATURE, buff);

		// read the calibration if needed
		if (readCalibrationCnt)
		{
			if (readCalibrationCnt == 1)
			{
				if (repStatus.num_packets > 0)
				{
					ReadTrackerSettings();
					readCalibrationCnt = 0;
				}
			} else {
				--readCalibrationCnt;
			}

		} else if (!isTrackerFound  &&  repStatus.num_packets >= 10) {

			// read the tracker settings if the dongle received data from the tracker
			ReadTrackerSettings();
		}


	} else if (autoConnect) {

		ConnectDongle();
		autoConnect = false;

	} else {

		SendMessage(GetCtrl(IDC_PRG_AXIS_X), PBM_SETPOS, 0, 0);
		SendMessage(GetCtrl(IDC_PRG_AXIS_Y), PBM_SETPOS, 0, 0);
		SendMessage(GetCtrl(IDC_PRG_AXIS_Z), PBM_SETPOS, 0, 0);

		SetStatusbarText(STATBAR_RF_STATUS, L"Disconnected");
		SetStatusbarText(STATBAR_VOLTAGE, L"Batt. voltage:");
		SetStatusbarText(STATBAR_TEMPERATURE, L"Temperature:");
	}
}

#ifdef MINIMIZE_TO_TRAY

void WHTDialog::OnMinimize()
{
	CreateTrayIcon();
	Hide();
}

void WHTDialog::CreateTrayIcon()
{
	NOTIFYICONDATA nid;
 
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hDialog;
	nid.uID = 100;
	nid.uVersion = NOTIFYICON_VERSION;
	nid.uCallbackMessage = WM_TRAYNOTIFY;
	nid.hIcon = hIconSmall;
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
	//} else if (lParam == WM_RBUTTONDOWN) {
	}
}

void WHTDialog::RemoveTrayIcon()
{
	NOTIFYICONDATA nid;
 
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hDialog;
	nid.uID = 100;
	nid.uVersion = NOTIFYICON_VERSION;
	nid.uCallbackMessage = WM_TRAYNOTIFY;
	nid.hIcon = hIconSmall;
	wcscpy_s(nid.szTip, L"Wireless Head Tracker");
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
 
	Shell_NotifyIcon(NIM_DELETE, &nid);
}

#endif

void WHTDialog::InitStatusbar()
{
	// make the status bar parts
	const int NUM_PARTS = 4;
	int parts[NUM_PARTS];
	parts[0] = 110;
	parts[1] = 230;
	parts[2] = 350;
	parts[3] = -1;

	SendMessage(GetCtrl(IDC_STATUS_BAR), SB_SETPARTS, NUM_PARTS, (LPARAM) parts);
}

int16_t WHTDialog::GetCtrlTextInt(int ctrl_id)
{
	const int BUFF_SIZE = 256;
	wchar_t buff[BUFF_SIZE];
	SendMessage(GetCtrl(ctrl_id), WM_GETTEXT, BUFF_SIZE, (LPARAM) buff);

	return (int16_t) _wtoi(buff);
}

float WHTDialog::GetCtrlTextFloat(int ctrl_id)
{
	const int BUFF_SIZE = 256;
	wchar_t buff[BUFF_SIZE];
	SendMessage(GetCtrl(ctrl_id), WM_GETTEXT, BUFF_SIZE, (LPARAM) buff);

	return (float) _wtof(buff);
}

void WHTDialog::ReadConfigFromDevice()
{
	FeatRep_DongleSettings rep;
	rep.report_id = DONGLE_SETTINGS_REPORT_ID;
	device.GetFeatureReport(rep);

	SetCtrlText(IDC_LBL_APPLIED_DRIFT_COMP, flt2str(rep.drift_per_1k / float(1024)));

	ignoreNotifications = true;

	SetComboSelection(IDC_CMB_AXIS_RESPONSE, rep.is_linear ? 1 : 0);
	SetComboSelection(IDC_CMB_AUTOCENTER, rep.autocenter);

	SetCtrlText(IDC_EDT_FACT_X, int2str(rep.fact[0]));
	SetCtrlText(IDC_EDT_FACT_Y, int2str(rep.fact[1]));
	SetCtrlText(IDC_EDT_FACT_Z, int2str(rep.fact[2]));

	isConfigChanged = false;

	ignoreNotifications = false;
}

void WHTDialog::ReadTrackerSettings()
{
	ClearCtrlText(IDC_LBL_CALIB_STATUS);
	ClearCtrlText(IDC_LBL_GYRO_BIAS_X);
	ClearCtrlText(IDC_LBL_GYRO_BIAS_Y);
	ClearCtrlText(IDC_LBL_GYRO_BIAS_Z);
	ClearCtrlText(IDC_LBL_ACCEL_BIAS_X);
	ClearCtrlText(IDC_LBL_ACCEL_BIAS_Y);
	ClearCtrlText(IDC_LBL_ACCEL_BIAS_Z);
	SetComboSelection(IDC_CMB_RF_POWER, -1);		// deselect

	FeatRep_TrackerSettings rep;
	rep.report_id = TRACKER_SETTINGS_REPORT_ID;
	device.GetFeatureReport(rep);

	if (rep.has_tracker_responded == 0)
	{
		SetCtrlText(IDC_LBL_CALIB_STATUS, L"Tracker not found");
		isTrackerFound = false;
	} else {
		SetCtrlText(IDC_LBL_CALIB_STATUS, rep.is_calibrated ? L"Calibrated" : L"Not calibrated");

		SetCtrlText(IDC_LBL_GYRO_BIAS_X, int2str(rep.gyro_bias[0]));
		SetCtrlText(IDC_LBL_GYRO_BIAS_Y, int2str(rep.gyro_bias[1]));
		SetCtrlText(IDC_LBL_GYRO_BIAS_Z, int2str(rep.gyro_bias[2]));

		SetCtrlText(IDC_LBL_ACCEL_BIAS_X, int2str(rep.accel_bias[0]));
		SetCtrlText(IDC_LBL_ACCEL_BIAS_Y, int2str(rep.accel_bias[1]));
		SetCtrlText(IDC_LBL_ACCEL_BIAS_Z, int2str(rep.accel_bias[2]));

		// refresh if the value is not changed
		if (!isPowerChanged)
		{
			ignoreNotifications = true;

			size_t sel;
			if (rep.rf_power == CMD_RF_PWR_LOWEST)
				sel = 0;
			else if (rep.rf_power == CMD_RF_PWR_LOWER)
				sel = 1;
			else if (rep.rf_power == CMD_RF_PWR_HIGHER)
				sel = 2;
			else if (rep.rf_power == CMD_RF_PWR_HIGHEST)
				sel = 3;

			SetComboSelection(IDC_CMB_RF_POWER, sel);

			ignoreNotifications = false;
		}

		isTrackerFound = true;
	}
}

void WHTDialog::SendConfigToDevice()
{
	FeatRep_DongleSettings rep;

	rep.is_linear = GetComboSelection(IDC_CMB_AXIS_RESPONSE);
	rep.autocenter = GetComboSelection(IDC_CMB_AUTOCENTER);

	rep.fact[0] = GetCtrlTextInt(IDC_EDT_FACT_X);
	rep.fact[1] = int16_t(GetCtrlTextInt(IDC_EDT_FACT_Y) * (float)4.0);
	rep.fact[2] = int16_t(GetCtrlTextInt(IDC_EDT_FACT_Z) * (float)4.0);

	rep.report_id = DONGLE_SETTINGS_REPORT_ID;
	device.SetFeatureReport(rep);
}

bool WHTDialog::ConnectDongle()
{
	if (device.Open())
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
	bool is_connected = device.IsOpen();

	SetCtrlText(IDC_BTN_CONNECT, is_connected ? L"Disconnect" : L"Connect");

	EnableWindow(GetCtrl(IDC_BTN_CALIBRATE), is_connected ? TRUE : FALSE);
	EnableWindow(GetCtrl(IDC_BTN_MINUS), is_connected ? TRUE : FALSE);
	EnableWindow(GetCtrl(IDC_BTN_PLUS), is_connected ? TRUE : FALSE);
	EnableWindow(GetCtrl(IDC_BTN_RESET_DRIFT_COMP), is_connected ? TRUE : FALSE);
	EnableWindow(GetCtrl(IDC_BTN_SAVE_DRIFT_COMP), is_connected ? TRUE : FALSE);
	EnableWindow(GetCtrl(IDC_CMB_AXIS_RESPONSE), is_connected ? TRUE : FALSE);
	EnableWindow(GetCtrl(IDC_CMB_AUTOCENTER), is_connected ? TRUE : FALSE);
	EnableWindow(GetCtrl(IDC_EDT_FACT_X), is_connected ? TRUE : FALSE);
	EnableWindow(GetCtrl(IDC_EDT_FACT_Y), is_connected ? TRUE : FALSE);
	EnableWindow(GetCtrl(IDC_EDT_FACT_Z), is_connected ? TRUE : FALSE);
	EnableWindow(GetCtrl(IDC_CMB_RF_POWER), is_connected ? TRUE : FALSE);

	EnableWindow(GetCtrl(IDC_BTN_SAVE_AXES_SETUP), is_connected ? isConfigChanged : FALSE);
	EnableWindow(GetCtrl(IDC_BTN_SAVE_RF_POWER), is_connected ? isPowerChanged : FALSE);
}
