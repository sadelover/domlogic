#include "StdAfx.h"
#include "RedundencyManager.h"
#include "../LAN_WANComm/NetworkComm.h"
#include "../DataEngineConfig/CDataEngineAppConfig.h"



#define  E_NUM_UPDATE_INTERVAL 90  //���һ�θ�����1�������ڼ�ʹ�û�ԭ
#define  E_STR_BACKUPFILENAME  _T("\\backup.bat")   //�����ļ���
#define  E_STR_RESTOREFILENAME  _T("\\restore.bat")   //��ԭ�ļ���
#define  E_STR_MYSQL_PATH	_T("\"C:\\Program Files\\MySQL\\MySQL Server 5.1\\bin\\\"")     
#define  E_STR_SQLPATH	_T("d:\RealTimeDataBase.sql")


CRedundencyManager::CRedundencyManager(void)
	: m_Backupthreadhandle(NULL)
	,m_nMainDropInterval(1000)
	,m_nActiveDelay(60)
{

	m_hEventActiveStandby = CreateEvent(NULL, TRUE, FALSE, NULL);
}


CRedundencyManager::~CRedundencyManager(void)
{
	if(m_Backupthreadhandle != NULL)
	{
		CloseHandle(m_Backupthreadhandle);
		m_Backupthreadhandle = NULL;
	}

}



UINT WINAPI CRedundencyManager::CheckIfNeedActiveSlaveThread( LPVOID lpVoid )
{
	CRedundencyManager* pManager = reinterpret_cast<CRedundencyManager*>(lpVoid);
	if (NULL == pManager)
	{
		return 0;
	}

	//�ж�����30s�յ�����״̬
	int nTotal = pManager->m_nActiveDelay;
	while(1)
	{
		// Analyze Data
		bool bActive = pManager->CheckIfNeedActiveSlave();
		if(bActive)
		{
			nTotal--;
			if(nTotal < 0)
			{
				pManager->ActiveSlaveEvent();
				nTotal = pManager->m_nActiveDelay;
				break;
			}
		}
		Sleep(1000);
	}

	return 1;
}


bool CRedundencyManager::WaitForActiveCommand(int nActiveDelay)
{
	//����һ���̼߳��
	HANDLE threadhandle = (HANDLE)_beginthreadex(NULL, 0, CheckIfNeedActiveSlaveThread, this, 0, NULL);

	
	//	pDlg->SetWindowTextW(L"��PC������������ģʽ��BEOPgateway������תΪ�����ȱ�״̬������ر�.");
	if(nActiveDelay <= 0)
		nActiveDelay = 60;

	m_nActiveDelay = nActiveDelay;
	DWORD waitstatus = WaitForSingleObject(m_hEventActiveStandby, INFINITE);
	if (waitstatus == WAIT_OBJECT_0)
	{
		CloseHandle(threadhandle);
		threadhandle = NULL;

		return true;
	}

	return false;
}


bool CRedundencyManager::CheckIfNeedActiveSlave()
{
	return CNetworkComm::GetInstance()->IsMainServerRunningMode();
}


bool CRedundencyManager::ActiveSlaveEvent()
{
	SetEvent(m_hEventActiveStandby);
	return true;
}

bool CRedundencyManager::Init(int nMainDropInterval, CString strIP, CString strIP2)
{

	m_strIP = strIP;
	m_strIP2 = strIP2;
	//���ò���
	CDataEngineAppConfig::GetInstance()->SetGateWayIP(strIP.GetString());
	CDataEngineAppConfig::GetInstance()->SetOtherServerIP(strIP2.GetString());


	//����һ������������  StartUdpComm
	//CNetworkComm::GetInstance()->SetUpperMachineMode(DOUBLE_MODE);
	//CNetworkComm::GetInstance()->StartUdpComm();
	CNetworkComm::GetInstance()->SetMachineModes(true,true,false);
	return true;
}

void CRedundencyManager::InitWatchDog( int nInterval )
{
	//_asm
	//{
	//	//��������ģʽ
	//	mov dx,0x4E
	//	mov al,055h
	//	out dx,al
	//	out dx,al

	//	//
	//	mov dx,0x4E
	//	mov al,7h
	//	out dx,al
	//	inc dx
	//	mov al,oah
	//	out dx,al

	//	//��ȡ����ʱ�Ĵ�������ַ
	//	mov dx,0x4E
	//	mov al,60h
	//	out dx,al

	//	inc dx
	//	in al,dx
	//	mov bh,al

	//	mov dx,0x4E
	//	mov al,61h
	//	out dx,al

	//	inc dx
	//	in al,dx
	//	mov bl,al

	//	//����GP60Ϊ���Ź���ʱ��
	//}
}