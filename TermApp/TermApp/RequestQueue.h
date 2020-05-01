#pragma once
#include<queue>
#include<curl/Request.h>
#include<curl/CURLClient.h>
#include<mutex>
#include<boost/algorithm/string.hpp>
#include "MatchRule.h"

class RequestQueue
{
public:
	bool Start();
	//�ļ������ϱ�
	void AddRequest(int type,LPWSTR wfileName,LPWSTR wprocessName,MatchRule *rule,LPWSTR wprameter = NULL,BOOL isSensitive = TRUE);
	//���ݿ��ϱ�
	void AddRequest(int type, MatchRule *rule, wchar_t *wdata);
	void AddRequest(Request &req);
	RequestQueue();
	~RequestQueue();
	static DWORD WINAPI SendRequestProc(LPVOID lParamter);
private:
	bool ToEscapeString(std::wstring &wstr);
	std::queue<Request>m_reqQueue;
	std::mutex m_queLock;
};

