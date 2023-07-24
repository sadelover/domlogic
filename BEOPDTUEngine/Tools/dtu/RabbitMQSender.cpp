#include "stdafx.h"
#include "RabbitMQSender.h"
#include "PackageTypeDefine.h"


CRabbitMQSender::CRabbitMQSender(string strRabbitMQHost/*=""*/, string strRabbitMQUser /*= ""*/, string strRabbitMQPsw /*= ""*/, string strRabbitMQSend /*= ""*/, string strRabbitMQReceive /*= ""*/, bool bRabbitMQSSL/*=false*/, CDataHandler* pDataHandler/*=NULL*/)
	: m_pSocketRabbitMQ(NULL)
	, m_conRabbitMQ(NULL)
	, m_nRabbitMQStatus(0)
	, m_nRabbitMQPort(5672)
	, m_nConnectTimeOut(60)
	, m_strRabbitMQHost(strRabbitMQHost)
	, m_strRabbitMQUser(strRabbitMQUser)
	, m_strRabbitMQPsw(strRabbitMQPsw)
	, m_strRabbitMQSend(strRabbitMQSend)
	, m_strRabbitMQReveive(strRabbitMQReceive)
	, m_strRabbitBindKey("")
	, m_hRabbitReceiveHandler(NULL)
	, m_bExitThread(false)
	, m_lpRecDataProc(NULL)
	, m_dwUserData(0)
	, m_pDataHandler(pDataHandler)
	, m_nRabbitMQErrorCount(0)
	, m_bBusy(false)
	, m_cFileBuffer(NULL)
	, m_nFileBuffer(0)
	, m_nCurrentFileBuffer(0)
	, m_strFileName("")
	, m_nChannelSend(2)
	, m_nChannelReceive(3)
	, m_strRabbitDirect("amq.direct")
	, m_strRabbitDirectType("direct")
	, m_nRabbitHeartbeat(0)
	, m_nRabbitDurable(1)
	, m_nRabbitNoAck(0)
{
	m_oleRabbitMQUpdateTime = COleDateTime::GetCurrentTime();
}

CRabbitMQSender::~CRabbitMQSender()
{
}

bool CRabbitMQSender::Init(LPRecDataProc proc1, DWORD userdata)
{
	m_bExitThread = false;
	m_bConnectSuccess = false;
	m_dwUserData = userdata;
	m_lpRecDataProc = proc1;

	//
	CString strDebugInfo;
	strDebugInfo.Format(_T("INFO: Init RabbitMQSender(Host:%s, Send:%s, Reveive:%s).\r\n"), Project::Tools::AnsiToWideChar(m_strRabbitMQHost.c_str()).c_str(), Project::Tools::AnsiToWideChar(m_strRabbitMQSend.c_str()).c_str(), Project::Tools::AnsiToWideChar(m_strRabbitMQReveive.c_str()).c_str());
	PrintLog(strDebugInfo.GetString());
	return true;
}

bool CRabbitMQSender::Exit()
{
	DisConnect();
	return true;
}

bool CRabbitMQSender::ReConnect()
{
	//�Լ��ж��費��Ҫ����
	COleDateTimeSpan oleSpan = COleDateTime::GetCurrentTime() - m_oleRabbitMQUpdateTime;
	if (m_bConnectSuccess == false || m_nRabbitMQErrorCount >= 5 || oleSpan.GetTotalMinutes() >= 30)			//��������ﵽ5�� ����30������δ�ɹ��շ���һ����Ҫ����
	{
		PrintLog(L"INFO: RabbitMQSender Reconnecting...\r\n");
		DisConnect();
		if (RabbitMQConnect())
		{
			PrintLog(L"INFO: RabbitMQSender Connect Success.\r\n");
			return true;
		}
		else
		{
			PrintLog(L"ERROR: RabbitMQSender Connect Fail.\r\n");
			return false;
		}
	}
	return false;
}

bool CRabbitMQSender::SendOneData(DTU_DATA_INFO & data)
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
				string strCmdID = "";
				if (!arrayObj["id"].isNull() && arrayObj["id"].isInt())
				{
					nPointCount = (DTU_CMD_TYPE)arrayObj["id"].asInt();
				}
				if (!arrayObj["type"].isNull() && arrayObj["type"].isInt())
				{
					type = (DTU_CMD_TYPE)arrayObj["type"].asInt();
				}

				if (!arrayObj["cmdid"].isNull() && arrayObj["cmdid"].isString())
				{
					strCmdID = arrayObj["cmdid"].asString();
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

				bSendResult = SendFile(strFilePath, type, data, strCmdID);
				GenerateLog(type, strFilePath, bSendResult, strReamrk);
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
				string strCmdID = "";
				if (!arrayObj["type"].isNull() && arrayObj["type"].isInt())
				{
					type = (DTU_CMD_TYPE)arrayObj["type"].asInt();
				}

				if (!arrayObj["cmdid"].isNull() && arrayObj["cmdid"].isString())
				{
					strCmdID = arrayObj["cmdid"].asString();
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
				bSendResult = SendString(strStrungContent, type, strReamrk, data, strCmdID);
				GenerateLog(type, strStrungContent, bSendResult, strReamrk);

			}
		}
	}
	return bSendResult;
}

