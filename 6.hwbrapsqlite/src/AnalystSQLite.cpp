// AnalystSQLite.cpp : 定义 DLL 应用程序的入口点。
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
// 导出函数
//



//链接SQLite数据库
long  zeno_SQLite_Connect( char* pFileName, HANDLE* pSqlHandle )
{
	if( NULL==pFileName )
	{
		return -1;
	}


	//创建数据管理器
	CDBManage* pSQL = new CDBManage;
	if ( NULL==pSQL )
	{
		return -1;
	}

	long lRes = pSQL->ConnectSQL( pFileName );

	if( lRes<0 )
	{
		OutputDebugStringA("创建SQLiteDB失败！\n");
		return -1;
	}

	*pSqlHandle = (HANDLE)pSQL;

	return 0;
}

//关闭数据库
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

//创建所有数据表
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

//添加联动信息
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

//查询联动信息
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

//更新联动信息
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

//删除联动信息
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

//释放联动缓存
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


//添加规则信息
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

//取得规则信息
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

//修改规则信息
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

//删除规则信息
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

//释放规则缓存，结构相同，使用同一接口
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

