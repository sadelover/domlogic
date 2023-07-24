
#include "stdafx.h"
#include "BEOPDataAccess.h"
#include "../Tools/CustomTools/CustomTools.h"
#include "../Tools/Util/UtilString.h"
#include "../DB_BasicIO/DBAccessContainer.h"
#include "../Tools/Maths/MathCalcs.h"
#include <limits>
#include "../DB_BasicIO/BEOPDataBaseInfo.h"

#include "../BEOPLogicEngine/BeopGatewayCoreWrapper.h"

const wchar_t* defalutdbname = _T("beopdata");

CBEOPDataAccess::CBEOPDataAccess(CBeopGatewayCoreWrapper *pParentWrapper):
m_threadhandle_UpdatePhyPointValRecord(NULL)	
{
	m_eSysAction = Act_Empty;
	m_bExitThread = false;
	m_bThreadActive = true;

	m_pParentWrapper = pParentWrapper;

	m_nLogicLogLevel = 0;
}


CBEOPDataAccess::~CBEOPDataAccess(void)
{
	if (m_threadhandle_UpdatePhyPointValRecord != NULL){
		CloseHandle(m_threadhandle_UpdatePhyPointValRecord);
		m_threadhandle_UpdatePhyPointValRecord = NULL;
	}
}

bool CBEOPDataAccess::InitConnection(bool bRealtimeDB, bool bLogDB, bool bCommonDB)
{
	// Connect to the database server.
	bool bresult = false;
	if(bRealtimeDB)
	{
		bresult =	m_pRealTimeDataAccess.ConnectToDB(UtilString::ConvertMultiByteToWideChar(m_dbParam.strDBIP), 
			UtilString::ConvertMultiByteToWideChar(m_dbParam.strUserName), 
			UtilString::ConvertMultiByteToWideChar(m_dbParam.strDBPsw), 
			UtilString::ConvertMultiByteToWideChar(m_dbParam.strRealTimeDBName),
			m_dbParam.nPort);
		if (!bresult)
		{
			//::MessageBox(NULL, TEXT("Connect Mysql RealTime DB Fail！"), TEXT("RealTime DB Login"), MB_OK | MB_ICONERROR | MB_TOPMOST);
			return false;
		}


		////检测版本号
		//if(!GetMySQLDBVersion())
		//{
		//	::MessageBox(NULL, 
		//		TEXT("实时数据库版本过期, 请升级实时数据库后重试."), 
		//		TEXT("实时数据库登录"), 
		//		MB_OK | MB_ICONERROR | MB_TOPMOST);
		//	return false;

		//}

	}
	if(bCommonDB)
	{
		bresult = m_pCommonDBAccess.ConnectToDB(UtilString::ConvertMultiByteToWideChar(m_dbParam.strDBIP), 
			UtilString::ConvertMultiByteToWideChar(m_dbParam.strUserName), 
			UtilString::ConvertMultiByteToWideChar(m_dbParam.strDBPsw), 
			UtilString::ConvertMultiByteToWideChar(m_dbParam.strRealTimeDBName),
			m_dbParam.nPort);

		if (!bresult)
		{
			//::MessageBox(NULL, TEXT("Connect Mysql RealTime DB Fail！"), TEXT("RealTime DB Login"), MB_OK | MB_ICONERROR | MB_TOPMOST);
			return false;
		}

	}



	if(bLogDB)
	{
		bresult =	m_pLogDBAccess.ConnectToDB(UtilString::ConvertMultiByteToWideChar(m_dbParam.strDBIP), 
			UtilString::ConvertMultiByteToWideChar(m_dbParam.strUserName), 
			UtilString::ConvertMultiByteToWideChar(m_dbParam.strDBPsw), 
			UtilString::ConvertMultiByteToWideChar(m_dbParam.strRealTimeDBName),
			m_dbParam.nPort);
		if (!bresult)
		{
			//::MessageBox(NULL, TEXT("Connect Mysql RealTime DB Fail！"), TEXT("RealTime DB Login"), MB_OK | MB_ICONERROR | MB_TOPMOST);
			return false;
		}

	}


	StartUpdatePTValThread();

	return true;

}

bool CBEOPDataAccess::InitConnection2(void)
{
	// Connect to the database server.
	bool bresult =	m_pRealTimeDataAccess.ConnectToDB(UtilString::ConvertMultiByteToWideChar(m_dbParam.strDBIP2), 
		UtilString::ConvertMultiByteToWideChar(m_dbParam.strUserName), 
		UtilString::ConvertMultiByteToWideChar(m_dbParam.strDBPsw),
		UtilString::ConvertMultiByteToWideChar(m_dbParam.strRealTimeDBName),
		m_dbParam.nPort);

	if (!bresult)
	{
		//::MessageBox(NULL, TEXT("Connect Redundancy Mysql RealTime DB Fail！"), TEXT("Redundancy RealTime DB Login"), MB_OK | MB_ICONERROR | MB_TOPMOST);
		return false;
	}


	bresult =	m_pLogDBAccess.ConnectToDB(UtilString::ConvertMultiByteToWideChar(m_dbParam.strDBIP2), 
		UtilString::ConvertMultiByteToWideChar(m_dbParam.strUserName), 
		UtilString::ConvertMultiByteToWideChar(m_dbParam.strDBPsw),
		UtilString::ConvertMultiByteToWideChar(m_dbParam.strRealTimeDBName),
		m_dbParam.nPort);

	if (!bresult)
	{
		//::MessageBox(NULL, TEXT("Connect Redundancy Mysql RealTime DB Fail！"), TEXT("Redundancy RealTime DB Login"), MB_OK | MB_ICONERROR | MB_TOPMOST);
		return false;
	}

	bresult = m_pCommonDBAccess.ConnectToDB(UtilString::ConvertMultiByteToWideChar(m_dbParam.strDBIP2), 
		UtilString::ConvertMultiByteToWideChar(m_dbParam.strUserName), 
		UtilString::ConvertMultiByteToWideChar(m_dbParam.strDBPsw), 
		UtilString::ConvertMultiByteToWideChar(m_dbParam.strRealTimeDBName),
		m_dbParam.nPort);

	if (!bresult)
	{
		//::MessageBox(NULL, TEXT("Connect Redundancy Mysql RealTime DB Fail！"), TEXT("Redundancy RealTime DB Login"), MB_OK | MB_ICONERROR | MB_TOPMOST);
		return false;
	}

	return true;

}

