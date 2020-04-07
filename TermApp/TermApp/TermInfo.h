#pragma once
#include<Windows.h>
#include<strsafe.h>
#include<string>
//TermInfo��ȡ�ն˵�ip��mac�����������ϵͳ
using namespace std;
class TermInfo
{
public:
	string m_ip;
	string m_mac;
	string m_hostname;  //������û���
	string m_os;
	string m_verifyId;  //mac��cpuidɢ����Ϊ�ͷ�����֮��ı�ʶ
	string m_tgDir;  //��װĿ¼
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

