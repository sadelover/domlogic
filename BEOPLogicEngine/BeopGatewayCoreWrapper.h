#pragma once

#include "../BEOPDataEngineCore/BEOPDataEngineCore.h"
#include "../LAN_WANComm/NetworkComm.h"
#include "../ServerDataAccess/BEOPDataAccess.h"
#include "../BEOPDataEngineCore/RedundencyManager.h"
#include "../Tools/CustomTools/CustomTools.h"
#include "../BEOPDataEngineCore/BEOPLogicManager.h"

#include "CoreFileTransfer.h"
#include "../BEOPDataPoint/DataPointManager.h"
#include "../BEOPDataEngineCore/LogicDllThread.h"
#include "../DB_BasicIO/BEOPDataBaseInfo.h"
#include "../Tools/Util/UtilsFile.h"


class CBeopGatewayCoreWrapper
{
public:
	CBeopGatewayCoreWrapper();
	CBeopGatewayCoreWrapper(vector<wstring> wstrPrefixList);
	~CBeopGatewayCoreWrapper(void);

	void InitBackupAnalysisAllLogicStatus();

	bool LoadDBSettingFromCoreConfig(gDataBaseParam &dbset);

	bool Run();
	bool Exit();
	bool UpdateResetCommand();
	bool Reset();
	bool Init(wstring strDBFilePath);
	bool Release();

	wstring PreInitFileDirectory();
	wstring PreInitFileDirectory_4db();
	void RunLogic();
	void SaveAndClearLog();
	void PrintLog(const wstring &strLog, bool bCache = false);
	void UpdateCheckLogic();
	bool ReadLogic();
	bool StartLogic();
	std::wstring Replace(const wstring& orignStr, const wstring& oldStr, const wstring& newStr, bool& bReplaced);
	bool OutPutLogString(CString strOut);
	bool FeedDog();				//Î¹¹·

	bool AddVirtualPointToDBFile(const wstring & wstrPointName, const wstring & wstrPointDescription);
	wstring GetLocalConfig(wstring wstrConfigName);

	bool InitSyslogicParameters();
	bool InitSyslogicParameter(CString strDllPath);
public:

	CBeopDataEngineCore *m_pDataEngineCore;
	CBEOPDataAccess * m_pDataAccess_Arbiter;
	CBEOPLogicManager *m_pLogicManager;
	bool m_bFirstInit;
	bool m_bExitCore;

	vector<SYSTEMTIME> g_tLogAll;
	vector<wstring> g_strLogAll;
	wstring m_strDBFilePath;
	wstring m_strLogFilePath;
	wstring m_wstrAppName;
	SYSTEMTIME m_stNow;

	vector<wstring> m_wstrPrefixList;
	vector<wstring> m_wstrExceptPrefixList;

};