bool CRabbitMQSender::IsConnected()
{
	if (m_conRabbitMQ && m_bConnectSuccess)
	{
		amqp_rpc_reply_t rp = amqp_get_rpc_reply(m_conRabbitMQ);
		if (rp.reply_type != AMQP_RESPONSE_NORMAL)
		{
			m_bConnectSuccess = false;
			return m_bConnectSuccess;
		}

		timeval* tv =  amqp_get_rpc_timeout(m_conRabbitMQ);

		if (m_bBusy == false)		//æµ״̬
		{
			COleDateTimeSpan oleSpan = COleDateTime::GetCurrentTime() - m_oleSendTime;
			if (oleSpan.GetTotalSeconds() >= RABBIT_SEND_TIME_OUT)			//æµ����ʱ��  �ж�Ϊ����ȷ�ϳ�ʱ ���»ָ�����
			{
				m_bBusy = true;
			}
		}
		return m_bBusy;
	}
	return false;
}

bool CRabbitMQSender::IsBusy()
{
	return m_bBusy;
}

bool CRabbitMQSender::SetConnectStatus(bool bConnectStatus)
{
	m_bConnectSuccess = bConnectStatus;
	return true;
}

bool CRabbitMQSender::DisConnect()
{
	m_bConnectSuccess = false;
	m_bExitThread = true;

	if (m_hRabbitReceiveHandler)
	{
		WaitForSingleObject(m_hRabbitReceiveHandler, 2000);
		CloseHandle(m_hRabbitReceiveHandler);
		m_hRabbitReceiveHandler = NULL;
	}

	if (m_conRabbitMQ)
	{
		//die_on_amqp_error(amqp_connection_close(m_conRabbitMQ, AMQP_REPLY_SUCCESS), "ERROR: RabbitMQSender Closing Connection");
		die_on_error(amqp_destroy_connection(m_conRabbitMQ), "ERROR: RabbitMQSender Destory Connection");
		m_conRabbitMQ = NULL;
		m_pSocketRabbitMQ = NULL;
		return true;
	}

	m_conRabbitMQ = NULL;
	m_pSocketRabbitMQ = NULL;

	return true;
}

