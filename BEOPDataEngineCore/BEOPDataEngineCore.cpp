// BeopDataEngineCore.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "BeopDataEngineCore.h"

#include "Tools/Util/UtilString.h"
#include "Tools/CustomTools/CustomTools.h"

#include "Tools/Util/UtilsFile.h"


//#include "Tools/vld.h"
#include "../DataEngineConfig/CDataEngineAppConfig.h"
#include "../LAN_WANComm/NetworkComm.h"


#include "BEOPDataPoint/DataPointManager.h"

#include "DB_BasicIO/RealTimeDataAccess.h"
//#include "../DTUDataUpload/DTUSender.h"
#include "DB_BasicIO/BEOPDataBaseInfo.h"

//#include "ControlEx/WarningDlg.h"

#include "EngineRegister.h"
#include "sstream"
#include "../BEOPDataPoint/sqlite/SqliteAcess.h"
#include "../ServerDataAccess/BEOPDataAccess.h"

#include "../LAN_WANComm/Tools/ToolsFunction/PingIcmp.h"


#include <iostream>
#include <sstream>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一的应用程序对象


using namespace std;



using Beopdatalink::CRealTimeDataAccess;


CBeopDataEngineCore::CBeopDataEngineCore(std::wstring strDBFilePath)
{
	m_bDBAcessSuccess = false;
	m_hThread_WarningScan = NULL;
	m_hThread_ReconRedundancyDB = NULL;

	m_nMainDropInterval = 30;
	m_nSendPackageInterval = 2;

	m_nSystemType = 0;
	m_bUpdateInputTableFromSite = false;

	m_strDBFileName = strDBFilePath;

	if(m_strDBFileName.length()>0)
		m_dbset = LoadConfig();



	m_pointmanager = NULL;

	m_dbcon = NULL;
	m_dbcon_common = NULL;
	m_dbcon_Redundancy = NULL;
	m_dbcon_log = NULL;
	//m_pdtusender = NULL;
	m_pEngineRegister= NULL;
	m_pDataAccess = NULL;

	m_bExitThread = false;
}

CBeopDataEngineCore::~CBeopDataEngineCore(void)
{

	if (m_pointmanager)
	{
		delete m_pointmanager;
		m_pointmanager = NULL;
	}


	if (m_dbcon)
	{
		delete m_dbcon;
		m_dbcon = NULL;
	}

	if (m_dbcon_common)
	{
		delete m_dbcon_common;
		m_dbcon_common = NULL;
	}

	if(m_dbcon_Redundancy)
	{
		delete m_dbcon_Redundancy;
		m_dbcon_Redundancy = NULL;
	}

	if (m_dbcon_log)
	{
		delete m_dbcon_log;
		m_dbcon_log = NULL;
	}

	/*if (m_pdtusender)
	{
		delete m_pdtusender;
		m_pdtusender = NULL;
	}
*/
	if(m_pEngineRegister)
	{
		delete m_pEngineRegister;
		m_pEngineRegister = NULL;
	}
}

void CBeopDataEngineCore::SetDataAccess(CBEOPDataAccess *pAccess)
{
	m_pDataAccess = pAccess;
}

bool CBeopDataEngineCore::Init(bool bDTUEnabled,bool bDTUChecked,bool bDTURecCmd,int nDTUPort,int nDTUSendMinType,bool bModbusReadOne,int nModbusInterval,bool bDTUDisableSendAllData,bool bTCPEnable,wstring strTCPName,wstring strTCPServerIP,int nTCPServerPort,int nSendAllData)
{

	m_pointmanager = new CDataPointManager();
	m_pointmanager->SetFilePath(m_strDBFileName);

	m_dbcon			= new CRealTimeDataAccess();
	m_dbcon_common	= new Beopdatalink::CCommonDBAccess();
	m_dbcon_Redundancy = new Beopdatalink::CCommonDBAccess();
	m_dbcon_log		= new Beopdatalink::CLogDBAccess();

	int ienableredundancy = CDataEngineAppConfig::GetInstance()->GetMasterSlaveEnable();

	if (ienableredundancy != 0)
	{

	}
	else
	{
		CNetworkComm::GetInstance()->SetMachineModes(true, true, false);
	}

	//m_pdtusender		= new CDTUSender(m_pDataLink,3);
	m_pEngineRegister	= new CEngineRegister(m_dbcon_common);


	return true;
}

void	CBeopDataEngineCore::SetMsgWnd(CWnd* msgwnd)
{

}

bool CBeopDataEngineCore::Exit()
{
	if(m_pointmanager==NULL)
		return true;

	m_pointmanager->Clear();

	if (m_dbcon_log)
	{
		if (m_dbcon_log->IsConnected())
		{
			m_dbcon_log->InsertLog(L"data engine exit!");
		}
	}

	_tprintf(L"Exit Datalink ");

	_tprintf(L"Success.\r\n");


	if (m_dbcon_common->IsConnected())
	{
		m_dbcon_common->DeleteUserRecord(L"admin", L"dataengine");
	}


	return true;
}


