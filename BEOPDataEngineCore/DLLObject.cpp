#include "StdAfx.h"
#include "DLLObject.h"
#include "LogicParameter.h"
#include "LogicParameterConst.h"
#include "LogicParameterLink.h"
#include "LogicParameterPoint.h"


const int LogStoreSize = 500;
const wstring LogicSysVersion = L"V2.3";
CDLLObject::CDLLObject(CBEOPDataAccess *pDataAccess):m_pDataAccess(pDataAccess)
{

	m_pLB			= NULL;
	m_pHinstance	= NULL;
	m_sDllName		= L"";
	m_bDllRun       = true;
	m_pFun			= NULL;
	m_iTimeSpan     = 0;
	m_sImportTime	= L"";
	m_sDllAuthor	= L"";
	m_sDllPathFile  = L"";
	m_sDllVersion	= L"";
	m_sDllDescription	= L"";


	m_bAvailable    = true;
	m_bLoaded		= false;

	m_vecCurDllInputParameterList.clear();
	m_vecCurDllOutputParameterList.clear();

	m_sS3DBPath		= L"";

	m_strLogList.clear();

}
bool CDLLObject::ReSet()
{


	m_pLB			= NULL;
	m_pHinstance	= NULL;
	m_sDllName		= L"";
	m_bDllRun       = true;
	m_pFun			= NULL;
	m_iTimeSpan     = 0;
	m_bLogicActing      = false;
	m_sImportTime	= L"";
	m_sDllAuthor	= L"";
	m_sDllPathFile  = L"";
	m_sDllVersion	= L"";
	m_sDllDescription	= L"";
	//ʹm_hEventClose�ָ�Ϊ�Ǽ���״̬

	m_bAvailable    = true;
	m_bLoaded		= false;

	unsigned int i = 0;
	for(i=0;i<m_vecCurDllInputParameterList.size();i++)
	{
		delete(m_vecCurDllInputParameterList[i]);
		m_vecCurDllInputParameterList[i] = NULL;
	}
	for(i=0;i<m_vecCurDllOutputParameterList.size();i++)
	{
		delete(m_vecCurDllOutputParameterList[i]);
		m_vecCurDllOutputParameterList[i] = NULL;
	}

	m_vecCurDllInputParameterList.clear();
	m_vecCurDllOutputParameterList.clear();

	m_strLogList.clear();

	return true;
}
CDLLObject::~CDLLObject(void)
{


}
void CDLLObject::SetMsgWnd(CWnd * MsgWnd)
{
	m_MsgWnd = MsgWnd;

	if(m_pLB)
		m_pLB->SetMsgWnd(MsgWnd);
}

void CDLLObject::SentMsgInfo(LPCTSTR loginfo)
{
	if (m_MsgWnd)
	{
	//	if(m_MsgWnd->GetSafeHwnd())
	//		::SendMessage(m_MsgWnd->GetSafeHwnd(), WM_DATAENGINELOG, (WPARAM)(L"Logic"), (LPARAM)loginfo);
	}
}

CLogicBase*	CDLLObject::GetLB()
{
	return m_pLB;
}
bool CDLLObject::SetLB(CLogicBase* pLB)
{
	m_pLB = pLB;
	return true;
}
HINSTANCE CDLLObject::GetHinstance()
{
	return m_pHinstance;
}
bool CDLLObject::SetHinstance(HINSTANCE pHinstance)
{
	m_pHinstance = pHinstance;
	return true;
}
wstring	CDLLObject::GetDllImportTime()
{
	return m_sImportTime;
}

wstring	CDLLObject::GetDllAuthor()
{
	return m_sDllAuthor;
}
bool CDLLObject::SetDllAuthor(wstring author)
{
	m_sDllAuthor = author;

	return true;
}
wstring	CDLLObject::GetDllPath()
{
	return m_sDllPathFile;
}
bool CDLLObject::SetDllPath(wstring path)
{
	m_sDllPathFile = path;

	return true;
}

bool CDLLObject::SetDllImportTime(wstring dllImporttime)
{
	m_sImportTime = dllImporttime;

	return true;
}

wstring CDLLObject::GetDllName()
{
	return m_sDllName;
}

bool CDLLObject::SetDllName(wstring dllname)
{
	m_sDllName = dllname;
	return true;
}

pfnLogicBase CDLLObject::GetFun()
{
	return m_pFun;
}

