#pragma once
#include "DataBaseSetting.h"
#include "DLLObject.h"
#include "ScheduleThread.h"

class CLogicBase;
class CLogicDllThread;

typedef std::vector<std::wstring>  vec_wstr;
typedef CLogicBase* (*pfnLogicBase)();



struct sDllStore
{
	string	nID;
	string	strDllName;
	string	strImportTime;
	string	strAuthor;
	string	strPeriodicity;
	string	strDllContent;
	string	strRunStatus;
	string unitproperty01;
	string	unitproperty02;
	string unitproperty03;
	string	unitproperty04;
	string unitproperty05;

};

struct sPointWatch
{
	int		nID;
	string	strName;
	string	strContent;

	string unitproperty01;
	string	unitproperty02;
	string unitproperty03;
	string	unitproperty04;
	string unitproperty05;
};

struct sParamConfig
{

	string	strVName;
	string	strPName;
	string	strPType;
	string	strVexplain;
	string	strDllName;
	string	nINorOut;

	string unitproperty01;
	string	unitproperty02;
	string unitproperty03;
	string	unitproperty04;
	string unitproperty05;
};



class  CBEOPLogicManager
{
public:
	CBEOPLogicManager(wstring strDBFileName,CWnd *pWnd, std::vector<wstring> wstrPrefixList, std::vector<wstring> wstrExceptPrefixList);
	~CBEOPLogicManager(void);

	bool ThreadExitFinish();

	bool StoreDllToDB(CDLLObject* dllOb, wstring strDllPath);
	bool    UpdateDLLParameter(CDLLObject* dllOb);
	bool    SaveInputParametertoDB(CString strDllName, std::vector<vec_str> &strLongPara);
	bool    SaveOutputParametertoDB(CString strDllName, std::vector<vec_str> &strLongPara);

	bool InsertDLLtoDB(CDLLObject* dllOb,char *pBlock,int nSize);
	bool ReadDLLfromDB(const wstring &s3dbpath,std::vector<CDLLObject*> &dllObList);
	bool UpdateDLLfromDB(const wstring &s3dbpath, CDLLObject *pDllOb);
	bool IfExistDll(const wstring &s3dbpath);
	void SortDllName(std::vector<wstring> &iniDllList,std::vector<vec_wstr> &DllList);
	bool LoadDllDetails();
	void PrintLog(const wstring &wstrLog);

	CDLLObject * GetDLLObjectByName(wstring dllname);

	bool InitSchedule();
	bool Exit();

	wstring GetLocalConfig(wstring wstrConfigName);

	int	GetAllImportDll();

	BOOL CreateFolder(wstring strPath);
	BOOL FolderExist(wstring strPath);
	BOOL FileExist(wstring strFileName);
	BOOL CopyDllFile(wstring selFilePath,wstring DllName);
	BOOL SaveMemToFile( const char* pBlock, const int nSize, wstring szFileName,CDLLObject* pDllOb);
	bool UpdateDllObjectByFile(wstring strDllOriginalName, wstring strFilePath, bool bNeedSaveDBFile);
	bool ReloadDllByOrginalName(wstring strDllOriginalName);
	bool FindDllByOrginalName(wstring strDllOriginalName, vector<CDLLObject *> & dllList);

	bool UpdateDllObjectByFile(wstring strUpdateState, wstring strThreadName, wstring strDLLName, wstring strFilePath, wstring strImportTime,wstring strAuthor, wstring strPeriod, wstring strDllContent, wstring strRunStatus,wstring strVersion,wstring strDescription,wstring strDllOldName, bool bNeedSaveDBFile);	//20180720 更新dll(新增，删除)
	bool DeleteDllfromDB(wstring strDllname);
	bool DeleteThreadName(wstring strThreadName);

	bool	SaveParametertoDB(int nInOrOutput, wstring strDllName,  wstring strVariableName, wstring strLinkDefine, wstring strLinkType);
	bool    GetInputParameterfromDB(wstring strDllName, std::vector<vec_wstr> &inputPara);
	bool    GetOutputParameterfromDB(wstring strDllName, std::vector<vec_wstr> &outputPara);
	bool	CompareInputParameter(std::vector<vec_wstr> &inputPara,std::vector<vec_wstr> &Ini_inputPara);
	bool	CompareOutputParameter(std::vector<vec_wstr> &outputPara,std::vector<vec_wstr> &Ini_outputPara);

	bool    SaveSpanTimetoDB(wstring DllName,double SpanTime);
	bool    SetTimeSpan(CDLLObject *pObject, double fSpanTime);
	std::vector<vec_wstr> SeparateParameter(wstring strLongPatameter);
	wstring LinkParameter(std::vector<vec_wstr> &matrixpara);

	CDLLObject * FindDLLObject(wstring strDllName);
	CLogicDllThread * FindDLLThreadByName(wstring strDllThreadName);
	bool	LoadDllThreadRelationship(CLogicDllThread *pDllThread);

	bool InitRelationships();

public:
	std::vector<vec_wstr> m_vecCurDllInputParameterList;
	std::vector<vec_wstr> m_vecCurDllOutputParameterList;
	std::vector<wstring> one_para;

	Project::Tools::Mutex	m_lock;

	wstring m_strDBFileName;

	wstring m_wstrAppName;

	bool m_bExitThread;
	bool m_bThreadExitFinished;

	std::vector<wstring>	 m_vecDllPath_Rep;
	std::vector<wstring>     m_vecSelDllName_Rep; 
	std::vector<wstring>	 m_vecSelDllName;   
	std::vector<CDLLObject*> vecImportDLLList;
	std::vector<CLogicDllThread *> m_vecDllThreadList;
	int						 m_iDeleteItem;
	double m_calcinternal;
	std::vector<wstring>	m_vecCurDllName_iniLoad;

public:
	CBEOPDataAccess *m_pDataAccess;
	CLogicBase            *m_pCLogicBaseOptimize;
	CScheduleThread* m_pSchedule;

	std::vector<wstring> m_wstrPrefixList;
	std::vector<wstring> m_wstrExceptPrefixList;


	CWnd *m_pWnd;
};

