#pragma once

#include "XSTRING"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <list>
#include <assert.h>
#include <string>
#include <windef.h>
#include <process.h>
#include <WinBase.h>
#include <winsock2.h>
#include "./Tools/json/json.h"
#include "Tools/zip/zlib.h"
#include "Tools/Zip/unzip.h"
#include "Tools/zip/zip.h"
#include "Tools/point/DataPointEntry.h"

#pragma comment(lib,"ZLib.lib")
#pragma comment(lib, "ws2_32.lib")

#ifdef _DEBUG
#pragma comment(lib,"json_vc71_libmtd.lib")
#else
#pragma comment(lib,"json_vc71_libmt.lib")
#endif

#define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS 1
#include <hash_map>

using std::vector;
using std::string;
using std::wstring;
using std::list;
using namespace std;
using namespace Json;

/*
	���ͣ�
		�ļ����ݣ�����ѹ��������־�ļ���������־,����ļ�
		�ı����ݣ����ñ�,�������ݣ�����������������¼��Զ�������

	���գ�
		�ļ����ݣ������ļ�����ʧ�ļ���������ļ�, �����ļ�
		�ı����ݣ��������������ͬ��/�޸�unit01,�޸ĵ�ֵ��

	//////////////////////////////////////////////////////////////////////////
	���Ͷ���      ��ţ�ʱ�䣬filename, subtype(),JsonContent,���Դ���


	
	���ȷ����ı�����:
		 ѡȡ filename='' �ҳ��Դ���С��3 ����ʱ���subtype�ۺ�����,�ɹ�ɾ����ʧ�����Լ���+1

	��η���һ�������ļ�   ѡȡ filename!='' subtupe = 0 ����ʱ������,�ɹ�ɾ����ʧ�����Լ���+1
	�ٴη���һ�����������ļ����� ѡȡ filename!='' subtupe != 0 �ҳ��Դ���С��3�� ����ʱ������,�ɹ�ɾ����ʧ�����Լ���+1
	
	ʵʱ����ѹ����(�ļ�����0)
	��־�ļ����ļ�����1��
	�����ļ����ļ�����2��
	����ļ����ļ�����3��
	�����ļ����ļ�����4��

//
*/

//Data Struct

#define DTU_FILE_TAG		"fdp:"				//�ļ���ͷ
#define DTU_FILE_REAL_TAG	"frp:"				//ʵʱ�ļ���ͷ
#define DTU_FILE_HISTORY_TAG "fhp:"				//��ʷ�ļ���ͷ
#define DTU_HEART_TAG		"hba:"				//������ͷ
#define DTU_ACK_TAG			"ack:"				//�ظ���ͷ
#define DTU_UNIT_TAG		"udp:"				//unit��ͷ
#define DTU_REG_TAG			"reg:"				//ע���ͷ
#define DTU_SPACE			"        "			//�ո�

#define MAX_REC_COUNT		8192				//��������
#define MAX_UPDATE_SIZE		50*1024				//���

const int DTU_SEND_PACKAGE_INTERNAL = 2 * 1000;	//dtu���������ļ��ʱ��
const int DTU_SEND_UNIT01_INTERNAL = 2 * 1000;	//dtu���������ļ��ʱ��(���ʱ����̻����)
const int DTU_PACKAGE_SIZE = 900;				//�ְ�ʱ��������
const int MAX_POINT_LENGTH = 120;				//������󳤶�

const int FILE_UPDATE_LENGTH = 4000;			//�ļ������̶�����Ϊ4000
const int TCP_SEND_INTERNAL = 200;				//200ms����һ֡
const int FILE_SIZE = 1024;						//С�Ķ�ȡ�ļ��Ŀռ�����
const int ONE_FILE_SIZE = 4096;					//ÿ��С�Ķ�ȡ�ļ��Ŀռ�

const int IOT_FILE_SIZE = 40;					//IOT��С
const int IOT_ONE_FILE_SIZE = 1024*250;			//IOTÿ���ļ������ʹ�С
const int IOT_RECEIVE_ONE_FILE_SIZE = 1024 * 60;//IOTÿ���ļ������մ�С
const int IOT_SEND_TIME_OUT = 15;				//IOT���ͳ�ʱ
const int IOT_EVENT_COUNT = 100;				//IOT�洢10�������¼�

