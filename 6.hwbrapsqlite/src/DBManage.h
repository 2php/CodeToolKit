/************************************************************************************ 
     * Copyright (c) houwenbin All Rights Reserved. 
     * 公司名称：杭州智诺科技股份有限公司
     * 文件名：  DBManage.h
     * 创建人：  侯文斌
     * 电子邮箱：houwenbin1986@gmail.com
     * 描述    : 封装sqlite3数据库api，提供更简单、明了、友好的接口
*/

#pragma once

#include "ZenoSQL.h"
#include <stdio.h>
#include <stdlib.h>

//////////////////////////////////////////////////////////////////////////
//同步临界锁类
class CCritSection
{
public:
	CCritSection() 
	{
		InitializeCriticalSection(&m_CritSec);
	}

	~CCritSection() 
	{
		DeleteCriticalSection(&m_CritSec);
	}

public:
	void Lock() 
	{
		EnterCriticalSection(&m_CritSec);
	}

	void Unlock() 
	{
		LeaveCriticalSection(&m_CritSec);
	}

private:
	// make copy constructor and assignment operator inaccessible
	CRITICAL_SECTION m_CritSec;
};


//////////////////////////////////////////////////////////////////////////
//SQLite数据库管理操作类
class CDBManage
{
public:
	CDBManage(void);
	~CDBManage(void);

public:
	PZENOSQLITECALLBACK m_pMsgCallback;//数据库操作状态消息回调
	void*   m_pContext;

public:
	long    ConnectSQL(char *pSqlPath);//创建数据库
	long    DisconnectSQL();//关闭数据库

	long    SetMsgCallback(PZENOSQLITECALLBACK pMsgCallback, void *pContext);//设置消息回调
	char    *GetDBLastError();//获取最近错误信息

	long    BeginTransaction();//开始事务
	long    EndTransaction();//结束事务

	long    CreateTable();//创建数据表
	long    DropTable(char *pTabName);//删除数据表

public:
	//联动相关
	long    AddLinkInfo( LPVOID param );
	long    DelLinkInfo( LPVOID param );
	long    GetLinkInfo( LPVOID param, long &lResCount, LPVOID &pResult );
	long    SetLinkInfo( int nChanId, LPVOID param );
	
	//规则相关
	long    AddRuleInfo( LPVOID param );
	long    DelRuleInfo( LPVOID param );
	long    GetRuleInfo( LPVOID param, long &lResCount, LPVOID &pResult );
	long    SetRuleInfo( int nChanId, LPVOID param );

	//回收空间
	void    FreeLinkInfo( char* &pBuffer );
	
public:
	BOOL    m_bExit;//线程控制
	BOOL    m_bCommit;

	HANDLE  m_hEvent;//线程句柄
	HANDLE  m_hCommitPro;	

	CCritSection m_csCommit;//临界资源

private:
	CZenoSQL*   m_db;//sqlite3 wrapper
};

