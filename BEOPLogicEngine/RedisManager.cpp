#include "StdAfx.h"
#include "RedisManager.h"

#include "../redis/hiredis.h"
#include "../Tools/CustomTools/CustomTools.h"

CRedisManager::CRedisManager(void)
{
}


CRedisManager::~CRedisManager(void)
{
}


bool CRedisManager::init()
{
	unsigned int j;
	redisContext *c;
	redisReply *reply;

	bool bRedisConnected = true;

	struct timeval timeout = { 1, 500000 }; // 1.5 seconds
	c = redisConnectWithTimeout((char*)"127.0.0.1", 6379, timeout);
	if (c->err) {
		bRedisConnected=  false;
	}

	redisFree(c);

	m_bRedisValid = bRedisConnected;
	return bRedisConnected;
}

bool CRedisManager::is_valid()
{
	//Connect Redis
	return m_bRedisValid;
}


bool CRedisManager::set_value(wstring wstrKey, wstring wstrValue)
{
	unsigned int j;
	redisContext *c;
	redisReply *reply;

	bool bSuccess = false;

	bool bRedisConnected = true;

	struct timeval timeout = { 1, 500000 }; // 1.5 seconds
	c = redisConnectWithTimeout((char*)"127.0.0.1", 6379, timeout);
	if (c->err) {
		bSuccess=  false;

	}
	else
	{

		string strKey_utf8, strValue_utf8;
		strKey_utf8 = Project::Tools::WideCharToAnsi(wstrKey.c_str() );

		strValue_utf8 = Project::Tools::WideCharToAnsi(wstrValue.c_str());
		reply = (redisReply *)  redisCommand(c,"SET %s %s",strKey_utf8.data(), strValue_utf8.data());
		freeReplyObject(reply);

		bSuccess = true;
	}


	redisFree(c);     

	return bSuccess;
}


//
//bool CRedisManager::get_value(wstring wstrKey, wstring & wstrValue)
//{
//	if(wstrKey==_T("LogicThread::heartbeat::XXX"))
//		int xxx=0;
//
//	unsigned int j;
//	redisContext *c;
//	redisReply *reply;
//
//	bool bSuccess = false;
//
//	bool bRedisConnected = true;
//
//	struct timeval timeout = { 1, 500000 }; // 1.5 seconds
//	c = redisConnectWithTimeout((char*)"127.0.0.1", 6379, timeout);
//	if (c->err) {
//		bSuccess=  false;
//
//	}
//	else
//	{
//
//		//用get命令获取数据
//		char szRedisBuff[256] = { 0 };
//		string strName = Project::Tools::WideCharToAnsi(wstrKey.c_str());
//		sprintf_s(szRedisBuff, "GET %s", strName.data());
//		reply = (redisReply*)redisCommand(c, "GET XXX");
//		if (NULL == reply)
//		{
//			printf("Get data Error!");
//			bSuccess=  false;
//		}
//		else if (NULL == reply->str)
//		{
//			freeReplyObject(reply);
//			bSuccess=  false;
//		}
//		else
//		{
//
//			string strRes(reply->str);
//			freeReplyObject(reply);
//
//			Project::Tools::UTF8ToWideChar(strRes, wstrValue);
//			bSuccess = true;
//		}
//	}
//
//
//	redisFree(c);     
//
//	return bSuccess;
//}


bool CRedisManager::set_value_mul(vector<wstring> wstrKeyList, vector<wstring> wstrValueList)
{
	if(wstrKeyList.size()!= wstrValueList.size())
		return false;

	unsigned int j;
	redisContext *c;
	redisReply *reply;

	bool bSuccess = false;

	bool bRedisConnected = true;

	struct timeval timeout = { 1, 500000 }; // 1.5 seconds
	c = redisConnectWithTimeout((char*)"127.0.0.1", 6379, timeout);
	if (c->err) {
		bRedisConnected=  false;

		bSuccess  = false;
	}
	else
	{
		for(int i=0;i<wstrKeyList.size();i++)
		{
			wstring wstrKey = wstrKeyList[i];
			wstring wstrValue = wstrValueList[i];

			string strKey_utf8, strValue_utf8;
			strKey_utf8 = Project::Tools::WideCharToAnsi(wstrKey.c_str() );

			strValue_utf8 = Project::Tools::WideCharToAnsi(wstrValue.c_str());
			reply = (redisReply *)  redisCommand(c,"SET %s %s",strKey_utf8.data(), strValue_utf8.data());
			freeReplyObject(reply);

		}

		bSuccess = true;
	}
	
	redisFree(c);     
	return bSuccess;
}