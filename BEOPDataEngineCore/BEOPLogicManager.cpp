#include "StdAfx.h"
#include "BEOPLogicManager.h"
#include "sstream"
#include "VECTOR"
#include "LogicBase.h"
#include "../BEOPDataPoint/sqlite/SqliteAcess.h"
#include "../Tools/Util/UtilString.h"
#include "LogicDllThread.h"
#include "LogicParameterConst.h"
#include "LogicParameterLink.h"
#include "LogicParameterPoint.h"
#include "LogicPipeline.h"
#include "json/json.h"
#include "json/reader.h"

CBEOPLogicManager::CBEOPLogicManager(wstring strDBFileName,CWnd *pWnd, std::vector<wstring> wstrPrefixList, std::vector<wstring> wstrExceptPrefixList)
	:m_pWnd(pWnd)
	,m_strDBFileName(strDBFileName)
{
	m_calcinternal = 10;
	m_vecSelDllName.clear();
	vecImportDLLList.clear();
	m_bExitThread = false;
	m_bThreadExitFinished = false;
	m_pDataAccess = NULL;
	m_iDeleteItem   = -1;
	//添加版本号
	m_pSchedule = NULL;

	m_wstrPrefixList = wstrPrefixList;
	m_wstrExceptPrefixList = wstrExceptPrefixList;

}


CBEOPLogicManager::~CBEOPLogicManager(void)
{

}


bool CBEOPLogicManager::Exit()
{
	//save running state into ini
	bool bSuccess = true;
	unsigned int i = 0;
	for(i=0;i<vecImportDLLList.size();i++)
	{
		if(!vecImportDLLList[i]->Exit())
			bSuccess = false;
	}

	for(i=0;i<vecImportDLLList.size();i++)
	{
		SAFE_DELETE(vecImportDLLList[i]);
	}
	vecImportDLLList.clear();

	for(i=0;i<m_vecDllThreadList.size();i++)
	{
		SAFE_DELETE(m_vecDllThreadList[i]);
	}
	m_vecDllThreadList.clear();

	m_calcinternal = 10;
	m_vecSelDllName.clear();
	m_pDataAccess = NULL;

	if(m_pSchedule)
	{
		m_pSchedule->Exit();
		delete m_pSchedule;
		m_pSchedule = NULL;
	}
	return bSuccess;
}


bool CBEOPLogicManager::StoreDllToDB(CDLLObject* dllOb, wstring strDllPath)
{
	FILE *fp;
	long filesize = 0;
	char * ffile = NULL;

	_bstr_t tem_des = strDllPath.c_str();


	char* des = (char*)tem_des;
	fopen_s(&fp,des, "rb");

	if(fp != NULL)
	{
		//计算文件的大小
		fseek(fp, 0, SEEK_END);
		filesize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		//读取文件
		ffile = new char[filesize+1];
		size_t sz = fread(ffile, sizeof(char), filesize, fp);
		fclose(fp);
	}

	if (ffile != NULL)
	{
		InsertDLLtoDB(dllOb,ffile,filesize);
		delete [] ffile;
		return true;
	}

	return false;
}


bool CBEOPLogicManager::InsertDLLtoDB(CDLLObject* dllOb,char *pBlock,int nSize)
{

	string strUtf8;
	Project::Tools::WideCharToUtf8(m_strDBFileName, strUtf8);

	CSqliteAcess access(strUtf8);
	if (!access.GetOpenState())
	{
		_tprintf(L"ERROR: Open S3DB File Failed!\r\n");
	}
	char szSQL[1024] = {0};
	memset(szSQL, 0, sizeof(szSQL));
	int rc = 0;
	int nID = 1;
	wstring importtime = dllOb->GetDllImportTime();
	wstring author = dllOb->GetDllAuthor();
	wstring dllname = dllOb->GetDllName();

	wstring importime_ = importtime;
	_bstr_t tem_des = importime_.c_str();
	char* des = (char*)tem_des;
	std::string importtime__(des);

	wstring dllname_ = dllname;
	tem_des = dllname_.c_str();
	des = (char*)tem_des;
	std::string dllname__(des);

	wstring author_ = author;
	tem_des = author_.c_str();
	des = (char*)tem_des;
	std::string author__(des);

	//dll版本
	wstring version_ = dllOb->GetDllVersion();
	tem_des = version_.c_str();
	des = (char*)tem_des;
	std::string version__(des);
	//dll描述
	wstring description_ = dllOb->GetDllDescription();
	tem_des = description_.c_str();
	des = (char*)tem_des;
	std::string description__(des);
	//dll original name
	wstring strOriginalName = dllOb->GetOriginalDllName();
	int nDot = (strOriginalName.find(L".dll"));
	if(nDot>0)
		strOriginalName = strOriginalName.substr(0, nDot);


	tem_des = strOriginalName.c_str();
	des = (char*)tem_des;
	std::string strOriginalDllName__(des);

	wstring threadname_ = dllOb->GetDllThreadName();
	tem_des = threadname_.c_str();
	des = (char*)tem_des;
	std::string threadname__(des);
	//////////////////////////////////////////////////////////////////////////
	//删除已有的同名dll
	string sql_del;
	ostringstream sqlstream_del;
	sqlstream_del << "delete from list_dllstore where DllName = "<< "'" << dllname__ << "';"; 
	sql_del = sqlstream_del.str();
	if (SQLITE_OK == access.SqlExe(sql_del.c_str()))
	{
		;
	}
	//////////////////////////////////////////////////////////////////////////
	int runstatus = 1;
	string sql_;
	ostringstream sqlstream;
	string em("");
	sqlstream << "insert into list_dllstore(id,DllName,importtime,author,periodicity,dllcontent,runstatus,unitproperty01,unitproperty02,unitproperty03,unitproperty04,unitproperty05)  values('" << nID<< "','" << dllname__ << "','" \
		<< importtime__ << "','" << author__ << "','" << dllOb->GetTimeSpan() << "',?,'"<<runstatus<<"','"<<version__ <<"','"<<description__<<"','"<<strOriginalDllName__<<"','"<<threadname__<<"','"<<em<<"')";
	sql_ = sqlstream.str();

	if (SQLITE_OK == access.SqlQuery(sql_.c_str()))
	{
		if (access.m_stmt)
		{
			rc = access.SqlBind_Blob(pBlock,1,nSize);
			assert(0 == rc);
			rc = access.SqlNext();
			//assert(0 == rc);
			rc = access.SqlFinalize();
			assert(0 == rc);
		}
	}

	access.CloseDataBase();

	return true;
}

bool CBEOPLogicManager::ThreadExitFinish()
{
	return m_bThreadExitFinished;
}
bool CBEOPLogicManager::SaveSpanTimetoDB(wstring DllName,double SpanTime)
{
	string strUtf8;
	Project::Tools::WideCharToUtf8(m_strDBFileName, strUtf8);
	CSqliteAcess access(strUtf8);

	string sql_del;
	ostringstream sqlstream_del;
	wstring dllname_ = DllName;
	_bstr_t tem_des = dllname_.c_str();
	char* des = (char*)tem_des;
	std::string sdllname(des);
	int runstatus = 0;
	sqlstream_del << "update list_dllstore set periodicity ="<< SpanTime <<",runstatus = "<< runstatus <<" where DllName = "<< "'" << sdllname << "';"; 
	sql_del = sqlstream_del.str();
	access.SqlExe(sql_del.c_str());

	access.CloseDataBase();
	return true;
}