bool CBeopDataEngineCore::InitEngine()
{


	//bresult = m_pdtusender->InitDTU();


	m_dbcon->DeleteRealTimeData_Output();

	UINT nThreadId;
	m_hThread_WarningScan = (HANDLE)_beginthreadex(NULL,
		0,
		ThreadFunc_ScanWarning, 
		(LPVOID)this,
		CREATE_SUSPENDED,
		&nThreadId);


	::ResumeThread(m_hThread_WarningScan);


	return true;
}


void CBeopDataEngineCore::ReadRealDBparam(gDataBaseParam &dbParam, wstring strDefaultDB)
{

	string strUtf8;
	Project::Tools::WideCharToUtf8(m_strDBFileName, strUtf8);
	CSqliteAcess access(strUtf8);

	ostringstream sqlstream;
	sqlstream << "select * from project_config;";
	string sql_ = sqlstream.str();
	char *ex_sql = const_cast<char*>(sql_.c_str());
	int re = access.SqlQuery(ex_sql);

	while(true)
	{
		if(SQLITE_ROW != access.SqlNext())
		{
			break;
		}
		if(access.getColumn_Text(6) != NULL)
		{
			string  dbip(const_cast<char*>(access.getColumn_Text(6)));
			dbParam.strDBIP = dbip;
		}
		if(access.getColumn_Text(7) != NULL)
		{
			string  dbname(const_cast<char*>(access.getColumn_Text(7)));
			dbParam.strDBName = dbname;
		}

		if(access.getColumn_Text(8) != NULL)
		{
			string  rtdbname(const_cast<char*>(access.getColumn_Text(8)));
			dbParam.strRealTimeDBName = rtdbname;
		}

		if(access.getColumn_Text(9) != NULL)
		{
			string  uname(const_cast<char*>(access.getColumn_Text(9)));
			dbParam.strUserName = uname;
		}


		dbParam.nAutoLogin = access.getColumn_Int(10);

		dbParam.nDTUPortNumber = access.getColumn_Int(11);

		dbParam.nDTUBaudrate = access.getColumn_Int(12);

		//if(access.getColumn_Text(13)!=NULL)
		//{
		//	string  dbip2(const_cast<char*>(access.getColumn_Text(13)));
		//	dbParam.strDBIP2 = dbip2;
		//}
		dbParam.nPort = access.getColumn_Int(14);


		//强制置为beopdata
		dbParam.strRealTimeDBName = Project::Tools::WideCharToAnsi(strDefaultDB.c_str());

	}
	access.SqlFinalize();
	access.CloseDataBase();
}

double CBeopDataEngineCore::GetMySQLDBVersion(wstring wstrDBName)
{
	wstring strVersion;
	bool bresult = false;
	CDatabaseSeesion m_dbSession;
	bresult =	m_dbSession.ConnectToDB(UtilString::ConvertMultiByteToWideChar(m_dbset.strDBIP), 
		UtilString::ConvertMultiByteToWideChar(m_dbset.strUserName), 
		UtilString::ConvertMultiByteToWideChar(m_dbset.strDBPsw),
		TEXT(""),
		m_dbset.nPort);

	if(bresult)
	{
		std::ostringstream sqlstream;
		sqlstream << "SELECT * FROM " << Project::Tools::WideCharToAnsi(wstrDBName.c_str()) << ".beopinfo;";
		MYSQL_RES* result = m_dbSession.QueryAndReturnResult(sqlstream.str().c_str());
		if (result == NULL)
		{
			strVersion = L"0.0";
		}
		else
		{
			MYSQL_ROW  row = NULL;
			if(row = m_dbSession.FetchRow(result) )
			{
				strVersion = Project::Tools::UTF8ToWideChar(row[1]);
			}
		}

		m_dbSession.DisConnectFromDB();
	}
	double fVersion = _wtof(strVersion.c_str());
	return fVersion;
}

bool CBeopDataEngineCore::InitDBConnection()
{
	wstring hostname = UtilString::ConvertMultiByteToWideChar(m_dbset.strDBIP);
	wstring username = UtilString::ConvertMultiByteToWideChar(m_dbset.strUserName);
	wstring strPwd =  UtilString::ConvertMultiByteToWideChar(m_dbset.strDBPsw);
	wstring strDBName       = UtilString::ConvertMultiByteToWideChar(m_dbset.strRealTimeDBName);
	

	bool bresult1 =  m_dbcon->ConnectToDB(hostname,
		username,
		strPwd, 
		strDBName,
		m_dbset.nPort);

	if(!bresult1)
	{
		wstring strLogOut = L"ERROR: Database connect failed.\r\n";
		_tprintf(strLogOut.c_str());
		
		return false;
	}

	bool bresult2 = m_dbcon_common->ConnectToDB(hostname,
		username,
		strPwd, 
		strDBName,
		m_dbset.nPort);

	if(!bresult2)
	{
		wstring strLogOut = L"ERROR: Database connect failed.\r\n";
		_tprintf(strLogOut.c_str());
		return false;
	}

	if(m_dbset.strDBIP2.length()>=7)		//冗余连接
	{
		//网络通
		wstring rehostname = UtilString::ConvertMultiByteToWideChar(m_dbset.strDBIP2);
		bool bresult5 = m_dbcon_Redundancy->ConnectToDB(rehostname,
			username,
			strPwd, 
			strDBName,
			m_dbset.nPort);

		if(!bresult5)
		{
			wstring strLogOut = L"ERROR: Redundency Database connect failed.\r\n";
			_tprintf(strLogOut.c_str());
			
			//启动一个线程连接冗余数据库直到成功为止
			if(m_hThread_ReconRedundancyDB == NULL)
				m_hThread_ReconRedundancyDB = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc_ConnectRedundancyDB, this, NORMAL_PRIORITY_CLASS, NULL);
	
		}
	}

	bool bresult3 = m_dbcon_log->ConnectToDB(hostname,
		username,
		strPwd, 
		strDBName,
		m_dbset.nPort);

	if(!bresult3)
	{
		wstring strLogOut = L"ERROR: Database connect failed.\r\n";
		_tprintf(strLogOut.c_str());

		return false;
	}

	bool bresult4 = Beopdatalink::DTUDBAccessSingleton::GetInstance()->ConnectToDB(hostname,
		username,
		strPwd, 
		strDBName,
		m_dbset.nPort);

	if(!bresult4)
	{
		wstring strLogOut = L"ERROR: Database connect failed.\r\n";
		_tprintf(strLogOut.c_str());

		return false;
	}

	return true;
}

