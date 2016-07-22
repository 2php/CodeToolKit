#include "StdAfx.h"
#include "ZenoSQL.h"


CZenoSQL::CZenoSQL(void)
{
	m_db       =  NULL;
	m_pError   =  NULL;
	m_pResult  =  NULL;
	m_nRow     =  0;
	m_nCol     =  0;
}

CZenoSQL::~CZenoSQL(void)
{
	if( m_db!=NULL )
	{
		sqlite3_close( m_db );
		m_db = NULL;
	}
}


//�����ݿ⣬û�оʹ���
long CZenoSQL::SQLiteConnect( char *pFileName )
{
	if( NULL==pFileName )
	{
		return -1;
	}

	if( m_db!=NULL )
	{
		sqlite3_close( m_db );
		m_db = NULL;
	}

    int rc = sqlite3_open( pFileName, &m_db );//��ָ�������ݿ��ļ�,��������ڽ�����һ��ͬ�������ݿ��ļ�
	if( rc )
    {
        strcpy_s( m_szErrMsg, 512, sqlite3_errmsg( m_db ) );//���������Ϣ
        sqlite3_close( m_db );

        return -1;
    }

    return rc;    
}


//ִ��SQL���
long CZenoSQL::SQLiteExec( char *pSQL )
{
	if( NULL==pSQL || NULL==m_db )
	{
		return -1;
	}

    int rc = -1;

    rc = sqlite3_exec( m_db, pSQL, 0, 0, &m_pError );
    if( rc==SQLITE_OK )
	{
        return 0;
	}

	if( m_pError!=NULL )
	{
		strcpy_s( m_szErrMsg, 512, m_pError );
	}

    return -1;
}

//ִ��SQL��ѯ
long CZenoSQL::SQLiteSearch( char *pSearch )
{
	if( NULL==pSearch || NULL==m_db )
	{
		return -1;
	}

	if( m_pResult!=NULL )
	{
		sqlite3_free_table( m_pResult );
		m_pResult = NULL;
		m_nCol = 0;
		m_nRow = 0;
	}

    sqlite3_get_table( m_db, pSearch, &m_pResult, &m_nRow, &m_nCol, &m_pError );

	if( m_pError != NULL )
	{
		strcpy_s( m_szErrMsg, 512, m_pError );
	}

	return 0;
}

//�ر����ݿ�
long CZenoSQL::SQLiteDisconnect()
{
	if( NULL==m_db )
	{
		return -1;
	}

	//�ͷŵ� Result ���ڴ�ռ�
	if( m_pResult!=NULL )
	{
		sqlite3_free_table( m_pResult );
	}

	//�ر����ݿ�
    if( SQLITE_OK == sqlite3_close( m_db ) )
	{
		m_db = NULL;

        return 0;
	}

    return -1;
}

