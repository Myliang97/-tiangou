#pragma once
#include<Windows.h>
#include<strsafe.h>
#include<string>
//TermInfo获取终端的ip，mac，计算机名，系统
using namespace std;
class TermInfo
{
public:
	string m_ip;
	string m_mac;
	string m_hostname;  //计算机用户名
	string m_os;
	string m_verifyId;  //mac，cpuid散列作为和服务器之间的标识
	string m_tgDir;  //安装目录
	string m_username;
private:
	BOOL GetHostInfo(char *ip,DWORD ipsize,char *hostname,DWORD hsize);
	BOOL GetOsVersion(char *osName,DWORD dwsize);
	BOOL GetMacAddress(char *pMacAdd, DWORD dwSize);
	BOOL GetCPUID(char *szCPUID, DWORD size);
	std::string GuidToString(const GUID &guid);
public:
	BOOL Init();
	TermInfo();
	~TermInfo();
};

