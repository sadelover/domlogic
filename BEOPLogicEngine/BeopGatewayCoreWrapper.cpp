#include "StdAfx.h"
#include "BeopGatewayCoreWrapper.h"
#include "DumpFile.h"
#include "../BEOPDataEngineCore/LogicPipeline.h"

#include "../Http/HttpOperation.h"
#include "../json/json.h"

/************************************************************************/
/* 
0909:增加了策略日志功能
1016:兼容了被添加了损坏的logic的db文件
V2.2.5
   增加在core_status表中记录运行时间的功能

V2.2.6
   统一了策略里的每个dll的运行时间间隔
   core_status里名称改为了domlogic以便于domhost统一监管
V2.2.7
   对数据库名、密码、4db切换的支持
V2.2.13:
   日程改进：如果每次用户修改日程时，改的对象值相同，则不发该指令。

V2.2.14:
   改进策略更新，支持仅仅策略成重启，而不是把文件复制替代

V2.2.15:
   增加了系统dll的全部内部参数开放到mysql的sys_logic表中供pysite后台读取

   V2.2.16:
   logic bad日志升级

   V2.2.17
   支持隔日

V2.3.16
   将模式日程的操作记录去掉，否则太多了。




*/
/************************************************************************/
//
//#define CORE_VERSION _T("V2.2.3_20180919")
//#define CORE_VERSION _T("V2.2.4_20181016")
//#define CORE_VERSION _T("V2.2.5_20181022")
//#define CORE_VERSION _T("V2.2.6_20181104")
//#define CORE_VERSION _T("V2.2.7_20181115")
//#define CORE_VERSION _T("V2.2.8_20181127")
//#define CORE_VERSION _T("V2.2.9_20181216")
//#define CORE_VERSION _T("V2.2.10_20181220")
//#define CORE_VERSION _T("V2.2.11_20181222")
//#define CORE_VERSION _T("V2.2.12_20190417")
//#define CORE_VERSION _T("V2.2.13")
//#define CORE_VERSION _T("V2.2.14")
//#define CORE_VERSION _T("V2.2.15")
//#define CORE_VERSION _T("V2.2.16")
//#define CORE_VERSION _T("V2.2.17")
//
////升级了AddWarning的实现
//#define CORE_VERSION _T("V2.2.17")
//
//
////转移log数据库到domlog
//#define CORE_VERSION _T("V2.3.1")
//
////增加日程对场景的执行支持
//#define CORE_VERSION _T("V2.3.2")
//
//#define CORE_VERSION _T("V2.3.3")
//
////支持日历和模式
//#define CORE_VERSION _T("V2.3.4")
//
////修复日历中的模式bug
//#define CORE_VERSION _T("V2.3.5")
//
//
////修复日历中的模式bug
//#define CORE_VERSION _T("V2.3.6")
//
////增加日历模式执行时的多点缓冲输出，不要一次性全部输出，那样负荷冲击大
//#define CORE_VERSION _T("V2.3.7")
//
////修复点名日程时间的bug
//#define CORE_VERSION _T("V2.3.8")
//
//
//#define CORE_VERSION _T("V2.3.9")
//#define CORE_VERSION _T("V2.3.10")
//#define CORE_VERSION _T("V2.3.11")
//
////增加策略LOG开头结尾空行，修复输出const时点名不被写，修复策略停止后重启没有Init
//#define CORE_VERSION _T("V2.3.12")
//
//#define CORE_VERSION _T("V2.3.13")
//
////增加模式中的浮点数值的发送的相等过滤，不重复发值
//#define CORE_VERSION _T("V2.3.14")
//
//
//#define CORE_VERSION _T("V2.3.15")
//#define CORE_VERSION _T("V2.3.16")
//#define CORE_VERSION _T("V2.3.17")
//
////log机制彻底改变，提升性能
//#define CORE_VERSION _T("V2.4.1")
//
////const output配置等同于point
//
//#define CORE_VERSION _T("V2.4.2")
//
////add version
//#define CORE_VERSION _T("V2.4.3")
//
////解决log文件读写冲突问题
//#define CORE_VERSION _T("V2.4.4")
//
////解决上一个版本不能实时更新策略文件的问题，总要重启domlogic才能生效
//#define CORE_VERSION _T("V2.4.5")
//
////解决多个domlogic进程可以同时运行的问题
//#define CORE_VERSION _T("V2.4.6")
//
////场景模式的指令输出记录保存至文本log目录，减轻数据库压力
//#define CORE_VERSION _T("V2.4.7")
//
//
////4db没有策略也能保持启动后运行
//#define CORE_VERSION _T("V2.4.8")
//
//
////去除LogicStatusBegin 和End对数据库的压力，只保留了heatbeattime
//#define CORE_VERSION _T("V2.4.9")
//
//
////解决domlogic内存逐渐增大的bug：主循环中ReadOrCreateCoreDebugItemValueMul数据库语句未回收
//#define CORE_VERSION _T("V2.4.10")
//
//
////增加redis的支持
//#define CORE_VERSION _T("V2.4.11")
//
//
////修复磁盘策略log文件损坏后的访问死机问题
//#define CORE_VERSION _T("V2.4.12")
////修复磁盘策略指令记录log文件损坏后的访问死机问题
//#define CORE_VERSION _T("V2.4.13")
//
//
////增加log显示计数
//#define CORE_VERSION _T("V2.4.14")
//
//
////增加log记录单线程模式下的策略执行，方便策略排错
//#define CORE_VERSION _T("V2.4.15")
//
//
////增加logic dll执行意外
//#define CORE_VERSION _T("V2.4.16")
//
//
////支持mode里场景到点执行一次
//#define CORE_VERSION _T("V2.4.17")
////
//#define CORE_VERSION _T("V2.4.18")
//
//
////增加启动时调用dompysite的updateRedis
//#define CORE_VERSION _T("V2.4.19")
//#define CORE_VERSION _T("V2.4.20")
//
////解决策略线程名称大小写和mysql大小写有出入导致停止启动策略线程无效的bug
//#define CORE_VERSION _T("V2.4.21")
//
//
//#define CORE_VERSION _T("V2.4.22")
//
////解决内存泄漏重大bug
//#define CORE_VERSION _T("V2.5.1")
//
////支持PointDepend接口
//#define CORE_VERSION _T("V2.5.2")
//
//
////支持dom_system_mode_of_sys0-9
//#define CORE_VERSION _T("V2.5.3")
//
//
////重大修复，之前版本读入参数可能有误!增加初始化时的心跳包更新
//#define CORE_VERSION _T("V2.5.4")
//
//
////重大修复，初始化打印dllname容易崩溃
//#define CORE_VERSION _T("V2.5.5")
//
//
////domlogic**副进程不再检测日历模式执行
//#define CORE_VERSION _T("V2.5.6")
//
////domlogic update机制改进
//#define CORE_VERSION _T("V2.5.7")
//
//
////domlogic 线程内按照名称顺序执行
//#define CORE_VERSION _T("V2.5.8")
//
//
////分支进程也能更新策略参数
//#define CORE_VERSION _T("V2.5.9")

