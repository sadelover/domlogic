
/*
 *
 *������ʵʱ��λ���ݵķ��ʣ���ȡ�ࡣ
 *
 *
 *
 *
 **/

#pragma once


#include "DatabaseSession.h"

#include "../Tools/CustomTools/CustomTools.h"
#include "../DB_BasicIO/WarningConfigItem.h"
#include "../Tools/USingletonTemplate.h"

#include "../BEOPDataPoint/DataPointEntry.h"
#include "WarningEntry.h"

#include "wtypes.h"
#include <vector>
#include <string>
#include <utility>
#include <hash_map>
#include "env.h"

#define RUNTIME_DB_NAME (L"beopdata")

using std::vector;
using std::string;
using std::wstring;
using std::pair;

namespace Beopdatalink
{
	struct  unitStateInfo
	{
		string unitproperty01;		
		string unitproperty02;
		string unitproperty03;
	};

	struct ErrInfo
	{
		int nErrCode;
		string strErrInfo;
	};

	struct ErrCodeInfo
	{
		int nErrCode;
		COleDateTime oleErrTime;
	};

	struct ErrPointInfo
	{
		string		strPointName;
		int			nPointType;
		int			nErrCode;
		COleDateTime oleErrTime;
	};

	struct ScheduleInfo
	{
		int			nID;			//�����¼����
		int         nWeedday;		//�ܼ�
		int			nGroupID;		//��ID
		int			nLoop;			//�Ƿ���ѭ��
		int			nEnable;		//�Ƿ�����
		string		strTimeFrom;	//��ʼʱ��
		string		strTimeTo;		//����ʱ��
		string		strPoint;		//����
		string		strValue;		//��ֵ
		string      strName;        //�ճ���
		string		strAuthor;		//������
		SYSTEMTIME  sExecute;		//ִ��ʱ��
		bool        bRealActed;     //����ִ�й�
		int         nType;
	};

	class  CRealTimeDataEntry
	{
	public:
		SYSTEMTIME GetTimestamp() const;
		void	SetTimestamp(const SYSTEMTIME& stime);

		string	GetName() const;
		void	SetName(const string& name);
		
		wstring	GetValue() const;
		void	SetValue(const wstring& value);

		bool	IsEngineSumPoint();
	private:
		SYSTEMTIME	m_timestamp;					// current timestamp
		string		m_pointname;					// the point name. shortname
		wstring		m_pointvalue;					// the real value.
	};

	//
	class  CRealTimeDataStringEntry: public CRealTimeDataEntry
	{
	public:
		wstring	GetValue() const;
		void	SetValue(const wstring& value);
	private:
		wstring		m_strvalue;		// with utf8 format.
	};
	

	class  CRealTimeDataEntry_Output : public CRealTimeDataEntry
	{
	public:
		CRealTimeDataEntry_Output();
		void	SetInit(bool binit);

		bool	IsValueInit();
	private:
		bool	m_binit;
	};

	// class 
	class  CRealTimeDataAccess : public CDatabaseSeesion
	{
	public:

	
		//==================���doubleֵ�Ĳ��룬��ȡ==============================
		// for double values
		bool	UpdatePointValue_Input(const CRealTimeDataEntry& entry);
		bool	UpdatePointValue_InputArray(const vector<CRealTimeDataEntry> & entryList);
		bool	UpdatePointValue_InputBuffer(const CRealTimeDataEntry& entry);
		bool	DeleteRealTimeData_Input();
		// insert double entries.
		bool	InsertRealTimeDatas_Input(const vector<CRealTimeDataEntry>& entrylist,int nMaxInsert=10000);
		bool	UpdateRealTimeDatas_Input(const vector<CRealTimeDataEntry>& entrylist);
		// read double entries.
		bool	ReadRealtimeData_Input(vector<CRealTimeDataEntry>& entrylist);
		
		// for write doublevalues
		bool	ReadRealTimeData_Output(vector<CRealTimeDataEntry>& entrylist);
		bool	UpdatePointData(const CRealTimeDataEntry& entry);
		bool	UpdatePointData(vector<CRealTimeDataEntry>& entrylist);
		bool	DeleteRealTimeData_Output();
		bool	DeleteEntryInOutput(const string& pointname);

		bool	DeleteRealTimeData_PointBuffer();
		bool    InsertPointBuffer(const vector<CRealTimeDataEntry>& datalist);

		//bool	ReadRealTimeData_Unit01(vector<unitStateInfo>& unitlist);
		bool	InsertUnit01(const vector<unitStateInfo>& unitlist,const string reip);

