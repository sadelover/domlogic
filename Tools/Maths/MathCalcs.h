#pragma once

#include "math.h"
#include "MathDefines.h"
#include <stdlib.h>
#include <windows.h>
#include "../NormalDefines.h"

#define Cpw		4.186			//	kJ/(kg*'C)
#define F_50	50

namespace Project
{
	namespace MathFuncs
	{
		static inline
		double
		Square( double dbParam )
		{
			return dbParam * dbParam;
		};

		static inline
		double
 		Cube( double dbParam )
 		{
 			return dbParam * dbParam * dbParam;
 		};

		static inline
		double
		SquareRoot( double dbParam )
		{
			return sqrt( dbParam );
		};

		static inline
		double
		CubeRoot( double dbParam )
		{
			return pow( dbParam, 3 );
		};

		static inline
		double
		Ln_e( double dbParam )
		{
			return log( dbParam ) / log( M_E );
		};


		static inline
		double
		Power_a_n( double a, int n )
		{
			return pow( a, n );
		};

		static inline
		double
		Power_a_n( double a, double n )
		{
			return pow( a, n );
		};

		static inline
		double
		Exp( double n )
		{
			return pow( M_E, n );
		};

		static inline
		double
		GetMin_dbCmp( double db1, double db2 )
		{
			return db1<db2?db1:db2;
		};

		static inline
		int
		GetMin_intCmp( int i1, int i2 )
		{
			return i1<i2?i1:i2;
		};

		static inline
		unsigned int
		GetMin_uiCmp( unsigned int ui1, unsigned int ui2 )
		{
			return ui1<ui2?ui1:ui2;
		};

		static inline
		double
		GetMax( double	db1, double	db2 )
		{
			return db1>db2?db1:db2;
		};

		static inline
		double
		GetAbs( double	dbparam )
		{
			return abs( dbparam );
		};

		static inline
		bool
		IsNearlyEqual( double dbparam1, double dbParam2, double dbPrecision )
		{
			return abs( dbparam1-dbParam2 ) <= abs(dbPrecision);
		};

		static inline
		double
		GetAverage( double dbparam1, double dbParam2 )
		{
			return ( dbparam1 + dbParam2 ) / 2;
		};

		/*
		�¶�ת��
		*/
		static inline
		double
		F_to_C( double dbF )
		{
			return ( dbF - 32 ) / 1.8;
		};

		static inline
		double
		C_to_F( double dbC )
		{
			return dbC * 1.8 + 32;
		};

		static inline
		unsigned int
		GetRandNum( unsigned int uiMax )	//��1-uiMax��ȡ�����
		{
			static bool ifInit = false;
			if(!ifInit){
				srand( GetTickCount() );
				ifInit = true;
			}

			return abs( rand() ) % uiMax + 1;
		}

		static inline
		int
		Double_To_BiggerInt( double dbParam )	//����ӽ��Ľϴ�����
		{
			if ( dbParam < 0 )
			{
				return int( dbParam );
			}
			else
			{
				return int( dbParam ) + ( (dbParam-double(int( dbParam ))>0) ? 1 : 0 );
			}
		}

		static inline
		int
		Double_To_Int( double dbParam )
		{
			return (int)dbParam + ( dbParam >= 0 ? 1 : -1 ) * ( GetAbs( dbParam - (int)dbParam ) > 0.5 ? 1 : 0 );
		}

		static inline
		double
		Calc_Pc( double dbQ_chw0, double dbT_chwr0, double dbT_chws0 )
		{
			return Cpw * dbQ_chw0 * ( dbT_chwr0 - dbT_chws0 ) / KW_TO_TON_MULTI;
		}

		static inline
		double
		Get_Cop( double dbPc, double dbPe )
		{
			return ( IsNearlyEqual(dbPe, 0, 0.01) ) ? 0 : (dbPc/dbPe);
		}

		static inline
		double
		Get_kW_per_Ton( double dbCop )
		{
			return IsNearlyEqual( dbCop, 0, 0.01 ) ? 0 : ( 1 / dbCop );
		}

		static inline
		double
		Get_kW_per_Ton( double dbPc, double dbPe )
		{
			return IsNearlyEqual( Get_Cop(dbPc,dbPe), 0, 0.01 ) ? 0 : (1 / Get_Cop(dbPc,dbPe));
		}

