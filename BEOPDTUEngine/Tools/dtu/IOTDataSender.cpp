#include "stdafx.h"
#include "IOTDataSender.h"
#include "PackageTypeDefine.h"

hash_map<string, DTU_DATA_INFO> CIotDataSender::m_mapData;
bool			CIotDataSender::m_bIotConnectSuccess = false;
CDataHandler*	CIotDataSender::m_pDataHandler = NULL;
char*			CIotDataSender::m_cFileBuffer = NULL;		//�ļ�����
int				CIotDataSender::m_nFileBuffer = 0;				//���ļ���С
int				CIotDataSender::m_nCurrentFileBuffer = 0;		//��ǰ�ļ���С
int				CIotDataSender::m_nCmdType = 0;						//��������
string			CIotDataSender::m_strFileName = "";				//�ļ���
LPRecDataProc	CIotDataSender::m_lpRecDataProc = NULL;			//�յ����ݻص�
DWORD			CIotDataSender::m_dwUserData = 0;				//�û�����
bool			CIotDataSender::m_bBusy = false;

CIotDataSender::CIotDataSender(wstring wstrIotConnection, CDataHandler*	pDataHandler)
	: m_wstrIotConnection(wstrIotConnection)
	, m_bExitThread(false)
	, m_nIOTErrorCount(0)
	, m_pIotHubClientHandle(NULL)
	, m_hActiveHandler(NULL)
	, m_nCurrentEvent(0)
{
	m_pDataHandler = pDataHandler;
	m_oleIOTUpdateTime = COleDateTime::GetCurrentTime();
}

CIotDataSender::~CIotDataSender()
{
}

bool CIotDataSender::Init(LPRecDataProc proc1, DWORD userdata)
{
	m_bExitThread = false;
	m_bIotConnectSuccess = false;
	m_dwUserData = userdata;
	m_lpRecDataProc = proc1;

	//
	CString strDebugInfo;
	strDebugInfo.Format(_T("INFO: Init IotDataSender(%s).\r\n"), m_wstrIotConnection.c_str());
	PrintLog(strDebugInfo.GetString());
	return true;
}

bool CIotDataSender::Exit()
{
	DisConnect();
	return true;
}

bool CIotDataSender::HandlerResult(string strMsgID, IOTHUB_CLIENT_CONFIRMATION_RESULT result)
{
	hash_map<string, DTU_DATA_INFO>::iterator iterData = m_mapData.find(strMsgID);
	bool bSendSuccess = false;
	if (result == IOTHUB_CLIENT_CONFIRMATION_OK)				//ִ�з��ͳɹ� Ҫɾ����Ϣ
	{
		bSendSuccess = true;
	}
	if (iterData != m_mapData.end())
	{
		DTU_DATA_INFO data = (*iterData).second;
		if (m_pDataHandler && m_pDataHandler->IsConnected())
		{
			m_pDataHandler->HandlerOneData(data, bSendSuccess);
		}
		m_mapData.erase(iterData);

		//
		std::ostringstream sqlstream;
		if (bSendSuccess)
		{
			sqlstream << "SEND: ";
		}
		else
		{
			sqlstream << "ERROR: ";
		}
		switch (data.nCmdType)
		{
		case DTU_CMD_DEFINE:
			break;
		case DTU_CMD_REAL_DATA_SEND:
			break;
		case DTU_CMD_REAL_DATA_SYN:
			break;
		case DTU_CMD_REAL_DATA_EDIT:
			break;
		case DTU_CMD_REAL_DATA_EDIT_MUL:
			break;
		case DTU_CMD_WARNING_DATA_SEND:
			break;
		case DTU_CMD_WARNING_DATA_SYN:
			break;
		case DTU_CMD_WARNING_OPERATION_DATA_SEND:
			break;
		case DTU_CMD_WARNING_OPERATION_DATA_SYN:
			break;
		case DTU_CMD_OPERATION_SEND:
			break;
		case DTU_CMD_OPERATION_SYN:
			break;
		case DTU_CMD_UNIT_SEND:
		case DTU_CMD_UNIT_SYN:
		{
			sqlstream << "Unit01(" << data.strContent.length() << ")" << "\r\n";
		}
		break;
		case DTU_CMD_UNIT_EDIT:
			break;
		case DTU_CMD_UNIT_EDIT_MUL:
			break;
		case DTU_CMD_LOG_SEND:
		case DTU_CMD_LOG_SYN:
		{
			sqlstream << "MysqlFile(" << data.strFileName << ")" << "\r\n";
		}
		break;
		case DTU_CMD_HEART_SEND:
			break;
		case DTU_CMD_HEART_SYN:
			break;
		case DTU_CMD_ERR_LIST:
			break;
		case DTU_CMD_ERR_CODE:
			break;
		case DTU_CMD_ERROR_SEND:
			break;
		case DTU_CMD_ERROR_SYN:
			break;
		case DTU_CMD_REAL_FILE_SEND:
		case DTU_CMD_REAL_FILE_SYN:
		{
			sqlstream << "RealFileData(" << data.strFileName << ")" << "\r\n";
		}
		break;
		case DTU_CMD_HISTORY_FILE_SEND:
		case DTU_CMD_HISTORY_FILE_SYN:
		{
			sqlstream << "HistoryFileData(" << data.strFileName << ")" << "\r\n";
		}
		break;
		case DTU_CMD_UPDATE_EXE:
			break;
		case DTU_CMD_UPDATE_POINT_CSV:
			break;
		case DTU_CMD_UPDATE_POINT_EXCEL:
			break;
		case DTU_CMD_UPDATE_DLL:
			break;
		case DTU_CMD_UPDATE_S3DB:
			break;
		case DTU_CMD_UPLOAD_POINT_CSV:
			break;
		case DTU_CMD_DELETE_POINT_MUL:
			break;
		case DTU_CMD_TIME_SYN:
			break;
		case DTU_CMD_RESTART_CORE:
			break;
		case DTU_CMD_RESTART_DOG:
			break;
		case DTU_CMD_RESTART_DTU:
			break;
		case DTU_CMD_CORE_STATUS:
			break;
		default:
			break;
		}
		PrintLog(Project::Tools::AnsiToWideChar(sqlstream.str().c_str()));
	}

	return true;
}