//分支进程也能更新策略参数改进
//#define CORE_VERSION _T("V2.5.10")


//
#define CORE_VERSION _T("2.7.4")

extern bool g_bSingleThread;
extern Project::Tools::Mutex	g_thread_lock;

CBeopGatewayCoreWrapper::CBeopGatewayCoreWrapper()
{

	m_pDataEngineCore = NULL;
	m_pDataAccess_Arbiter = NULL;
	m_pLogicManager = NULL;
	m_bFirstInit = true;
	m_bExitCore = false;
	GetLocalTime(&m_stNow);

}


CBeopGatewayCoreWrapper::CBeopGatewayCoreWrapper(vector<wstring> wstrPrefixList)
{

	m_pDataEngineCore = NULL;
	 m_pDataAccess_Arbiter = NULL;
	m_pLogicManager = NULL;
	m_bFirstInit = true;
	m_bExitCore = false;
	GetLocalTime(&m_stNow);
	m_wstrPrefixList = wstrPrefixList;
}

CBeopGatewayCoreWrapper::~CBeopGatewayCoreWrapper(void)
{
}


void CBeopGatewayCoreWrapper::InitBackupAnalysisAllLogicStatus()
{
	if(m_pLogicManager==NULL)
		return;

	int i = 0;
	wchar_t wsChar[1024];


	COleDateTime tNow = COleDateTime::GetCurrentTime();
	wstring wstrErrTime ;
	Project::Tools::OleTime_To_String(tNow, wstrErrTime);

	CString strLabel;
	for(int i=0;i<m_wstrPrefixList.size();i++)
	{
		strLabel+= _T("_");
		strLabel+= m_wstrPrefixList[i].c_str();
	}

	CString strErrAllLast;
	strErrAllLast.Format(_T("==================ERROR LAST RUN REPORT START(%s)==================\r\n"), wstrErrTime.c_str());

	for (i=0;i<m_pLogicManager->m_vecDllThreadList.size();++i)
	{	
		CLogicDllThread *pDllThread = m_pLogicManager->m_vecDllThreadList[i];
		wstring strDllRunValue = m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue(pDllThread->GetName());
		if(strDllRunValue==L"1")
		{
			for(int j=0;j<pDllThread->GetDllCount();j++)
			{
				CDLLObject *pobj = pDllThread->GetDllObject(j);
				if(!pobj)
					continue;

				CString strDllName = pobj->GetDllName().c_str();

				CString strErrOne;
				bool bErr = m_pDataAccess_Arbiter->GetLogicBeginEndStatusErr(strDllName, strErrOne);
				if(bErr)
				{
					CString strTemp;
					strTemp.Format(_T("ERROR in Logic: %s, Info:%s\r\n"), strDllName, strErrOne);
					strErrAllLast+=strTemp;
				}

				
			}
		}

	}
	strErrAllLast+= _T("==================ERROR LAST RUN REPORT  END==================\r\n");

	PrintLog(strErrAllLast.GetString(), false);


	wstring wstrErrTillNow;
	m_pDataAccess_Arbiter->ReadCoreDebugItemValue(_T("system_bad_logic_info"), strLabel.GetString(), wstrErrTillNow);

	

	strErrAllLast = wstrErrTillNow.c_str()+strErrAllLast;

	if(strErrAllLast.GetLength()>20000)
	{
		strErrAllLast = strErrAllLast.Mid(strErrAllLast.GetLength()-15000);
	}

	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(_T("system_bad_logic_info"),strLabel.GetString(), strErrAllLast.GetString());
}

