
// -*- c++ -*-
// ***********************************************************
// Copyright (C) 2013 R&B
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Module:   
// Author:   david
// Date:     2010/7/13
// Revision: 1.0
//
// ***********************************************************
#ifndef DBACCESSCONTAINER_H
#define DBACCESSCONTAINER_H

#pragma once
#include <map>
#include <vector>
#include <utility>
#include <string>

#include "DBInterfaceImp.h"
#include "DatabaseSession.h"

#include "../Tools/CustomTools/CustomTools.h"
#include "../Tools/NormalDefines.h"

#include "RunningRecordOperation.h" //bird add

using std::string;
using std::wstring;
using std::map;
using std::multimap;
using std::vector;
using std::pair;
using Project::Tools::Mutex;
using Project::Tools::Scoped_Lock;
;
#define TEMPLATEDATABASE_CREATE "Beop_template";

typedef struct tag_Project_DB
{
	wstring		strprojectname;		//	��Ŀ����,��Ҫ�ڽ�������ʾ
	string		strdatabasename;	//	���ݿ�����
}Project_DB;

//����������ݵ���Ϣ
typedef struct tag_StartUpInfo
{
	string		bpicture;			//	��ĿͼƬ
	Project_DB  projectdb;		//	���ݿ�������Ŀ��
}StartUpInfo;

typedef struct tag_BeopUserInfo
{
	wstring		username;
	int			readright;
	int			writeright;
	int			modelright;
	int			simulateright;
	wstring		lastlogintime;
}BeopUserInfo;

class DBDLL LoadRecord
{
public:
	SYSTEMTIME recordtime;
	double	   loadvalue;
};


//���ݿ������
class DBDLL DBAccessContainer : public CDatabaseSeesion
{
private:
	DBAccessContainer();
public:
	~DBAccessContainer();
	static DBAccessContainer *GetInstance();
	static void DestroyInstance();

public:


	// ������¼
	bool		InsertWarningRecord(int warningindex);	// ���뾯����¼
	bool		InsertWarningRecord(SYSTEMTIME st, const wstring& warninginfo);	
	// �û�������¼
	bool		InsertOperationRecord(const SYSTEMTIME& st, const wstring& remark, const wstring& wstrUser);
	
	//Beop_USER.users���д�ӿ�
	bool	DeleteUser(wstring username);			//ɾ��һ��user
	bool	ReadAllUsers(vector<wstring>& userlist);	//�����е�user�б�
	bool	ReadUserInfo(wstring username, BeopUserInfo* puserinfo);	// ��user����Ϣ
	bool	ReadALLUserInfo(vector<BeopUserInfo*>& userlist);
	bool	ValidateUser(wstring username, wstring password); // �ж��û��Ƿ����
	
    bool    ReadAllOperationRecord(VEC_OP_RECORD&  vecVecOperation, const COleDateTime& timeStart, const COleDateTime& timeStop);
	
	bool    ReadWarningRecord(WARNING_LIST& warninglist, const COleDateTime& timebegin, const COleDateTime& timeend);
    //bird add, read power consumption total

	vector<wstring> ReadWarningConfigList();
	
	void	LogDBError(const char* sql);
	void	LogDBError(const string& sql);
	
private:

	static Mutex               m_mutex;

    Mutex       m_recordmutex;
	static DBAccessContainer * g_DBAccessContainer;
};
#endif