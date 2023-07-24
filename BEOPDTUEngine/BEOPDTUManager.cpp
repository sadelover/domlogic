#include "stdafx.h"
#include "BEOPDTUManager.h"

CBEOPDTUManager::CBEOPDTUManager(CDataHandler * pDataHandler)
	: m_pDataHandler(pDataHandler)
	, m_pBEOPDTUCtrl(NULL)
	, m_hDTUConnect(NULL)
	, m_hDTUSend(NULL)
	, m_hDTUReceive(NULL)
	, m_hMonit(NULL)
	, m_bExitThread(false)
	, m_nTryCount(3)
{
	m_oleHeartTime = COleDateTime::GetCurrentTime() - COleDateTimeSpan(0, 1, 0, 0);
	m_oleSend = COleDateTime::GetCurrentTime();
	m_oleReceive = COleDateTime::GetCurrentTime();
	m_oleConnect = COleDateTime::GetCurrentTime();
}

CBEOPDTUManager::~CBEOPDTUManager()
{
	Exit();
}

/*
��ʼ��������
1����ȡ���ݿ��������
2����������/�����߳�
*/
bool CBEOPDTUManager::Init()
{
	bool bInitSuccess = false;
	if (m_pBEOPDTUCtrl == NULL)
	{
		m_pBEOPDTUCtrl = new CBEOPDTUCtrl(m_pDataHandler);
	}

	if (m_pBEOPDTUCtrl)
	{
		bInitSuccess = m_pBEOPDTUCtrl->Init();

		//�����߳�
		if(m_hDTUConnect == NULL)
			m_hDTUConnect = (HANDLE)_beginthreadex(NULL, 0, ThreadConnectDTU, this, NORMAL_PRIORITY_CLASS, NULL);

		if (m_hDTUSend == NULL)
			m_hDTUSend = (HANDLE)_beginthreadex(NULL, 0, ThreadSendDTU, this, NORMAL_PRIORITY_CLASS, NULL);

		if (m_hDTUReceive == NULL)
			m_hDTUReceive = (HANDLE)_beginthreadex(NULL, 0, ThreadReceiveDTU, this, NORMAL_PRIORITY_CLASS, NULL);

		//����ά���߳�
		if(m_hMonit == NULL)
			m_hMonit = (HANDLE)_beginthreadex(NULL, 0, ThreadMonit, this, NORMAL_PRIORITY_CLASS, NULL);
	}
	return bInitSuccess;
}

bool CBEOPDTUManager::Exit()
{
	if (m_hMonit)
	{
		WaitForSingleObject(m_hMonit, 2000);
		CloseHandle(m_hMonit);
		m_hMonit = NULL;
	}

	//�ر��߳�
	if (m_hDTUConnect)
	{
		WaitForSingleObject(m_hDTUConnect, 2000);
		CloseHandle(m_hDTUConnect);
		m_hDTUConnect = NULL;
	}

	if (m_hDTUSend)
	{
		WaitForSingleObject(m_hDTUSend, 2000);
		CloseHandle(m_hDTUSend);
		m_hDTUSend = NULL;
	}

	if (m_hDTUReceive)
	{
		WaitForSingleObject(m_hDTUReceive, 2000);
		CloseHandle(m_hDTUReceive);
		m_hDTUReceive = NULL;
	}

	//�ͷ���
	if (m_pBEOPDTUCtrl)
	{
		m_pBEOPDTUCtrl->Exit();
		delete m_pBEOPDTUCtrl;
		m_pBEOPDTUCtrl = NULL;
	}
	return true;
}

bool CBEOPDTUManager::GetExitThread()
{
	return m_bExitThread;
}

unsigned int CBEOPDTUManager::ThreadConnectDTU(LPVOID lpVoid)
{
	CBEOPDTUManager* pDTUConnect = reinterpret_cast<CBEOPDTUManager*>(lpVoid);
	if (NULL == pDTUConnect)
	{
		return 0;
	}

	while (!pDTUConnect->GetExitThread())
	{
		pDTUConnect->ConnectDTU();

		int nWaitDTU = 60;			//�ȴ�60��
		while (nWaitDTU >= 0 && !pDTUConnect->GetExitThread())
		{
			nWaitDTU--;
			Sleep(1000);
		}
	}
	return 1;
}

unsigned int CBEOPDTUManager::ThreadSendDTU(LPVOID lpVoid)
{
	CBEOPDTUManager* pDTUSend = reinterpret_cast<CBEOPDTUManager*>(lpVoid);
	if (NULL == pDTUSend)
	{
		return 0;
	}

	while (!pDTUSend->GetExitThread())
	{
		pDTUSend->SendDTU();
		Sleep(1000);
	}
	return 1;
}

unsigned int CBEOPDTUManager::ThreadReceiveDTU(LPVOID lpVoid)
{
	CBEOPDTUManager* pDTUReceive = reinterpret_cast<CBEOPDTUManager*>(lpVoid);
	if (NULL == pDTUReceive)
	{
		return 0;
	}

	while (!pDTUReceive->GetExitThread())
	{
		pDTUReceive->ReceiveDTU();
		Sleep(1000);
	}
	return 1;
}

