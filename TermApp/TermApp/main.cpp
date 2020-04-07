#include"TermApp.h"
#include"InitHook.h"
#include<Dbghelp.h>
#include<global.h>
#include<stdio.h>
#include<sysLib/sys.h>
#include<register/RegLib.h>
#include<windows.h>

DWORD curProcId = 0;

bool InitTipWindow()
{

}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	curProcId = GetCurrentProcessId();
	if (!IsKeyExists(HKEY_CURRENT_USER, TG_REG_ROOT_PATH))
	{
		if (!CreateRegKey(HKEY_CURRENT_USER, TG_REG_ROOT_PATH))
		{
			return -1;
		}
	}
	DWORD sessionId = GetCurrentSessionId();
	char mutexStr[50];
	sprintf(mutexStr, TG_USER_MUTEX, sessionId);
	HANDLE mutexHd = CreateMutexA(NULL, FALSE, mutexStr);
	WriteReg(HKEY_CURRENT_USER, TG_REG_ROOT_PATH, TG_REG_USER_PID, (LPBYTE)&curProcId, sizeof(DWORD), REG_DWORD);
	DWORD fn = 0;
	WriteReg(HKEY_CURRENT_USER, TG_REG_ROOT_PATH, TG_REG_OFFICE_END, (LPBYTE)&fn, sizeof(DWORD), REG_DWORD);
	TermApp *app = TermApp::Instance();
	if (app->Init())
	{
		if (app->Start())
		{
			MSG msg;
			while (GetMessage(&msg, NULL, 0, 0)) //从消息队列中取出消息
			{
				TranslateMessage(&msg); //转化消息
				DispatchMessage(&msg);  //分发消息
			}
		}
	}
	return -1;
}