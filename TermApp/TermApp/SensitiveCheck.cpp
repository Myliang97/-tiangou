#include "SensitiveCheck.h"
#include "TermApp.h"
#include<stringLib/strLib.hpp>
#include<file/FileLib.h>
#include"TermApp.h"
#include"FileParser.h"

BOOL IsSecretFileA(LPSTR fileName, MatchRule &rule)
{
	if (!fileName)
	{
		return FALSE;
	}
	LPWSTR wfileName = StrLib::Utf8ToUnicode(fileName);
	BOOL ret = IsSecretFileW(wfileName,rule);
	if(wfileName)
		delete[]wfileName;
	return ret;
}

BOOL IsSecretFileW(LPWSTR wfileName, MatchRule &rule)
{
	if (!wfileName)
	{
		return FALSE;
	}
	if (IsDirW(wfileName))
	{
		return CheckDirSecretW(wfileName, rule);
	}
	if (!IsScanType(wfileName))
	{
		return FALSE;
	}
	std::wstring wdata;
	FileType fileType = FileParser::GetFileType(wfileName);
	BOOL ret = FALSE;
	switch (fileType)
	{
		case UnKnownType:
		case TXT:
		case XML:
		case HTML:
			if (FileParser::ParserTextFile(wfileName, wdata))
			{
				CheckDataMatchRule(wdata.c_str(), rule,ret);
			}
			break;
		case DOCX:
		case XLSX:
		case PPTX:
		case PDF:
			if (FileParser::MsOfficeParser(wfileName, wdata))
			{
				CheckDataMatchRule(wdata.c_str(), rule, ret);
			}
			break;
		case Z7:
		case RAR:
		case ZIP:
			{
				std::wstring termDir = GetTempFileW();
				CreateDirectoryW(termDir.c_str());
				if (FileParser::DecompressFile(wfileName, fileType, termDir.c_str()))
				{
					MatchRule dirRule;
					if (CheckDirSecretW((LPWSTR)termDir.c_str(), dirRule))
					{
						rule.AddMacthRule(dirRule);
						ret = TRUE;
					}
				}
				DeleteDirectoryW((LPWSTR)termDir.c_str());
			}
			break;
		case PST:
			{
				std::wstring termDir = GetTempFileW();
				CreateDirectoryW(termDir.c_str());
				if (FileParser::ParserPstFile(wfileName, wdata, termDir.c_str()))
				{
					MatchRule dataRule;
					CheckDataMatchRule(wdata.c_str(), dataRule, ret);
					if (ret) {
						rule.AddMacthRule(dataRule);
					}
					MatchRule dirRule;
					if (CheckDirSecretW((LPWSTR)termDir.c_str(), dirRule))
					{
						rule.AddMacthRule(dirRule);
						ret = TRUE;
					}
				}
				DeleteDirectoryW((LPWSTR)termDir.c_str());
			}
			break;
		default:
			break;
	}
	return ret;
}

