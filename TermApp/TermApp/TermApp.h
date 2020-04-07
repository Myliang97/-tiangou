#pragma once
#include<Windows.h>
#include<pipe/PipeServer.h>
#include"TermInfo.h"
#include<log/DebugString.hpp>
#include<log/Logger.h>
#include"TGConfig.h"
#include "TGLogin.h"
#include<tcp/TcpServer.h>
#include "RequestQueue.h"
class TermApp
{
public:
	TermApp();
	~TermApp();
	static TermApp * Instance();
	BOOL Init();
	BOOL Start();
	TGConfig *GetConfig();
	TermInfo *GetTermInfo() { return m_termInfo; }
	Logger *GetLogger() { return m_log; }
	static DWORD WINAPI StopProc(LPVOID lParameter);
	static void Release() {
		TermApp *app = TermApp::Instance();
		if (app)
		{
			delete app;
		}
	}
	static DWORD WINAPI ListenTcpProc(LPVOID lParameter);
	const char * GetUrl() { return m_url.c_str(); }
	int GetPort() { return m_config->GetConfigData()->m_server.m_port; }
private:
	static TermApp * m_App;
	Logger *m_log;
	TermInfo *m_termInfo;
	TGConfig *m_config;
	TGLogin *m_login;
	PipeServer *m_stopPipe;
	TcpServer *m_tcpHookServer;
	RequestQueue *m_reqQueProc;
	std::string m_url;
};

