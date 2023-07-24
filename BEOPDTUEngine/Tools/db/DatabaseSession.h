// -*- c++ -*-
// ***********************************************************
//
// Copyright (C) 2013 R&B
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Module:   
// Author:   zhu huawei
// Date:     2010/7/13
// Revision: 1.0
//
// ***********************************************************

#ifndef DATABASESESSION_H
#define DATABASESESSION_H

#include <stdio.h>
#include <comutil.h>
#include <WinSock.h>
#include "./mySqlInclude/mysql.h"
#include "../CustomTools.h"
#include <string>
using std::string;
using std::wstring;

#define BUFFER_LEN 512

typedef struct tag_DataBaseParam
{// ���ݿ���Ϣ�����ṹ��
	tag_DataBaseParam()
		:nPort(0)
	{}
	string strDBIP;        //  host��ַ
	string strDBIP2;	   //  ����IP
	string strDBName;      //  ���ݿ�����
	string strRealTimeDBName;      //  ʵʱ���ݿ�����
	string strUserName;    //  �û���
	int    nPort;          //  �˿�
	int    nAutoLogin;
	int    nDTUPortNumber;
	int    nDTUBaudrate;
}gDataBaseParam;

//���ݿ����
typedef struct Database_Param
{
  const char*	host;			//������
  const char*	user;			//�û���
  const char*	password;		//����
  const char*	db;				//���ݿ���
  unsigned int	port;			//�˿ڣ�һ��Ϊ0
  const char*	unix_socket;	//�׽��֣�һ��ΪNULL
  unsigned int	client_flag;	//һ��Ϊ0
}DatabaseParam, *pDatabaseParam;

//���ݿ����
typedef struct Database_Config
{
	string strDBIP;				//  host��ַ
	string strDBName;			//  ���ݿ�����
	string strUserName;			//  �û���
	string strPsw;				//  ����
	int    nPort;				//  �˿�
}DatabaseConfig;

//���ݿ��������
typedef struct Data_Param
{
  char*	db_name;			//���ݿ���
  char*	tab_name;			//����
  char*	col_name;			//����
  char*	select_exp;			//ѡ����ʽ
  char*	where_def;			//ѡ���޶�����
  char*	insert_val;			//����ֵ
  char*	set_exp;			//�������ݿ����ݵı��ʽ
}DataParam, *pDataParam;

/*
 This class will holds a connection to the database, and provide 
 a lot of database operations , such as query, insert and update. 
*/

enum MYSQL_CHARACTERSET
{
	MYSQL_UTF8,    // utf8����
	MYSQL_LATIN1,  // ��ŷ�����ַ���
	MYSQL_GBK,     // GBK�ַ���
	MYSQL_GB2312   // GB2312.
};

class CDatabaseSeesion
{
public:
  CDatabaseSeesion();
  ~CDatabaseSeesion();

public:
  
  /*
  ȡ��mysql�Ĵ�����Ϣ
  */
  const char * GetMysqlError();					//���������Ϣ

  /*
  ȡ�ÿͻ�����Ϣ
  */  
  const char * GetClientInfo( void );			

  /*
  ȡ��������Ϣ
  */    
  const char * GetHostInfo( void );
  
  /*
  ȡ�÷�������Ϣ
  */    
  const char * GetServerInfo( void );

  /*
  ȡ�÷�����״̬��Ϣ
  */    
  const char * GetServerState( void );

  /*
  �������ݿ�
  @param db, ���ݿ���
  @return    ������
  */    
  int CreateDB( char * db );		

  /*
  ɾ�����ݿ�
  @param db, ���ݿ���
  @return    ������
  */      
  bool DropDB(const char * db );					

  /*
  ȡ��Э����Ϣ
  @param
  @return  ������
  */        
  int GetProtocolInfo( void );				//Э����Ϣ

  /*
  ȡ��Field������
  @param FieldNum, field����������0��ʼ
  @return  ����
  */        
  char *GetFieldName( int FieldNum );		

  /*
  ȡ�ý����������,һ������ȡ�ý������
  @param 
  @return  ����
  */          
  unsigned int GetFieldNum( void );		

  /*
  ȡ�ý����������,һ������ȡ�ý������
  @param 
  @return  ������ĵ�����
  */            
  my_ulonglong GetRowNum( void );			

  /*
  �����������ÿһ��,һ������ȡ�ý������
  @param 
  @return  ÿһ�е�ֵ������β����NULL
  */              
  MYSQL_ROW GetRow( void );			

  /*
  �ڲ�ѯ�������Ѱ�������С�ƫ��ֵΪ�кţ���Χ��0��mysql_num_rows(result)-1��
  �ú���Ҫ�������ṹ������ѯ�����н������ˣ�so mysql_data_seek()��Ӧ��
  mysql_store_result()����ʹ�ã���������mysql_use_result()��
  */
  void SeekData( int offset );	

  /*
  �ͷŲ�ѯ�Ľ��������ѯ���õ�һ��������󣬱����ͷţ���������ڴ�й¶
  @param 
  @return 
  */
  void FreeResultSet( void );					//�ͷŽ����

  /*
  ���ӵ�ָ�����ݿ�,�ú������Զ��رյ�ǰ������
  @param 
  @return 
  */  
  bool Connect( Database_Param *pDbParam );	//�������ݿ�

