#include "TermInfo.h"
#include<log/DebugString.hpp>
#include<stdio.h>
#include<md5/MD5.h>
#include <Iphlpapi.h>
#include<tchar.h>
#include<WinSock2.h>
#include<register/RegLib.h>
#include<global.h>
#include<Shlwapi.h>
#pragma comment(lib,"Ws2_32.lib")
#pragma comment(lib,"Iphlpapi.lib")
#pragma comment(lib,"Shlwapi.lib")

BOOL TermInfo::GetHostInfo(char *ip, DWORD ipsize, char *hostname, DWORD hsize)
{
	if (!ip || !hostname)
	{
		return FALSE;
	}
	memset(ip, 0, ipsize);
	memset(hostname, 0, hsize);
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);
	struct hostent *hp = NULL;
	char *p = NULL;

	if (gethostname(hostname, hsize) < 0)
	{
		return FALSE;
	}
	hp = gethostbyname(hostname);
	if (hp == NULL)
		return FALSE;

	int i = 0;
	while (p = hp->h_addr_list[i]) {
		sprintf_s(ip, 16, "%d.%d.%d.%d", (int)p[0] & 0xFF, (int)p[1] & 0xFF, (int)p[2] & 0xFF, (int)p[3] & 0xFF);
		if (strcmp(ip, "127.0.0.1") != 0) break;
		i++;
	}
	return TRUE;
}
BOOL TermInfo::GetOsVersion(char *osName,DWORD dwsize)
{
	if (!osName)
	{
		return FALSE;
	}
	memset(osName, 0, dwsize);
	OSVERSIONINFOEX osvi;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if (!GetVersionEx((OSVERSIONINFO *)&osvi))
		return FALSE;
	if (osvi.dwMajorVersion >= 6)
	{
		void (WINAPI *GetVer)(DWORD*, DWORD*, DWORD*) = \
			(void (WINAPI *)(DWORD*, DWORD*, DWORD*))GetProcAddress(GetModuleHandle(_T("Ntdll.dll")), "RtlGetNtVersionNumbers");

		if (GetVer)
			GetVer(&osvi.dwMajorVersion, &osvi.dwMinorVersion, &osvi.dwBuildNumber);
		osvi.dwBuildNumber &= 0xFFFF;
	}
	switch (osvi.dwPlatformId)
	{
	case VER_PLATFORM_WIN32_NT:

		// Test for the product.
		if (osvi.dwMajorVersion <= 4)
			StringCchCopyA(osName, MAX_PATH, "Windows NT 4.0");
		else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
			StringCchCopyA(osName, MAX_PATH, "Microsoft Windows 2000 ");
		else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1)
			StringCchCopyA(osName, MAX_PATH, "Microsoft Windows XP ");

		//Windows Vista之后
		else if (osvi.dwMajorVersion >= 6)
		{
			if (osvi.dwMajorVersion == 10)//10.0
			{
				if (osvi.wProductType == VER_NT_WORKSTATION)
					StringCchCopyA(osName, MAX_PATH, "Windows 10");
				else
				{
					StringCchCopyA(osName, MAX_PATH, "Windows Server 2016");
				}
			}
			else if (osvi.dwMajorVersion == 6)
			{
				if (osvi.dwMinorVersion == 0) //6.0
				{
					if (osvi.wProductType == VER_NT_WORKSTATION)
					{
						StringCchCopyA(osName, MAX_PATH, "Windows Vista");
					}
					else
					{
						StringCchCopyA(osName, MAX_PATH, "Windows Server 2008");
					}

				}
				else if (osvi.dwMinorVersion == 1)//win7
				{
					if (osvi.wProductType == VER_NT_WORKSTATION)
					{
						StringCchCopyA(osName, MAX_PATH, "Windows 7");
					}
					else
					{
						StringCchCopyA(osName, MAX_PATH, "Windows Server 2008 R2");
					}
				}
				else if (osvi.dwMinorVersion == 2) //6.2
				{
					if (osvi.wProductType == VER_NT_WORKSTATION)  //Windows server 2012
					{
						StringCchCopyA(osName, MAX_PATH, "Windows 8");
					}
					else
					{
						StringCchCopyA(osName, MAX_PATH, "Windows Server 2012");
					}
				}
				else if (osvi.dwMinorVersion == 3) //6.3
				{

					if (osvi.wProductType == VER_NT_WORKSTATION)
					{
						StringCchCopyA(osName, MAX_PATH, "Windows 8.1");
					}
					else  //Windows server 2012 R2
					{
						StringCchCopyA(osName, MAX_PATH, "Windows Server 2012 R2");
					}
				}
			}
		}
		break;

	case VER_PLATFORM_WIN32_WINDOWS:

		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)
		{
			if (osvi.szCSDVersion[1] == 'C' || osvi.szCSDVersion[1] == 'B')
				StringCchCopyA(osName, MAX_PATH, "Microsoft Windows 95 OSR2 ");
		}
		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10)
		{
			if (osvi.szCSDVersion[1] == 'A')
				StringCchCopyA(osName, MAX_PATH, "Microsoft Windows 98 SE");
		}

		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90)
		{
			StringCchCopyA(osName, MAX_PATH, "Microsoft Windows Me ");
		}
		break;

	case VER_PLATFORM_WIN32s:
		StringCchCopyA(osName, MAX_PATH, "Microsoft Win32s ");
		break;
	}
	return osName[0] != '\0';
}
 
