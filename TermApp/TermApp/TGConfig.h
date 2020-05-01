#pragma once
#include<Windows.h>
#include<string>
#include<xml/XmlReader.h>
#include<vector>
#include<mutex>
using namespace std;
struct ServerInfo 
{
	ServerInfo() {
		m_ip = L"";
		m_port = 0;
	}
	std::wstring m_ip;  //服务器ip
	int m_port;  //监听端口
};
struct UploadFile  //上传文件的规则
{
	UploadFile() {
		m_cmp = 0;
		m_type = 1;
		int m_size = 0;
	}
	int m_cmp;  //cmp:0等于size 1:大于size 2:小于size
	int m_type;  // 0:不上传 1:上传敏感文件 2:全部上传
	int m_size; //上传的文件大小
};
struct FileOperator  //对文件的操作
{
	int m_encrypt;//敏感文件是否加密 0:不加密 1：加密
	int oper;//对于敏感文件限定操作 0:不记录日志 1:记录日志
};
struct KeyWordAttribute
{
	wstring m_keyword;  //正则表达式
	int num;     //匹配次数
};
struct RegexAttribute
{
	wstring m_regex;
	int num;
};
struct ScanFileRule  //扫描的文件规则，命中一个则为敏感文件
{
	vector<KeyWordAttribute>m_keywords; //关键词
	vector<RegexAttribute>m_regexs;  //正则表达式
	vector<wstring>m_types; //扫描的文件类型
	wstring m_ruleName;  //规则名
};


struct ConfigData
{
	ServerInfo m_server;
	UploadFile m_uploadFileRule;
	FileOperator m_oper;
	vector<ScanFileRule>m_scanFileRuleList;
};
class TGConfig
{
public:
	static TGConfig *Instance();
	BOOL UpdateConfig();
	ConfigData *GetConfigData() { return m_configData; }
	TGConfig();
	~TGConfig();
private:
	static DWORD WINAPI UpdateConfig(LPVOID);
	BOOL LoadConf(XmlReader &reader);
	static TGConfig *m_config;
	ConfigData *m_configData;
	HANDLE m_updateConf;
	HANDLE m_finish;
};