bool   CBEOPLogicManager::SetTimeSpan(CDLLObject *pObject, double fSpanTime)
{
	bool bFindThread = false;
	unsigned int i=0;
	for(i=0;i<m_vecDllThreadList.size();i++)
	{
		if(m_vecDllThreadList[i]->GetName()==pObject->GetDllThreadName())
		{
			bFindThread = true;
			break;
		}

	}

	if(bFindThread)
	{
		CLogicDllThread *pDllThread = m_vecDllThreadList[i];
		for(int j=0;j<pDllThread->GetPipelineCount();j++)
		{
			CLogicPipeline *pPipeLine = pDllThread->GetPipeline(j);
			if(pPipeLine->FindDllObject(pObject))
			{
				pPipeLine->SetTimeSpanSeconds(fSpanTime);
				return true;
			}
		}
	}
	return false;
}

bool CBEOPLogicManager::UpdateDLLfromDB(const wstring &s3dbpath,  CDLLObject *pDllOb)
{
	if(pDllOb==NULL)
		return false;

	if(pDllOb->GetDllName().length()<=0)
		return false;

	string strUtf8;
	Project::Tools::WideCharToUtf8(s3dbpath, strUtf8);
	CSqliteAcess access(strUtf8);

	string strNameUtf8;
	Project::Tools::WideCharToUtf8(pDllOb->GetDllName(), strNameUtf8);
	ostringstream sqlstream;
	sqlstream << "select * from list_dllstore where DllName = '"<< strNameUtf8 <<"';";
	string sql_ = sqlstream.str();
	char *ex_sql = const_cast<char*>(sql_.c_str());
	int re = access.SqlQuery(ex_sql);

	if(SQLITE_ROW != access.SqlNext())
	{
		return false;
	}

	int id = access.getColumn_Int(0);
	wstring dllname		= L"";
	wstring importtime  = L"";
	wstring author      = L"";
	wstring version		= L"";
	wstring description = L"";
	wstring dlloriginname = L"";
	wstring threadname = L"default";

	if (const_cast<char*>(access.getColumn_Text(1)) !=NULL)
	{
		string   name_temp(const_cast<char*>(access.getColumn_Text(1)));
		wstring ss = UtilString::ConvertMultiByteToWideChar(name_temp);
		dllname = ss.c_str();
	}
	if (const_cast<char*>(access.getColumn_Text(1)) !=NULL)
	{
		string   t_temp(const_cast<char*>(access.getColumn_Text(2)));
		wstring ss = UtilString::ConvertMultiByteToWideChar(t_temp);
		importtime = ss.c_str();
	}
	if (const_cast<char*>(access.getColumn_Text(1)) !=NULL)
	{
		string   a_temp(const_cast<char*>(access.getColumn_Text(3)));
		wstring ss = UtilString::ConvertMultiByteToWideChar(a_temp);
		author = ss.c_str();
	}

	double timespan = access.getColumn_Double(4);

	const void* pf = access.getColumn_Blob(5);
	int nSize      = access.getColumn_Bytes(5);	
	int runstatus  = access.getColumn_Int(6);


	if (const_cast<char*>(access.getColumn_Text(7)) !=NULL)
	{
		string   v_temp(const_cast<char*>(access.getColumn_Text(7)));
		wstring ss = UtilString::ConvertMultiByteToWideChar(v_temp);
		version = ss.c_str();
	}
	if (const_cast<char*>(access.getColumn_Text(8)) !=NULL)
	{
		string   d_temp(const_cast<char*>(access.getColumn_Text(8)));
		wstring ss = UtilString::ConvertMultiByteToWideChar(d_temp);
		description = ss.c_str();
	}
	if (const_cast<char*>(access.getColumn_Text(9)) !=NULL)
	{
		string   d_temp(const_cast<char*>(access.getColumn_Text(9)));
		wstring ss = UtilString::ConvertMultiByteToWideChar(d_temp);
		dlloriginname = ss.c_str();
	}
	if (const_cast<char*>(access.getColumn_Text(10)) !=NULL)
	{
		string   d_temp(const_cast<char*>(access.getColumn_Text(10)));
		wstring ss = UtilString::ConvertMultiByteToWideChar(d_temp);
		threadname = ss.c_str();
		if(threadname.length()<=0)
			threadname = L"default";
	}

	pDllOb->SetDllName(dllname.c_str());
	pDllOb->SetDllAuthor(author.c_str());
	pDllOb->SetDllImportTime(importtime.c_str());
	pDllOb->SetTimeSpan(timespan);
	pDllOb->SetAvailable(false);
	pDllOb->SetDllVersion(version.c_str());
	pDllOb->SetDllDescription(description.c_str());
	pDllOb->SetDllThreadName(threadname);
	pDllOb->SetOriginalDllName(dlloriginname+L".dll");

	//////////////////////////////////////////////////////////////////////////
	const char* pff = (char*)pf;
	//从数据库读取文件
	bool bCopyToDisk = false;
	if (pff != NULL && nSize >0)
	{
		bCopyToDisk = SaveMemToFile(pff,nSize,dllname,pDllOb);
	}
	
	access.SqlFinalize();
	access.CloseDataBase();
	return bCopyToDisk;
}

bool CBEOPLogicManager::ReadDLLfromDB(const wstring &s3dbpath,std::vector<CDLLObject*> &dllObList)
{
	string strUtf8;
	Project::Tools::WideCharToUtf8(s3dbpath, strUtf8);
	CSqliteAcess access(strUtf8);

	ostringstream sqlstream;
	if(m_wstrPrefixList.size()==0)
	    sqlstream << "select * from list_dllstore order by DllName;";
	else
	{

		sqlstream << "select * from list_dllstore where unitproperty04 like '";
		for(int i=0;i< m_wstrPrefixList.size();i++)
		{
			string strChar;
			Project::Tools::WideCharToUtf8(m_wstrPrefixList[i], strChar);
			if(i==(m_wstrPrefixList.size()-1))
			{
				sqlstream<< strChar<<"%'  order by DllName;";
			}
			else
			{

				sqlstream<<strChar<<"%' or unitproperty04 like '";
			}
		}
	}
	string sql_ = sqlstream.str();
	char *ex_sql = const_cast<char*>(sql_.c_str());
	int re = access.SqlQuery(ex_sql);

	while(true)
	{
		if(SQLITE_ROW != access.SqlNext())
		{
			break;
		}

		int id = access.getColumn_Int(0);
		wstring dllname		= L"";
		wstring importtime  = L"";
		wstring author      = L"";
		wstring version		= L"";
		wstring description = L"";
		wstring dlloriginname = L"";
		wstring threadname = L"default";

		if (const_cast<char*>(access.getColumn_Text(1)) !=NULL)
		{
			string   name_temp(const_cast<char*>(access.getColumn_Text(1)));
			wstring ss = UtilString::ConvertMultiByteToWideChar(name_temp);
			dllname = ss.c_str();
		}
		if (const_cast<char*>(access.getColumn_Text(2)) !=NULL)
		{
			string   t_temp(const_cast<char*>(access.getColumn_Text(2)));
			wstring ss = UtilString::ConvertMultiByteToWideChar(t_temp);
			importtime = ss.c_str();
		}
		if (const_cast<char*>(access.getColumn_Text(3)) !=NULL)
		{
			string   a_temp(const_cast<char*>(access.getColumn_Text(3)));
			wstring ss = UtilString::ConvertMultiByteToWideChar(a_temp);
			author = ss.c_str();
		}

		double timespan = access.getColumn_Double(4);

		const void* pf = access.getColumn_Blob(5);
		int nSize      = access.getColumn_Bytes(5);	
		int runstatus  = access.getColumn_Int(6);


		if (const_cast<char*>(access.getColumn_Text(7)) !=NULL)
		{
			string   v_temp(const_cast<char*>(access.getColumn_Text(7)));
			wstring ss = UtilString::ConvertMultiByteToWideChar(v_temp);
			version = ss.c_str();
		}
		if (const_cast<char*>(access.getColumn_Text(8)) !=NULL)
		{
			string   d_temp(const_cast<char*>(access.getColumn_Text(8)));
			wstring ss = UtilString::ConvertMultiByteToWideChar(d_temp);
			description = ss.c_str();
		}
		if (const_cast<char*>(access.getColumn_Text(9)) !=NULL)
		{
			string   d_temp(const_cast<char*>(access.getColumn_Text(9)));
			wstring ss = UtilString::ConvertMultiByteToWideChar(d_temp);
			dlloriginname = ss.c_str();
		}
		if (const_cast<char*>(access.getColumn_Text(10)) !=NULL)
		{
			string   d_temp(const_cast<char*>(access.getColumn_Text(10)));
			wstring ss = UtilString::ConvertMultiByteToWideChar(d_temp);
			threadname = ss.c_str();
			if(threadname.length()<=0)
				threadname = L"default";
		}

		if(Project::Tools::StringContainElementOfVector(threadname, m_wstrExceptPrefixList))
		{
			continue;
		}

		if(m_wstrPrefixList.size()>0)
		{
			bool bFoundPrefix = false;
			for(int ww=0;ww<m_wstrPrefixList.size();ww++)
			{
				int nPos = threadname.find(m_wstrPrefixList[ww].c_str());
				if(nPos>=0)
				{
					bFoundPrefix = true;
				}
			}

			if(!bFoundPrefix)
			{
				continue;
			}

		}

		if(dllname.find(L"[Thread:")!=-1)
		{
			CString strError;
			strError.Format(_T("ERROR: Read One Logic(name:%s) but is destroyed!!!!\r\n"), dllname.c_str());
			_tprintf(strError.GetString());
			continue;
		}

		CDLLObject* pDllOb = new CDLLObject(m_pDataAccess);

		pDllOb->SetDllName(dllname.c_str());
		pDllOb->SetDllAuthor(author.c_str());
		pDllOb->SetDllImportTime(importtime.c_str());
		pDllOb->SetTimeSpan(timespan);
		pDllOb->SetAvailable(false);
		pDllOb->SetDllVersion(version.c_str());
		pDllOb->SetDllDescription(description.c_str());
		pDllOb->SetDllThreadName(threadname);
		pDllOb->SetOriginalDllName(dlloriginname+L".dll");

		dllObList.push_back(pDllOb);

		//////////////////////////////////////////////////////////////////////////
		const char* pff = (char*)pf;
		//从数据库读取文件
		if (pff != NULL && nSize >0)
		{
			SaveMemToFile(pff,nSize,dllname,pDllOb);
		}
	}
	access.SqlFinalize();
	access.CloseDataBase();

	return true;

}


