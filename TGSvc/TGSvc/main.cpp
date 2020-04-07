#include<Windows.h>
#include<global.h>
#include<stdio.h>
#include<register/RegLib.h>
#include<iostream>
#include<fstream>
#include"Task.h"
#define SELF_STOP_SERVICE 0x00000080   //自定义消息，终止服务
static SERVICE_STATUS_HANDLE g_serviceStatusHandle = NULL;
VOID WINAPI ServiceHandler(DWORD dwControlCode);  //服务控制程序
BOOL SetServiceStatus(DWORD status, BOOL selfStop = FALSE);

static DWORD g_dwCurrentState = 0;

VOID WINAPI ServiceMain(
	DWORD dwArgc,     // number of arguments
	LPWSTR *lpwzArgv  // array of arguments
)
{
	g_serviceStatusHandle = RegisterServiceCtrlHandlerA(SERVICE_NAME, &ServiceHandler);
	SetServiceStatus(SERVICE_START_PENDING);
	SetServiceStatus(SERVICE_RUNNING);
	Task *task = Task::Instance();
	task->StartTerm();
	do
	{
		Sleep(200);
	} while (g_dwCurrentState != SERVICE_STOPPED && g_dwCurrentState != SERVICE_START_PENDING);
}
BOOL SetServiceStatus(DWORD status, BOOL selfStop)
{
	SERVICE_STATUS serviceStatus;
	serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	serviceStatus.dwWin32ExitCode = NO_ERROR;
	serviceStatus.dwServiceSpecificExitCode = 0;
	serviceStatus.dwWaitHint = 2000;
	serviceStatus.dwCheckPoint = 0;
	serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN;  //状态控制
	if (selfStop)
	{
		serviceStatus.dwControlsAccepted |= SERVICE_ACCEPT_STOP;
	}
	serviceStatus.dwCurrentState = status;
	g_dwCurrentState = status;
	return SetServiceStatus(g_serviceStatusHandle, &serviceStatus);
}
BOOL InstallService()
{
	SC_HANDLE schSCManager = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	CHAR ImagePath[MAX_PATH] = { 0 };
	::GetModuleFileName(NULL, ImagePath, MAX_PATH);
	// 创建服务
	SC_HANDLE schService = ::CreateServiceA(
		schSCManager,               // SCManager database
		SERVICE_NAME,        // name of service
		SERVICE_DISPLAY_NAME,       // name to display
		SERVICE_ALL_ACCESS,         // desired access
		SERVICE_WIN32_SHARE_PROCESS | SERVICE_INTERACTIVE_PROCESS,    // type of service
		SERVICE_AUTO_START,         // start type (SERVICE_DEMAND_START)
		SERVICE_ERROR_NORMAL,       // error control type
		ImagePath,                // name of binary file
		SERVICE_NAME,    // name of load ordering group
		NULL,                       // tag identifier
		NULL,                       // array of dependency names
		NULL,                       // account name 
		NULL);                      // account password

									//写注册表
	CHAR newImagePath[MAX_PATH];
	sprintf_s(newImagePath, "%s /start", ImagePath);
	CHAR keyPath[MAX_PATH];
	if (schService == NULL) {
		BOOL bCreateResult = FALSE;
		DWORD dwErrorCode = ::GetLastError();
		if (dwErrorCode == ERROR_SERVICE_EXISTS) {
			sprintf(keyPath, "SYSTEM\\CurrentControlSet\\Services\\%s", SERVICE_NAME);
			if (WriteReg(HKEY_LOCAL_MACHINE, keyPath, "ImagePath", (LPBYTE)newImagePath, strlen(newImagePath), REG_EXPAND_SZ))
			{
				schService = ::OpenServiceA(schSCManager, SERVICE_NAME, SERVICE_START);
				if (schService == NULL)
				{
					::CloseServiceHandle(schSCManager);
					return FALSE;
				}
			}
		}
		if (schService == NULL) {
			::CloseServiceHandle(schSCManager);
			return FALSE;
		}
	}

	BOOL bInstallResult = FALSE;

	CHAR oldImagePath[MAX_PATH_EX] = { 0 };

	::sprintf_s(keyPath, _countof(keyPath), "SYSTEM\\CurrentControlSet\\Services\\%s", SERVICE_NAME);
	if (ReadRegVal(HKEY_LOCAL_MACHINE, keyPath, "ImagePath", (LPBYTE)oldImagePath, MAX_PATH_EX))
	{
		if (stricmp(oldImagePath, newImagePath) != 0)
		{
			WriteReg(HKEY_LOCAL_MACHINE, keyPath, "ImagePath", (LPBYTE)newImagePath, strlen(newImagePath), REG_EXPAND_SZ);
		}
	}
	schService = OpenServiceA(schSCManager,
		SERVICE_NAME,
		SERVICE_ALL_ACCESS);
	// 启动服务
	if (!::StartService(schService, 0, NULL)) {
		DWORD dwErrorCode = ::GetLastError();
		if (dwErrorCode == ERROR_SERVICE_ALREADY_RUNNING) {
			bInstallResult = TRUE;
		}
	}
	else {
		OutputDebugString("service start...");
		bInstallResult = TRUE;
	}

	::CloseServiceHandle(schService);
	::CloseServiceHandle(schSCManager);


	return bInstallResult;
}
BOOL RunService()
{
	const SERVICE_TABLE_ENTRYW serviceTable[] = {
		{ SERVICE_NAMEW, ServiceMain },
		{ NULL, NULL }
	};
	StartServiceCtrlDispatcherW(&serviceTable[0]);
	return TRUE;
}
BOOL StopService()
{
	SC_HANDLE scmHandle = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	IS_QUE_AND_RETURN(scmHandle, NULL, FALSE)

		SC_HANDLE serviceHandle = OpenServiceA(scmHandle,
			SERVICE_NAME,
			SERVICE_ALL_ACCESS);
	IS_QUE_AND_RETURN(serviceHandle, NULL, FALSE)

		BOOL ret = TRUE;
	do
	{
		SERVICE_STATUS serviceStatus;
		if (!QueryServiceStatus(serviceHandle, &serviceStatus))
		{
			ret = FALSE;
			break;
		}
		if (serviceStatus.dwCurrentState != SERVICE_STOPPED) {
			if (!ControlService(serviceHandle, SELF_STOP_SERVICE, &serviceStatus))
			{
				ret = FALSE;
				break;
			}
			do {
				if (!QueryServiceStatus(serviceHandle, &serviceStatus))
				{
					ret = FALSE;
					break;
				}
				Sleep(1000);
			} while (serviceStatus.dwCurrentState != SERVICE_STOPPED);
		}
	} while (0);
	CloseServiceHandle(scmHandle);
	CloseServiceHandle(serviceHandle);
	return ret;
}
BOOL UninstallService()
{
	SC_HANDLE scmHandle = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	IS_QUE_AND_RETURN(scmHandle, NULL, FALSE)

		SC_HANDLE serviceHandle = OpenServiceA(scmHandle,
			SERVICE_NAME,
			SERVICE_ALL_ACCESS);
	IS_QUE_AND_RETURN(serviceHandle, NULL, FALSE)

		BOOL ret = TRUE;
	do
	{
		SERVICE_STATUS serviceStatus;
		if (!QueryServiceStatus(serviceHandle, &serviceStatus))
		{
			ret = FALSE;
			break;
		}
		if (serviceStatus.dwCurrentState == SERVICE_RUNNING) {
			if (!ControlService(serviceHandle, SELF_STOP_SERVICE, &serviceStatus))
			{
				ret = FALSE;
				break;
			}
			do {
				if (!QueryServiceStatus(serviceHandle, &serviceStatus))
				{
					ret = FALSE;
					break;
				}
				Sleep(1000);
			} while (serviceStatus.dwCurrentState != SERVICE_STOPPED);
		}
		if (!DeleteService(serviceHandle))
		{
			ret = FALSE;
		}
	} while (0);
	CloseServiceHandle(scmHandle);
	CloseServiceHandle(serviceHandle);
	return ret;
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	int argc = 0;
	LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	int i = 0;
	for (; i < argc; ++i)
	{
		if (0 == wcsicmp(argv[i], L"/install"))
		{
			if (!InstallService())
			{
				std::cout << GetLastError();
				return -1;
			} 
			return 0;
		}
		if (0 == wcsicmp(argv[i], L"/start"))
		{
			if (!RunService())
			{
				std::cout << GetLastError();
				return -2;
			}
			return 0;
		}
		if (0 == wcsicmp(argv[i], L"/stop"))
		{
			Task *task = Task::Instance();
			task->Stop();
			if (!StopService())
			{
				std::cout << GetLastError();
				return -3;
			}
			return 0;
		}
		if (0 == wcsicmp(argv[i], L"/uninstall"))
		{
			if (!UninstallService())
			{
				std::cout << GetLastError();
				return -4;
			}
			return 0;
		}
	}
	return 0;
}
VOID WINAPI ServiceHandler(DWORD dwControlCode)
{
	switch (dwControlCode)
	{
	case SERVICE_CONTROL_CONTINUE:
		SetServiceStatus(SERVICE_START_PENDING);    break;
	case SERVICE_CONTROL_INTERROGATE:
		break;
	case SERVICE_CONTROL_PAUSE:
		SetServiceStatus(SERVICE_PAUSED);           break;
	case SERVICE_CONTROL_SHUTDOWN:
		SetServiceStatus(SERVICE_STOPPED);          break;
	case SERVICE_CONTROL_STOP:
		SetServiceStatus(SERVICE_STOPPED);          break;
	case SELF_STOP_SERVICE:
		SetServiceStatus(SERVICE_STOPPED, TRUE);    break;
	default:
		break;
	}
}