		bool	InsertErrInfo(const vector<ErrInfo> errlist);
		bool	InsertErrCodeInfo(const vector<ErrCodeInfo> errlist);
		bool	InsertErrPoints(const vector<ErrPointInfo> errlist);

		double    CheckDBVersion();

		bool    WriteCoreDebugItemValue(const wstring &strItemName, const wstring &strItemValue);
	private:
		Project::Tools::Mutex m_lock;
	};

	class  CLogDBAccess : public CDatabaseSeesion
	{
	public:
		CLogDBAccess(){
			m_nLogBatchCount = 100;
			GetLocalTime(&m_tLastInsertLog); 

			Project::Tools::GetSysPath(m_wstrFilePath);
		};
		std::string GetSourceFromLog(wstring wstrLog);
		bool	InsertLog(const wstring& loginfo);
		bool	InsertLogToDB(const vector<SYSTEMTIME> & sysTimeList, const vector<wstring> & loginfoList);
		bool	InsertLogToDiskFile(const vector<wstring> & strLogicNameList, const vector<SYSTEMTIME> & sysTimeList, const vector<wstring> & loginfoList);
		
		bool	ReadLog(vector< pair<wstring,wstring> >& loglist, const SYSTEMTIME& start, const SYSTEMTIME& end);
		bool	ReadLog( vector< pair<wstring,wstring> >& loglist, const int nCount);

		bool	ExportLog(const SYSTEMTIME& start, const SYSTEMTIME& end, const wstring& filepath);

		bool    DeleteLog(const SYSTEMTIME tEnd);
	private:
		Project::Tools::Mutex m_lock;

		vector<wstring> m_strLogicNameList;
		vector<SYSTEMTIME> m_TimeList;
		vector<wstring>  m_strLogList;
		SYSTEMTIME m_tLastInsertLog;


		wstring m_wstrFilePath;

	public:
		int m_nLogBatchCount;
	};

	// Operation record
	class  COperationRecords
	{
	public:
		COperationRecords(){};

		std::wstring    m_strTime;
		std::wstring    m_strOperation;
		std::wstring    m_strUser;
	};
	typedef std::vector<COperationRecords>   VEC_OPT_RECORD;

	// user online record
	class  CUserOnlineEntry
	{
	public:
		CUserOnlineEntry();

		wstring	GetUserName() const;
		void	SetUserName(const wstring& username);


		wstring	GetUserHost() const;
		void	SetUserHost(const wstring& userhost);


		int		GetUserPriority() const;
		void	SetUserPriority(int userpriority);

		wstring	GetUserType() const;
		void	SetUserType(const wstring& usertype);

		SYSTEMTIME GetTimestamp() const;
		void	SetTimestamp(const SYSTEMTIME& stime);

	private:	
		wstring m_username;
		wstring m_userhost;
		int	m_priority;

		wstring m_usertype;		
		SYSTEMTIME m_time;
	};

	class  CCommonDBAccess : public CDatabaseSeesion
	{
	public:
		bool InsertLogicParameters(wstring strThreadName, wstring strLogicName, wstring strSetType, wstring strVariableName, wstring strLinkName, wstring strLinkType, wstring strCondition);
		bool InsertLogicRecord(wstring strLogicName, wstring wstrTime, wstring strPointName, wstring strPointValue);
		bool ClearLogicParameters();
		bool    ReadLogicParameters(wstring &strThreadName, wstring &strLogicName, wstring &strSetType, wstring &strVariableName, wstring &strLinkName, wstring &strLinkType, wstring &strImportTime,wstring &strAuthor, wstring &strPeriod, wstring &strDllContent, wstring &strRunStatus,wstring &strVersion,wstring &strDescription,wstring &strDllOldName, wstring &strNeedSaveDBFile);
		bool    DeleteLogicParameters(wstring strThreadName, wstring strLogicName, wstring strSetType, wstring strVariableName, wstring strLinkName, wstring strLinkType);
		bool	ReadWarningConfig(vector<CWarningConfigItem>& configList);
		wstring ReadOrCreateCoreDebugItemValue(wstring strItemName);