bool CBEOPLogicManager::FindDllByOrginalName(wstring strDllOriginalName, vector<CDLLObject *> & dllList)
{
	CString strError;
	for(int i=0;i<vecImportDLLList.size();i++)
	{

		CDLLObject *pDllObject = vecImportDLLList[i];
		wstring strOriginname = pDllObject->GetOriginalDllName();

		if(pDllObject && strOriginname== strDllOriginalName)
		{
			dllList.push_back(pDllObject);
		}
	}

	return true;
}

bool CBEOPLogicManager::ReloadDllByOrginalName(wstring strDllOriginalName)
{
	CString strError;
	for(int i=0;i<vecImportDLLList.size();i++)
	{

		CDLLObject *pDllObject = vecImportDLLList[i];
		wstring strOriginname = pDllObject->GetOriginalDllName();

		strError.Format(_T("Reload Logic: Compare %s====%s\r\n"),strOriginname.c_str(), strDllOriginalName.c_str());
		_tprintf(strError.GetString());
		if(pDllObject && strOriginname== strDllOriginalName)
		{
			
			strError.Format(_T("Reload Logic: Unload %s\r\n"), pDllObject->GetDllName().c_str());
			_tprintf(strError.GetString());

			//unload library
			pDllObject->UnLoad();

			strError.Format(_T("Reload Logic: Reload File from 4db %s\r\n"), pDllObject->GetDllName().c_str());
			_tprintf(strError.GetString());
			if(!UpdateDLLfromDB(m_strDBFileName,pDllObject))
			{
				_tprintf(_T("Reload Logic: Reload File Successfully\r\n"));
				return false;
			}
			else
			{
				_tprintf(_T("Reload Logic: Reload File Failed\r\n"));
			}


			//load new
			if(!pDllObject->Load(pDllObject->GetDllPath()))
			{
				_tprintf(_T("Reload Logic: Reload All Content Successfully\r\n"));
				return false;
			}
			else
			{
				_tprintf(_T("Reload Logic: Reload All Content Failed\r\n"));
			}

		}


	}
	return true;
}

bool CBEOPLogicManager::UpdateDllObjectByFile(wstring strDllOriginalName, wstring strFilePath, bool bNeedSaveDBFile)
{
	for(int i=0;i<vecImportDLLList.size();i++)
	{
		
		CDLLObject *pDllObject = vecImportDLLList[i];
		wstring strOriginname = pDllObject->GetOriginalDllName();


		if(pDllObject && strOriginname== strDllOriginalName)
		{
			//unload library
			pDllObject->UnLoad();
			
			if(bNeedSaveDBFile)
			{
				if(!StoreDllToDB(pDllObject, strFilePath))
					return false;

			}


			if(!UpdateDLLfromDB(m_strDBFileName,pDllObject))
				return false;


			//load new
			if(!pDllObject->Load(pDllObject->GetDllPath()))
				return false;

			//初始化
			if(pDllObject->GetLB() != NULL)
				pDllObject->GetLB()->Init();
			
		}

		
	}
	return true;
}

