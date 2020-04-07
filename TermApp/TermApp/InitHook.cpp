#include "InitHook.h"
#include<stdio.h>
typedef BOOL(*HookDef)();
static HMODULE hModule = NULL;
static HookDef ProcStartHook = NULL;
static HookDef ProcStopHook = NULL;
BOOL InitHook()
{
	do
	{
		hModule = LoadLibraryA("TGHook.dll");
		if (!hModule)
			break;
		ProcStartHook = (HookDef)GetProcAddress(hModule, "StartHook");
		if (ProcStartHook == NULL)
			break;
		ProcStopHook = (HookDef)GetProcAddress(hModule, "StopHook");
		if (ProcStopHook == NULL)
		{
			break;
		}
		return TRUE;
	} while (0);
	char buff[30];
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "init hook fail:%d", GetLastError());
	OutputDebugString(buff);
	return FALSE;
}
BOOL StartHook()
{
	if (!InitHook())
		return FALSE;
	if (ProcStartHook)
	{
		return ProcStartHook();
	}
	return FALSE;
}

BOOL StopHook()
{
	if (ProcStopHook)
	{
		return ProcStopHook();
	}
	return FALSE;
}