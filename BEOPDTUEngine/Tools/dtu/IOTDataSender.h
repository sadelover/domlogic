#pragma once
#include "Tools/CustomTools.h"
#include <stdio.h>
#include <stdlib.h>
#include "./Tools/CommonDefine.h"
#include "./Tools/db/DataHandler.h"

/*
���ͣ�
�ļ����ݣ�����ѹ��������־�ļ���������־
�ı����ݣ����ñ�,�������ݣ�������������������ʧ�ļ����ݰ�

���գ�
�ļ����ݣ������ļ���ַ����ʧ�ļ���������ļ�
�ı����ݣ����������ͬ��/�޸�unit01,�޸ĵ�ֵ��

�ļ����ݸ�ʽ�������ͣ�
1����250K��ַ���
2����Ϣ���Ƕ����ƶ�ȡ����
3���������ԣ�
type:		0:�ı�����  1:Json�ı� 2:ѹ���ļ���zip��-���ְ�  3��ѹ���ļ���zip��-�ְ�    (�ر�)
cmdtype��	��������																	(�ر�)
size:		���ݴ�С																	(�ر�)
name:		�ļ���
index:		��ǰ��/�ܵİ���

�ı����ݸ�ʽ�������ͣ�
1��������
2����Ϣ���� Json
3���������ԣ�
type:		0:�ı�����  1:Json�ı� 2:ѹ���ļ���zip��-���ְ�  3��ѹ���ļ���zip��-�ְ�  (�ر�)
cmdtype��	��������																(�ر�)
size:		���ݴ�С																	(�ر�)
id:			����ID

Json:�����ͣ�
{
id:����ID		//����Ƿ������������������Ϊ0
state: 0:ʧ�� 1���ɹ�   //������Ƿ��������Ĭ��true
type: int     // 0:����  1��unit����  2����������  3���������� 4:log����
msg:''		//ʧ��ԭ��
data:''
}

data0
{
unitNameList:[]
unitValueList:[]
}
data1
{
warningData:[{},{}]
}
data2:
{
logData:[{},{}]
}

Json:�����գ�
{
id:				//����ID
time:			//����ʱ��
type: int		// 0:����  1��ʵʱ����   2��unit����  3:log����  4��������ʱ�� 5:���� 6:���� 7:����
data:''
}

data1:
{
type:  0:ͬ��ȫ��  1:ͬ��ָ��  1���޸�
pointNameList:[]
pointValueList:[]
}

data2
{
type:  0:ͬ��ȫ��  1:ͬ��ָ��  1���޸�
unitNameList:[]
unitValueList:[]
}

data3
{
timeStart:
timeEnd:
like:		//�ؼ���
}

data4
{
type:  0��ͬ�� :1���޸�
time:''
}

data5
{
type: 0:����Core  1������Logic  2:����Update
}

data6
{
type: 0:����Core  1������Logic  2:����Update 3:���µ��  4������dll
fileurl:''
}

data7
{
dataFileList:[]				�ļ��б�
}

//////////////////////////////////////////////////////////////////////////
���Ͷ���      ��ţ�ʱ�䣬��ע���ļ��������ݣ��ı�ΪJson  �ļ�Ϊ·����ַ��
���ն���      ��ţ�ʱ�䣬���ݣ��ı�ΪJson��  ��ֱ�Ӵ����ļ�

//
*/

typedef LRESULT(*LPRecDataProc)(const unsigned char* pRevData, DWORD length, DWORD userdata);					//�������ݵĻص�����

class CIotDataSender
{
public:
	CIotDataSender(wstring wstrIotConnection=L"", CDataHandler*	pDataHandler=NULL);
	~CIotDataSender();

	bool	Init(LPRecDataProc proc1 = NULL, DWORD userdata = NULL);											//��ʼ������
	bool	Exit();																								//�˳�����
	bool	ReConnect();																						//ʧ���Զ���������
	bool	SendOneData(DTU_DATA_INFO& data);																	//����һ������	
	bool	IsConnected();																						//����״̬  �����������ӵ����ǿ��е�״̬
	bool	IsBusy();																							//æµ״̬
private:
	static UINT WINAPI ActiveThread(LPVOID lpVoid);																//���������߳�
	bool	ActiveIot();

	bool	DisConnect();																						//�ر�����

	//IOT
	bool	IOTConnect(const char* connectionString);															//����Iot����

	//
	bool	SendString(string strContent, DTU_CMD_TYPE nCmdType, string strCmdID,DTU_DATA_INFO& data);			//�����ı�����
	bool	SendFile(string strFilePath, DTU_CMD_TYPE nCmdType, DTU_DATA_INFO& data);							//�����ļ�	
	
	//�ص�����
	static IOTHUBMESSAGE_DISPOSITION_RESULT ReceiveMessageCallback(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback);
	static void SendConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback);
	static void ConnectStatusCallback(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* userContextCallback);			//����״̬�ص�

	//��̬����
	static bool HandlerResult(string strMsgID, IOTHUB_CLIENT_CONFIRMATION_RESULT result);						//��Ϣ���ͷ�������
	static IOTHUBMESSAGE_DISPOSITION_RESULT HandlerReceive(const unsigned char* buffer, int nType, int nCmdType, int nSize, int nCmdID, string strFileName, string strIndex);	//�����յ�������
	static void HandlerText(const unsigned char* buffer, int nType, int nCmdType, int nSize, int nCmdID, string strFileName, string strIndex);	//�����յ����ı�����
	static void HandlerFile(const unsigned char* buffer, int nType, int nCmdType, int nSize, int nCmdID, string strFileName, string strIndex);	//�����յ����ļ�����
	static void HandlerFilePackage(const unsigned char* buffer, int nType, int nCmdType, int nSize, int nCmdID, string strFileName, string strIndex);	//�����յ����ļ��ְ�����

	static void	PrintLog(const wstring &strLog);																//��ӡ��ʾlog
	static bool	ClearFileBuffer();																				//�ͷ��ļ�����

	//��������
	static void	StringReplace(string&s1, const string&s2, const string&s3);											//�滻�ַ���

private:
	//Iot
	wstring						m_wstrIotConnection;		//�����ַ���
	
	static CDataHandler*		m_pDataHandler;				//���ݿ⴦����
	static bool					m_bIotConnectSuccess;		//IoT�ͻ������ӳɹ�
	static hash_map<string, DTU_DATA_INFO> m_mapData;		//�����¼�����
	EVENT_INSTANCE				m_aEvent[IOT_EVENT_COUNT];	// ��Ϣ�¼�����
	int							m_nCurrentEvent;			//��ǰ��Ϣ���
															
	IOTHUB_CLIENT_LL_HANDLE		m_pIotHubClientHandle;		//IoT ���Ŀͻ��˾��
	bool						m_bExitThread;				//�˳��¼�
	int							m_nIOTErrorCount;			//IOT���ʹ���
	HANDLE						m_hActiveHandler;			//�������պͷ���

	COleDateTime				m_oleIOTUpdateTime;			//IOT���ͽ��ո���ʱ��(�ɹ�)
	COleDateTime				m_oleSendTime;				//IOT�����¼�

	static LPRecDataProc		m_lpRecDataProc;			//�յ����ݻص�
	static DWORD				m_dwUserData;				//�û�����

	static	char*				m_cFileBuffer;				//�ļ�����
	static	int					m_nFileBuffer;				//���ļ���С
	static	int					m_nCurrentFileBuffer;		//��ǰ�ļ���С
	static	int					m_nCmdType;					//��������
	static	string				m_strFileName;				//�ļ���
	static  bool				m_bBusy;					//æµ״̬
};