#pragma once

#include "DatabaseSession.h"
#include "Tools/CommonDefine.h"

typedef struct IOT_MSG_DATA_TAG
{
	string strMsgID;			//��ϢID
	string strMsgTime;			//��Ϣʱ��
	string strMsgRemark;		//��ע��Ϣ���̣�
	string strFileName;			//�ļ��� ����Ϊ�ı����ݣ���Ϊ�գ�
	string strMsgContent;		//��Ϣ����
	DWORD  dwMsgFailCount;		//��Ϣʧ�ܴ���
}IOT_MSG_DATA;

typedef struct IOT_MSG_REC_DATA_TAG
{
	string strMsgID;			//��ϢID
	string strMsgTime;			//��Ϣʱ��
	string strMsgContent;		//��Ϣ����
	DWORD  dwMsgFailCount;		//��Ϣʧ�ܴ���
}IOT_MSG_REC_DATA;

class CDataHandler: public CDatabaseSeesion
{
public:
	CDataHandler(void);
	~CDataHandler(void);

public:
	wstring ReadOrCreateCoreDebugItemValue_Defalut(wstring strItemName, wstring strValue = L"0");
	bool    WriteCoreDebugItemValue(const wstring &strItemName, const wstring &strItemValue);

	bool	GetOneLatestNormalFile(DTU_DATA_INFO & data, int nTryLimit = 3);			// ��η���ʱ�����µ��ļ����ݣ�����ʧ�ܴ����ﵽ3�κ󣬲��ٴ���
	bool	GetOneLatestDataFile(DTU_DATA_INFO & data);									// ����ʱ�����µ��ļ����ݣ���һֱ���ԣ�
	bool	GetLatestString(DTU_DATA_INFO & data, int nTryLimit = 3);					// ����ʱ�����µ��ı����ݣ�����ʧ�ܴ����ﵽ3�κ󣬲��ٴ���
	bool	GetLatestReceiveData(DTU_RECEIVE_INFO& data, int nMinute=60);				// ��ȡ���µ�һ�����ݣ�ʱ����60�����ڵģ�
	bool	GetUpdateData(DTU_UPDATE_INFO& data);										// ��ȡ����������

	bool	HandlerOneData(DTU_DATA_INFO & data,bool bResult);							//���ݷ��ͽ������һ������
	bool	SaveOneSendData(DTU_DATA_INFO & data);										//����һ�������͵���Ϣ
	bool	SaveMulSendData(vector<DTU_DATA_INFO>& vecData);							//�����������͵���Ϣ
	bool	SaveOneRecData(DTU_RECEIVE_INFO& data);										//�����յ�����Ϣ
	bool	SaveOneUpdateData(DTU_UPDATE_INFO& data);									//����һ��������Ϣ

	bool	ClearExpireData(string strExpireTime);										//����ʱ�������������
	bool	DeleteOneRecData(int nID);													//ɾ��һ������
	bool	DeleteOneUpdateData(DTU_UPDATE_INFO& data);									//ɾ��һ������������

	//////////////////////////////////////////////////////////////////////////
	bool	ReadUnit01(vector<UNIT01_INFO>& unitList);									//��ȡUnit01������
	bool	ChangeValues(vector<REAL_DATA_INFO>& vecValue);								//�޸ĵ�
	string	ReadErrCodeRecentMinute(string nMintue = "10");								//��ȡ����Ĵ������
	string	ReadMysqlVariable(string strVariableName);									//��ȡ���ò���

private:
	bool	DeleteOneData(DTU_DATA_INFO & data);										//ɾ��һ������
	bool	SetDataFailCount(DTU_DATA_INFO & data);										//��������ʧ�ܴ���

private:
	Project::Tools::Mutex	m_lock;

};

