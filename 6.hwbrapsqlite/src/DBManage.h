/************************************************************************************ 
     * Copyright (c) houwenbin All Rights Reserved. 
     * ��˾���ƣ�������ŵ�Ƽ��ɷ����޹�˾
     * �ļ�����  DBManage.h
     * �����ˣ�  ���ı�
     * �������䣺houwenbin1986@gmail.com
     * ����    : ��װsqlite3���ݿ�api���ṩ���򵥡����ˡ��ѺõĽӿ�
*/

#pragma once

#include "ZenoSQL.h"
#include <stdio.h>
#include <stdlib.h>

//////////////////////////////////////////////////////////////////////////
//ͬ���ٽ�����
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
//SQLite���ݿ���������
class CDBManage
{
public:
	CDBManage(void);
	~CDBManage(void);

public:
	PZENOSQLITECALLBACK m_pMsgCallback;//���ݿ����״̬��Ϣ�ص�
	void*   m_pContext;

public:
	long    ConnectSQL(char *pSqlPath);//�������ݿ�
	long    DisconnectSQL();//�ر����ݿ�

	long    SetMsgCallback(PZENOSQLITECALLBACK pMsgCallback, void *pContext);//������Ϣ�ص�
	char    *GetDBLastError();//��ȡ���������Ϣ

	long    BeginTransaction();//��ʼ����
	long    EndTransaction();//��������

	long    CreateTable();//�������ݱ�
	long    DropTable(char *pTabName);//ɾ�����ݱ�

public:
	//�������
	long    AddLinkInfo( LPVOID param );
	long    DelLinkInfo( LPVOID param );
	long    GetLinkInfo( LPVOID param, long &lResCount, LPVOID &pResult );
	long    SetLinkInfo( int nChanId, LPVOID param );
	
	//�������
	long    AddRuleInfo( LPVOID param );
	long    DelRuleInfo( LPVOID param );
	long    GetRuleInfo( LPVOID param, long &lResCount, LPVOID &pResult );
	long    SetRuleInfo( int nChanId, LPVOID param );

	//���տռ�
	void    FreeLinkInfo( char* &pBuffer );
	
public:
	BOOL    m_bExit;//�߳̿���
	BOOL    m_bCommit;

	HANDLE  m_hEvent;//�߳̾��
	HANDLE  m_hCommitPro;	

	CCritSection m_csCommit;//�ٽ���Դ

private:
	CZenoSQL*   m_db;//sqlite3 wrapper
};