bool CBeopDataEngineCore::InitPoints()
{

	m_pointmanager->SetFilePath(m_strDBFileName);

	bool bInitSuccess = m_pointmanager->Init();

	if(!m_bUpdateInputTableFromSite)//仿真下，所有点位置为软点
	{
		m_pointmanager->SetAllPointTypeAsVirtual();
	}

	return bInitSuccess;
}


Beopdatalink::CRealTimeDataAccess* CBeopDataEngineCore::GetRealTimeDataAccess()
{
	return m_dbcon;
}
Beopdatalink::CCommonDBAccess * CBeopDataEngineCore::GetCommonDBAccess()
{
	return m_dbcon_common;
}
Beopdatalink::CLogDBAccess * CBeopDataEngineCore::GetLogDBAccess()
{
	return m_dbcon_log;
}

void CBeopDataEngineCore::SetRealTimeDataAccess(Beopdatalink::CRealTimeDataAccess* pAccess)
{
	m_dbcon = pAccess;
}
void CBeopDataEngineCore::SetCommonDBAccess(Beopdatalink::CCommonDBAccess *pAccess)
{
	m_dbcon_common = pAccess;
}
void CBeopDataEngineCore::SetLogDBAccess(Beopdatalink::CLogDBAccess *pAccess)
{
	m_dbcon_log = pAccess;
}
CDataPointManager* CBeopDataEngineCore::GetDataPointManager()
{
	if (m_pointmanager != NULL)
	{
		return m_pointmanager;
	}else
	{
		return NULL;
	}
	
}
//CBEOPDataAccess *CBeopDataEngineCore::GetServerDataAccess()
//{
//	return m_pDataAccess;
//}
void CBeopDataEngineCore::EnableLog(BOOL bEnable)
{
	m_bLog = bEnable;
}
//
//CDTUSender * CBeopDataEngineCore::GetDTUSender()
//{
//	return m_pdtusender;
//}


bool CBeopDataEngineCore::LoadDBSettingFromCoreConfig(DataBaseSetting &dbset)
{
	//强制置为beopdata
	wstring cstrFile;
	Project::Tools::GetSysPath(cstrFile);
	wstring exepath;
	exepath = cstrFile + L"\\domcore.ini";

	wchar_t charDBName[1024];
	GetPrivateProfileString(L"core", L"defaultdb", L"", charDBName, 1024, exepath.c_str());
	wstring strDefaultDB = charDBName;
	if(strDefaultDB == L"" || strDefaultDB == L"0")
		strDefaultDB = L"beopdata";

	dbset.strRealTimeDBName = Project::Tools::WideCharToAnsi(strDefaultDB.c_str());


	wchar_t charRootPassword[1024];
	GetPrivateProfileString(L"core", L"dbpassword", L"", charRootPassword, 1024, exepath.c_str());
	wstring wstrDBPassword = charRootPassword;
	if(wstrDBPassword == L"")
		wstrDBPassword = L"RNB.beop-2013";

	dbset.strDBPsw = Project::Tools::WideCharToAnsi(wstrDBPassword.c_str());


	wchar_t charServerIP[1024];
	GetPrivateProfileString(L"core", L"server", L"", charServerIP, 1024, exepath.c_str());
	wstring wstrServerIP = charServerIP;
	if(wstrServerIP == L"" || wstrServerIP == L"0")
		wstrServerIP = L"127.0.0.1";

	dbset.strDBIP = Project::Tools::WideCharToAnsi(wstrServerIP.c_str());

	return true;
}

