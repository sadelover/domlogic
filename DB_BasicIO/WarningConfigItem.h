#pragma once

#include <string>

#include "DBInterfaceImp.h"

class   CWarningConfigItem
{
public:
	CWarningConfigItem(void);
	~CWarningConfigItem(void);

	double m_fLimitHH;
	double m_fLimitLL;
	double m_fLimitL;
	double m_fLimitH;

	bool m_bEnableHH;
	bool m_bEnableLL;
	bool m_bEnableH;
	bool m_bEnableL;

	std::wstring m_strPointName;
	std::wstring m_strWarningContent_HH;
	std::wstring m_strWarningContent_H;
	std::wstring m_strWarningContent_L;
	std::wstring m_strWarningContent_LL;

	//������bool�㱨��������Ϣ
	int m_nWarningType; //�Ƿ���bool������
	std::wstring m_strWarningContent;
	int m_nWarningLevel;
};

