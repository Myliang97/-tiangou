#include "TGLogin.h"
#include"TermApp.h"
#include <md5/MD5.h>
#include<global.h>
#include<curl/Request.h>
#include <curl/CURLClient.h>
#include<datetime/DateTime.hpp>

#define CONFIG_URL "http://%s/config.xml"
DWORD WINAPI TGLogin::SendHeartProc(LPVOID parameter) //心跳只填充CmdHeader
{
	TGLogin *login = (TGLogin*)parameter;
	while (1) 
	{
		if (login->SendHeart())
		{
			login->m_log->Trace(INFO_FORMAT, "Connect to server success");
		}
		else
		{
			login->m_log->Trace(ERROR_FORMAT, "Connect to server fail");
		}
		Sleep(HEART_TIME);
	}
	return 0;
}
bool TGLogin::SendHeart()
{
	Request req;
	req.mInfo.mCmdId = 1;
	wchar_t *wverifyId = StrLib::Utf8ToUnicode(TermApp::Instance()->GetTermInfo()->m_verifyId.c_str());
	if (wverifyId)
	{
		req.mInfo.mVerifyId = wverifyId;
		delete[]wverifyId;
	}
	req.mInfo.mTime = CDateTime::GetLocalTimeW();
	CURLClient client;
	client.init(TermApp::Instance()->GetUrl(), TermApp::Instance()->GetPort());
	std::string strxml;
	if (!RequestToXmL::RequestToXmlA(req, strxml))
	{
		m_log->Trace(ERROR_FORMAT, "RequestToXml error");
		return false;
	}
	if (client.SendData(strxml.c_str(), strxml.length()))
	{
		const char *recvbuffer = client.RecvData();   //根据服务器反馈回来的xml信息判断是否在线
		if (!ParserResponse(recvbuffer))
		{
			m_log->Trace(ERROR_FORMAT, "parserResponse fail");
			return false;
		}
	}
	else
	{
		m_log->Trace(ERROR_FORMAT, "client SendData fail");
		return false;
	}
	return m_isConnect;
}

bool TGLogin::ParserResponse(const char *xml)
{
	XmlReader reader;
	if (!reader.LoadXml(xml))
	{
		m_log->Trace(ERROR_FORMAT, "parser response xml %s error", xml);
		return false;
	}
	const char *stat = reader.SelectNode("status")->GetText();
	if (0 == stricmp(stat, "200"))
	{
		m_isConnect = true;
	}
	else
	{
		m_isConnect = false;
	}
	const char *conf = reader.SelectNode("conf")->GetText();
	if (0 == stricmp(conf, "1"))
	{
		if (!DownConf())
		{
			m_isConnect = false;
			m_log->Trace(ERROR_FORMAT, "download config.xml fail");
		}
		else
		{
			m_isConnect = true;
			m_log->Trace(INFO_FORMAT, "download config.xml success");
			TermApp::Instance()->GetConfig()->UpdateConfig();
		}		
	}
	return true;
}

bool TGLogin::DownConf()
{
	Request req;
	req.mInfo.mCmdId = 3;
	wchar_t *wverifyId = StrLib::Utf8ToUnicode(TermApp::Instance()->GetTermInfo()->m_verifyId.c_str());
	if (wverifyId)
	{
		req.mInfo.mVerifyId = wverifyId;
		delete[]wverifyId;
	}
	CURLClient client;
	client.init(TermApp::Instance()->GetUrl(), TermApp::Instance()->GetPort());
	std::string strxml;
	if (!RequestToXmL::RequestToXmlA(req, strxml))
	{
		return false;
	}
	std::string downPath = TermApp::Instance()->GetTermInfo()->m_tgDir + "\\config.bak.xml";
	if (!client.DownLoadFileByRequest(strxml.c_str(), strxml.length(), downPath.c_str()))
	{
		DeleteFileA(downPath.c_str());
		return false;
	}
	std::string confPath = TermApp::Instance()->GetTermInfo()->m_tgDir + "\\" + TG_CONFIG_PATH;
	if (!DeleteFileA(confPath.c_str()))
	{
		DeleteFileA(downPath.c_str());
		return false;
	}
	if (rename(downPath.c_str(), confPath.c_str()))
	{
		return false;
	}
	return true;
}

