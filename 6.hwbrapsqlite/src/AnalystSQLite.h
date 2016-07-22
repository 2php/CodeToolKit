/************************************************************************************ 
     * Copyright (c) houwenbin All Rights Reserved. 
     * 公司名称：杭州智诺科技股份有限公司
     * 文件名：  AnalystSQLite.h
     * 创建人：  侯文斌
     * 电子邮箱：houwenbin1986@gmail.com
     * 描述    : 封装sqlite3数据库api，提供更简单、明了、友好的接口
*/

#ifndef __ANALYSTSQLITESDK_H__
#define __ANALYSTSQLITESDK_H__

//接口函数导出宏
#ifdef HWBRAPSQLITE_EXPORTS
	#define ZENOSQLSDK __declspec(dllexport)
#else
	#define ZENOSQLSDK __declspec(dllimport)
#endif

//---------------Start智能系统表----------------//

#define	TBSYST		"table_system"		//系统表
#define	TBCHAN		"table_channels"	//通道表
#define	TBRULE		"table_rules"		//规则表

//---------------Stop智能系统表----------------//

#define  WM_SQLITE_LOGOUT -1


//
// 表结构体声明区开始
//
#pragma pack(4)

//////////////////////////////////////////////////////////////////////////
//识别联动通道
typedef struct _LINK_DATA
{
	int id;
	char szDevName[64];
	char szDevIP[64];
	char szDevManu[32];
	char szLoginUser[64];
	char szLoginPass[64];
	int  nDevPort;
	int  nDevChan;
	int  nAnaChannel;//分析通道号
	int  nPtziPos;//云台初始预置点
	int  nPtztPos;//云台目标预置点
	int  nHLimit;//高度标尺
	
	int  nAnaMainType;//算法类型(比如，人脸、周界、车牌等)
	int  nAnaSubType; //分析子类型(比如，单线、双线、区域等)

	//以下为周界规则数据
	int  nLineNum;//[0-5]
	char szAlarmDirect[5][256];//警戒方向
	char szBZSLineType[5][256];//规则数据
	char szBZSLineType_2[5][256];//第二条规则数据

}LINK_DATA, *PLINK_DATA;

//设备
typedef struct _DEV_DATA
{
	int id;
	char szDevName[64];
	char szDevIp[64];
	int  nDevPort;
	int  nChannelCount;
	int  regionId;
	char szDevUser[32];
	char szDevPass[32];
	char szManuFactory[32];
	char szBackInfo[128];
}DEV_DATA, *PDEV_DATA;

//通道
typedef struct _CHAN_DATA
{
	int id;
	int nDeviceId;
	int nChannelNO;
	char szChanName[64];
	char szBackInfo[128];
}CHAN_DATA, *PCHAN_DATA;

#pragma pack()


//
//表结构声明区结束
//
//////////////////////////////////////////////////////////////////////////

typedef void (CALLBACK * PZENOSQLITECALLBACK)( HANDLE hHandle, long lCmd, unsigned char *pData, long lSize, void *pContext );


#ifdef __cplusplus
extern "C"
{
#endif

	//////////////////连接数据库///////////////////////
	ZENOSQLSDK  long  zeno_SQLite_Connect( char *pFileName, HANDLE *pSqlHandle );

	ZENOSQLSDK  long  zeno_SQLite_DisConnect( HANDLE hSql );

	////////////////////创建所有表///////////////////////
	
	ZENOSQLSDK  long  zeno_SQLite_CreateTab( HANDLE hSql );


	////////////////////联动信息表///////////////////////

	ZENOSQLSDK  long  zeno_SQLite_AddLinkInfo( HANDLE hSql, LPVOID param );

	ZENOSQLSDK  long  zeno_SQLite_GetLinkInfo( HANDLE hSql, LPVOID param, long &lResultCount, LPVOID &pResult );

	ZENOSQLSDK  long  zeno_SQLite_SetLinkInfo( HANDLE hSql, int nDevId, LPVOID param );

	ZENOSQLSDK  long  zeno_SQLite_DelLinkInfo( HANDLE hSql, LPVOID param );

	////////////////////规则信息表///////////////////////

	ZENOSQLSDK  long  zeno_SQLite_AddRuleInfo( HANDLE hSql, LPVOID param );

	ZENOSQLSDK  long  zeno_SQLite_GetRuleInfo( HANDLE hSql, LPVOID param, long &lResultCount, LPVOID &pResult );

	ZENOSQLSDK  long  zeno_SQLite_SetRuleInfo( HANDLE hSql, int nDevId, LPVOID param );

	ZENOSQLSDK  long  zeno_SQLite_DelRuleInfo( HANDLE hSql, LPVOID param );

	//////////////////////////////////////////////////////

	ZENOSQLSDK  long  zeno_SQLite_FreeLinkInfo( HANDLE hSql, char* &pBuff );
	ZENOSQLSDK  long  zeno_SQLite_FreeRuleInfo( HANDLE hSql, char* &pBuff );


#ifdef __cplusplus
}
#endif

#endif