		static inline
		double
		Milliseconds_To_Hour( unsigned int uiMilliSecond )
		{
			return ( double(uiMilliSecond) / 1000 ) / 3600;
		}

		static inline
		double
		RH_TD__to__TW( double dbRH, double dbTD )
		{
			//RH 20-95
			return ( 0 > dbRH || 100 < dbRH ) ? 0 : ( -5.250 + 0.6901 * dbTD + 0.05159 * dbRH + 0.003432 * dbTD * dbRH - 1.898 * dbTD / dbRH );
		}
		
		static inline
		double Resolve_Piecewise_function(					//���κ������
										double dbX1,
										double dbY1,
										double dbX2,
										double dbY2,
										double dbX )
		{
			if ( dbX <= dbX1 )		//max
			{
				return dbY1;
			}
			else if ( dbX > dbX1 && dbX < dbX2 )
			{
				return ( dbY2 - dbY1 ) / ( dbX2 - dbX1 ) * dbX + ( dbX2 * dbY1 - dbX1 * dbY2 ) / ( dbX2 - dbX1 );
			}
			else /*if ( dbX >= dbX2 )*/
			{
				return dbY2;
			}
		}

		//�������ֵ�ļ���(10��ֵ)
		static inline
		WORD _TEN_( WORD w1_10 )
		{
			return 10 * w1_10;
		}

		static BOOL
		PointInLine(int x, int y, int x0, int y0, int x1, int y1, int tol)
		{
			//�޶���x,y����һ������֮��
			if ((x0<x1)&&( x<x0-tol|| x>x1+tol))
				return FALSE;
			if ((x1<x0)&&( x<x1-tol|| x>x0+tol))
				return FALSE;
			if ((y0<y1)&&( y<y0-tol|| y>y1+tol))
				return FALSE;
			if ((y1<y0)&&( y<y1-tol|| y>y0+tol))
				return FALSE;

			//ֱ��Ӧ�õ㵽ֱ�ߵľ��빫ʽ
			int d = 0;
			if (x0 != x1)
			{
				int l =abs((y1 - y0) * x / (x1 - x0) - y-(y1 - y0) * x1 / (x1 - x0) + y1);
				int n =(int)sqrt(pow((y1 - y0) / (x1 - x0), (double)2.0 ) + 1 );
				d = l / n;
			}

			if (x0 == x1){
				d=abs(x-x0);
			}
			if (d<tol){
				return TRUE;
			} 
			else{
				return FALSE;
			}
		}

		//��1-8760ת��Ϊ*��*��
		static inline
		void TranslateNToMD(int n, int &month, int &day)
		{
			n = n%8760;
			if(n<0)
				n+=8760;

			int mm[12]={31,28,31,30,31,30,31,31,30,31,30,31};
			month = 0;
			int curN = 0;
			while(curN<=n)
			{
				curN+=mm[month];
				month++;
			}

			day = n - (curN-mm[month-1])+1;//���������Сʱ��
		}

		/*
		PWP1 Ϊ����
		PWP2 Ϊ�Ҳ��
		dP = ������ѹ��-������ѹ��
		dT = ��ˮ��ˮ�¶� - ��ˮ��ˮ�¶�
		*/
		static inline
			BOOL Is_Chiller_SURGE( PwFunc_2P PWP1, double dbDP, double dbDeltaT )
		{
			if ( dbDeltaT < PWP1.dbX1 )	//������ǰΪ����
			{
				if ( dbDP >= PWP1.dbY1 )
				{
					return TRUE;		//�Ϸ�������
				}
				else
				{
					return FALSE;		//�·���������
				}
			}
			else
			{
				//ZHW Add 2010-07-13
				if ( /*( dbDP - PWP1.dbY1 ) / ( dbDeltaT - PWP1.dbX1 ) <= 0
					 ||*/( dbDP - PWP1.dbY1 ) / ( dbDeltaT - PWP1.dbX1 ) >= ( PWP1.dbY2 - PWP1.dbY1 ) / ( PWP1.dbX2 - PWP1.dbX1 ) )
				{
					return TRUE;		//���������ӵ�б�ʱȵڶ�����������Ϸ������񣬷��򲻴���
				}
				else
				{
					return FALSE;
				}
			}

			return FALSE;
		}
	}
}