wstring		CDLLObject::GetOriginalDllName()
{
	return m_strOriginalName;
}

bool		CDLLObject::SetOriginalDllName(wstring dllname)
{
	m_strOriginalName = dllname;
	return true;
}

bool CDLLObject::SetFun(pfnLogicBase fun)
{
	m_pFun = fun;
	return true;
}
double	CDLLObject::GetTimeSpan()
{
	return m_iTimeSpan;
}
bool CDLLObject::SetTimeSpan(double Span)
{
	m_iTimeSpan = Span;
	return true;
}
bool CDLLObject::GetAvailable()
{
	return m_bAvailable;
}
bool CDLLObject::SetAvailable(bool use)
{
	m_bAvailable = use;

	return true;
}


bool CDLLObject::Exit()
{
	UnLoad();
	wstring str = L"========" + m_sDllName + L" Engine (Version: 1.0) is Unload...========";
	SentMsgInfo(str.c_str());

	wstring dll_name =m_sDllName;

	HWND hwnd = m_MsgWnd->GetSafeHwnd();

	ReSet();

	//::SendMessage(hwnd, WM_UPDATEDLLCOUNT, 0, (LPARAM)(LPCTSTR)dll_name.c_str());

	return true;
}

bool CDLLObject::GetLoaded()
{
	return m_bLoaded;
}
bool CDLLObject::SetLoaded( bool isload )
{
	m_bLoaded = isload;

	return true;
}

bool CDLLObject::GetDllInputParamterValue( CLogicBase* LB )
{
	if (LB == NULL)
	{
		return false;
	}

	for (int i=0;i<m_vecCurDllInputParameterList.size();++i)
	{
		wstring strCurValue;
		if(m_vecCurDllInputParameterList[i]->UpdateValue(strCurValue))
			m_pLB->SetInputValue(i, strCurValue.c_str());
	}

	return true;
}

bool CDLLObject::SetDllOutputParamterValue( CLogicBase* LB )
{
	if (LB == NULL)
	{
		return false;
	}

	for (int i=0;i<m_vecCurDllOutputParameterList.size();++i)
	{
		CLogicParameter *pPara = m_vecCurDllOutputParameterList[i];
		if (pPara->Enabled())
		{
			if (LB->GetOutputChangeFlag(i))//�����ֵ����
			{
				CString strOutputValue;
				LB->GetOutputValue(i,strOutputValue);
				wstring wstr = strOutputValue.GetString();

				bool bSetSuccess = pPara->UpdateValue(wstr); 

				if(bSetSuccess)
				{
					//if(pPara->GetOutputString().find(L"point")>=0 && pPara->GetSettingValue().length()>0)
					if(pPara->GetSettingValue().length()>0) //����������const����point������Ϊ�Ǹ���������
					{//��ζ���ǵ�����
						COleDateTime tNow = COleDateTime::GetCurrentTime();
						wstring wstrTime;
						Project::Tools::OleTime_To_String(tNow, wstrTime);
						CString strID;
						strID.Format(_T("[Thread:%s][logic:%s]"), m_sDllThreadName.c_str(), LB->GetDllName());
						m_pDataAccess->InsertLogicRecord(strID.GetString(), wstrTime, pPara->GetSettingValue(), wstr);
					}

				}

				
				//������Ϊ��д
				LB->SetOutputChangeFlag(i,FALSE);

				if(bSetSuccess==false)
					return false;

			}
		}
	}

	return true;
}


bool  CDLLObject::SetDllInputParamter(wstring strVariableName, wstring strLinkValue, wstring strLinkType)
{ //���Ըı�����
	int nPIndex = FindInputParameter(strVariableName);
	if(nPIndex<0)
		return false;


	CLogicParameter *pPara = m_vecCurDllInputParameterList[nPIndex];

	if(strLinkType== pPara->GetType())
		pPara->SetSettingValue(strLinkValue); 
	else
	{ // type ���޸�
		if(strLinkType==L"const")
		{
			m_vecCurDllInputParameterList[nPIndex] = new CLogicParameterConst(pPara->GetName(),0,strLinkType, pPara->GetDataAccess(),strLinkValue);
		}
		else if(strLinkType==L"logiclink")
		{
			//��֧�������޸�
		}
		else
		{
			m_vecCurDllInputParameterList[nPIndex] = new CLogicParameterPoint(pPara->GetName(),0,strLinkType, pPara->GetDataAccess(),strLinkValue);
		}

		if(pPara)
		{
			delete(pPara);
			pPara = NULL;
		}


	}

	return true;
}


