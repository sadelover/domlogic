#include "StdAfx.h"
#include "CoreUnitTest.h"
#include "BeopGatewayCoreWrapper.h"


CCoreUnitTest::CCoreUnitTest(void)
{
}


CCoreUnitTest::~CCoreUnitTest(void)
{
}

bool CCoreUnitTest::RunTest()
{
	bool bSuccess = true;
	Test08();
	TestAddVPoint();
	return true;
}

bool CCoreUnitTest::RunCommon()
{
	CBeopGatewayCoreWrapper coreWrapper;
	coreWrapper.Run();

	return true;

}


bool CCoreUnitTest::Test00()
{

	wstring cstrFile;
	Project::Tools::GetSysPath(cstrFile);
	wstring strTestFilePath = cstrFile + L"\\Test\\TEST01.s3db";
	wstring strTestFilePath2 = cstrFile + L"\\Test\\TEST02.s3db";

	//重启两次
	CBeopGatewayCoreWrapper coreWrapper;
	for(int i=0;i<2;i++)
	{
		coreWrapper.Init(strTestFilePath);
		coreWrapper.Reset();

	}

	//继续运行

	coreWrapper.Init(strTestFilePath2);
	for(int i=0;i<100;i++)
	{
		Sleep(100);

		_tprintf(L"Loop once test00\r\n");
		
		//update logic
		coreWrapper.UpdateCheckLogic();

		//update restart
		coreWrapper.UpdateResetCommand();

	}

	coreWrapper.Reset();

	return true;
}

bool CCoreUnitTest::Test01()
{
	bool bSuccess = true;


	return bSuccess;
}

bool CCoreUnitTest::Test02()
{
	bool bSuccess = true;
	wstring cstrFile;
	Project::Tools::GetSysPath(cstrFile);
	wstring strTestFilePath = cstrFile + L"\\Test\\TEST02.s3db";
	wstring strTestDllFilePath = cstrFile + L"\\Test\\PIDMultiIOtest.dll";
	wstring strTestOldDllFilePath = cstrFile + L"\\Test\\PIDMultiIOcommon.dll";


	CBeopGatewayCoreWrapper coreWrapper;
	coreWrapper.Init(strTestFilePath);
	coreWrapper.m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"CT_PID", L"1");
	coreWrapper.m_pDataAccess_Arbiter->InsertLogicParameters(L"CT_PID", L"CT_PID.dll", L"2", L"1", L"", L"", L""); //修改策略周期
	coreWrapper.UpdateCheckLogic();

	for(int i=0;i<1;i++)
	{
		Sleep(10000);

		coreWrapper.m_pDataAccess_Arbiter->InsertLogicParameters(L"", L"", L"3", L"PIDMultiIO.dll", L"", L"", L"");
		coreWrapper.m_pDataAccess_Arbiter->UpdateLibInsertIntoFilestorage(strTestOldDllFilePath, L"PIDMultiIO.dll", L"PIDMultiIO.dll");
		coreWrapper.UpdateCheckLogic();
		
		Sleep(10000);

		coreWrapper.m_pDataAccess_Arbiter->InsertLogicParameters(L"", L"", L"3", L"PIDMultiIO.dll", L"", L"", L"");
		coreWrapper.m_pDataAccess_Arbiter->UpdateLibInsertIntoFilestorage(strTestDllFilePath, L"PIDMultiIO.dll", L"PIDMultiIO.dll");
		coreWrapper.UpdateCheckLogic();


	}

	coreWrapper.Reset();

	return true;
}