void CIotDataSender::PrintLog(const wstring & strLog)
{
	_tprintf(strLog.c_str());
}

IOTHUBMESSAGE_DISPOSITION_RESULT CIotDataSender::HandlerReceive(const unsigned char* buffer, int nType, int nCmdType, int nSize, int nCmdID, string strFileName, string strIndex)
{
	if (buffer != NULL && nSize > 0)
	{
		if (nType == 0)					//0:�ı�����
		{
			HandlerText(buffer, nType, nCmdType, nSize, nCmdID, strFileName, strIndex);
		}
		else if (nType == 1)				//1:Json�ı�
		{

		}
		else if (nType == 2)			//2:ѹ���ļ���zip��-���ְ�
		{
			HandlerFile(buffer, nType, nCmdType, nSize, nCmdID, strFileName, strIndex);
		}
		else if (nType == 3)			//3��ѹ���ļ���zip��-�ְ�
		{
			HandlerFilePackage(buffer, nType, nCmdType, nSize, nCmdID, strFileName, strIndex);
		}
	}
	return IOTHUBMESSAGE_ACCEPTED;
}

void CIotDataSender::HandlerText(const unsigned char * buffer, int nType, int nCmdType, int nSize, int nCmdID, string strFileName, string strIndex)
{
	try
	{
		string strBuffer = (char*)buffer;
		if (buffer != NULL && nSize>13 && strBuffer.length() == nSize)
		{
			rnbtech::CPackageType::DTU_DATATYPE type = rnbtech::CPackageType::GetPackageType((const char*)buffer);
			switch (type)
			{
			case rnbtech::CPackageType::Type_DTUServerCmd:
			{
				rnbtech::CPackageType::RemovePrefix((char*)buffer, nSize);
				char* pReveive = strtok((char*)buffer, ";");

				string strReveive = pReveive;
				//�������ռ�¼ ������
				Json::Value jsonBody;
				jsonBody["type"] = 0;    //�ļ�
				jsonBody["content"] = strReveive;
				string strContent = jsonBody.toStyledString();
				if (m_lpRecDataProc)
					m_lpRecDataProc((const unsigned char*)strContent.data(), strContent.size(), m_dwUserData);
			}
			}
		}
	}
	catch (const std::exception&)
	{

	}
}

void CIotDataSender::HandlerFile(const unsigned char * buffer, int nType, int nCmdType, int nSize, int nCmdID, string strFileName, string strIndex)
{
	if (buffer != NULL && nSize > 0 && strFileName.length()>0 && strIndex.length()>0)
	{
		m_nCmdType = nCmdType;
		m_strFileName = strFileName;

		//�����ļ�
		wstring wstrFileFolder;
		Project::Tools::GetSysPath(wstrFileFolder);
		SYSTEMTIME sNow;
		GetLocalTime(&sNow);
		wstrFileFolder += L"\\fileupdate";
		if (Project::Tools::FindOrCreateFolder(wstrFileFolder))
		{
			CString strFilePath;
			strFilePath.Format(_T("%s\\%s"), wstrFileFolder.c_str(), Project::Tools::AnsiToWideChar(m_strFileName.c_str()).c_str());
			CFile file(strFilePath, CFile::modeCreate | CFile::modeWrite);
			file.Seek(0, CFile::begin);
			file.Write(buffer, nSize);
			file.Close();

			CString str;
			str.Format(_T("RECV: TCPDataSender Receive Update File Package(%s)...\r\n"), Project::Tools::AnsiToWideChar(m_strFileName.c_str()).c_str());
			PrintLog(str.GetString());

			//�������ռ�¼ ������
			string strRecevieFilePath = Project::Tools::WideCharToAnsi(strFilePath);
			Json::Value jsonBody;
			jsonBody["type"] = 1;    //�ļ�
			if (nCmdType == DTU_CMD_HISTORY_FILE_SYN || nCmdType == DTU_CMD_HISTORY_FILE_SEND)
			{
				jsonBody["type"] = 2;    //�ļ�
			}
			jsonBody["content"] = strRecevieFilePath;
			string strContent = jsonBody.toStyledString();
			if (m_lpRecDataProc)
				m_lpRecDataProc((const unsigned char*)strContent.data(), strContent.size(), m_dwUserData);
		}
	}
}

