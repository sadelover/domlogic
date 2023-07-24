#pragma once
#include "Tools/CommonDefine.h"
#include "Tools/CustomTools.h"
#include "Tools/db/BEOPDataBaseInfo.h"
#include "Tools/db/DataHandler.h"
#include "Tools/UtilsFile.h"
#include "BEOPDTUManager.h"

class CBEOPDTUEngineWrapper
{
public:
	CBEOPDTUEngineWrapper();
	~CBEOPDTUEngineWrapper();

public:
	bool	Run();									//�����к���
	bool	Init();									//��ʼ������
	bool	Exit();									//���˳�����
	bool	Release();								//�˳��ͷŲ���
	void	PrintLog(const wstring &strLog);		//��ӡ��ʾlog
	
private:

	bool	DeleteBackupFolderByDate();				//ɾ�������ļ�
	bool	WriteActiveTime();						//���߻ʱ��
	
	bool	FindAndDeleteBackupDirectory(CString strDir,CString strDeadFolder);
	bool	FindAndDeleteErrFile(CString strDir,CString strDeadFileName);
	bool	FindAndDeleteErrFile(CString strDir,CString strFilterName,CString strFilterPri,CString strDeadFileName);
	bool	DelteFoder(CString strDir);
	bool	DelteFile(CString strFilePath);						//ɾ���ļ����ⱸ��
	
public:
	wstring				m_strDBFilePath;			//���·��
	bool				m_bExitEngine;				//�˳����

	CBEOPDTUManager*	m_pBEOPDTUManager;			//DTU������
	CDataHandler*		m_pDataAccess;				//���ݿ����Ӿ��
	COleDateTime		m_oleLastDleteTime;			//��һ������ļ�ʱ��
	

};

