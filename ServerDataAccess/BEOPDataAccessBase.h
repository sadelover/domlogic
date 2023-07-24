#pragma once

#include "xstring"
#include "VECTOR"

using namespace std;

class CBEOPDataAccessBase
{
public:
	CBEOPDataAccessBase(void) {};
	virtual ~CBEOPDataAccessBase(void) {};

	virtual bool GetValue(const wstring& strName, bool &bParam) = 0;
	virtual bool GetValue(const wstring& strName, double &dbParam) =0;
	virtual bool GetValue(const wstring& strName, int &uiParam) = 0;
	virtual bool GetValue(const wstring& strName, wstring &bParam) = 0;
	virtual bool SetValue( const wstring& strName, bool bParam ) =0;
	virtual bool SetValue( const wstring& strName, int nParam ) = 0;
	virtual bool SetValue( const wstring& strName, double dParam ) =0;
	virtual bool SetValue( const wstring& strName, wstring nParam ) =0;
	virtual bool    GetPointExist(const wstring & strPointName) =0;
	virtual bool	AddWarning(int nWarningLevel , const wstring &strWarningContent, const wstring &strBindPoint) =0;
	virtual bool    AddOperationRecord(const wstring &strUserName, const wstring & strOperationContent) =0;
	virtual bool GetHistoryValue(const wstring& strName,const SYSTEMTIME &st,const int &nTimeFormat,  wstring &strParam) = 0;
	virtual bool GetHistoryValue(const wstring& strName,const SYSTEMTIME &stStart,const SYSTEMTIME &stEnd,const int &nTimeFormat,  wstring &strParam) = 0;
	virtual bool	InsertLog(const wstring& loginfo) = 0;
	virtual bool	InsertLog(const vector<SYSTEMTIME> & sysTimeList, const vector<wstring> & loginfoList) =0;

	//V2.0
	virtual bool GetValueByKey( const wstring& wstrKeyName, wstring &wstrValue) =0;
	virtual bool SetValueByKey( const wstring& wstrKeyName, const wstring &wstrValue) =0;
	virtual bool RemoveKey( const wstring& wstrKeyName) =0;
	virtual bool GetAllPointRealtimeValues(vector<wstring> &strPointNameList,vector<SYSTEMTIME> &tPointTimeList, vector<wstring> &strPointValueList)=0;

	//V3.0
	virtual bool AddVirtualPointToDBFile(const wstring & wstrPointName, const wstring & wstrPointDescription) = 0;
	virtual wstring GetLocalConfig(const wstring & wstrConfigName) = 0;

	virtual bool	InsertLogOneRecord(SYSTEMTIME sysTime, const wstring &loginfo) =0;

};

