
#ifndef DBACCESSPAGE_H__
#define DBACCESSPAGE_H__

#pragma  once

/*
	This class implement the access of the page elements.				
*/
//#include "../EqmDraw/EqmDrawDevice.h"
//#include "../EqmDraw/EqmDrawPipe.h"
//#include "../EqmDraw/EqmDrawText.h"
//#include "../EqmDraw/EqmDrawProject.h"
//#include "../EqmDraw/EqmDrawGraphic.h"
#include "../OPCdll/CPLCDataPoint.h"

#include "DBInterfaceImp.h"
#include "DatabaseSession.h"
#include "../ComponentDraw/EqmDrawDevice.h"
#include "../ComponentDraw/EqmDrawGraphic.h"
#include "../ComponentDraw/EqmDrawPicture.h"
#include "../ComponentDraw/EqmDrawPipe.h"
#include "../ComponentDraw/EqmDrawText.h"
#include "../ComponentDraw/EqmDrawProject.h"

#include <string>
#include <ostream>
#include <sstream>
#include <map>
#include <vector>

using namespace std;

enum PAGE_CONTAIN_EQUIPMENT
{
	PCE_Ident,			// id
	PCE_PageID,			// pageid
	PCE_EqmType,		// EqmType
	PCE_EqmSubType,		// EqmSubType
	PCE_XPos,			// center x pos
	PCE_YPos,			// center y pos	
	PCE_DataComID,		// data comid
	PCE_RotateAngle,	// rotate angel
	PCE_EqmName,		// equipment name
	PCE_Width,			// width
	PCE_Height,			// height
	PCE_PORTPOINTLIST,
	PCE_STEPINTERVAL,   //���
	PCE_UPPERLIMIT,     //����
	PCE_DOWNLIMIT       //����
};

enum PAGE_CONTAIN_TEXT
{	
	PCT_IDENT,
	PCT_PAGEID,
	PCT_INITIALIALVALUE,
	PCT_XPOS,
	PCT_YPOS,
	PCT_HEIGHT,
	PCT_WIDTH,
	PCT_FONTSIZE,
	PCT_FONTSTYLE,
	PCT_DATACOMID,
	PCT_FONTCOLOR,
	PCT_SHOWMODE,
	PCT_TRANSPARENCE,	//͸����
};

enum PAGE_CONTAIN_PIPE
{
	PCP_IDENT,
	PCP_PAGEID,
	PCP_INNERCOLOR,
	PCP_OUTERCOLOR,
	PCP_STRPOINTLIST
};

enum PAGE_CONTAIN_GRAPHIC
{
	PCG_ident,
	PCG_pageid,
	PCG_GraphicType,
	PCG_LineColor,
	PCG_LineStyle,
	PCG_LineWidth,
	PCG_FilledColor,
	PCG_PointList,
	PCG_ForwardPageID,
	PCG_TRANSPARENCE,	//͸����
};



enum PROJECT
{
	PROJECT_ID,
	PROJECT_Name,
	BACKCOLOR
};


enum PAGELIST
{
	PAGELIST_ID,
	PAGELIST_NAME,
	PAGELIST_PROJECTID,
	PAGELIST_SHOWORDER
};

enum PLCPointMap
{
	PPM_IDENT,
	PPM_PLCNAME,
	PPM_SHORTNAME,
	PPM_RWPROPERTY,
	PPM_REMARK,
	PPM_UNIT
};

enum PAGE_CONTAIN_PICTURE
{
	PCP_ident,
	PCP_Picpath,
	PCP_PicStateNum
};



//������ʾuppc client�ĵ�������
enum DEVICEINFOTYPE
{
	DEVICEINFOTYPE_CHILLER,			//���
	DEVICEINFOTYPE_COOLINGPUMP,		//��ȴ��
	DEVICEINFOTYPE_VALVE,			//����
	DEVICEINFOTYPE_COOLINGTOWER,	//��ȴ��
	DEVICEINFOTYPE_PRICHILLEDPUMP,	//һ���䶳��
	DEVICEINFOTYPE_SECCHILLEDPUMP,	//�����䶳��
};


