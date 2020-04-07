#pragma once
#include<Windows.h>
#include<global.h>
#include<mutex>
class Task
{
public:
	Task();
	~Task();
	static Task * m_Task;
public:
	static Task * Instance();
	static DWORD WINAPI GuardTaskThreadProc(LPVOID lpParameter);
	void Stop();
	BOOL StartTerm();
	
};