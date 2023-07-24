#include "StdAfx.h"
#include "ScheduleThread.h"
#include <iostream>
#include "../Tools/Util/UtilString.h"

CScheduleThread::CScheduleThread(CBEOPDataAccess* pDataAccess, hash_map<wstring, wstring> &mapTimeDefine)
	:m_pDataAccess(pDataAccess)
	, m_bCreateTable(false)
	, m_bExitThread(false)
	, m_hSchedulehandle(NULL)
{
	m_mapScheduleTask.clear();

	m_mapTimeDef = mapTimeDefine;

}


CScheduleThread::~CScheduleThread(void)
{
	
}

bool CScheduleThread::Init()
{
	m_bExitThread = false;
	m_hSchedulehandle = (HANDLE) _beginthreadex(NULL, 0, ThreadFunc, this, NORMAL_PRIORITY_CLASS, NULL);
	return true;
}

bool CScheduleThread::Exit()
{
	m_bExitThread = true;
	m_mapScheduleTask.clear();
	if (m_hSchedulehandle != NULL)
	{
		::CloseHandle(m_hSchedulehandle);
		m_hSchedulehandle = NULL;
	}
	return true;
}

bool CScheduleThread::CreateTable()
{
	if(m_pDataAccess && !m_bCreateTable)
	{
		m_bCreateTable = m_pDataAccess->CreateScheduleTableIfNotExist();
	}
	return m_bCreateTable;
}

UINT WINAPI CScheduleThread::ThreadFunc( LPVOID lparam )
{
	CScheduleThread* pScheduleHandle = reinterpret_cast<CScheduleThread*>(lparam);
	if (NULL == pScheduleHandle)
	{
		return 0;
	}
	int nScanUserOperation = 1;		//25s扫描一次
	pScheduleHandle->CreateTable();		//创建表
	Sleep(1000);

	COleDateTime tActCalendar = COleDateTime::GetCurrentTime()- COleDateTimeSpan(0,0,0,30);
	while(!pScheduleHandle->m_bExitThread)
	{
		pScheduleHandle->CheckScehduleResult();

		//5中执行一次
		pScheduleHandle->HandleScheduleTask();
		int nPollSleep = 5;
		while (!pScheduleHandle->m_bExitThread)
		{
			if(nPollSleep <= 0)
			{
				break;
			}
			nPollSleep--;
			Sleep(1000);
		}

		//calendar
		COleDateTime tNow = COleDateTime::GetCurrentTime();
		COleDateTimeSpan tspan = tNow - tActCalendar;
		if(tspan.GetTotalSeconds()>=60)
		{
			
			pScheduleHandle->HandleDayCalendarTask(tActCalendar);

			tActCalendar = tNow;
		}
	}
	return 1;
}


