#include "StdAfx.h"
#include "LogicDllThread.h"
#include "LogicPipeline.h"
#include "DLLObject.h"
#include "../BEOPLogicEngine/RedisManager.h"


#include "../redis/hiredis.h"
#include "../Http/HttpOperation.h"

extern bool g_bSingleThread;
extern Project::Tools::Mutex	g_thread_lock;

CLogicDllThread::CLogicDllThread(wstring strThreadName)
	:m_strThreadName(strThreadName)
{
	m_hThread_dll			= NULL;
	m_hThreadId				= 0;
	//初始处于非激发状态，在变成激发状态后，不重置为非激发状态
	m_hEventClose   = CreateEvent(NULL,TRUE,FALSE,NULL);

	m_bRunning = false;

	m_bNeedInit = true;
}


CLogicDllThread::~CLogicDllThread(void)
{
	if (m_hEventClose != NULL)
	{
		CloseHandle(m_hEventClose);
		m_hEventClose =NULL;
	}

	if (m_hThread_dll != NULL)
	{
		CloseHandle(m_hThread_dll);
		m_hThread_dll =NULL;
	}
}


void CLogicDllThread::AddDll(CDLLObject *pDLLObject)
{
	CLogicBase *pBase = pDLLObject->GetLB();
	CString dllver = pBase->GetSystVersion();
	double  ddllver = _ttof(dllver.Mid(1,dllver.GetLength()));

	if(pBase && ddllver>=3.2)
	{
		pBase->SetDllThreadName(m_strThreadName.c_str());
	}
	m_vecImportDLLList.push_back(pDLLObject);
}

int CLogicDllThread::GetDllCount()
{
	return m_vecImportDLLList.size();
}

int CLogicDllThread::GetPipelineCount()
{
	return m_pLogicPipelineList.size();
}

CLogicPipeline * CLogicDllThread::GetPipeline(int nIndex)
{
	if(nIndex>= m_pLogicPipelineList.size())
		return NULL;

	if(nIndex<0)
		return NULL;

	return m_pLogicPipelineList[nIndex];
}


CDLLObject * CLogicDllThread::GetDllObject(int iIndex)
{
	if(iIndex<0 || iIndex>=m_vecImportDLLList.size())
		return NULL;

	return m_vecImportDLLList[iIndex];
}

wstring CLogicDllThread::GetName()
{
	return m_strThreadName;
}

void CLogicDllThread::SetThreadID(int nID, int nTotal)
{
	m_nLogicThreadID = nID;
	m_nLogicTotalCount = nTotal;
}

wstring CLogicDllThread::GetStructureString()
{
	CString strAll;
	strAll.Format(L"Thread Name: %s\r\n", GetName().c_str());
	for(int mLine =0; mLine<m_pLogicPipelineList.size();mLine++)
	{
		CLogicPipeline *pLine =  m_pLogicPipelineList[mLine];
		CString strTemp = L"    |----";
		for(int j=0;j<pLine->GetLogicDllCount();j++)
		{
			CDLLObject *pObject = pLine->GetLogicObject(j);
			strTemp += pObject->GetDllName().c_str();
			CString strTimeSpan;
			strTimeSpan.Format(_T("(Period: %.1f seconds)"), pObject->GetTimeSpan());
			strTemp += strTimeSpan;
			if(j<pLine->GetLogicDllCount()-1)
				strTemp+=L"--->";
			else
				strTemp+=L"----|";
		}

		strAll +=strTemp;
		strAll +=L"\r\n";
	}
	wstring wstrAll = strAll.GetString();
	return wstrAll;
}



UINT WINAPI CLogicDllThread::ThreadFunc(LPVOID lparam)
{
	CLogicDllThread * pThisDll = (CLogicDllThread*)lparam;

	//循环调用线程主内容
	pThisDll->ThreadMerberFunc();


	return 0;
}

HANDLE CLogicDllThread::GetThreadHandle()
{
	return m_hThread_dll;
}


bool CLogicDllThread::SetThreadHandle(HANDLE hThread)
{
	m_hThread_dll = hThread;
	return true;
}

