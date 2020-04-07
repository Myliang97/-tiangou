#include "RequestQueue.h"
#include<md5/MD5.h>
#include "TermApp.h"
#include<datetime/DateTime.hpp>
#include "TermApp.h"

RequestQueue::RequestQueue()
{
}


RequestQueue::~RequestQueue()
{
}

void RequestQueue::AddRequest(int type, LPWSTR wfileName, LPWSTR wprocessName, MatchRule *rule , LPWSTR wprameter /* = NULL */, BOOL isSensitive /* = TRUE *//* =NULL*/)
{
	Request req;
	req.mInfo.mCmdId = 2;
	wchar_t *wverifyId = StrLib::Utf8ToUnicode(TermApp::Instance()->GetTermInfo()->m_verifyId.c_str());
	if (wverifyId)
	{
		req.mInfo.mVerifyId = wverifyId;
		delete[]wverifyId;
	}
	char filemd5[33];
	MDFileW(wfileName, filemd5);
	wchar_t *wmd5 = StrLib::Utf8ToUnicode(filemd5);
	if (wmd5)
	{
		req.mContent.mMd5 = wmd5;
		delete[]wmd5;
	}
	char pathMd5[33];
	MDStringW(wfileName, pathMd5);
	wchar_t *wpathMd5 = StrLib::Utf8ToUnicode(pathMd5);
	if (wpathMd5)
	{
		req.mContent.mPathMd5 = wpathMd5;
		delete[]wpathMd5;
	}
	req.mContent.mCurTime = CDateTime::GetLocalTimeW();
	req.mContent.mFileName = wfileName;
	req.mContent.mOperator = wprocessName;
	req.mContent.mMatchRule = rule->GetMatchRuleName();
	req.mContent.mMatchInfo = rule->GetMatchInfo();
	if (wprameter)
	{
		req.mContent.mNewFile = wprameter;
		char newPathMd5[33];
		MDStringW(wprameter, newPathMd5);
		wchar_t *wnewPathMd5 = StrLib::Utf8ToUnicode(newPathMd5);
		req.mContent.mNewPathMd5 = wnewPathMd5; 
		if (wnewPathMd5)
			delete[]wnewPathMd5;
	}
	req.mContent.mIsSensitive = isSensitive;
	req.mContent.mType = type;
	m_queLock.lock();
	m_reqQueue.push(req);
	m_queLock.unlock();
}

void RequestQueue::AddRequest(Request &req)
{
	m_queLock.lock();
	m_reqQueue.push(req);
	m_queLock.unlock();
}

DWORD WINAPI RequestQueue::SendRequestProc(LPVOID lParamter)  //上报数据线程
{
	RequestQueue *que = (RequestQueue*)lParamter;
	if (!que)
	{
		return 0;
	}
	std::queue<Request> cacheQue;
	while (true)  
	{
		que->m_queLock.lock();
		size_t size = que->m_reqQueue.size();
		if (size <=0)
		{
			que->m_queLock.unlock();
			Sleep(1000);
			continue;
		}
		while (que->m_reqQueue.size()>0) {
			Request req = que->m_reqQueue.front();
			que->m_reqQueue.pop();
			cacheQue.push(req);
		}
		que->m_queLock.unlock();
		CURLClient client;
		client.init(TermApp::Instance()->GetUrl(), TermApp::Instance()->GetPort());
		std::string strxml;
		while (cacheQue.size()) {
			Request req = cacheQue.front();
			if (RequestToXmL::RequestToXmlA(req, strxml))
			{
				//std::ofstream file("C:\\Users\\Liang\\Desktop\\Release\\test.xml",ios::app);
				//file.write(strxml.c_str(), strxml.length());
				if (!client.SendData(strxml.c_str(), strxml.length()))
				{
					TermApp::Instance()->GetLogger()->Trace(ERROR_FORMAT, "send data fail");
				}
			}
			cacheQue.pop();
		}

	}
	return 0;
}

bool RequestQueue::Start()
{
	HANDLE hd = CreateThread(NULL, NULL, SendRequestProc, this, NULL, NULL);
	if (hd == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	CloseHandle(hd);
	return true;
}
bool RequestQueue::ToEscapeString(std::wstring &wstr)
{
	int pos = wstr.find(L'\\');
	while (pos !=std::wstring::npos)
	{
		wstr.replace(pos, pos + 1, L"\\\\");
		pos = wstr.find(L'\\', pos + 1);
	}
	return true;
}