bool CScheduleThread::HandleScheduleTask()
{
	if(m_pDataAccess)
	{
		COleDateTime oleFrom = COleDateTime::GetCurrentTime();
		CString strTimeFrom,strDate;
		strTimeFrom.Format(_T("%02d:%02d:00"),oleFrom.GetHour(),oleFrom.GetMinute());
		strDate.Format(_T("%04d-%02d-%02d"),oleFrom.GetYear(),oleFrom.GetMonth(),oleFrom.GetDay());
		string strTimeFrom_Ansi,strDate_Ansi;
		strTimeFrom_Ansi = Project::Tools::WideCharToAnsi(strTimeFrom);
		strDate_Ansi = Project::Tools::WideCharToAnsi(strDate);
		vector<Beopdatalink::ScheduleInfo> vecSchedule;
		vector<wstring> vecSettingPointNameList;
		vector<wstring> vecSettingValueList;
		if(m_pDataAccess->GetScheduleInfo(strDate_Ansi,strTimeFrom_Ansi,strTimeFrom_Ansi,oleFrom.GetDayOfWeek(),vecSchedule))
		{
			for(int i=0; i<vecSchedule.size(); ++i)
			{
				wstring strPointName = Project::Tools::AnsiToWideChar(vecSchedule[i].strPoint.c_str());
				

				if(vecSchedule[i].nType==0)
				{//普通点名组统一设置值
					wstring strPointValue = Project::Tools::AnsiToWideChar(vecSchedule[i].strValue.c_str());
					vector<wstring> vecPointName;
					Project::Tools::SplitStringByChar(strPointName.c_str(), ',', vecPointName);


					for(int j=0;j<vecPointName.size();j++)
					{
						wstring wstrCurValue ;
						bool bRead = m_pDataAccess->GetValue(vecPointName[j], wstrCurValue);
						if(bRead && wstrCurValue!=strPointValue)
						{
							m_pDataAccess->SetValue(vecPointName[j],strPointValue);
							SYSTEMTIME sExecute;
							GetLocalTime(&sExecute);
							vecSchedule[i].sExecute = sExecute;

							vecSchedule[i].bRealActed = true;

							wstring wstrTime;
							Project::Tools::SysTime_To_String(sExecute, wstrTime);
							m_pDataAccess->InsertLogicRecord(_T("SYS_Schedule"), wstrTime, strPointName, strPointValue);

							vecSettingPointNameList.push_back(strPointName);
							vecSettingValueList.push_back(strPointValue);

						}

					}
				}
				else if(vecSchedule[i].nType==1 && strPointName.length()>0)
				{//场景, pointname里存储的就是场景的ID

					int nEnvId = _wtoi(strPointName.c_str());
					vector<wstring> wstrPointNameList;
					vector<wstring> wstrPointValueList;
					m_pDataAccess->GetEnvContent(nEnvId, wstrPointNameList, wstrPointValueList);
					
					for(int j=0;j<wstrPointNameList.size();j++)
					{
						wstring wstrCurValue ;
						bool bRead = m_pDataAccess->GetValue(wstrPointNameList[j], wstrCurValue);
						if(bRead && wstrCurValue!=wstrPointValueList[j])
						{
							m_pDataAccess->SetValue(wstrPointValueList[j],wstrPointValueList[j]);
							SYSTEMTIME sExecute;
							GetLocalTime(&sExecute);
							vecSchedule[i].sExecute = sExecute;

							vecSchedule[i].bRealActed = true;

							wstring wstrTime;
							Project::Tools::SysTime_To_String(sExecute, wstrTime);
							m_pDataAccess->InsertLogicRecord(_T("SYS_Schedule"), wstrTime, wstrPointNameList[j], wstrPointValueList[j]);

							vecSettingPointNameList.push_back(wstrPointNameList[j]);
							vecSettingValueList.push_back(wstrPointValueList[j]);

						}
					}

				}
				
				m_mapScheduleTask[vecSchedule[i].nID] = vecSchedule[i];

				
				
			}

			//check and reset
			vector<bool> bGoodList;
			bGoodList.resize(vecSettingPointNameList.size(), false);
			for(int xx=0;xx<vecSettingPointNameList.size();xx++)
			{
				int nWaitMaxCount = 10;//2秒*10
				while(nWaitMaxCount>0 && bGoodList[xx]==false)
				{
					Sleep(3000); 
					wstring wstrReadValue;
					bool bRead = m_pDataAccess->GetValue(vecSettingPointNameList[xx], wstrReadValue);
					if(bRead && wstrReadValue==vecSettingValueList[xx])
					{
						bGoodList[xx] = true;
					}
					nWaitMaxCount--;
					
					//再设置一次
					m_pDataAccess->SetValue(vecSettingPointNameList[xx], vecSettingValueList[xx]);
					SYSTEMTIME sExecute;
					GetLocalTime(&sExecute);
					wstring wstrTime;
					Project::Tools::SysTime_To_String(sExecute, wstrTime);
					m_pDataAccess->InsertLogicRecord(_T("SYS_Schedule"), wstrTime, vecSettingPointNameList[xx], vecSettingValueList[xx]);
				}
			}

			bool bFailed;
			for(int xx=0;xx<bGoodList.size();xx++)
			{
				if(bGoodList[xx]==false)
				{
					CString strTemp;
					strTemp.Format(_T("日程设置(%s)失败,指令值期望为%s,但设置反馈检测不成功"), vecSettingPointNameList[xx].c_str(), vecSettingValueList[xx].c_str());
					wstring wstrContent = strTemp.GetString();
					SYSTEMTIME st;
					GetLocalTime(&st);
					m_pDataAccess->InsertOperationRecord(st, wstrContent, L"Schedule");
				}
			}

			return true;
		}
	}
	return false;
}