bool CBeopGatewayCoreWrapper::Run()
{
	//初始化Dump文件
	DeclareDumpFile();


	wstring wstrAppName;
	Project::Tools::GetAppName(wstrAppName);
	wstrAppName = wstrAppName.substr(0, wstrAppName.find(L".exe"));
	string strAppName = Project::Tools::WideCharToUtf8(wstrAppName.c_str());

	//初始化dog
	//CDogTcpCtrl::GetInstance()->SetDogMode(E_SLAVE_LOGIC);			//logic模式

	//FeedDog();


	HANDLE m_hmutex_instancelock = CreateMutex(NULL, FALSE, wstrAppName.c_str());
	if (ERROR_ALREADY_EXISTS == GetLastError())
	{
		PrintLog(L"Another domlogic is running, Start failed.\r\n", false);
		Sleep(10000);
		exit(0);
		return false;
	}

	wchar_t wcChar[1024];
	COleDateTime tnow = COleDateTime::GetCurrentTime();

	wsprintf(wcChar, L"---- %s %s starting ----\r\n", wstrAppName.c_str(), CORE_VERSION);
	PrintLog(wcChar, false);

	PrintLog(L"Check Database Status...\r\n", false);
	m_strDBFilePath = PreInitFileDirectory_4db();
	if(m_strDBFilePath.length()==0)
	{
		m_strDBFilePath = PreInitFileDirectory();
	}



	if(!Init(m_strDBFilePath))
	{
		//FeedDog();
		Release();
		PrintLog(L"ERROR: Init Project domdb.S3DB or domdb.4DB File Failed, Will quit after 30 seconds automatic...\r\n", false);
		Sleep(30000);
		return false;
	}

	OutPutLogString(_T("Start to update syslogic infomations"));
	InitSyslogicParameters();

	OutPutLogString(_T("domlogic.exe start"));

	m_bFirstInit = false;

	wstring strTimeServer;
	GetLocalTime(&m_stNow);

	//读取线程模式
	wstring strThreadMode = m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue(L"logic_thread_mode");
	if(strThreadMode==L"1")
	{
		g_bSingleThread = true;
	}

	////////////////test
	//for(int i=0;i<999;i++)
	//{	m_pDataAccess_Arbiter->InsertLog(_T("[aaaaa.dll]fdsafdsafaaaaaaaaaaaaaaaaaaa"));
	//m_pDataAccess_Arbiter->InsertLog(_T("[bbbbbb.dll]fdsafdsaf"));
	//m_pDataAccess_Arbiter->InsertLog(_T("[ccccc.dll]fdsafdsaf"));
	//}


	wstring wstrLogicBatchCount = L"";
	int nLogicBatchCount = 100;
	wstrLogicBatchCount = m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue(L"logic_batch_log_count" );
	if(wstrLogicBatchCount.length()>0)
	{
		nLogicBatchCount = _wtoi(wstrLogicBatchCount.c_str());
	}
	m_pDataAccess_Arbiter->SetLogicBatchCount(nLogicBatchCount);

	//更新服务器启动时间

	Project::Tools::SysTime_To_String(m_stNow, strTimeServer);
	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"logicstarttime", strTimeServer);

	//统计重启次数
	wstring wstrRestartCount = L"";
	int nRestartCount = 0;
	m_pDataAccess_Arbiter->ReadCoreDebugItemValue(L"logicrestartcount", wstrRestartCount);
	if(wstrRestartCount.length()>0)
	{
		nRestartCount = _wtoi(wstrRestartCount.c_str());
	}
	nRestartCount+=1;
	if(nRestartCount>=1e9)
		nRestartCount = 0;
	CString strRestartCount;
	strRestartCount.Format(_T("%d"), nRestartCount);
	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"logicrestartcount", strRestartCount.GetString());

	//setup fdd database table
	

	//read log level
	wstring wstrLogicLogLevel = L"0";
	m_pDataAccess_Arbiter->ReadCoreDebugItemValue(L"logic_log_level", wstrLogicLogLevel);
	if(wstrLogicLogLevel==L"-1")
	{
		PrintLog(L"Logic Log Disabled!!!\r\n");
		m_pDataAccess_Arbiter->m_nLogicLogLevel = -1;
	}else
	{
		m_pDataAccess_Arbiter->m_nLogicLogLevel = 0;
	}


	CString strLabel;
	for(int i=0;i<m_wstrPrefixList.size();i++)
	{
		strLabel+= _T("_");
		strLabel+= m_wstrPrefixList[i].c_str();
	}

	COleDateTime tLastUpdateStatus = COleDateTime::GetCurrentTime()-COleDateTimeSpan(0,1,0,0);
	wstring strAlwaysLog = _T("");
	while(!m_bExitCore)
	{
		Sleep(10000); //20200519 之前是2秒一个循环，这样压力很大
		
		//GetLocalTime(&m_stNow);

		//更新服务器时间
		
		//Project::Tools::SysTime_To_String(m_stNow, strTimeServer);
		
		//CString strLogicTimeString = L"logicservertime" + strLabel;
		//m_pDataAccess_Arbiter->WriteCoreDebugItemValue(strLogicTimeString.GetString(), strTimeServer);

		//喂狗
		//FeedDog();

		//更新心跳时间
		COleDateTime tNow = COleDateTime::GetCurrentTime();
		if((tNow-tLastUpdateStatus).GetTotalSeconds()>=60)
		{
			strAlwaysLog = m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue(L"alwayslog");
			m_pDataAccess_Arbiter->UpdateHistoryStatusTable(strAppName);

			//update logic_log_level
			if(tNow.GetMinute()%5==0)//整5分才刷新，减轻系统负载
			{
				m_pDataAccess_Arbiter->ReadCoreDebugItemValue(L"logic_log_level", wstrLogicLogLevel);
				if(wstrLogicLogLevel==L"-1")
				{
					PrintLog(L"Logic Log Disabled!!!\r\n");
					m_pDataAccess_Arbiter->m_nLogicLogLevel = -1;
				}else
				{
					m_pDataAccess_Arbiter->m_nLogicLogLevel = 0;
				}
			}

			tLastUpdateStatus = tNow;
		}
		
		
		//update logic
		if(m_wstrPrefixList.size()==0)//不带指定分字母线程生产运行，只有main进程才有权力
		{
			

			//update restart
			UpdateResetCommand();

		}

		UpdateCheckLogic();

		//save log
		if(strAlwaysLog==L"1")
		{
			SaveAndClearLog();
		}
	}
	return true;
}

bool CBeopGatewayCoreWrapper::UpdateResetCommand()
{
	if(m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue(L"resetlogic")==L"1")
	{
		m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"resetlogic", L"0");
		PrintLog(L"Reset Signal Recved, Restarting...\r\n", false);
		Reset();
		PrintLog(L"logic engine restarted, initializing...\r\n\r\n");
		OutPutLogString(_T("BEOPLogicEngine.exe manual restart"));
		Init(m_strDBFilePath);
	}

	return true;
}

bool CBeopGatewayCoreWrapper::Reset()
{

	PrintLog(L"Start Unloading Logic Threads...");
	
	if(m_pLogicManager)
	{
		int nDllCount = m_pLogicManager->GetAllImportDll();
		wstring strPrefix = L"Unloading Threads:";
		for(int i=0; i<m_pLogicManager->m_vecDllThreadList.size(); i++)
		{
			CLogicDllThread *pLogicThread = m_pLogicManager->m_vecDllThreadList[i];
			wstring strDllName = pLogicThread->GetName();
			wstring strOut = strPrefix + strDllName ;
			bool bSuccess = pLogicThread->Exit();
			if(bSuccess)
				strOut+= L" Success.\r\n";
			else
				strOut+= L" Failed ERROR!!!\r\n";
			PrintLog(strOut);
		}

		PrintLog(L"Unloading logic Manager: ");
		if(m_pLogicManager->Exit())
			PrintLog(L"Success. \r\n");
		else
			PrintLog(L"Failed ERROR!!! \r\n");

		delete(m_pLogicManager);
		m_pLogicManager = NULL;
	}
	else
		PrintLog(L"ERROR.\r\n");

	PrintLog(L"Exit DataEngineCore...");
	if(m_pDataEngineCore)
	{
		m_pDataEngineCore->SetDBAcessState(false);
		m_pDataEngineCore->TerminateScanWarningThread();
		m_pDataEngineCore->TerminateReconnectThread();

		m_pDataEngineCore->Exit();
		delete(m_pDataEngineCore);
		m_pDataEngineCore = NULL;
		PrintLog(L"Successfully.\r\n");
	}
	else
		PrintLog(L"ERROR.\r\n");
	
	PrintLog(L"Disconnect Database for Logic...");
	if(m_pDataAccess_Arbiter)
	{
		m_pDataAccess_Arbiter->TerminateAllThreads();
		delete(m_pDataAccess_Arbiter);
		m_pDataAccess_Arbiter = NULL;
		PrintLog(L"Successfully\r\n");
	}
	else
		PrintLog(L"ERROR.\r\n");

	return true;

}

bool CBeopGatewayCoreWrapper::InitSyslogicParameter(CString strDllPath)
{
	CDLLObject *pDLL = new CDLLObject(m_pDataAccess_Arbiter);		//创建策略

	//load new
	if(!pDLL->Load(strDllPath.GetString()))
	{
		delete(pDLL);
		pDLL = NULL;
		return false;
	}

	CString strLogicName;
	int nFF = strDllPath.ReverseFind('\\');
	CString strNN = strDllPath.Mid(nFF+1);
	nFF = strNN.ReverseFind('.');
	CString strSysLogicName = strNN.Left(nFF);

	wstring wstrAuthor = pDLL->GetDllAuthor();
	wstring wstrDescription = pDLL->GetDllDescription();
	wstring wstrVersion  = pDLL->GetDllVersion();
	wstring wstrName = pDLL->GetOriginalDllName();
	wstring wstrInputParameters = pDLL->GetLB()->GetAllInputParameter().GetString();
	wstring wstrOutputParameters = pDLL->GetLB()->GetAllOutputParameter().GetString();

	wstring wstrSysLogicName = strSysLogicName.GetString();
	m_pDataAccess_Arbiter->SaveSysDllInfo(wstrSysLogicName, wstrDescription, wstrAuthor, wstrVersion, wstrInputParameters, wstrOutputParameters);

	pDLL->UnLoad();
	delete(pDLL);

	return true;
}