DataBaseSetting CBeopDataEngineCore::LoadConfig()
{
	DataBaseSetting dbset;
	string strUtf8;
	Project::Tools::WideCharToUtf8(m_strDBFileName,strUtf8 );
	CSqliteAcess access(strUtf8);
	//m_project.ClearProject();
	ostringstream sqlstream;
	sqlstream << "select * from project_config;";
	string sql_ = sqlstream.str();
	//char *ex_sql = const_cast<char*>();
	int re = 0;
	re = access.SqlQuery(sql_.c_str());

	while(true)
	{
		if(SQLITE_ROW != access.SqlNext())
		{
			break;
		}
		int iProjectID = access.getColumn_Int(0);
		if(access.getColumn_Text(1) != NULL)
		{
			string   name_temp(const_cast<char*>(access.getColumn_Text(1)));
			wstring   name = UtilString::ConvertMultiByteToWideChar(name_temp);
		}
		int ScreenWidth = access.getColumn_Int(2);
		int ScreenHeight = access.getColumn_Int(3);
		int curVersion =   access.getColumn_Int(4);
		if(access.getColumn_Text(5) != NULL)
		{
			string   xupTime_temp(const_cast<char*>(access.getColumn_Text(5)));
			wstring   xupTime = UtilString::ConvertMultiByteToWideChar(xupTime_temp);
		}
		if(access.getColumn_Text(6) != NULL)
		{
			string  dbip(const_cast<char*>(access.getColumn_Text(6)));
			dbset.strDBIP = dbip;
		}
		if(access.getColumn_Text(7) != NULL)
		{
			string  dbname(const_cast<char*>(access.getColumn_Text(7)));
			dbset.strDBName = dbname;
		}
		if(access.getColumn_Text(8) != NULL)
		{
			string  rtdbname(const_cast<char*>(access.getColumn_Text(8)));
			dbset.strRealTimeDBName = rtdbname;
		}
		if(access.getColumn_Text(9) != NULL)
		{
			string  uname(const_cast<char*>(access.getColumn_Text(9)));
			dbset.strUserName = uname;
		}
		dbset.nAutoLogin = access.getColumn_Int(10);
		dbset.DTUPortNumber = access.getColumn_Int(11);
		dbset.DTUBaudrate = access.getColumn_Int(12);
		//if(access.getColumn_Text(13)!=NULL)
		//{
		//	string  dbip2(const_cast<char*>(access.getColumn_Text(13)));
		//	dbset.strDBIP2 = dbip2;
		//}
		dbset.nPort = access.getColumn_Int(14);
	}

	LoadDBSettingFromCoreConfig(dbset);



	access.SqlFinalize();
	access.CloseDataBase();
	return dbset;

}


bool CBeopDataEngineCore::SaveConfig(DataBaseSetting &dbset)
{
	string strUtf8;
	Project::Tools::WideCharToUtf8(m_strDBFileName, strUtf8);
	CSqliteAcess access(strUtf8);
	ostringstream sqlstream;
	sqlstream << "select * from project_config;";
	string sql_ = sqlstream.str();
	char *ex_sql = const_cast<char*>(sql_.c_str());
	int re = access.SqlQuery(ex_sql);

	int  iCount = 0;
	while(true)
	{
		if(SQLITE_ROW != access.SqlNext())
		{
			break;
		}
		iCount = access.getColumn_Int(0);
	}
	access.SqlFinalize();


	//////////////////////////////////////////////////////////////////////////
	//更新sql
	ostringstream sqlstream_set;
	sqlstream_set << "update project_config set HostIP = '"<< dbset.strDBIP <<"', DataBaseName =  '"<< dbset.strDBName <<"',RealTimeDBName = '"<< dbset.strRealTimeDBName <<"',UserName = '"\
		<< dbset.strUserName <<"', AutoLogin = "<< dbset.nAutoLogin << ", DTUPortNumber = "<< dbset.DTUPortNumber << ", DTUBaudrate = "<< dbset.DTUBaudrate <<", HostIP2 = '"<< dbset.strDBIP2 << "',port = "<< dbset.nPort<<";";
	sql_ = sqlstream_set.str();
	ex_sql = const_cast<char*>(sql_.c_str());
	re = access.SqlExe(ex_sql);

	access.CloseDataBase();
	if (re == SQLITE_OK)
	{
		return true;
	}else
	{
		return false;
	}
}

wstring CBeopDataEngineCore::GetConfigVersion()
{
	string strUtf8;
	Project::Tools::WideCharToUtf8(m_strDBFileName, strUtf8);
	CSqliteAcess access(strUtf8);

	ostringstream sqlstream;
	sqlstream << "select * from project_config;";
	string sql_ = sqlstream.str();
	char *ex_sql = const_cast<char*>(sql_.c_str());
	int re = access.SqlQuery(ex_sql);
	wstring version = L"";
	while(true)
	{
		if(SQLITE_ROW != access.SqlNext())
		{
			break;
		}
		if(access.getColumn_Text(4) != NULL)
		{
			string   version_temp(const_cast<char*>(access.getColumn_Text(4)));
			version   = UtilString::ConvertMultiByteToWideChar(version_temp).c_str();
		}
	}
	access.SqlFinalize();
	access.CloseDataBase();
	return version;
}

