#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////////
//			������ֵ�������������ݿ�CS.optgroup���趨��������ݿ��ֵ�ı䣬����ö��Ҳ��Ҫͬ��			//
//////////////////////////////////////////////////////////////////////////////////////////////////////
enum Operations
{
	Opt_StartSystem		= 1,	//�Զ�����
	Opt_StopSystem,				//�Զ��ػ�
	Opt_System_Standby_Auto,	//�Զ�����
	Opt_System_Standby_manu,	//�ֶ�����
	Opt_Optimize,				//�Ż�����
	Opt_Optimize_Cwp_Twb_Low,	//ʪ���¶ȵ�ʱ����ȴˮ������
	Opt_Switch_Auto,			//�л����Զ�
	Opt_Switch_Manu,			//�л����ֶ�
	Opt_CoolDown,				//Ԥ��
	Opt_AppStart,				//�����Ѿ�����
};