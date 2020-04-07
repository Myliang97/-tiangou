#pragma once
#include<Windows.h>
#include<global.h>
#include<string>
#include<regex>
#include "MatchRule.h"
/*
* 文件读取的dll文件名统一为unicode
* 取得的文件buffer也是unicode
*/

BOOL IsSecretFileA(LPSTR fileName,MatchRule &rule);
BOOL IsSecretFileW(LPWSTR wfileName, MatchRule &rule);

BOOL IsScanType(LPWSTR fileName);

BOOL CheckDirSecretA(LPSTR dirName, MatchRule &rule);
BOOL CheckDirSecretW(LPWSTR wdirName, MatchRule &rule);


void CheckDataMatchRule(const wchar_t *wdata,MatchRule &rule,BOOL &isSensitive);

//去掉空白字符
void ProcessText(const wchar_t *wdata,std::wstring &outText);

void RegexSearch(std::wstring &text, std::wregex &reg, int &num);