bool CBEOPDataAccess::StartUpdatePTValThread(void)
{
	// create the threadd.
	UINT dwThreadId = 0;

	m_threadhandle_UpdatePhyPointValRecord = (HANDLE)_beginthreadex(NULL,
		0,
		ThreadFunc_UpdatePhyPointValRecord, 
		(LPVOID)this,
		0,
		&dwThreadId);
	ASSERT(m_threadhandle_UpdatePhyPointValRecord != NULL);
	return true;
}

void CBEOPDataAccess::SetDBparam(gDataBaseParam mdbParam)
{
	m_dbParam = mdbParam;
}

bool CBEOPDataAccess::DisConnection()
{
	m_pRealTimeDataAccess.DisConnectFromDB();

	m_pLogDBAccess.DisConnectFromDB();

	m_pCommonDBAccess.DisConnectFromDB();

	return true;
}

bool CBEOPDataAccess::GetPhyPointValueRecord()
{
	Project::Tools::Scoped_Lock<Mutex>	scope(m_lock);
	m_entrylist.clear();
	//m_mapPhyPointValueRecord.clear(); //golding add 2018-03-03, 不加的话core读不到数据了，策略仍然认为读到，会有问题
	if(m_pRealTimeDataAccess.ReadRealtimeData_Input(m_entrylist))
	{
		if (m_entrylist.size() == 0)
		{
			return false;
		}

		wstring phyID = _T("");
		gPhyPointValueStruct gPhyValueStruct;
		for (unsigned int i=0;i<m_entrylist.size();i++)
		{
			phyID = UtilString::ConvertMultiByteToWideChar(m_entrylist[i].GetName());

			gPhyValueStruct.time =m_entrylist[i].GetTimestamp();
			gPhyValueStruct.strValue =m_entrylist[i].GetValue();

		//	if(phyID==L"MSecChWTempReturn02")
		//	{
		//		_tprintf(L"MSecChWTempReturn02");
		//		_tprintf(m_entrylist[i].GetValue().c_str());
		//	}

			m_mapPhyPointValueRecord[phyID] = gPhyValueStruct;
		}
	}

	return true;
}


void CBEOPDataAccess::LogPointNotInMapError(wstring strPointName)
{
	CString strTemp = L"ERROR: " ;
	strTemp += strPointName.c_str();
	strTemp += L"  - Point Not Find!";
	wstring wstrTemp = strTemp.GetString();
//	InsertLog(wstrTemp);
}



/****************************************************************
*@brief: 获取数据(按类型，bool型)
*@param: const tstring strName 点位名称, bool &bParam参数数值
*@return: bool, true成功，false失败
*@author: ZHW 2010-08-13
*****************************************************************/
bool CBEOPDataAccess::GetValue(const tstring& strName, bool &bParam)
{
	Project::Tools::Scoped_Lock<Mutex>	scope(m_lock);
	if (m_mapPhyPointValueRecord.size() == 0)
	{
		return false;
	}

	hash_map<wstring, gPhyPointValueStruct>::const_iterator it = m_mapPhyPointValueRecord.find(strName);
	if (it != m_mapPhyPointValueRecord.end())
	{
		 double dvalue = _wtof(((*it).second).strValue.c_str());

		 if(dvalue == 0.f)
		 {
			 bParam = FALSE;
		 }
		 else
		 {
			 bParam = TRUE;
		 }

	}
	else
	{
		return false;
	}

	return true;
}


bool CBEOPDataAccess::SetValueMul( vector<wstring> & strNameList, vector<wstring> & strValueList )
{
	Project::Tools::Scoped_Lock<Mutex>	scope(m_lock);
	vector<CRealTimeDataEntry> rtList;
	for(int i=0;i<strNameList.size();i++)
	{
		hash_map<wstring, gPhyPointValueStruct>::const_iterator it = m_mapPhyPointValueRecord.find(strNameList[i]);
		if (it != m_mapPhyPointValueRecord.end())
		{
			CRealTimeDataEntry tRealTimeDataEntry;

			string strTemp = UtilString::ConvertWideCharToMultiByte(strNameList[i]);
			tRealTimeDataEntry.SetName(strTemp);
			tRealTimeDataEntry.SetValue(strValueList[i]);

			rtList.push_back(tRealTimeDataEntry);

			//SYSTEMTIME st;
			//GetLocalTime(&st);
			//tRealTimeDataEntry.SetTimestamp(st);

			
		}
		else
		{
			continue;
		}
	}
	if(rtList.size()>0)
	{
		bool bresult = m_pRealTimeDataAccess.UpdatePointData(rtList);
		return bresult;
	}

	return true;
	
}

