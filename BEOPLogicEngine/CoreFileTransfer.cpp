#include "StdAfx.h"
#include "CoreFileTransfer.h"
#include "../Tools/CustomTools/CustomTools.h"
#include "../Tools/Util/UtilsFile.h"
#include "../ServerDataAccess/BEOPDataAccess.h"
#include "../BEOPDataPoint/sqlite/SqliteAcess.h"
#include "sstream"


CCoreFileTransfer::CCoreFileTransfer(CBEOPDataAccess *pAccess, int nPort)
	:m_nPort(nPort)
	,m_pDataAccess(pAccess)
{
	m_bIsTransmitting = FALSE;
	m_bIsStop = FALSE;

	m_hThreadListen = NULL;
	m_hThreadSend = NULL;

	m_bExitThread = false;

	m_bNewFileRecved = false;
	m_bFileSent = false;

	m_bRecvS3DB = true;
}



CCoreFileTransfer::~CCoreFileTransfer(void)
{
}


void CCoreFileTransfer::PrintLog(const wstring &strLog)
{
	_tprintf(strLog.c_str());
	
	if(m_pDataAccess)
		m_pDataAccess->InsertLog(strLog);
}


void CCoreFileTransfer::SendFile(CSocket &senSo)
{
	m_bIsTransmitting = TRUE;

	//��Ҫ���͵��ļ�
	CFile file;
	if(!file.Open(m_strPath, CFile::modeRead | CFile::typeBinary))
	{
		PrintLog(L"ERROR: CCoreFileTransfer::SendFile Failed.");

		senSo.Close();

		return ;
	}


	int nSize = 0, nLen = 0;
	DWORD dwCount = 0;
	char buf[BLOCKSIZE] = {0};
	file.Seek(0, CFile::begin);
	//��ʼ�����ļ�
	for(;;)
	{
		//һ�ζ�ȡBLOCKSIZE��С���ļ�����
		nLen = file.Read(buf, BLOCKSIZE);
		if(nLen == 0)
			break;

		//�����ļ�����
		nSize = senSo.Send(buf, nLen);

		dwCount += nSize;
		CString strTransfersSize;
		strTransfersSize.Format(L"%ld BYTE", dwCount);


		if(m_bIsStop)
		{
			m_bIsStop = FALSE;
			break;
		}

		if(nSize == SOCKET_ERROR)
			break;
	}
	m_pDataAccess->WriteCoreDebugItemValue(L"filerecved", L"finish");

	//�ر��ļ�
	file.Close();
	//�ر��׽���
	senSo.Close();

	m_bIsTransmitting = FALSE;	
	m_bFileSent = true;
}


bool CCoreFileTransfer::ReceiveFile(CSocket &recSo)//�ļ����մ���
{
	m_bIsTransmitting = TRUE;
	PrintLog(L"Starting DB File Download...\r\n");
	int nSize = 0;
	DWORD dwCount = 0;
	char buf[BLOCKSIZE] = {0};	

	wstring strDir;
	Project::Tools::GetSysPath(strDir);
	
	CString strNewFileName, strNewFilePath;
	if(m_bRecvS3DB)
	{
		COleDateTime tnow = COleDateTime::GetCurrentTime();
		strNewFileName.Format(L"BEOPDATA%s.s3db",tnow.Format(L"%Y%m%d%H%M%S"));
		strNewFilePath.Format(L"%s\\DBFileVersion\\%s",strDir.c_str(), strNewFileName);

	}
	else
		strNewFilePath =m_strPath;

	CFile file(strNewFilePath, CFile::modeCreate|CFile::modeWrite);
	//��ʼ�����ļ�
	CString strTemp;
	bool bfailed = false;
	for(int i = 0;;i++)
	{
		//ÿ�ν���BLOCKSIZE��С���ļ�����
		nSize = recSo.Receive(buf, BLOCKSIZE);
		if(nSize == 0)
			break;

		if(dwCount>1024*256*1000 || i>=20000)
		{
			bfailed = true;
			break;
		}
		//�����յ����ļ�д���½����ļ���ȥ
		file.Write(buf, nSize);
		dwCount += nSize;

		//�û��Ƿ�Ҫֹͣ����
		if(m_bIsStop)
		{
			bfailed = true;
			m_bIsStop = FALSE;
			break;
		}
		
	//	strTemp.Format(L"���յ�%d�ֽ�\r\n", dwCount);

		if(i%10==0 && m_pDataAccess)
			m_pDataAccess->WriteCoreDebugItemValue(L"filerecved", to_wstring((long long)dwCount));

	}
	//�ر��ļ�
	file.Close(); 
	//�ر��׽���
	recSo.Close();


	if(!bfailed)
	{
		if(TestDBFile(strNewFilePath.GetString()))
		{
			strTemp.Format(L"Download Success(File Size:%.2f MB)\r\n", dwCount/1000.f/1000.f);
			PrintLog(strTemp.GetString());
			//�ļ�����
			CString strSourcePath = strNewFilePath;
			CString strDisPath;
			strDisPath.Format(L"%s\\%s",strDir.c_str(), strNewFileName);

			
			if(CopyFile(strSourcePath.GetString(),strDisPath.GetString(),FALSE))
			{
				PrintLog(L"DB File Copy Success...\r\n");
			}
			else
			{
				PrintLog(L"DB File Copy Failed ERROR!!!\r\n");

			}
			Sleep(3000);

			m_bIsTransmitting = FALSE;	
			m_bNewFileRecved = true;
			return true;
		}
		else
		{
			PrintLog(L"Download Failed (File Check failed) ERROR!!!\r\n");
			m_bIsTransmitting = FALSE;
			m_bNewFileRecved = false;
		}
	}
	else
	{
		PrintLog(L"Download Failed (Timeout)\r\n");
		m_bIsTransmitting = FALSE;
		m_bNewFileRecved = false;
	}

	return false;

}

