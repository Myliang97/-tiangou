#include "FileParser.h"
#include<tgarchive.h>
#include<pstParser.h>
#include<file/FileLib.h>
#include<shellapi.h>
#include"TermApp.h"

#define TGARCHIVE_DLL "tgarchive.dll"
#define PSTPARSER_DLL "pstParser.dll"

bool FileParser::ParserTextFile(const wchar_t *wfileName, std::wstring &wdata)
{
	if (!wfileName)return false;
	std::wifstream file(wfileName);
	if (!file.is_open())return false;
	std::wstring line;
	wdata.clear();
	while (getline(file,line))
	{
		wdata.append(line);
	}
	return true;
}

bool FileParser::MsOfficeParser(const wchar_t *fileName, std::wstring &wdata)
{
	wchar_t buff[MAX_PATH];
	GetModuleFileNameW(NULL, buff, sizeof(buff));
	LPWSTR lpstr = wcsrchr(buff, '\\');
	lpstr[0] = 0;
	LPWSTR m_dir = buff;
	wchar_t cmd[1024];
	std::wstring outFile = GetTempFileW();
	wsprintfW(cmd, L" %s %s",fileName, outFile.c_str());
	wchar_t lpfile[1024];
	wsprintfW(lpfile, L"%s\\MsOfficeParser.exe", m_dir);
	STARTUPINFOW si = { sizeof(STARTUPINFOW) };
	PROCESS_INFORMATION pi = { 0 };
	ShellExecuteW(NULL, L"open", lpfile, cmd, NULL, SW_HIDE);
	DWORD fn = 0;
	int times = 50;
	do
	{
		HKEY hKey = NULL;
		LONG lRet = RegOpenKeyA(HKEY_CURRENT_USER, TG_REG_ROOT_PATH, &hKey);
		if (lRet != ERROR_SUCCESS)
		{
			break;
		}
		DWORD dwValueType = REG_NONE;
		DWORD dwValueSize = sizeof(DWORD);
		lRet = RegQueryValueExA(hKey, "officeFn", 0, &dwValueType, (LPBYTE)&fn, &dwValueSize);
		if (lRet != ERROR_SUCCESS)
		{
			RegCloseKey(hKey);
			break;
		}
		RegCloseKey(hKey);
		Sleep(200);
	} while (0 == fn && --times);
	HKEY hKey = NULL;
	LONG lRet = RegOpenKeyA(HKEY_CURRENT_USER, TG_REG_ROOT_PATH, &hKey);
	DWORD dwValueType = REG_NONE;
	DWORD dwValueSize = sizeof(DWORD);
	DWORD resetNum = 0;
	RegSetValueExA(hKey, "officeFn", 0, dwValueType, (LPBYTE)&resetNum, dwValueSize);
	RegCloseKey(hKey);
	if (fn)
	{
		char *zfilename = StrLib::UnicodeToUtf8(outFile.c_str());
		if (zfilename) {
			std::ifstream file(zfilename);
			if (!file.is_open()) {
				DeleteFileA(zfilename);
				delete[]zfilename;
				return false;
			}
			std::string line;
			std::string data;
			while (getline(file,line))
			{
				data.append(line);
			}
			file.close();
			wchar_t *buff = StrLib::Utf8ToUnicode(data.c_str());
			if (buff) {
				wdata = buff;
				delete[]buff;
			}
			//TermApp::Instance()->GetLogger()->Trace(INFO_FORMAT, "file:%s", zfilename);
			DeleteFileA(zfilename);
			delete[]zfilename;
		}
		return true;
	}
	DeleteFileW(outFile.c_str());
	return false;
}

static HMODULE HTgextract = NULL;
static ExtractAllDef ExtractFunc = NULL;														   
bool FileParser::DecompressFile(const wchar_t *wfileName, FileType fileType, const wchar_t *tempDir)
{
	ArchiveFormats formats;
	switch (fileType)
	{
	case Z7:
		formats = ArchiveFormats::format_7z;
		break;
	case RAR:
		formats = ArchiveFormats::format_rar5;
		break;
	case ZIP:
		formats = ArchiveFormats::format_zip;
		break;
	default:
		formats = ArchiveFormats::format_7z;
		break;
	}
	if (NULL == HTgextract) {
		HTgextract = LoadLibraryA(TGARCHIVE_DLL);
		if (NULL == HTgextract)return false;
	}
	if (NULL == ExtractFunc) {
		ExtractFunc = (ExtractAllDef)GetProcAddress(HTgextract, "ExtractAll");
		if (NULL == ExtractFunc)return false;
	}
	int ret = ExtractFunc(wfileName, tempDir, formats);
	return ret == 0;
}

static HMODULE HPstparser = NULL;
static PstParserDef pstParserFunc = NULL;
bool FileParser::ParserPstFile(const wchar_t *wfileName, std::wstring &wdata, const wchar_t *tempDir)
{
	std::wstring newFileName = tempDir;
	newFileName += L"\\tmp.pst";
	if (!CopyFileW(wfileName, newFileName.c_str(), FALSE))  //防止pst文件出现出现中文路径
	{
		return false;
	}
	if (NULL == HPstparser)
	{
		HPstparser = LoadLibraryA(PSTPARSER_DLL);
		if (NULL == HPstparser)return false;
	}
	
	if (NULL == pstParserFunc) {
		pstParserFunc = (PstParserDef)GetProcAddress(HPstparser, "PstParser");
		if (NULL == pstParserFunc)return false;
	}
	std::wstring contentFile = tempDir;
	contentFile += L"\\content.txt";
	if (!pstParserFunc(newFileName.c_str(), contentFile.c_str(), tempDir))
	{
		return false;
	}
	DeleteFileW(newFileName.c_str());
	std::wifstream file(contentFile.c_str());
	if (!file.is_open())return false;
	std::wstring line;
	while (getline(file,line))
	{
		wdata.append(line);
	}
	DeleteFileW(contentFile.c_str());
	return true;
}

FileType FileParser::GetFileType(const wchar_t *fileName)
{
	if (!fileName)
	{
		return FileType::UnKnownType;
	}
	const wchar_t * lpstr = wcsrchr(fileName, '.');
	if (lpstr)
	{
		lpstr++;
	}
	else
		return FileType::UnKnownType;
	if (0 == wcsicmp(L"txt", lpstr))
	{
		return FileType::TXT;
	}
	else if (0 == wcsicmp(L"xml", lpstr))
	{
		return FileType::XML;
	}
	else if (0 == wcsicmp(L"html", lpstr))
	{
		return FileType::HTML;
	}
	else if (0 == wcsicmp(L"docx", lpstr))
	{
		return FileType::DOCX;
	}
	else if (0 == wcsicmp(L"xlsx", lpstr))
	{
		return FileType::XLSX;
	}
	else if (0 == wcsicmp(L"pptx", lpstr))
	{
		return FileType::PPTX;
	}
	else if (0 == wcsicmp(L"pdf", lpstr))
	{
		return FileType::PDF;
	}
	else if (0 == wcsicmp(L"7z", lpstr))
	{
		return FileType::Z7;
	}
	else if (0 == wcsicmp(L"rar", lpstr))
	{
		return FileType::RAR;
	}
	else if (0 == wcsicmp(L"zip", lpstr))
	{
		return FileType::ZIP;
	}
	else if (0 == wcsicmp(L"pst", lpstr))
	{
		return FileType::PST;
	}
	return FileType::UnKnownType;
}