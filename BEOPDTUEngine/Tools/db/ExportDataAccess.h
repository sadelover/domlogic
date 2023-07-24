#pragma once


#include "DatabaseSession.h"
#include "../CustomTools.h"


namespace ExportDataAccess
{

	struct	 ExportParam{
		string	tablename;
		string  dbname;
		SYSTEMTIME	st_begin;
		SYSTEMTIME	st_end;
		string	exportpath;
		string pointname;
	};

	struct  ExportValueEntry
	{
		SYSTEMTIME	st_begin;
		string		pointname;
		double		value;
		ExportValueEntry(){
			value = 0.0;
			GetLocalTime(&st_begin);
		}
	};

	class ExportDataAccess: public CDatabaseSeesion
	{
	public:
		 bool	ExportSensorDataDTU(const ExportParam& exportparam);
		 bool	ExportRecordCommon(const ExportParam& exportparam);

		 //�����������ݵ������ڴ��
		 bool	ExportSensorDataDTU(const ExportParam& exportparam, vector<ExportValueEntry>& resultlist);
	private:
		Project::Tools::Mutex	m_lock;
	};
	

	struct  DTUDatabaseInfo{
		std::wstring databasename;
		std::wstring projectname;
	};
	
	struct  DtuPointEntry
	{
		enum {
			POINTNAME_SIZE = 128,
			POINTCOMMENT_SIZE = 256
		};
		int index;				//���
		WCHAR pointname[POINTNAME_SIZE];	//����
		WCHAR comment[POINTCOMMENT_SIZE];		//ע��
		int type;				//����
		bool useindex;			//����dtu��ʷ����ʹ�ñ�Ż��ǵ�������ʶ��λ��trueʹ�ñ�ţ�falseʹ�õ���
		int	 reserved1;			//����
		int  reserved2;			//����

		DtuPointEntry(){
			index = 0;
			memset(pointname, 0, POINTNAME_SIZE);
			memset(comment, 0, POINTCOMMENT_SIZE);
			type = 0;
			useindex = false;
			reserved1 = reserved2 = 0;
		}
	};

	class  CDTUDatabasePointInfoAccess : public CDatabaseSeesion
	{
	public:
		bool	ReadDatabaseList(vector<DTUDatabaseInfo>& dblist);

		//���������ĵĵ㡣
		bool	ReadPointList(const string& databasename, vector<DtuPointEntry>& resultlist);

		//���������ݿ�ĵ�
		bool	ReadPointList_Local(const string& databasename, vector<DtuPointEntry>& resultlist);

		//����DTU���ݵ��ļ�
		bool	ExportDTUDataToFile(const ExportParam& exportparam);

		bool	ExportDTUDataToMemory(const ExportParam& exportparam, vector<ExportValueEntry>& resultlist);

	private:
		Project::Tools::Mutex m_lock;
	};
}

