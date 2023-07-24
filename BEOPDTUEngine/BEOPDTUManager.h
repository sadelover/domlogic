#pragma once
#include "Tools/dtu/BEOPDTUCtrl.h"
class CBEOPDTUManager
{
public:
	CBEOPDTUManager(CDataHandler* pDataHandler);
	~CBEOPDTUManager();

	bool	Init();									//��ʼ������
	bool	Exit();									//�˳�����
	bool	GetExitThread();						//��ȡ�˳����

	static  unsigned int WINAPI ThreadConnectDTU(LPVOID lpVoid);	//DTU�����߳�
	static  unsigned int WINAPI ThreadSendDTU(LPVOID lpVoid);		//DTU�����߳�
	static  unsigned int WINAPI ThreadReceiveDTU(LPVOID lpVoid);	//DTU���մ����߳�
	static  unsigned int WINAPI ThreadMonit(LPVOID lpVoid);			//�ػ����Ӵ����߳�

	bool	ConnectDTU();							//����DTU
	bool	SendDTU();								//���ͺ���
	bool	ReceiveDTU();							//���պ���
	bool	MonitThread();							//�����߳�

private:
	CDataHandler*	m_pDataHandler;					//���ݿ⴦����
	CBEOPDTUCtrl*	m_pBEOPDTUCtrl;					//DTU������
	HANDLE			m_hDTUConnect;					//DTU�����߳̾��
	HANDLE			m_hDTUSend;						//DTU�����߳̾��
	HANDLE			m_hDTUReceive;					//DTU�����߳̾��
	HANDLE			m_hMonit;						//�ػ��̣߳�����,���գ����ӣ�
	bool			m_bExitThread;					//�˳��̱߳��
	int				m_nTryCount;					//���Դ���
	COleDateTime	m_oleHeartTime;					//������ע��ʱ��

	COleDateTime	m_oleSend;						//����ʱ��
	COleDateTime	m_oleReceive;					//����ʱ��
	COleDateTime	m_oleConnect;					//����ʱ��
};
