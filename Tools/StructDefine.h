#pragma once

enum E_MODBUS_VALUE_TYPE
{
	E_MODBUS_SIGNED = 0,
	E_MODBUS_UNSIGNED,
	E_MODBUS_BITE,
	E_MODBUS_LONG,
	E_MODBUS_LONG_INVERSE,
	E_MODBUS_FLOAT,
	E_MODBUS_FLOAT_INVERSE,
	E_MODBUS_DOUBLE,
	E_MODBUS_DOUBLE_INVERSE,
	E_MODBUS_STRING,
	E_MODBUS_STRING_INVERSE,
	E_MODBUS_POWERLINK,			//11 争对powerLink 3位寄存器，前两位可读，后一位只写
};

struct _ModbusReadUnit
{
	wstring strPointName;
	unsigned int nSlaveId;
	DWORD dwAddFrom;
	DWORD dwAddTo;
	DWORD dwFuncCode;
	int nReadSuccessCount;
	bool	bHasErrPoint;
	bool	bMultiRead;			//true 是批量读取  false 是单点读取   批量读取错误 切换到单点读一次 再切回批量
	bool	bModbusEnd;
};

struct _ModbusWriteCmd
{
	WORD dwAddFrom;
	WORD dwFuncCode;
	WORD dwValue;
};

enum E_CO3P_PARAMS_TYPE
{
	E_CO3P_IP = 1,
	E_CO3P_ADDR1,
	E_CO3P_ADDR2,
	E_CO3P_TYPE,
	E_CO3P_CMD,
	E_CO3P_INTEGER,
	E_CO3P_DECIMAL,
};

struct _CKECO3PReadUnit
{
	_CKECO3PReadUnit()
	{
		memset(cRecBuffer,0,1024);
		nLength = 0;
		bResponseSuccess = false;
	}
	wstring strPointName;
	WORD	wAddr1;				//管理器地址
	WORD	wAddr2;				//仪表地址
	WORD	wType;				//仪表类型
	WORD	wCmd;				//仪表操作命令
	char	cRecBuffer[1024];	//收到的数据包内容
	int		nLength;			//收到的数据包长度
	int		nReadSuccessCount;
	bool	bResponseSuccess;	//是否成功返回
};

struct _MysqlCrossTableUnit
{
	_MysqlCrossTableUnit()
	{
		nColumnCount = 0;
		nFilterColumnIndex = 0;
		vecFilter.clear();
	}
	wstring strTableName;
	wstring strOrderColumn;
	wstring strFilterColumn;
	int		nColumnCount;
	int		nFilterColumnIndex;
	vector<wstring> vecFilter;
};