bool CBEOPDataAccess::GetValue(const tstring& strName, wstring &strParam)
 {
	 Project::Tools::Scoped_Lock<Mutex>	scope(m_lock);
	if (m_mapPhyPointValueRecord.size() == 0)
	{
		return false;
	}
	hash_map<wstring, gPhyPointValueStruct>::const_iterator it = m_mapPhyPointValueRecord.find(strName);
	if (it != m_mapPhyPointValueRecord.end())
	{
		strParam = ((*it).second).strValue;
	}
	else
	{
		return false;
	}

	return true;
}

bool CBEOPDataAccess::SetValueDirect(const tstring & strName,const wstring & strParam)
{
	Project::Tools::Scoped_Lock<Mutex>	scope(m_lock);
	hash_map<wstring, gPhyPointValueStruct>::const_iterator it = m_mapPhyPointValueRecord.find(strName);
	if (it != m_mapPhyPointValueRecord.end())
	{
		gPhyPointValueStruct gPhyValueStruct;
		gPhyValueStruct.time = (*it).second.time;
		gPhyValueStruct.strValue = strParam;
		m_mapPhyPointValueRecord[strName] = gPhyValueStruct;
	}
	else
	{
		return false;
	}

	return true;
}


bool CBEOPDataAccess::SetValue( const tstring& strName, wstring nParam )
{
	Project::Tools::Scoped_Lock<Mutex>	scope(m_lock);
	hash_map<wstring, gPhyPointValueStruct>::const_iterator it = m_mapPhyPointValueRecord.find(strName);
	if (it != m_mapPhyPointValueRecord.end())
	{
		CRealTimeDataEntry tRealTimeDataEntry;

		string strTemp = UtilString::ConvertWideCharToMultiByte(strName);
		tRealTimeDataEntry.SetName(strTemp);
		tRealTimeDataEntry.SetValue(nParam);

		//SYSTEMTIME st;
		//GetLocalTime(&st);
		//tRealTimeDataEntry.SetTimestamp(st);

		bool bresult = m_pRealTimeDataAccess.UpdatePointData(tRealTimeDataEntry);
		return bresult;
	}
	else
	{
		return false;
	}

	return true;
}

///****************************************************************
//*@brief: 获取数据(按类型，double型)
//*@param: const tstring strName 点位名称, double &dbParam参数数值
//*@return: bool, true成功，false失败
//*@author: ZHW 2010-08-13
//*****************************************************************/
bool CBEOPDataAccess::GetValue(const tstring& strName, double &dbParam)
{
	Project::Tools::Scoped_Lock<Mutex>	scope(m_lock);
	if (m_mapPhyPointValueRecord.size() == 0)
	{
		return false;
	}
	
	hash_map<wstring, gPhyPointValueStruct>::const_iterator it = m_mapPhyPointValueRecord.find(strName);
	if (it != m_mapPhyPointValueRecord.end())
	{
		dbParam = _wtof(((*it).second).strValue.c_str());
	}
	else
	{
		return false;
	}

	return true;
}


/*******************************************************************
*@brief: 获取数据(按类型，unsigned int型)
*@param: const tstring strName 点位名称, unsigned int &uiParam参数数值
*@return: bool, true成功，false失败
*@author: ZHW 2010-08-13
********************************************************************/
bool CBEOPDataAccess::GetValue(const tstring& strName, int &uiParam)
{
	Project::Tools::Scoped_Lock<Mutex>	scope(m_lock);
	if (m_mapPhyPointValueRecord.size() == 0)
	{
		return false;
	}
	
	hash_map<wstring, gPhyPointValueStruct>::const_iterator it = m_mapPhyPointValueRecord.find(strName);
	if (it != m_mapPhyPointValueRecord.end())
	{
		double dbParam = _wtof(((*it).second).strValue.c_str());
		uiParam = (int)dbParam;
	}
	else
	{
		return false;
	}

	return true;
}

bool CBEOPDataAccess::SetValue( const tstring& strName, double dParam )
{

	Project::Tools::Scoped_Lock<Mutex>	scope(m_lock);
	hash_map<wstring, gPhyPointValueStruct>::const_iterator it = m_mapPhyPointValueRecord.find(strName);
	if (it != m_mapPhyPointValueRecord.end())
	{
		CRealTimeDataEntry tRealTimeDataEntry;

		string strTemp = UtilString::ConvertWideCharToMultiByte(strName);
		tRealTimeDataEntry.SetName(strTemp);

		wchar_t wcChar[255];
		swprintf(wcChar, L"%lf", dParam);
		wstring strValueSet = wcChar;
		tRealTimeDataEntry.SetValue(strValueSet);

		//SYSTEMTIME st;
		//GetLocalTime(&st);
		//tRealTimeDataEntry.SetTimestamp(st);

		bool bresult = m_pRealTimeDataAccess.UpdatePointData(tRealTimeDataEntry);
		return bresult;
	}
	else
	{
		return false;
	}

	return false;
}

bool CBEOPDataAccess::SetValue( const tstring& strName, bool bParam )
{
	Project::Tools::Scoped_Lock<Mutex>	scope(m_lock);
	hash_map<wstring, gPhyPointValueStruct>::const_iterator it = m_mapPhyPointValueRecord.find(strName);
	if (it != m_mapPhyPointValueRecord.end())
	{
		CRealTimeDataEntry tRealTimeDataEntry;

		string strTemp = UtilString::ConvertWideCharToMultiByte(strName);
		tRealTimeDataEntry.SetName(strTemp);

		int nVal = (bParam)?1:0;
		tRealTimeDataEntry.SetValue(to_wstring((long long)nVal));

		//SYSTEMTIME st;
		//GetLocalTime(&st);
		//tRealTimeDataEntry.SetTimestamp(st);

		bool bresult = m_pRealTimeDataAccess.UpdatePointData(tRealTimeDataEntry);
		return bresult;
	}
	else
	{
		return false;
	}

	return false;
}