bool  CDLLObject::SetDllOutputParamter(wstring strVariableName, wstring strLinkValue, wstring strLinkType)
{
	int nPIndex = FindOutputParameter(strVariableName);
	if(nPIndex<0)
		return false;

	CLogicParameter *pPara = m_vecCurDllOutputParameterList[nPIndex];
	
	if(strLinkType== pPara->GetType())
		pPara->SetSettingValue(strLinkValue); 
	else
	{ // type ���޸�
		//20200516 const��ͬ��point
		//if(strLinkType==L"const" || strLinkType==L"strconst")
		//{
		//	m_vecCurDllOutputParameterList[nPIndex] = new CLogicParameterConst(pPara->GetName(),1,strLinkType, pPara->GetDataAccess(),strLinkValue);
		//}
		//else 
		if(strLinkType==L"logiclink")
		{
			//��֧�������޸�
		}
		else
		{
			m_vecCurDllOutputParameterList[nPIndex] = new CLogicParameterPoint(pPara->GetName(),1,strLinkType, pPara->GetDataAccess(),strLinkValue);
		}

		if(pPara)
		{
			delete(pPara);
			pPara = NULL;
		}


	}
	
	return true;
}

int CDLLObject::FindInputParameter(wstring strVariableName)
{
	int i = 0;
	for(i=0;i<m_vecCurDllInputParameterList.size();i++)
	{
		if(m_vecCurDllInputParameterList[i]->GetName()== strVariableName)
			return i;
	}

	return -1;
}



int CDLLObject::FindOutputParameter(wstring strVariableName)
{
	for(int i=0;i<m_vecCurDllOutputParameterList.size();i++)
	{
		if(m_vecCurDllOutputParameterList[i]->GetName()== strVariableName)
			return i;
	}

	return -1;
}

bool CDLLObject::SetDllVersion( const wstring &ver )
{
	m_sDllVersion = ver;
	return true;
}

wstring CDLLObject::GetDllVersion()
{
	return m_sDllVersion;
}

bool CDLLObject::SetDllDescription( const wstring &des )
{
	m_sDllDescription = des;

	return true;
}

wstring CDLLObject::GetDllDescription()
{
	return m_sDllDescription;
}

wstring CDLLObject::GetS3DBPath()
{
	return m_sS3DBPath;
}

bool CDLLObject::SetS3DBPath(const wstring &s3dbpath)
{
	m_sS3DBPath = s3dbpath;

	return true;
}


void CDLLObject::AddLogInfo(const wstring &strLog)
{
	
		if (m_strLogList.size()<LogStoreSize)
		{
			m_strLogList.push_back(strLog);
		}
		else
		{
			// ����һ��������������¼Ϊһ���ļ������
			wstring strFilePath;
			Project::Tools::GetSysPath(strFilePath);

			COleDateTime tnow = COleDateTime::GetCurrentTime();
			char* old_locale = _strdup( setlocale(LC_CTYPE,NULL) ); 
			setlocale( LC_CTYPE, ("chs"));

			CString strLogPath;
			strLogPath.Format(L"%s\\log\\%s-%s.txt",strFilePath, GetDllName(), tnow.Format(L"%Y%m%d%H%M%S"));
			CStdioFile ff(strLogPath, CFile::modeCreate | CFile::modeWrite | CFile::typeText);
			for(int iLine=0;iLine<m_strLogList.size();iLine++)
			{
				wstring strInfo = m_strLogList[iLine];
				strInfo+=L"\r\n";
				ff.WriteString(strInfo.c_str());
			}
			ff.Close();
			setlocale( LC_CTYPE, old_locale ); 
			free( old_locale ); 


			Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock_strLog);
			m_strLogList.clear();
		}
	
}


bool CDLLObject::GetLogAll(wstring &strAllLog)
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock_strLog);
	for (int i =0;i<m_strLogList.size();++i)
	{
		strAllLog += m_strLogList[i];
		strAllLog += L"\r\n";
	}

	return true;
}



bool CDLLObject::SetDllThreadName(wstring strDllThreadName)
{
	m_sDllThreadName = strDllThreadName;

	return true;
}

wstring  CDLLObject::GetDllThreadName()
{
	return m_sDllThreadName;
}