UINT WINAPI CBeopDataEngineCore::ThreadFunc_ScanWarning(LPVOID lparam)
{
	CBeopDataEngineCore * pEngineCore = (CBeopDataEngineCore*)lparam;


	while(true)
	{
		Sleep(2000);

		if(pEngineCore->m_bExitThread)
			break;

		if(!pEngineCore->m_bDBAcessSuccess)
			continue;

		std::vector<CWarningConfigItem> wcItemList;
		if(pEngineCore->m_pDataAccess==NULL)
			continue;

		pEngineCore->m_pDataAccess->ReadWarningConfig(wcItemList);
		unsigned int i = 0;
		for(i=0;i<wcItemList.size();i++)
		{
			if(pEngineCore->m_bExitThread)
			{
				pEngineCore->m_bThreadExitFinished = true;
				return 0;
			}

			wstring strValueCur;
			bool bGet = pEngineCore->m_pDataAccess->GetValue(wcItemList[i].m_strPointName, strValueCur);
			if(!bGet)
			{

				continue;
			}
	//		if(wcItemList[i].m_strPointName==L"MSecChWTempReturn02")
	//		{
	//			_tprintf(L"Scanwarning MSecChWTempReturn02");
	//			_tprintf(strValueCur.c_str());
	//		}
			SYSTEMTIME st;
			GetLocalTime(&st);

			if(wcItemList[i].m_nWarningType==1)
			{
				bool bErr = (_wtoi(strValueCur.c_str())==1);
				if(bErr)
				{
					wchar_t wcChar[1024];
					wstring strWTemp;
					if(wcItemList[i].m_strWarningContent.length()>0)
						strWTemp = wcItemList[i].m_strWarningContent.c_str();
					else
					{
						wsprintf(wcChar, L"%s Warning",wcItemList[i].m_strPointName.c_str());
						strWTemp = wcChar;
					}

					pEngineCore->m_pDataAccess->AddWarning(3,strWTemp, wcItemList[i].m_strPointName);

				}
			}
			else if(wcItemList[i].m_nWarningType==2)
			{//INT, diagnose报警
				bool bErr = (_wtoi(strValueCur.c_str())!=1);
				if(bErr)
				{
					wchar_t wcChar[1024];
					wstring strWTemp;
					if(wcItemList[i].m_strWarningContent.length()>0)
						strWTemp = wcItemList[i].m_strWarningContent.c_str();
					else
					{
						wsprintf(wcChar, L"%s Warning",wcItemList[i].m_strPointName.c_str());
						strWTemp = wcChar;
					}

					pEngineCore->m_pDataAccess->AddWarning(3,strWTemp, wcItemList[i].m_strPointName);

				}
			}
			else if(wcItemList[i].m_nWarningType==0)
			{
				double fValueCur = _wtof(strValueCur.c_str());
				if(fValueCur>= wcItemList[i].m_fLimitHH && wcItemList[i].m_bEnableHH)
				{//高高报

					wchar_t wcChar[1024];
					wstring strWTemp;

					if(wcItemList[i].m_strWarningContent_HH.length()>0)
						wsprintf(wcChar, L"%s", wcItemList[i].m_strWarningContent_HH.c_str());
					else
						wsprintf(wcChar, L"Higer Warning %s",wcItemList[i].m_strPointName.c_str());
					strWTemp = wcChar;
					pEngineCore->m_pDataAccess->AddWarning(3,strWTemp, wcItemList[i].m_strPointName);

				}
				else if(fValueCur>=wcItemList[i].m_fLimitH && wcItemList[i].m_bEnableH)
				{//高报
					wchar_t wcChar[1024];
					wstring strWTemp;

					if(wcItemList[i].m_strWarningContent_H.length()>0)
						wsprintf(wcChar, L"%s", wcItemList[i].m_strWarningContent_H.c_str());
					else
						wsprintf(wcChar, L"High Warning %s",wcItemList[i].m_strPointName.c_str());
					strWTemp = wcChar;
					pEngineCore->m_pDataAccess->AddWarning(2,strWTemp, wcItemList[i].m_strPointName);

				}

				if(fValueCur<=wcItemList[i].m_fLimitLL && wcItemList[i].m_bEnableLL)
				{//低低报
					wchar_t wcChar[1024];
					wstring strWTemp;
					if(wcItemList[i].m_strWarningContent_LL.length()>0)
						wsprintf(wcChar, L"%s", wcItemList[i].m_strWarningContent_LL.c_str());
					else
						wsprintf(wcChar, L"Lower Warning %s",wcItemList[i].m_strPointName.c_str());
					strWTemp = wcChar;
					pEngineCore->m_pDataAccess->AddWarning(3,strWTemp, wcItemList[i].m_strPointName);

				}
				else if(fValueCur<=wcItemList[i].m_fLimitL && wcItemList[i].m_bEnableL)
				{//低报
					wchar_t wcChar[1024];
					wstring strWTemp;
					if(wcItemList[i].m_strWarningContent_L.length()>0)
						wsprintf(wcChar, L"%s", wcItemList[i].m_strWarningContent_L.c_str());
					else
						wsprintf(wcChar, L"Low Warning %s",wcItemList[i].m_strPointName.c_str());
					strWTemp = wcChar;
					pEngineCore->m_pDataAccess->AddWarning(2,strWTemp, wcItemList[i].m_strPointName);
				}
			}

		}

	}

	pEngineCore->m_bThreadExitFinished = true;
	return 0;
}

