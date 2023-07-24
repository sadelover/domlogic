#pragma once
#include "STRING"

using namespace std;

class Env
{
public:
	Env(void);
	~Env(void);


	string m_strTriggerTime;
	int m_nTriggerTimeType; //1:固定时间，0:系统点名时间
	int m_nID;
	bool m_bActOnce;

	void UpdateTime(COleDateTime tCheckTime, string strTime);

	COleDateTime GetTriggerOleTime(){return m_tTriggerTime;}
private:
	COleDateTime m_tTriggerTime;
};