bool CBEOPDataAccess::SetValue( const tstring& strName, int nParam )
{
	Project::Tools::Scoped_Lock<Mutex>	scope(m_lock);
	hash_map<wstring, gPhyPointValueStruct>::const_iterator it = m_mapPhyPointValueRecord.find(strName);
	if (it != m_mapPhyPointValueRecord.end())
	{
		CRealTimeDataEntry tRealTimeDataEntry;

		string strTemp = UtilString::ConvertWideCharToMultiByte(strName);
		tRealTimeDataEntry.SetName(strTemp);

		tRealTimeDataEntry.SetValue(to_wstring((long long)nParam));

		//SYSTEMTIME st;
		//GetLocalTime(&st);
		//tRealTimeDataEntry.SetTimestamp(st);

		bool bresult = m_pRealTimeDataAccess.UpdatePointData(tRealTimeDataEntry);
		return bresult;
	}
	else
	{
		return false;
	}

	return false;

}


void CBEOPDataAccess::TerminateAllThreads()
{
	m_bExitThread = true;
	if(m_threadhandle_UpdatePhyPointValRecord)
		WaitForSingleObject(m_threadhandle_UpdatePhyPointValRecord,INFINITE);

}


void CBEOPDataAccess::SetDataUpdate(bool bUpdate)
{

	m_bThreadActive = bUpdate;
	
}

UINT WINAPI CBEOPDataAccess::ThreadFunc_UpdatePhyPointValRecord( LPVOID lparam )
{
	CBEOPDataAccess* pThis = (CBEOPDataAccess*)lparam;
	if (pThis != NULL)
	{
		while(TRUE)
		{
			if(pThis->m_bExitThread)
			{
				return 0;
			}
			if(pThis->m_bThreadActive)
			{
				pThis->UpdatePhyPointValRecord();
			}
			Sleep(2*1000);
		}

	}

	return 0;
}

//UINT WINAPI CBEOPDataAccess::ThreadFunc_WriteDeviceWithRealDBValue( LPVOID lparam )
//{
//	COptimizeLogic* pThis = (COptimizeLogic*)lparam;
//	if (pThis != NULL)
//	{
//		while(TRUE)
//		{
//			pThis->WriteDeviceWithRealDBValue();
//
//			Sleep(1*1000);
//		}
//	}
//
//	return 0;
//}

void CBEOPDataAccess::UpdatePhyPointValRecord()
{
	GetPhyPointValueRecord();
}
void CBEOPDataAccess::SetSystemAction(Beop_SYSTEM_ACTION eAction)
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);
	int SysAction = -1;
	m_eSysAction = eAction;
	CString strTmp("");
	switch (eAction)
	{
	case Act_Empty:
		strTmp = L"空闲";
		SysAction = 0;
		break;
	case Act_RunOptimizing:
		strTmp = L"优化执行中";
		SysAction = 1;
		break;
	case  Act_Starting:
		strTmp = _T("开机中");
		SysAction = 2;
		break;
	case Act_Closing:
		strTmp = _T("关机中");
		SysAction = 3;
		break;
	case Act_Standbying:
		strTmp = _T("待机中");
		SysAction = 4;
		break;
	case Act_CHProtecting:
		strTmp = _T("排热量保护中");
		SysAction = 5;
		break;

	default:
		break;
	}
	SetValue(_T("Beopsystemstate"),SysAction);
	m_strSysAction = strTmp;
}

Beop_SYSTEM_ACTION CBEOPDataAccess::GetSystemAction()
{
	Project::Tools::Scoped_Lock<Mutex> scopelock(m_lock);
	return m_eSysAction;
}

CString& CBEOPDataAccess::GetActionString()
{
	return m_strSysAction;
}

bool CBEOPDataAccess::IsSystemFree()
{
	return m_eSysAction == Act_Empty;
}


void CBEOPDataAccess::SetDataBaseParam(gDataBaseParam dataBaseParam)
{
	m_dbParam.strDBIP = dataBaseParam.strDBIP;
	
	m_dbParam.strDBName = dataBaseParam.strDBName;

	m_dbParam.strUserName = dataBaseParam.strUserName;
	m_dbParam.nPort = dataBaseParam.nPort;
}


bool	CBEOPDataAccess::ReadLog(vector< pair<wstring,wstring> >& loglist, const SYSTEMTIME& start, const SYSTEMTIME& end)
{
	bool hasLogInfo = m_pLogDBAccess.ReadLog(loglist, start, end);

	if(!hasLogInfo)
	{
		return false;
	}

	return true;

}

bool	CBEOPDataAccess::ReadLog(vector< pair<wstring,wstring> >& loglist, const int nCount)
{
	bool hasLogInfo = m_pLogDBAccess.ReadLog(loglist, nCount);

	if(!hasLogInfo)
	{
		return false;
	}

	return true;
}

bool	CBEOPDataAccess::InsertLog(const vector<SYSTEMTIME> & sysTimeList, const vector<wstring> & loginfoList)
{
	if(m_nLogicLogLevel<0)
	{
		_tprintf(L"logic_log_level=-1,will not save log.(set 0 to save).\r\n");
		return true;
	}

	vector<wstring> wstrLogicNameList;
	for(int i=0;i<loginfoList.size();i++)
	{
		CString strOne = loginfoList[i].c_str();
		int aIndex = strOne.Find(_T("["));
		int bIndex = strOne.Find(_T("]"));
		if(aIndex<0  || bIndex<0 || bIndex<=(aIndex+1))
		{
			wstrLogicNameList.push_back(L"Unknown");
		}
		else
		{
			CString strSub = strOne.Mid(aIndex+1, bIndex-aIndex-1);
			wstrLogicNameList.push_back(strSub.GetString());
		}

	}

	return m_pLogDBAccess.InsertLogToDiskFile(wstrLogicNameList, sysTimeList, loginfoList);
}