bool CRabbitMQSender::RabbitMQConnect()
{
	m_bConnectSuccess = false;
	m_bExitThread = true;

	if(m_conRabbitMQ == NULL)
		m_conRabbitMQ = amqp_new_connection();

	if (m_conRabbitMQ)
	{
		if(m_pSocketRabbitMQ == NULL)
			m_pSocketRabbitMQ = amqp_tcp_socket_new(m_conRabbitMQ);			////����SSL/TLS socket

		if (m_pSocketRabbitMQ)
		{
			//�������ӳ�ʱ		
			struct timeval tval;
			struct timeval *tv;
			if (m_nConnectTimeOut > 0)
			{
				tv = &tval;

				tv->tv_sec = m_nConnectTimeOut;
				tv->tv_usec = 0;
			}
			else
			{
				tv = NULL;
			}

			//����
			if(die_on_error(amqp_socket_open_noblock(m_pSocketRabbitMQ, m_strRabbitMQHost.c_str(), m_nRabbitMQPort, tv),"ERROR: RabbitMQSender Open Scoket"))
			{
				//�û���¼
				if (die_on_amqp_error(amqp_login(m_conRabbitMQ, "/", 0, 131072, m_nRabbitHeartbeat, AMQP_SASL_METHOD_PLAIN, m_strRabbitMQUser.c_str(), m_strRabbitMQPsw.c_str()), "ERROR: RabbitMQSender Login"))
				{
					bool bOpenAndSetSendSuccess = false;
					bool bOpenAndSetReceiveSuccess = false;

					//����RPC��ʱ
					die_on_error(amqp_set_rpc_timeout(m_conRabbitMQ, tv),"ERROR: RabbitMQSender Set RPC TimeOut");

					//�򿪲����÷���ͨ��
					amqp_channel_open(m_conRabbitMQ, m_nChannelSend);
					if (die_on_amqp_error(amqp_get_rpc_reply(m_conRabbitMQ), "ERROR: RabbitMQSender Opening Send Channel"))
					{
						//��Ϣ����������ָ����Ϣ��ʲô����·�ɵ��ĸ����С�
						amqp_exchange_declare(m_conRabbitMQ, m_nChannelSend, amqp_cstring_bytes(m_strRabbitDirect.c_str()), amqp_cstring_bytes(m_strRabbitDirectType.c_str()), 0, m_nRabbitDurable, 0, 0, amqp_empty_table);
						if (die_on_amqp_error(amqp_get_rpc_reply(m_conRabbitMQ), "ERROR: RabbitMQSender Declaring Send Exchange"))
						{
							//����һ������
							amqp_queue_declare(m_conRabbitMQ, m_nChannelSend, amqp_cstring_bytes(m_strRabbitMQSend.c_str()), 0, m_nRabbitDurable, 0, 0, amqp_empty_table);
							if (die_on_amqp_error(amqp_get_rpc_reply(m_conRabbitMQ), "ERROR: RabbitMQSender Declaring Send Queue"))
							{
								//�󶨶���
								amqp_queue_bind(m_conRabbitMQ, m_nChannelSend, amqp_cstring_bytes(m_strRabbitMQSend.c_str()), amqp_cstring_bytes(m_strRabbitDirect.c_str()), amqp_cstring_bytes(m_strRabbitMQSend.c_str()), amqp_empty_table);
								if (die_on_amqp_error(amqp_get_rpc_reply(m_conRabbitMQ), "ERROR: RabbitMQSender  Binding Send Queue"))
								{
									bOpenAndSetSendSuccess = true;
								}

							}
						}
					}

					//�򿪲����ý���ͨ��
					if (bOpenAndSetSendSuccess)
					{
						//�򿪲����ý���ͨ��
						amqp_channel_open(m_conRabbitMQ, m_nChannelReceive);
						if(die_on_amqp_error(amqp_get_rpc_reply(m_conRabbitMQ), "ERROR: RabbitMQSender Opening Receive Channel"))
						{ 
							//����һ������
							amqp_queue_declare(m_conRabbitMQ, m_nChannelReceive, amqp_cstring_bytes(m_strRabbitMQReveive.c_str()), 0, m_nRabbitDurable, 0, 0, amqp_empty_table);
							if(die_on_amqp_error(amqp_get_rpc_reply(m_conRabbitMQ), "ERROR: RabbitMQSender Declaring Receive Queue"))
							{
								//�󶨶���
								amqp_queue_bind(m_conRabbitMQ, m_nChannelReceive, amqp_cstring_bytes(m_strRabbitMQReveive.c_str()), amqp_cstring_bytes(m_strRabbitDirect.c_str()), amqp_cstring_bytes(m_strRabbitMQReveive.c_str()), amqp_empty_table);
								if (die_on_amqp_error(amqp_get_rpc_reply(m_conRabbitMQ), "ERROR: RabbitMQSender  Binding Receive Queue"))
								{
									//����Ԥȡ��ϢΪ1
									amqp_basic_qos(m_conRabbitMQ, m_nChannelReceive, 0, 1, 0);
									if (die_on_amqp_error(amqp_get_rpc_reply(m_conRabbitMQ), "ERROR: RabbitMQSender QOS"))
									{
										amqp_basic_consume(m_conRabbitMQ, m_nChannelReceive, amqp_cstring_bytes(m_strRabbitMQReveive.c_str()), amqp_cstring_bytes("beopgateware"), 0, m_nRabbitNoAck, 0, amqp_empty_table);
										if (die_on_amqp_error(amqp_get_rpc_reply(m_conRabbitMQ), "ERROR: RabbitMQSender Consuming"))
										{
											bOpenAndSetReceiveSuccess = true;
										}
									}
								}
							}
						
						}
					}

					if (bOpenAndSetReceiveSuccess && bOpenAndSetSendSuccess)
					{
						//���ӳɹ� ����������Ϣ
						m_bConnectSuccess = true;
						m_bExitThread = false;
						m_nRabbitMQErrorCount = 0;
						m_oleRabbitMQUpdateTime = COleDateTime::GetCurrentTime();

						if (m_hRabbitReceiveHandler == NULL)
							m_hRabbitReceiveHandler = (HANDLE)_beginthreadex(NULL, 0, RecvThread, this, 0, NULL);

						return m_bConnectSuccess;
					}
				}
			}
			else      //��socketʧ��
			{
				die_on_error(amqp_destroy_connection(m_conRabbitMQ), "ERROR: RabbitMQSender Destory Connection");
				m_conRabbitMQ = NULL;
				m_pSocketRabbitMQ = NULL;
			}
		}
	}
	return m_bConnectSuccess;
}

