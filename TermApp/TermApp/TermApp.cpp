#include "TermApp.h"
#include"InitHook.h"
#include<global.h>
#include<stdio.h>
#include "Pipe/PipeServer.h"
#include<file/FileLib.h>
#include "SensitiveCheck.h"
#include"MatchRule.h"
#include<mutex>


#define SERVER_URL "http://%s/TgSvc.php"  //服务器接收post数据url

TermApp *TermApp::m_App = NULL;
TermApp * TermApp::Instance()
{
	if (m_App == NULL)
	{
		m_App = new TermApp;
	}
	return m_App;
}
BOOL TermApp::Init()  //初始化TermApp
{
	//初始化终端信息
	m_termInfo = new TermInfo();
	if (!m_termInfo->Init())
	{
		TGDebug::debug(DEBUG_STRING, "terminfo init fail");
		return FALSE;
	}
	//初始化日志
	CHAR logPath[MAX_PATH];
	sprintf(logPath, "%s\\log", m_termInfo->m_tgDir.c_str());
	if (!IsDirectoryExistsA(logPath))
	{
		CreateDirectoryA(logPath, NULL);
	}
	sprintf(logPath, "%s\\%s.log", logPath, m_termInfo->m_username.c_str());
	m_log = new Logger;
	if (!m_log->Init(logPath))
	{
		TGDebug::debug(DEBUG_STRING, "log init fail");
		return FALSE;
	}
	//读取config
	m_config = new TGConfig;
	if (!m_config->UpdateConfig())
	{
		return FALSE;
	}
	//构建url
	char buff[MAX_PATH];
	memset(buff, 0, sizeof(buff));
	char *ip = StrLib::UnicodeToUtf8(m_config->GetConfigData()->m_server.m_ip.c_str());
	if (ip)
	{
		sprintf(buff, SERVER_URL, ip);
		m_url = buff;
	}
	//登陆初始化
	m_login = new TGLogin();
	if (!m_login->Init(ip,m_config->GetConfigData()->m_server.m_port))
	{
		delete[]ip;
		return FALSE;
	}
	if (ip)
	{
		delete[]ip;
	}
	//终止进程管道服务器启动
	m_stopPipe = new PipeServer(STOP_TERMAPP_PIPE_NAME, sizeof(ExECmd), sizeof(ExECmd), 50);
	m_stopPipe->SetProcessClientThread(TermApp::StopProc, this);
	m_stopPipe->Start();

	//启动tcp服务器接收hook日志
	m_tcpHookServer = new TcpServer;
	if (!m_tcpHookServer->Init(HOOK_TCP_PORT, TermApp::ListenTcpProc, this))
	{
		m_log->Trace(ERROR_FORMAT, "tcp server init fail %d", GetLastError());
		return FALSE;
	}
	m_tcpHookServer->Start();
	m_tip = new TGTip;
	return TRUE;
}

BOOL TermApp::Start()
{
	m_log->Trace(INFO_FORMAT, "------------TianGou start------------");
	m_log->Trace(INFO_FORMAT, "ip:%s,mac:%s,hostname:%s,os:%s,verifyId:%s", m_termInfo->m_ip.c_str(),
		m_termInfo->m_mac.c_str(),m_termInfo->m_hostname.c_str(), m_termInfo->m_os.c_str(), m_termInfo->m_verifyId.c_str());
	//开始登陆
	if (!m_login->Start())
	{
		m_log->Trace(ERROR_FORMAT, "login error");
	}
	//开启上报数据队列
	m_reqQueProc = new RequestQueue;
	if (!m_reqQueProc->Start())
	{
		m_log->Trace(ERROR_FORMAT, "RequestQueue start error");
	}

	//开始监控
	if (StartHook() == FALSE)
	{
		m_log->Trace(ERROR_FORMAT, "start hook error");
		return FALSE;
	}
	return TRUE;
}
TGConfig *TermApp::GetConfig()
{
	return m_config;
}