bool CBeopDataEngineCore::SetDBAcessState( bool bState )
{
	m_bDBAcessSuccess = bState;
	return true;
}

bool CBeopDataEngineCore::TerminateScanWarningThread()
{
	m_bExitThread = true;
	//WaitForSingleObject(m_hThread_WarningScan, INFINITE);
	TerminateThread(m_hThread_WarningScan,0);
	if(m_hThread_WarningScan != NULL)
	{
		CloseHandle(m_hThread_WarningScan);
		m_hThread_WarningScan = NULL;
	}
	return true;
}

/**
  判断1、beopdata库存在
  同时2、beopinfo表存在
      
*/
bool CBeopDataEngineCore::CheckDBExist(wstring wstrDBName)
{

	bool bresult = false;
	CDatabaseSeesion	m_dbSession;
	bresult =	m_dbSession.ConnectToDB(UtilString::ConvertMultiByteToWideChar(m_dbset.strDBIP), 
	UtilString::ConvertMultiByteToWideChar(m_dbset.strUserName), 
	UtilString::ConvertMultiByteToWideChar(m_dbset.strDBPsw),
	TEXT("information_schema"),
	m_dbset.nPort);
	if(bresult)
	{
		std::ostringstream sqlstream;
		sqlstream << "SELECT * FROM information_schema.SCHEMATA where SCHEMA_NAME = '" << Project::Tools::WideCharToAnsi(wstrDBName.c_str()) << "';";
		MYSQL_RES* result = m_dbSession.QueryAndReturnResult(sqlstream.str().c_str());
		if (result == NULL){
			bresult =  false;
		}
		else
		{

			int length = static_cast<int>(m_dbSession.GetRowNum(result));
			if (0 == length){
				bresult =  false;
			}
			m_dbSession.FreeReturnResult(result);
			sqlstream.str("");
			sqlstream << "SELECT * FROM information_schema.`TABLES` where TABLE_SCHEMA ='" << Project::Tools::WideCharToAnsi(wstrDBName.c_str()) << "' and TABLE_NAME = 'beopinfo';";
			result = m_dbSession.QueryAndReturnResult(sqlstream.str().c_str());
			if (result == NULL){
				bresult =  false;
			}
			length = static_cast<int>(m_dbSession.GetRowNum(result));
			if (0 == length){
				bresult =  false;
			}
			m_dbSession.FreeReturnResult(result);
		}


		m_dbSession.DisConnectFromDB();
	}

	return bresult;
}

bool CBeopDataEngineCore::DropDatabase(wstring wstrDBName)
{
	bool bresult = false;
	CDatabaseSeesion	m_dbSession;
	bresult =	m_dbSession.ConnectToDB(UtilString::ConvertMultiByteToWideChar(m_dbset.strDBIP), 
		UtilString::ConvertMultiByteToWideChar(m_dbset.strUserName), 
		UtilString::ConvertMultiByteToWideChar(m_dbset.strDBPsw),
		TEXT(""),
		m_dbset.nPort);

	if(bresult)
	{
		bool bR = false;
		//删除并创建数据库
		char* cFind = "beopdata";
		char cReplace[56];
		memset(cReplace,0,56);
		string strDB_Ansi = Project::Tools::WideCharToAnsi(wstrDBName.c_str());
		memcpy(cReplace,strDB_Ansi.data(),strDB_Ansi.length());
		m_dbSession.Execute(RelaceStr(E_DROPDB_BEOP,cFind,cReplace));
	}

	m_dbSession.DisConnectFromDB();
	return true;
}
/************************************************************************/
/*                                                                      */
/************************************************************************/
bool CBeopDataEngineCore::ReBuildDatabase(wstring wstrDBName)
{
	bool bresult = false;
	CDatabaseSeesion	m_dbSession;
	bresult =	m_dbSession.ConnectToDB(UtilString::ConvertMultiByteToWideChar(m_dbset.strDBIP), 
	UtilString::ConvertMultiByteToWideChar(m_dbset.strUserName), 
	UtilString::ConvertMultiByteToWideChar(m_dbset.strDBPsw),
	TEXT(""),
	m_dbset.nPort);

	if(bresult)
	{
		bool bR = false;
		char* cFind = "beopdata";
		char cReplace[56];
		memset(cReplace,0,56);
		string strDB_Ansi = Project::Tools::WideCharToAnsi(wstrDBName.c_str());
		memcpy(cReplace,strDB_Ansi.data(),strDB_Ansi.length());
		
		//删除并创建数据库
		//m_dbSession.Execute(E_DROPDB_BEOP);
		m_dbSession.Execute(RelaceStr(E_CREATEDB_BEOP,cFind,cReplace));


		//删除表
		m_dbSession.StartTransaction();
		m_dbSession.Execute(RelaceStr(E_DROPTB_BEOPINFO,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_DROPTB_LOG,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_DROPTB_OPERATION_RECORD,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_DROPTB_POINT_VALUE_BUFFER,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_DROPTB_REALTIMEDATA_INPUT,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_DROPTB_REALTIMEDATA_OUTPUT,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_DROPTB_UNIT01,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_DROPTB_UNIT02,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_DROPTB_UNIT03,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_DROPTB_UNIT04,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_DROPTB_UNIT05,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_DROPTB_UNIT06,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_DROPTB_UNIT07,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_DROPTB_UNIT08,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_DROPTB_UNIT09,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_DROPTB_UNIT10,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_DROPTB_USERLIST_ONLINE,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_DROPTB_WARNING_CONFIG,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_DROPTB_WARNING_RECORD,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_DROPTB_FILESTORAGE,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_DROPTB_FILESTORAGE,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_DROPTB_BEOPUSER,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_DROPTB_BEOPROLE,cFind,cReplace));

		m_dbSession.Commit();

		//创建表
		m_dbSession.StartTransaction();
		m_dbSession.Execute(RelaceStr(E_CREATETB_BEOPINFO,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_CREATETB_LOG,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_CREATETB_OPERATION_RECORD,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_CREATETB_POINT_VALUE_BUFFER,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_CREATETB_REALTIMEDATA_INPUT,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_CREATETB_REALTIMEDATA_OUTPUT,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_CREATETB_UNIT01,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_CREATETB_UNIT02,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_CREATETB_UNIT04,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_CREATETB_UNIT05,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_CREATETB_UNIT06,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_CREATETB_UNIT07,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_CREATETB_UNIT08,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_CREATETB_UNIT09,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_CREATETB_UNIT10,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_CREATETB_USERLIST_ONLINE,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_CREATETB_WARNING_CONFIG,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_CREATETB_WARNING_RECORD,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_CREATETB_FILESTORAGE,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_CREATETB_FILESTORAGE,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_CREATETB_BEOPUSER,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_CREATETB_BEOPROLE,cFind,cReplace));
		m_dbSession.Commit();

		//插入表
		m_dbSession.StartTransaction();
		m_dbSession.Execute(RelaceStr(E_INSERTTB_BEOPINFO,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_INSERTTB_USERLIST_ONLINE,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_INSERTTB_WARNING_CONFIG,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_INITUSER_BEOPUSER,cFind,cReplace));
		m_dbSession.Commit();
		m_dbSession.DisConnectFromDB();
	}
	
	return true;
}