bool CRabbitMQSender::SendString(string strContent, DTU_CMD_TYPE nCmdType, string strRemark, DTU_DATA_INFO & data, string strCmdID)
{
	bool bSendScuess = false;
	if (m_bConnectSuccess)
	{
		if (m_conRabbitMQ)
		{
			CString strLog, strDataID;
			char* pBuffer = NULL;
			int nContentLength = strContent.length();
			if (nContentLength > 0)
			{
				strDataID = Project::Tools::AnsiToWideChar(data.strID.c_str()).c_str();			//��Ϣ���

				pBuffer = (char*)malloc(sizeof(char)*nContentLength);
				if (pBuffer)
				{
					memset(pBuffer, 0, nContentLength);
					memcpy(pBuffer, strContent.c_str(), nContentLength);

					//��Ϣ����
					amqp_basic_properties_t props;
					props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG |AMQP_BASIC_DELIVERY_MODE_FLAG |AMQP_BASIC_HEADERS_FLAG ;
					props.content_type = amqp_cstring_bytes("text/plain");
					props.delivery_mode = 2;
					
					amqp_table_entry_t amqpEntries[5];

					//type  0:�ı�����
					amqpEntries[0].key = amqp_cstring_bytes("type");
					amqpEntries[0].value.kind = AMQP_FIELD_KIND_I32;
					amqpEntries[0].value.value.i32 = 0;

					//��������
					amqpEntries[1].key = amqp_cstring_bytes("cmdtype");
					amqpEntries[1].value.kind = AMQP_FIELD_KIND_I32;
					amqpEntries[1].value.value.i32 = nCmdType;

					//����С
					amqpEntries[2].key = amqp_cstring_bytes("size");
					amqpEntries[2].value.kind = AMQP_FIELD_KIND_I32;
					amqpEntries[2].value.value.i32 = nContentLength;

					//dtuname
					amqpEntries[3].key = amqp_cstring_bytes("dtuname");
					amqpEntries[3].value.kind = AMQP_FIELD_KIND_UTF8;
					amqpEntries[3].value.value.bytes = amqp_cstring_bytes(m_strRabbitMQReveive.c_str());

					//cmdid
					amqpEntries[4].key = amqp_cstring_bytes("cmdid");
					amqpEntries[4].value.kind = AMQP_FIELD_KIND_UTF8;
					amqpEntries[4].value.value.bytes = amqp_cstring_bytes(strCmdID.c_str());

					props.headers.num_entries = 5;
					props.headers.entries = amqpEntries;

					amqp_bytes_t message_bytes;
					message_bytes.len = nContentLength;
					message_bytes.bytes = pBuffer;
					if (m_bConnectSuccess && die_on_error(amqp_basic_publish(m_conRabbitMQ, m_nChannelSend, amqp_cstring_bytes(m_strRabbitDirect.c_str()), amqp_cstring_bytes(m_strRabbitMQSend.c_str()), 0, 0, &props, message_bytes), "ERROR: RabbitMQSender Publishing"))
					{
						m_oleRabbitMQUpdateTime = COleDateTime::GetCurrentTime();
						bSendScuess = true;
					}
					
					//�ͷ�buffer
					free(pBuffer);
					pBuffer = NULL;
				}
			}
		}
	}
	return bSendScuess;
}

bool CRabbitMQSender::SendFile(string strFilePath, DTU_CMD_TYPE nCmdType, DTU_DATA_INFO & data, string strCmdID)
{
	if (m_bConnectSuccess)
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
					strLog.Format(_T("ERROR: RabbitMQSender SendFile Fail(%s)!\r\n"), Project::Tools::AnsiToWideChar(data.strFileName.c_str()).c_str());
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
						char* cFile = new char[RABBIT_ONE_FILE_SIZE];
						nLastLen = gzread(gz, cFile, RABBIT_ONE_FILE_SIZE);
						vecFile.push_back(cFile);
						if (nLastLen < RABBIT_ONE_FILE_SIZE)
							break;
					}
					//�ر��ļ�
					gzclose(gz);

					//����
					m_bBusy = false;
					bool bResult = true;
					int nType = 2;
					int nCount = vecFile.size();
					if (nCount > 1)								//��Ҫ�ְ�����
						nType = 3;

					for (int i = 0;i < nCount; ++i)
					{
						int nLength = RABBIT_ONE_FILE_SIZE;

						//��Ϣ����
						amqp_basic_properties_t props;
						props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG | AMQP_BASIC_HEADERS_FLAG;
						props.content_type = amqp_cstring_bytes("application/octet-stream");
						props.delivery_mode = 2;

						amqp_table_entry_t amqpEntries[7];

						//type  0:�ı�����  1:Json�ı� 2:ѹ���ļ���zip��-���ְ�  3��ѹ���ļ���zip��-�ְ�  (�ر�)
						amqpEntries[0].key = amqp_cstring_bytes("type");
						amqpEntries[0].value.kind = AMQP_FIELD_KIND_I32;
						amqpEntries[0].value.value.i32 = nType;

						//��������
						amqpEntries[1].key = amqp_cstring_bytes("cmdtype");
						amqpEntries[1].value.kind = AMQP_FIELD_KIND_I32;
						amqpEntries[1].value.value.i32 = nCmdType;

						//����С
						DWORD dwFileSize = 0;
						if (i == nCount - 1)
						{
							dwFileSize = nLastLen;
						}
						else
						{
							dwFileSize = RABBIT_ONE_FILE_SIZE;
						}
						amqpEntries[2].key = amqp_cstring_bytes("size");
						amqpEntries[2].value.kind = AMQP_FIELD_KIND_I32;
						amqpEntries[2].value.value.i32 = dwFileSize;

						//name
						amqpEntries[3].key = amqp_cstring_bytes("name");
						amqpEntries[3].value.kind = AMQP_FIELD_KIND_UTF8;
						amqpEntries[3].value.value.bytes = amqp_cstring_bytes(data.strFileName.c_str());

						//index
						std::ostringstream sqlstream;
						sqlstream << i + 1 << "/" << nCount;
						amqpEntries[4].key = amqp_cstring_bytes("index");
						amqpEntries[4].value.kind = AMQP_FIELD_KIND_UTF8;
						amqpEntries[4].value.value.bytes = amqp_cstring_bytes(sqlstream.str().c_str());

						//dtuname
						amqpEntries[5].key = amqp_cstring_bytes("dtuname");
						amqpEntries[5].value.kind = AMQP_FIELD_KIND_UTF8;
						amqpEntries[5].value.value.bytes = amqp_cstring_bytes(m_strRabbitMQReveive.c_str());

						//cmdid
						amqpEntries[6].key = amqp_cstring_bytes("cmdid");
						amqpEntries[6].value.kind = AMQP_FIELD_KIND_UTF8;
						amqpEntries[6].value.value.bytes = amqp_cstring_bytes(strCmdID.c_str());

						props.headers.num_entries = 7;
						props.headers.entries = amqpEntries;

						amqp_bytes_t message_bytes;
						message_bytes.len = dwFileSize;
						message_bytes.bytes = vecFile[i];

						//����
						m_oleSendTime = COleDateTime::GetCurrentTime();
						if (m_bConnectSuccess && bResult && die_on_error(amqp_basic_publish(m_conRabbitMQ, m_nChannelSend, amqp_cstring_bytes(m_strRabbitDirect.c_str()), amqp_cstring_bytes(m_strRabbitMQSend.c_str()), 0, 0, &props, message_bytes), "ERROR: RabbitMQSender Publishing"))
						{
							m_oleRabbitMQUpdateTime = COleDateTime::GetCurrentTime();
							bResult = true;
						}
						else
						{
							m_bConnectSuccess = false;
							bResult = false;
						}
					}
					m_bBusy = true;
					return bResult;
				}
			}
			else
			{
				strLog.Format(_T("ERROR: RabbitMQSender SendFile Fail(%s)!\r\n"), Project::Tools::AnsiToWideChar(data.strFileName.c_str()).c_str());
				PrintLog(strLog.GetString());
			}
		}
		else
		{
			strLog.Format(_T("ERROR: RabbitMQSender SendFile Fail(%s)!\r\n"), Project::Tools::AnsiToWideChar(data.strFileName.c_str()).c_str());
			PrintLog(strLog.GetString());
		}
	}
	return false;
}