const int RABBIT_FILE_SIZE = 40;				//RABBIT��С
const int RABBIT_ONE_FILE_SIZE = 1024 * 250;	//RABBITÿ���ļ������ʹ�С
const int RABBIT_SEND_TIME_OUT = 15;			//RabbitMQ���ͳ�ʱ

const string EXE_LOGIC = "BEOPLogicEngine.exe";	//Logic
const string EXE_DOG = "BEOPDog.exe";			//Dog
const string EXE_CORE = "BEOPGatewayCore.exe";	//Core
const string EXE_DTU = "BEOPDTUEngine.exe";		//DTUEngine

const string AES_PSW = "RNB.beop-2013";			//AES256����

enum DTU_CMD_TYPE								//DTU��������
{
	DTU_CMD_DEFINE = 0,							//Ĭ��
	DTU_CMD_REAL_DATA_SEND,						//ʵʱ���ݣ����ͣ�
	DTU_CMD_REAL_DATA_SYN,						//ʵʱ���ݣ�ͬ����
	DTU_CMD_REAL_DATA_EDIT,						//ʵʱ���ݣ��޸ģ�
	DTU_CMD_REAL_DATA_EDIT_MUL,					//ʵʱ���ݣ������޸ģ�
	DTU_CMD_WARNING_DATA_SEND,					//��������(����)
	DTU_CMD_WARNING_DATA_SYN,					//��������(ͬ��)
	DTU_CMD_WARNING_OPERATION_DATA_SEND,		//��������(����)
	DTU_CMD_WARNING_OPERATION_DATA_SYN,			//��������(����)
	DTU_CMD_OPERATION_SEND,						//������¼�����ͣ�
	DTU_CMD_OPERATION_SYN,						//������¼��ͬ����
	DTU_CMD_UNIT_SEND,							//unit01(����)
	DTU_CMD_UNIT_SYN,							//unit01(ͬ��)
	DTU_CMD_UNIT_EDIT,							//unit01(�޸�)
	DTU_CMD_UNIT_EDIT_MUL,						//unit01(�����޸�)
	DTU_CMD_LOG_SEND,							//Log�����ͣ�
	DTU_CMD_LOG_SYN,							//Log��ͬ����
	DTU_CMD_HEART_SEND,							//������(����)
	DTU_CMD_HEART_SYN,							//������(ͬ��)
	DTU_CMD_ERR_LIST,							//ERROR(�����б�)
	DTU_CMD_ERR_CODE,							//ERROR(�������)
	DTU_CMD_ERROR_SEND,							//ERROR�����ͣ�
	DTU_CMD_ERROR_SYN,							//ERROR��ͬ����
	DTU_CMD_REAL_FILE_SEND,						//ʵʱ�ļ������ͣ�
	DTU_CMD_REAL_FILE_SYN,						//ʵʱ�ļ���ͬ����
	DTU_CMD_HISTORY_FILE_SEND,					//��ʷ�ļ������ͣ�
	DTU_CMD_HISTORY_FILE_SYN,					//��ʷ�ļ���ͬ����
	DTU_CMD_UPDATE_EXE,							//���³���
	DTU_CMD_UPDATE_POINT_CSV,					//����CSV���
	DTU_CMD_UPDATE_POINT_EXCEL,					//����EXCEL���
	DTU_CMD_UPDATE_DLL,							//����DLL
	DTU_CMD_UPDATE_S3DB,						//����s3db
	DTU_CMD_UPLOAD_POINT_CSV,					//�ϴ�CSV���
	DTU_CMD_DELETE_POINT_MUL,					//����ɾ����
	DTU_CMD_TIME_SYN,							//ͬ��ʱ��
	DTU_CMD_RESTART_CORE,						//����Core
	DTU_CMD_RESTART_DOG,						//����Dog
	DTU_CMD_RESTART_LOGIC,						//����Logic
	DTU_CMD_RESTART_DTU,						//����DTU
	DTU_CMD_RESTART_DTU_ENGINE,					//����DTUEngine
	DTU_CMD_CORE_STATUS,						//��ȡ������״̬
	DTU_CMD_VERSION_CORE,						//Core�汾��
	DTU_CMD_VERSION_DOG,						//Dog�汾��
	DTU_CMD_VERSION_DTU,						//DTU�汾��
	DTU_CMD_UPDATE_BAT							//����s3db
};

