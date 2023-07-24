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
dtunam:		dtuname
type:		0:�ı�����  1:Json�ı� 2:ѹ���ļ���zip��-���ְ�  3��ѹ���ļ���zip��-�ְ�    (�ر�)
cmdtype��	��������																	(�ر�)
cmdid:		������																	(�ر�)
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

class CRabbitMQSender
{
public:
	CRabbitMQSender(string strRabbitMQHost="", string strRabbitMQUser = "", string strRabbitMQPsw = "", string strRabbitMQSend = "", string strRabbitMQReceive = "", bool bRabbitMQSSL=false, CDataHandler*	pDataHandler=NULL);
	~CRabbitMQSender();

	bool	Init(LPRecDataProc proc1 = NULL, DWORD userdata = NULL);											//��ʼ������
	bool	Exit();																								//�˳�����
	bool	ReConnect();																						//ʧ���Զ���������
	bool	SendOneData(DTU_DATA_INFO& data);																	//����һ������	
	bool	IsConnected();																						//����״̬  �����������ӵ����ǿ��е�״̬
	bool	IsBusy();																							//æµ״̬
	bool	SetConnectStatus(bool bConnectStatus);

private:
	bool	DisConnect();																						//�ر�����
	bool	RabbitMQConnect();																					//����Rabbitmq����

																												//
	bool	SendString(string strContent, DTU_CMD_TYPE nCmdType, string strRemark, DTU_DATA_INFO& data, string strCmdID ="");			//�����ı�����
	bool	SendFile(string strFilePath, DTU_CMD_TYPE nCmdType, DTU_DATA_INFO& data,string strCmdID ="");							//�����ļ�	

	static UINT WINAPI RecvThread(LPVOID lpVoid);																//���������߳�
	bool	RecvRabbitMQ();
	void	HandlerReceive(amqp_message_t message, string strType, string strCmdType, string strSize, string strCmdID, string strFileName, string strIndex);	//�����յ�������
	void	HandlerText(amqp_message_t message, string strType, string strCmdType, string strSize, string strCmdID, string strFileName, string strIndex);	//�����յ����ı�����
	void	HandlerFile(amqp_message_t message, string strType, string strCmdType, string strSize, string strCmdID, string strFileName, string strIndex);	//�����յ����ļ�����
	void	HandlerFilePackage(amqp_message_t message, string strType, string strCmdType, string strSize, string strCmdID, string strFileName, string strIndex);	//�����յ����ļ��ְ�����

	//
	bool	ClearFileBuffer();																					//�ͷ��ļ�����
	void	PrintLog(const wstring &strLog);																	//��ӡ��ʾlog
	void	StringReplace(string&s1, const string&s2, const string&s3);											//�滻�ַ���
	bool	GenerateLog(DTU_CMD_TYPE type, string strContent, bool bSendSuccess, string strRemark = "");			//���log
	//////////////////////////////////////////////////////////////////////////
	void	die(const char *fmt, ...);
	bool	die_on_error(int x, char const *context);
	bool	die_on_amqp_error(amqp_rpc_reply_t x, char const *context);
	void	amqp_dump(void const *buffer, size_t len);
	void	dump_row(long count, int numinrow, int *chs);
	int		rows_eq(int *a, int *b);

private:
	amqp_socket_t*				m_pSocketRabbitMQ;
	amqp_connection_state_t		m_conRabbitMQ;
	bool						m_bConnectSuccess;

	int							m_nRabbitMQStatus;
	int							m_nRabbitMQPort;
	int							m_nConnectTimeOut;
	string						m_strRabbitMQHost;
	string						m_strRabbitMQUser;
	string						m_strRabbitMQPsw;
	string						m_strRabbitMQSend;
	string						m_strRabbitMQReveive;
	string						m_strRabbitDirect;
	string						m_strRabbitDirectType;
	string						m_strRabbitBindKey;				//·��key
	int							m_nRabbitHeartbeat;				//����ʱ��
	int							m_nRabbitDurable;				//�־û�
	int							m_nRabbitNoAck;					//�Զ�ȷ��

	HANDLE						m_hRabbitReceiveHandler;		//Rabbit�����߳̾��
	bool						m_bExitThread;					//TRabbit�˳��߳�
	LPRecDataProc				m_lpRecDataProc;				//�յ����ݻص�
	DWORD						m_dwUserData;					//�û�����
	CDataHandler*				m_pDataHandler;					//���ݿ⴦����
	COleDateTime				m_oleRabbitMQUpdateTime;		//Rabbit���ͽ��ո���ʱ��(�ɹ�)
	COleDateTime				m_oleSendTime;					//Rabbit�����¼�
	int							m_nRabbitMQErrorCount;			//RabbitMQ���ʹ���

	char*						m_cFileBuffer;					//�ļ�����
	int							m_nFileBuffer;					//���ļ���С
	int							m_nCurrentFileBuffer;			//��ǰ�ļ���С
	string						m_strFileName;					//�ļ���
	bool						m_bBusy;						//æµ״̬

	int							m_nChannelSend;					//����ͨ��
	int							m_nChannelReceive;				//����ͨ��
};