bool CCoreUnitTest::Test03()
{
	bool bSuccess = true;
	wstring cstrFile;
	Project::Tools::GetSysPath(cstrFile);
	wstring strTestFilePath = cstrFile + L"\\Test\\Wanda_Qingdao_Store_20140820.s3db";


	CBeopGatewayCoreWrapper coreWrapper;
	coreWrapper.Init(strTestFilePath);
	coreWrapper.m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"sitemode", L"0");
	coreWrapper.m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"1Click", L"1");
	coreWrapper.UpdateCheckLogic();

	for(int i=0;i<10;i++)
	{
		Sleep(5000);
		coreWrapper.m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"1Click", L"0");
		coreWrapper.UpdateCheckLogic();


		Sleep(5000);

		coreWrapper.m_pDataAccess_Arbiter->WriteCoreDebugItemValue(L"1Click", L"1");
		coreWrapper.UpdateCheckLogic();



	}

	coreWrapper.Reset();

	return true;
}
bool CCoreUnitTest::TestDeleteDBAndRebuild()
{
	bool bSuccess = true;

	return true;
}

bool CCoreUnitTest::Test04()
{
	bool bSuccess = true;
	
	return bSuccess;
}

bool CCoreUnitTest::Test05()
{

	return true;
}

bool CCoreUnitTest::Test06()
{

	return true;
}

bool CCoreUnitTest::Test08()//by golding, testing  GetHistoryValue
{
	CBeopGatewayCoreWrapper coreWrapper;
	wstring m_strDBFilePath = coreWrapper.PreInitFileDirectory();

	if(m_strDBFilePath.length()==0)
	{
		m_strDBFilePath = coreWrapper.PreInitFileDirectory_4db();
	}

	if(coreWrapper.Init(m_strDBFilePath))
	{


	}

	while(true)
	{
		SYSTEMTIME  stStart;
		SYSTEMTIME  stEnd;


		COleDateTime	OleTimeStart, OleTimeEnd;
		OleTimeEnd = COleDateTime::GetCurrentTime();
		OleTimeEnd.SetDate(2018,4,27);
		OleTimeEnd.GetAsSystemTime( stEnd );

		OleTimeStart = OleTimeEnd - COleDateTimeSpan(1,0,0,0);
		OleTimeStart.GetAsSystemTime( stStart );

		stEnd.wHour=23;

		wstring wstrResult;
		bool bGot1 = coreWrapper.m_pDataAccess_Arbiter->GetHistoryValue(_T("ChPower01"), stStart, stEnd, 2, wstrResult);
		_tprintf(wstrResult.c_str());

		OleTimeEnd -=COleDateTimeSpan(1,0,0,0);
		OleTimeEnd.GetAsSystemTime( stEnd );
		bGot1 = coreWrapper.m_pDataAccess_Arbiter->GetHistoryValue(_T("ChPower01"), stStart, stEnd, 2, wstrResult);
		_tprintf(wstrResult.c_str());

		//get data at time
		OleTimeStart = COleDateTime::GetCurrentTime();
		OleTimeStart.GetAsSystemTime( stStart );
		stStart.wDay-=1;
		stStart.wSecond=0;
		stStart.wMilliseconds = 0;
		bool bGot2 = coreWrapper.m_pDataAccess_Arbiter->GetHistoryValue(_T("SecChWPGroupPowerTotal"), stStart, 2, wstrResult);
		_tprintf(wstrResult.c_str());

		Sleep(5000);
	}

	return true;
}



bool CCoreUnitTest::TestAddVPoint()//by golding, testing  GetHistoryValue
{
	CBeopGatewayCoreWrapper coreWrapper;
	wstring m_strDBFilePath = coreWrapper.PreInitFileDirectory();

	if(m_strDBFilePath.length()==0)
	{
		m_strDBFilePath = coreWrapper.PreInitFileDirectory_4db();
	}

	if(coreWrapper.Init(m_strDBFilePath))
	{


	}

	while(true)
	{

		wstring wstrResult;
		bool bGot1 = coreWrapper.m_pDataAccess_Arbiter->AddVirtualPointToDBFile(_T("AAAA"), _T("这是一个测试点"));
		
		wstring wstrValue = coreWrapper.m_pDataAccess_Arbiter->GetLocalConfig(_T("globalconfig"));

		Sleep(5000);
	}

	return true;
}