void CDLLObject::ClearParameters()
{
	int i = 0;
	for(i=0;i<m_vecCurDllInputParameterList.size();i++)
	{
		SAFE_DELETE(m_vecCurDllInputParameterList[i]);
	}

	for(i=0;i<m_vecCurDllOutputParameterList.size();i++)
	{
		SAFE_DELETE(m_vecCurDllOutputParameterList[i]);
	}

	m_vecCurDllInputParameterList.clear();
	m_vecCurDllOutputParameterList.clear();
}

bool CDLLObject::UnLoad()
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock_LB);
	//�����̲߳��ڽ�����һ����
	if (m_pLB != NULL)
	{
		m_pLB->Exit();
		delete m_pLB;
		m_pLB = NULL;
	}
	if (m_pHinstance != NULL)
	{
		FreeLibrary(m_pHinstance);
	}

	return true;
}

bool CDLLObject::Load(wstring strPath)
{
	int nTick1 = GetTickCount();
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock_LB);
	wstring strComp = GetDllName();

	if (GetLoaded())
	{
		return true;
	}

	HINSTANCE m_hDLL;
	m_hDLL = NULL;
	m_hDLL = LoadLibrary(strPath.c_str());//���ض�̬���ӿ�
	int nErr = GetLastError();
	if(m_hDLL==NULL)
	{
		_tprintf(strComp.c_str());
		_tprintf(_T(" Logic Error: Logic Loadlibrary return NULL!\r\n"));
		return false ;
	}

	pfnLogicBase m_pf_Dll = (pfnLogicBase)GetProcAddress(m_hDLL,"fnInitLogic");

	if(m_pf_Dll==NULL)
	{
		CString strTemp;
		strTemp.Format(_T("Logic Warning: Cant find InitLogic entrance : %s!\r\n"),strPath.c_str() );
		_tprintf(strTemp.GetString());
		FreeLibrary(m_hDLL);
		return false;
	}
	//////////////////////////////////////////////////////////////////////////
	//���е�ǰ���ص�dll����
	int nTick1a = GetTickCount();
	CLogicBase *pLBInterface = m_pf_Dll();

	int nTick2 = GetTickCount();

	if(pLBInterface==NULL)
		return false;



	wstring dllver = pLBInterface->GetSystVersion().GetString();

	CString cstrdllver = dllver.c_str();
	CString cstrgatewayver = LogicSysVersion.c_str();

	double  ddllver = _ttof(cstrdllver.Mid(1,cstrdllver.GetLength()));
	double  dgatewayver = _ttof(cstrgatewayver.Mid(1,cstrgatewayver.GetLength()));

	if (ddllver < dgatewayver)
	{
		_tprintf(_T("Logic Error: Logic Version Overdue!  \r\n"));
		FreeLibrary(m_hDLL);
		return false;

	}

	pLBInterface->SetDllName(GetDllName().c_str());
	pLBInterface->SetDataAccess(m_pDataAccess);

	//�������Ӧ������dll�ڲ���
	SetDllVersion(pLBInterface->GetDllLogicVersion().GetString());
	SetDllAuthor(pLBInterface->GetDllAuthor().GetString());
	SetDllDescription(pLBInterface->GetDllDescription().GetString());

	int nTick3 = GetTickCount();
	//////////////////////////////////////////////////////////////////////////

	//�洢dell�����Ϣ
	//dll ���
	SetHinstance(m_hDLL);
	//dll �����ӿ�
	SetFun(m_pf_Dll);
	SetLB(pLBInterface);

	int nTick4 = GetTickCount();

	if((nTick4-nTick1)>10000)
	{
		CString strInfo;
		strInfo.Format(_T("Warning:load too slow(%.1fs,%.1fs,%.1fs,%.1fs)"), (nTick1a-nTick1)/1000.0, (nTick2-nTick1a)/1000.0, (nTick3-nTick2)/1000.0,  (nTick4-nTick3)/1000.0);
		_tprintf(strInfo.GetString());
	}

	return true;
}