unsigned int CBEOPDTUManager::ThreadMonit(LPVOID lpVoid)
{
	CBEOPDTUManager* pDTUMonit = reinterpret_cast<CBEOPDTUManager*>(lpVoid);
	if (NULL == pDTUMonit)
	{
		return 0;
	}

	while (!pDTUMonit->GetExitThread())
	{
		pDTUMonit->MonitThread();

		int nWaitDTU = 60;			//�ȴ�60��
		while (nWaitDTU >= 0 && !pDTUMonit->GetExitThread())
		{
			nWaitDTU--;
			Sleep(1000);
		}
	}
	return 1;
}

bool CBEOPDTUManager::ConnectDTU()
{
	m_oleConnect = COleDateTime::GetCurrentTime();
	if (m_pBEOPDTUCtrl)
	{
		return m_pBEOPDTUCtrl->ReConnect();
	}
	return false;
}

bool CBEOPDTUManager::SendDTU()
{
	m_oleSend = COleDateTime::GetCurrentTime();
	if (m_pBEOPDTUCtrl && m_pBEOPDTUCtrl->IsConnected())
	{
		COleDateTimeSpan oleSpan = COleDateTime::GetCurrentTime() - m_oleHeartTime;
		if (oleSpan.GetTotalSeconds() >= 60)
		{
			if (m_pBEOPDTUCtrl->IsConnected())
			{
				m_pBEOPDTUCtrl->SendRegData();
				m_oleHeartTime = COleDateTime::GetCurrentTime();
			}
		}

		//����ѡȡ filename = '' �ҳ��Դ���С��3 ����ʱ���subtype�ۺ�����, �ɹ�ɾ����ʧ�����Լ��� + 1
		if (m_pBEOPDTUCtrl->IsConnected())
		{
			if (m_pDataHandler && m_pDataHandler->IsConnected())
			{
				DTU_DATA_INFO data;
				if (m_pDataHandler->GetLatestString(data, m_nTryCount))
				{
					m_pBEOPDTUCtrl->SendOneData(data);
					Sleep(1000);
				}
			}
		}

		//��η���һ�������ļ�   ѡȡ filename != '' subtupe = 0 ����ʱ������, �ɹ�ɾ����ʧ�����Լ��� + 1
		if (m_pBEOPDTUCtrl->IsConnected())
		{
			if (m_pDataHandler && m_pDataHandler->IsConnected())
			{
				DTU_DATA_INFO data;
				if (m_pDataHandler->GetOneLatestDataFile(data))
				{
					m_pBEOPDTUCtrl->SendOneData(data);
					Sleep(1000);
				}
			}
		}

		//�ٴη���һ�����������ļ����� ѡȡ filename != '' subtupe != 0 �ҳ��Դ���С��3�� ����ʱ������, �ɹ�ɾ����ʧ�����Լ��� + 1
		if (m_pBEOPDTUCtrl->IsConnected())
		{
			if (m_pDataHandler && m_pDataHandler->IsConnected())
			{
				DTU_DATA_INFO data;
				if (m_pDataHandler->GetOneLatestNormalFile(data,m_nTryCount))
				{
					m_pBEOPDTUCtrl->SendOneData(data);
					Sleep(1000);
				}
			}
		}
	}
	return true;
}

bool CBEOPDTUManager::ReceiveDTU()
{
	m_oleReceive = COleDateTime::GetCurrentTime();
	if (m_pBEOPDTUCtrl)
	{
		return m_pBEOPDTUCtrl->HandlerRecCmd();
	}
	return false;
}

bool CBEOPDTUManager::MonitThread()
{
	COleDateTime oleNow = COleDateTime::GetCurrentTime();
	COleDateTimeSpan oleSpanSend = oleNow - m_oleSend;
	if (oleSpanSend.GetTotalMinutes() >= 10)				//�����̳߳�ʱ
	{
		if (m_hDTUSend)
		{
			WaitForSingleObject(m_hDTUSend, 2000);
			CloseHandle(m_hDTUSend);
			m_hDTUSend = NULL;
		}

		if (m_hDTUSend == NULL)
			m_hDTUSend = (HANDLE)_beginthreadex(NULL, 0, ThreadSendDTU, this, NORMAL_PRIORITY_CLASS, NULL);
	}

	oleNow = COleDateTime::GetCurrentTime();
	COleDateTimeSpan oleSpanReceive = oleNow - m_oleReceive;
	if (oleSpanSend.GetTotalMinutes() >= 10)				//�����̳߳�ʱ
	{
		if (m_hDTUReceive)
		{
			WaitForSingleObject(m_hDTUReceive, 2000);
			CloseHandle(m_hDTUReceive);
			m_hDTUReceive = NULL;
		}

		if (m_hDTUReceive == NULL)
			m_hDTUReceive = (HANDLE)_beginthreadex(NULL, 0, ThreadReceiveDTU, this, NORMAL_PRIORITY_CLASS, NULL);
	}

	oleNow = COleDateTime::GetCurrentTime();
	COleDateTimeSpan oleSpanConnect = oleNow - m_oleConnect;
	if (oleSpanSend.GetTotalMinutes() >= 10)				//�����̳߳�ʱ
	{
		if (m_hDTUConnect)
		{
			WaitForSingleObject(m_hDTUConnect, 2000);
			CloseHandle(m_hDTUConnect);
			m_hDTUConnect = NULL;
		}

		if (m_hDTUConnect == NULL)
			m_hDTUConnect = (HANDLE)_beginthreadex(NULL, 0, ThreadConnectDTU, this, NORMAL_PRIORITY_CLASS, NULL);
	}
	return true;
}
