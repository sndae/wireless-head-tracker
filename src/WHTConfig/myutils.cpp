#include "stdafx.h"

std::wstring int2str(const int i)
{
	const int BUFF_SIZE = 20;
	wchar_t buffer[BUFF_SIZE];
	swprintf_s(buffer, BUFF_SIZE, L"%i", i);
	return buffer;
}

std::wstring flt2str(const float f)
{
	if (f < 0.001  &&  f > -0.001)
		return L"0.0";

	const int BUFF_SIZE = 32;
	wchar_t buff[BUFF_SIZE];

	// not really rounding, but it will do
	const double rf = int(f * 1000) / 1000.0;

	swprintf_s(buff, BUFF_SIZE, L"%g", rf);

	return buff;
}
