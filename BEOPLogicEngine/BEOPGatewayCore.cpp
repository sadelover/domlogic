// BEOPGatewayCore.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "BEOPGatewayCore.h"
#include "winsock.h"
#include <Winsvc.h>
#include "BeopGatewayCoreWrapper.h"
#include "CoreUnitTest.h"
#include "conio.h"
#include "../redis/hiredis.h"
#include "../BEOPDataEngineCore/DLLObject.h"

#pragma comment(lib, "advapi32")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



//#pragma comment(linker,"/subsystem:windows")
#pragma comment(linker,"/subsystem:\"Windows\" /entry:\"wmainCRTStartup\"")

// 唯一的应用程序对象

CWinApp theApp;


using namespace std;
CBeopGatewayCoreWrapper *m_pGatewayCoreWrapper = NULL;

bool g_bSingleThread;
Project::Tools::Mutex	g_thread_lock;

// 注意服务名和服务显示名不一定相同，后者是一个友好名
bool IsSvcRun(LPCTSTR lpszSvcName)
{ 
	SERVICE_STATUS svcStatus = {0};
	return QueryServiceStatus(OpenService(OpenSCManager(NULL, NULL, GENERIC_READ), lpszSvcName, GENERIC_READ), &svcStatus) ? (svcStatus.dwCurrentState == SERVICE_RUNNING) : false; 
}

bool ctrlhandler( DWORD fdwctrltype )
{
	switch( fdwctrltype )
	{
		// handle the ctrl-c signal.
	case CTRL_C_EVENT:
		printf( "ctrl-c event\n" );
		break;
		// ctrl-close: confirm that the user wants to exit.
	case CTRL_CLOSE_EVENT:
		printf( "ctrl-close event\n" );
		return m_pGatewayCoreWrapper->Exit();
	case CTRL_BREAK_EVENT:
		printf( "ctrl-break event\n" );
		break;
	case CTRL_LOGOFF_EVENT:
		printf( "ctrl-logoff event\n" );
		break;
	case CTRL_SHUTDOWN_EVENT:
		printf( "ctrl-shutdown event\n" );
		break;
	default:
		break;
	}
	return false;
}


wstring LoadLogicConfig(int nLogicProcessNumber)
{
	wstring cstrFile;
	Project::Tools::GetSysPath(cstrFile);
	wstring exepath;
	exepath = cstrFile + L"\\domlogic.ini";
	CString strLogFileName;

	vector<CString> strNeedRunExeCommandList;
	vector<CString> strNeedRunParamList;


	CString strThreadName;
	strThreadName.Format(_T("Thread%02d"), nLogicProcessNumber);
	wchar_t chPrefix[1024];
	GetPrivateProfileString(L"Thread", strThreadName.GetString(), L"", chPrefix, 1024, exepath.c_str());
	wstring wstrRun = chPrefix;

	return wstrRun;
}



int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		setlocale(LC_ALL, ".936"); 
		// 初始化 MFC 并在失败时显示错误
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO: 更改错误代码以符合您的需要
			_tprintf(L"Error: MFC Init Failed\n");
			nRetCode = 1;
		}
		else
		{
			// TODO: 在此处为应用程序的行为编写代码。

#ifdef BEOP_RUNTEST

			CCoreUnitTest coreTest;
			if(coreTest.RunTest())
				AfxMessageBox(L"TEST APPROVED....\r\n");
			else
				AfxMessageBox(L"TEST ERROR!!!!!!!ERROR!!!!!ERROR!!!!!ERROR!!!!!....\r\n");
			kbhit();


#else

			wstring wstrAppName;
			Project::Tools::GetAppName(wstrAppName);
			if(SetConsoleCtrlHandler( (PHANDLER_ROUTINE) ctrlhandler, true ))
			{

				vector<wstring> wstrPrefixList;
				if(wstrAppName==L"domlogic01.exe")
				{
					wstrPrefixList.push_back(LoadLogicConfig(1));
				}
				else if(wstrAppName==L"domlogic02.exe")
				{
					wstrPrefixList.push_back(LoadLogicConfig(2));
				}
				else if(wstrAppName==L"domlogic03.exe")
				{
					wstrPrefixList.push_back(LoadLogicConfig(3));
				}
				else if(wstrAppName==L"domlogic04.exe")
				{
					wstrPrefixList.push_back(LoadLogicConfig(4));
				}
				else if(wstrAppName==L"domlogic05.exe")
				{
					wstrPrefixList.push_back(LoadLogicConfig(5));
				}
				else if(wstrAppName==L"domlogic016.exe")
				{
					wstrPrefixList.push_back(LoadLogicConfig(6));
				}
				else if(wstrAppName==L"domlogic017.exe")
				{
					wstrPrefixList.push_back(LoadLogicConfig(7));
				}
				else if(wstrAppName==L"domlogic08.exe")
				{
					wstrPrefixList.push_back(LoadLogicConfig(8));
				}
				else if(wstrAppName==L"domlogic09.exe")
				{
					wstrPrefixList.push_back(LoadLogicConfig(9));
				}
				else if(wstrAppName==L"domlogic10.exe")
				{
					wstrPrefixList.push_back(LoadLogicConfig(10));
				}
				else if(wstrAppName==L"domlogic.exe")
				{
					if(argc==2) //分析策略用
					{
						wstring wstrLogicFilePath = argv[1];

						CBEOPDataAccess *pDataAccess = new CBEOPDataAccess(NULL);
						pDataAccess->InitConnection(false, false, true);
						CDLLObject *pDLLObject = new CDLLObject(pDataAccess);
						pDLLObject->Load(wstrLogicFilePath);

						//分析完就退出
						return 0;
					}
					for(int i=1;i<argc;i++)
					{
						wstrPrefixList.push_back(argv[i]);
					}
				}
	
				m_pGatewayCoreWrapper = new CBeopGatewayCoreWrapper(wstrPrefixList);

				CString strTemp;
				strTemp.Format(_T("ProcessInfo: Init ProcessName:%s, PrefixList size:%d. argc:%d\r\n"), wstrAppName.c_str(), wstrPrefixList.size(), argc);
				m_pGatewayCoreWrapper->PrintLog(strTemp.GetString());
				for(int i=0;i<argc;i++)
				{
					strTemp.Format(_T("ProcessInfo: arg index:%d. argv:%s\r\n"), i, argv[i]);
					m_pGatewayCoreWrapper->PrintLog(strTemp.GetString());
				}

				m_pGatewayCoreWrapper->Run();
			}
#endif
		}
	}
	else
	{
		// TODO: 更改错误代码以符合您的需要
		_tprintf(L"Error: GetModuleHandle fails.\n");
		nRetCode = 1;
	}

	return nRetCode;
}