bool CBeopDataEngineCore::DatabaseUpdateFrom22To23(wstring wstrDBName)
{
	bool bresult = false;
	CDatabaseSeesion	m_dbSession;
	bresult =	m_dbSession.ConnectToDB(UtilString::ConvertMultiByteToWideChar(m_dbset.strDBIP), 
		UtilString::ConvertMultiByteToWideChar(m_dbset.strUserName), 
		UtilString::ConvertMultiByteToWideChar(m_dbset.strDBPsw), 
		TEXT(""),
		m_dbset.nPort);

	if(bresult)
	{
		bool bR = true;

		char* cFind = "beopdata";
		char cReplace[56];
		memset(cReplace,0,56);
		string strDB_Ansi = Project::Tools::WideCharToAnsi(wstrDBName.c_str());
		memcpy(cReplace,strDB_Ansi.data(),strDB_Ansi.length());
		//删除表
		m_dbSession.StartTransaction();
		m_dbSession.Execute(RelaceStr(E_DROPTB_FILESTORAGE,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_DROPTB_BEOPUSER,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_DROPTB_BEOPROLE,cFind,cReplace));
		bR = m_dbSession.Commit() && bR;

		//创建表
		m_dbSession.StartTransaction();
		m_dbSession.Execute(RelaceStr(E_CREATETB_FILESTORAGE,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_CREATETB_BEOPUSER,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_CREATETB_BEOPROLE,cFind,cReplace));
		m_dbSession.Execute(RelaceStr(E_INITUSER_BEOPUSER,cFind,cReplace));

		bR = m_dbSession.Commit() && bR;

		if(bR)
		{
			std::ostringstream sqlstream;
			sqlstream << "update " << Project::Tools::WideCharToAnsi(wstrDBName.c_str()) << ".`beopinfo` set incocontent = '2.3' where infoname = 'version';";
			string strVersionUpdate = sqlstream.str();
			m_dbSession.Execute(strVersionUpdate);
		}
		//插入表, 暂无

		m_dbSession.DisConnectFromDB();
	}

	return true;
}

UINT WINAPI CBeopDataEngineCore::ThreadFunc_ConnectRedundancyDB( LPVOID lparam )
{
	CBeopDataEngineCore * pEngineCore = (CBeopDataEngineCore*)lparam;

	while(true)
	{
		Sleep(5000);

		if(pEngineCore->m_bExitThread)
			break;

		if(!pEngineCore->m_dbcon_Redundancy)
			continue;
		
		if(pEngineCore->ReconnectRedundancyDB())
			break;

	}

	pEngineCore->m_bThreadExitFinished = true;
	return 0;
}

bool CBeopDataEngineCore::ReconnectRedundancyDB()
{
	//网络通
	wstring username = UtilString::ConvertMultiByteToWideChar(m_dbset.strUserName);
	wstring strPwd =  UtilString::ConvertMultiByteToWideChar(m_dbset.strDBPsw);
	wstring strDBName       = UtilString::ConvertMultiByteToWideChar(m_dbset.strRealTimeDBName);
	wstring rehostname = UtilString::ConvertMultiByteToWideChar(m_dbset.strDBIP2);
	bool bresult5 = m_dbcon_Redundancy->ConnectToDB(rehostname,
		username,
		strPwd, 
		strDBName,
		m_dbset.nPort);

	return bresult5;
}