bool CBeopGatewayCoreWrapper::InitSyslogicParameters()
{
	wstring cstrFile;
	Project::Tools::GetSysPath(cstrFile);

	wstring strDBFileName;


	CFileFind s3dbfilefinder;
	CString strSysLogicPath;
	strSysLogicPath.Format(_T("%s\\dompysite\\syslogic\\*.dll"), cstrFile.c_str());
	wstring filename;
	BOOL bfind = s3dbfilefinder.FindFile(strSysLogicPath.GetString());
	wstring SourcePath, DisPath;
	vector<wstring> strAvailableFileNameList;
	while (bfind)
	{
		bfind = s3dbfilefinder.FindNextFile();
		filename = s3dbfilefinder.GetFileName();

		int nFF = filename.find(_T(".dll"));
		if(nFF>0)
			strAvailableFileNameList.push_back(filename);
	}

	for(int i=0;i<strAvailableFileNameList.size();i++)
	{
		CString strDllFilePath;
		strDllFilePath.Format(_T("%s\\dompysite\\syslogic\\%s"), cstrFile.c_str(), strAvailableFileNameList[i].c_str());

		InitSyslogicParameter(strDllFilePath);
	}

	return true;
}

wstring CBeopGatewayCoreWrapper::PreInitFileDirectory()
{
	wstring cstrFile;
	Project::Tools::GetSysPath(cstrFile);

	wstring strDBFileName;


		CFileFind s3dbfilefinder;
		wstring strSearchPath = cstrFile + L"\\domdb.s3db";
		wstring filename;
		BOOL bfind = s3dbfilefinder.FindFile(strSearchPath.c_str());
		wstring SourcePath, DisPath;
		vector<wstring> strAvailableFileNameList;
		while (bfind)
		{
			bfind = s3dbfilefinder.FindNextFile();
			filename = s3dbfilefinder.GetFileName();

			strAvailableFileNameList.push_back(filename);
		}


		if(strAvailableFileNameList.size()>0)
		{
			strDBFileName = cstrFile + L"\\" + strAvailableFileNameList[0];
			for(int nFileIndex=1; nFileIndex<strAvailableFileNameList.size();nFileIndex++)
			{
				wstring strFileNameToDelete = cstrFile + L"\\" + strAvailableFileNameList[nFileIndex];
				if(!UtilFile::DeleteFile(strFileNameToDelete.c_str()))
				{
					_tprintf(L"ERROR: Delete UnNeccesary DB Files Failed!");
				}
			}
		}

	

	//log目录
	wstring strLogDir = cstrFile + L"\\Temp";
	if(!UtilFile::DirectoryExists(strLogDir))
	{
		UtilFile::CreateDirectory(strLogDir);
	}
	wstring strDBFileDir = cstrFile + L"\\DBFileVersion";
	if(!UtilFile::DirectoryExists(strDBFileDir))
	{
		UtilFile::CreateDirectory(strDBFileDir);
	}

	return strDBFileName;
}


wstring CBeopGatewayCoreWrapper::PreInitFileDirectory_4db()
{
	wstring cstrFile;
	Project::Tools::GetSysPath(cstrFile);

	wstring strDBFileName;


	CFileFind s3dbfilefinder;
	wstring strSearchPath = cstrFile + L"\\domdb.4db";
	wstring filename;
	BOOL bfind = s3dbfilefinder.FindFile(strSearchPath.c_str());
	wstring SourcePath, DisPath;
	vector<wstring> strAvailableFileNameList;
	while (bfind)
	{
		bfind = s3dbfilefinder.FindNextFile();
		filename = s3dbfilefinder.GetFileName();

		strAvailableFileNameList.push_back(filename);
	}


	if(strAvailableFileNameList.size()>0)
	{
		strDBFileName = cstrFile + L"\\" + strAvailableFileNameList[0];
		for(int nFileIndex=1; nFileIndex<strAvailableFileNameList.size();nFileIndex++)
		{
			wstring strFileNameToDelete = cstrFile + L"\\" + strAvailableFileNameList[nFileIndex];
			if(!UtilFile::DeleteFile(strFileNameToDelete.c_str()))
			{
				_tprintf(L"ERROR: Delete UnNeccesary DB Files Failed!");
			}

			CString strInfo;
			strInfo.Format(_T("INFO: Delete No-use DB File: %s\r\n"), strFileNameToDelete.c_str());
			_tprintf(strInfo.GetString());
		}

		wstring strNewFileName = cstrFile+ L"\\domdb.4db";
		if(strAvailableFileNameList[0]!=L"domdb.4db")
		{
			bool bRename = UtilFile::RenameFile(strDBFileName, strNewFileName);
			if(!bRename)
			{
				CString strInfo;
				strInfo.Format(_T("ERROR: Rename DB File Failed.\r\n"));
				_tprintf(strInfo.GetString());
			}
			else
			{
				strDBFileName = strNewFileName;
			}
		}
		
		
	}



	//log目录
	wstring strLogDir = cstrFile + L"\\Temp";
	if(!UtilFile::DirectoryExists(strLogDir))
	{
		UtilFile::CreateDirectory(strLogDir);
	}
	wstring strDBFileDir = cstrFile + L"\\DBFileVersion";
	if(!UtilFile::DirectoryExists(strDBFileDir))
	{
		UtilFile::CreateDirectory(strDBFileDir);
	}

	return strDBFileName;
}

bool CBeopGatewayCoreWrapper::LoadDBSettingFromCoreConfig(gDataBaseParam &dbset)
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
	WritePrivateProfileString(L"core", L"defaultdb", strDefaultDB.c_str(), exepath.c_str());

	dbset.strRealTimeDBName = Project::Tools::WideCharToAnsi(strDefaultDB.c_str());


	wchar_t charRootPassword[1024];
	GetPrivateProfileString(L"core", L"dbpassword", L"", charRootPassword, 1024, exepath.c_str());
	wstring wstrDBPassword = charRootPassword;
	if(wstrDBPassword == L"")
		wstrDBPassword = L"RNB.beop-2013";

	dbset.strDBPsw = Project::Tools::WideCharToAnsi(wstrDBPassword.c_str());


	wchar_t charServer[1024];
	GetPrivateProfileString(L"core", L"server", L"", charServer, 1024, exepath.c_str());
	wstring wstrServer = charServer;
	if(wstrServer == L"")
		wstrServer = L"127.0.0.1";

	dbset.strDBIP = Project::Tools::WideCharToAnsi(wstrServer.c_str());

	return true;
}