bool	CBEOPDataAccess::InsertLogOneRecord(SYSTEMTIME sysTime, const wstring &loginfo)
{

	return true;
}

bool	CBEOPDataAccess::InsertLog(const wstring& loginfo)
{
	if(m_nLogicLogLevel<0)
		return true;

	if(loginfo.length() <=0)
		return false;

	


	bool bInsertLog = m_pLogDBAccess.InsertLog(loginfo);

	if(!bInsertLog)
	{
		return false;
	}

	return true;
}

bool    CBEOPDataAccess::DeleteLog(const SYSTEMTIME tEnd)
{
	return m_pLogDBAccess.DeleteLog(tEnd);
}

hash_map<wstring, gPhyPointValueStruct> CBEOPDataAccess::GetValueRecordMap()
{
	return m_mapPhyPointValueRecord;
}

bool    CBEOPDataAccess::AddOperationRecord(const wstring &strUserName, const wstring & strOperationContent)
{
	SYSTEMTIME st;
	GetLocalTime(&st);

	return InsertOperationRecord(st, strOperationContent, strUserName);
}

bool	CBEOPDataAccess::AddWarning(int nWarningLevel , const wstring &strWarningContent, const wstring &strBindPoint)
{
	CWarningEntry entry;
	entry.SetWarningLevel(nWarningLevel);
	entry.SetWarningInfo(strWarningContent);
	entry.SetWarningPointName(strBindPoint);
	bool bAddWarning = m_pCommonDBAccess.AddWarning( entry);

	if(!bAddWarning)
	{
		return false;
	}

	return true;
}

bool	CBEOPDataAccess::ReadWarning(vector<CWarningEntry>& resultlist, const SYSTEMTIME& start, const SYSTEMTIME& end)
{
	bool bReadWarning = m_pCommonDBAccess.ReadWarning( resultlist,  start,  end);

	if(!bReadWarning)
	{
		return false;
	}

	return true;
}

bool CBEOPDataAccess::GetHistoryValue(const wstring& strName,const SYSTEMTIME &st,const int &nTimeFormat,  wstring &strParam)
{
	return m_pCommonDBAccess.GetHistoryValue(strName, st, nTimeFormat, strParam);
}

bool CBEOPDataAccess::GetHistoryValue(const wstring& strName,const SYSTEMTIME &stStart,const SYSTEMTIME &stEnd,const int &nTimeFormat,  wstring &strParam)
{
	return m_pCommonDBAccess.GetHistoryValue(strName, stStart,stEnd, nTimeFormat, strParam);
}

bool CBEOPDataAccess::UpdateSavePoints(const vector<DataPointEntry>& ptList)
{
	return m_pCommonDBAccess.UpdateSavePoints(ptList);
}

bool	CBEOPDataAccess::DeleteWarning()
{
	bool bDeleteWarning = m_pCommonDBAccess.DeleteWarning();

	if(!bDeleteWarning)
	{
		return false;
	}

	return true;
}


bool	CBEOPDataAccess::ReadWarningConfig(vector<CWarningConfigItem>& configList)
{
	return m_pCommonDBAccess.ReadWarningConfig(configList);
}


wstring     CBEOPDataAccess::ReadOrCreateCoreDebugItemValue(wstring strItemName)
{
	return m_pCommonDBAccess.ReadOrCreateCoreDebugItemValue(strItemName);
}

wstring CBEOPDataAccess::ReadOrCreateCoreDebugItemValueMul(vector<wstring> strItemNameList, vector<wstring> & strItemValueList)
{
	return m_pCommonDBAccess.ReadOrCreateCoreDebugItemValueMul(strItemNameList, strItemValueList);
}

bool   CBEOPDataAccess::GetLogicBeginEndStatusErr(CString strDllName, CString &strInfo)
{
	return m_pCommonDBAccess.GetLogicBeginEndStatusErr(strDllName, strInfo);
}

bool CBEOPDataAccess::ClearCoreDebugItemValueLike(wstring strKeyLike)
{
	return m_pCommonDBAccess.ClearCoreDebugItemValueLike(strKeyLike);
}
bool     CBEOPDataAccess::ReadCoreDebugItemValue(wstring strItemName, wstring &strItemValue)
{
	return m_pCommonDBAccess.ReadCoreDebugItemValue(strItemName, strItemValue);
}

bool     CBEOPDataAccess::ReadCoreDebugItemValue(wstring strItemName, wstring strItemName2,wstring &strItemValue)
{
	return m_pCommonDBAccess.ReadCoreDebugItemValue(strItemName, strItemName2, strItemValue);
}

bool    CBEOPDataAccess::WriteCoreDebugItemValue(const wstring &strItemName, const wstring &strItemValue)
{
	return m_pCommonDBAccess.WriteCoreDebugItemValue(strItemName, strItemValue);
}


bool    CBEOPDataAccess::WriteCoreDebugItemValue(const wstring &strItemName, wstring strItemName2, const wstring &strItemValue)
{
	return m_pCommonDBAccess.WriteCoreDebugItemValue(strItemName, strItemName2, strItemValue);
}

bool    CBEOPDataAccess::WriteCoreDebugItemValue(const wstring &strItemName, wstring strItemName2,const wstring &strItemValue,const wstring &strItemValue2,const wstring &strItemValue3)
{
	return m_pCommonDBAccess.WriteCoreDebugItemValue(strItemName, strItemName2, strItemValue, strItemValue2, strItemValue3);
}