UINT WINAPI CRabbitMQSender::RecvThread(LPVOID lpVoid)
{
	CRabbitMQSender* pRabbitMQSender = reinterpret_cast<CRabbitMQSender*>(lpVoid);
	if (NULL == pRabbitMQSender)
	{
		return 0;
	}

	pRabbitMQSender->RecvRabbitMQ();
	return 1;
}

bool CRabbitMQSender::RecvRabbitMQ()
{
	while(!m_bExitThread && m_conRabbitMQ && m_bConnectSuccess)
	{
		amqp_rpc_reply_t ret;
		amqp_envelope_t envelope;
		
		amqp_maybe_release_buffers(m_conRabbitMQ);
		ret = amqp_consume_message(m_conRabbitMQ, &envelope, NULL, 0);		//����ģʽ
		if (m_bConnectSuccess)
		{
			if (die_on_amqp_error(ret, "ERROR: RabbitMQSender Consume Message"))
			{
				string strType = "";		//����
				string strCmdType = "";		//��������
				string strSize = "";		//����С
				string strCmdID = "";		//����ID
				string strFileName = "";	//�ļ���
				string strIndex = "";		//���
				
				for (int i = 0; i < envelope.message.properties.headers.num_entries; i++)
				{
					amqp_table_entry_t entries = envelope.message.properties.headers.entries[i];
					char cName[128] = { 0 };
					sprintf(cName, "%.*s", (int)entries.key.len, (char *)entries.key.bytes);
					char cValue[128] = { 0 };
					sprintf(cValue, "%.*s", (int)entries.value.value.bytes.len, (char *)entries.value.value.bytes.bytes);

					string strName = cName;
					string strValue = cValue;
					if (strName == "type")
					{
						strType = strValue;
					}
					else if (strName == "cmdtype")
					{
						strCmdType = strValue;
					}
					else if (strName == "size")
					{
						strSize = strValue;
					}
					else if (strName == "name")
					{
						strFileName = strValue;
					}
					else if (strName == "index")
					{
						strIndex = strValue;
					}
					else if (strName == "cmdid")
					{
						strCmdID = strValue;
					}
				}

				//������Ϣ
				m_oleRabbitMQUpdateTime = COleDateTime::GetCurrentTime();
				HandlerReceive(envelope.message, strType, strCmdType, strSize, strCmdID, strFileName, strIndex);

				//ɾ����Ϣ
				amqp_destroy_envelope(&envelope);

				//ȷ����Ϣ
				if (m_nRabbitNoAck == 0)
					die_on_error(amqp_basic_ack(m_conRabbitMQ, m_nChannelReceive, envelope.delivery_tag, 1), "ERROR: RabbitMQSender ACK");
			}
			else
			{
				//�˳�
				m_bConnectSuccess = false;
				return false;
			}
		}
		Sleep(100);
	}
	return true;
}

void CRabbitMQSender::HandlerReceive(amqp_message_t message, string strType, string strCmdType, string strSize, string strCmdID, string strFileName, string strIndex)
{
	if (message.body.bytes != NULL && message.body.len > 0)
	{
		if (strType == "0")					//0:�ı�����
		{
			HandlerText(message, strType, strCmdType, strSize, strCmdID, strFileName, strIndex);
		}
		else if (strType == "1")				//1:Json�ı�
		{

		}
		else if (strType == "2")			//2:ѹ���ļ���zip��-���ְ�
		{
			HandlerFile(message, strType, strCmdType, strSize, strCmdID, strFileName, strIndex);
		}
		else if (strType == "3")			//3��ѹ���ļ���zip��-�ְ�
		{
			HandlerFilePackage(message, strType, strCmdType, strSize, strCmdID, strFileName, strIndex);
		}
	}
}

