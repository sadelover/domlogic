#pragma once

#include "VECTOR"
#include "DataBaseSetting.h"
#include "../Tools/CustomTools/CustomTools.h"
#include "../BEOPLogicEngine/RedisManager.h"

class CDLLObject;



class  CLogicPipeline
{
public:
	CLogicPipeline(CString strThreadName);
	~CLogicPipeline(void);

	bool Init();
	bool Exit();
	bool ActLogic(int nActCount);
	void PushLogicObject(CDLLObject *pObject);
	bool FindDllObject(CDLLObject *pObject);

	CDLLObject * GetLogicObject(int nIndex);
	int GetLogicDllCount();

	double GetTimeSpanSeconds();
	void SetTimeSpanSeconds(double fSeconds);
	//°´Ë³ÐòÅÅµÄdll

	CString GetLineString();

private:
	std::vector<CDLLObject*> m_vecImportDLLList;
	Project::Tools::Mutex	m_lock_timespan;
	double m_fTimeSpanSeconds;

	CString m_strThreadName;


	CRedisManager m_Redis;
};

