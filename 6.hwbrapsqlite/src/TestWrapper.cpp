/************************************************************************************ 
     * Copyright (c) houwenbin All Rights Reserved. 
     * 公司名称：杭州智诺科技股份有限公司
     * 文件名：  TestWrapper.cpp
     * 创建人：  侯文斌
     * 电子邮箱：houwenbin1986@gmail.com
     * 描述    : 封装sqlite3数据库api，提供更简单、明了、友好的接口
*/

#include <Windows.h>
#include <stdio.h>
#include "AnalystSQLite.h"


#define DBFILE "hwbsqlite.db"


int main(int argc, char* argv[])
{
	HANDLE hsqlite = INVALID_HANDLE_VALUE;
	//创建数据库
	zeno_SQLite_Connect(DBFILE, &hsqlite);
	if( hsqlite==INVALID_HANDLE_VALUE )
	{
		return -1;
	}

	printf("open sqlite %s, handle %p ok!\n", DBFILE, hsqlite);
	//创建数据表
	zeno_SQLite_CreateTab(hsqlite);

	//添加记录
	LINK_DATA info;
	memset(&info,0,sizeof(LINK_DATA));//重要
	info.nAnaChannel = 1;
	info.nAnaMainType;
	info.nAnaSubType;
	strcpy_s(info.szDevIP, 64, "10.12.15.199");//设备IP
	info.nDevChan = 1;//设备chan
	info.nDevPort = 8000;//设备port
	strcpy_s(info.szDevManu,32,"ZL");//设备厂商
	strcpy_s(info.szDevName,64,"10.12.15.199");
	strcpy_s(info.szLoginUser,64,"admin");
	strcpy_s(info.szLoginPass,64,"password");
	long id = zeno_SQLite_AddLinkInfo(hsqlite, &info);
	if( id>0 )
	{
		printf("insert data id=%d ok!\n", id);
	}

	//查询记录
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

	//删除记录
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

