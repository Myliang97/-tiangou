#pragma once
#include<global.h>
#include<windows.h>
#include<fstream>
#include<string>
enum FileCodingType {
	Multi_Byte,
	Utf_8,
	Unicode
};
class FileParser
{
public:
	static FileCodingType GetFileCoding(const wchar_t *wfilename);
	static FileType GetFileType(const wchar_t *wfileName);
	static bool ParserTextFile(const wchar_t *wfileName, std::wstring &wdata);
	static bool MsOfficeParser(const wchar_t *wfileName, std::wstring &wdata);
	static bool DecompressFile(const wchar_t *wfileName,FileType fileType, const wchar_t *tempDir);
	static bool ParserPstFile(const wchar_t *wfileName, std::wstring &wdata,const wchar_t *tempDir);
};