bool CBeopDataEngineCore::TerminateReconnectThread()
{
	m_bExitThread = true;
	//WaitForSingleObject(m_hThread_WarningScan, INFINITE);
	TerminateThread(m_hThread_ReconRedundancyDB,0);
	if(m_hThread_ReconRedundancyDB != NULL)
	{
		CloseHandle(m_hThread_ReconRedundancyDB);
		m_hThread_ReconRedundancyDB = NULL;
	}
	return true;
}

string CBeopDataEngineCore::RelaceStr( char* strSrc, char* strFind, char* strReplace )
{
	size_t pos = 0;
	wstring tempStr = AnsiToWideChar(strSrc);
	wstring oldStr = AnsiToWideChar(strFind);
	wstring newStr = AnsiToWideChar(strReplace);

	wstring::size_type newStrLen = newStr.length();
	wstring::size_type oldStrLen = oldStr.length();
	while(true)
	{
		pos = tempStr.find(oldStr, pos);
		if (pos == wstring::npos) break;

		tempStr.replace(pos, oldStrLen, newStr);        
		pos += newStrLen;
	}

	string strtempStr_Ansi;
	WideCharToUtf8(tempStr,strtempStr_Ansi);
	return strtempStr_Ansi; 
}

bool CBeopDataEngineCore::InsertRedundancyOperation( wstring wstrRedundancyIp )
{
	if(wstrRedundancyIp.length() <= 0)
		return false;

	SYSTEMTIME sNow;
	GetLocalTime(&sNow);
	wchar_t wcTemp[1024];
	wsprintf(wcTemp, L"Slaver Server (%s) Start Working.", wstrRedundancyIp.c_str());
	wstring strUser = L"Core";

	if(m_dbcon_common != NULL)
	{
		if(m_dbcon_common->IsConnected())
		{
			m_dbcon_common->InsertOperationRecord(sNow,wcTemp,strUser);
		}
	}

	if(m_dbcon_Redundancy != NULL)
	{
		if(m_dbcon_Redundancy->IsConnected())
		{
			return m_dbcon_Redundancy->InsertOperationRecord(sNow,wcTemp,strUser);
		}
	}
	return false;
}

bool CBeopDataEngineCore::SetDefaultDBIP( string strIP )
{
	m_dbset.strDBIP = strIP;
	return true;
}

bool CBeopDataEngineCore::SetDefaultDBUser( string strUser )
{
	m_dbset.strUserName = strUser;
	return true;
}

bool CBeopDataEngineCore::InitLogic()
{
	m_dbcon			= new CRealTimeDataAccess();
	m_dbcon_common	= new Beopdatalink::CCommonDBAccess();
	m_dbcon_Redundancy = new Beopdatalink::CCommonDBAccess();
	m_dbcon_log		= new Beopdatalink::CLogDBAccess();

	return true;
}


bool CBeopDataEngineCore::AddVirtualPointToDBFile(const wstring & wstrPointName, const wstring & wstrPointDescription)
{
	if(m_pointmanager ==NULL)
	{

		m_pointmanager = new CDataPointManager();
		m_pointmanager->SetFilePath(m_strDBFileName);
		m_pointmanager->Init();
		m_pointmanager->ParsePointlist();
	}

	if(m_pointmanager->GetPointExist(wstrPointName))
	{
		wchar_t wcTemp[1024];
		wsprintf(wcTemp, L"Recv Add Point Command (%s) failed for this point exist.", wstrPointName.c_str());
		return false;
	}


	wchar_t wcTemp[1024];
	wsprintf(wcTemp, L"Recv Add Point Command (%s) act.", wstrPointName.c_str());
	m_pointmanager->InsertVirtualPointToSqlite(wstrPointName, wstrPointDescription);
	m_pointmanager->Init();
	return true;
}

wstring CBeopDataEngineCore::GetLocalConfig(wstring wstrConfigName)
{
	string strUtf8;
	Project::Tools::WideCharToUtf8(m_strDBFileName, strUtf8);
	CSqliteAcess access(strUtf8);

	wstring wstr = _T("");

	string strConfigName = UtilString::ConvertWideCharToMultiByte(wstrConfigName);

	ostringstream sqlstream;
	sqlstream << "select content from local_config where name = '"<< strConfigName <<"'"<<";";
	string sql_ = sqlstream.str();
	char *ex_sql = const_cast<char*>(sql_.c_str());
	int re = access.SqlQuery(ex_sql);

	while(true)
	{
		if(SQLITE_ROW != access.SqlNext())
		{
			break;
		}

		if (access.getColumn_Text(0) != NULL)
		{
			string strContetn(access.getColumn_Text(0));
			wstr = UtilString::ConvertMultiByteToWideChar(strContetn);

		}

	}

	access.SqlFinalize();
	access.CloseDataBase();

	return wstr;
}