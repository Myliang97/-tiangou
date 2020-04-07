#pragma once
#include<global.h>
#include<windows.h>
#include<fstream>
#include<string>
class FileParser
{
public:
	static FileType GetFileType(const wchar_t *wfileName);
	static bool ParserTextFile(const wchar_t *wfileName, std::wstring &wdata);
	static bool MsOfficeParser(const wchar_t *wfileName, std::wstring &wdata);
	static bool DecompressFile(const wchar_t *wfileName,FileType fileType, const wchar_t *tempDir);
	static bool ParserPstFile(const wchar_t *wfileName, std::wstring &wdata,const wchar_t *tempDir);
};