DWORD WINAPI TermApp::StopProc(LPVOID lParameter)
{
	PIPE_INFO *pInfo = (PIPE_INFO*)lParameter;
	if (!pInfo)
	{
		return 0;
	}
	HANDLE hd = pInfo->hd;
	TermApp *app = (TermApp*)pInfo->parameter;
	if (!app)
	{
		return 0;
	}
	delete pInfo;
	ExECmd cmd;
	PipeServer::ReadFromPipe(hd, &cmd, sizeof(ExECmd));
	PipeServer::ClosePipe(hd);
	if (cmd.cmdType == ExeCmdType::STOP_TERMAPP)
	{
		app->GetLogger()->Trace(INFO_FORMAT, "stop TermApp ...");
		if (!StopHook())
		{
			app->GetLogger()->Trace(ERROR_FORMAT, "stop hook error %d", GetLastError());
		}
		TermApp::Release();  //释放内存
		HANDLE termHd = GetCurrentProcess();
		return TerminateProcess(termHd, 0);
	}
	return 0;
}
DWORD WINAPI TermApp::ListenTcpProc(LPVOID lParameter)
{
	TcpInfo *tcpInfo = (TcpInfo *)lParameter;
	if (NULL == tcpInfo)
	{
		return 0;
	}
	TermApp *app = (TermApp *)tcpInfo->lParameter;
	SOCKET sock = tcpInfo->sock;
	if (!app || tcpInfo->sock == INVALID_SOCKET)
	{
		return 0;
	}
	delete tcpInfo;
	HOOK_MESSAGE msg;
	memset(&msg, 0, sizeof(HOOK_MESSAGE));
	TcpServer::ReadFromSock(sock, &msg, sizeof(HOOK_MESSAGE));
	WCHAR *buffer = NULL;
	int wLen = 0;
	if (msg.dataLength)
	{
		wLen = msg.dataLength / 2 + 1;
		buffer = new WCHAR[wLen];
		wmemset(buffer, 0, wLen);
		TcpServer::ReadFromSock(sock, buffer, msg.dataLength);
	}

	HOOK_MESSAGE_RET ret;
	memset(&ret, 0, sizeof(HOOK_MESSAGE_RET));
	if (msg.hookType == HOOK_TYPE::HOOK_TYPE_OPEN_FILE || msg.hookType == HOOK_TYPE::HOOK_TYPE_DELETE_FILE) 
	{
		MatchRule rule;
		if (IsSecretFileW(msg.wzlpFileName,rule))
		{
			TermApp::Instance()->GetLogger()->TraceW(INFO_FORMAT, L"file:%s -- is Sensitived", msg.wzlpFileName);
			app->m_reqQueProc->AddRequest(msg.hookType, msg.wzlpFileName, msg.processName,&rule,NULL,1);
			app->m_tip->AddStr(msg.wzlpFileName,(int)msg.hookType);
		}
		TcpServer::WriteToSock(sock, &ret, sizeof(HOOK_MESSAGE_RET));
		
	}
	else if (msg.hookType == HOOK_TYPE::HOOK_TYPE_COPY_FILE || msg.hookType == HOOK_TYPE::HOOK_TYPE_MOVE_FILE || msg.hookType == HOOK_TYPE_RENAME_FILE)
	{
		TcpServer::WriteToSock(sock, &ret, sizeof(HOOK_MESSAGE_RET));
		MatchRule rule;
		if (IsSecretFileW(msg.wzlparameter,rule))
		{
			TermApp::Instance()->GetLogger()->TraceW(INFO_FORMAT, L"file:%s -- is Sensitived", msg.wzlpFileName);
			app->m_reqQueProc->AddRequest(msg.hookType, msg.wzlpFileName, msg.processName,&rule, msg.wzlparameter,1);
		}
	}
	else if (msg.hookType == HOOK_TYPE::HOOK_TYPE_PRINT_DATA) //敏感数据打印
	{
		if (buffer)
		{
			MatchRule rule;
			BOOL isSensitive = FALSE;
			CheckDataMatchRule(buffer, rule, isSensitive); //敏感数据打印，不上报
			delete[]buffer;
			buffer = NULL;
			ret.nExtern = isSensitive ?1:0;
			if(isSensitive)
				TermApp::Instance()->GetLogger()->TraceW(INFO_FORMAT, L"print sensitive data");
		}
		TcpServer::WriteToSock(sock, &ret, sizeof(HOOK_MESSAGE_RET));
	}
	return 0;
}
TermApp::TermApp()
{
	m_log=NULL;
	m_termInfo=NULL; 
	m_config = NULL;
	m_log = NULL;
	m_stopPipe = NULL;
	m_tcpHookServer = NULL;
	m_reqQueProc = NULL;
	m_tip = NULL;
}


TermApp::~TermApp()
{
	if (m_log)
	{
		delete m_log;
		m_log = NULL;
	}
	if (m_termInfo)
	{
		delete m_termInfo;
		m_termInfo = NULL;
	}
	if (m_config)
	{
		delete m_config;
		m_config = NULL;
	}
	if (m_login)
	{
		delete m_login;
		m_login = NULL;
	}
	if (m_stopPipe)
	{
		delete m_stopPipe;
		m_stopPipe = NULL;
	}
	if (m_tcpHookServer)
	{
		delete m_tcpHookServer;
		m_tcpHookServer = NULL;
	}
	if (m_reqQueProc)
	{
		delete m_reqQueProc;
		m_reqQueProc = NULL;
	}
	if (m_tip)
	{
		delete m_tip;
		m_tip = NULL;
	}
}