void CRabbitMQSender::HandlerText(amqp_message_t message, string strType, string strCmdType, string strSize, string strCmdID, string strFileName, string strIndex)
{
	try
	{
		int nSize = ATOI(strSize.c_str());
		if (nSize > 13 && message.body.len == nSize)
		{
			rnbtech::CPackageType::DTU_DATATYPE type = rnbtech::CPackageType::GetPackageType((const char*)message.body.bytes);
			switch (type)
			{
				case rnbtech::CPackageType::Type_DTUServerCmd:
				{
					rnbtech::CPackageType::RemovePrefix((char*)message.body.bytes, nSize);
					char* pReveive = strtok((char*)message.body.bytes, ";");

					string strReveive = pReveive;
					//�������ռ�¼
					Json::Value jsonBody;
					jsonBody["type"] = 0;
					jsonBody["cmdtype"] = ATOI(strCmdType.c_str());
					jsonBody["cmdid"] = strCmdID;
					jsonBody["content"] = strReveive;
					string strContent = jsonBody.toStyledString();
					if (m_lpRecDataProc)
						m_lpRecDataProc((const unsigned char*)strContent.data(), strContent.size(), m_dwUserData);
				}
			}
		}
		else
		{
			PrintLog(L"ERROR: RabbitMQSender Receive Undefine Message.\r\n");
		}
	}
	catch (const std::exception&)
	{

	}
}

void CRabbitMQSender::HandlerFile(amqp_message_t message, string strType, string strCmdType, string strSize, string strCmdID, string strFileName, string strIndex)
{
	int nSize = ATOI(strSize.c_str());
	int nCmdType = ATOI(strCmdType.c_str());
	if (message.body.bytes != NULL && nSize > 0 && strFileName.length() > 0 && strIndex.length() > 0)
	{
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
			file.Write(message.body.bytes, nSize);
			file.Close();

			CString str;
			str.Format(_T("RECV: RabbitMQSender Receive Update File Package(%s)...\r\n"), Project::Tools::AnsiToWideChar(m_strFileName.c_str()).c_str());
			PrintLog(str.GetString());

			//�������ռ�¼
			string strRecevieFilePath = Project::Tools::WideCharToAnsi(strFilePath);
			Json::Value jsonBody;
			jsonBody["type"] = 1;    //�ļ�
			if (nCmdType == DTU_CMD_HISTORY_FILE_SYN || nCmdType == DTU_CMD_HISTORY_FILE_SEND)
			{
				jsonBody["type"] = 2;    //�ļ�
			}
			jsonBody["cmdtype"] = ATOI(strCmdType.c_str());
			jsonBody["cmdid"] = strCmdID;
			
			StringReplace(strRecevieFilePath, "\\", "/");				//ת��		
			jsonBody["content"] = strRecevieFilePath;
			string strContent = jsonBody.toStyledString();
			if (m_lpRecDataProc)
				m_lpRecDataProc((const unsigned char*)strContent.data(), strContent.size(), m_dwUserData);
		}
	}
}