		wstring ReadOrCreateCoreDebugItemValueMul(vector<wstring> strItemNameList, vector<wstring> & strItemValueList);
		bool GetLogicBeginEndStatusErr(CString strDllName, CString &strInfo);
		wstring ReadOrCreateCoreDebugItemValue_Defalut(wstring strItemName,wstring strValue = L"0");
		bool     ReadCoreDebugItemValue(wstring strItemName, wstring &strItemValue);
		bool     ReadCoreDebugItemValue(wstring strItemName, wstring strItemName2,wstring &strItemValue);
		bool    WriteCoreDebugItemValue(const wstring &strItemName, const wstring &strItemValue);
		bool    WriteCoreDebugItemValue(const wstring &strItemName,const wstring &strItemName2, const wstring &strItemValue);
		bool    WriteCoreDebugItemValue(const wstring &strItemName,const wstring &strItemName2, const wstring &strItemValue,const wstring &strItemValue2,const wstring &strItemValue3);
		bool    WriteCoreDebugItemValue2(const wstring &strItemName, const wstring &strItemValue);
		bool    RemoveCoreDebugItemValue(const wstring &strItemName);
		bool	AddWarning(const CWarningEntry& entry);
		bool    ClearCoreDebugItemValueLike(wstring strKeyLike);

		bool	ReadWarning(vector<CWarningEntry>& resultlist, const SYSTEMTIME& start, const SYSTEMTIME& end);
		bool	ReadRecentWarning(vector<CWarningEntry>& resultlist, const int nSecondBefore);
		bool	ReadRecentOperation(vector<WarningOperation>& resultlist, const int nSecondBefore);
		bool	ReadRecentUserOperation(vector<UserOperation>& resultlist, const int nSecondBefore);
		bool	ReadRecentUserOperation(vector<UserOperation>& resultlist, wstring strTime);			//20150519 Observe��Coreʱ�䲻ͬ��

		bool	DownloadFile(wstring strFileID, wstring strFileSavePath);
		bool	UpdateLibInsertIntoFilestorage(const wstring strFilePathName, const wstring strFileName, const wstring strFileId);
		bool	DeleteFile(wstring strFileID);
		bool	DeleteWarning();


		bool	InsertRealTimeDatas_Input(const vector<CRealTimeDataEntry>& entrylist);
		bool	InsertHistoryData(const SYSTEMTIME &stNow, const POINT_STORE_CYCLE &tCycle, const vector<CRealTimeDataEntry>& datalist,int nMaxInsert=10000);
		string  GetHistoryTableNameByStoreCycle(const SYSTEMTIME &stNow, const POINT_STORE_CYCLE &tCycle);
		bool    CheckCreateHistoryTable(const SYSTEMTIME &stNow, const POINT_STORE_CYCLE &tCycle);
		bool    InsertPointBuffer(const vector<CRealTimeDataEntry>& datalist);
		bool    UpdatePointBuffer(const vector<CRealTimeDataEntry>& datalist);

		bool SaveSysDllInfo(wstring &wstrDllName, wstring &wstrDllDescription, wstring &wstrDllAuthor, wstring &wstrVersion, wstring &wstrInputs, wstring &wstrOutputs);


		//��ȡ�������ʷ��¼����ֹ���ݿ�����ʱmemory�㶪ʧ��
		bool	ReadPointBufferData(vector<CRealTimeDataEntry>& resultlist);
		bool	ReadPointBufferData(hash_map<string,CRealTimeDataEntry>& resultmap);		//add by robert
		bool	InsertOperationRecord(const SYSTEMTIME& st, const wstring& remark, const wstring& wstrUser);
		bool	ReadAllOperationRecord(VEC_OPT_RECORD&  vecOperation, const COleDateTime& timeStart, const COleDateTime& timeStop);
		bool	DeleteOperationRecord();

		bool	ReadRealTimeData_Unit01(vector<unitStateInfo>& unitlist);
		bool	InsertUnit01(const vector<unitStateInfo>& unitlist,const string reip);

		double	GetMySQLDBVersion();

		bool	RegisterUserRecord(const CUserOnlineEntry& entry);

		bool	UpdateUserOnlineTime(wstring username, wstring usertype, SYSTEMTIME time);

		bool	ReadUserRecord(vector<CUserOnlineEntry>& resultlist/*, const SYSTEMTIME& start, const SYSTEMTIME& end*/);

		bool	DeleteUserRecord(wstring username, wstring usertype);

		bool	IsUserOnline(wstring username, wstring usertype);

		bool	GetServerTime(SYSTEMTIME &tServerTime);