  //���ӵ����ݿ�
  bool ConnectToDB( const wstring& host, const wstring& username, 
					const wstring& password, const wstring& database,
					unsigned int port, int connectionflags = 0);

  /*
  �Ͽ������ݿ������
  @param 
  @return 
  */    
  void DisConnectFromDB();

  /*
  ʹ��dbָ�������ݿ��Ϊ��mysqlָ���������ϵ�Ĭ�����ݿ⣨��ǰ���ݿ⣩��
  �ں�����ѯ�У������ݿ⽫��δ������ȷ���ݿ����ַ��ı����õ�Ĭ�����ݿ⡣
  ���������ӵ��û�����ʹ�����ݿ��Ȩ�ޣ�����mysql_select_db()��ʧ�ܡ�
  @param 
  @return 
  */
  bool SelectDB( const char* dbname );


  /*
  ����ָ��sql���.
  @param command ��Ҫ���е�sql��������һ��select��insert��delete

  @return true��ʾ�ɹ���ʧ��Ϊfalse
  */  
  bool Execute(const char* command);

/*
  ����ָ��sql���.
  @param command ��Ҫ���е�sql��������һ��select��insert��delete

  @return true��ʾ�ɹ���ʧ��Ϊfalse
  */  
  bool Execute(const string& command);

  
  /*
  ��ѯ���洢�����������
  @param str ��ѯ�ַ���
  @return true ��ʾ�ɹ�����֮Ϊfalse
  */
  bool QueryAndStoreResult(const char *str );	


  //����5�������ѹ���
  bool UpdateRecord( Data_Param *para );		//���¼�¼
  bool SelectRecord( Data_Param *para );		//ѡ���¼
  bool InsertRecord( Data_Param *para );		//�����¼
  bool DeleteRecord( Data_Param *para );			//ɾ����¼
  bool SelectAll( Data_Param *para );			//ѡ�����м�¼

  /* �����ϴ�UPDATE���ĵ��������ϴ�DELETEɾ�������������ϴ�INSERT�������������
   ����UPDATE��DELETE��INSERT��䣬����mysql_query()�����̵��á�
   ����SELECT��䣬mysql_affected_rows()�Ĺ�����ʽ��mysql_num_rows()���ơ�
  */
  my_ulonglong GetAffectedRows(void);

  /*
  ���ģʽΪ��1��������autocommitģʽ��
  ���ģʽΪ��0������ֹautocommitģʽ��
  �ɹ�����true��ʧ�ܷ���false
  */
  bool SetAutoCommit(bool isautocommit);

  /*
  �ú����ṩ�˹���Ĭ�Ͽͻ����ַ�������Ϣ��
  ����ʹ��mysql_set_character_set()��������Ĭ�ϵ��ַ�����
  �ú�������MySQL 5.0.10�����ӵġ�
  */
  void GetCharacterSetInfo(MY_CHARSET_INFO* cs);

  /*
  �ύ��ǰ����
  return true��ʾ�ɹ���false��ʾʧ��
  */
  bool Commit(void);

  /*
  ��ʼһ������
  */
  bool StartTransaction();

  /*
  �ع���ǰ����
  */
  bool RollBack();

  /*
  ���õ�ǰ�����ַ���
  */
  bool SetConnectionCharacterSet(MYSQL_CHARACTERSET charset = MYSQL_UTF8);

  /*
  �������ݿ�,�ú����ᴴ��һ����Ϊdestdb�����ݿ⣬Ȼ��sourcedb�����б�ṹ��
  ���ݿ����������ݿ⡣
  */
  bool CopyDataBase(const char* destdb, const char* sourcedb);

  //ֻ������ṹ������������
  bool CopyDatabaseWithNoData(const char* destdb, const char* sourcedb);

  //ÿ����ѯ����һ�������
  MYSQL_RES* QueryAndReturnResult(const char* sql);
	
  MYSQL_ROW  FetchRow(MYSQL_RES* result);
  
  my_ulonglong GetRowNum(MYSQL_RES* result);

  void  FreeReturnResult(MYSQL_RES* result);

  bool TruncateTable(const char* tablename);

  MYSQL_FIELD* FetchField(MYSQL_RES* result);

  int	GetFieldNum(MYSQL_RES* result);

  unsigned long* GetFieldLengths(MYSQL_RES* result);

  string GetCreateTableSQL(string dbname, string tablename);

  //ȡ�õ�ǰmysql�İ�װĿ¼
  string	GetMysqlInstallPath();
	
  char * GetDBName();
	
  bool	 IsConnected();
  unsigned long RealEscapeString(char *pDest, const char *pSrc, unsigned long ulLen);
	bool Execute(const string& command, unsigned long ulLen);

	bool	AddMysqlErrLog(string strOperation, string strErrInfo);

	Database_Config GetDatabaseParam();

protected:
  MYSQL			m_mysql;	//���ݿ����Ӿ��
  MYSQL_ROW		m_row;		//��¼��
  MYSQL_RES		*m_pQuery;	//�����
  MYSQL_FIELD		*m_pField;	//�ֶ���Ϣ���ṹ�壩
  char m_szStrSQL[BUFFER_LEN];	
  bool m_isconnected;
  Database_Config	m_dbparam;		//���ݿ�������Ϣ
  private:
	  Project::Tools::Mutex	m_lock;
};

#endif //_DATABS_H