bool CBEOPLogicManager::UpdateDllObjectByFile( wstring strUpdateState, wstring strThreadName, wstring strDLLName, wstring strFilePath, wstring strImportTime,wstring strAuthor, wstring strPeriod, wstring strDllContent, wstring strRunStatus,wstring strVersion,wstring strDescription,wstring strDllOldName ,  bool bNeedSaveDBFile)
{
	if(strUpdateState == L"3")		//更新
	{	
		for(int i=0;i<vecImportDLLList.size();i++)
		{
			CDLLObject *pDllObject = vecImportDLLList[i];

			if(pDllObject && pDllObject->GetDllThreadName() == strThreadName && pDllObject->GetDllName()== strDLLName)		//找到相应线程dll名称
			{
				//unload library
				pDllObject->UnLoad();

				if(bNeedSaveDBFile)
				{
					if(!StoreDllToDB(pDllObject, strFilePath))
						return false;
				}
				

				if(!UpdateDLLfromDB(m_strDBFileName,pDllObject))
					return false;

				//load new
				if(!pDllObject->Load(pDllObject->GetDllPath()))
					return false;

				return true;
			}
		}
	}
	else if(strUpdateState == L"4")		//新增
	{
		//查找是否有相应的dll 如果没有才新建
		CDLLObject *findImportDLL =  FindDLLObject(strDLLName);

		if(findImportDLL == NULL)
		{
			findImportDLL = new CDLLObject(m_pDataAccess);		//创建策略
			if(findImportDLL)
			{
				//load new
				if(!findImportDLL->Load(strFilePath))
					return false;

				findImportDLL->SetMsgWnd(m_pWnd);
				findImportDLL->SetS3DBPath(m_strDBFileName.c_str());
				vecImportDLLList.push_back(findImportDLL);

				findImportDLL->SetDllName(strDLLName);
				findImportDLL->SetDllThreadName(strThreadName);
				findImportDLL->SetOriginalDllName(strDllOldName+L".dll");

				if(bNeedSaveDBFile)
				{
					if(!StoreDllToDB(findImportDLL, strFilePath))		//存储s3db
						return false;
				}

				//存储本次导入dll
				UpdateDLLParameter(findImportDLL);

				if(!UpdateDLLfromDB(m_strDBFileName,findImportDLL))	//读取
					return false;

				//查找相应线程名
				CLogicDllThread *pDllThread = FindDLLThreadByName(strThreadName);

				if(pDllThread==NULL)	//线程不存在 创建
				{
					pDllThread = new CLogicDllThread(strThreadName);
					m_vecDllThreadList.push_back(pDllThread);
					pDllThread->AddDll(findImportDLL);

					LoadDllThreadRelationship(pDllThread);	//添加线程关系

					pDllThread->GeneratePipelines();	

					pDllThread->StartThread();
				}
				else				//存在 添加
				{
					pDllThread->AddDll(findImportDLL);

					LoadDllThreadRelationship(pDllThread);	//添加线程关系

					pDllThread->GeneratePipelines();		
				}				
				return true;
			}
		}
	}
	else if(strUpdateState == L"5")		//删除策略
	{
		for(int i=0;i<vecImportDLLList.size();i++)
		{
			CDLLObject *pDllObject = vecImportDLLList[i];

			if(pDllObject && pDllObject->GetDllThreadName() == strThreadName)		//找到相应线程dll名称
			{					
				pDllObject->Exit();		//退出

				vecImportDLLList.erase(vecImportDLLList.begin() + i);		//移除

			    //还要从线程中移除
				CLogicDllThread *pDllThread = FindDLLThreadByName(strThreadName);

				if(pDllThread != NULL)	//线程存在删除 线程中的dll
				{
					pDllThread->DeleteDll(pDllObject);
				}

				SAFE_DELETE(pDllObject);

				if(bNeedSaveDBFile)
				{
					//删除s3db
					DeleteDllfromDB(strDLLName);
				}

				return true;
			}
		}
	}
	else if(strUpdateState == L"6")		//删除线程
	{
		for(int j=0;j<m_vecDllThreadList.size();j++)
		{
			CLogicDllThread *pDllThread = m_vecDllThreadList[j];
			if(pDllThread && pDllThread->GetName()==strThreadName)
			{
				m_vecDllThreadList.erase(m_vecDllThreadList.begin()+j);

				for(int i=0;i<vecImportDLLList.size();i++)
				{
					CDLLObject *pDllObject = vecImportDLLList[i];

					if(pDllObject && pDllObject->GetDllThreadName() == strThreadName)		//找到相应线程dll名称
					{					
						pDllObject->Exit();		//退出

						vecImportDLLList.erase(vecImportDLLList.begin() + i);		//移除

						if(pDllThread != NULL)	//线程存在删除 线程中的dll
						{
							pDllThread->DeleteDll(pDllObject);
						}

						SAFE_DELETE(pDllObject);

					}
				}
				pDllThread->Exit();

				SAFE_DELETE(pDllThread);

				if(bNeedSaveDBFile)
				{
					//删除s3db
					DeleteThreadName(strThreadName);

				}

				return true;
			}
		}

	}
	return false;
}

BOOL CBEOPLogicManager::SaveMemToFile( const char* pBlock, const int nSize, wstring szFileName,CDLLObject* pDllOb)
{
	wstring  tempPath;
	Project::Tools::GetSysPath(tempPath);
	tempPath += L"\\importdll";

	const wchar_t* lpszFolderPath =  tempPath.c_str();
	if (!FolderExist(lpszFolderPath))
	{
		BOOL bCreateDir = CreateFolder(lpszFolderPath);
		if(!bCreateDir)
		{
			PrintLog(L"ERROR: SaveMemToFile CreateFolder failed.\r\n");
			return FALSE;
		}
	}
	wstring strFileName;
	wchar_t wcChar[1024];
	wsprintf(wcChar, L"%s\\%s", lpszFolderPath,szFileName.c_str());
	strFileName = wcChar;

	if (FileExist(strFileName))
	{
		BOOL bDelted = DeleteFile(strFileName.c_str());
		if(!bDelted)
		{
			CString strErr;
			strErr.Format(L"ERROR: SaveMemToFile DeleteFile(%s) failed.\r\n", strFileName.c_str());
			PrintLog(strErr.GetString());
			return FALSE;
		}
	}
	pDllOb->SetDllPath(strFileName);
	//写入文件
	FILE* pFile = NULL;
	if(_wfopen_s(&pFile, strFileName.c_str(), L"wb")==0)
	{
		fseek(pFile, 0, SEEK_SET);
		fwrite(pBlock, nSize, 1, pFile);
		fclose(pFile);
	}
	else
	{
		PrintLog(L"ERROR: SaveMemToFile _wfopen_s failed.\r\n");
		return FALSE;	}


	return TRUE;
}

bool CBEOPLogicManager::SaveParametertoDB(int nInOrOutput, wstring strDllName,  wstring strVariableName, wstring strLinkDefine, wstring strLinkType)
{
	string strUtf8;
	Project::Tools::WideCharToUtf8(m_strDBFileName, strUtf8);
	CSqliteAcess access(strUtf8);

	if (!access.GetOpenState())
	{
		return false;
	}

	//dll name
	wstring dllname = strDllName;
	_bstr_t tem_des = dllname.c_str();
	char* des = (char*)tem_des;
	std::string dllname__(des);


	int nIN = 0;
	bool bAllSuccess = true;
	access.BeginTransaction();
	std::vector<wstring> onerecord;

	string sql_in;
	ostringstream sqlstream;


	//变量名
	wstring vname_ = strVariableName;
	tem_des = vname_.c_str();
	des = (char*)tem_des;
	std::string vname__(des);

	//点名
	wstring pointname_ = strLinkDefine;
	tem_des = pointname_.c_str();
	des = (char*)tem_des;
	std::string pointname__(des);

	//类型
	wstring ptype = strLinkType;
	tem_des = ptype.c_str();
	des = (char*)tem_des;
	std::string ptype__(des);

	std::string em("");

	sqlstream << "update list_paramterConfig set pname = '"<<pointname__<<"', ptype = '"<<ptype__<<"' where vname = '"<< vname__<<"' and DllName = '"<<dllname__ <<"' and INorOut = "<<nInOrOutput<<";";
	sql_in = sqlstream.str();

	int re = access.SqlExe(sql_in.c_str());

	if (re != SQLITE_OK)
	{
		bAllSuccess =false;
	}


	if (access.CommitTransaction() != SQLITE_OK || !bAllSuccess)
	{
		_tprintf(L"Edit Dll Fails!\r\n");
	}

	access.CloseDataBase();
	return bAllSuccess;
}

bool CBEOPLogicManager::GetInputParameterfromDB(wstring strDllName,  std::vector<vec_wstr> &inputPara )
{
	string strUtf8;
	Project::Tools::WideCharToUtf8(m_strDBFileName, strUtf8);
	CSqliteAcess access(strUtf8);
	
	if (!access.GetOpenState())
	{
		TRACE(L"Warning:Open S3DB Fail!\r\n");
	}

	//dll name
	wstring dllname = strDllName;
	_bstr_t tem_des = dllname.c_str();
	char* des = (char*)tem_des;
	std::string dllname__(des);

	ostringstream sqlstream;
	sqlstream << "select * from list_paramterConfig where INorOut = 0 and DllName = '"<< dllname__ <<"';";
	string sql_ = sqlstream.str();
	char *ex_sql = const_cast<char*>(sql_.c_str());
	int re = access.SqlQuery(ex_sql);
	if (re != SQLITE_OK)
	{
		TRACE(L"Warning:Read S3db Paramter Config Fail!\r\n");
	}
	inputPara.clear();
	std::vector<wstring> onerecord;
	while(true)
	{
		if(SQLITE_ROW != access.SqlNext())
		{
			break;
		}
		onerecord.clear();
		if(access.getColumn_Text(0) != NULL)
		{
			string  vname(const_cast<char*>(access.getColumn_Text(0)));
			wstring vname_ = UtilString::ConvertMultiByteToWideChar(vname);
			onerecord.push_back(vname_.c_str());
		}
		if(access.getColumn_Text(1) != NULL)
		{
			string  pname(const_cast<char*>(access.getColumn_Text(1)));
			wstring pname_ = UtilString::ConvertMultiByteToWideChar(pname);
			onerecord.push_back(pname_.c_str());
		}

		if(access.getColumn_Text(2) != NULL)
		{
			string  ptype(const_cast<char*>(access.getColumn_Text(2)));
			wstring ptype_ = UtilString::ConvertMultiByteToWideChar(ptype);
			onerecord.push_back(ptype_.c_str());
		}

		if(access.getColumn_Text(3) != NULL)
		{
			string  pexplain(const_cast<char*>(access.getColumn_Text(3)));
			wstring pexplain_ = UtilString::ConvertMultiByteToWideChar(pexplain);
			onerecord.push_back(pexplain_.c_str());
		}

		inputPara.push_back(onerecord);
	}
	access.SqlFinalize();
	access.CloseDataBase();
	return true;
}

