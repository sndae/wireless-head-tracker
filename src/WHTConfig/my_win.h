#pragma once

// this is a very light weight windows class library
// only the stuff I actually use in this project

class Window
{
protected:
	HWND		_hWnd;

public:
	Window(HWND hWnd)
		: _hWnd(hWnd)
	{}

	bool IsValid() const		{ return _hWnd != 0; }
};

class Dialog: public Window
{
protected:
	// HWND for our dialog

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
