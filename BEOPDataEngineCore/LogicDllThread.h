#pragma once

#include "VECTOR"
#include "DataBaseSetting.h"

class CDLLObject;
class CLogicPipeline;

using namespace std;

class  CLogicDllThread
{
public:
	CLogicDllThread(wstring strThreadName);
	~CLogicDllThread(void);


	//�����߳�
	bool    GeneratePipelines();
	bool	StartThread();
	bool    Exit();
	static  UINT WINAPI ThreadFunc(LPVOID lparam);

	HANDLE		GetThreadHandle();
	bool		SetThreadHandle(HANDLE hThread);

	bool		GetRunStatus();
	bool		SetRunStatus(bool runstatus);

	void AddDll(CDLLObject *pDLLObject);
	void		DeleteDll(CDLLObject *pDLLObject);
	int  GetDllCount();
	int GetPipelineCount();
	CLogicPipeline * GetPipeline(int nIndex);
	CDLLObject * GetDllObject(int iIndex);
	wstring GetName();
	wstring GetStructureString();

	void SetThreadID(int nID, int nTotal);

	bool AddThreadActionLog( CString strErrInfo );
	bool AddThreadCrashLog( CString strErrInfo );
	bool AddThreadActExceptionLog( CString strErrInfo );

protected:
	virtual UINT ThreadMerberFunc();
	HANDLE						m_hThread_dll;
	UINT						m_hThreadId;
	HANDLE			m_hEventClose;


private:
	std::vector<CDLLObject*> m_vecImportDLLList;
	wstring m_strThreadName;
	bool m_bRunning;
	bool m_bNeedInit;

	int m_nLogicThreadID;
	int m_nLogicTotalCount;

	std::vector<CLogicPipeline *> m_pLogicPipelineList;
};

