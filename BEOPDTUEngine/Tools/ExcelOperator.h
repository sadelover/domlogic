#pragma once

#include "Tools/point/DataPointEntry.h"
#include "Tools/point/DataPointManager.h"

typedef unsigned long           u32;
const int	  g_Point_Length = 25;
const int g_nLenPointAttrSystem	= 5;
const int g_nLenPointAttrDevice	= 7;
const int g_nLenPointAttrType	= 11;
const CString g_strPointAttrSystem[g_nLenPointAttrSystem] = {_T("��"), _T("ůͨ"), _T("����"), _T("��������"), _T("���ܿյ�")};
const CString g_strPointAttrDevice[g_nLenPointAttrDevice] = {_T("��"), _T("���"), _T("ˮ��"), _T("��ȴ��"), _T("AHU"), _T("VAV"), _T("ϵͳ")};
const CString g_strPointAttrType[g_nLenPointAttrType] = {_T("��"), _T("������"), _T("������"), _T("�¶�"), _T("����"), _T("ѹ��"), _T("����"), _T("����"), _T("Ƶ��"), _T("��ͣ"), _T("����")};

class CExcelOperator
{
public:
	CExcelOperator(void);
	~CExcelOperator(void);
	static CExcelOperator& Instance();
	static void DestoryInstance();
	void SaveToExcel(CString sExcelFile, CDataPointManager& pointmap);
	bool ReadFromExcel(CString sExcelFile, CDataPointManager& pointmap);
	bool ReadFromExcel(CString sExcelFile, vector<DataPointEntry>& vecPoint,int& nErrCode);
	bool ReadFromCSV(CString sExcelFile, vector<DataPointEntry>& vecPoint,int& nErrCode);
	bool ReadFromCSV_Comma(CString sExcelFile, vector<DataPointEntry>& vecPoint,int& nErrCode);		//����ֵ������ŵ�CSV
	CString GetExcelDriver();
	bool	OutPutUpdateLog(CString strLog);		//��¼����Log
	void	RemoveSubStr(const string& r, string& str);
	int		GetColumnIndex(std::map<std::string, int> map, string strColumnName);
private:
	static CExcelOperator* m_instance;
};

