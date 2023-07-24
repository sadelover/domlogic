#pragma once

#include "BEOPGatewaycore.h"
#include "afxsock.h"


#define BLOCKSIZE 1024*256         //��������С

class CBEOPDataAccess;
class CCoreFileTransfer
{
public:
	CCoreFileTransfer(CBEOPDataAccess *pAccess, int nPort);
	~CCoreFileTransfer(void);

	void PrintLog(const wstring &strLog);
	bool ReceiveFile(CSocket &recSo);//�ļ����մ���
	void SendFile(CSocket &senSo);//�ļ����ʹ���

	void StartAsSender(CString strIPConnectTo, int nPortConnectTo);
	void StartAsReceiver();
	bool Exit();

	bool TestDBFile(wstring strFilePath);

	static DWORD WINAPI _ListenThread(LPVOID lpParameter);
	static DWORD WINAPI _SendThread(LPVOID lpParameter );

	BOOL m_bIsTransmitting;
	BOOL m_bIsStop;
	CString m_strPath;

	HANDLE m_hThreadListen;

	bool m_bExitThread;


	int m_nPort;
	bool m_bRecvS3DB;

	//sender

	CString m_strIPConnectTo;
	int m_nPortConnectTo;
	bool m_bNewFileRecved;
	bool m_bFileSent;
	HANDLE m_hThreadSend;

	CBEOPDataAccess *m_pDataAccess;
};