bool CLogicDllThread::GeneratePipelines()
{
	for(int mLine =0; mLine<m_pLogicPipelineList.size();mLine++)
	{
		if(m_pLogicPipelineList[mLine])
		{
			delete(m_pLogicPipelineList[mLine]);
			m_pLogicPipelineList[mLine] = NULL;
		}
	}
	m_pLogicPipelineList.clear();

	for(int i=0;i<m_vecImportDLLList.size();i++)
	{
		CLogicPipeline *pLine = new CLogicPipeline(m_strThreadName.c_str());
		pLine->PushLogicObject(m_vecImportDLLList[i]);
		m_pLogicPipelineList.push_back(pLine);
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
bool CLogicDllThread::StartThread()
{
	m_hThread_dll = (HANDLE)_beginthreadex(NULL,
		0,
		ThreadFunc, 
		(LPVOID)this,
		CREATE_SUSPENDED,
		&m_hThreadId);

	if(m_hThread_dll == NULL)
	{
		int nErrCode = GetLastError();
		CString str;
		str.Format(_T("ERROR: _beginthreadex failed( Threadname:%s,errCode:%d)"),GetName().c_str(),nErrCode);
		_tprintf(str);
		return false;
	}

	::ResumeThread(m_hThread_dll);
	return true;
}



UINT CLogicDllThread::ThreadMerberFunc()
{

	int nClockCount = 0;
	int nClockMS = 100;

	int i=0;
	vector<int> nClockCountList;
	for(i=0;i<m_pLogicPipelineList.size();i++)
	{
		nClockCountList.push_back(0);
	}
	
	ResetEvent(m_hEventClose);


	int nActCount = 0;

	COleDateTime tLastLoopRecord = COleDateTime::GetCurrentTime();
	COleDateTime tLastActRecord = COleDateTime::GetCurrentTime();
	while (TRUE)
	{	
		Sleep(nClockMS);
		COleDateTime tnow = COleDateTime::GetCurrentTime();
	
		CBEOPDataAccess *pDataAccess = NULL;
		if (WaitForSingleObject(m_hEventClose,0) == WAIT_OBJECT_0)
		{
			if(m_pLogicPipelineList.size()>0)
			{
				if(m_pLogicPipelineList[0]->GetLogicDllCount()>0)
				{
					pDataAccess = m_pLogicPipelineList[0]->GetLogicObject(0)->m_pDataAccess;
					if(pDataAccess)
					{
						CString strTT;
						strTT.Format(_T("[SYSTEM]LogicThread(%02d::%d::%s) Recv Close Event"), m_nLogicThreadID, m_nLogicTotalCount, m_strThreadName.c_str());
						pDataAccess->InsertLog(strTT.GetString());
					}
				}
			}
			//break;
		}

		if(!m_bRunning)
		{

			//_tprintf(_T("ThreadMerberFunc: Act Once but running disabled\r\n"));
			m_bNeedInit = true;
			Sleep(5000);
			continue;
		}

		//_tprintf(_T("ThreadMerberFunc: Need Init Pre Start"));

		if(m_bNeedInit)
		{//当暂停后继续回来，需要初始化
			for(i=0;i<m_pLogicPipelineList.size();i++)
				m_pLogicPipelineList[i]->Init();

			m_bNeedInit = false;
		}

		//求所有策略里最短时间
		double fMinSpan = 4*60*60;//最长4小时必须执行一次
		for(i=0;i<m_pLogicPipelineList.size();i++)
		{
			double fspan = m_pLogicPipelineList[i]->GetTimeSpanSeconds();
			if(fspan<fMinSpan)
				fMinSpan = fspan;
		}

		if(fMinSpan<=0)
			fMinSpan = 1;


		bool bDidRealLogic = false;
		for(i=0;i<m_pLogicPipelineList.size();i++)
		{
			if(i>=nClockCountList.size())
			{
				int nPushCount = i-nClockCountList.size()+1;
				for(int mm=0;mm<nPushCount;mm++)
				    nClockCountList.push_back(0);
			}
			nClockCountList[i]++;
			//double fspan = m_pLogicPipelineList[i]->GetTimeSpanSeconds();
			int nClockActiveCount = (int)(fMinSpan*1000.0)/nClockMS;



			bDidRealLogic = true;

			if(g_bSingleThread)
			{
				_tprintf(L"     \r\n");
				CString str;
				str.Format(_T("!!!!!!!! INFO: SingleThreadMode: ActLogic: pipeliine %d !!!!!!!!\r\n"), i);
				_tprintf(str);
				_tprintf(L"     \r\n");
				Project::Tools::Scoped_Lock<Mutex>	scopelock(g_thread_lock);

				AddThreadActionLog(_T("单线程调试模式策略流程执行开始: ")+ m_pLogicPipelineList[i]->GetLineString() +_T("\r\n"));
				m_pLogicPipelineList[i]->ActLogic(nActCount++);
				AddThreadActionLog(_T("单线程调试模式策略流程执行结束: ")+ m_pLogicPipelineList[i]->GetLineString()+_T("\r\n")+_T("\r\n"));
			}
			else
			{
				if (nClockCountList[i]<nClockActiveCount)
				{
					continue;
				}
				nClockCountList[i] = 0; //执行一次就清零

				bool bRunGood = m_pLogicPipelineList[i]->ActLogic(nActCount++);
				if(!bRunGood)
				{
					bDidRealLogic = false;
					CString strTT;
					strTT.Format(_T("[SYSTEM]LogicThread(%02d::%d::%s) ActLogic Failed(Maybe Some Child Logic)"), m_nLogicThreadID, m_nLogicTotalCount, m_strThreadName.c_str());
					AddThreadActExceptionLog(strTT);
				}
				else
				{
					
				}
			}

			
			
			if(nActCount>=100000000)
				nActCount = 0;
		}

		if(bDidRealLogic)
		{
			tnow = COleDateTime::GetCurrentTime();
			//if((tnow-tLastActRecord).GetTotalSeconds()>=60)
			{
				
				tLastActRecord = tnow;
				
				
					
				
			}
		}
		else
		{

		}
			
	}

	return 0;
}

bool CLogicDllThread::AddThreadActExceptionLog( CString strErrInfo )
{
	wstring strFilePath;
	Project::Tools::GetSysPath(strFilePath);
	strFilePath += L"\\log";
	if(Project::Tools::FindOrCreateFolder(strFilePath))
	{
		SYSTEMTIME stNow;
		GetLocalTime(&stNow);
		CString strLogPath;
		strLogPath.Format(_T("%s\\domlogic_thread_exception_%d_%02d_%02d.log"),strFilePath.c_str(),stNow.wYear,stNow.wMonth,stNow.wDay);
		CStdioFile	ff;
		if(ff.Open(strLogPath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
		{
			wstring strTime;
			Project::Tools::SysTime_To_String(stNow,strTime);
			CString strLog;
			strLog.Format(_T("%s    %s"),strTime.c_str(),strErrInfo);
			ff.Seek(0,CFile::end);
			ff.WriteString(strLog);
			ff.Close();
			return true;
		}
	}
	return false;
}

bool CLogicDllThread::AddThreadActionLog( CString strErrInfo )
{
	wstring strFilePath;
	Project::Tools::GetSysPath(strFilePath);
	strFilePath += L"\\log";
	if(Project::Tools::FindOrCreateFolder(strFilePath))
	{
		SYSTEMTIME stNow;
		GetLocalTime(&stNow);
		CString strLogPath;
		strLogPath.Format(_T("%s\\domlogic_thread_process_%d_%02d_%02d.log"),strFilePath.c_str(),stNow.wYear,stNow.wMonth,stNow.wDay);
		CStdioFile	ff;
		if(ff.Open(strLogPath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
		{
			wstring strTime;
			Project::Tools::SysTime_To_String(stNow,strTime);
			CString strLog;
			strLog.Format(_T("%s    %s"),strTime.c_str(),strErrInfo);
			ff.Seek(0,CFile::end);
			ff.WriteString(strLog);
			ff.Close();
			return true;
		}
	}
	return false;
}


bool CLogicDllThread::AddThreadCrashLog( CString strErrInfo)
{
	wstring strFilePath;
	Project::Tools::GetSysPath(strFilePath);
	strFilePath += L"\\log";
	if(Project::Tools::FindOrCreateFolder(strFilePath))
	{
		SYSTEMTIME stNow;
		GetLocalTime(&stNow);
		CString strLogPath;
		strLogPath.Format(_T("%s\\domlogic_thread_crash.log"),strFilePath.c_str());
		CStdioFile	ff;
		if(ff.Open(strLogPath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite |CFile::modeWrite))
		{
			wstring strTime;
			Project::Tools::SysTime_To_String(stNow,strTime);
			CString strLog;
			strLog.Format(_T("%s    %s"),strTime.c_str(),strErrInfo);
			ff.Seek(0,CFile::end);
			ff.WriteString(strLog);
			ff.Close();
			return true;
		}
	}
	return false;
}

bool CLogicDllThread::Exit()
{
	SetEvent(m_hEventClose);

	DWORD WaitFlag = WaitForSingleObject(m_hThread_dll,INFINITE);

	if (m_hThread_dll != NULL)
	{
		CloseHandle(m_hThread_dll);
		m_hThread_dll = NULL;
	}

	bool bSuccess = true;
	unsigned int i=0;
	for(i=0;i<m_pLogicPipelineList.size();i++)
	{
		if(!m_pLogicPipelineList[i]->Exit())
			bSuccess = false;
	}

	for(i=0;i<m_pLogicPipelineList.size();i++)
	{
		SAFE_DELETE(m_pLogicPipelineList[i]);
	}

	m_pLogicPipelineList.clear();

	return bSuccess;
}


bool CLogicDllThread::GetRunStatus()
{
	return m_bRunning;
}


bool CLogicDllThread::SetRunStatus(bool runstatus)
{
	if(runstatus && !m_bRunning)
	{
		m_bNeedInit = true;
	}

	m_bRunning = runstatus ;
	return true;
}

void CLogicDllThread::DeleteDll( CDLLObject *pDLLObject )
{
	for(int i=0; i<m_vecImportDLLList.size(); ++i)
	{
		if(m_vecImportDLLList[i]->GetDllName() == pDLLObject->GetDllName())
		{
			m_vecImportDLLList.erase(m_vecImportDLLList.begin() + i);
			break;
		}
	}

	//从m_pLogicPipelineList中移除
	for(int i=0; i<m_pLogicPipelineList.size(); ++i)
	{
		CLogicPipeline *pLogicPipeline = m_pLogicPipelineList[i];
		if(pLogicPipeline && pLogicPipeline->FindDllObject(pDLLObject))
		{
			pLogicPipeline->Exit();
			m_pLogicPipelineList.erase(m_pLogicPipelineList.begin() + i);
			break;
		}
	}
}
