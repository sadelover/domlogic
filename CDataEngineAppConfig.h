

/*
 * Data Engine����ʱ�Ķ�̬������.
 * ���Զ�̬�޸ĵĲ���.
 * �������ʱ��д�뵽ini�ļ�.����ʱ��ini�ļ���load.
 *
 * ����ཫ��ʵ��Ϊsingleton
 */

#pragma once

#include <Tools/CustomTools/CustomTools.h>
#include <string>
using std::wstring;

struct MDODBInfo
{
	wstring strHost;
	wstring strUser;
	wstring strPwd;
	wstring strSchema;
	int		nPort;
};

class CDataEngineAppConfig
{
private:
	CDataEngineAppConfig(void);

public:
	static CDataEngineAppConfig* GetInstance();
	static void DestroyInstance();

	~CDataEngineAppConfig(void);

	int	GetModbusSendCommandInternal() ;
	void SetModbusSendCommandInternal(int sendinternal);

	wstring GetAnotherServerIP() ;
	void SetOtherServerIP(const wstring& ip_other);

	wstring GetGateWayIP();
	void SetGateWayIP(const wstring& ip_gateway);

	int GetMasterSlaveEnable();
	void SetMasterSlaveEnable(int enablevalue);

	wstring	GetIniFileFullPath();

private:
	int		m_modbus_sendcommand_internal;			//modbus�����ʱ����
	int		m_heartbeat_checkinternal_seconds;		//����������ʱ����
	int		m_master_deadjudgement_seconds;			//�������ߵĳ���ʱ��,����ж��������߳�����һʱ���趨,����Ϊ��������.
	int		m_masterslave_enable;	//�Ƿ�����master��slave����ģʽ��
	std::wstring m_ip_other;	//��һ̨��������ip
	std::wstring m_ip_gateway;	//���ص�ip

	Project::Tools::Mutex	m_lock;

	static CDataEngineAppConfig* m_pconfiginstance;
	static Project::Tools::Mutex m_singleton_lock;
};