void CIotDataSender::HandlerFilePackage(const unsigned char * buffer, int nType, int nCmdType, int nSize, int nCmdID, string strFileName, string strIndex)
{
	if (buffer != NULL && nSize > 0)
	{
		if (strIndex.length() > 0)
		{
			vector<int> vecPacketIndex;
			Project::Tools::SplitString(strIndex.c_str(), "/", vecPacketIndex);
			if (vecPacketIndex.size() == 2)
			{
				int nPacketIndex = vecPacketIndex[0];
				int nPacketCount = vecPacketIndex[1];

				if (nPacketIndex == 1)			//�ļ���ʼ  �����ǰ���ļ����� ����һ���ļ�����
				{
					ClearFileBuffer();
					m_nFileBuffer = nPacketCount * IOT_RECEIVE_ONE_FILE_SIZE;
					m_cFileBuffer = new char[m_nFileBuffer];
					ZeroMemory(m_cFileBuffer, 0, m_nFileBuffer);
					m_nCmdType = nCmdType;
					m_strFileName = strFileName;
				}

				//�洢
				if (m_cFileBuffer && m_nCurrentFileBuffer + nSize <= m_nFileBuffer)
				{
					memcpy(m_cFileBuffer + m_nCurrentFileBuffer, buffer, nSize);
					m_nCurrentFileBuffer = m_nCurrentFileBuffer + nSize;
				}

				if (nPacketIndex == nPacketCount)			//�����ļ� �����Сһ�£����ļ��ɹ������� �������
				{
					if (m_strFileName.length() > 0 && m_nCurrentFileBuffer > (m_nFileBuffer - IOT_RECEIVE_ONE_FILE_SIZE) && m_nCurrentFileBuffer <= m_nFileBuffer)
					{
						//�����ļ�
						wstring wstrFileFolder;
						Project::Tools::GetSysPath(wstrFileFolder);
						SYSTEMTIME sNow;
						GetLocalTime(&sNow);
						wstrFileFolder += L"\\fileupdate";
						if (Project::Tools::FindOrCreateFolder(wstrFileFolder))
						{
							CString strFilePath;
							strFilePath.Format(_T("%s\\%s"), wstrFileFolder.c_str(), Project::Tools::AnsiToWideChar(m_strFileName.c_str()).c_str());
							CFile file(strFilePath, CFile::modeCreate | CFile::modeWrite);
							file.Seek(0, CFile::begin);
							file.Write(m_cFileBuffer, m_nCurrentFileBuffer);
							file.Close();

							CString str;
							str.Format(_T("RECV: TCPDataSender Receive Update File Package(%s)...\r\n"), Project::Tools::AnsiToWideChar(m_strFileName.c_str()).c_str());
							PrintLog(str.GetString());

							//�������ռ�¼ ������
							string strRecevieFilePath = Project::Tools::WideCharToAnsi(strFilePath);
							Json::Value jsonBody;
							jsonBody["type"] = 1;    //�ļ�
							if (nCmdType == DTU_CMD_HISTORY_FILE_SYN || nCmdType == DTU_CMD_HISTORY_FILE_SEND)
							{
								jsonBody["type"] = 2;    //�ļ�
							}
							jsonBody["content"] = strRecevieFilePath;
							string strContent = jsonBody.toStyledString();
							if (m_lpRecDataProc)
								m_lpRecDataProc((const unsigned char*)strContent.data(), strContent.size(), m_dwUserData);
						}
					}
					ClearFileBuffer();
				}
			}
		}
	}
}

bool CIotDataSender::ClearFileBuffer()
{
	if (m_cFileBuffer)
	{
		delete[]m_cFileBuffer;
		m_cFileBuffer = NULL;
	}
	m_nFileBuffer = 0;
	m_nCurrentFileBuffer = 0;
	m_nCmdType = 0;
	m_strFileName = "";
	return true;
}

