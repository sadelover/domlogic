#pragma once

#include "../../Tools/DllDefine.h"
#include <map>
#include <vector>
using namespace std;

struct StructParam3
{
	CString strName;
	map<int,CString> mapping; 
};

class DLLEXPORT CParseStringParam3
{
public:
	CParseStringParam3(void);
	~CParseStringParam3(void);
	static CParseStringParam3& Instance();//��ȡ�����ʵ��
	static void DestoryInstance();//���ٸ����ʵ��
	void ParseParam3(const CString strName, const CString strParam3 );//�������Ϊ������ƺ�param3�ַ���
	const CString GetText(const CString strName, const int nValue);//ͨ��������ƺ�param3�����úõ�ֵ���õ���ֵҪ��ʾ���ı�
private:
	static CParseStringParam3* m_instane;
	vector<StructParam3> m_vecStructParam3;
};

