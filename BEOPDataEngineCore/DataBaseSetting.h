#pragma once


#include "XSTRING"

using namespace std;

class  DataBaseSetting
{
public:
	DataBaseSetting(void);
	~DataBaseSetting(void);



	// ���ݿ���Ϣ�����ṹ��
		string strDBIP;					//  host��ַ
		string strDBIP2;				//  ����IP
		string strDBName;				//  ���ݿ�����
		string strRealTimeDBName;       //  ʵʱ���ݿ�����
		string strUserName;				//  �û���
		string strDBPsw;				//  ���ݿ�����
		int    nAutoLogin;				//	�Ƿ��Զ���¼
		int    DTUPortNumber;			//	DTU�˿ں�
		int    DTUBaudrate;				//	DTU������
		int    nPort;					//  �˿�


};

