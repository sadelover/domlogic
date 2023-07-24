
#pragma once
// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� BeopDATAENGINECORE_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// BeopDATAENGINECORE_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�


#include "DB_BasicIO/RealTimeDataAccess.h"
#include "../ServerDataAccess/BEOPDataAccess.h"
#include "DataBaseSetting.h"

class CBEOPDataLinkManager;
class CDataPointManager;
class CLogicPointManager;
class CAlarmPointManager;
class Beopdatalink::CRealTimeDataAccess;
class Beopdatalink::CCommonDBAccess;
class CMDOPointConfigDataAccess;
//class CDTUSender;
class CEngineRegister;


// �����Ǵ� BeopDataEngineCore.dll ������
class CBeopDataEngineCore 
{

public:
	CBeopDataEngineCore(std::wstring strDBFilePath);
	~CBeopDataEngineCore(void);

	bool InitDBConnection();
	bool InitEngine();
	bool InitLogic();
	bool Exit();
	bool Init(bool bDTUEnabled,bool bDTUChecked,bool bDTURecCmd,int nDTUPort,int nDTUSendMinType,bool bModbusReadOne,int nModbusInterval,bool bDTUDisableSendAllData,bool bTCPEnable,wstring strTCPName,wstring strTCPServerIP,int nTCPServerPort,int nSendAllData);
	bool TerminateScanWarningThread();
	bool TerminateReconnectThread();
	void SetDataAccess(CBEOPDataAccess *pAccess);
	void	SetMsgWnd(CWnd* msgwnd);

	Beopdatalink::CRealTimeDataAccess* GetRealTimeDataAccess();
	Beopdatalink::CCommonDBAccess * GetCommonDBAccess();
	Beopdatalink::CLogDBAccess * GetLogDBAccess();
	void SetRealTimeDataAccess(Beopdatalink::CRealTimeDataAccess* pAccess);
	void SetCommonDBAccess(Beopdatalink::CCommonDBAccess *pAccess);
	void SetLogDBAccess(Beopdatalink::CLogDBAccess *pAccess);
	bool SetDBAcessState( bool bState );
	void ReadRealDBparam(gDataBaseParam &dbParam, wstring strDefaultDB = L"beopdata");
	bool AddVirtualPointToDBFile(const wstring & wstrPointName, const wstring & wstrPointDescription);
	wstring GetLocalConfig(wstring wstrConfigName);

	void EnableLog(BOOL bEnable);
	//CDTUSender * GetDTUSender();

	bool	InitPoints();

	CDataPointManager* GetDataPointManager();
	//CBEOPDataAccess *GetServerDataAccess();

	bool	InsertRedundancyOperation(wstring wstrRedundancyIp);
	
	DataBaseSetting LoadConfig();
	bool LoadDBSettingFromCoreConfig(DataBaseSetting &dbset);
	bool SaveConfig(DataBaseSetting &dbset);
	wstring GetConfigVersion();



	bool CheckDBExist(wstring wstrDBName = L"beopdata");
	bool ReBuildDatabase(wstring wstrDBName = L"beopdata");
	bool DropDatabase(wstring wstrDBName = L"beopdata");
	bool DatabaseUpdateFrom22To23(wstring wstrDBName = L"beopdata");
	double GetMySQLDBVersion(wstring wstrDBName = L"beopdata");

	bool	SetDefaultDBIP(string strIP);
	bool	SetDefaultDBUser(string strUser);

	bool ReconnectRedundancyDB();
	string	RelaceStr(char* strSrc, char* strFind, char* strReplace);

	static  UINT WINAPI ThreadFunc_ScanWarning(LPVOID lparam);
	static  UINT WINAPI ThreadFunc_ConnectRedundancyDB(LPVOID lparam);
private:

	void	InitCommandLog();

private:
	CDataPointManager*	m_pointmanager;
	Beopdatalink::CRealTimeDataAccess* m_dbcon;
	Beopdatalink::CCommonDBAccess* m_dbcon_common;
	Beopdatalink::CCommonDBAccess* m_dbcon_Redundancy;
	Beopdatalink::CLogDBAccess*	m_dbcon_log;
	CBEOPDataAccess *m_pDataAccess;
	//CDTUSender*	m_pdtusender;


	CEngineRegister*	m_pEngineRegister;


	BOOL m_bLog;

public:

	HANDLE  m_hThread_WarningScan;
	HANDLE  m_hThread_ReconRedundancyDB;

public:
	//���ò���
	wstring m_strDBFileName;
	DataBaseSetting m_dbset;
	BOOL m_bUpdateInputTableFromSite;

		int m_nSendPackageInterval;
	int m_nMainDropInterval;		

	int m_nSystemType;

	bool m_bExitThread;
	bool m_bThreadExitFinished;
	bool m_bDBAcessSuccess; //���ݿ������Ƿ����ɹ�
};