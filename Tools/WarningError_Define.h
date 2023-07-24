#pragma once

#define WARNING_ID_START	10000
#define WARNING_ID_END		20000

#define CANCELWARNING_MULTIPLE	4

//#define STRING_CONNECT_PLC_FAIL _T("��PLC���ӶϿ���������������")

//////////////////////////////////////////////////////////////////////////////////////////////////////
//			������ֵ�������������ݿ�CS.warniggroup���趨��������ݿ��ֵ�ı䣬����ö��Ҳ��Ҫͬ��			//
//////////////////////////////////////////////////////////////////////////////////////////////////////
enum Error_System			//ϵͳ����
{
	Error_Interbus		= 10001,
	Error_PLCMode,
	Error_Connect_PLC_Faied,
	Error_DB_opt_failed,
	Error_DB_InstSensor,			//���ݿ���봫��������
	Error_DB_InstCalc,				//���ݿ�����Ż��������
	Error_DB_InstOpt,				//���ݿ�����û���������
	Error_Connect_CH_Failed,
	Error_PLC_Value_Read_Failed,
	Error_PLC_Value_Write_Failed,
};

enum Error_Function			//���ܲ�������
{
	Optimize_failed		= 10101,
	Start_System_Failed,
	Stop_System_Failed,
	Standby_Failed,
	Open_CH_Failed,
	Open_CHValve_Failed,
	Open_Tower_Failed,
	Open_CWP_Failed,
	Open_PriChwp_Failed,
	Open_SecChwp_Failed,
	Open_ThrChwp_Failed,
	Shut_CH_Failed,
	Shut_CHValve_Failed,
	Shut_Tower_Failed,
	Shut_CWP_Failed,
	Shut_PriChwp_Failed,
	Shut_SecChwp_Failed,
	Shut_ThrChwp_Failed,
	CoolDown_Failed,
};

enum Error_CH				//�������
{
	Warning_CH_Start		= 10200,
	Warning_CH_1			= 10201,
};

enum Error_CHValve			//������Ź���
{
	Error_CH_Valve_Start	= 10300,
	Error_CH_Valve_1		= 10301,
};

enum Error_Tower			//��ȴ������
{
	Error_CT_Start			= 10400,
	Error_CT_1 				= 10401,
};

enum Error_TowerFan			//��ȴ���������
{
	CT_Fan_Overflow_Start	= 10500,
	CT_1_Fan_Overflow		= 10501,
};

enum Error_Tower_VSD		//��ȴ����Ƶ������
{
	Error_Tower_VSD_Start	= 20500,
	Error_Tower_1_VSD		= 20501,
};


enum Error_CWP				//��ȴˮ�ù���
{
	Error_CWP_Start			= 10600,
	Error_CWP_1				= 10601,
};

enum Error_CWP_VSD			//��ȴˮ�ñ�Ƶ������
{
	Error_CWP_Transducer_Start	= 10700,
	Error_CWP_1_Transducer		= 10701,
};


enum Error_PriChwp			//һ���䶳ˮ�ù���
{
	Error_PriChwp_Start		= 10800,
	Error_PriChwp_1			= 10801,
};

enum Error_PriChwp_VSD		//һ���䶳ˮ�ñ�Ƶ������
{
	Error_PriChwp_VSD_Start	= 10900,
	Error_PriChwp_1_VSD		= 10901,
};

enum Error_SecChwp			//�����䶳ˮ�ù���
{
	Error_SecChwp_Start		= 11000,
	Error_SecChwp_1			= 11001,
};

enum Error_SecChwp_VSD		//�����䶳ˮ�ñ�Ƶ������
{
	Error_SecChwp_VSD_Start	= 11100,
	Error_SecChwp_1_VSD		= 11101,
};

enum Error_ThrChwp			//�����䶳ˮ�ù���
{
	Error_ThrChwp_Start		= 11200,
	Error_ThrChwp_1			= 11201,
};

enum Error_ThrChwp_VSD		//�����䶳ˮ�ñ�Ƶ������
{
	Error_ThrChwp_VSD_Start	= 11300,
	Error_ThrChwp_1_VSD		= 11301,
};

enum Warning_CH_Surge		//�䶳������
{
	Warning_Surge_CH_Start	= 11400,
	Warning_Surge_CH_1		= 11401,
};

enum AutoErr_CHValve		//�������ģʽ����
{
	Error_CHValve_notAuto_Start	= 11500,
	Error_CHValve_1_notAuto		= 11501,
};

enum AutoErr_CT				//��ȴ��ģʽ����
{
	Error_CT_notAuto_Start	= 11600,
	Error_CT_1_notAuto		= 11601,
};

enum AutoErr_CWP			//��ȴˮ��ģʽ����
{
	Error_CWP_notAuto_Start	= 11700,
	Error_CWP_1_notAuto		= 11701,
};

enum AutoErr_PriCHWP		//һ�α�ģʽ����
{
	Error_PriCHWP_notAuto_Start	= 11800,
	Error_PriCHWP_1_notAuto		= 11801,
};

enum AutoErr_SecCHWP		//���α�ģʽ����
{
	Error_SecCHWP_notAuto_Start	= 11900,
	Error_SecCHWP_1_notAuto		= 11901,
};

enum AutoErr_ThrCHWP		//���α�ģʽ����
{
	Error_ThrCHWP_notAuto_Start	= 12000,
	Error_ThrCHWP_1_notAuto		= 12001,
};

enum CT_Valve_Error
{
	Error_CTValve_Error_Start = 12100,
	Error_CTValve_1_Error = 12101,
};

enum AutoErr_CTValve		//�������ģʽ����
{
	Error_CTValve_notAuto_Start	= 12200,
	Error_CTValve_1_notAuto		= 12201,
};


enum NetWorkConnectionError
{
	Error_NetWorkError_Start = 13000,
	Error_NetWorkError_Disconnected,
};

enum Pump_Pressure_Error
{
	Error_PumpPressure_Start	= 14000,
	Error_PumpPressure_1_InValid	= 14001,
};

#define  NETWORKERRORID  (WARNING_ID_START + 10)