bool CIotDataSender::ReConnect()
{
	//�Լ��ж��費��Ҫ����
	COleDateTimeSpan oleSpan = COleDateTime::GetCurrentTime() - m_oleIOTUpdateTime;
	if (m_bIotConnectSuccess == false || m_nIOTErrorCount >= 5 || oleSpan.GetTotalMinutes() >= 30)			//��������ﵽ5�� ����30������δ�ɹ��շ���һ����Ҫ����
	{
		DisConnect();
		if (IOTConnect(Project::Tools::WideCharToUtf8(m_wstrIotConnection.c_str()).c_str()))
		{
			PrintLog(L"INFO: IotDataSender Connect Success.\r\n");
			return true;
		}
		else
		{
			PrintLog(L"ERROR: IotDataSender Connect Fail.\r\n");
			return false;
		}
	}
	return false;
}

bool CIotDataSender::DisConnect()
{
	m_bIotConnectSuccess = false;
	m_bExitThread = true;
	if (m_pIotHubClientHandle)
	{
		//ȡ����ʼ�� IoT �� �˵����ͷ� IoTHubClient_CreateFromConnectionString ������ǰ�������Դ��
		IoTHubClient_LL_Destroy(m_pIotHubClientHandle);
		m_pIotHubClientHandle = NULL;
	}
	platform_deinit();
	return true;
}

bool CIotDataSender::IOTConnect(const char * connectionString)
{
	m_bIotConnectSuccess = false;
	m_bExitThread = true;

	if (m_pIotHubClientHandle == NULL)
	{
		if (platform_init() != 0)
		{
			PrintLog(L"ERROR: IotDataSender Initialize Platform Fail!\r\n");
			return false;
		}
		else
		{
			//�豸�����ַ������ݸ��˺����� ����ָ��Ҫʹ�õ�ͨ��Э��
			if ((m_pIotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(connectionString, AMQP_Protocol)) == NULL)
			{
				PrintLog(L"ERROR: IotDataSender Create IotHubClientHandle Fail!\r\n");
				return false;
			}
			else
			{
				bool traceOn = false;   //log����ر�
										//���ü���ѡ�������Ŀ�Ĺ�����ʽ
				IoTHubClient_LL_SetOption(m_pIotHubClientHandle, "logtrace", &traceOn);

				//�����������  60*1000ms
				if (IoTHubClient_LL_SetRetryPolicy(m_pIotHubClientHandle, IOTHUB_CLIENT_RETRY_INTERVAL, 10 * 1000) != IOTHUB_CLIENT_OK)
				{
					PrintLog(L"ERROR: IotDataSender Set RetryPolicy Fail!\r\n");
					return false;
				}

				/* Setting Message call back, so we can receive Commands. */
				//������Ϣ��һ���첽���������ȣ���ע�ᵱ�豸������Ϣʱ��Ҫ���õĻص���
				int receiveContext = 0;
				if (IoTHubClient_LL_SetMessageCallback(m_pIotHubClientHandle, ReceiveMessageCallback, &receiveContext) != IOTHUB_CLIENT_OK)
				{
					PrintLog(L"ERROR: IotDataSender Set MessageCallback Fail!\r\n");
					return false;
				}

				//����״̬�ص�����
				receiveContext = 0;
				if (IoTHubClient_LL_SetConnectionStatusCallback(m_pIotHubClientHandle, ConnectStatusCallback, &receiveContext) != IOTHUB_CLIENT_OK)
				{
					PrintLog(L"ERROR: IotDataSender Set ConnectionStatusCallback Fail!\r\n");
					return false;
				}

				//�ɹ����� ������
				m_bIotConnectSuccess = true;
				m_bExitThread = false;
				if (m_hActiveHandler == NULL)
					m_hActiveHandler = (HANDLE)_beginthreadex(NULL, 0, ActiveThread, this, 0, NULL);
				return m_bIotConnectSuccess;
			}
		}
	}
	return false;
}