bool CScheduleThread::HandleDayCalendarTask(COleDateTime tLastAct)
{
	if(m_pDataAccess)
	{
		COleDateTime tnow = COleDateTime::GetCurrentTime();


		
		vector<wstring> wstrPointNameList;
		vector<wstring> wstrPointValueSettinList;

		std::map<wstring, wstring>  wstrPointSettingMap;


		std::map<wstring, wstring>  wstrTimeMap; //系统时间与点名的对应关系


		vector<string> strModeNameList;
		vector<int> nTodayModeList;
		vector<int> nTodayModeOfSystemList;
		m_pDataAccess->GetDayCalendarPointSetting(tLastAct, tnow,  wstrPointSettingMap, strModeNameList, nTodayModeList, nTodayModeOfSystemList);


		CString strInfo ;
		strInfo.Format(_T("[SYS_Calendar]Calendar Act Start===(Command:%d count)=====\r\n"), wstrPointSettingMap.size());
		_tprintf(strInfo.GetString());
		m_pDataAccess->InsertLog(strInfo.GetString());
		
		vector<int> nSystemCurMode;
		nSystemCurMode.resize(10, -1);
		for(int mm=0;mm<nTodayModeList.size();mm++)
		{
			if(mm<nTodayModeOfSystemList.size())
			{
				if(nTodayModeOfSystemList[mm]<nSystemCurMode.size() && nTodayModeOfSystemList[mm]>=0)
				{
					nSystemCurMode[nTodayModeOfSystemList[mm]] = nTodayModeList[mm];
				}
				
			}
		}

		vector<wstring> domSysModeNameList;
		vector<wstring> domSysModeValueList;
		for(int ss=0;ss<nSystemCurMode.size();ss++)
		{
			//檢測模式是否被改变
			CString strModeSettingPointName;
			strModeSettingPointName.Format(_T("dom_system_mode_setting_of_sys%d"), ss);
			if(m_pDataAccess->GetPointExist(strModeSettingPointName.GetString()))
			{
				int nSettingValue = -1;
				m_pDataAccess->GetValue(strModeSettingPointName.GetString(), nSettingValue);

				if(nSystemCurMode[ss]!=nSettingValue)
				{

					CString strInfo ;
					strInfo.Format(_T("[SYS_Calendar]Calendar Got Command of System(%d) Mode Setting: from %d to %d\r\n"), ss, nSystemCurMode[ss], nSettingValue);
					_tprintf(strInfo.GetString());
					m_pDataAccess->InsertLog(strInfo.GetString());

					m_pDataAccess->SetDayCalendarMode(tnow, nSettingValue, ss);
				}
				
			}

			CString strModePointName;
			strModePointName.Format(_T("dom_system_mode_of_sys%d"), ss);
			if(m_pDataAccess->GetPointExist(strModePointName.GetString()))
			{
				domSysModeNameList.push_back(strModePointName.GetString());
				CString strModeValue;
				strModeValue.Format(_T("%d"), nSystemCurMode[ss]);
				domSysModeValueList.push_back(strModeValue.GetString());
			}
		}
		if(domSysModeValueList.size()>0)
		{
			m_pDataAccess->SetValueMul(domSysModeNameList, domSysModeValueList);
		}


		std::map<wstring, wstring>::iterator iter = wstrPointSettingMap.begin();

		vector<wstring> vecSettingPointNameList;
		vector<wstring> vecSettingValueList;
		vector<wstring> vecBatchOnceSettingPointNameList;
		vector<wstring> vecBatchOnceSettingValueList;


		
		while(iter != wstrPointSettingMap.end())
		{

			wstring wstrCurValue ;
			bool bRead = m_pDataAccess->GetValue(iter->first, wstrCurValue);
			if(bRead && wstrCurValue!= iter->second)
			{
				bool bNeedSendCommand = true;
				if(UtilString::CheckStringIsNumber(wstrCurValue) && UtilString::CheckStringIsNumber(iter->second))
				{
					double fCurValue = _wtof(wstrCurValue.c_str());
					double fSetValue = _wtof(iter->second.c_str());
					if(fabs(fCurValue-fSetValue)<=1e-5)
						bNeedSendCommand = false;
				}

				if(bNeedSendCommand)
				{
					vecBatchOnceSettingPointNameList.push_back(iter->first);
					vecBatchOnceSettingValueList.push_back(iter->second);
					vecSettingPointNameList.push_back(iter->first);
					vecSettingValueList.push_back(iter->second);

					if(vecBatchOnceSettingPointNameList.size()>=50)
					{
						//每多少个输出批量执行时，休息500ms
						m_pDataAccess->SetValueMul(vecBatchOnceSettingPointNameList, vecBatchOnceSettingValueList);

						wstring wstrTime;
						COleDateTime tnowCur = COleDateTime::GetCurrentTime();
						Project::Tools::OleTime_To_String(tnowCur, wstrTime);

						for(int xx=0;xx<vecBatchOnceSettingPointNameList.size();xx++)
						{
							if(xx>=vecBatchOnceSettingValueList.size())
								break;

							m_pDataAccess->InsertLogicRecord(_T("SYS_Calendar"), wstrTime, vecBatchOnceSettingPointNameList[xx], vecBatchOnceSettingValueList[xx]);
						}

						
						vecBatchOnceSettingPointNameList.clear();
						vecBatchOnceSettingValueList.clear();
						Sleep(500);
						
					}

				}

				

			}


			iter++;
		}

		if(vecSettingPointNameList.size()>0)
		{
			CString strAllCommandOperation;
			strAllCommandOperation.Format(_T("[SYS_Calendar]模式日程执行%s等%d条指令"), vecSettingPointNameList[0].c_str(), vecSettingPointNameList.size() );
			m_pDataAccess->InsertLog( strAllCommandOperation.GetString() );
		}

		 strInfo = _T("[SYS_Calendar]Calendar Act End=======================\r\n");
		_tprintf(strInfo.GetString());
		m_pDataAccess->InsertLog(strInfo.GetString());


		COleDateTime tstart = COleDateTime::GetCurrentTime();
		strInfo = _T("[SYS_Calendar]Calendar Check Start=======================\r\n");
		_tprintf(strInfo.GetString());
		m_pDataAccess->InsertLog(strInfo.GetString());

		//check and reset

		vector<wstring> vecFailedSettingPointNameList;
		vector<wstring> vecFailedSettingValueList;
		
		bool bAllSuccess = SetAndCheckPointValue(vecSettingPointNameList, vecSettingValueList, vecFailedSettingPointNameList, vecFailedSettingValueList );
		int nTryCount = 2;
		while(!bAllSuccess && nTryCount>0)
		{
			Sleep(10000);
			nTryCount--;

			//copy
			vecSettingPointNameList.clear();
			vecSettingValueList.clear();
			for(int xx=0;xx<vecFailedSettingPointNameList.size();xx++)
			{
				vecSettingPointNameList.push_back(vecFailedSettingPointNameList[xx]);
				vecSettingValueList.push_back(vecFailedSettingValueList[xx]);
			}
			bAllSuccess = SetAndCheckPointValue(vecSettingPointNameList, vecSettingValueList, vecFailedSettingPointNameList, vecFailedSettingValueList );
			
			CString strMsg;
			strMsg.Format(_T("[SYS_Calendar]Calendar Loop Setting Once:Setting %d count, Left Failed:%d count \r\n"),vecSettingPointNameList.size(),  vecFailedSettingValueList.size());
			m_pDataAccess->InsertLog(strMsg.GetString());
		}
		

		bool bFailed;
		for(int xx=0;xx<vecFailedSettingPointNameList.size();xx++)
		{
			
			CString strTemp;
			strTemp.Format(_T("[SYS_Calendar]模式日程设置(%s)失败,指令值期望为%s,但设置反馈检测不成功"), vecFailedSettingPointNameList[xx].c_str(), vecFailedSettingValueList[xx].c_str());
			wstring wstrContent = strTemp.GetString();
			//SYSTEMTIME st;
			//GetLocalTime(&st);
			//m_pDataAccess->InsertOperationRecord(st, wstrContent, L"模式日程");

			m_pDataAccess->InsertLog(strTemp.GetString());
			
		}


		strInfo = _T("[SYS_Calendar]Calendar Check End=======================\r\n");
		_tprintf(strInfo.GetString());
		m_pDataAccess->InsertLog(strInfo.GetString());	

		COleDateTime tend = COleDateTime::GetCurrentTime();
		double fCostSeconds = (tend-tstart).GetTotalSeconds();
		if(fCostSeconds>=60.0)
		{

			strInfo.Format(_T("[SYS_Calendar]Calendar Check Cost Time Too Long:%d seconds\r\n"), (int)fCostSeconds);
			m_pDataAccess->InsertLog(strInfo.GetString());	
		}
	}
	return false;
}