bool CBEOPLogicManager::GetOutputParameterfromDB(wstring strDllName,  std::vector<vec_wstr> &outputPara )
{
	string strUtf8;
	Project::Tools::WideCharToUtf8(m_strDBFileName, strUtf8);
	CSqliteAcess access(strUtf8);
	if (!access.GetOpenState())
	{
		TRACE(L"Warning:Open S3DB Fail!\r\n");
	}
	//dll name
	wstring dllname = strDllName;
	_bstr_t tem_des = dllname.c_str();
	char* des = (char*)tem_des;
	std::string dllname__(des);

	ostringstream sqlstream;
	sqlstream << "select * from list_paramterConfig where INorOut = 1 and DllName = '"<< dllname__<<"';";
	string sql_ = sqlstream.str();
	char *ex_sql = const_cast<char*>(sql_.c_str());
	int re = access.SqlQuery(ex_sql);
	if (re != SQLITE_OK)
	{
		TRACE(L"Warning:Read S3db Paramter Config Fail!\r\n");
	}
	outputPara.clear();
	std::vector<wstring> onerecord;
	while(true)
	{
		if(SQLITE_ROW != access.SqlNext())
		{
			break;
		}
		onerecord.clear();
		if(access.getColumn_Text(0) != NULL)
		{
			string  vname(const_cast<char*>(access.getColumn_Text(0)));
			wstring vname_ = UtilString::ConvertMultiByteToWideChar(vname);
			onerecord.push_back(vname_.c_str());
		}
		if(access.getColumn_Text(1) != NULL)
		{
			string  pname(const_cast<char*>(access.getColumn_Text(1)));
			wstring pname_ = UtilString::ConvertMultiByteToWideChar(pname);
			onerecord.push_back(pname_.c_str());
		}

		if(access.getColumn_Text(2) != NULL)
		{
			string  ptype(const_cast<char*>(access.getColumn_Text(2)));
			wstring ptype_ = UtilString::ConvertMultiByteToWideChar(ptype);
			onerecord.push_back(ptype_.c_str());
		}

		if(access.getColumn_Text(3) != NULL)
		{
			string  pexplain(const_cast<char*>(access.getColumn_Text(3)));
			wstring pexplain_ = UtilString::ConvertMultiByteToWideChar(pexplain);
			onerecord.push_back(pexplain_.c_str());
		}

		if(access.getColumn_Text(6) != NULL)
		{
			string  pexplain(const_cast<char*>(access.getColumn_Text(6)));
			wstring pexplain_ = UtilString::ConvertMultiByteToWideChar(pexplain);
			onerecord.push_back(pexplain_.c_str());
		}

		outputPara.push_back(onerecord);
	}
	access.SqlFinalize();
	access.CloseDataBase();
	return true;
}

bool CBEOPLogicManager::CompareInputParameter( std::vector<vec_wstr> &inputPara ,std::vector<vec_wstr> &Ini_inputPara)
{

	for (int i=0;i<Ini_inputPara.size();++i)
	{
		for (int j=0;j<inputPara.size();++j)
		{
			if (Ini_inputPara[i][0] == inputPara[j][0])
			{
				Ini_inputPara[i] = inputPara[j];
				break;
			}
		}
	}

	return true;
}

bool CBEOPLogicManager::CompareOutputParameter( std::vector<vec_wstr> &outputPara ,std::vector<vec_wstr> &Ini_outputPara)
{

	for (int i=0;i<Ini_outputPara.size();++i)
	{
		for (int j=0;j<outputPara.size();++j)
		{
			if (Ini_outputPara[i][0] == outputPara[j][0])
			{
				Ini_outputPara[i] = outputPara[j];
				break;
			}
		}
	}

	return true;
}
std::vector<vec_wstr> CBEOPLogicManager::SeparateParameter( wstring strLongPatameter_ )
{
	std::vector<vec_wstr> MatrixPara;

	CString strLongPatameter = strLongPatameter_.c_str();
	if(strLongPatameter.Find(_T("$"))>0)
	{//第二版的字符串

		std::vector<wstring> strItems;
		Project::Tools::SplitStringByChar(strLongPatameter,'$', strItems);

		TCHAR Sep = '/';
		CString P = _T("");
		for (int i=0;i<strItems.size();++i)
		{

			std::vector<wstring> OneGroup;
			Project::Tools::SplitStringByChar(strItems[i].c_str(),'/', OneGroup);
			MatrixPara.push_back(OneGroup);		
		}
	}
	else
	{
		TCHAR Sep = '/';
		CString P = _T("");
		std::vector<wstring> OneGroup;

		for (int i=0;i<strLongPatameter.GetLength();++i)
		{
			if (strLongPatameter.GetAt(i) == Sep)
			{
				OneGroup.push_back(P.GetString());
				P = _T("");
				if (OneGroup.size() == 4)
				{
					MatrixPara.push_back(OneGroup);
					OneGroup.clear();
				}
			}else
			{
				CString ii(strLongPatameter.GetAt(i));
				P += ii;
			}
		}
	}



	return MatrixPara;
}

wstring CBEOPLogicManager::LinkParameter( std::vector<vec_wstr> &matrixpara )
{
	wstring strLongPara = L"";
	if (matrixpara.empty())
	{
		return strLongPara;
	}else
	{
		if (matrixpara[0].size() == 4)
		{
			for (int i=0;i<matrixpara.size();++i)
			{
				strLongPara += matrixpara[i][0]+_T("/")+matrixpara[i][1]+_T("/")+matrixpara[i][2]+_T("/")+matrixpara[i][3]+_T("/");
			}
			return strLongPara;

		}else if (matrixpara[0].size() == 5)
		{
			for (int i=0;i<matrixpara.size();++i)
			{
				strLongPara += matrixpara[i][0]+_T("/")+matrixpara[i][1]+_T("/")+matrixpara[i][2]+_T("/")+matrixpara[i][3]+_T("/")+ matrixpara[i][4]+_T("/");
			}
			return strLongPara;
		}
	}

	return strLongPara;
}


CDLLObject * CBEOPLogicManager::GetDLLObjectByName(wstring dllname)
{

	for(int i =0;i<vecImportDLLList.size();i++)
	{
		wstring strComp = vecImportDLLList[i]->GetDllName();
		if(strComp==dllname)
		{

			return vecImportDLLList[i];
		}
	}

	return NULL;
}