void CRabbitMQSender::HandlerFilePackage(amqp_message_t message, string strType, string strCmdType, string strSize, string strCmdID, string strFileName, string strIndex)
{
	int nSize = ATOI(strSize.c_str());
	int nCmdType = ATOI(strCmdType.c_str());
	if (message.body.bytes != NULL && nSize > 0)
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
					m_nFileBuffer = nPacketCount * RABBIT_ONE_FILE_SIZE;
					m_cFileBuffer = new char[m_nFileBuffer];
					ZeroMemory(m_cFileBuffer, 0, m_nFileBuffer);
					m_strFileName = strFileName;
				}

				//�洢
				if (m_cFileBuffer && m_nCurrentFileBuffer + nSize <= m_nFileBuffer)
				{
					memcpy(m_cFileBuffer + m_nCurrentFileBuffer, message.body.bytes, nSize);
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
							str.Format(_T("RECV: RabbitMQSender Receive Update File Package(%s)...\r\n"), Project::Tools::AnsiToWideChar(m_strFileName.c_str()).c_str());
							PrintLog(str.GetString());

							//�������ռ�¼
							string strRecevieFilePath = Project::Tools::WideCharToAnsi(strFilePath);
							Json::Value jsonBody;
							jsonBody["type"] = 1;    //�ļ�
							if (nCmdType == DTU_CMD_HISTORY_FILE_SYN || nCmdType == DTU_CMD_HISTORY_FILE_SEND)
							{
								jsonBody["type"] = 2;    //�ļ�
							}
							jsonBody["cmdtype"] = ATOI(strCmdType.c_str());
							jsonBody["cmdid"] = strCmdID;
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

bool CRabbitMQSender::ClearFileBuffer()
{
	if (m_cFileBuffer)
	{
		delete[]m_cFileBuffer;
		m_cFileBuffer = NULL;
	}
	m_nFileBuffer = 0;
	m_nCurrentFileBuffer = 0;
	m_strFileName = "";
	return true;
}

void CRabbitMQSender::PrintLog(const wstring &strLog)
{
	_tprintf(strLog.c_str());
}

void CRabbitMQSender::StringReplace(string&s1, const string&s2, const string&s3)
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

bool CRabbitMQSender::GenerateLog(DTU_CMD_TYPE type, string strContent, bool bSendSuccess, string strRemark /*= ""*/)
{
	std::ostringstream sqlstream;
	if (bSendSuccess)
	{
		sqlstream << "SEND: ";
	}
	else
	{
		sqlstream << "ERROR: ";
	}
	switch (type)
	{
	case DTU_CMD_DEFINE:
		break;
	case DTU_CMD_REAL_DATA_SEND:
	case DTU_CMD_REAL_DATA_SYN:
	{
		sqlstream << "RealData(" << strContent.length() << ")" << "\r\n";
	}
		break;
	case DTU_CMD_REAL_DATA_EDIT:
	case DTU_CMD_REAL_DATA_EDIT_MUL:
	{
		sqlstream << "RealData Edit(" << strContent.length() << ")" << "\r\n";
	}
		break;
	case DTU_CMD_WARNING_DATA_SEND:
	case DTU_CMD_WARNING_DATA_SYN:
	{
		sqlstream << "WarningData(" << strContent.length() << ")" << "\r\n";
	}
	break;
	case DTU_CMD_WARNING_OPERATION_DATA_SEND:
	case DTU_CMD_WARNING_OPERATION_DATA_SYN:
	{
		sqlstream << "WarningOperationData(" << strContent.length() << ")" << "\r\n";
	}
		break;
	case DTU_CMD_OPERATION_SEND:
	case DTU_CMD_OPERATION_SYN:
	{
		sqlstream << "OperationData(" << strContent.length() << ")" << "\r\n";
	}
		break;
	case DTU_CMD_UNIT_SEND:
	case DTU_CMD_UNIT_SYN:
	{
		sqlstream << "Unit01(" << strContent.length() << ")" << "\r\n";
	}
	break;
	case DTU_CMD_UNIT_EDIT:
	case DTU_CMD_UNIT_EDIT_MUL:
	{
		sqlstream << "Unit01 Edit(" << strContent.length() << ")" << "\r\n";
	}
		break;
	case DTU_CMD_LOG_SEND:
	case DTU_CMD_LOG_SYN:
	{
		int pos = strContent.find_last_of('/');
		sqlstream << "MysqlFile(" << strContent.substr(pos + 1) << ")" << "\r\n";
	}
	break;
	case DTU_CMD_HEART_SEND:
	case DTU_CMD_HEART_SYN:
	{
		sqlstream << "HeartBeat(" << strContent.length() << ")" << "\r\n";
	}
		break;
	case DTU_CMD_ERR_LIST:
	{
		sqlstream << "ERROR List(" << strContent.length() << ")" << "\r\n";
	}
		break;
	case DTU_CMD_ERR_CODE:
	{
		sqlstream << "ERROR Code(" << strContent.length() << ")" << "\r\n";
	}
		break;
	case DTU_CMD_ERROR_SEND:;
	case DTU_CMD_ERROR_SYN:
	{
		sqlstream << "ERROR Log(" << strContent.length() << ")" << "\r\n";
	}
		break;
	case DTU_CMD_REAL_FILE_SEND:
	case DTU_CMD_REAL_FILE_SYN:
	{
		int pos = strContent.find_last_of('/');
		sqlstream << "RealFileData(" << strContent.substr(pos + 1) << ")" << "\r\n";
	}
	break;
	case DTU_CMD_HISTORY_FILE_SEND:
	case DTU_CMD_HISTORY_FILE_SYN:
	{
		int pos = strContent.find_last_of('/');
		sqlstream << "HisoryFileData(" << strContent.substr(pos + 1) << ")";
		if (strRemark.length() > 0)
		{
			sqlstream << "(" << strRemark << ")";
		}
		sqlstream << "\r\n";
	}
	break;
	case DTU_CMD_UPDATE_EXE:
	{
		sqlstream << "Update Exe(" << strContent << ")" << "\r\n";
	}
		break;
	case DTU_CMD_UPDATE_POINT_CSV:
	{
		sqlstream << "Update Point CSV(" << strContent << ")" << "\r\n";
	}
		break;
	case DTU_CMD_UPDATE_POINT_EXCEL:
	{
		sqlstream << "Update Point Excel(" << strContent << ")" << "\r\n";
	}
		break;
	case DTU_CMD_UPDATE_DLL:
	{
		sqlstream << "Update Point DLL(" << strContent << ")" << "\r\n";
	}
		break;
	case DTU_CMD_UPDATE_S3DB:
	{
		sqlstream << "Update Point S3db(" << strContent << ")" << "\r\n";
	}
		break;
	case DTU_CMD_UPLOAD_POINT_CSV:
	{
		sqlstream << "Upload Point CSV(" << strContent << ")" << "\r\n";
	}
		break;
	case DTU_CMD_DELETE_POINT_MUL:
	{
		sqlstream << "Delete Point(" << strContent.length() << ")" << "\r\n";
	}
		break;
	case DTU_CMD_TIME_SYN:
	{
		sqlstream << "SynTime(" << strContent << ")" << "\r\n";
	}
		break;
	case DTU_CMD_RESTART_CORE:
	{
		sqlstream << "Restart Core(" << strContent.length() << ")" << "\r\n";
	}
		break;
	case DTU_CMD_RESTART_DOG:
	{
		sqlstream << "Restart Dog(" << strContent.length() << ")" << "\r\n";
	}
		break;
	case DTU_CMD_RESTART_DTU:
	{
		sqlstream << "Restart DTU(" << strContent.length() << ")" << "\r\n";
	}
		break;
	case DTU_CMD_CORE_STATUS:
	{
		sqlstream << "Core Status(" << strContent << ")" << "\r\n";
	}
		break;
	case DTU_CMD_VERSION_CORE:
	case DTU_CMD_VERSION_DOG:
	case DTU_CMD_VERSION_DTU:
	{
		sqlstream << "Version(" << strContent.length() << ")" << "\r\n";
	}
	break;
	default:
	{
		sqlstream << "Undefine(" << strContent.length() << ")" << "\r\n";
	}
		break;
	}
	PrintLog(Project::Tools::AnsiToWideChar(sqlstream.str().c_str()));
	return false;
}

void CRabbitMQSender::die(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
	//exit(1);
}

bool CRabbitMQSender::die_on_error(int x, char const * context)
{
	if (x < 0) 
	{
		fprintf(stderr, "%s: %s.\r\n", context, amqp_error_string2(x));
		return false;
	}
	return true;
}

bool CRabbitMQSender::die_on_amqp_error(amqp_rpc_reply_t x, char const * context)
{
	switch (x.reply_type) 
	{
		case AMQP_RESPONSE_NORMAL:
			return true;

		case AMQP_RESPONSE_NONE:
			fprintf(stderr, "%s: missing RPC reply type!\n", context);
			return false;

		case AMQP_RESPONSE_LIBRARY_EXCEPTION:
			fprintf(stderr, "%s: %s\n", context, amqp_error_string2(x.library_error));
			return false;

		case AMQP_RESPONSE_SERVER_EXCEPTION:
			switch (x.reply.id)
			{
				case AMQP_CONNECTION_CLOSE_METHOD: 
				{
					amqp_connection_close_t *m = (amqp_connection_close_t *)x.reply.decoded;
					fprintf(stderr, "%s: server connection error %u, message: %.*s\n",
						context,
						m->reply_code,
						(int)m->reply_text.len, (char *)m->reply_text.bytes);
					return false;
				}
				case AMQP_CHANNEL_CLOSE_METHOD:
				{
					amqp_channel_close_t *m = (amqp_channel_close_t *)x.reply.decoded;
					fprintf(stderr, "%s: server channel error %u, message: %.*s\n",
						context,
						m->reply_code,
						(int)m->reply_text.len, (char *)m->reply_text.bytes);
					return false;
				}
				default:
					fprintf(stderr, "%s: unknown server error, method id 0x%08X\n", context, x.reply.id);
					return false;
			}
			return false;
	}
	return true;
}

void CRabbitMQSender::amqp_dump(void const * buffer, size_t len)
{
	unsigned char *buf = (unsigned char *)buffer;
	long count = 0;
	int numinrow = 0;
	int chs[16];
	int oldchs[16] = { 0 };
	int showed_dots = 0;
	size_t i;

	for (i = 0; i < len; i++) {
		int ch = buf[i];

		if (numinrow == 16) {
			int j;

			if (rows_eq(oldchs, chs)) {
				if (!showed_dots) {
					showed_dots = 1;
					printf("          .. .. .. .. .. .. .. .. : .. .. .. .. .. .. .. ..\n");
				}
			}
			else {
				showed_dots = 0;
				dump_row(count, numinrow, chs);
			}

			for (j = 0; j < 16; j++) {
				oldchs[j] = chs[j];
			}

			numinrow = 0;
		}

		count++;
		chs[numinrow++] = ch;
	}

	dump_row(count, numinrow, chs);

	if (numinrow != 0) {
		printf("%08lX:\n", count);
	}
}

void CRabbitMQSender::dump_row(long count, int numinrow, int *chs)
{
	int i;

	printf("%08lX:", count - numinrow);

	if (numinrow > 0) {
		for (i = 0; i < numinrow; i++) {
			if (i == 8) {
				printf(" :");
			}
			printf(" %02X", chs[i]);
		}
		for (i = numinrow; i < 16; i++) {
			if (i == 8) {
				printf(" :");
			}
			printf("   ");
		}
		printf("  ");
		for (i = 0; i < numinrow; i++) {
			if (isprint(chs[i])) {
				printf("%c", chs[i]);
			}
			else {
				printf(".");
			}
		}
	}
	printf("\n");
}

int CRabbitMQSender::rows_eq(int *a, int *b)
{
	int i;

	for (i = 0; i < 16; i++)
		if (a[i] != b[i]) {
			return 0;
		}

	return 1;
}