bool CIotDataSender::SendOneData(DTU_DATA_INFO & data)
{
	bool bSendResult = false;
	if (data.strFileName.length() > 0)				//�ļ�����  �ɹ�ɾ����ʧ�����Լ���+1
	{
		string strJson = data.strContent;
		if (strJson.length() > 0)
		{
			StringReplace(strJson, "\\", "/");				//ת��		

			Json::Reader reader;
			Json::Value value;
			if (reader.parse(strJson, value))
			{
				Value arrayObj = value;
				DTU_CMD_TYPE type = DTU_CMD_DEFINE;
				int nPointCount = 0;
				string strFilePath = "";
				string strReamrk = "";
				if (!arrayObj["id"].isNull() && arrayObj["id"].isInt())
				{
					nPointCount = (DTU_CMD_TYPE)arrayObj["id"].asInt();
				}
				if (!arrayObj["type"].isNull() && arrayObj["type"].isInt())
				{
					type = (DTU_CMD_TYPE)arrayObj["type"].asInt();
				}

				if (!arrayObj["content"].isNull() && arrayObj["content"].isString())
				{
					strFilePath = arrayObj["content"].asString();
				}
				if (!arrayObj["remark"].isNull() && arrayObj["remark"].isString())
				{
					strReamrk = arrayObj["remark"].asString();
				}
				data.nCmdType = type;
				return SendFile(strFilePath, type, data);
			}
		}
	}
	else
	{
		string strJson = data.strContent;
		if (strJson.length() > 0)
		{
			StringReplace(strJson, "\\", "/");				//ת��		

			Json::Reader reader;
			Json::Value value;
			if (reader.parse(strJson, value))
			{
				Value arrayObj = value;
				DTU_CMD_TYPE type = DTU_CMD_DEFINE;
				string strStrungContent = "";
				string strReamrk = "";
				if (!arrayObj["type"].isNull() && arrayObj["type"].isInt())
				{
					type = (DTU_CMD_TYPE)arrayObj["type"].asInt();
				}

				if (!arrayObj["content"].isNull() && arrayObj["content"].isString())
				{
					strStrungContent = arrayObj["content"].asString();
				}

				if (!arrayObj["remark"].isNull() && arrayObj["remark"].isString())
				{
					strReamrk = arrayObj["remark"].asString();
				}
				data.nCmdType = type;
				return SendString(strStrungContent, type, strReamrk,data);
			}
		}
	}
	return false;
}

bool CIotDataSender::SendString(string strContent, DTU_CMD_TYPE nCmdType, string strCmdID, DTU_DATA_INFO& data)
{
	if (m_bIotConnectSuccess)
	{
		CString strLog, strDataID;
		if (strContent.length() > 0)
		{
			strDataID = Project::Tools::AnsiToWideChar(data.strID.c_str()).c_str();

			if (m_nCurrentEvent >= IOT_EVENT_COUNT)
				m_nCurrentEvent = 0;

			int nLength = strContent.length();
			if ((m_aEvent[m_nCurrentEvent].messageHandle = IoTHubMessage_CreateFromByteArray((const unsigned char*)strContent.c_str(), nLength)) == NULL)
			{
				strLog.Format(_T("ERROR: IotDataSender IoTHubMessage_CreateFromByteArray Fail(msgID:%s)!\r\n"), strDataID);
				PrintLog(strLog.GetString());
				return false;
			}
			else
			{
				m_aEvent[m_nCurrentEvent].messageTrackingId = data.strID;
				m_mapData[data.strID] = data;

				//��Ӹ������� type  remark
				MAP_HANDLE propMap = IoTHubMessage_Properties(m_aEvent[m_nCurrentEvent].messageHandle);

				//type  0:�ı�����
				if (Map_AddOrUpdate(propMap, "type", "0") != MAP_OK)
				{
					strLog.Format(_T("ERROR: IotDataSender Map_AddOrUpdate Failed(msgID:%s)!\r\n"), strDataID);
					PrintLog(strLog.GetString());
					return false;
				}
				
				std::ostringstream sqlstream;
				sqlstream << nCmdType;		//��������
				if (Map_AddOrUpdate(propMap, "cmdtype", sqlstream.str().c_str()) != MAP_OK)
				{
					strLog.Format(_T("ERROR: IotDataSender Map_AddOrUpdate Failed(msgID:%s)!\r\n"), strDataID);
					PrintLog(strLog.GetString());
					return false;
				}

				//����С
				sqlstream.str("");
				sqlstream << strContent.length();
				if (Map_AddOrUpdate(propMap, "size", sqlstream.str().c_str()) != MAP_OK)
				{
					strLog.Format(_T("ERROR: IotDataSender Map_AddOrUpdate Failed(msgID:%s)!\r\n"), strDataID);
					PrintLog(strLog.GetString());
					return false;
				}

				//����ID
				if (Map_AddOrUpdate(propMap, "id", strCmdID.c_str()) != MAP_OK)
				{
					strLog.Format(_T("ERROR: IotDataSender Map_AddOrUpdate Failed(msgID:%s)!\r\n"), strDataID);
					PrintLog(strLog.GetString());
					return false;
				}

				m_bBusy = false;
				m_oleSendTime = COleDateTime::GetCurrentTime();
				if (IoTHubClient_LL_SendEventAsync(m_pIotHubClientHandle, m_aEvent[m_nCurrentEvent].messageHandle, SendConfirmationCallback, &m_aEvent[m_nCurrentEvent]) != IOTHUB_CLIENT_OK)
				{
					strLog.Format(_T("ERROR: IotDataSender Map_AddOrUpdate Failed(msgID:%s)!\r\n"), strDataID);
					PrintLog(strLog.GetString());
					return false;
				}
				else
				{
					return true;
				}
			}
		}
	}
	return false;
}

