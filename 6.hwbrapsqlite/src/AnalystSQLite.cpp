// AnalystSQLite.cpp : ���� DLL Ӧ�ó������ڵ㡣
//

#include "stdafx.h"

#include "AnalystSQLite.h"
#include "DBManage.h"


#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif








//////////////////////////////////////////////////////////////////////////
//
// ��������
//



//����SQLite���ݿ�
long  zeno_SQLite_Connect( char* pFileName, HANDLE* pSqlHandle )
{
	if( NULL==pFileName )
	{
		return -1;
	}


	//�������ݹ�����
	CDBManage* pSQL = new CDBManage;
	if ( NULL==pSQL )
	{
		return -1;
	}

	long lRes = pSQL->ConnectSQL( pFileName );

	if( lRes<0 )
	{
		OutputDebugStringA("����SQLiteDBʧ�ܣ�\n");
		return -1;
	}

	*pSqlHandle = (HANDLE)pSQL;

	return 0;
}

//�ر����ݿ�
long  zeno_SQLite_DisConnect( HANDLE hSql )
{
	if( NULL==hSql )
	{
		return -1;
	}

	CDBManage  *pSQL = (CDBManage*)hSql;

	pSQL->DisconnectSQL();

	return 0;
}

//�����������ݱ�
long  zeno_SQLite_CreateTab( HANDLE hSql )
{
	if( NULL==hSql )
	{
		return -1;
	}

	CDBManage* pSQL = (CDBManage*)hSql;

	long lRet = pSQL->CreateTable();

	return lRet;
}

//���������Ϣ
long  zeno_SQLite_AddLinkInfo( HANDLE hSql, LPVOID param )
{
	if( NULL==hSql || NULL==param )
	{
		return -1;
	}

	CDBManage  *pSQL = (CDBManage*)hSql;

	long  lRet = pSQL->AddLinkInfo( param );

	return lRet;
}

//��ѯ������Ϣ
long  zeno_SQLite_GetLinkInfo( HANDLE hSql, LPVOID param, long &lResultCount, LPVOID &pResult )
{
	if( NULL==hSql || NULL==param )
	{
		return -1;
	}

	CDBManage  *pSQL = (CDBManage*)hSql;

	long  lRet = pSQL->GetLinkInfo( param, lResultCount, pResult );

	return lRet;
}

//����������Ϣ
long  zeno_SQLite_SetLinkInfo( HANDLE hSql, int nDevId, LPVOID param )
{
	if( NULL==hSql || NULL==param || 0>=nDevId )
	{
		return -1;
	}

	CDBManage* pSQL = (CDBManage*)hSql;

	long lRet = pSQL->SetLinkInfo( nDevId, param );

	return lRet;
}

//ɾ��������Ϣ
long  zeno_SQLite_DelLinkInfo( HANDLE hSql, LPVOID param )
{
	if( NULL==hSql || NULL==param )
	{
		return -1;
	}

	CDBManage  *pSQL = (CDBManage*)hSql;

	long  lRet = pSQL->DelLinkInfo( param );

	return lRet;
}

//�ͷ���������
ZENOSQLSDK  long zeno_SQLite_FreeLinkInfo( HANDLE hSql, char* &pBuff )
{
	if( NULL==hSql )
	{
		return -1;
	}

	CDBManage  *pSQL = (CDBManage*)hSql;

	pSQL->FreeLinkInfo( pBuff );

	return 0;
}


//��ӹ�����Ϣ
ZENOSQLSDK  long zeno_SQLite_AddRuleInfo( HANDLE hSql, LPVOID param )
{
	if( NULL==hSql || NULL==param )
	{
		return -1;
	}

	CDBManage  *pSQL = (CDBManage*)hSql;

	long  lRet = pSQL->AddRuleInfo( param );

	return lRet;
}

//ȡ�ù�����Ϣ
ZENOSQLSDK  long zeno_SQLite_GetRuleInfo( HANDLE hSql, LPVOID param, long &lResultCount, LPVOID &pResult )
{
	if( NULL==hSql || NULL==param )
	{
		return -1;
	}

	CDBManage  *pSQL = (CDBManage*)hSql;

	long  lRet = pSQL->GetRuleInfo( param, lResultCount, pResult );

	return lRet;
}

//�޸Ĺ�����Ϣ
ZENOSQLSDK  long zeno_SQLite_SetRuleInfo( HANDLE hSql, int nDevId, LPVOID param )
{
	if( NULL==hSql || NULL==param || 0>=nDevId )
	{
		return -1;
	}

	CDBManage* pSQL = (CDBManage*)hSql;

	long lRet = pSQL->SetRuleInfo( nDevId, param );

	return lRet;
}

//ɾ��������Ϣ
long  zeno_SQLite_DelRuleInfo( HANDLE hSql, LPVOID param )
{
	if( NULL==hSql || NULL==param )
	{
		return -1;
	}

	CDBManage  *pSQL = (CDBManage*)hSql;

	long  lRet = pSQL->DelRuleInfo( param );

	return lRet;
}

//�ͷŹ��򻺴棬�ṹ��ͬ��ʹ��ͬһ�ӿ�
ZENOSQLSDK  long zeno_SQLite_FreeRuleInfo( HANDLE hSql, char* &pBuff )
{
	if( NULL==hSql )
	{
		return -1;
	}

	CDBManage  *pSQL = (CDBManage*)hSql;

	pSQL->FreeLinkInfo( pBuff );

	return 0;
}