BOOL IsScanType(LPWSTR fileName)
{
	if (!fileName)
	{
		return FALSE;
	}
	ConfigData *conf = TermApp::Instance()->GetConfig()->GetConfigData();
	if (!conf)
	{
		return FALSE;
	}
	LPWSTR lpStr = wcsrchr(fileName, L'.');
	if (lpStr)
	{
		lpStr++;
	}
	else
	{
		lpStr = L"";
	}
	for (int i=0;i<conf->m_scanFileRuleList.size();++i)
	{
		std::vector<std::wstring>types = conf->m_scanFileRuleList[i].m_types;
		for (int i = 0; i < types.size(); ++i)
		{
			if (0 == wcsicmp(types[i].c_str(), lpStr))
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}


BOOL CheckDirSecretA(LPSTR dirName, MatchRule &rule) //检查目录下的敏感文件
{
	if (!dirName)return FALSE;
	LPWSTR wdirName = StrLib::Utf8ToUnicode(dirName);
	BOOL ret = CheckDirSecretW(wdirName,rule);
	if (wdirName)
	{
		delete[]wdirName;
	}
	return ret;
}
BOOL CheckDirSecretW(LPWSTR wdirName, MatchRule &rule)
{
	if (!wdirName)
	{
		return FALSE;
	}
	HANDLE hFile;
	WIN32_FIND_DATAW finddata;
	wchar_t enumPath[MAX_PATH];
	wsprintfW(enumPath, L"%s\\*",wdirName);
	hFile = ::FindFirstFileW(enumPath, &finddata);
	if (hFile == INVALID_HANDLE_VALUE) {
		return FALSE;
	}
	do
	{
		if (wcscmp(finddata.cFileName, L".") == 0 || wcscmp(finddata.cFileName, L"..") == 0)
		{
			FindNextFileW(hFile, &finddata);
			continue;
		}
		if (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			wsprintfW(enumPath, L"%s\\%s", wdirName, finddata.cFileName);
			MatchRule dirRule;
			if (CheckDirSecretW(enumPath, dirRule))
			{
				rule.AddMacthRule(dirRule);
			}
		}
		wchar_t scanFile[MAX_PATH];
		wsprintfW(scanFile, L"%s\\%s", wdirName, finddata.cFileName);
		MatchRule fileRule;
		if (IsSecretFileW(scanFile, fileRule))
		{
			rule.AddMacthRule(fileRule);
		}
	} while (FindNextFileW(hFile, &finddata));
	FindClose(hFile);
	return TRUE;
}

void CheckDataMatchRule(const wchar_t* wdata, MatchRule &rule, BOOL &isSensitive)  //匹配规则
{
	std::wstring text;
	ProcessText(wdata, text);
	if (text == L"")
		return;
	ConfigData *conf = TermApp::Instance()->GetConfig()->GetConfigData();
	std::vector<ScanFileRule> ruleList = conf->m_scanFileRuleList;
	//对配置文件的所有规则都进行判断
	for (std::vector<ScanFileRule>::iterator begin = ruleList.begin();begin !=ruleList.end();++begin)
	{
		MatchRule oneRule;
		bool isMatch = false;
		//匹配关键词
		vector<KeyWordAttribute>keywordList = begin->m_keywords;
		for (int i=0;i<keywordList.size();++i)
		{
			if (keywordList[i].m_keyword == L"") {
				continue;
			}
			wregex reg(keywordList[i].m_keyword);
			int num = 0;
			RegexSearch(text, reg, num);
			if (num >= keywordList[i].num) {
				oneRule.AddMacthInfo(keywordList[i].m_keyword);
				isMatch = true;
			}
		}
		vector<RegexAttribute>regexList = begin->m_regexs;
		for (int i=0;i<regexList.size();++i)
		{
			if (regexList[i].m_regex == L"") {
				continue;
			}
			wregex reg(regexList[i].m_regex);
			int num = 0;
			RegexSearch(text, reg, num);
			if (num>=regexList[i].num){
				oneRule.AddMacthInfo(regexList[i].m_regex);
				isMatch = true;
			}
		}
		if (isMatch) {
			oneRule.AddMacthRuleName(begin->m_ruleName);
			rule.AddMacthRule(oneRule);
			isSensitive = TRUE;
		}
	}
}

void ProcessText(const wchar_t *wdata, std::wstring &outText)
{
	outText.clear();
	for (int i=0;i<wcslen(wdata);++i)
	{
		if (wdata[i] == L' ')  //去掉空格
		{
			continue;
		}
		outText.push_back(wdata[i]);
	}
}

void RegexSearch(std::wstring &text, std::wregex &reg, int &num)
{
	wsmatch res;
	wstring::const_iterator start = text.begin();
	wstring::const_iterator end = text.end();
	num = 0;
	try {
		while (regex_search(start, end, res, reg))
		{
			num++;
			start = res[0].second;
		}
	}
	catch (...) {
		num = 0;
	}
}