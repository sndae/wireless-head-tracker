#include "stdafx.h"

#include "resource.h"
#include "hid.h"
#include "wht_device.h"
#include "myutils.h"
#include "wht_dialog.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	// load hid.dll and the necessary functions from it
	if (!InitHID())
		return -1;

	CoInitialize(0);		// we need common controls for the status bar and progress bars
	InitCommonControls();

	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN_DIALOG), 0, (DLGPROC) WHTDialog::MyDlgProc, 0);

	CoUninitialize();

	return 0;
}