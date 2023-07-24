#pragma once
#include "Tools/db/DataHandler.h"
#include "Tools/dtu/TCPDataSender.h"
#include "Tools/dtu/DTUDataSender.h"
//#include "Tools/dtu/IOTDataSender.h"
#include "Tools/dtu/RabbitMQSender.h"
#include "Tools/ProcessView.h"
#include "Tools/ExcelOperator.h"
#include "Tools/LogicBase.h"

typedef CLogicBase* (*pfnLogicBase)();

class CBEOPDTUCtrl
{
public:
	CBEOPDTUCtrl(CDataHandler* pDataHandler);
	~CBEOPDTUCtrl();

	bool	Init();									//��ʼ������
	bool	Exit();									//�˳�����
	void	PrintLog(const wstring &strLog);		//��ӡ��ʾlog

	bool	IsConnected();							//���ӳɹ�

	static LRESULT OnRecData(const unsigned char* pRevData, DWORD dwLength, DWORD dwUserData);
	bool	StoreDTUCmd(DTU_RECEIVE_INFO& data);

	//���ͺ���
	bool	SendOneData(DTU_DATA_INFO& data);		//����һ������	

	//����ע�ắ��
	bool	SendRegData();							//����DTUע��

	//�����յ�������
	bool	HandlerRecCmd();						//�����յ�������

	//���Ӵ�����
	bool	ReConnect();							//ʧ���Զ���������

private:
	bool	HandlerRecData(DTU_RECEIVE_INFO& data);	//�����յ�������
	bool	HandlerString(DTU_RECEIVE_INFO& data);	//�����ı�
	bool	HandlerFile(DTU_RECEIVE_INFO& data);	//�����ļ�
	bool	HandlerLostFile(DTU_RECEIVE_INFO& data);//����ʧ�ļ�
	bool	HandlerUpdateCmd(DTU_UPDATE_INFO& data);//����һ�����µ�����

	bool	HandlerSynRealData(POINT_STORE_CYCLE nType,string strCmdID="");											//ͬ��ĳ����������
	bool	HandlerSynUnit01(string strCmdID="");																	//ͬ��Unit01
	bool	HandlerReadLostFileAndSendHistory(string strPath,string strCmdID="");									//�����ļ��ط���
	bool	HandlerChangeValues(string strReceive,string strCmdID="");												//�����޸ĵ�
	bool	HandlerAckHeartBeat(string strCmdID="");																//����������
	bool	HandlerAckReSendFileData(string strFileName,string strCmdID="");										//�ط���ʧʵʱ�ļ���
	bool	HandlerAckServerState(string strCmdID="");																//����������
	bool	HandlerChangeUnits(string strReceive,string strCmdID="");												//��������Unit01
	bool	HandlerSendHistoryDataFile(string strStart, string strEnd,string strCmdID="");							//��ʷ����
	bool	HandlerChangeUnit(string strParamName, string strValue,string strCmdID="");								//����Unit01
	bool	HandlerDeleteMutilPoint(string strPoints,string strCmdID="");											//ɾ����
	bool	HandlerUpdateRestart(string strCmdID="");																//����Core
	bool	HandlerRestartDTUEngine(string strCmdID="");															//����DTUEngine
	bool	HandlerAckCoreVersion(string strCmdID="");																//Core�汾��
	bool	HandlerAckLocalPoint(string strCmdID="");																//ͬ���ֳ����
	bool	HandlerAckErrFileList(string strCmdID="");																//��Ӧ�����ļ��б�
	bool	HandlerAckCoreErrCode(string nMintue,string strCmdID="");												//Core�������
	bool	HandlerAckErrFileByName(string strErrName,string strCmdID="");											//��Ӧ�����ļ��б�
	bool	HandlerAckReSendLostZipData(string strMinutes, string strFileNames,string strCmdID="");					//��Ӧ��ʧ�����ڼ�����
	bool	HandlerAckReSendLostData(string strMinutes, string strPacketIndex,string strCmdID="");					//��Ӧ��ʧ�����ڼ�����
	bool	HandlerUpdateMutilPoint(string strPoints,string strCmdID="");											//��/�޵��
	bool	HandlerUpdateMutilPoint_(string strPropertyCount, string strPoints,string strCmdID="");					//��/�޵��
	bool	HandlerMysqlFileByTableName(string strTableName,string strCmdID="");									//��Ӧ���ݿ��ļ�
	bool	HandlerAckSynSystemTime(string strSystemTime,string strCmdID="");										//��Ӧ������ͬ��ϵͳʱ��
	bool	HandlerAckReStartDTU(string strCmdID="");																//��Ӧ����DTU
	bool	HandlerRestartLogic(string strCmdID="");																//����Logic
	bool	HandlerChangeValueByDTUServer(string strPointName, string strValue,string strCmdID="");					//�޸ĵ���

