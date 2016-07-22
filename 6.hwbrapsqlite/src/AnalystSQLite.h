/************************************************************************************ 
     * Copyright (c) houwenbin All Rights Reserved. 
     * ��˾���ƣ�������ŵ�Ƽ��ɷ����޹�˾
     * �ļ�����  AnalystSQLite.h
     * �����ˣ�  ���ı�
     * �������䣺houwenbin1986@gmail.com
     * ����    : ��װsqlite3���ݿ�api���ṩ���򵥡����ˡ��ѺõĽӿ�
*/

#ifndef __ANALYSTSQLITESDK_H__
#define __ANALYSTSQLITESDK_H__

//�ӿں���������
#ifdef HWBRAPSQLITE_EXPORTS
	#define ZENOSQLSDK __declspec(dllexport)
#else
	#define ZENOSQLSDK __declspec(dllimport)
#endif

//---------------Start����ϵͳ��----------------//

#define	TBSYST		"table_system"		//ϵͳ��
#define	TBCHAN		"table_channels"	//ͨ����
#define	TBRULE		"table_rules"		//�����

//---------------Stop����ϵͳ��----------------//

#define  WM_SQLITE_LOGOUT -1


//
// ��ṹ����������ʼ
//
#pragma pack(4)

//////////////////////////////////////////////////////////////////////////
//ʶ������ͨ��
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
	int  nAnaChannel;//����ͨ����
	int  nPtziPos;//��̨��ʼԤ�õ�
	int  nPtztPos;//��̨Ŀ��Ԥ�õ�
	int  nHLimit;//�߶ȱ��
	
	int  nAnaMainType;//�㷨����(���磬�������ܽ硢���Ƶ�)
	int  nAnaSubType; //����������(���磬���ߡ�˫�ߡ������)

	//����Ϊ�ܽ��������
	int  nLineNum;//[0-5]
	char szAlarmDirect[5][256];//���䷽��
	char szBZSLineType[5][256];//��������
	char szBZSLineType_2[5][256];//�ڶ�����������

}LINK_DATA, *PLINK_DATA;

//�豸
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

//ͨ��
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
//��ṹ����������
//
//////////////////////////////////////////////////////////////////////////

typedef void (CALLBACK * PZENOSQLITECALLBACK)( HANDLE hHandle, long lCmd, unsigned char *pData, long lSize, void *pContext );


#ifdef __cplusplus
extern "C"
{
#endif

	//////////////////�������ݿ�///////////////////////
	ZENOSQLSDK  long  zeno_SQLite_Connect( char *pFileName, HANDLE *pSqlHandle );

	ZENOSQLSDK  long  zeno_SQLite_DisConnect( HANDLE hSql );

	////////////////////�������б�///////////////////////
	
	ZENOSQLSDK  long  zeno_SQLite_CreateTab( HANDLE hSql );


	////////////////////������Ϣ��///////////////////////

	ZENOSQLSDK  long  zeno_SQLite_AddLinkInfo( HANDLE hSql, LPVOID param );

	ZENOSQLSDK  long  zeno_SQLite_GetLinkInfo( HANDLE hSql, LPVOID param, long &lResultCount, LPVOID &pResult );

	ZENOSQLSDK  long  zeno_SQLite_SetLinkInfo( HANDLE hSql, int nDevId, LPVOID param );

	ZENOSQLSDK  long  zeno_SQLite_DelLinkInfo( HANDLE hSql, LPVOID param );

	////////////////////������Ϣ��///////////////////////

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