/************************************************************************/
/* 
  插入unit01表的两个字段，不带2后缀的是只利用一个字段
*/
/************************************************************************/
bool    CBEOPDataAccess::WriteCoreDebugItemValue2(const wstring &strItemName, const wstring &strItemValue)
{
	return m_pCommonDBAccess.WriteCoreDebugItemValue2(strItemName, strItemValue);
}

bool   CBEOPDataAccess::ClearLogicParameters()
{
	return m_pCommonDBAccess.ClearLogicParameters();
}

bool CBEOPDataAccess::UpdateHistoryStatusTable(string strProcessName)
{
	return m_pCommonDBAccess.UpdateHistoryStatusTable( strProcessName);
}

bool   CBEOPDataAccess::ReadLogicParameters(wstring &strThreadName, wstring &strLogicName, wstring &strSetType, wstring &strVariableName, wstring &strLinkName, wstring &strLinkType, wstring &strImportTime,wstring &strAuthor, wstring &strPeriod, wstring &strDllContent, wstring &strRunStatus,wstring &strVersion,wstring &strDescription,wstring &strDllOldName, wstring &strNeedSaveDBFile)
{
	return m_pCommonDBAccess.ReadLogicParameters(strThreadName, strLogicName, strSetType, strVariableName, strLinkName, strLinkType, strImportTime,strAuthor, strPeriod, strDllContent, strRunStatus, strVersion, strDescription, strDllOldName, strNeedSaveDBFile);
}
bool    CBEOPDataAccess::DeleteLogicParameters(wstring strThreadName, wstring strLogicName, wstring strSetType, wstring strVariableName, wstring strLinkName, wstring strLinkType)
{
	return m_pCommonDBAccess.DeleteLogicParameters(strThreadName, strLogicName, strSetType, strVariableName, strLinkName, strLinkType);
}

bool CBEOPDataAccess::InsertLogicParameters(wstring strThreadName, wstring strLogicName, wstring strSetType, wstring strVariableName, wstring strLinkName, wstring strLinkType, wstring strCondition)
{
	return m_pCommonDBAccess.InsertLogicParameters(strThreadName, strLogicName, strSetType, strVariableName, strLinkName, strLinkType,  strCondition);
}


bool	CBEOPDataAccess::InsertOperationRecord(const SYSTEMTIME& st, const wstring& remark, const wstring& wstrUser)
{
	bool bInsertOperation = m_pCommonDBAccess.InsertOperationRecord( st,  remark,  wstrUser);

	if(!bInsertOperation)
	{
		return false;
	}

	return true;

}

bool	CBEOPDataAccess::ReadAllOperationRecord(std::vector<COperationRecords>&  vecOperation, const COleDateTime& timeStart, const COleDateTime& timeStop)
{
	bool bReadOperation = m_pCommonDBAccess.ReadAllOperationRecord( vecOperation,  timeStart,  timeStop);

	if(!bReadOperation)
	{
		return false;
	}

	return true;
}

bool	CBEOPDataAccess::DeleteOperationRecord()
{
	bool bDeleteOperation = m_pCommonDBAccess.DeleteOperationRecord();

	if(!bDeleteOperation)
	{
		return false;
	}

	return true;
}

bool	CBEOPDataAccess::InsertLogicRecord(wstring strLogicName, wstring wstrTime, wstring strPointName, wstring strPointValue)
{
	if(m_nLogicLogLevel<0)
	{
		_tprintf(L"logic_log_level=-1,InsertLogicRecord not save log.(set 0 to save).\r\n");
		return true;
	}


	return m_pCommonDBAccess.InsertLogicRecord( strLogicName,  wstrTime,  strPointName,  strPointValue);
}

bool	CBEOPDataAccess::SetStatus(const wstring& statustype, int value)
{
	bool bSetStatus = m_pCommonDBAccess.SetStatus( statustype, value);

	if(!bSetStatus)
	{
		return false;
	}

	return true;
}


bool	CBEOPDataAccess::GetStatus(const wstring statustype, int nStatus)
{
	vector< pair<wstring, int> > statuslist;
	bool bGetStatus = m_pCommonDBAccess.GetStatus( statuslist);

	if(!bGetStatus)
	{
		return false;
	}

	return true;
}

bool	CBEOPDataAccess::ReadRealTimeData_Output(vector<CRealTimeDataEntry>& entrylist)
{
	bool bReadOutput = m_pRealTimeDataAccess.ReadRealTimeData_Output( entrylist);

	if(!bReadOutput)
	{
		return false;
	}

	return true;

}
bool	CBEOPDataAccess::UpdatePointData(const CRealTimeDataEntry& entry)
{
	bool bUpdatePointData = m_pRealTimeDataAccess.UpdatePointData( entry);

	if(!bUpdatePointData)
	{
		return false;
	}

	return true;
}
bool	CBEOPDataAccess::DeleteRealTimeData_Output()
{
	bool bDeleteOutput = m_pRealTimeDataAccess.DeleteRealTimeData_Output();

	if(!bDeleteOutput)
	{
		return false;
	}

	return true;
}

bool CBEOPDataAccess::UpdatePointValueInput(const wchar_t* lpszName, const wchar_t* lpszValue)
{
	ASSERT(lpszName);
	ASSERT(lpszValue);

	char szTemp[MAX_PATH] = {0};
	CRealTimeDataEntry entry;

	size_t converted = 0;
	wcstombs_s(&converted, szTemp, sizeof(szTemp), lpszName, sizeof(szTemp));
	entry.SetName(szTemp);

	entry.SetValue( lpszValue );
	return m_pRealTimeDataAccess.UpdatePointValue_Input(entry);
}