BOOL TGLogin::Start()
{
	Request req;
	CmdLogin login;
	TermInfo *tginfo = TermApp::Instance()->GetTermInfo();
	wchar_t *whostname = StrLib::Utf8ToUnicode(tginfo->m_hostname.c_str());
	if (whostname)
	{
		login.mHostName = whostname;
		delete[]whostname;
	}
	wchar_t *wip = StrLib::Utf8ToUnicode(tginfo->m_ip.c_str());
	if (wip)
	{
		login.mIp = wip;
		delete[]wip;
	}
	wchar_t *wmac = StrLib::Utf8ToUnicode(tginfo->m_mac.c_str());
	if (wmac)
	{
		login.mMac = wmac;
		delete[]wmac;
	}
	wchar_t *wos = StrLib::Utf8ToUnicode(tginfo->m_os.c_str());
	if (wos)
	{
		login.mOs = wos;
		delete[]wos;
	}
	wchar_t *wusername = StrLib::Utf8ToUnicode(tginfo->m_username.c_str());
	if (wusername)
	{
		login.mUserName = wusername;
		delete[]wusername;
	}
	req.mLogin = login;
	req.mInfo.mCmdId = 0;
	wchar_t *wverifyId = StrLib::Utf8ToUnicode(TermApp::Instance()->GetTermInfo()->m_verifyId.c_str());
	if (wverifyId)
	{
		req.mInfo.mVerifyId = wverifyId;
		delete[]wverifyId;
	}
	req.mInfo.mTime = CDateTime::GetLocalTimeW();
	m_verifyId = tginfo->m_verifyId;  //保存认证id发送心跳
	std::wstring wstrxml;
	if (!RequestToXmL::RequestToXmlW(req, wstrxml))
	{
		m_log->Trace(ERROR_FORMAT, "RequestToXml error");
		return FALSE;
	}
	do
	{
		CURLClient client;
		if (!client.init(TermApp::Instance()->GetUrl(), TermApp::Instance()->GetPort()))
		{
			m_log->Trace(ERROR_FORMAT, "CURL init error %d", GetLastError());
			return FALSE;
		}
		if (client.SendData(wstrxml.c_str(), wstrxml.length()))
		{
			const char *recvbuffer = client.RecvData();   //根据服务器反馈回来的xml信息判断是否在线
			if (!ParserResponse(recvbuffer))
				m_log->Trace(ERROR_FORMAT, "parserResponse fail");
		}
		else
		{
			m_log->Trace(ERROR_FORMAT, "client SendData fail");
		}
	} while (m_isConnect ==false);
	m_log->Trace(INFO_FORMAT, "Login success");
	DownConf();  //登录成功,下载一次策略，保证和页面一致
	HANDLE hd = CreateThread(NULL, NULL, TGLogin::SendHeartProc, this, NULL, NULL);
	if (hd == NULL)
	{
		m_log->Trace(ERROR_FORMAT, "CreateThread SendHeart fail");
		return FALSE;
	}
	CloseHandle(hd);
	return TRUE;
}

BOOL TGLogin::Init(std::string serverIp, int port)
{
	m_isConnect = false;
	//构建登陆日志
	char logPath[MAX_PATH];
	sprintf(logPath, "%s\\log\\Login.log", TermApp::Instance()->GetTermInfo()->m_tgDir.c_str());
	m_log = new Logger;
	if (!m_log->Init(logPath))
		return FALSE;
	m_serverIp = serverIp;
	m_port = port;
	return TRUE;
}

TGLogin::TGLogin()
{
	m_serverIp = "";
	m_port = 0;
}


TGLogin::~TGLogin()
{
}