		bool	GetHistoryValue(const wstring& strName,const SYSTEMTIME &st,const int &nTimeFormat,  wstring &strParam);
		bool	GetHistoryValue(const wstring& strName,const SYSTEMTIME &stStart,const SYSTEMTIME &stEnd,const int &nTimeFormat,  wstring &strParam);
		bool	GetHistoryValueInOneDay(const wstring& strName,const SYSTEMTIME &stStart,const SYSTEMTIME &stEnd,const int &nTimeFormat,  wstring &strParam);

		bool	GetAllHistoryValueByTime(const SYSTEMTIME stStart,const SYSTEMTIME stEnd,vector<Beopdatalink::CRealTimeDataEntry> &entrylist,bool bFileExist = false);		//�����ݿ��л�ȡȫ��ֵ��Ĭ�ϴ�����ӱ����ã�
		// ==========================connnection status =====================================


		bool	InsertErrInfo(const vector<Beopdatalink::ErrInfo> errlist);
		bool	InsertErrCodeInfo(const vector<Beopdatalink::ErrCodeInfo> errlist);
		bool	InsertErrPoints(const vector<Beopdatalink::ErrPointInfo> errlist);

	public:
		bool	SetStatus(const wstring& statustype, int value);
		bool	GetStatus(vector< pair<wstring, int> >& statuslist);

		bool	SetPLCConnectionStatus(bool bconnected);
		bool	GetPLCConnectionStatus();
		bool	GetModbusConnectionStatus();
		bool    InsertS3DBpath(const wstring& s3dbpathname,const wstring& s3dbpathvalue);
		bool	GetS3DBPath(const wstring& s3dbpathname,wstring& s3dbpathvalue);

		bool UpdateSavePoints(const vector<DataPointEntry>& ptList);

		//////////////////////////////////////////////////////////////////////////
		bool	CreateScheduleTableIfNotExist();
		bool	GetScheduleInfo(const string strDate,const string strTimeFrom,const string strTimeTo, const int nDayOfWeek,vector<ScheduleInfo>& vecSchedule);
		bool	DeleteScheduleInfo(const int nID);
		bool	UpdateScheduleInfoExecuteDate(const int nID,const string strDate);
		bool	GetScheduleExist();
		bool    UpdateHistoryStatusTable(string strProcessName);
		
		bool GetEnvContent(int nEnvID, std::vector<wstring> &wstrPointNameList, std::vector<wstring> &wstrPointValueList);

		bool  GetDayCalendarPointSetting(COleDateTime tLastAct, COleDateTime tNow, std::map<wstring, wstring> &wstrPointSettingMap, vector<string> &strModeNameList,vector<int> &nTodayModeList, vector<int> & nTodayModeOfSystemList);
		bool  GetDayCalendarModeList(COleDateTime tNow, vector<int> & nModeIDList, vector<int> & nOfSystemIDList);
		bool  SetDayCalendarMode(COleDateTime tNow, int nModeID, int nOfSystemID);
		bool  GetModeEnvList( vector<int>  nModeIDList, vector<Env> & envList);
		bool GetModeNameList(vector<int>  nModeIDList ,vector<string> &strModeNameList );

		bool  UpdateTriggerTime(COleDateTime tLastCheckTime,  COleDateTime tCheckTime, vector<Env> &envList, vector<Env> &envWillActOneHourList);

		bool UpdateEnvTime(vector<Env> & envList);

		bool ReadRealtimeData_Input( vector<string>& strPointNameList , map<string, string>& strPointValueMap);

		double GetDatabaseVersion();


	private:
		Project::Tools::Mutex	m_lock;

		map<string, string> m_strPointValueMap;
	};

	struct  optrecordfordtu
	{
		string time;		
		string username;	//utf8 format
		string info;	//utf8 format
	};

	class  CDBAccessToDTU : public CDatabaseSeesion
	{
	public:
		CDBAccessToDTU();
		 bool    GetOperationRecordAsString(vector<optrecordfordtu>&  resultlist,
											const COleDateTime& timeStart,
											const COleDateTime& timeStop);
	private:
		Project::Tools::Mutex	m_lock;
	};

	class  DTUDBAccessSingleton : public CDBAccessToDTU
	{
	public:
		static DTUDBAccessSingleton* GetInstance();
		
	private:
		static DTUDBAccessSingleton* _pinstance;
		static void DestroyInstance();

		DTUDBAccessSingleton(const DTUDBAccessSingleton&);
		DTUDBAccessSingleton& operator=(const DTUDBAccessSingleton&);

	private:
		static Project::Tools::Mutex m_lock;

	protected:
		DTUDBAccessSingleton();
		~DTUDBAccessSingleton(){};
	};
}