bool CIotDataSender::IsConnected()
{
	if (m_pIotHubClientHandle && m_bIotConnectSuccess)
	{
		//��Ҫ�ж��¶����Ƿ�æµ
		IOTHUB_CLIENT_STATUS status;
		if (IoTHubClient_LL_GetSendStatus(m_pIotHubClientHandle, &status) == IOTHUB_CLIENT_OK)
		{
			if (status == IOTHUB_CLIENT_SEND_STATUS_BUSY)
				return false;
		}

		if (m_bBusy == false)		//æµ״̬
		{
			COleDateTimeSpan oleSpan = COleDateTime::GetCurrentTime() - m_oleSendTime;
			if (oleSpan.GetTotalSeconds() >= IOT_SEND_TIME_OUT)			//æµ����ʱ��  �ж�Ϊ����ȷ�ϳ�ʱ ���»ָ�����
			{
				m_bBusy = true;
			}
		}
		return m_bBusy;
	}
	return false;
}

bool CIotDataSender::IsBusy()
{
	return m_bBusy;
}

UINT CIotDataSender::ActiveThread(LPVOID lpVoid)
{
	CIotDataSender* pIOTDataSender = reinterpret_cast<CIotDataSender*>(lpVoid);
	if (NULL == pIOTDataSender)
	{
		return 0;
	}

	while (!pIOTDataSender->m_bExitThread)
	{
		pIOTDataSender->ActiveIot();
	}
	return 0;
}

bool CIotDataSender::ActiveIot()
{
	if (m_pIotHubClientHandle && m_bIotConnectSuccess)
	{
		IoTHubClient_LL_DoWork(m_pIotHubClientHandle);
	}
	ThreadAPI_Sleep(10);
	return false;
}

