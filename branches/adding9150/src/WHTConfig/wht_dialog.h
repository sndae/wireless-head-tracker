#pragma once

// undefine to enable minimize to tray
//#define MINIMIZE_TO_TRAY

class WHTDialog: public Dialog
{
private:
	HICON		_hIconSmall;
	HICON		_hIconBig;
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

	void InitStatusbar();
	void SetStatusbarText(int part, const std::wstring& text)
	{
		SendMessage(GetCtrl(IDC_STATUS_BAR), SB_SETTEXT, part, (LPARAM) text.c_str());
	}

	void SetCtrlText(int ctrl_id, const std::wstring& text)
	{
		SendMessage(GetCtrl(ctrl_id), WM_SETTEXT, 0, (LPARAM) text.c_str());
	}

	void SetCtrlTextFloat(int ctrl_id, float flt)	{ SetCtrlText(ctrl_id, flt2str(flt)); }
	void SetCtrlTextInt(int ctrl_id, int val)		{ SetCtrlText(ctrl_id, int2str(val)); }

	void ClearCtrlText(int ctrl_id)
	{
		SetCtrlText(ctrl_id, L"");
	}

	float GetCtrlTextFloat(int ctrl_id);
	int16_t GetCtrlTextInt(int ctrl_id);

	void AddComboString(int ctrl_id, const wchar_t* str)
	{
		SendMessage(GetCtrl(ctrl_id), CB_ADDSTRING, 0, (LPARAM) str);
	}

	void SetComboSelection(int ctrl_id, int selection)
	{
		SendMessage(GetCtrl(ctrl_id), CB_SETCURSEL, (WPARAM) selection, 0);
	}

	int GetComboSelection(int ctrl_id)
	{
		return SendMessage(GetCtrl(ctrl_id), CB_GETCURSEL, 0, 0);
	}

	void SetCheckState(int ctrl_id, bool new_state)
	{
		CheckDlgButton(_hWnd, ctrl_id, new_state ? BST_CHECKED : BST_UNCHECKED);
	}

	bool GetCheckState(int ctrl_id)
	{
		return IsDlgButtonChecked(_hWnd, ctrl_id) == BST_CHECKED;
	}

#ifdef MINIMIZE_TO_TRAY

	void CreateTrayIcon();
	void RemoveTrayIcon();
	void OnTrayNotify(LPARAM lParam);
	void OnMinimize();

	void Hide()
	{
		ShowWindow(hDialog, SW_HIDE);
	}

	void Show()
	{
		ShowWindow(hDialog, SW_SHOW);
	}

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
};