bool CBeopGatewayCoreWrapper::Init(wstring strDBFilePath)
{
	if(strDBFilePath.length()<=0)
		return false;

	wchar_t wcChar[1024];
	if(m_pDataEngineCore == NULL)
		m_pDataEngineCore = new CBeopDataEngineCore(strDBFilePath);

	if(m_pDataEngineCore->m_strDBFileName.length()<=0)
	{
		PrintLog(L"ERROR: None DB File Found.\r\n");
		delete(m_pDataEngineCore);
		m_pDataEngineCore = NULL;
		return false;
	}

	wsprintf(wcChar, L"Reading DBFile:%s\r\n", m_pDataEngineCore->m_strDBFileName.c_str());
	PrintLog(wcChar);

	//读取服务器数据库
	wstring cstrFile;
	Project::Tools::GetSysPath(cstrFile);
	wstring exepath;
	exepath = cstrFile + L"\\domlogic.ini";
	CString strLogFileName;
	COleDateTime tnow = COleDateTime::GetCurrentTime();
	strLogFileName.Format(_T("\\log\\domlogic_err_%s.log"), tnow.Format(L"%Y_%m_%d"));
	m_strLogFilePath  = cstrFile + strLogFileName.GetString();

	wstring wstrAppName;
	Project::Tools::GetAppName(wstrAppName);

	if(wstrAppName==L"domlogic.exe")// is main process
	{
		vector<CString> strNeedRunExeCommandList;
		vector<CString> strNeedRunParamList;
		for(int i=0;i<10;i++)
		{
			CString strThreadName;
			strThreadName.Format(_T("Thread%02d"), i+1);
			wchar_t chPrefix[1024];
			GetPrivateProfileString(L"Thread", strThreadName.GetString(), L"", chPrefix, 1024, exepath.c_str());
			wstring wstrRun = chPrefix;
			if(wstrRun == L"" || wstrRun == L"0")
			{

			}
			else
			{

				m_wstrExceptPrefixList.push_back(wstrRun.c_str());


				CString strEXEName;
				strEXEName.Format(_T("domlogic%02d.exe"), i+1);
				if(Project::Tools::FindProcessByName(strEXEName))
				{
					continue;
				}
				else
				{
					CString strInfo;
					strInfo.Format(_T("%s not in Process , will start!\r\n"), strEXEName);
					_tprintf(strInfo.GetString());
				}

				CString strCommand;
				strCommand.Format(_T("%s\\domlogic%02d.exe"), cstrFile.c_str(), i+1);
				strNeedRunExeCommandList.push_back(strCommand);
				strNeedRunParamList.push_back(L"");//wstrRun.c_str());

			}
		}

		for(int i=0;i<strNeedRunExeCommandList.size();i++)
		{
			
			::ShellExecute(NULL, _T("open"), strNeedRunExeCommandList[i].GetString(), strNeedRunParamList[i].GetString(), cstrFile.c_str(), SW_SHOW);
		}
		
	}


	//init dataaccess
	gDataBaseParam realDBParam;
	m_pDataEngineCore->ReadRealDBparam(realDBParam);
	if(realDBParam.strDBIP.length()<=0 ||
		realDBParam.strUserName.length()<=0)
	{
		PrintLog(L"ERROR: Read s3db Info(IP,Name) Failed, Please Check the s3db File.\r\n");	//edit 2016-03-11 S3db数据表project_config信息不见了？
		delete(m_pDataEngineCore);
		m_pDataEngineCore = NULL;
		return false;
	}

	//更新ini文件 服务器时间
	GetLocalTime(&m_stNow);
	wstring strTimeServer;
	Project::Tools::SysTime_To_String(m_stNow, strTimeServer);

	LoadDBSettingFromCoreConfig(realDBParam);

	if(m_pDataAccess_Arbiter == NULL)
		m_pDataAccess_Arbiter = new CBEOPDataAccess(this);
	m_pDataAccess_Arbiter->SetDBparam(realDBParam);


	CString strInfo;
	strInfo.Format(_T("Init Data Connection to %s\r\n"), m_pDataAccess_Arbiter->GetServerIP());
	PrintLog(strInfo.GetString());

	if(m_pDataAccess_Arbiter->InitConnection(true, true, true)==false)
	{
		m_pDataAccess_Arbiter->TerminateAllThreads();
		delete(m_pDataAccess_Arbiter);
		m_pDataAccess_Arbiter = NULL;
		return false;
	}


	wstring wstrAppNameTemp = wstrAppName.substr(0, wstrAppName.find(L".exe"));
	string strAppName = Project::Tools::WideCharToUtf8(wstrAppNameTemp.c_str());
	m_pDataAccess_Arbiter->UpdateHistoryStatusTable(strAppName);

	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"logicservertime",strTimeServer);
	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"logicstarttime",strTimeServer);		//Core 启动时间
	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"logicversion",CORE_VERSION);			//写入Core的版本号


	wstring wstrAppNameWithoutPostfix = wstrAppName.substr(0, wstrAppName.length()-4);
	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(_T("version_")+ wstrAppNameWithoutPostfix,CORE_VERSION);			//写入Core的版本号
	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(_T("version_")+ wstrAppNameWithoutPostfix,CORE_VERSION, strTimeServer);			//写入Core的版本号

	wstring wstrStartCount = L"0";
	if(m_pDataAccess_Arbiter->ReadCoreDebugItemValue(L"logicstartcount", wstrStartCount))
	{
		int nLogicStartCount = _wtoi(wstrStartCount.c_str());
		CString strCountPlus;
		strCountPlus.Format(_T("%d"), nLogicStartCount+1);
		m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"logicstartcount", strCountPlus.GetString());
	}


	PrintLog(L"Init Logic Engine...\r\n");
	m_pDataEngineCore->InitLogic();

	strInfo.Format(_T("Init DB Connection to %s\r\n"), Project::Tools::UTF8ToWideChar(m_pDataEngineCore->m_dbset.strDBIP.data()).c_str());
	PrintLog(strInfo.GetString());

	if(!m_pDataEngineCore->InitDBConnection())
	{
		PrintLog(L"ERROR: InitDBConnection Failed.\r\n");
		return false;
	}

	//init dataenginecore
	m_pDataEngineCore->SetDataAccess(m_pDataAccess_Arbiter);

	m_pDataEngineCore->SetDBAcessState(true);

	PrintLog(L"Reading Logics...\r\n");
	m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"runlogic", L"0");
	if(ReadLogic())
		PrintLog(L"Reading Logics Success!\r\n");
	else
	{
		PrintLog(L"Reading Logics Failed! No Logic will run, Please check logic file in DBFile.\r\n");
		return false;
	}


	//备份当前的策略运行状态，供检查用
	if(m_wstrPrefixList.size()==0) //只有main进程才有权限做这个分析
	{

		InitBackupAnalysisAllLogicStatus();
	}

	StartLogic();

	//clear unit02 when starting
	m_pDataAccess_Arbiter->ClearLogicParameters();

	wchar_t wcTemp[1024];
	wsprintf(wcTemp, L"%s engine started successfully!\r\n", tnow.Format(L"%Y-%m-%d %H:%M:%S ") );
	PrintLog(wcTemp);
	return true;
}