bool CIotDataSender::SendFile(string strFilePath, DTU_CMD_TYPE nCmdType, DTU_DATA_INFO& data)
{
	if (m_bIotConnectSuccess)
	{
		CString strLog, strDataID;
		if (strFilePath.length() > 0)
		{
			if (Project::Tools::FindFileExist(Project::Tools::AnsiToWideChar(strFilePath.c_str())))		//�ļ�����
			{
				//
				strDataID = Project::Tools::AnsiToWideChar(data.strID.c_str()).c_str();

				//�ְ����ļ�
				gzFile gz = gzopen(strFilePath.c_str(), "rb");
				if (!gz)
				{
					strLog.Format(_T("ERROR: IotDataSender SendFile Fail(%s)!\r\n"), Project::Tools::AnsiToWideChar(data.strFileName.c_str()).c_str());
					PrintLog(strLog.GetString());
					return false;
				}
				else
				{
					vector<char*> vecFile;
					int nLastLen = 0;
					//��ʼ�����ļ�
					for (;;)
					{
						//һ�ζ�ȡBLOCKSIZE��С���ļ�����
						char* cFile = new char[IOT_ONE_FILE_SIZE];
						nLastLen = gzread(gz, cFile, IOT_ONE_FILE_SIZE);
						vecFile.push_back(cFile);
						if (nLastLen < IOT_ONE_FILE_SIZE)
							break;
					}
					//�ر��ļ�
					gzclose(gz);

					//����
					bool bResult = true;
					int nType = 2;
					int nCount = vecFile.size();
					if (nCount > 1)								//��Ҫ�ְ�����
						nType = 3;

					for (int i = 0;i < nCount; ++i)
					{
						if (m_nCurrentEvent >= IOT_EVENT_COUNT)
							m_nCurrentEvent = 0;

						int nLength = IOT_ONE_FILE_SIZE;
						if ((m_aEvent[m_nCurrentEvent].messageHandle = IoTHubMessage_CreateFromByteArray((const unsigned char*)vecFile[i], nLength)) == NULL)
						{
							strLog.Format(_T("ERROR: IotDataSender IoTHubMessage_CreateFromByteArray Fail(msgID:%s)!\r\n"), strDataID);
							PrintLog(strLog.GetString());
							bResult = false;
							break;					//����ѭ��
						}
						else
						{
							m_aEvent[m_nCurrentEvent].messageTrackingId = data.strID;
							m_mapData[data.strID] = data;

							//��Ӹ������� type  remark name size index
							MAP_HANDLE propMap = IoTHubMessage_Properties(m_aEvent[m_nCurrentEvent].messageHandle);
							std::ostringstream sqlstream;
							sqlstream << nType;

							//type				0:�ı�����  1:Json�ı� 2:ѹ���ļ���zip��-���ְ�  3��ѹ���ļ���zip��-�ְ�  (�ر�)
							if (Map_AddOrUpdate(propMap, "type", sqlstream.str().c_str()) != MAP_OK)
							{
								strLog.Format(_T("ERROR: IotDataSender Map_AddOrUpdate Failed(msgID:%s)!\r\n"), strDataID);
								PrintLog(strLog.GetString());

								bResult = false;
								break;
							}

							sqlstream.str("");
							sqlstream << nCmdType;		//��������
							if (Map_AddOrUpdate(propMap, "cmdtype", sqlstream.str().c_str()) != MAP_OK)
							{
								strLog.Format(_T("ERROR: IotDataSender Map_AddOrUpdate Failed(msgID:%s)!\r\n"), strDataID);
								PrintLog(strLog.GetString());
								bResult = false;
								break;
							}

							//name
							if (Map_AddOrUpdate(propMap, "name", data.strFileName.c_str()) != MAP_OK)
							{
								strLog.Format(_T("ERROR: IotDataSender Map_AddOrUpdate Failed(msgID:%s)!\r\n"), strDataID);
								PrintLog(strLog.GetString());
								bResult = false;
								break;
							}

							//size
							DWORD dwFileSize = 0;
							if (i == nCount - 1)
							{
								dwFileSize = nLastLen;
							}
							else
							{
								dwFileSize = IOT_ONE_FILE_SIZE;
							}
							sqlstream.str("");
							sqlstream << dwFileSize;
							if (Map_AddOrUpdate(propMap, "size", sqlstream.str().c_str()) != MAP_OK)
							{
								strLog.Format(_T("ERROR: IotDataSender Map_AddOrUpdate Failed(msgID:%s)!\r\n"), strDataID);
								PrintLog(strLog.GetString());
								bResult = false;
								break;
							}

							//index
							sqlstream.str("");
							sqlstream << i + 1 << "/" << nCount;
							if (Map_AddOrUpdate(propMap, "index", sqlstream.str().c_str()) != MAP_OK)
							{
								strLog.Format(_T("ERROR: IotDataSender Map_AddOrUpdate Failed(msgID:%s)!\r\n"), strDataID);
								PrintLog(strLog.GetString());
								bResult = false;
								break;
							}

							//����
							m_bBusy = false;
							m_oleSendTime = COleDateTime::GetCurrentTime();
							if (IoTHubClient_LL_SendEventAsync(m_pIotHubClientHandle, m_aEvent[m_nCurrentEvent].messageHandle, SendConfirmationCallback, &m_aEvent[m_nCurrentEvent]) != IOTHUB_CLIENT_OK)
							{
								strLog.Format(_T("ERROR: IotDataSender IoTHubClient_SendEventAsync Failed(msgID:%s)!\r\n"), strDataID);
								PrintLog(strLog.GetString());

								bResult = false;
								break;
							}
							else
							{
								bResult = true;
							}

							ThreadAPI_Sleep(1000);			//���ͼ��1s
						}
					}
					return bResult;
				}
			}
			else
			{
				strLog.Format(_T("ERROR: IotDataSender SendFile Fail(%s)!\r\n"), Project::Tools::AnsiToWideChar(data.strFileName.c_str()).c_str());
				PrintLog(strLog.GetString());
			}
		}
		else
		{
			strLog.Format(_T("ERROR: IotDataSender SendFile Fail(%s)!\r\n"), Project::Tools::AnsiToWideChar(data.strFileName.c_str()).c_str());
			PrintLog(strLog.GetString());
		}
	}
	return false;
}