//�������ݽṹ
typedef struct DTU_RECEIVE_INFO_TAG
{
	int				nType;			//0:�ı�   1���ļ�
	int				nID;			//ID
	int				nCmdType;		//��������
	string			strCmdID;		//������
	string			strContent;		//����
	string			strTime;		//ʱ��
}DTU_RECEIVE_INFO;

//�������ݽṹ
typedef struct DTU_UPDATE_INFO_TAG
{
	string			strCmdID;		//������
	string			strFileName;	//�ļ���
	string			strTime;		//ʱ��
}DTU_UPDATE_INFO;

//DTU���ͽṹ
typedef struct DTU_DATA_INFO_TAG
{
	DTU_DATA_INFO_TAG()
	{
		strID = "0";
		nSubType = 0;
		nTryCount = 0;
		strTime = "";
		strFileName = "";
		strContent = "";
		nCmdType = DTU_CMD_DEFINE;
	}
	string		strID;							//���
	string		strTime;						//ʱ��
	string		strFileName;					//�ļ���
	int			nSubType;						//������   0��ʾ�����ļ�  1��ʾ�����ļ�
	int			nTryCount;						//���Դ���
	string		strContent;						//�ı�����  Ansi
	DTU_CMD_TYPE nCmdType;						//��������
} DTU_DATA_INFO;

//IOT���ݽṹ
typedef struct IOT_DATA_INFO_TAG
{
	IOT_DATA_INFO_TAG()
	{
		nLength = 0;
		memset(cData, 0, IOT_ONE_FILE_SIZE);
	}
	char	cData[IOT_ONE_FILE_SIZE];		//����
	int		nLength;						//����
	int		nType;							//0:�ı�����  1:Json�ı� 2:ѹ���ļ���zip��-���ְ�  3��ѹ���ļ���zip��-�ְ�
	int		nCmdType;						//��������
}IOT_DATA_INFO;

//�ļ��ṹ
typedef struct FILE_INFO_TAG
{
	string strFileName;		//�ļ���
	string strFilePath;		//�ļ�·��
}FILE_INFO;

//Iot
//#include "azure_c_shared_utility/platform.h"
//#include "azure_c_shared_utility/threadapi.h"
//#include "azure_c_shared_utility/crt_abstractions.h"
//#include "iothub_client.h"
//#include "iothub_message.h"
//#include "iothubtransportamqp.h"
//
//typedef struct EVENT_INSTANCE_TAG
//{
//	IOTHUB_MESSAGE_HANDLE messageHandle;
//	string messageTrackingId;  // For tracking the messages within the user callback.
//} EVENT_INSTANCE;

//TCP

const int g_nRecMaxNum = 8192;				//�������ݻ����С
const int g_nPackageSize = 4096;			//ÿ�����ݰ���С
typedef struct TCP_DATA_PACKAGE_TAG			//�ְ�һ����
{
	TCP_DATA_PACKAGE_TAG()
	{
		nLength = 0;
		memset(cData, 0, g_nPackageSize);
	}
	char   cData[g_nPackageSize];			//����
	int	   nLength;							//����
} TCP_DATA_PACKAGE;

//RabbitMQ
#include <amqp_ssl_socket.h>
#include <amqp_framing.h>
#include <amqp_tcp_socket.h>
#include <utils.h>

//unit01���ݽṹ
typedef struct  UNIT01_INFO_TAG
{
	string		strProterty01;				//����1
	string		strProterty02;
	string		strProterty03;
}UNIT01_INFO;

//Data���ݽṹ
typedef struct  REAL_DATA_INFO_TAG
{
	string		strTime;				//ʱ��
	string		strPointName;			//����
	string		strPointValue;			//ֵ(Ansi)
}REAL_DATA_INFO;