bool CScheduleThread::SetAndCheckPointValue(vector<wstring> &vecSettingPointNameList, vector<wstring> &vecSettingValueList,vector<wstring> &vecFailedSettingPointNameList,
	vector<wstring> &vecFailedSettingValueList)
{
	if(vecSettingPointNameList.size()==0)
		return true;

	CString strInfo;
	vecFailedSettingPointNameList.clear();
	vecFailedSettingValueList.clear();

	vector<wstring> vecRetrySettingPointNameList;
	vector<wstring> vecRetrySettingValueList;

	int nSettingCount = 0;
	for(int xx=0;xx<vecSettingPointNameList.size();xx++)
	{
		wstring wstrReadValue;
		bool bRead = m_pDataAccess->GetValue(vecSettingPointNameList[xx], wstrReadValue);
		if(bRead && wstrReadValue==vecSettingValueList[xx])
		{
			continue;
		}

		vecRetrySettingPointNameList.push_back(vecSettingPointNameList[xx]);
		vecRetrySettingValueList.push_back(vecSettingValueList[xx]);

		vecFailedSettingPointNameList.push_back(vecSettingPointNameList[xx]);
		vecFailedSettingValueList.push_back(vecSettingValueList[xx]);
		
	}


	//再设置一次
	m_pDataAccess->SetValueMul(vecRetrySettingPointNameList, vecRetrySettingValueList);

	SYSTEMTIME sExecute;
	GetLocalTime(&sExecute);
	wstring wstrTime;
	Project::Tools::SysTime_To_String(sExecute, wstrTime);
	for(int xx=0;xx<vecRetrySettingPointNameList.size();xx++)
	{
		if(xx>=vecSettingValueList.size())
			break;

		m_pDataAccess->InsertLogicRecord(_T("SYS_Calendar"), wstrTime, vecRetrySettingPointNameList[xx], vecSettingValueList[xx]);
	}


	if(vecSettingValueList.size()==0)
		return true;

	return false;
}


