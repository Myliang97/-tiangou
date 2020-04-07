#pragma once
#include<Windows.h>
#include<string>
#include<log/Logger.h>
#define HEART_TIME 20000

/*
*	Login:
*	1.负责首次登陆的验证
*	2.发送心跳，并取策略
*/
class TGLogin
{
public:
	BOOL Init(std::string serverIp,int port);
	BOOL Start();
	static DWORD WINAPI SendHeartProc(LPVOID);
	bool SendHeart();
	bool IsOnLine() { return m_isConnect; }
	TGLogin();
	~TGLogin();
private:
	bool ParserResponse(const char *xml);
	bool DownConf();
	std::string m_verifyId;
	bool m_isConnect;
	std::string m_serverIp;
	int m_port;
	Logger *m_log;
};


