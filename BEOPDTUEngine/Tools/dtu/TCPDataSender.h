#pragma once
#include "Tools/CustomTools.h"
#include "Tools/CommonDefine.h"
#include "mswsock.h"
#include <process.h>
#include <string>
#include <vector>
#include <queue>

using namespace std;
using std::vector;

#define MAX_RECEIVED_BUFFER_SIZE  2048		//ÿ�ν������ݴ�С

typedef LRESULT(*LPRecDataProc)(const unsigned char* pRevData, DWORD length, DWORD userdata);					//�������ݵĻص�����

class CTCPDataSender
{
public:
	CTCPDataSender(wstring wstrTCPIP=L"beopdata.rnbtech.com.hk", wstring wstrTCPPort=L"9500", wstring wstrTCPName=L"", wstring wstrTCPSendFilePackageSize=L"1000", wstring wstrTCPSendFilePackageInterval=L"2000", wstring wstrTCPAES256=L"0");
	~CTCPDataSender();

	bool	Init(LPRecDataProc proc1 = NULL, DWORD userdata = NULL);											//��ʼ������
	bool	Exit();																								//�˳�����
	bool	ReConnect();																						//ʧ���Զ���������
	bool	SendOneData(DTU_DATA_INFO& data);																	//����һ������	
	bool	SendString(string strContent);																		//�����ı�����
	bool	IsConnected();																						//����״̬

private:
	//TCP����
	bool	TCPConnect(const string& strHost, u_short uPort);													//TCP����
	bool	DisConnect();																						//�ر�����
	bool	SendFile(string strFilePath);																		//�����ļ�
	bool	SendRealDataFile(string strFilePath, int nPointCount);												//����ʵʱ�����ļ�
	bool	SendHistoryDataFile(string strFilePath, int nPointCount);											//������ʷ��ʧ�ļ�
	bool	GenerateLog(DTU_CMD_TYPE type, string strContent,bool bSendSuccess,string strRemark="");			//���log

	bool	TCPSendPackageByWrap(char *pData, int nLen);														//�Զ���ӻ��з�
	bool	TCPSendPackage(const char *pSrcPackage, unsigned int nSrcPackSize);									// send package	
	SOCKET  GetTCPConnectedSocket();

	bool	HandleRecData(const char* pSrcBuffer, int nSize);													//������յ�������
	void	PrintLog(const wstring &strLog);																	//��ӡ��ʾlog
	void	SplitStringSpecial(const char* buffer, int nSize, std::vector<TCP_DATA_PACKAGE>& resultlist);		//����;\n����
	bool	OnRecvData(const unsigned char* pRevData, DWORD dwLength);											//
	bool	StoreRecvData(TCP_DATA_PACKAGE& data);																//�洢���յ�������
	void	SplitString(const std::string& source, const char* sep, std::vector<string>& resultlist);
	unsigned int BufferToInt(const unsigned char* buffer);

	bool	AnalyzeRecvData();																					//��������
	bool	AnalyzeReceive(const unsigned char* pRevData, DWORD length);										//�����յ�������
	void	AnzlyzeString(char* pBuffer);																		//һ������
	void	AnalyzeUpdateFile(const unsigned char* buffer);														//���³���
	void	AnalyzeRecLogFile(const unsigned char* buffer);														//��ʧ�ļ�ѹ����
	void	GenerateUpdateFile();
	void	GenerateLostFile();																					//
	bool	CreateOrDeleteUpdateFileBuffer(int nSize = 1024, bool bDeelete = false);
	void	ReleaseUpdateBufferSize(int nTimeOut = 300);
	void	StringReplace(string&s1, const string&s2, const string&s3);											//�滻�ַ���

	static UINT WINAPI RecvThread(LPVOID lpVoid);																//���������߳�
	static UINT WINAPI AnalyzeRecvCmd(LPVOID lpVoid);															//�������������߳�
	static UINT WINAPI DTUThreadReleaseUpdateSize(LPVOID lparam);												//�����ͷŸ��»����߳�

private:
	bool			m_bTCPConnectSuccess;			//TCP����״̬
	bool			m_bAES256;						//ʹ��AES256����

	//TCP
	wstring			m_wstrTCPIP;					//TcpSenderIP TCP��IP������		114.215.172.232  beopdata.rnbtech.com.hk
	wstring			m_wstrTCPPort;					//TcpSenderPort TCP�˿�			9500
	wstring			m_wstrTCPName;					//TcpSenderName TCP��ʶ��11λ��
	wstring			m_wstrTCPSendFilePackageSize;	//TCPSendFilePackageSize TCP����С	1000
	wstring			m_wstrTCPSendFilePackageInterval;//TCPSendFilePackageInterval TCP����� 2000
	wstring			m_wstrTcpSenderEnabled;			//TcpSenderEnabled		����

	SOCKET          m_sockTCP;      	 			//TCP�ͻ���Socket
	HANDLE			m_hTCPReceiveHandler;			//TCP�����߳̾��
	bool			m_bExitThread;					//TCP�˳��߳�
	int				m_nTCPErrorCount;				//TCP���ʹ���
	COleDateTime	m_oleTCPUpdateTime;				//TCP���ͽ��ո���ʱ��
	Project::Tools::Mutex	m_lockTCP;				//TCP������

	HANDLE			m_hReleaseBufferHandler;		//�ͷŻ����ڴ���
	HANDLE			m_hAnalyzeHandler;				//���������߳̾��
	char			m_cSpiltBuffer[g_nRecMaxNum];
	char			m_cRecvBuf[g_nRecMaxNum];		//���ݽ��ջ���
	int				m_nRecSize;						//��ǰ���ݽ��ջ����С
	char			m_cAnzlyzeBuff[g_nRecMaxNum];	//���ݽ�������
	int				m_nAnzlyzeCount;				//��ǰ���ݽ��������С
	char			*m_szWriteBuffer;				//����������
	int				m_nWriteSize;					//�����ʹ�С
	int				m_nDTUFilePackageNumber;		//�ļ������

	LPRecDataProc	m_lpRecDataProc;				//�յ����ݻص�
	DWORD			m_dwUserData;					//�û�����

	CRITICAL_SECTION m_csDTUDataSync;				//�ٽ������� ����������Դ
	queue<TCP_DATA_PACKAGE>	m_queCmdPaket;			//�洢�����������

	Project::Tools::Mutex	m_lockReveive;			//�����߳���
	Project::Tools::Mutex	m_lockReceiveBuffer;	//���ջ�����

	hash_map<int, int>	m_mapFileUpdateState;		//�ļ��հ����
	string			m_strCoreFileName;				//Core�ļ�����
	int				m_nCoreFileCount;				//Core�ļ�������
	int				m_nRecCoreFileCount;			//�յ���Core�ļ�������
	int				m_nLastUpdateExeSize;			//���һ�����Ĵ�С
	int				m_nUpdateFileSize;				//���¶��д�С
	char**			m_cFileUpdateBuffer;			//���»���
	COleDateTime	m_oleUpdateFileTime;				//�����ļ����ʹ��ʱ�䣬����10��������ڴ�
};