bool CBEOPDataAccess::UpdatePointValueBuffer(const wchar_t* lpszName, const wchar_t* lpszValue)
{
	ASSERT(lpszName);
	ASSERT(lpszValue);

	char szTemp[MAX_PATH] = {0};
	CRealTimeDataEntry entry;

	size_t converted = 0;
	wcstombs_s(&converted, szTemp, sizeof(szTemp), lpszName, sizeof(szTemp));
	entry.SetName(szTemp);

	entry.SetValue(lpszValue);
	return m_pRealTimeDataAccess.UpdatePointValue_InputBuffer(entry);
}

bool CBEOPDataAccess::UpdatePointValueOutput( const wchar_t* lpszName, const wchar_t* lpszValue )
{
	ASSERT(lpszName);
	ASSERT(lpszValue);

	char szTemp[MAX_PATH] = {0};
	CRealTimeDataEntry entry;

	size_t converted = 0;
	wcstombs_s(&converted, szTemp, sizeof(szTemp), lpszName, sizeof(szTemp));
	entry.SetName(szTemp);

	entry.SetValue(lpszValue);
	return UpdatePointData(entry);
}

bool CBEOPDataAccess::ExportLog( const SYSTEMTIME& start, const SYSTEMTIME& end, const wstring& filepath )
{
	return m_pLogDBAccess.ExportLog(start, end, filepath);
}


bool	CBEOPDataAccess::RegisterUserRecord(const CUserOnlineEntry& entry)
{
	bool bAddUserRecord = m_pCommonDBAccess.RegisterUserRecord( entry);

	if(!bAddUserRecord)
	{
		return false;
	}

	return true;
}


bool	CBEOPDataAccess::UpdateUserOnlineTime(wstring username, wstring usertype, SYSTEMTIME time)
{
	bool bUpdateUserOnlineTime = m_pCommonDBAccess.UpdateUserOnlineTime( username,  usertype, time);

	if(!bUpdateUserOnlineTime)
	{
		return false;
	}

	return true;
}



bool	CBEOPDataAccess::ReadUserRecord(vector<CUserOnlineEntry>& resultlist)
{
	bool bReadUserRecord = m_pCommonDBAccess.ReadUserRecord( resultlist);

	if(!bReadUserRecord)
	{
		return false;
	}

	return true;
}

bool	CBEOPDataAccess::DeleteUserRecord(wstring username, wstring usertype)
{
	bool bDeleteUserRecord = m_pCommonDBAccess.DeleteUserRecord( username, usertype);

	if(!bDeleteUserRecord)
	{
		return false;
	}

	return true;
}

bool	CBEOPDataAccess::IsUserOnline(wstring username, wstring usertype)
{
	return m_pCommonDBAccess.IsUserOnline( username, usertype);
}

bool CBEOPDataAccess::GetServerTime(SYSTEMTIME &time)
{
	return m_pCommonDBAccess.GetServerTime(time);
}

int	CBEOPDataAccess::GetClientUserCount()
{
	vector<CUserOnlineEntry> userOnlinelist;

	int nCount = 0;
	if(ReadUserRecord(userOnlinelist))
	{
		for(unsigned int i=0;i<userOnlinelist.size();i++)
		{
			if(_T("client") == userOnlinelist[i].GetUserType()) 
			{
				nCount++;				
			}
		}
	}

	return nCount;
}


bool	CBEOPDataAccess::InsertHistoryData(const SYSTEMTIME &st, const POINT_STORE_CYCLE &tCycle, const vector<CRealTimeDataEntry>& datalist)
{
	bool bInsertHistoryData = m_pCommonDBAccess.InsertHistoryData(st, tCycle, datalist);

	if(!bInsertHistoryData)
	{
		return false;
	}

	return true;
}

int CBEOPDataAccess::GetSpanTimeMonths( COleDateTime& start, const COleDateTime& end )
{	
	return (end.GetYear()-start.GetYear())*12 + (end.GetMonth()-start.GetMonth());
}

bool CBEOPDataAccess::DeleteEntryInOutput( const string& pointname )
{
	bool bDeleteOutput = m_pRealTimeDataAccess.DeleteEntryInOutput(pointname);

	if(!bDeleteOutput)
	{
		return false;
	}

	return true; 
}

CRealTimeDataAccess* CBEOPDataAccess::GetRealTimeDataAccess()
{
	if (&m_pRealTimeDataAccess)
	{
		return &m_pRealTimeDataAccess;
	}
	return NULL;
}


bool CBEOPDataAccess::GetPointExist(const wstring& strName)
{
	if (m_mapPhyPointValueRecord.size() == 0)
	{
		//LogPointNotInMapError(strName);
		//m_vctWrongPoints.push_back(strName);
		return false;
	}

	hash_map<wstring, gPhyPointValueStruct>::const_iterator it = m_mapPhyPointValueRecord.find(strName);
	if (it != m_mapPhyPointValueRecord.end())
	{
		return true;
	}
	
	return false;
	
}
bool CBEOPDataAccess::InsertS3DBPath( const wstring& s3dbpathname,const wstring& s3dbpathvalue )
{
	return m_pCommonDBAccess.InsertS3DBpath(s3dbpathname,s3dbpathvalue);
}

bool CBEOPDataAccess::GetS3DBPath(const wstring& s3dbpathname,wstring& s3dbpathvalue )
{
	return m_pCommonDBAccess.GetS3DBPath(s3dbpathname,s3dbpathvalue);
}


bool CBEOPDataAccess::DownloadFile(wstring strFileID, wstring strFileSavePath)
{

	return m_pCommonDBAccess.DownloadFile(strFileID, strFileSavePath);
}