	//////////////////////////////////////////////////////////////////////////
	bool	UpdateDll(DTU_UPDATE_INFO& data,string& strResultString);												//���²���
	bool	UpdateS3db(DTU_UPDATE_INFO& data,string& strResultString);												//����S3db
	bool	UpdateCore(DTU_UPDATE_INFO& data,string& strResultString);												//����Core
	bool	UpdateDTU(DTU_UPDATE_INFO& data,string& strResultString);												//����DTU
	bool	UpdateLogic(DTU_UPDATE_INFO& data,string& strResultString);												//����Logic
	bool	UpdateBat(DTU_UPDATE_INFO& data,string& strResultString);												//ִ���������ļ�
	bool	UpdatePointExcel(DTU_UPDATE_INFO& data,string& strResultString);										//���µ��Excel�ļ�
	bool	UpdatePointCSV(DTU_UPDATE_INFO& data,string& strResultString);											//���µ��CSV�ļ�

	bool	UpdatePointXls(CString strExcelPath,CString strS3dbPath,int& nErrCode);
	bool	UpdatePointCSV(CString strCSVPath,CString strS3dbPath,int& nErrCode);
	bool	UpdateDll(int nUpdateSate,CString strDllName,CString strThreadName,CString strS3dbPath);
	bool	StoreDllToDB(CString strDllPath,CString strS3dbPath,CString strDllName,CString strThreadName,CString strVersion,CString strDescription,CString strAuthor);
	bool	DeleteDll(CString strS3dbPath,CString strDllName,CString strThreadName);
	bool	UpdateDllThread(CString strS3dbPath,CString strDllName,CString strThreadName);
	bool	UpdateDllParamter(CString strS3dbPath,wstring strParamSql);
	bool	DeleteDllParamter(CString strS3dbPath,wstring strParamSql);
	bool	UpdateThreadState(wstring strS3dbPath,wstring strThreadName,wstring strState,int nPeriod=-1);
	bool	UpdateThreadState(wstring strS3dbPath,wstring strState,wstring strThreadName,wstring strValue);

	bool	FindFile(CString strFolder,CString strExten,vector<CString>& vecFileName);		//��ָ��Ŀ¼���Һ�׺��ΪXX���ļ�
	bool	FindFile(CString strFolder,CString strName);
	//////////////////////////////////////////////////////////////////////////

	string	SendCmdState(const string strSendTime, int nCmdType, int nCmdState, string strPacketIndex);	//�����
	bool	CheckDiskFreeSpace(string strDisk, string& strDiskInfo);								//��ȡ���̿ռ�
	int		GetMemoryPervent();																		//��ȡ�ڴ�
	int		GetCPUUseage();																			//��ȡCPU
	double	FileTimeToDouble(FILETIME &filetime);
	bool	ReadDataPoint(string strS3dbPath, vector<DataPointEntry> &vecPoint);					//��ȡ���
	bool	GeneratePointCSV(vector<DataPointEntry> &vecPoint, wstring strFolder, CString& strCSVName, wstring & strCSVPath);
	bool	GeneratePointFromInfo(vector<vector<wstring>> vecPoints, vector<DataPointEntry>& vecPoint);
	string	GetFileInfoPointCont(string strPath, string strFileName);
	bool	UpdateMutilPoint(vector<DataPointEntry>& vecPoint);
	int		GetColumnIndex(std::map<std::string, int> map, string strColumnName);
	bool	DelteFoder(wstring strDir);																//ɾ���ļ���
	bool	ImproveProcPriv();																		//����Ȩ��	
	bool	SetSystemTimeByBat(SYSTEMTIME st);														//���÷�����ʱ��
	bool	RestartLogic(wstring strPath);															//����Logic