void CBeopGatewayCoreWrapper::UpdateCheckLogic()
{
	if(m_pLogicManager==NULL)
		return;

	int i = 0;
	wchar_t wsChar[1024];
	vector<wstring> wstrAllThreadRunValueList;

	vector<wstring> wstrAllThreadNameList;
	for (i=0;i<m_pLogicManager->m_vecDllThreadList.size();++i)
	{	
		CLogicDllThread *pDllThread = m_pLogicManager->m_vecDllThreadList[i];
		wstrAllThreadNameList.push_back(pDllThread->GetName());
	}

	m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValueMul(wstrAllThreadNameList, wstrAllThreadRunValueList);

	for (i=0;i<m_pLogicManager->m_vecDllThreadList.size();++i)
	{	
		CLogicDllThread *pDllThread = m_pLogicManager->m_vecDllThreadList[i];
		wstring strDllRunValue = wstrAllThreadRunValueList[i];

		if(strDllRunValue==L"1" &&pDllThread->GetRunStatus()==false)
		{
			wsprintf(wsChar, L"%s Logic Thread Loaded.\r\n",  pDllThread->GetName().c_str() );
			PrintLog(wsChar);
			pDllThread->SetRunStatus(true);		
		}
		else if(strDllRunValue==L"0" && pDllThread->GetRunStatus()==true)
		{
			wsprintf(wsChar, L"%s Logic Thread UnLoaded.\r\n", pDllThread->GetName().c_str());
			PrintLog(wsChar);
			pDllThread->SetRunStatus(false);		
		}
	}

	//扫描参数变动
	wstring strThreadName, strLogicName, strSetType, strVariableName, strLinkName, strLinkType, strImportTime,strAuthor, strPeriod, strDllContent, strRunStatus, strVersion, strDescription, strDllOldName, strNeedSaveDBFile;
	if(m_pDataAccess_Arbiter->ReadLogicParameters(strThreadName, strLogicName, strSetType, strVariableName, strLinkName, strLinkType, strImportTime,strAuthor, strPeriod, strDllContent, strRunStatus, strVersion, strDescription, strDllOldName, strNeedSaveDBFile))
	{
		CString strTemp;
		strTemp.Format(_T("Recv Logic Parameter Event of Logic:%s \r\n"), strLogicName.c_str());
		PrintLog(strTemp.GetString());
		CDLLObject *pObject = m_pLogicManager->GetDLLObjectByName(strLogicName.c_str());

		if(strLogicName.length()>0 && pObject!=NULL)
		{
			
			//修改db文件
			if(strSetType==L"0")//修改输入
			{
				PrintLog(L"Logic Parameter Input Changed, Updating...\r\n");
				
				
				if(strNeedSaveDBFile==L"1")
				{
					m_pLogicManager->SaveParametertoDB(0, strLogicName.c_str(), strVariableName.c_str(), strLinkName.c_str(), strLinkType.c_str());
				}
				
				pObject->SetDllInputParamter(strVariableName.c_str(), strLinkName.c_str(), strLinkType.c_str());//修改内存

				
			}
			else if(strSetType==L"1")//修改输出
			{
				PrintLog(L"Logic Parameter Output Changed, Updating...\r\n");
			
				if(strNeedSaveDBFile==L"1")
				{
					m_pLogicManager->SaveParametertoDB(1, strLogicName.c_str(), strVariableName.c_str(), strLinkName.c_str(), strLinkType.c_str());
				}
				pObject->SetDllOutputParamter(strVariableName.c_str(), strLinkName.c_str(), strLinkType.c_str());//修改内存
				
			}
			else if(strSetType==L"2")//修改策略时间
			{
				PrintLog(L"Logic Parameter Period Changed, Updating...\r\n");
			
				double fTimeSpan = _wtof(strVariableName.c_str());
				if(fTimeSpan<=0.f)
					fTimeSpan = 20.f;
				m_pLogicManager->SaveSpanTimetoDB(strLogicName.c_str(), _wtof(strVariableName.c_str()));
				m_pLogicManager->SetTimeSpan(pObject, fTimeSpan); //实时在线修改时间，立即生效，无需重启
				
			}
			else if(strSetType==L"3")//更新dll文件
			{
				PrintLog(L"Logic File Changed, Updating...\r\n");
				//先根据文件号码下载文件(mysql)
				if(strVariableName.length()>0)
				{
					//int nFileID = _wtoi(strVariableName.c_str());
					wstring cstrFile;
					Project::Tools::GetSysPath(cstrFile);
					wstring strDllPath;
					strDllPath = cstrFile + L"\\Temp\\";
					strDllPath += strVariableName.c_str();
					if(m_pDataAccess_Arbiter->DownloadFile(strVariableName, strDllPath))
					{
						bool bUpdate = m_pLogicManager->UpdateDllObjectByFile(strVariableName, strDllPath, strNeedSaveDBFile==L"1");
						if(bUpdate)
							PrintLog(L"Logic File Download and Update Successfully.\r\n");
						else
							PrintLog(L"ERROR: Logic File Download and Update Failed.\r\n");

						m_pDataAccess_Arbiter->DeleteFile(strVariableName);
					}
				}
					
			}/*
			else if(strSetType==L"4")			//新建策略
			{
				PrintLog(L"Add Logic, Updating...\r\n");

				if(strThreadName.length()>0 && strLogicName.length()>0 && strVariableName.length()>0)
				{
					//先根据文件号码下载文件(mysql)
					if(strVariableName.length()>0)		//新dll名称
					{
						//int nFileID = _wtoi(strVariableName.c_str());
						wstring cstrFile;
						Project::Tools::GetSysPath(cstrFile);
						wstring strDllPath;
						strDllPath = cstrFile + L"\\Temp\\";
						strDllPath += strVariableName.c_str();
						if(m_pDataAccess_Arbiter->DownloadFile(strVariableName, strDllPath))	//下载dll到temp目录
						{
							bool bUpdate = m_pLogicManager->UpdateDllObjectByFile(strSetType, strThreadName, strLogicName,strDllPath, strImportTime,strAuthor, strPeriod, strDllContent, strRunStatus, strVersion, strDescription, strDllOldName, strNeedSaveDBFile==L"1");
							if(bUpdate)
								PrintLog(L"Logic File Download and Update Successfully.\r\n");
							else
								PrintLog(L"ERROR: Logic File Download and Update Failed.\r\n");

							m_pDataAccess_Arbiter->DeleteFile(strVariableName);
						}
					}
				}
			}
			else if(strSetType==L"5")//删除策略
			{
				PrintLog(L"Delte Logic, Updating...\r\n");

				if(strThreadName.length()>0 && strLogicName.length()>0)
				{
					bool bUpdate = m_pLogicManager->UpdateDllObjectByFile(strSetType, strThreadName, strLogicName,L"", strImportTime,strAuthor, strPeriod, strDllContent, strRunStatus, strVersion, strDescription, strDllOldName, strNeedSaveDBFile==L"1");
					if(bUpdate)
						PrintLog(L"Logic File Download and Update Successfully.\r\n");
					else
						PrintLog(L"ERROR: Logic File Download and Update Failed.\r\n");

					m_pDataAccess_Arbiter->DeleteFile(strVariableName);
				}

			}
			else if(strSetType==L"6")//删除策略綫程
			{
				PrintLog(L"Delte Thread, Updating...\r\n");

				if(strThreadName.length()>0)
				{
					bool bUpdate = m_pLogicManager->UpdateDllObjectByFile(strSetType, strThreadName, strLogicName,L"", strImportTime,strAuthor, strPeriod, strDllContent, strRunStatus, strVersion, strDescription, strDllOldName, strNeedSaveDBFile==L"1");
					if(bUpdate)
						PrintLog(L"Logic File Download and Update Successfully.\r\n");
					else
						PrintLog(L"ERROR: Logic File Download and Update Failed.\r\n");

					m_pDataAccess_Arbiter->DeleteFile(strVariableName);
				}

			}*/
			m_pDataAccess_Arbiter->DeleteLogicParameters(strThreadName, strLogicName, strSetType, strVariableName, strLinkName, strLinkType);
		}
		else if(strVariableName.length()>0 &&  strSetType==L"7")//重启策略
		{
			vector<CDLLObject *> pDllTempList;
			bool bFound = m_pLogicManager->FindDllByOrginalName(strVariableName, pDllTempList);
			if(pDllTempList.size()>0)
			{
				CString strTemp;
				strTemp.Format(_T("Reload dll(orginal name:%s) from 4db, Updating...\r\n"), strVariableName);
				PrintLog(strTemp.GetString());


				bool bUpdate = m_pLogicManager->ReloadDllByOrginalName(strVariableName);
				if(bUpdate)
					PrintLog(L"Logic File Download and Update Successfully.\r\n");
				else
					PrintLog(L"ERROR: Logic File Download and Update Failed.\r\n");



				m_pDataAccess_Arbiter->DeleteLogicParameters(strThreadName, strLogicName, strSetType, strVariableName, strLinkName, strLinkType);
			}
		}

	}

}