bool	CBEOPDataAccess::UpdateLibInsertIntoFilestorage(const wstring strFilePathName, const wstring strFileName, const wstring strFileId)
{
	return m_pCommonDBAccess.UpdateLibInsertIntoFilestorage(strFilePathName, strFileName, strFileId);
}


bool CBEOPDataAccess::DeleteFile(wstring strFileID)
{

	return m_pCommonDBAccess.DeleteFile(strFileID);
}

wstring CBEOPDataAccess::ReadOrCreateCoreDebugItemValue_Defalut( wstring strItemName,wstring strValue /*= L"0"*/ )
{
	return m_pCommonDBAccess.ReadOrCreateCoreDebugItemValue_Defalut(strItemName,strValue);
}

bool CBEOPDataAccess::CreateScheduleTableIfNotExist()
{
	return m_pCommonDBAccess.CreateScheduleTableIfNotExist();
}

bool CBEOPDataAccess::GetScheduleInfo( const string strDate,const string strTimeFrom,const string strTimeTo, const int nDayOfWeek,vector<Beopdatalink::ScheduleInfo>& vecSchedule )
{
	return m_pCommonDBAccess.GetScheduleInfo(strDate,strTimeFrom,strTimeTo,nDayOfWeek,vecSchedule);
}

bool CBEOPDataAccess::GetEnvContent(int nEnvID, std::vector<wstring> &wstrPointNameList, std::vector<wstring> &wstrPointValueList)
{
	return m_pCommonDBAccess.GetEnvContent(nEnvID,wstrPointNameList,wstrPointValueList);
}

bool CBEOPDataAccess::GetDayCalendarPointSetting(COleDateTime tLastAct, COleDateTime tNow,std::map<wstring, wstring> &wstrPointSettingMap, vector<string> &strModeNameList,vector<int> &nTodayModeList, vector<int> & nTodayModeOfSystemList)
{
	return m_pCommonDBAccess.GetDayCalendarPointSetting(tLastAct, tNow, wstrPointSettingMap, strModeNameList, nTodayModeList, nTodayModeOfSystemList);
}

bool CBEOPDataAccess::SetDayCalendarMode(COleDateTime tNow, int nModeID, int nOfSystemID)
{
	return m_pCommonDBAccess.SetDayCalendarMode( tNow,  nModeID,  nOfSystemID);
}

bool CBEOPDataAccess::DeleteScheduleInfo( const int nID )
{
	return m_pCommonDBAccess.DeleteScheduleInfo(nID);
}

bool CBEOPDataAccess::GetScheduleExist()
{
	return m_pCommonDBAccess.GetScheduleExist();
}

bool CBEOPDataAccess::UpdateScheduleInfoExecuteDate( const int nID,const string strDate )
{
	return m_pCommonDBAccess.UpdateScheduleInfoExecuteDate(nID,strDate);
}


bool CBEOPDataAccess::GetValueByKey( const wstring& wstrKeyName, wstring &wstrValue)
{
	return m_pCommonDBAccess.ReadCoreDebugItemValue(wstrKeyName,wstrValue);
}
bool CBEOPDataAccess::SetValueByKey( const wstring& wstrKeyName, const wstring &wstrValue)
{
	return m_pCommonDBAccess.WriteCoreDebugItemValue(wstrKeyName,wstrValue);
}

bool CBEOPDataAccess::SaveSysDllInfo(wstring &wstrDllName, wstring &wstrDllDescription, wstring &wstrDllAuthor, wstring &wstrVersion, wstring &wstrInputs, wstring &wstrOutputs)
{
	return m_pCommonDBAccess.SaveSysDllInfo(wstrDllName,wstrDllDescription, wstrDllAuthor, wstrVersion, wstrInputs, wstrOutputs);
}


bool CBEOPDataAccess::RemoveKey( const wstring& wstrKeyName)
{
	return m_pCommonDBAccess.RemoveCoreDebugItemValue(wstrKeyName);

}


bool CBEOPDataAccess::GetAllPointRealtimeValues(vector<wstring> &strPointNameList,vector<SYSTEMTIME> &tPointTimeList, vector<wstring> &strPointValueList)
{
	strPointNameList.clear();
	tPointTimeList.clear();
	strPointValueList.clear();
	vector<CRealTimeDataEntry> entrylist;
	if(!m_pRealTimeDataAccess.ReadRealtimeData_Input(entrylist))
		return false;

	for(int i=0;i<entrylist.size();i++)
	{
		wstring wstrName;
		Project::Tools::String_To_Tstring(entrylist[i].GetName().data(), wstrName);

		wstring wstrValue = entrylist[i].GetValue();

		SYSTEMTIME tTime = entrylist[i].GetTimestamp();

		strPointNameList.push_back(wstrName);
		strPointValueList.push_back(wstrValue);
		tPointTimeList.push_back(tTime);
	}

	return true;
}


bool CBEOPDataAccess::AddVirtualPointToDBFile(const wstring & wstrPointName, const wstring & wstrPointDescription)
{
	if(m_pParentWrapper ==NULL)
		return false;

	m_pParentWrapper->AddVirtualPointToDBFile(wstrPointName, wstrPointDescription);
}


wstring CBEOPDataAccess::GetLocalConfig(const wstring & wstrConfigName)
{
	wstring wstr;
	if(m_pParentWrapper ==NULL)
		return wstr;

	return m_pParentWrapper->GetLocalConfig(wstrConfigName);
}


CString CBEOPDataAccess::GetServerIP()
{
	wstring wstr = Project::Tools::UTF8ToWideChar(m_dbParam.strDBIP.data());

	return wstr.c_str();
}


void CBEOPDataAccess::SetLogicBatchCount(int nCOunt)
{
	m_pLogDBAccess.m_nLogBatchCount = nCOunt;
}