	bool	CloseExe(CString strExeName);															//�رճ���
	bool	OpenExe(CString strExePath);															//��������
	bool	RestartExe(CString strExeName,CString strFolder);										//��������
	bool	OutPutUpdateLog(CString strLog);
	void	SplitStringSpecial(const char* buffer,int nSize, std::vector<string>& resultlist);		//����),����
private:
	CDataHandler*	m_pDataHandler;					//���ݿ⴦����
	CDTUtDataSender* m_pDTUtDataSender;				//DTU������
	CTCPDataSender*	m_pTCPDataSender;				//TCP������
	//CIotDataSender*	m_pIotDataSender;				//Iot������
	CRabbitMQSender* m_pRabbitMQSender;				//RabbitMQ������

	bool			m_bDTUConnectSuccess;			//DTU����״̬
	wstring			m_wstrDTUType;					//DTUType 0:����  1:2gDTU(UDP+DDP)  2:2gDTU(TCP)  3:TCP  4:Iot
	wstring			m_wstrDTUMinType;				//DTUMinType	DTU��С�������ͼ�� 2:1���ӵ�  3:5���� Ĭ��Ϊ3
	wstring			m_wstrDTUDisableSendAll;		//DTUDisableSendAll �Ƿ������㷢��
	wstring			m_wstrDTUSendAll;				//sendall			����ȫ������

	//���ڲ���
	wstring			m_wstrDTUEnabled;				//DTUEnabled DTU����
	wstring			m_wstrDTUComPort;				//DTUComPort DTUCom��   Ĭ��Ϊ����1
	wstring			m_wstrDTUChecked;				//DTUChecked DTU���    Ĭ��0

	//TCP
	wstring			m_wstrTCPIP;					//TcpSenderIP TCP��IP������		114.215.172.232  beopdata.rnbtech.com.hk
	wstring			m_wstrTCPPort;					//TcpSenderPort TCP�˿�			9500
	wstring			m_wstrTCPName;					//TcpSenderName TCP��ʶ��11λ��
	wstring			m_wstrTCPSendFilePackageSize;	//TCPSendFilePackageSize TCP����С	1000
	wstring			m_wstrTCPSendFilePackageInterval;//TCPSendFilePackageInterval TCP����� 2000
	wstring			m_wstrTcpSenderEnabled;			//TcpSenderEnabled		����
	wstring			m_wstrTCPAES256;				//AES256

	//Iot
	wstring			m_wstrIotEnable;				//IotEnable	����
	wstring			m_wstrIotConnection;			//IotConnection Iot�����ַ���

	//RabbitMQ
	wstring			m_wstrRabbitMQHost;				//RabbitMQ��ַ
	wstring			m_wstrRabbitMQUser;				//RabbitMQ�û�
	wstring			m_wstrRabbitMQPsw;				//RabbitMQ����
	wstring			m_wstrRabbitMQSend;				//RabbitMQ����
	wstring			m_wstrRabbitMQReveive;			//RabbitMQ����
	wstring			m_wstrRabbitMQSSL;				//SSSL/TSL����

	//�̶���Ҫд��
	wstring			m_wstrRunDTU;					//rundtu ״̬
	CRITICAL_SECTION m_csDTUDataSync;				//�ٽ������� ����������Դ
	queue<DTU_RECEIVE_INFO>	m_queReceivePaket;		//�洢�������������ݽṹ

	//��
	Project::Tools::Mutex	m_lockSaveReceive;		//�洢�����¼���
	Project::Tools::Mutex	m_lockConnect;			//�����¼���

	int				m_nDebugIndex;

	CProcessView	m_PrsView;
};
