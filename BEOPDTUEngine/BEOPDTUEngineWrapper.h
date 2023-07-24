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
	bool	Run();									//主运行函数
	bool	Init();									//初始化函数
	bool	Exit();									//主退出函数
	bool	Release();								//退出释放操作
	void	PrintLog(const wstring &strLog);		//打印显示log
	
private:

	bool	DeleteBackupFolderByDate();				//删除过期文件
	bool	WriteActiveTime();						//在线活动时间
	
	bool	FindAndDeleteBackupDirectory(CString strDir,CString strDeadFolder);
	bool	FindAndDeleteErrFile(CString strDir,CString strDeadFileName);
	bool	FindAndDeleteErrFile(CString strDir,CString strFilterName,CString strFilterPri,CString strDeadFileName);
	bool	DelteFoder(CString strDir);
	bool	DelteFile(CString strFilePath);						//删除文件另外备份
	
public:
	wstring				m_strDBFilePath;			//点表路径
	bool				m_bExitEngine;				//退出标记

	CBEOPDTUManager*	m_pBEOPDTUManager;			//DTU处理类
	CDataHandler*		m_pDataAccess;				//数据库连接句柄
	COleDateTime		m_oleLastDleteTime;			//上一次清除文件时间
	

};