IOTHUBMESSAGE_DISPOSITION_RESULT CIotDataSender::ReceiveMessageCallback(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback)
{
	const unsigned char* buffer = NULL;
	size_t size = 0;
	const char* messageId;

	//����Ϣ�м�����Ϣ ID ����� ID��
	// Message properties
	if ((messageId = IoTHubMessage_GetMessageId(message)) != NULL)
	{
		//������Ϣ����
		IOTHUBMESSAGE_CONTENT_TYPE contentType = IoTHubMessage_GetContentType(message);
		if (contentType == IOTHUBMESSAGE_BYTEARRAY)
		{
			if (IoTHubMessage_GetByteArray(message, &buffer, &size) == IOTHUB_MESSAGE_OK)
			{
				//printf("Received  Message ID: %s  Size=%d\r\n", messageId, (int)size);
			}

		}
		else if (contentType == IOTHUBMESSAGE_STRING)
		{
			if ((buffer = (const unsigned char*)IoTHubMessage_GetString(message)) != NULL && (size = strlen((const char*)buffer)) > 0)
			{
				//printf("Received Message ID: %s Size=%d\r\n", messageId, (int)size);
			}

		}
		else
		{
			PrintLog(L"ERROR: IotDataSender Receive Fail!\r\n");	
		}

		if (size > 0)
		{
			//����Ϣ�м����κ��Զ������ԡ�
			MAP_HANDLE mapProperties = IoTHubMessage_Properties(message);
			if (mapProperties != NULL)
			{
				const char*const* keys;
				const char*const* values;
				size_t propertyCount = 0;
				if (Map_GetInternals(mapProperties, &keys, &values, &propertyCount) == MAP_OK)
				{
					if (propertyCount > 0)
					{
						int nType = 0;		//����
						int nCmdType = 0;	//��������
						int nSize = 0;		//����С
						int nCmdID = 0;		//����ID
						string strFileName, strIndex;
						for (size_t index = 0; index < propertyCount; index++)
						{
							string strKey = keys[index];
							string strValue = values[index];
							if (strKey == "type")
							{
								nType = ATOI(strValue.c_str());
							}
							else if(strKey == "cmdtype")
							{
								nCmdType = ATOI(strValue.c_str());
							}
							else if (strKey == "size")
							{
								nSize = ATOI(strValue.c_str());
							}
							else if (strKey == "name")
							{
								strFileName = strValue;
							}
							else if (strKey == "index")
							{
								strIndex = strValue;
							}
							else if (strKey == "id")
							{
								nCmdID = ATOI(strValue.c_str());
							}
						}
						//
						return HandlerReceive(buffer, nType, nCmdType, nSize, nCmdID, strFileName, strIndex);
					}
				}
			}
		}
	}
	/*
	IOTHUBMESSAGE_ACCEPTED - ��Ϣ�ѳɹ�����IoTHubClient �⽫����ͬһ��Ϣ�ٴε��ûص�������

	IOTHUBMESSAGE_REJECTED - δ������Ϣ���ҽ���Ҳ���ᴦ��IoTHubClient �ⲻӦ�ö�ͬһ��Ϣ�ٴε��ûص�������

	IOTHUBMESSAGE_ABANDONED - ��Ϣδ�ɹ������� IoTHubClient ��Ӧ�ö�ͬһ��Ϣ�ٴε��ûص�������
	*/
	return IOTHUBMESSAGE_ABANDONED;
}

void CIotDataSender::SendConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void * userContextCallback)
{
	EVENT_INSTANCE* eventInstance = (EVENT_INSTANCE*)userContextCallback;
	string strMsgID = "";
	if (eventInstance)
	{
		strMsgID = eventInstance->messageTrackingId;
		if(eventInstance->messageHandle)
			IoTHubMessage_Destroy(eventInstance->messageHandle);		//ɾ����Ϣ��

		HandlerResult(strMsgID, result);
	}
	m_bBusy = true;
}

void CIotDataSender::ConnectStatusCallback(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* userContextCallback)
{
	if (result == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED)		//��Ȩ  ���ӳɹ�
	{
		m_bIotConnectSuccess = true;
	}
	else        //δ��Ȩ		�ɹ�
	{
		m_bIotConnectSuccess = false;
		if (reason == IOTHUB_CLIENT_CONNECTION_EXPIRED_SAS_TOKEN)
		{
			PrintLog(L"ERROR: IotDataSender Connect Fail(IOTHUB_CLIENT_CONNECTION_EXPIRED_SAS_TOKEN)!\r\n");
		}
		else if (reason == IOTHUB_CLIENT_CONNECTION_DEVICE_DISABLED)
		{
			PrintLog(L"ERROR: IotDataSender Connect Fail(IOTHUB_CLIENT_CONNECTION_DEVICE_DISABLED)!\r\n");
		}
		else if (reason == IOTHUB_CLIENT_CONNECTION_BAD_CREDENTIAL)
		{
			PrintLog(L"ERROR: IotDataSender Connect Fail(IOTHUB_CLIENT_CONNECTION_BAD_CREDENTIAL)!\r\n");
		}
		else if (reason == IOTHUB_CLIENT_CONNECTION_RETRY_EXPIRED)
		{
			PrintLog(L"ERROR: IotDataSender Connect Fail(IOTHUB_CLIENT_CONNECTION_RETRY_EXPIRED)!\r\n");
		}
		else if (reason == IOTHUB_CLIENT_CONNECTION_NO_NETWORK)
		{
			PrintLog(L"ERROR: IotDataSender Connect Fail(IOTHUB_CLIENT_CONNECTION_NO_NETWORK)!\r\n");
		}
		else if (reason == IOTHUB_CLIENT_CONNECTION_OK)
		{
			PrintLog(L"ERROR: IotDataSender Connect Fail(IOTHUB_CLIENT_CONNECTION_OK)!\r\n");
		}
	}
}

void CIotDataSender::StringReplace(string&s1, const string&s2, const string&s3)
{
	string::size_type pos = 0;
	string::size_type a = s2.size();
	string::size_type b = s3.size();
	while ((pos = s1.find(s2, pos)) != string::npos)
	{
		s1.replace(pos, a, s3);
		pos += b;
	}
}
