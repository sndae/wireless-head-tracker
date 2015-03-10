#include "stdafx.h"

#include "resource.h"
#include "hid.h"
#include "wht_device.h"
#include "myutils.h"
#include "my_win.h"
#include "wht_dialog.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	try {
		// load hid.dll and the necessary functions from it
		if (!InitHID())
			return -1;

		// we need common controls for the status bar and progress bars
		//CoInitialize(0);
		//InitCommonControls();

		// create & show the app window
		WHTDialog mainDlg;

		mainDlg.CreateDlg(IDD_MAIN_DIALOG);

		// Enter the message loop
		MSG msg;
		do {
			if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0)
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			//} else {
			//	appDlg.Render();
			}
		} while (msg.message != WM_QUIT);

	} catch (std::wstring& e) {
		::MessageBox(0, e.c_str(), L"Exception", MB_OK | MB_ICONERROR);
	}

	//CoUninitialize();

	return 0;

	/*
	// load hid.dll and the necessary functions from it
	if (!InitHID())
		return -1;

	CoInitialize(0);		// we need common controls for the status bar and progress bars
	InitCommonControls();

	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN_DIALOG), 0, (DLGPROC) WHTDialog::MyDlgProc, 0);

	CoUninitialize();

	return 0;
	*/
}