// table create statement.

struct ProjectDB
{
	wstring projectname;
	string dbname;
};

class DBDLL DBAccessPage : public CDatabaseSeesion
{
public:
	// constructor
	DBAccessPage();
	~DBAccessPage();

public:
	// create project.
	// this function will create the database needed for this project.
	// and returns the database name.
	bool	CreateProject(CEqmDrawProject* project, string& dbname);
	
	// this class will update contents the table project.
	bool	UpdateProjectInfo();
	
	// ����ӿ�
	/*	save one page,this function will update the info of the 
		table pagecontaindevice, pagecontainpipe, pagecontaintext
	*/
	bool	SavePage(CEqmDrawPage* page);
	
	/*
		This function will save all the pages of the project.
		first, it will delete all the pages ,
		and then insert all the pages into the database.
	*/
	bool	SaveProject(CEqmDrawProject* project);
	
		
	//ɾ���ӿ�,֧�� ɾ��һ��ҳ
	//ɾ�������豸�����豸�Ĺ���

	// this function will delete the object in the pagecontain<XXX>.
	
	// this function will delete the pageid entry in the table  "pagelist".
	bool	DeletePage(CEqmDrawPage* page);
	bool	DeleteProject();

	//ȡ��ҳ���϶�������ID
	// this function will query the table pagecontaindevice,
	// pagecontainpipe, pagecontaintext, and get the max id of the specified page.
	int		GetMaxIDOFPageObjects(int pageid);

	// this function will return the max value of the table pagelist.
	int		GetMaxIDOFPageList();

	//������
	// read once page can be devided into reading more pages.
	//read one page, this function will scans the table "pagecontaindevice",
	// "pagecontaintext",  "pagecontainpipe";
	bool ReadPage(int pageid, CEqmDrawPage* page);

	// this function will first get the page list.
	// then searches the pagecontaintable .
	bool	ReadProject(CEqmDrawProject* project);
	bool	ReadAllProjectFromServer(vector<ProjectDB>& resultliust);

	//PLC��λ����ؽӿ�
	bool	ReadPlcPointMap(CPlcDataPointMap& plcpointmap);
	
	// insert into the point map to the database.
	bool	SavePlcPointMap(CPlcDataPointMap& plcpointmap);

	//��xml�ļ����뵽���ݿ�ı���
	bool	LoadPlcPointMapFromXml(const wstring& filepath, 
									CPlcDataPointMap& pointmap);

	bool	GetDevicePropertyList(vector<DataPointMapEntry>& resultlist, int start);

private:
	//��һ��ҳ���е��豸
	bool	ReadPageEquipments(CEqmDrawPage* page);

	//��һ��ҳ����ı�
	bool	ReadPageTexts(CEqmDrawPage* page);

	//��һ��ҳ��Ĺܵ�.
	bool	ReadPagePipes(CEqmDrawPage* page);

	//��һ��ҳ���2dͼ��
	bool	ReadPage2DGraphic(CEqmDrawPage* page);

	//����һ��ҳ���е��豸
	bool	SavePageEquipments(CEqmDrawPage* page);

	//����һ��ҳ����ı�
	bool	SavePageTexts(CEqmDrawPage* page);

	//����һ��ҳ��Ĺܵ�.
	bool	SavePagePipes(CEqmDrawPage* page);

	bool	SavePage2DGraphic(CEqmDrawPage* page);

	bool    ReadProjectInfo(CEqmDrawProject* project);

	bool	ReadDatabaseList(vector<string>& dbnames);

	bool    ReadPicInfo(CEqmDrawProject* page);
	bool    SavePictueInfo( CEqmDrawProject* page );
	bool    SaveProjectInfo(CEqmDrawProject* project);
};

#endif







