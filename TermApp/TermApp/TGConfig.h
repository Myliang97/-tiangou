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
	std::wstring m_ip;  //������ip
	int m_port;  //�����˿�
};
struct UploadFile  //�ϴ��ļ��Ĺ���
{
	UploadFile() {
		m_cmp = 0;
		m_type = 1;
		int m_size = 0;
	}
	int m_cmp;  //cmp:0����size 1:����size 2:С��size
	int m_type;  // 0:���ϴ� 1:�ϴ������ļ� 2:ȫ���ϴ�
	int m_size; //�ϴ����ļ���С
};
struct FileOperator  //���ļ��Ĳ���
{
	int m_encrypt;//�����ļ��Ƿ���� 0:������ 1������
	int oper;//���������ļ��޶����� 0:����¼��־ 1:��¼��־
};
struct KeyWordAttribute
{
	wstring m_keyword;  //������ʽ
	int num;     //ƥ�����
};
struct RegexAttribute
{
	wstring m_regex;
	int num;
};
struct ScanFileRule  //ɨ����ļ���������һ����Ϊ�����ļ�
{
	vector<KeyWordAttribute>m_keywords; //�ؼ���
	vector<RegexAttribute>m_regexs;  //������ʽ
	vector<wstring>m_types; //ɨ����ļ�����
	wstring m_ruleName;  //������
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

