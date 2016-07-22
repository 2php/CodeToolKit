/************************************************************************************ 
     * Copyright (c) houwenbin All Rights Reserved. 
     * ��˾���ƣ�������ŵ�Ƽ��ɷ����޹�˾
     * �ļ�����  TestWrapper.cpp
     * �����ˣ�  ���ı�
     * �������䣺houwenbin1986@gmail.com
     * ����    : ��װsqlite3���ݿ�api���ṩ���򵥡����ˡ��ѺõĽӿ�
*/

#include <Windows.h>
#include <stdio.h>
#include "AnalystSQLite.h"


#define DBFILE "hwbsqlite.db"


int main(int argc, char* argv[])
{
	HANDLE hsqlite = INVALID_HANDLE_VALUE;
	//�������ݿ�
	zeno_SQLite_Connect(DBFILE, &hsqlite);
	if( hsqlite==INVALID_HANDLE_VALUE )
	{
		return -1;
	}

	printf("open sqlite %s, handle %p ok!\n", DBFILE, hsqlite);
	//�������ݱ�
	zeno_SQLite_CreateTab(hsqlite);

	//��Ӽ�¼
	LINK_DATA info;
	memset(&info,0,sizeof(LINK_DATA));//��Ҫ
	info.nAnaChannel = 1;
	info.nAnaMainType;
	info.nAnaSubType;
	strcpy_s(info.szDevIP, 64, "10.12.15.199");//�豸IP
	info.nDevChan = 1;//�豸chan
	info.nDevPort = 8000;//�豸port
	strcpy_s(info.szDevManu,32,"ZL");//�豸����
	strcpy_s(info.szDevName,64,"10.12.15.199");
	strcpy_s(info.szLoginUser,64,"admin");
	strcpy_s(info.szLoginPass,64,"password");
	long id = zeno_SQLite_AddLinkInfo(hsqlite, &info);
	if( id>0 )
	{
		printf("insert data id=%d ok!\n", id);
	}

	//��ѯ��¼
	LINK_DATA cond;
	memset(&cond,0,sizeof(LINK_DATA));
	long count = 0;
	LINK_DATA* result = NULL;
	zeno_SQLite_GetLinkInfo(hsqlite, &cond, count, (LPVOID&)result);
	for (int i=0; i<count; ++i)
	{
		printf("id %d,ip %s,port %d...\n",result[i].id,result[i].szDevIP,result[i].nDevPort);
	}
	zeno_SQLite_FreeLinkInfo(hsqlite, (char* &)result);

	//ɾ����¼
	LINK_DATA cdel;
	memset(&cdel,0,sizeof(LINK_DATA));
	cdel.id = 1;
	if( zeno_SQLite_DelLinkInfo(hsqlite, &cdel)==0 )
	{
		printf("delete item %d ok!\n", cdel.id);
	}

	zeno_SQLite_DisConnect(hsqlite);

	system("pause");

	return 0;
}

