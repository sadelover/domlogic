#include "StdAfx.h"
#include "WarningEntry.h"



CWarningEntry::CWarningEntry()
{
	GetLocalTime(&m_time);
	m_warningcode = 0;
	m_warninglevel = 0;
	m_timeEnd = m_time;
	m_nConfirmedType = 0;
}

SYSTEMTIME CWarningEntry::GetTimestamp() const
{
	return m_time;
}

void CWarningEntry::SetTimestamp( const SYSTEMTIME& stime )
{
	m_time = stime;
}

wstring CWarningEntry::GetWarningInfo() const
{
	return m_warninginfo;
}

void CWarningEntry::SetWarningInfo( const wstring& warninginfo )
{
	m_warninginfo = warninginfo;
}


wstring CWarningEntry::GetWarningPointName() const
{
	return m_strBindPointName;
}

void CWarningEntry::SetWarningPointName( const wstring& strName )
{
	m_strBindPointName = strName;
}


int CWarningEntry::GetWarningCode() const
{
	return m_warningcode;
}

void CWarningEntry::SetWarningCode( int warningcode )
{
	m_warningcode = warningcode;
}



void	CWarningEntry::SetEndTimestamp( const SYSTEMTIME& stime )
{
	m_timeEnd = stime;
}


SYSTEMTIME	CWarningEntry::GetEndTimestamp() const
{
	return m_timeEnd;
}

int CWarningEntry::GetWarningLevel() const
{
	return m_warninglevel;
}

void CWarningEntry::SetWarningLevel( int warninglevel )
{
	m_warninglevel = warninglevel;
}

CWarningEntry::~CWarningEntry(void)
{
}

int CWarningEntry::GetWarningConfirmedType() const
{
	return m_nConfirmedType;
}

void CWarningEntry::SetWarningConfirmedType( int nConfirmedType )
{
	m_nConfirmedType = nConfirmedType;
}