bool CBEOPLogicManager::LoadDllDetails()
{
	if (vecImportDLLList.empty())
	{
		return true;
	}
	CDLLObject *pDllOb = NULL;
	for (int k = 0;k < vecImportDLLList.size();++k)
	{
		CString strInfo;
		strInfo.Format(_T("Loading %s.\r\n"), vecImportDLLList[k]->GetDllName().c_str());
		PrintLog(strInfo.GetString());
		if(!vecImportDLLList[k]->Load(vecImportDLLList[k]->GetDllPath()))
		{

			strInfo.Format(_T("ERROR In Loading %s, Load Failed!!!\r\n"), vecImportDLLList[k]->GetDllName().c_str());
			PrintLog(strInfo.GetString());
			return false;
		}

		vecImportDLLList[k]->SetMsgWnd(m_pWnd);
		vecImportDLLList[k]->SetS3DBPath(m_strDBFileName.c_str());

	}
	return true;
}


void CBEOPLogicManager::PrintLog(const wstring &wstrLog)
{

	if(m_wstrAppName.length()==0)
	{
		wstring wstrTemp;
		Project::Tools::GetAppName(wstrTemp);

		m_wstrAppName = wstrTemp.substr(0, wstrTemp.find(L".exe"));
	}

	//if(m_pDataAccess_Arbiter)
	//	m_pDataAccess_Arbiter->InsertLog(g_tLogAll, g_strLogAll);
	SYSTEMTIME st;
	GetLocalTime(&st);
	string time_utf8 = Project::Tools::SystemTimeToString(st);

	wstring wstrTime ;
	Project::Tools::UTF8ToWideChar(time_utf8, wstrTime);

	CString strOneItem;
	strOneItem.Format(_T("%s    %s"), wstrTime.c_str(), wstrLog.c_str());


	try
	{
		wstring strPath;
		GetSysPath(strPath);
		strPath += L"\\log";
		if(Project::Tools::FindOrCreateFolder(strPath))
		{
			COleDateTime oleNow = COleDateTime::GetCurrentTime();
			CString strLogPath;
			strLogPath.Format(_T("%s\\%s_err_%d_%02d_%02d.log"), strPath.c_str(), m_wstrAppName.c_str(), oleNow.GetYear(),oleNow.GetMonth(),oleNow.GetDay());


			char* old_locale = _strdup( setlocale(LC_CTYPE,NULL) );
			setlocale( LC_ALL, "chs" );
			//记录Log
			CStdioFile	ff;
			if(ff.Open(strLogPath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite))
			{
				ff.Seek(0,CFile::end);
				ff.WriteString(strOneItem);
				ff.Close();
			}
			setlocale( LC_CTYPE, old_locale ); 
			free( old_locale ); 	
		}
	}
	catch (CMemoryException* e)
	{

	}
	catch (CFileException* e)
	{
	}
	catch (CException* e)
	{
	}


}

void CBEOPLogicManager::SortDllName( std::vector<wstring> &iniDllList,std::vector<vec_wstr> &DllList )
{
	if (iniDllList.empty() || DllList.empty())
	{
		return;
	}
	std::vector<vec_wstr> temList(DllList);
	std::vector<int>  label(DllList.size(),1);

	DllList.clear();
	for (int i= 0;i<iniDllList.size();++i)
	{
		for (int j = 0;j<temList.size();++j)
		{
			if (iniDllList[i] == temList[j][0])
			{
				DllList.push_back(temList[j]);
				label[j] = 0;
				break;
			}
		}
	}

	for (int i = 0;i<temList.size();++i)
	{
		if (label[i] == 1)
		{
			DllList.push_back(temList[i]);
		}
	}

	return;
}


BOOL CBEOPLogicManager::CopyDllFile(wstring selFilePath,wstring DllName)
{
	wstring m_wstrFilePath;
	Project::Tools::GetSysPath(m_wstrFilePath);
	m_wstrFilePath += L'\\';
	m_wstrFilePath += L"importdll";
	wstring dec(m_wstrFilePath.c_str());
	if (!FolderExist(dec.c_str()))
	{
		CreateFolder(dec.c_str());
	}
	dec += L"\\";
	dec += DllName;
	CopyFile(selFilePath.c_str(),dec.c_str(),false);

	return TRUE;
}



