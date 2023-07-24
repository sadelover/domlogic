

#include "stdafx.h"

#include "PackageTypeDefine.h"

namespace rnbtech
{
	const int PACKAGE_PREFIX_LEN = 13;
	const int PACKAGE_PREFIX_LEN_TIME = 5;
	const int PACKAGE_PREFIX_LEN_DSC_INFO = 20;
	const char* POINTDATAPACKAGE_PREFIX = "pdp:";
	const char* OPERATIONRECORDPACKAGE_PREFIX = "orp:";
	const char* CALCRESULTPACKAGE_PREFIX = "crp:";
	const char* POINTDATAPACKAGE_PREFIX_TIME = "time:";
	const char* DSC_INFO_PREFIX	= "[INFO]: DSC DATA";
	const char* WARNINGDATAPACKAGE_PREFIX = "wdp:";				//�������ݰ�ͷ
	const char* WARNINGOPERATIONPACKAGE_PREFIX = "wop:";		//����������ͷ
	const char* DTUSERVETCMD_PREFIX = "DTUServerCmd:";			//DTUServer����
	const char* DTUSERVETFRL_PREFIX = "DTUServerFrl:";			//DTUServer�ļ�����Log
	const char* DTUSERVETFUP_PREFIX = "DTUServerFup:";			//DTUServer�ļ�����
	const char* WEBSERVETFUP_PREFIX = "WebServerFup:";			//WEBServer�ļ�����
	const char* DTUWRITECMD_PREFIX = "DTUWriteTime:";			//DTU����ʱ��
}

//DTU�豸����ǰ �ظ��������ֽ� ��V32-S507C�汾���ԣ�
//rnbtech::CPackageType::DTU_DATATYPE rnbtech::CPackageType::GetPackageType( const char* buffer )
//{
//	ASSERT(buffer != NULL);
//	int bufferlen = strlen(buffer);
//	if (bufferlen <= PACKAGE_PREFIX_LEN+PACKAGE_PREFIX_LEN_DSC_INFO){
//		return Type_InValid;
//	}
//
//	char buffer_dsc_prefix[PACKAGE_PREFIX_LEN_DSC_INFO-3];
//	memcpy(buffer_dsc_prefix, buffer+2, PACKAGE_PREFIX_LEN_DSC_INFO-4);
//	buffer_dsc_prefix[PACKAGE_PREFIX_LEN_DSC_INFO-4] = '\0';
//
//	if(strcmp(buffer_dsc_prefix, DSC_INFO_PREFIX) == 0)			//���ֽ�Ϊ [INFO]: DSC DATA
//	{
//		char buffer_prefix[PACKAGE_PREFIX_LEN+1];
//		memcpy(buffer_prefix, buffer+PACKAGE_PREFIX_LEN_DSC_INFO, PACKAGE_PREFIX_LEN);
//		buffer_prefix[PACKAGE_PREFIX_LEN] = '\0';
//
//		if(strcmp(buffer_prefix, DTUSERVETCMD_PREFIX) == 0)
//		{
//			return Type_DTUServerCmd;
//		}
//	}
//	return Type_InValid;
//}
//
//char* rnbtech::CPackageType::RemovePrefix( char* buffer )
//{
//	DTU_DATATYPE datatype = GetPackageType(buffer);
//	if (datatype != Type_InValid)
//	{
//		if (datatype == Type_PointData_WithTime)
//		{
//			int len = strlen(buffer);
//			memmove(buffer,buffer+29,len-29);
//			buffer[len-29]='\0';
//		}
//		else if(datatype == Type_DTUServerCmd)		//13
//		{
//			int len = strlen(buffer);
//			memmove(buffer,buffer+33,len-33);
//			buffer[len-33]='\0';
//		}
//		else
//		{
//			int len = strlen(buffer);
//			memmove(buffer,buffer+24,len-24);
//			buffer[len-24]='\0';
//		}
//		
//	}
//
//	return buffer;
//}

//DTU�豸������ �ظ��������ֽ� ��V32-S507C�汾���ԣ�
rnbtech::CPackageType::DTU_DATATYPE rnbtech::CPackageType::GetPackageType( const char* buffer )
{
	ASSERT(buffer != NULL);
	int bufferlen = strlen(buffer);
	if (bufferlen <= PACKAGE_PREFIX_LEN){
		return Type_InValid;
	}

	char buffer_prefix[PACKAGE_PREFIX_LEN+1];
	memcpy(buffer_prefix, buffer, PACKAGE_PREFIX_LEN);
	buffer_prefix[PACKAGE_PREFIX_LEN] = '\0';

	if(strcmp(buffer_prefix, DTUSERVETCMD_PREFIX) == 0)
	{
		return Type_DTUServerCmd;
	}
	if(strcmp(buffer_prefix, DTUSERVETFRL_PREFIX) == 0)
	{
		return Type_DTUServerFrl;
	}
	if(strcmp(buffer_prefix, DTUSERVETFUP_PREFIX) == 0)
	{
		return Type_DTUServerFup;
	}
	else if(strcmp(buffer_prefix, DTUWRITECMD_PREFIX) == 0)
	{
		return Type_DTUWriteTime;
	}
	else if(strcmp(buffer_prefix, WEBSERVETFUP_PREFIX) == 0)
	{
		return Type_WebServerFup;
	}

	return Type_InValid;
}

int rnbtech::CPackageType::RemovePrefix( char* buffer  ,int length)
{
	DTU_DATATYPE datatype = GetPackageType(buffer);

	if(length == -1)
	{
		length = strlen(buffer);
	}

	if (datatype != Type_InValid)
	{
		if (datatype == Type_PointData_WithTime)
		{
			length = length-9;
			memmove(buffer,buffer+9,length);
			buffer[length]='\0';
		}
		else if(datatype == Type_DTUServerCmd || datatype == Type_DTUServerFup || datatype == Type_DTUServerFrl || datatype == Type_WebServerFup)		//13
		{
			length = length-13;
			memmove(buffer,buffer+13,length);
			buffer[length]='\0';
		}
		else if(datatype == Type_DTUWriteTime)		//13
		{
			length = length-13;
			memmove(buffer,buffer+13,length);
			buffer[length]='\0';
		}
		else
		{
			length = length-4;
			memmove(buffer,buffer+4,length);
			buffer[length]='\0';
		}
	}
	return length;
}
