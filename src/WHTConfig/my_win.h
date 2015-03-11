#pragma once

// this is a very light weight windows class library
// only the stuff I actually use in this project

#define WM_TRAYNOTIFY		(WM_APP+1)

class Window
{
protected:
	HWND		_hWnd;

public:
	explicit Window(HWND hWnd)
		: _hWnd(hWnd)
	{}

	Window()
		: _hWnd(0)
	{}

	void SetHandle(HWND hWnd)
	{
		_hWnd = hWnd;
	}

	bool IsValid() const		{ return _hWnd != 0; }

	bool SetText(const wchar_t* str)
	{
		return ::SetWindowText(_hWnd, str) == TRUE;
	}

	bool SetText(const std::wstring& str)
	{
		return SetText(str.c_str());
	}

	bool SetText(const int val)
	{
		return SetText(int2str(val));
	}

	bool SetText(const float val)
	{
		return SetText(flt2str(val));
	}

	bool ClearText()
	{
		return SetText(L"");
	}

	std::wstring GetText()
	{
		const int BUFF_SIZE = 2048;		// should be enough, i guess...
		wchar_t buff[BUFF_SIZE];

		int bytes_copied = ::GetWindowText(_hWnd, buff, BUFF_SIZE);

		return buff;
	}

	float GetFloat()
	{
		return (float) _wtof(GetText().c_str());
	}

	int16_t GetInt()
	{
		return (int16_t) _wtoi(GetText().c_str());
	}
};

class ComboBox: public Window
{
public:
	explicit ComboBox(HWND hWnd)
		: Window(hWnd)
	{}

	ComboBox()
	{}

	void AddString(const wchar_t* str)
	{
		::SendMessage(_hWnd, CB_ADDSTRING, 0, (LPARAM) str);
	}

	void SetSelection(int selection)
	{
		::SendMessage(_hWnd, CB_SETCURSEL, (WPARAM) selection, 0);
	}

	int GetSelection()
	{
		return ::SendMessage(_hWnd, CB_GETCURSEL, 0, 0);
	}
};


class CheckBox: public Window
{
public:
	explicit CheckBox(HWND hWnd)
		: Window(hWnd)
	{}

	CheckBox()
	{}

	void SetState(bool is_checked)
	{
		::SendMessage(_hWnd, BM_SETCHECK, is_checked ? BST_CHECKED : BST_UNCHECKED, 0);
	}

	bool IsChecked()
	{
		return ::SendMessage(_hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED;
	}
};

class ProgressBar: public Window
{
public:
	ProgressBar()
	{}

	explicit ProgressBar(HWND hWnd)
		: Window(hWnd)
	{}

	void SetRange(uint16_t min, uint16_t max)
	{
		::SendMessage(_hWnd, PBM_SETRANGE, 0, MAKELPARAM(min, max));
	}

	void SetPos(uint16_t pos)
	{
		::SendMessage(_hWnd, PBM_SETPOS, (WPARAM) pos, 0);
	}
};

class StatusBar: public Window
{
public:
	StatusBar()
	{}

	explicit StatusBar(HWND hWnd)
		: Window(hWnd)
	{}

	void SetParts(const int* parts, int num_parts)
	{
		::SendMessage(_hWnd, SB_SETPARTS, num_parts, (LPARAM) parts);
	}

	void SetPartText(int part, const std::wstring& txt)
	{
		::SendMessage(_hWnd, SB_SETTEXT, part, (LPARAM) txt.c_str());
	}
};

class Dialog: public Window
{
protected:

	// the dialog procedure
	static LRESULT CALLBACK DialogProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

public:
	Dialog()
		: Window(0)
	{}

	virtual ~Dialog()	{}

	bool CreateDlg(int dlgID, HWND hWndParent = NULL);
	HWND GetCtrl(int ctrlID)
	{
		return ::GetDlgItem(_hWnd, ctrlID);
	}

	void SetCtrlText(int ctrl_id, const std::wstring& text)
	{
		Window ctrl(GetCtrl(ctrl_id));
		ctrl.SetText(text);
	}

	void Hide()
	{
		::ShowWindow(_hWnd, SW_HIDE);
	}

	void Show()
	{
		::ShowWindow(_hWnd, SW_SHOW);
	}

	virtual void OnInit()					{}
	virtual void OnTimer(int timerID)		{}
	virtual void OnClose()					{ ::DestroyWindow(_hWnd); }
	virtual void OnDestroy()				{}
	virtual void OnSize(int width, int height, WPARAM wParam)				{}
	virtual void OnMouseMove(int x, int y, WPARAM wParam)					{}
	virtual void OnLButtonDown(int x, int y, WPARAM wParam)					{}
	virtual void OnLButtonUp(int x, int y, WPARAM wParam)					{}
	virtual void OnMouseWheel(int x, int y, int delta, WPARAM wParam)		{}
	virtual void OnControl(int ctrlID, int notifyID, HWND hWndCtrl)			{}
	virtual void OnMenu(int menuID)											{}
	virtual void OnAccelerator(int acceleratorID)							{}
	virtual void OnSysCommand(WPARAM wParam)								{}
	virtual void OnTrayNotify(LPARAM lParam)								{}
};

class WaitCursor
{
protected:
	HCURSOR		_prev_cursor;

public:
	WaitCursor()
	{
		_prev_cursor = ::GetCursor();
		::SetCursor(::LoadCursor(NULL, IDC_WAIT));
	}

	void Restore()
	{
		if (_prev_cursor != 0)
		{
			::SetCursor(_prev_cursor);
			_prev_cursor = 0;
		}
	}

	~WaitCursor()
	{
		Restore();
	}
};
