#pragma once
#include <string>
#include <TlHelp32.h>
using namespace std;

class CProcessView
{
public:
	CProcessView(void);
	~CProcessView(void);

public:
	BOOL FindProcessByName_Myexe(CString exefile);		//�������Ƿ������У�������
	BOOL FindProcessByName_other(CString exefile);			//�������Ƿ������У����Ǳ�����
	BOOL OpenApp(CString exefile);
	BOOL CloseApp(CString exefile);
	float	 GetProessCPU(CString exefile);
	void LogData(const char* sql);
	void LogData(const string& sql);

};
