#pragma once

// this is a very light weight windows class library
// only the stuff I actually use in this project

#define WM_TRAYNOTIFY		(WM_APP+1)

// only icons from resources are supported
class Icon
{
private:
	HICON		_hIcon;
	bool		_is_large;
public:
	Icon(int iconID, bool is_large);

	bool IsLarge() const	{ return _is_large; }

	HICON GetHandle() const
	{
		return _hIcon;
	}
};

class Window
{
protected:
	HWND		_hWnd;

public:
	Window()
		: _hWnd(0)
	{}

	void SetHandle(HWND hWnd)	{ _hWnd = hWnd; }
	HWND GetHandle() const		{ return _hWnd; }

	bool IsValid() const		{ return _hWnd != 0; }
	void GetRect(RECT& r) const
	{
		::GetWindowRect(_hWnd, &r);
	}

	bool SetText(const wchar_t* str)
	{
		return ::SetWindowText(_hWnd, str) == TRUE;
	}

	bool SetText(const std::wstring& str)
	{
		return SetText(str.c_str());
	}

	bool ClearText()
	{
		return SetText(L"");
	}

	std::wstring GetText();

	bool Enable(bool should_enable)
	{
		return ::EnableWindow(_hWnd, should_enable ? TRUE : FALSE) == 0;
	}

	// the int and float variants are shortcuts -- we use these a lot
	bool SetText(const int val)
	{
		return SetText(int2str(val));
	}

	bool SetText(const float val)
	{
		return SetText(flt2str(val));
	}

	float GetFloat()
	{
		return (float) _wtof(GetText().c_str());
	}

	int GetInt()
	{
		return _wtoi(GetText().c_str());
	}

	int16_t GetInt16()
	{
		return (int16_t) GetInt();
	}
};

class ComboBox: public Window
{
public:
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
	void SetParts(const int* parts, int num_parts)
	{
		::SendMessage(_hWnd, SB_SETPARTS, num_parts, (LPARAM) parts);
	}

	void SetPartText(int part, const std::wstring& txt)
	{
		::SendMessage(_hWnd, SB_SETTEXT, part, (LPARAM) txt.c_str());
	}
};

// at the moment, we can't really do anything with these simple controls that we can not do with a base window
// but I will keep them anyway
class Button: public Window		{};
class Label: public Window		{};

class Dialog: public Window
{
protected:

	// the dialog procedure
	static LRESULT CALLBACK DialogProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

public:
	virtual ~Dialog()	{}

	bool CreateDlg(int dlgID, HWND hWndParent = NULL);

	HWND GetCtrl(int ctrlID)
	{
		return ::GetDlgItem(_hWnd, ctrlID);
	}

	void Hide()
	{
		::ShowWindow(_hWnd, SW_HIDE);
	}

	void Show()
	{
		::ShowWindow(_hWnd, SW_SHOW);
	}

	void MsgBox(const std::wstring& msg, const std::wstring& title, const UINT boxtype)
	{
		::MessageBox(_hWnd, msg.c_str(), title.c_str(), boxtype);
	}

	void SetIcon(const Icon& ic);

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
	virtual bool OnSysCommand(WPARAM wParam)								{ return false; }
	virtual void OnTrayNotify(LPARAM lParam)								{}

	// pure virtual - the derived class MUST handle exceptions
	virtual void OnException(const std::wstring& str) = 0;
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

// i never liked the std C++ stream library...
class SimpleFile
{
protected:
	HANDLE	_hFile;

public:
	SimpleFile()
		: _hFile(INVALID_HANDLE_VALUE)
	{}

	~SimpleFile()
	{
		Close();
	}

	bool Open(const std::wstring& fname, bool for_write, bool trunc = false);

	void Close()
	{
		if (_hFile != INVALID_HANDLE_VALUE)
			::CloseHandle(_hFile);

		_hFile = NULL;
	}

	int Read(char* buff, const int num_bytes);
	void Write(char* buff, const int num_bytes);
};

class OpenSaveFileDialog
{
private:
	OPENFILENAME				_ofname;
	std::vector<std::wstring>	_filters;
	std::wstring				_default_file_name;
	std::wstring				_default_path;
	std::wstring				_selected_full_file_name;
	std::wstring				_selected_file_name;

	bool Run(const std::wstring& window_title, const Window& winOwner, bool runSave);
	
public:
	OpenSaveFileDialog()
	{
		::memset(&_ofname, 0, sizeof(_ofname));
		_ofname.lStructSize = sizeof(_ofname);
	}

	void AddFilter(const std::wstring& desc, const std::wstring& filter)
	{
		_filters.push_back(desc);
		_filters.push_back(filter);
	}

	void SetDefaultFileName(const std::wstring& fname)
	{
		_default_file_name = fname;
	}

	void SetDefaultPath(const std::wstring& path)
	{
		_default_path = path;
	}

	bool GetSaveFile(const std::wstring& window_title, const Window& winOwner)
	{
		return Run(window_title, winOwner, true);
	}

	bool GetOpenFile(const std::wstring& window_title, const Window& winOwner)
	{
		return Run(window_title, winOwner, false);
	}

	const std::wstring& GetFileName() const			{ return _selected_file_name; }
	const std::wstring& GetFullFileName() const		{ return _selected_full_file_name; }
};