bool CDLLObject::RunOnce(int nActCount)
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock_LB);

	CLogicBase *pLB = GetLB();

	bool bRunGood=  true;

	if ( pLB != NULL)
	{

		bool bInputPara = GetDllInputParamterValue(pLB); 
		CString strLogicBaseSystemVersion = pLB->GetSystVersion();
		if(strLogicBaseSystemVersion.GetLength()>0 && strLogicBaseSystemVersion.Left(1)==_T("V"))
			strLogicBaseSystemVersion = strLogicBaseSystemVersion.Mid(1);
		double fVersion  = _wtof(strLogicBaseSystemVersion.GetString());

		
		if (bInputPara)
		{
			COleDateTime tnow = COleDateTime::GetCurrentTime();
			m_bLogicActing = true;
			if(fVersion>=3.0)
			{
				pLB->SetActTime(tnow.Format(L"%Y-%m-%d %H:%M:%S"));
			}
			//CString strCount;
			//strCount.Format(_T("%d"), nActCount);
			//m_pDataAccess->WriteCoreDebugItemValue(_T("LogicStatusBegin"), pLB->GetDllName().GetString(), tnow.Format(L"%Y-%m-%d %H:%M:%S").GetString(),strCount.GetString(), _T(""));
			LogLine(true, nActCount);
			bool bActSuccess = true;
			try
			{

				pLB->ActLogic();
			}
			catch (CException* e)
			{
				//m_pDataAccess->WriteCoreDebugItemValue(_T("LogicCrash"), pLB->GetDllName().GetString(), tnow.Format(L"%Y-%m-%d %H:%M:%S").GetString(),strCount.GetString(), _T(""));
				TCHAR   szError[1024];   
				e->GetErrorMessage(szError,1024);
				wstring wstrError = szError;
				CString strErrInsert;
				strErrInsert.Format(_T("[%s]-SYSTEM EXCEPTION:%s\r\n"), pLB->GetDllName(), wstrError.c_str());
				m_pDataAccess->InsertLog(strErrInsert.GetString());
				bActSuccess = false;

				_tprintf(strErrInsert.GetString());
				bRunGood = false;
			}

			LogLine(false, nActCount);

			COleDateTime tEnd = COleDateTime::GetCurrentTime();
			//CString strCostSeconds;
			//strCostSeconds.Format(_T("%d"), (tEnd-tnow).GetTotalSeconds());
			//m_pDataAccess->WriteCoreDebugItemValue(_T("LogicStatusEnd"), pLB->GetDllName().GetString(), tEnd.Format(L"%Y-%m-%d %H:%M:%S").GetString(),strCount.GetString(),strCostSeconds.GetString());
			
			m_bLogicActing = false;

			if(bActSuccess)
			{
				bool bSetSuccess = SetDllOutputParamterValue(pLB);
				if(!bSetSuccess) //д���ʧ��
				{
					CString strTemp;
					strTemp.Format(L"ERROR: Logic(%s) Set Output Parameters Value Failed.\r\n", pLB->GetDllName());
					//m_pLB->SentMsgInfo(strTemp.GetString());
					_tprintf(strTemp.GetString());
				}
				else
				{

				}
			}
			else
			{
				CString strTemp;
				strTemp.Format(L"ERROR: Logic(%s) Act Failed.\r\n", pLB->GetDllName());
				_tprintf(strTemp.GetString());
			}
			
		}
		else //��ȡ����ʧ��
		{
			CString strTemp;
			strTemp.Format(L"ERROR: Logic %s Get Input Parameters Value Failed.\r\n", pLB->GetDllName());
			//m_pLB->SentMsgInfo(strTemp.GetString());
			_tprintf(strTemp.GetString());
			bRunGood = false;
		}
		//GetDllInputParamterValue(m_pLB);

	}

	return bRunGood;
}


void        CDLLObject::LogLine(bool bBegin, int nTick)
{
	Project::Tools::Scoped_Lock<Mutex>	scopelock(m_lock_LB);

	CLogicBase *pLB = GetLB();

	COleDateTime tNow = COleDateTime::GetCurrentTime();
	wstring wstrTime;
	Project::Tools::OleTime_To_String(tNow, wstrTime);

	if ( pLB != NULL)
	{

		CString strID;
		if(bBegin)
			strID.Format(_T("[%s]-VERSION:%s---------(COUNT %d)ACT ONCE BEGIN----------\r\n"), pLB->GetDllName(), pLB->GetDllLogicVersion(), nTick);
		else
		{
			strID.Format(_T("[%s]-VERSION:%s----------(COUNT %d)ACT ONCE END  ----------\r\n"), pLB->GetDllName(), pLB->GetDllLogicVersion(), nTick);
		}
		m_pDataAccess->InsertLog(strID.GetString());
	}
}