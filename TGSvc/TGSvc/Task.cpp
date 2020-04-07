#include"Task.h"
#include<sysLib/sys.h>
#include<string>
#include<sysLib/mutex.h>
#include<register/RegLib.h>
#include<Pipe/PipeClient.h>


#define  MAX_SESSION_COUNT 64
Task *Task::m_Task = NULL;
std::mutex m_mutex;
BOOL userInit = FALSE;
Task * Task::Instance()
{
	if (m_Task == NULL)
	{
		m_Task = new Task;
	}
	return m_Task;
}

DWORD WINAPI Task::GuardTaskThreadProc(LPVOID lpParameter)  //目前防杀进程只是重启启动，未添加其他功能
{
	OutputDebugString("GuardTaskThreadProc start...");
	while (1)
	{
		DWORD isEnd = 0;
		ReadRegVal(HKEY_LOCAL_MACHINE, TG_REG_ROOT_PATH, TG_REG_END_SYSTEM, (LPBYTE)&isEnd, sizeof(DWORD));
		if (isEnd !=0)
		{ 
			break;
		}
		char dirPath[240];
		memset(dirPath, 0, sizeof(dirPath));
		GetModuleDir(NULL, dirPath, 240);
		std::string appPath = dirPath;
		appPath.append("\\TermApp.exe");
		WTS_SESSION_INFO *pSessionInfo = NULL;
		DWORD dwSessionCount = 0;
		BOOL bEnumResult = WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pSessionInfo, &dwSessionCount);
		if (bEnumResult == FALSE)
		{
			return FALSE;
		}
		for (DWORD i = 0; i < dwSessionCount; i++) {
			WTS_SESSION_INFO *si = &pSessionInfo[i];
			if (si->State != WTSActive) {
				continue;
			}
			char user_mutex[50];
			sprintf(user_mutex, TG_USER_MUTEX, si->SessionId);
			if (!IsValidMutex(user_mutex))  //查询user进程是否存在
			{
				if (userInit == TRUE)
				{
					BOOL bCreateResult = CreateProcessAsUserWithSessionId((LPSTR)appPath.c_str(), NULL, si->SessionId);
					if (bCreateResult == FALSE)
					{
						return FALSE;
					}
				}
				else
				{
					BOOL bCreateResult = CreateProcessAsUserWithSessionId((LPSTR)appPath.c_str(), NULL, si->SessionId);
					if (bCreateResult == FALSE)
					{
						return FALSE;
					}
					else
					{
						m_mutex.lock();
						userInit = TRUE;
						m_mutex.unlock();
					}
				}
			}

		}
		Sleep(2000);
	}
	return 0;
}
BOOL Task::StartTerm()
{
	if (!IsKeyExists(HKEY_LOCAL_MACHINE, TG_REG_ROOT_PATH))
	{
		if (!CreateRegKey(HKEY_LOCAL_MACHINE, TG_REG_ROOT_PATH))
		{
			return -1;
		}
	}
	int isEnd = 0;
	WriteReg(HKEY_LOCAL_MACHINE, TG_REG_ROOT_PATH, TG_REG_END_SYSTEM, (LPBYTE)&isEnd, sizeof(DWORD), REG_DWORD);
	HANDLE hd = CreateThread(NULL, NULL, GuardTaskThreadProc,NULL, NULL, NULL);
	CloseHandle(hd);
	return TRUE;
}
//把终止命令通过管道写到TG进程，终止进程
void Task::Stop()
{
	int isEnd = 1;
	WriteReg(HKEY_LOCAL_MACHINE, TG_REG_ROOT_PATH, TG_REG_END_SYSTEM, (LPBYTE)&isEnd, sizeof(DWORD), REG_DWORD);
	PipeClient pClient(STOP_TERMAPP_PIPE_NAME);
	ExECmd cmd;
	cmd.cmdType = ExeCmdType::STOP_TERMAPP;
	if (pClient.ConnectToServer())
	{
		if (pClient.WriteToPipe(&cmd, sizeof(ExECmd)))
		{
			char buff[MAX_PATH];
			sprintf(buff, "write to pipe error:%d", GetLastError());
			OutputDebugString(buff);
		}
	}
	Sleep(1000);
}
Task::Task()
{
}


Task::~Task()
{
}
