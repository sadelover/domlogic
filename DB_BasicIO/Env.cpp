#include "StdAfx.h"
#include "Env.h"
#include "../Tools/Util//UtilString.h"

Env::Env(void)
{
	m_tTriggerTime = COleDateTime(2000,1,1,0,0,0);
	m_bActOnce = false;
}


Env::~Env(void)
{
}


void Env::UpdateTime(COleDateTime tCheckTime, string strTime)
{
	if(strTime.length()<=0)
		return;

	COleDateTime tTrigger = tCheckTime;
	if(strTime.length()>0)
	{
		vector<int> nnList;
		UtilString::SplitString(strTime, ":", nnList);
		if(nnList.size()>=2)
		{
			if(nnList[0]<0 || nnList[0]>=48)
				return;

			if(nnList[0]<0 || nnList[0]>59)
				return;

			if(nnList[0]>23 && nnList[0]<48)
			{
				int nHour = nnList[0]-24;
				int nMinute = nnList[1];
				tTrigger.SetDateTime(tCheckTime.GetYear(), tCheckTime.GetMonth(), tCheckTime.GetDay(), nHour, nMinute, 0);

				m_tTriggerTime = tTrigger+ COleDateTimeSpan(1,0,0,0);
				return;
			}

			int nHour = nnList[0];
			int nMinute = nnList[1];
			tTrigger.SetDateTime(tCheckTime.GetYear(), tCheckTime.GetMonth(), tCheckTime.GetDay(), nHour, nMinute, 0);
			
			m_tTriggerTime = tTrigger;
		}
	}

}