#define  MAC_REG_PATH  "SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\%s\\Connection"
BOOL TermInfo::GetMacAddress(char *pMacAdd, DWORD dwSize)
{
	if (!pMacAdd || !dwSize)return FALSE;
	memset(pMacAdd, 0, dwSize);

	IP_ADAPTER_INFO AdapterInfo[16];            // 定义网卡信息存贮区。
	DWORD dwBufLen = sizeof(AdapterInfo);
	DWORD dwStatus = GetAdaptersInfo(
		AdapterInfo,                        // [output] 指向接收数据缓冲指针
		&dwBufLen);                         // [input] 缓冲区大小

	if (dwStatus != ERROR_SUCCESS)return FALSE;
	PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
	while (pAdapterInfo)
	{
		if (pAdapterInfo->Type != MIB_IF_TYPE_ETHERNET)
		{
			continue;
		}
		memset(pMacAdd, 0, dwSize);
		char szTemp[260] = { 0 };
		for (unsigned int i = 0; i < pAdapterInfo->AddressLength; i++)
		{
			if (i == pAdapterInfo->AddressLength - 1)
			{
				StringCchPrintfA(szTemp, _countof(szTemp), "%02x", pAdapterInfo->Address[i]);
				StringCchCatA(pMacAdd, dwSize, szTemp);
			}
			else
			{
				StringCchPrintfA(szTemp, _countof(szTemp), "%02x-", pAdapterInfo->Address[i]);
				StringCchCatA(pMacAdd, dwSize, szTemp);
			}
		}
		CHAR regPath[MAX_PATH];
		sprintf(regPath, MAC_REG_PATH, pAdapterInfo->AdapterName);
		CHAR pnPInstanceId[MAX_PATH];
		ReadRegVal(HKEY_LOCAL_MACHINE, regPath, "PnPInstanceId",(LPBYTE)&pnPInstanceId, sizeof(pnPInstanceId));
		LPSTR lpStr = StrStrIA(pnPInstanceId, "PCI");
		if (lpStr)
		{
			return TRUE;
		}
		pAdapterInfo = pAdapterInfo->Next;
	}
	return TRUE;
}
BOOL TermInfo::GetCPUID(char *szCPUID, DWORD size)
{
	if (!szCPUID)
	{
		return FALSE;
	}
	memset(szCPUID, 0, size);
	unsigned long s1, s2, s3, s4;
	__asm {
		mov eax, 01h
		xor edx, edx
		cpuid
		mov s1, edx
		mov s2, eax
	}
	__asm {
		mov eax, 03h
		xor ecx, ecx
		cpuid
		mov s3, edx
		mov s4, ecx
	}
	StringCchPrintfA(szCPUID, MAX_PATH, "%lu%lu%lu%lu", s1, s2, s3, s4);
	return TRUE;
}
BOOL TermInfo::Init()
{
	//获得用户安装目录
	CHAR installDir[MAX_PATH];
	GetModuleFileName(NULL, installDir, sizeof(installDir));
	LPSTR dir = strrchr(installDir, '\\');
	if (dir == NULL)
	{
		TGDebug::debug(DEBUG_STRING, "GetModuleFileName error%d", GetLastError());
		return FALSE;
	}
	dir[0] = '\0';
	m_tgDir = installDir;

	//获得客户端的ip和主机名
	CHAR ip[20];
	CHAR hostname[MAX_PATH];
	if (!GetHostInfo(ip, 20, hostname, MAX_PATH))
	{
		TGDebug::debug(DEBUG_STRING, "GetClientIp error");
		return FALSE;
	}
	CHAR userName[MAX_PATH];
	DWORD len = MAX_PATH;
	if (!GetUserNameA(userName, &len))
	{
		return FALSE;
	}
	m_username = userName;
	m_ip = ip;
	m_hostname = hostname;
	//获得客户端的mac
	CHAR mac[MAX_PATH];
	if (!GetMacAddress(mac, MAX_PATH))
	{
		TGDebug::debug(DEBUG_STRING, "GetMacAddress error%d", GetLastError());
		return FALSE;
	}
	m_mac = mac;
	//获得客户端的os
	CHAR os[MAX_PATH];
	if (!GetOsVersion(os, MAX_PATH))
	{
		TGDebug::debug(DEBUG_STRING, "GetOsVersion error%d", GetLastError());
		return FALSE;
	}
	m_os = os;
	CHAR verid[33];
	if (!ReadRegVal(HKEY_CURRENT_USER, TG_REG_ROOT_PATH, TG_REG_VERIFY_ID, (LPBYTE)verid, sizeof(verid)))
	{
		CHAR cpuId[MAX_PATH];
		//获得cpuid
		if (!GetCPUID(cpuId, MAX_PATH))
		{
			TGDebug::debug(DEBUG_STRING, "GetCPUID error%d", GetLastError());
			return FALSE;
		}
		GUID guid;
		HRESULT h = CoCreateGuid(&guid);
		if (h != S_OK) {
			TGDebug::debug(DEBUG_STRING, "GetGUID error%d", GetLastError());
			return FALSE;
		}
		std::string key = GuidToString(guid);
		hmac_md5(cpuId,(char*)key.c_str(),verid);
		WriteReg(HKEY_CURRENT_USER, TG_REG_ROOT_PATH, TG_REG_VERIFY_ID, (LPBYTE)verid, sizeof(verid),REG_SZ);
	}
	m_verifyId = verid;
	return TRUE;
}
std::string TermInfo::GuidToString(const GUID &guid)
{
	char buf[64] = { 0 };
	sprintf_s(buf, sizeof(buf),
		"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
		guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
	return std::string(buf);
}
TermInfo::TermInfo()
{
}
TermInfo::~TermInfo()
{
}