bool CBeopGatewayCoreWrapper::StartLogic()
{

	for (int i=0;i<m_pLogicManager->m_vecDllThreadList.size();++i)
	{
		CLogicDllThread *pDllThread = m_pLogicManager->m_vecDllThreadList[i];

		PrintLog(pDllThread->GetStructureString());
		pDllThread->StartThread(); //策略线程启动，但是策略开关在其内部控制


	}
	return true;
}

bool CBeopGatewayCoreWrapper::ReadLogic()
{
	wstring wstrAppName;
	Project::Tools::GetAppName(wstrAppName);

	


	m_pLogicManager = new CBEOPLogicManager(m_pDataEngineCore->m_strDBFileName, NULL, m_wstrPrefixList, m_wstrExceptPrefixList);
	if(m_pLogicManager)
	{
		m_pLogicManager->m_pDataAccess = m_pDataAccess_Arbiter ;

		if(wstrAppName==L"domlogic.exe")// is main process
		{
			m_pLogicManager->InitSchedule();
		}

		PrintLog(L"Reading logics files from DB.\r\n");
		bool bReadInDB = m_pLogicManager->ReadDLLfromDB(m_pDataEngineCore->m_strDBFileName, m_pLogicManager->vecImportDLLList);
		if(!bReadInDB)
			return false;

		if(m_pLogicManager->vecImportDLLList.size() > 0)
			m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"runlogic", L"1");

		CString strTemp;
		strTemp.Format(_T("Read Logic count: %d.\r\n"), m_pLogicManager->vecImportDLLList.size());
		PrintLog(strTemp.GetString());

		PrintLog(L"Loading logic details.\r\n");
		bool bLoad = m_pLogicManager->LoadDllDetails();
		if(!bLoad)
			return false;


		PrintLog(L"Loading logic relationships.\r\n");
		m_pLogicManager->InitRelationships();

		CString strAllThreadName = _T("");

		//清除策略的心跳时间
		//m_pDataAccess_Arbiter->ClearCoreDebugItemValueLike(_T("LogicThread##"));

		int i =0;
		for (i=0;i<m_pLogicManager->m_vecDllThreadList.size();++i)
		{	
			CLogicDllThread *pDllThread = m_pLogicManager->m_vecDllThreadList[i];
			
			int ppcount = m_pLogicManager->m_vecDllThreadList[i]->GetPipelineCount();
			for(int j=0;j<ppcount;j++)
			{
				CLogicPipeline *pp = m_pLogicManager->m_vecDllThreadList[i]->GetPipeline(j);
				for(int k=0;k<pp->GetLogicDllCount();k++)
				{
					CString strDllName = pp->GetLogicObject(k)->GetDllName().c_str();
					strAllThreadName+=pDllThread->GetName().c_str();
					strAllThreadName+=_T("##");
					strAllThreadName+=strDllName;
					strAllThreadName+=_T(",");
				}

				
			}
			
			if(!m_pDataEngineCore->m_bUpdateInputTableFromSite)
			{//仿真模式下直接运行策略
				wstring strRunnableValue = L"0";
				bool bRead = m_pDataAccess_Arbiter->ReadCoreDebugItemValue(pDllThread->GetName(), strRunnableValue);
				if(bRead)
				{//读到策略值就认为已经配置过，那么保留上次的开关
					if(strRunnableValue==L"1")
						pDllThread->SetRunStatus(true);
					else
						pDllThread->SetRunStatus(false);
				}
				else
				{//读不到策略值的话，就认为数据库刚建，在demo下用仿真，默认应该打开所有策略
					m_pDataAccess_Arbiter->WriteCoreDebugItemValue(pDllThread->GetName(), L"1");
					pDllThread->SetRunStatus(true);
				}
			}
			else
			{	
				//现场模式下，只有读到策略属性打开才开，其他情况均默认关闭
				wstring strDllRunValue = m_pDataAccess_Arbiter->ReadOrCreateCoreDebugItemValue(pDllThread->GetName());

				if(strDllRunValue==L"1")
					pDllThread->SetRunStatus(true);
				else
					pDllThread->SetRunStatus(false);
			}

		}


		if(strAllThreadName.GetLength()>0)
		{
			strAllThreadName = strAllThreadName.Left(strAllThreadName.GetLength()-1);
		}
		//init logic health
		CHttpOperation http(_T("127.0.0.1"), 5000, _T("logic/updateHealthLog"));
		Json::Value myRoot;
		myRoot["threadNameList"]=Project::Tools::WideCharToUtf8(strAllThreadName);
		string szBuf = myRoot.toStyledString();
		CString strResult;
		bool bRet = http.HttpPostGetRequestEx(0, "1", strResult,  _T("Content-Type: application/json"));
		if(bRet)
		{
			Json::Reader jsReader;
			string strResult_utf8;
			Project::Tools::WideCharToUtf8(strResult.GetString(), strResult_utf8);
			Json::Value myReturnRoot;
			if (!jsReader.parse(strResult_utf8, myReturnRoot))
			{
				PrintLog(_T("updateHealthLog by dompysite failed\r\n"), true);
			}
		}


		PrintLog(_T("updateHealthLog by dompysite success\r\n"), true);
		return true;
	}
	return false;
}

