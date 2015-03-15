#include "stdafx.h"

#include "hid.h"
#include "dllwrap.h"

Dll hidDll;

void (__stdcall *HidD_GetHidGuid) (LPGUID HidGuid) = 0;
BOOLEAN (__stdcall *HidD_GetAttributes) (HANDLE HidDeviceObject, PHIDD_ATTRIBUTES Attributes) = 0;
BOOLEAN (__stdcall *HidD_GetFeature) (HANDLE HidDeviceObject, PVOID ReportBuffer, ULONG ReportBufferLength) = 0;
BOOLEAN (__stdcall *HidD_SetFeature) (HANDLE HidDeviceObject, PVOID ReportBuffer, ULONG ReportBufferLength) = 0;
BOOLEAN (__stdcall *HidD_GetInputReport) (HANDLE HidDeviceObject, PVOID ReportBuffer, ULONG ReportBufferLength) = 0;

bool InitHID()
{
	if (!hidDll.Load(L"hid.dll"))
	{
		MessageBox(0, L"Unable to load hid.dll", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	// get the function from the loaded DLLs
	hidDll.GetProc(HidD_GetHidGuid,		"HidD_GetHidGuid");
	hidDll.GetProc(HidD_GetAttributes,	"HidD_GetAttributes");
	hidDll.GetProc(HidD_GetFeature,		"HidD_GetFeature");
	hidDll.GetProc(HidD_SetFeature,		"HidD_SetFeature");
	hidDll.GetProc(HidD_GetInputReport,	"HidD_GetInputReport");

	if (!HidD_GetHidGuid  ||  !HidD_GetAttributes  ||  !HidD_GetFeature  ||  !HidD_SetFeature  ||  !HidD_GetInputReport)
	{
		std::wstring msg(L"Unable to load all the necessary functions from hid.dll\n\nMissing functions:");

		if (!HidD_GetHidGuid)
			msg += L"\nHidD_GetHidGuid";
			
		if (!HidD_GetAttributes)
			msg += L"\nHidD_GetAttributes";

		if (!HidD_GetFeature)
			msg += L"\nHidD_GetFeature";
			
		if (!HidD_SetFeature)
			msg += L"\nHidD_SetFeature";
			
		if (!HidD_GetInputReport)
			msg += L"\nHidD_GetInputReport";

		MessageBox(0, msg.c_str(), L"Error", MB_OK | MB_ICONERROR);

		return false;
	}

	return true;
}