BOOL CBEOPLogicManager::FolderExist(wstring strPath)
{
	WIN32_FIND_DATA wfd;
	BOOL rValue = FALSE;
	HANDLE hFind = FindFirstFile(strPath.c_str(), &wfd);
	if ((hFind!=INVALID_HANDLE_VALUE) &&
		(wfd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
	{
		rValue = TRUE;
	}
	FindClose(hFind);
	return rValue;
}


BOOL CBEOPLogicManager::CreateFolder(wstring strPath)
{
	SECURITY_ATTRIBUTES attrib;
	attrib.bInheritHandle = FALSE;
	attrib.lpSecurityDescriptor = NULL;
	attrib.nLength = sizeof(SECURITY_ATTRIBUTES);
	return ::CreateDirectory(strPath.c_str(), &attrib);
}

BOOL CBEOPLogicManager::FileExist(wstring strFileName)
{
	CFileFind fFind;
	return fFind.FindFile(strFileName.c_str());
}

int CBEOPLogicManager::GetAllImportDll()
{
	return vecImportDLLList.size();
}


CDLLObject * CBEOPLogicManager::FindDLLObject(wstring strDllName)
{
	for(int i=0;i<vecImportDLLList.size();i++)
	{
		if(vecImportDLLList[i]->GetDllName() == strDllName)
			return vecImportDLLList[i];
	}

	return NULL;
}

bool CBEOPLogicManager::InitRelationships()
{
	//generate all threads
	for (int i =0 ;i<vecImportDLLList.size();++i)
	{
		CDLLObject *curImportDLL = vecImportDLLList[i];
		CString strDllThreadName = curImportDLL->GetDllThreadName().c_str();
		CLogicDllThread *pDllThread = FindDLLThreadByName(strDllThreadName.GetString());
		if(pDllThread==NULL)
		{
			pDllThread = new CLogicDllThread(strDllThreadName.GetString());
			m_vecDllThreadList.push_back(pDllThread);
			pDllThread->AddDll(curImportDLL);

		}
		else
			pDllThread->AddDll(curImportDLL);
	}

	//
	for(int i=0;i<m_vecDllThreadList.size();i++)
	{
		LoadDllThreadRelationship(m_vecDllThreadList[i]);
		m_vecDllThreadList[i]->GeneratePipelines();

		m_vecDllThreadList[i]->SetThreadID(i+1, m_vecDllThreadList.size());
	}

	return true;
}



CLogicDllThread * CBEOPLogicManager::FindDLLThreadByName(wstring strDllThreadName)
{
	for(int i=0;i<m_vecDllThreadList.size();i++)
	{
		if(m_vecDllThreadList[i]->GetName()==strDllThreadName)
			return m_vecDllThreadList[i];
	}

	return NULL;
}

bool CBEOPLogicManager::LoadDllThreadRelationship(CLogicDllThread *pDllThread)
{
	int i=0, j=0;
	wstring strDllVersionList = L"";
	for(i=0;i<pDllThread->GetDllCount();i++)
	{
		CDLLObject *pObject=  pDllThread->GetDllObject(i);

		strDllVersionList += pObject->GetDllVersion();
		strDllVersionList += L"|||";

		std::vector<vec_wstr> strParaList;
		pObject->ClearParameters();

		GetInputParameterfromDB(pObject->GetDllName(),strParaList);
		for(j=0;j< strParaList.size();j++)
		{
			CLogicParameter *pp = NULL;
			vec_wstr strPara = strParaList[j];
			if(strPara[2]==L"const" || strPara[2]==L"strconst")
			{
				pp = new CLogicParameterConst(strPara[0], 0,strPara[2], m_pDataAccess, strPara[1]);
			}
			else if(strPara[2]==L"logiclink")
			{
				CString strTT = strPara[1].c_str();
				int nMao = strTT.Find(':');
				if(nMao>=0)
				{
					CString strLinkDllName = strTT.Left(nMao);
					CString strLinkDllPortName = strTT.Mid(nMao+1);
					CDLLObject *pLinkObject = FindDLLObject(strLinkDllName.GetString());
					pp = new CLogicParameterLink(strPara[0],0,strPara[2], m_pDataAccess, pLinkObject, strLinkDllPortName.GetString());
				}

			}
			else
			{
				pp = new CLogicParameterPoint(strPara[0],0,strPara[2], m_pDataAccess, strPara[1]);
			}
			pObject->m_vecCurDllInputParameterList.push_back(pp);
		}

		//read output
		std::vector<vec_wstr> strOutputParaList;
		GetOutputParameterfromDB(pObject->GetDllName(),strOutputParaList);
		for(j=0;j< strOutputParaList.size();j++)
		{
			CLogicParameter *pp = NULL;
			vec_wstr strPara = strOutputParaList[j];

			//output将const等同point处理，防止误配置
			//if(strPara[2]==L"const" || strPara[2]==L"strconst")
			//{
			//	pp = new CLogicParameterConst(strPara[0],1,strPara[2], m_pDataAccess, strPara[1]);
			//}
			//else 
			if(strPara[2]== L"logiclink")
			{
				CString strPP = strPara[1].c_str();
				int nMao = strPP.Find(':');
				if(nMao>=0)
				{
					CString strLinkDllName = strPP.Left(nMao);
					CString strLinkDllPortName = strPP.Mid(nMao+1);
					CDLLObject *pLinkObject = FindDLLObject(strLinkDllName.GetString());
					pp = new CLogicParameterLink(strPara[0],1,strPara[2], m_pDataAccess, pLinkObject, strLinkDllPortName.GetString());
				}

			}
			else
			{
				pp = new CLogicParameterPoint(strPara[0],1,strPara[2], m_pDataAccess, strPara[1], strPara[4]);
			}
			pObject->m_vecCurDllOutputParameterList.push_back(pp);
		}

	}

	m_pDataAccess->WriteCoreDebugItemValue2(pDllThread->GetName(), strDllVersionList);

	return true;
}

bool CBEOPLogicManager::IfExistDll(const wstring &s3dbpath)
{
	string strUtf8;
	Project::Tools::WideCharToUtf8(s3dbpath, strUtf8);
	CSqliteAcess access(strUtf8);

	ostringstream sqlstream;
	sqlstream << "  select count(*) from list_dllstore;";
	string sql_ = sqlstream.str();
	char *ex_sql = const_cast<char*>(sql_.c_str());
	int re = access.SqlQuery(ex_sql);
	int nCount = 0;
	while(true)
	{
		if(SQLITE_ROW != access.SqlNext())
		{
			break;
		}

		nCount = access.getColumn_Int(0);
	}
	access.SqlFinalize();
	access.CloseDataBase();
	if(nCount > 0)
		return true;
	return false;
}


wstring CBEOPLogicManager::GetLocalConfig(wstring wstrConfigName)
{
	string strUtf8;
	Project::Tools::WideCharToUtf8(m_strDBFileName, strUtf8);
	CSqliteAcess access(strUtf8);

	wstring wstr = _T("");

	string strConfigName = UtilString::ConvertWideCharToMultiByte(wstrConfigName);

	ostringstream sqlstream;
	sqlstream << "select content from local_config where name = '"<< strConfigName <<"'"<<";";
	string sql_ = sqlstream.str();
	char *ex_sql = const_cast<char*>(sql_.c_str());
	int re = access.SqlQuery(ex_sql);

	while(true)
	{
		if(SQLITE_ROW != access.SqlNext())
		{
			break;
		}

		if (access.getColumn_Text(0) != NULL)
		{
			string strContetn(access.getColumn_Text(0));
			wstr = UtilString::ConvertMultiByteToWideChar(strContetn);

		}

	}

	access.SqlFinalize();
	access.CloseDataBase();

	return wstr;
}

bool CBEOPLogicManager::InitSchedule()
{
	if(m_pSchedule == NULL)
	{

		hash_map<wstring, wstring> mapTimeDefine;
		wstring wstrDefine = GetLocalConfig(L"system_time_define");

		Json::Reader jsonReader;
		string strDefineJson_utf8;
		Project::Tools::WideCharToUtf8(wstrDefine, strDefineJson_utf8);
		Json::Value jsonRoot;
		if(jsonReader.parse(strDefineJson_utf8, jsonRoot))
		{
			if(jsonRoot.isObject())
			{
				Json::Value::Members member = jsonRoot.getMemberNames();
				for(Json::Value::Members::iterator iter = member.begin(); iter != member.end(); ++iter)
				{
					std::string name = *iter;  
					wstring wstrName;
					Project::Tools::UTF8ToWideChar(name, wstrName);
					std::string value_str = jsonRoot[name]["point"].asString(); 
					wstring wstrValue;
					Project::Tools::UTF8ToWideChar(value_str, wstrValue);

					mapTimeDefine[wstrName] = wstrValue;

				}
			}
		}

		m_pSchedule = new CScheduleThread(m_pDataAccess, mapTimeDefine);
		m_pSchedule->Init();
		

	}

	return true;
}
bool CBEOPLogicManager::DeleteDllfromDB( wstring strDllname )
{
	string strUtf8,strDllName_Utf8;
	Project::Tools::WideCharToUtf8(m_strDBFileName, strUtf8);
	Project::Tools::WideCharToUtf8(strDllname, strDllName_Utf8);

	CSqliteAcess access(strUtf8);
	if (!access.GetOpenState())
	{
		_tprintf(L"ERROR: Open S3DB File Failed!\r\n");
	}
	else
	{
		string sql_del;
		ostringstream sqlstream_del;
		sqlstream_del << "delete from list_dllstore where DllName = "<< "'" << strDllName_Utf8 << "';"; 
		sql_del = sqlstream_del.str();
		int re = access.SqlExe(sql_del.c_str());
		access.CloseDataBase();
		if (SQLITE_OK == re)
		{
			return true;
		}
	}
	
	return false;
}

bool CBEOPLogicManager::UpdateDLLParameter( CDLLObject* dllOb )
{
	if (dllOb == NULL)
	{
		return false;
	}
	CString strLongInputPara = dllOb->GetLB()->GetAllInputParameter();
	CString strLongOutputPara = dllOb->GetLB()->GetAllOutputParameter();
	m_vecCurDllInputParameterList  = SeparateParameter(strLongInputPara.GetString());
	m_vecCurDllOutputParameterList = SeparateParameter(strLongOutputPara.GetString());

	for (int i=0;i<m_vecCurDllOutputParameterList.size();++i)
	{
		if (m_vecCurDllOutputParameterList[i].size()<5)
		{
			m_vecCurDllOutputParameterList[i].push_back(_T("1"));
		}
	}

	std::vector<vec_wstr> inputDB,outputDB;
	GetInputParameterfromDB(dllOb->GetDllName(),inputDB);
	GetOutputParameterfromDB(dllOb->GetDllName(),outputDB);
	CompareInputParameter(inputDB,m_vecCurDllInputParameterList);
	CompareOutputParameter(outputDB,m_vecCurDllOutputParameterList);

	for (int i=0;i<m_vecCurDllInputParameterList.size();++i)
	{
		dllOb->GetLB()->SetInputValue(1,m_vecCurDllInputParameterList[i][1].c_str());
	}

	//////////////////////////////////////////////////////////////////////////
	std::vector<vec_str> vecCurDllInputParameterList_temp,vecCurDllOutputParameterList_temp;
	for (int i=0;i<m_vecCurDllInputParameterList.size();++i)
	{
		std::vector<CString> temp;
		for (int j = 0;j<m_vecCurDllInputParameterList[i].size();++j)
		{
			temp.push_back(m_vecCurDllInputParameterList[i][j].c_str());
		}

		vecCurDllInputParameterList_temp.push_back(temp);
	}
	for (int i=0;i<m_vecCurDllOutputParameterList.size();++i)
	{
		std::vector<CString> temp;
		for (int j = 0;j<m_vecCurDllOutputParameterList[i].size();++j)
		{
			temp.push_back(m_vecCurDllOutputParameterList[i][j].c_str());
		}

		vecCurDllOutputParameterList_temp.push_back(temp);
	}

	//////////////////////////////////////////////////////////////////////////
	SaveInputParametertoDB(dllOb->GetDllName().c_str(),vecCurDllInputParameterList_temp);
	SaveOutputParametertoDB(dllOb->GetDllName().c_str(),vecCurDllOutputParameterList_temp);
	return true;
}

bool CBEOPLogicManager::SaveInputParametertoDB(CString strDllName,  std::vector<vec_str> &strLongPara )
{

	std::vector<vec_str> inputPara = strLongPara;

	string strUtf8;
	Project::Tools::WideCharToUtf8(m_strDBFileName, strUtf8);
	CSqliteAcess access(strUtf8);
	if (!access.GetOpenState())
	{
		_tprintf(L"ERROR: Open S3DB File Failed!\r\n");
	}
	else
	{
		//dll name
		wstring dllname = strDllName.GetString();
		_bstr_t tem_des = dllname.c_str();
		char* des = (char*)tem_des;
		std::string dllname__(des);

		string sql_del;
		ostringstream sqlstream_del;
		sqlstream_del << "delete from list_paramterConfig where INorOut = 0 and DllName = '"<< dllname__ <<"';";
		sql_del = sqlstream_del.str();
		if (SQLITE_OK == access.SqlExe(sql_del.c_str()))
		{
			;
		}
		int nIN = 0;
		access.BeginTransaction();
		std::vector<CString> onerecord;
		for (int k = 0;k < inputPara.size();++k)
		{
			string sql_in;
			ostringstream sqlstream;
			onerecord = inputPara[k];

			//变量名
			wstring vname_ = onerecord[0];
			_bstr_t tem_des = vname_.c_str();
			char* des = (char*)tem_des;
			std::string vname__(des);

			//点名
			wstring pointname_ = onerecord[1];
			tem_des = pointname_.c_str();
			des = (char*)tem_des;
			std::string pointname__(des);

			//类型
			wstring ptype = onerecord[2];
			tem_des = ptype.c_str();
			des = (char*)tem_des;
			std::string ptype__(des);

			//说明
			wstring pexplain = onerecord[3];
			tem_des = pexplain.c_str();
			des = (char*)tem_des;
			std::string pexplain__(des);

			//能否修改参数类型
			std::string strCanTypeModify("");
			if(onerecord.size()>=5)
			{
				wstring pexplain = onerecord[4];
				tem_des = pexplain.c_str();
				des = (char*)tem_des;
				strCanTypeModify =des;
			}

			std::string em("");

			sqlstream << "insert into list_paramterConfig(vname,pname,ptype,vexplain,DllName,INorOut,unitproperty01,unitproperty02,unitproperty03,unitproperty04,unitproperty05)  values('" << vname__<< "','" << pointname__ << "','"<< ptype__<< "','" << pexplain__ << "','" << dllname__<< "'," << nIN<< ",'" << em<<"','" << strCanTypeModify<<"','" << em<<"','" << em<<"','" << em<<"');";
			sql_in = sqlstream.str();

			int re = access.SqlExe(sql_in.c_str());

			if (re != SQLITE_OK)
			{
				_tprintf(L"ERROR: 存储参数列表失败!\r\n");
			}
		}

		if (access.CommitTransaction() != SQLITE_OK)
		{
			_tprintf(L"ERROR: 存储参数列表失败!\r\n");
		}
		return true;
	}

	return false;
}

bool CBEOPLogicManager::SaveOutputParametertoDB(CString strDllName,  std::vector<vec_str> &strLongPara )
{
	std::vector<vec_str> outputPara = strLongPara;

	string strUtf8;
	Project::Tools::WideCharToUtf8(m_strDBFileName, strUtf8);
	CSqliteAcess access(strUtf8);
	if (!access.GetOpenState())
	{
		_tprintf(L"ERROR: Open S3DB File Failed!\r\n");
	}
	else
	{
		//dll name
		wstring dllname = strDllName.GetString();
		_bstr_t tem_des = dllname.c_str();
		char* des = (char*)tem_des;
		std::string dllname__(des);

		string sql_del;
		ostringstream sqlstream_del;
		sqlstream_del << "delete from list_paramterConfig where INorOut = 1 and DllName = '"<< dllname__ <<"';";
		sql_del = sqlstream_del.str();
		if (SQLITE_OK != access.SqlExe(sql_del.c_str()))
		{
			;
		}
		int nIN = 1;
		access.BeginTransaction();
		std::vector<CString> onerecord;
		for (int k = 0;k < outputPara.size();++k)
		{
			string sql_in;
			ostringstream sqlstream;
			onerecord = outputPara[k];

			//变量名
			wstring vname_ = onerecord[0];
			_bstr_t tem_des = vname_.c_str();
			char* des = (char*)tem_des;
			std::string vname__(des);

			//点名
			wstring pointname_ = onerecord[1];
			tem_des = pointname_.c_str();
			des = (char*)tem_des;
			std::string pointname__(des);

			//类型
			wstring ptype = onerecord[2];
			tem_des = ptype.c_str();
			des = (char*)tem_des;
			std::string ptype__(des);

			//说明
			wstring pexplain = onerecord[3];
			tem_des = pexplain.c_str();
			des = (char*)tem_des;
			std::string pexplain__(des);

			wstring conp = onerecord[4];
			tem_des = conp.c_str();
			des = (char*)tem_des;
			std::string con_pointname__(des);

			//能否修改参数类型
			std::string strCanTypeModify("");
			if(onerecord.size()>=6)
			{
				wstring pexplain = onerecord[5];
				tem_des = pexplain.c_str();
				des = (char*)tem_des;
				strCanTypeModify =des;
			}

			std::string em("");
			sqlstream << "insert into list_paramterConfig(vname,pname,ptype,vexplain,DllName,INorOut,unitproperty01,unitproperty02,unitproperty03,unitproperty04,unitproperty05)  values('" << vname__<< "','" << pointname__ << "','"<< ptype__<< "','" << pexplain__ << "','" << dllname__<< "'," << nIN<< ",'" << con_pointname__<<"','" << strCanTypeModify<<"','" << em<<"','" << em<<"','" << em<<"');";
			sql_in = sqlstream.str();
			int re = access.SqlExe(sql_in.c_str());
			if (re != SQLITE_OK)
			{
				_tprintf(L"ERROR: 存储参数列表失败!\r\n");
			}
		}

		//access.CommitTransaction();
		if (access.CommitTransaction() != SQLITE_OK)
		{
			_tprintf(L"ERROR: 存储参数列表失败!\r\n");
		}

		return true;
	}
	return false;
}

bool CBEOPLogicManager::DeleteThreadName( wstring strThreadName )
{
	string strUtf8;
	Project::Tools::WideCharToUtf8(m_strDBFileName, strUtf8);

	CSqliteAcess access(strUtf8);
	if (!access.GetOpenState())
	{
		_tprintf(L"ERROR: Open S3DB File Failed!\r\n");
	}
	else
	{
		string strThreadOldName;
		Project::Tools::WideCharToUtf8(strThreadName, strThreadOldName);

		ostringstream sqlstream;
		sqlstream << "delete from list_dllstore where unitproperty04 = '"<< strThreadOldName <<"';";
		string sql_del = sqlstream.str();
		int re = access.SqlExe(sql_del.c_str());
		access.CloseDataBase();
		if (SQLITE_OK == re)
		{
			return true;
		}
	}

	return false;
}