void CBeopGatewayCoreWrapper::SaveAndClearLog()
{
	if(g_strLogAll.size()<=0)
		return;

	if(m_wstrAppName.length()==0)
	{
		wstring wstrTemp;
		Project::Tools::GetAppName(wstrTemp);

		m_wstrAppName = wstrTemp.substr(0, wstrTemp.find(L".exe"));
	}

	//if(m_pDataAccess_Arbiter)
	//	m_pDataAccess_Arbiter->InsertLog(g_tLogAll, g_strLogAll);

	CString strLogAll;
	for(int i=0;i<g_tLogAll.size();i++)
	{
		if(i>=g_strLogAll.size())
			continue;

		string time_utf8 = Project::Tools::SystemTimeToString(g_tLogAll[i]);

		wstring wstrTime ;
		Project::Tools::UTF8ToWideChar(time_utf8, wstrTime);

		CString strOneItem;
		strOneItem.Format(_T("%s    %s"), wstrTime.c_str(), g_strLogAll[i].c_str());
		strLogAll+=strOneItem;
	}

	try
	{
		wstring strPath;
		GetSysPath(strPath);
		strPath += L"\\log";
		if(Project::Tools::FindOrCreateFolder(strPath))
		{
			COleDateTime oleNow = COleDateTime::GetCurrentTime();
			CString strLogPath;
			strLogPath.Format(_T("%s\\%s_err_%d_%02d_%02d.log"), strPath.c_str(), m_wstrAppName.c_str(), oleNow.GetYear(),oleNow.GetMonth(),oleNow.GetDay());


			char* old_locale = _strdup( setlocale(LC_CTYPE,NULL) );
			setlocale( LC_ALL, "chs" );
			//记录Log
			CStdioFile	ff;
			if(ff.Open(strLogPath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
			{
				ff.Seek(0,CFile::end);
				ff.WriteString(strLogAll);
				ff.Close();
			}
			setlocale( LC_CTYPE, old_locale ); 
			free( old_locale ); 	
		}
	}
	catch (CMemoryException* e)
	{

	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}


	g_strLogAll.clear();
	g_tLogAll.clear();

}

void CBeopGatewayCoreWrapper::PrintLog(const wstring &strLog, bool bCache)
{
	_tprintf(strLog.c_str());
	SYSTEMTIME st;
	GetLocalTime(&st);
	g_tLogAll.push_back(st);
	g_strLogAll.push_back(strLog);
	if(bCache == false)
	{
		SaveAndClearLog();
	}
	else if(g_strLogAll.size()>=100)
	{
		// 超过一定条数量，将记录到数据库，清空
		SaveAndClearLog();
	}
}

std::wstring CBeopGatewayCoreWrapper::Replace( const wstring& orignStr, const wstring& oldStr, const wstring& newStr, bool& bReplaced )
{
	size_t pos = 0;
	wstring tempStr = orignStr;
	wstring::size_type newStrLen = newStr.length();
	wstring::size_type oldStrLen = oldStr.length();
	bReplaced = false;
	while(true)
	{
		pos = tempStr.find(oldStr, pos);
		if (pos == wstring::npos) break;

		tempStr.replace(pos, oldStrLen, newStr);        
		pos += newStrLen;
		bReplaced = true;
	}

	return tempStr; 
}

bool CBeopGatewayCoreWrapper::OutPutLogString( CString strOut )
{
	COleDateTime tnow = COleDateTime::GetCurrentTime();
	CString strLog;
	strLog += tnow.Format(_T("%Y-%m-%d %H:%M:%S "));
	strLog += strOut;
	strLog += _T("\n");

	//记录Log
	CStdioFile	ff;
	if(ff.Open(m_strLogFilePath.c_str(),CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
	{
		ff.Seek(0,CFile::end);
		ff.WriteString(strLog);
		ff.Close();
		return true;
	}
	return false;
}

bool CBeopGatewayCoreWrapper::Exit()
{
	m_bExitCore = true;
	if(m_pDataEngineCore)
	{
		m_pDataEngineCore->Exit();
		delete m_pDataEngineCore;
		m_pDataEngineCore = NULL;
	}


	return true;
}

bool CBeopGatewayCoreWrapper::FeedDog()
{
	//return CDogTcpCtrl::GetInstance()->SendActiveSignal();
	return true;
}

bool CBeopGatewayCoreWrapper::Release()
{
	if(m_pLogicManager)
	{
		delete m_pLogicManager;
		m_pLogicManager = NULL;
	}

	if(m_pDataEngineCore)
	{
		m_pDataEngineCore->SetDBAcessState(false);
		m_pDataEngineCore->TerminateScanWarningThread();
		m_pDataEngineCore->TerminateReconnectThread();

		m_pDataEngineCore->Exit();
		delete(m_pDataEngineCore);
		m_pDataEngineCore = NULL;

	}

	if(m_pDataAccess_Arbiter)
	{
		m_pDataAccess_Arbiter->TerminateAllThreads();
		delete(m_pDataAccess_Arbiter);
		m_pDataAccess_Arbiter = NULL;
	}

	return true;
}

bool CBeopGatewayCoreWrapper::AddVirtualPointToDBFile(const wstring & wstrPointName, const wstring & wstrPointDescription)
{

	return m_pDataEngineCore->AddVirtualPointToDBFile(wstrPointName,wstrPointDescription );
}


wstring CBeopGatewayCoreWrapper::GetLocalConfig(wstring wstrConfigName)
{
	return m_pDataEngineCore->GetLocalConfig(wstrConfigName );
}