bool CCoreFileTransfer::Exit()
{
	m_bExitThread = true;
	
	::WaitForSingleObject(m_hThreadListen, INFINITE);

	return true;
}

void CCoreFileTransfer::StartAsSender(CString strIPConnectTo, int nPortConnectTo)
{
	m_strIPConnectTo = strIPConnectTo;
	m_nPortConnectTo = nPortConnectTo;
	m_hThreadListen = CreateThread(NULL, 0, _SendThread, this, 0, NULL); 
}


void CCoreFileTransfer::StartAsReceiver()
{
	m_hThreadSend = CreateThread(NULL, 0, _ListenThread, this, 0, NULL); 
}


//���������ļ��߳�
DWORD WINAPI CCoreFileTransfer::_ListenThread( LPVOID lpParameter )
{
	CCoreFileTransfer *pTransfer = (CCoreFileTransfer *) lpParameter;
	//�����׽���
	while(!pTransfer->m_bExitThread)
	{

		CSocket sockSrvr;
		if(!sockSrvr.Create(pTransfer->m_nPort))
		{
			return -1;
		}

		//��ʼ����
		if(!sockSrvr.Listen())
		{
			return -1;
		}

		//��������
		CSocket recSo;
		if(!sockSrvr.Accept(recSo))
		{
			return -1;
		}

		sockSrvr.Close();

		if(pTransfer->ReceiveFile(recSo))
			return 0;

		Sleep(2000);

	}


	return 0;
}


DWORD WINAPI CCoreFileTransfer::_SendThread( LPVOID lpParameter )
{
	CCoreFileTransfer *pTransfer = (CCoreFileTransfer *) lpParameter;

	//�����ͻ����׽���
	CSocket sockClient;
	if(!sockClient.Create())
	{
		return -1;
	}
	//���������������

	if(!sockClient.Connect(pTransfer->m_strIPConnectTo, pTransfer->m_nPortConnectTo))
	{
		return -1;
	}

	pTransfer->SendFile(sockClient);

	return 0;
}


bool CCoreFileTransfer::TestDBFile(wstring strFilePath)
{

	string strUtf8;
	Project::Tools::WideCharToUtf8(strFilePath, strUtf8);
	CSqliteAcess access(strUtf8);

	ostringstream sqlstream;
	sqlstream << "select * from project_config;";
	string sql_ = sqlstream.str();
	char *ex_sql = const_cast<char*>(sql_.c_str());
	int re = access.SqlQuery(ex_sql);

	bool bReadOK = true;
	if (re != SQLITE_OK)
	{
		bReadOK = false;
	}

	if(SQLITE_ROW != access.SqlNext())
	{
		bReadOK = false;
	}
	if(access.getColumn_Text(6) != NULL)
	{
		bReadOK = true;
	}


	
	access.SqlFinalize();
	access.CloseDataBase();

	return bReadOK;
}