bool CScheduleThread::CheckScehduleResult()
{
	hash_map<int,Beopdatalink::ScheduleInfo>::iterator iter = m_mapScheduleTask.begin();
	while(iter != m_mapScheduleTask.end())
	{
		string strScheduleName = (*iter).second.strName;
		string strPointName = (*iter).second.strPoint;
		string strPointValue = (*iter).second.strValue;
		SYSTEMTIME sExecute = (*iter).second.sExecute;
		int		nLoop = (*iter).second.nLoop;
		int		nID = (*iter).first;
		bool bReadActed = (*iter).second.bRealActed;
		bool bUpdateSuccess = false;
		if(m_pDataAccess)
		{
			wstring strValue;
			if(m_pDataAccess->GetValue(Project::Tools::AnsiToWideChar(strPointName.c_str()),strValue))
			{
				if(strValue == Project::Tools::AnsiToWideChar(strPointValue.c_str()))
				{
					bUpdateSuccess = true;
				}
			}	
		}

		//写log
		if(m_pDataAccess)
		{
			if(bReadActed)
			{
				std::ostringstream sqlstream;
				sqlstream << "系统配置日程(" << strScheduleName << ")生效. 设置"<<strPointName<<"为" << strPointValue;
				wstring strValue = Project::Tools::AnsiToWideChar(sqlstream.str().c_str());
				m_pDataAccess->InsertLog(strValue);

				//写操作记录
				m_pDataAccess->InsertOperationRecord(sExecute,strValue,L"Schedule");
			}
			

			if(nLoop == 0)	//不重复	清除该条记录
			{
				m_pDataAccess->DeleteScheduleInfo(nID);
			}
			else
			{
				COleDateTime oleFrom = COleDateTime::GetCurrentTime();
				CString strDate;
				strDate.Format(_T("%04d-%02d-%02d"),oleFrom.GetYear(),oleFrom.GetMonth(),oleFrom.GetDay());
				string strDate_Ansi = Project::Tools::WideCharToAnsi(strDate);
				m_pDataAccess->UpdateScheduleInfoExecuteDate(nID,strDate_Ansi);
			}
		}

		//移除
		iter = m_mapScheduleTask.erase(iter);
	}
	return true;
}
