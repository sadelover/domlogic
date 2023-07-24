#pragma once

#include "xstring"
#include "VECTOR"
using namespace std;

class CRedisManager
{
public:
	CRedisManager(void);
	~CRedisManager(void);

	bool init();
	bool is_valid();
	bool set_value(wstring wstrKey, wstring wstrValue);
	//bool get_value(wstring wstrKey, wstring & wstrValue); //C++ get_value––≤ªÕ®
	bool set_value_mul(vector<wstring> wstrKeyList, vector<wstring> wstrValueList);
	

	bool m_bRedisValid;
};

