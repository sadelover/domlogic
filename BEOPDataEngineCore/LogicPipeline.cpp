#include "StdAfx.h"
#include "LogicPipeline.h"
#include "DLLObject.h"
#include "LogicBase.h"

CLogicPipeline::CLogicPipeline(CString strThreadName)
{
	m_fTimeSpanSeconds = 1e10;

	m_strThreadName = strThreadName;
}


CLogicPipeline::~CLogicPipeline(void)
{
}


bool CLogicPipeline::Init()
{
	for(int i=0;i<m_vecImportDLLList.size();i++)
	{
		if(m_vecImportDLLList[i]->GetLB() != NULL)
			m_vecImportDLLList[i]->GetLB()->Init();
	}

	m_Redis.init();
	return true;
}

bool CLogicPipeline::Exit()
{
	bool bSuccess = true;

	for(int i=0;i<m_vecImportDLLList.size();i++)
	{
		if(m_vecImportDLLList[i]->GetLB() != NULL)
			m_vecImportDLLList[i]->GetLB()->Exit();
	}

	return bSuccess;
}

bool CLogicPipeline::ActLogic(int nActCount)
{

	if(m_Redis.is_valid())
	{
		_tprintf(L"Redis Valid.\r\n");
	}
	else
	{
		_tprintf(L"Redis Not Valid.\r\n");
	}




	//Ö´ÐÐ²ßÂÔ
	bool bRunGood = true;
	for(int i=0;i<m_vecImportDLLList.size();i++)
	{
		CString strKey;
		strKey.Format(_T("LogicThread##heartbeat##%s##%s"),  m_strThreadName, m_vecImportDLLList[i]->GetDllName().c_str());
		COleDateTime tnow = COleDateTime::GetCurrentTime();
		CString strValue = tnow.Format(L"{\"time\": \"%Y-%m-%d %H:%M:%S\"}");

		if(m_Redis.is_valid())
		{
			m_Redis.set_value(strKey.GetString(), strValue.GetString());
		}

		if(!m_vecImportDLLList[i]->RunOnce(nActCount))
		{
			bRunGood = false;
		}

		CString strKey2;
		strKey2.Format(_T("LogicThread##actoncetime##%s##%s"), m_strThreadName, m_vecImportDLLList[i]->GetDllName().c_str());
		tnow = COleDateTime::GetCurrentTime();
		strValue = tnow.Format(L"{\"time\": \"%Y-%m-%d %H:%M:%S\"}");

		if(m_Redis.is_valid())
		{
			m_Redis.set_value(strKey2.GetString(), strValue.GetString());
		}


	}

	return bRunGood;
}


double CLogicPipeline::GetTimeSpanSeconds()
{
	Project::Tools::Scoped_Lock<Mutex>	guardlock(m_lock_timespan);
	return m_fTimeSpanSeconds;
}

void CLogicPipeline::SetTimeSpanSeconds(double fSeconds)
{
	Project::Tools::Scoped_Lock<Mutex>	guardlock(m_lock_timespan);
	m_fTimeSpanSeconds =  fSeconds;
}

bool CLogicPipeline::FindDllObject(CDLLObject *pObject)
{
	for(int i=0;i<m_vecImportDLLList.size();i++)
	{
		if(m_vecImportDLLList[i]->GetDllName()==pObject->GetDllName())
			return true;
	}

	return false;
}

void CLogicPipeline::PushLogicObject(CDLLObject *pObject)
{
	m_vecImportDLLList.push_back(pObject);

	if(m_fTimeSpanSeconds> pObject->GetTimeSpan())
		m_fTimeSpanSeconds = pObject->GetTimeSpan();

	if(m_fTimeSpanSeconds<=1)
			m_fTimeSpanSeconds=1;
}

CDLLObject * CLogicPipeline::GetLogicObject(int nIndex)
{
	return m_vecImportDLLList[nIndex];
}

int CLogicPipeline::GetLogicDllCount()
{
	return m_vecImportDLLList.size();
}

CString CLogicPipeline::GetLineString()
{
	CString strAll;
	for(int i=0;i<m_vecImportDLLList.size();i++)
	{
		if(m_vecImportDLLList[i])
		{

			strAll+= m_vecImportDLLList[i]->GetDllName().c_str();
			strAll+=_T(";");
		}
	}

	return strAll;
}