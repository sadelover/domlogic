#pragma once

#include "wtypes.h"
#include "XSTRING"

using namespace std;

struct  WarningOperation
{
	wstring			strTime;
	wstring			strWarningInfo;	
	wstring			strBindPointName;
	wstring			strOpeartion;	
	wstring			strUser;
};

struct  UserOperation
{
	wstring			strTime;
	wstring			strOpeartion;	
	wstring			strUser;
};

class  CWarningEntry
{
public:
	~CWarningEntry(void);

public:
	CWarningEntry();
	SYSTEMTIME GetTimestamp() const;
	void	SetTimestamp(const SYSTEMTIME& stime);

	wstring	GetWarningInfo() const;
	void	SetWarningInfo(const wstring& warninginfo);

	wstring	GetWarningPointName() const;
	void	SetWarningPointName(const wstring& warninginfo);

	int	GetWarningCode() const;
	void	SetWarningCode(int warningcode);

	int	GetWarningLevel() const;
	void	SetWarningLevel(int warninglevel);

	SYSTEMTIME	GetEndTimestamp() const;
	void	SetEndTimestamp(const SYSTEMTIME& stime);

	int		GetWarningConfirmedType() const;
	void	SetWarningConfirmedType(int nConfirmedType);
private:
	SYSTEMTIME m_time;
	int	m_warningcode;
	wstring m_warninginfo;
	int m_warninglevel;
	int	m_nConfirmedType;

	SYSTEMTIME m_timeEnd;

	wstring m_strBindPointName;
};

