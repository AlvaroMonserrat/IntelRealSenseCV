//SampleDll.cpp

#include "stdafx.h"
#define EXPORTING_DLL
#include "SampleDll.h"

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	return TRUE;
}

void HelloWorld()
{
	MessageBox(NULL, TEXT("Hello World"), TEXT("In a DLL"), MB_OK);
}