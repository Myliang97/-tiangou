#pragma once
#include<Windows.h>
#include<global.h>
#include<string>
#include<regex>
#include "MatchRule.h"
/*
* �ļ���ȡ��dll�ļ���ͳһΪunicode
* ȡ�õ��ļ�bufferҲ��unicode
*/

BOOL IsSecretFileA(LPSTR fileName,MatchRule &rule);
BOOL IsSecretFileW(LPWSTR wfileName, MatchRule &rule);

BOOL IsScanType(LPWSTR fileName);

BOOL CheckDirSecretA(LPSTR dirName, MatchRule &rule);
BOOL CheckDirSecretW(LPWSTR wdirName, MatchRule &rule);


void CheckDataMatchRule(const wchar_t *wdata,MatchRule &rule,BOOL &isSensitive);

//ȥ���հ��ַ�
void ProcessText(const wchar_t *wdata,std::wstring &outText);

void RegexSearch(std::wstring &text, std::wregex &reg, int &num);