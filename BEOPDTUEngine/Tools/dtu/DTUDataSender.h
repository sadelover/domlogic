#pragma once
#include "Tools/CustomTools.h"

class CDTUtDataSender
{
public:
	CDTUtDataSender(UINT uPort = 1, UINT uBaud = 19200, char cParity = 'N', UINT uDatabits = 8, UINT uStopbits = 1, DWORD dwCommEvents = EV_RXCHAR, UINT uWritebuffersize = 1024);
	~CDTUtDataSender();

	bool	Init();									//��ʼ������
	bool	Exit();									//�˳�����
	void	PrintLog(const wstring &strLog);		//��ӡ��ʾlog

private:

};
