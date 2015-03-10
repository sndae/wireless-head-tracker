#include "stdafx.h"

#include "my_win.h"

LRESULT CALLBACK Dialog::DialogProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Dialog* pDlg = (Dialog*)(::GetWindowLong(hwnd, GWL_USERDATA));

	try {
		if (message == WM_INITDIALOG)
		{
			// get the dialog<> pointer as the creation param
			pDlg = reinterpret_cast<Dialog*> (lParam);
			pDlg->_hWnd = hwnd;

			// set the dialog pointer
			::SetWindowLong(hwnd, GWL_USERDATA, (LONG) pDlg);

			pDlg->OnInit();		// send the init dialog message

			return TRUE;

		} else if (pDlg == 0) {
			return FALSE;
		}

		switch (message)
		{
		case WM_CLOSE:
			pDlg->OnClose();
			return TRUE;
		case WM_DESTROY:
			pDlg->OnDestroy();

			// clear the dialog pointer
			::SetWindowLong(hwnd, GWL_USERDATA, 0);

			return TRUE;
		case WM_MOUSEMOVE:
			pDlg->OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
			break;
		case WM_LBUTTONDOWN:
			pDlg->OnLButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
			break;
		case WM_LBUTTONUP:
			pDlg->OnLButtonUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
			break;
		case WM_SIZE:
			pDlg->OnSize(LOWORD(lParam), HIWORD(lParam), wParam);
			break;
		case WM_TIMER:
			pDlg->OnTimer(wParam);
			break;
		case WM_MOUSEWHEEL:
			pDlg->OnMouseWheel(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), GET_WHEEL_DELTA_WPARAM(wParam), wParam);
			return TRUE;
		case WM_COMMAND:

			if (HIWORD(wParam) == 0  &&  lParam == 0)
				pDlg->OnMenu(LOWORD(lParam));
			else if (HIWORD(wParam) == 1  &&  lParam == 0)
				pDlg->OnAccelerator(LOWORD(lParam));
			else
				pDlg->OnControl(LOWORD(wParam), HIWORD(wParam), (HWND) lParam);

			return TRUE;
		}

	} catch (std::wstring& e) {
		::MessageBox(hwnd, e.c_str(), L"Exception", MB_OK | MB_ICONERROR);
	}

	return FALSE;
}

bool Dialog::CreateDlg(int dlgID, HWND hWndParent)
{
	HWND hDlg = ::CreateDialogParam(::GetModuleHandle(0),
									MAKEINTRESOURCE(dlgID),
									hWndParent,
									(DLGPROC) DialogProcedure,
									(LPARAM) this);

	return hDlg != NULL;
}
