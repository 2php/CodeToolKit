#include "StdAfx.h"
#include "DBManage.h"

#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include <atlstr.h>


//事务提交线程
DWORD  WINAPI  CommitTransactionProc( LPVOID param );

//
//数据库操作类
//
CDBManage::CDBManage(void)
{
	m_db  =  NULL;

	m_bExit       =  FALSE;
	m_bCommit	  =  FALSE;
	m_hEvent      =  NULL;
	m_hCommitPro  =  NULL;

	m_pMsgCallback  =  NULL;
	m_pContext      =  NULL;
}

CDBManage::~CDBManage(void)
{
}


//开始事务
long CDBManage::BeginTransaction()
{
	m_csCommit.Lock();

	if( NULL==m_db )
	{
		m_csCommit.Unlock();
		return -1;
	}

	char szSql[1024] = {'\0'};
	sprintf_s( szSql, 1024, "BEGIN TRANSACTION" );

	long  lRet = m_db->SQLiteExec( szSql );

	m_csCommit.Unlock();

	return lRet;
}

//结束事务
long CDBManage::EndTransaction()
{
	m_csCommit.Lock();

	if( NULL==m_db )
	{
		m_csCommit.Unlock();
		return -1;
	}

	char szSql[1024] = {'\0'};
	sprintf_s( szSql, 1024, "COMMIT TRANSACTION" );

	long  lRet = m_db->SQLiteExec( szSql ); 

	m_csCommit.Unlock();

	return lRet;
}


//获取出错记录
char* CDBManage::GetDBLastError()
{
	if( NULL==m_db )
	{
		return NULL;
	}

	return m_db->GetErrorMsg();
}


//设备提示信息回调
long CDBManage::SetMsgCallback( PZENOSQLITECALLBACK pMsgCallback, void *pContext )
{
	m_pMsgCallback = pMsgCallback;
	m_pContext     = pContext;

	return 0;
}


//提交事务线程
DWORD  WINAPI  CommitTransactionProc( LPVOID param )
{
	if( param==NULL )
	{
		return 0;
	}

	CDBManage *pMain = (CDBManage*)param;

	while( 1 )
	{
		if( pMain->m_bExit )
		{
			break;
		}

		if ( pMain->m_bCommit )
		{
			//提交本次事务
			pMain->EndTransaction();

			//开启下一次事务
			pMain->BeginTransaction();
			pMain->m_bCommit = FALSE;
		}

		for (int i = 0; i<1000; ++i)
		{
			if( pMain->m_bExit || pMain->m_bCommit )
			{
				break;
			}

			::Sleep( 10 );
		}
	}

	::SetEvent( pMain->m_hEvent );

	return 0;
}


//连接数据库
long CDBManage::ConnectSQL( char *pSqlPath )
{	
	m_csCommit.Lock();

	if( pSqlPath==NULL )
	{
		m_csCommit.Unlock();
		return -1;
	}
	
	if( m_db!=NULL )
	{
		DisconnectSQL();
	}

	//
	m_db = new CZenoSQL;

	//char pBuf[256]; 
	//_getcwd(pBuf, 256);//这种方法得到的路径，每次打开一个新的对话框，如导入图片，得到的路径就会变成图片所在的路径，而非程序工作路径
	//strcat_s(pBuf,"\\");
	//strcat_s(pBuf, pSqlPath);
	//long  lRet  =  m_db->SQLiteConnect( pBuf );

	CString strAppPath; 
	::GetModuleFileName(NULL, strAppPath.GetBuffer(_MAX_PATH), _MAX_PATH); 
	strAppPath.ReleaseBuffer(); 

	int nPos = strAppPath.ReverseFind('\\'); 
	CString tt = strAppPath.Left(nPos );
	//tt = tt + "\\" +  pSqlPath;

	::SetCurrentDirectory( tt );


	//创建并连接数据库
	long lRet = m_db->SQLiteConnect( pSqlPath );
	
	//开启事务
	BeginTransaction();

	m_csCommit.Unlock();

	//停止已开启的事务处理线程
	if( m_hCommitPro!=NULL )
	{
		m_bExit = TRUE;

		WaitForSingleObject( m_hEvent, INFINITE );

		CloseHandle( m_hEvent );
		m_hEvent = NULL;

		CloseHandle( m_hCommitPro );
		m_hCommitPro = NULL;

		m_bExit = FALSE;
	}

	//开启事务线程
	m_hEvent     = CreateEvent( NULL, FALSE, FALSE, NULL );
	m_hCommitPro = CreateThread( NULL, 0, CommitTransactionProc, this, 0, NULL );

	return lRet;
}

//关闭数据库，提交事务
long CDBManage::DisconnectSQL()
{
	if( NULL==m_db )
	{
		return -1;
	}

	//提交事务
	EndTransaction();

	//停止事务线程
	if( m_hCommitPro != NULL )
	{
		m_bExit = TRUE;
		WaitForSingleObject( m_hEvent, INFINITE );

		CloseHandle( m_hEvent );
		m_hEvent = NULL;

		CloseHandle( m_hCommitPro );
		m_hCommitPro = NULL;

		m_bExit = FALSE;
	}

	long lRet = m_db->SQLiteDisconnect();

	delete  m_db;
	m_db  =  NULL;

	if (m_pMsgCallback != NULL)
	{
		m_pMsgCallback( this, WM_SQLITE_LOGOUT, NULL, 0, m_pContext );
	}

	return lRet;
}


//初始化所有数据表
long CDBManage::CreateTable()
{
	m_csCommit.Lock();

	m_bCommit = FALSE;

	if( NULL==m_db )
	{
		m_csCommit.Unlock();
		return -1;
	}

	long lRet = 0;

	char szSql[1024] = {'\0'};

	//创建通道设备表
	sprintf_s( szSql, 1024, "CREATE TABLE %s ( id INTEGER PRIMARY KEY autoincrement, anachannel INTEGER, devicename VARCHAR(64), deviceip VARCHAR(64), deviceport INTEGER, devicechan INTEGER, devicemanu VARCHAR(32), loginuser VARCHAR(64), loginpass VARCHAR(64), ptzipos INTEGER, ptztpos INTEGER, hlimit INTEGER );", TBCHAN ) ;

	lRet = m_db->SQLiteExec( szSql );

	//创建通道规则表
	sprintf_s( szSql, 1024, "CREATE TABLE %s ( id INTEGER PRIMARY KEY autoincrement, anachannel INTEGER, anamaintype INTEGER, anasubtype INTEGER, linenum INTEGER, line1_1 VARCHAR(256), line1_2 VARCHAR(256), line1_3 VARCHAR(256), line1_4 VARCHAR(256), line1_5 VARCHAR(256), line2_1 VARCHAR(256), line2_2 VARCHAR(256), line2_3 VARCHAR(256), line2_4 VARCHAR(256), line2_5 VARCHAR(256), direct_1 VARCHAR(256), direct_2 VARCHAR(256), direct_3 VARCHAR(256), direct_4 VARCHAR(256), direct_5 VARCHAR(256), hlimit INTEGER );", TBRULE ) ;

	lRet = m_db->SQLiteExec( szSql );

	m_csCommit.Unlock();

	m_bCommit = TRUE;

	return lRet;
}


//删除数据表X
long CDBManage::DropTable( char *pTabName )
{
	m_csCommit.Lock();

	if( NULL==m_db || pTabName==NULL )
	{
		m_csCommit.Unlock();
		return -1;
	}

	long lRet = 0;

	char szSql[1024] = {'\0'};

	sprintf_s( szSql, 1024, "DROP TABLE %s", pTabName ) ;

	lRet = m_db->SQLiteExec( szSql );

	m_csCommit.Unlock();

	m_bCommit = TRUE;

	return lRet;
}

//////////////////////////////////////////////////////////////////////////
//插入联动记录
long CDBManage::AddLinkInfo( LPVOID param )
{
	m_csCommit.Lock();

	if( NULL==param || NULL==m_db )
	{
		m_csCommit.Unlock();
		return -1;
	}

	LINK_DATA* pChan = (LINK_DATA*)param;

	char szSql[1024] = {'\0'};

	//id, anachannel, devicename, deviceip, deviceport, devicechan, devicemanu, loginuser, loginpass, ptzipos, ptztpos, hlimit
	sprintf_s( szSql, 1024, "INSERT INTO \"%s\" VALUES( NULL, %d, \"%s\", \"%s\", %d, %d, \"%s\", \"%s\", \"%s\", %d, %d, %d )",
		TBCHAN, pChan->nAnaChannel, pChan->szDevName, pChan->szDevIP, pChan->nDevPort, pChan->nDevChan, pChan->szDevManu, pChan->szLoginUser, pChan->szLoginPass,
		pChan->nPtziPos, pChan->nPtztPos, pChan->nHLimit ) ;

	long  lRet = m_db->SQLiteExec( szSql );

	m_csCommit.Unlock();

	m_bCommit = TRUE;

	long lNewId = m_db->getLastInsertId();

	return lNewId;
}

//删除关联通道
long CDBManage::DelLinkInfo( LPVOID param )
{
	m_csCommit.Lock();

	if( NULL==param || NULL==m_db )
	{
		m_csCommit.Unlock();
		return -1;
	}

	char szSql[1024] = {'\0'};

	BOOL bMulti = FALSE;

	LINK_DATA  *pInfo = (LINK_DATA*)param;

	sprintf_s( szSql, 1024, "DELETE FROM \"%s\"", TBCHAN ) ;

	if( pInfo->id>0 )
	{//分析通道号
		sprintf_s( szSql, 1024, "%s WHERE anachannel = %d", szSql, pInfo->id ) ;
		bMulti = TRUE;
	}

	if( pInfo->nAnaChannel>0 )
	{//分析类型
		if( bMulti )
		{
			sprintf_s( szSql, 1024, "%s AND anachannel = %d", szSql, pInfo->nAnaChannel );
		}
		else
		{
			sprintf_s( szSql, 1024, "%s WHERE anachannel = %d", szSql, pInfo->nAnaChannel );
			bMulti = TRUE;
		}
	}

	long lRet = m_db->SQLiteExec( szSql );

	m_csCommit.Unlock();

	m_bCommit = TRUE;

	return lRet;
}

////查询关联通道
//long CDBManage::GetLinkInfo( LPVOID param, long &lResCount, LPVOID &pResult )
//{
//	m_csCommit.Lock();
//
//	if( NULL==param || NULL==m_db || pResult )
//	{
//		m_csCommit.Unlock();
//
//		return -1;
//	}
//
//	lResCount = 0;
//
//	char szSql[1024] = {'\0'};
//	long lRet = 0;
//	BOOL bMulti = FALSE;
//
//	LINK_DATA* pInfo = (LINK_DATA*)param;
//
//	if( pResult==NULL )//返回 lResCount 结果到 pResult 中
//	{
//
//		//////////////////////////////////////////////////////////////////////////
//		//先查询channels_table中的通道联动设备信息
//
//		sprintf_s( szSql, 1024, "SELECT * from %s", CHANTAB );
//		if( pInfo->id>0 )
//		{//索引号
//			sprintf_s( szSql, 1024, "%s WHERE id = %d", szSql, pInfo->id );
//			bMulti = TRUE;
//		}
//
//		if( pInfo->nAnaChannel>0 )
//		{//分析通道号
//			if( bMulti )
//			{
//				sprintf_s( szSql, 1024, "%s AND anachannel = %d", szSql, pInfo->nAnaChannel );
//			}
//			else
//			{
//				sprintf_s( szSql, 1024, "%s WHERE anachannel = %d", szSql, pInfo->nAnaChannel );
//
//				bMulti = TRUE;
//			}
//		}
//
//		//执行查询
//		lRet = m_db->SQLiteSearch( szSql );
//
//		if ( m_db->GetTableRow()<=0 || m_db->GetTableColumn()<9 )
//		{
//			lResCount = 0;
//			m_csCommit.Unlock();
//
//			return 0;
//		}
//
//		int nChanNum = m_db->GetTableRow();
//
//		LINK_DATA*  pChanInfo = new LINK_DATA[nChanNum];
//		memset(pChanInfo, 0, nChanNum*sizeof(LINK_DATA));
//
//		for( int i=0; i<m_db->GetTableRow(); i++ )
//		{
//			int j = 0;//列
//
//			//j++;
//			pChanInfo[i].id = atoi( m_db->GetTableData(j, i+1) );//id
//			j++;
//			pChanInfo[i].nAnaChannel = atoi( m_db->GetTableData(j, i+1) );//识别通道
//			j++;
//			strcpy_s(pChanInfo[i].szDevName, 64, m_db->GetTableData(j, i+1) );//设备Name
//			j++;
//			strcpy_s(pChanInfo[i].szDevIP, 64, m_db->GetTableData(j, i+1) );//设备IP
//			j++;
//			pChanInfo[i].nDevPort = atoi( m_db->GetTableData(j, i+1) );//设备PORT
//			j++;
//			pChanInfo[i].nDevChan = atoi( m_db->GetTableData(j, i+1) );//设备Channel
//			j++;			
//			strcpy_s(pChanInfo[i].szDevManu, 32, m_db->GetTableData(j, i+1) );//设备manu
//			j++;			
//			strcpy_s(pChanInfo[i].szLoginUser, 64, m_db->GetTableData(j, i+1) );//LoginUser
//			j++;
//			strcpy_s(pChanInfo[i].szLoginPass, 64, m_db->GetTableData(j, i+1) );//LoginPass
//
//			//break;
//		}
//
//		m_db->FreeTableData();
//
//
//		//////////////////////////////////////////////////////////////////////////
//		//再查询通道规则rules_table
//		bMulti = FALSE;
//		memset(szSql, 0, 1024);
//		sprintf_s( szSql, 1024, "SELECT * FROM %s", RULETAB );
//		
//		if( pInfo->id>0 )
//		{//索引号
//			sprintf_s( szSql, 1024, "%s WHERE id = %d", szSql, pInfo->id );
//
//			bMulti = TRUE;
//		}
//
//		if( pInfo->nAnaChannel>0 )
//		{//分析通道号
//			if( bMulti )
//			{
//				sprintf_s( szSql, 1024, "%s AND anachannel = %d", szSql, pInfo->nAnaChannel );
//			}
//			else
//			{
//				sprintf_s( szSql, 1024, "%s WHERE anachannel = %d", szSql, pInfo->nAnaChannel );
//
//				bMulti = TRUE;
//			}
//		}
//
//		if( pInfo->nAnaMainType>0 )
//		{//算法类型
//			if( bMulti )
//			{
//				sprintf_s( szSql, 1024, "%s AND anamaintype = %d", szSql, pInfo->nAnaMainType );
//			}
//			else
//			{
//				sprintf_s( szSql, 1024, "%s WHERE anamaintype = %d", szSql, pInfo->nAnaSubType );
//
//				bMulti = TRUE;
//			}
//		}
//
//		if( pInfo->nAnaSubType>0 )
//		{//识别子类型
//			if( bMulti )
//			{
//				sprintf_s( szSql, 1024, "%s AND anasubtype = %d", szSql, pInfo->nAnaSubType );
//			}
//			else
//			{
//				sprintf_s( szSql, 1024, "%s WHERE anasubtype = %d", szSql, pInfo->nAnaSubType );
//
//				bMulti = TRUE;
//			}
//		}
//
//		//执行查询
//		lRet = m_db->SQLiteSearch( szSql );
//
//		if ( m_db->GetTableRow()<=0 || m_db->GetTableColumn()<15 )
//		{
//			//返回通道信息
//			lResCount = nChanNum;
//			pResult   = pChanInfo;
//
//			m_csCommit.Unlock();
//
//			return 0;
//		}
//
//		//获取查询结果
//		lResCount = m_db->GetTableRow();
//
//		LINK_DATA  *pLink = new LINK_DATA[lResCount];
//		memset( pLink, 0, lResCount*sizeof(LINK_DATA) );
//
//		for( int i=0; i<lResCount; i++ )
//		{
//			int j = 0;//列
//
//			//j++;
//			pLink[i].id = atoi( m_db->GetTableData(j, i+1) );//id
//			j++;
//			pLink[i].nAnaChannel = atoi( m_db->GetTableData(j, i+1) );//识别通道
//			j++;
//			pLink[i].nAnaMainType = atoi( m_db->GetTableData(j, i+1) );//算法类型
//			j++;
//			pLink[i].nAnaSubType = atoi( m_db->GetTableData(j, i+1) );//识别子类型
//			j++;
//			pLink[i].nLineNum = atoi( m_db->GetTableData(j, i+1));//规则个数
//			j++;
//			strcpy_s(pLink[i].szBZSLineType[0], 256, m_db->GetTableData(j, i+1));//Line1规则1
//			j++;
//			strcpy_s(pLink[i].szBZSLineType[1], 256, m_db->GetTableData(j, i+1));//Line1规则2
//			j++;
//			strcpy_s(pLink[i].szBZSLineType[2], 256, m_db->GetTableData(j, i+1));//Line1规则3
//			j++;
//			strcpy_s(pLink[i].szBZSLineType[3], 256, m_db->GetTableData(j, i+1));//Line1规则4
//			j++;
//			strcpy_s(pLink[i].szBZSLineType[4], 256, m_db->GetTableData(j, i+1));//Line1规则5
//			j++;
//			strcpy_s(pLink[i].szBZSLineType_2[0], 256, m_db->GetTableData(j, i+1));//Line2规则1
//			j++;
//			strcpy_s(pLink[i].szBZSLineType_2[1], 256, m_db->GetTableData(j, i+1));//Line2规则2
//			j++;
//			strcpy_s(pLink[i].szBZSLineType_2[2], 256, m_db->GetTableData(j, i+1));//Line2规则3
//			j++;
//			strcpy_s(pLink[i].szBZSLineType_2[3], 256, m_db->GetTableData(j, i+1));//Line2规则4
//			j++;
//			strcpy_s(pLink[i].szBZSLineType_2[4], 256, m_db->GetTableData(j, i+1));//Line2规则5
//
//
//			//填充通道信息
//			for ( int k=0; k<nChanNum; k++ )
//			{
//				if ( pChanInfo[k].nAnaChannel==pLink[i].nAnaChannel )
//				{
//					strcpy_s(pLink[i].szDevName, 64, pChanInfo[k].szDevName);//设备Name
//					strcpy_s(pLink[i].szDevIP,   64, pChanInfo[k].szDevIP);//设备IP
//					pLink[i].nDevPort = pChanInfo[k].nDevPort;//设备PORT
//					pLink[i].nDevChan = pChanInfo[k].nDevChan;//设备Channel			
//					strcpy_s(pLink[i].szDevManu,   32, pChanInfo[k].szDevManu);//设备manu		
//					strcpy_s(pLink[i].szLoginUser, 64, pChanInfo[k].szLoginUser);//LoginUser
//					strcpy_s(pLink[i].szLoginPass, 64, pChanInfo[k].szLoginPass);//LoginPass
//
//					break;
//				}
//			}
//		}
//
//		m_db->FreeTableData();
//
//		if ( pChanInfo )
//		{
//			delete []pChanInfo;
//			pChanInfo = NULL;
//		}
//
//		pResult = pLink;
//	}
//
//	m_csCommit.Unlock();
//
//	return 0;
//}

//查询关联通道
long CDBManage::GetLinkInfo( LPVOID param, long &lResCount, LPVOID &pResult )
{
	m_csCommit.Lock();

	if( NULL==param || NULL==m_db || pResult )
	{
		m_csCommit.Unlock();

		return -1;
	}

	lResCount = 0;

	char szSql[1024] = {'\0'};
	long lRet = 0;
	BOOL bMulti = FALSE;

	LINK_DATA* pInfo = (LINK_DATA*)param;

	if( pResult==NULL )//返回 lResCount 结果到 pResult 中
	{
		//
		//查询channels_table中的通道联动设备信息

		sprintf_s( szSql, 1024, "SELECT * FROM %s", TBCHAN );
		if( pInfo->id>0 )
		{//索引号
			sprintf_s( szSql, 1024, "%s WHERE id = %d", szSql, pInfo->id );
			bMulti = TRUE;
		}

		if( pInfo->nAnaChannel>0 )
		{//分析通道号
			if( bMulti )
			{
				sprintf_s( szSql, 1024, "%s AND anachannel = %d", szSql, pInfo->nAnaChannel );
			}
			else
			{
				sprintf_s( szSql, 1024, "%s WHERE anachannel = %d", szSql, pInfo->nAnaChannel );

				bMulti = TRUE;
			}
		}

		//执行查询
		lRet = m_db->SQLiteSearch( szSql );

		if ( m_db->GetTableRow()<=0 || m_db->GetTableColumn()<12 )
		{
			lResCount = 0;
			m_csCommit.Unlock();

			return 0;
		}

		lResCount = m_db->GetTableRow();

		LINK_DATA*  pChanInfo = new LINK_DATA[lResCount];
		memset(pChanInfo, 0, lResCount*sizeof(LINK_DATA));

		for( int i=0; i<m_db->GetTableRow(); i++ )
		{
			int j = 0;//列

			pChanInfo[i].id = atoi( m_db->GetTableData(j, i+1) );//id
			j++;
			pChanInfo[i].nAnaChannel = atoi( m_db->GetTableData(j, i+1) );//识别通道
			j++;
			strcpy_s(pChanInfo[i].szDevName, 64, m_db->GetTableData(j, i+1) );//设备Name
			j++;
			strcpy_s(pChanInfo[i].szDevIP, 64, m_db->GetTableData(j, i+1) );//设备IP
			j++;
			pChanInfo[i].nDevPort = atoi( m_db->GetTableData(j, i+1) );//设备PORT
			j++;
			pChanInfo[i].nDevChan = atoi( m_db->GetTableData(j, i+1) );//设备Channel
			j++;			
			strcpy_s(pChanInfo[i].szDevManu, 32, m_db->GetTableData(j, i+1) );//设备manu
			j++;			
			strcpy_s(pChanInfo[i].szLoginUser, 64, m_db->GetTableData(j, i+1) );//LoginUser
			j++;
			strcpy_s(pChanInfo[i].szLoginPass, 64, m_db->GetTableData(j, i+1) );//LoginPass
			j++;
			pChanInfo[i].nPtziPos = atoi( m_db->GetTableData(j, i+1) );//云台初始预置点
			j++;
			pChanInfo[i].nPtztPos = atoi( m_db->GetTableData(j, i+1) );//云台目标预置点
			j++;
			pChanInfo[i].nHLimit = atoi( m_db->GetTableData(j, i+1) );//高度标尺
		}

		m_db->FreeTableData();

		pResult = pChanInfo;
	}

	m_csCommit.Unlock();

	return 0;
}

//更新关联通道
long CDBManage::SetLinkInfo( int nChanId, LPVOID param )
{
	m_csCommit.Lock();

	if( NULL==param || NULL==m_db || 0>nChanId )
	{
		m_csCommit.Unlock();
		return -1;
	}

	int nRow = 0;

	LINK_DATA  *pChan = (LINK_DATA*)param;

	char szSql[1024] = {'\0'};

	//查询是否记录
	sprintf_s(szSql, 1024, "SELECT * FROM %s WHERE anachannel=%d", TBCHAN, nChanId, pChan->nAnaMainType, pChan->nAnaSubType);

	m_db->SQLiteSearch( szSql );
	nRow = m_db->GetTableRow();
	m_db->FreeTableData();

	if ( nRow<=0 )
	{
		//插入数据
		//id, anachannel, devicename, deviceip, deviceport, devicechan, devicemanu, loginuser, loginpass, ptzipos, ptztpos, hlimit
		sprintf_s( szSql, 1024, "INSERT INTO \"%s\" VALUES( NULL, %d, \"%s\", \"%s\", %d, %d, \"%s\", \"%s\", \"%s\", %d, %d, %d )",
			TBCHAN, pChan->nAnaChannel, pChan->szDevName, pChan->szDevIP, pChan->nDevPort, pChan->nDevChan, pChan->szDevManu, pChan->szLoginUser, pChan->szLoginPass,
			pChan->nPtziPos, pChan->nPtztPos, pChan->nHLimit ) ;
	}
	else
	{
		//更新数据
		if ( pChan->nHLimit>0 )
		{
			//id, anachannel, devicename, deviceip, deviceport, devicechan, devicemanu, loginuser, loginpass, ptzipos, ptztpos, hlimit
			sprintf_s( szSql, 1024, "UPDATE %s SET hlimit = %d WHERE anachannel = %d",
				TBCHAN, pChan->nHLimit, nChanId ) ;
		}
		//else if ( pChan->nPtziPos>0 )
		//{
		//	//id, anachannel, devicename, deviceip, deviceport, devicechan, devicemanu, loginuser, loginpass, ptzipos, ptztpos, hlimit
		//	sprintf_s( szSql, 1024, "UPDATE %s SET ptzipos = %d WHERE anachannel = %d",
		//		CHANTAB, pChan->nPtziPos, nChanId ) ;
		//}
		//else if ( pChan->nPtztPos>0 )
		//{
		//	//id, anachannel, devicename, deviceip, deviceport, devicechan, devicemanu, loginuser, loginpass, ptzipos, ptztpos, hlimit
		//	sprintf_s( szSql, 1024, "UPDATE %s SET ptztpos = %d WHERE anachannel = %d",
		//		CHANTAB, pChan->nPtztPos, nChanId ) ;
		//}
		else
		{
			//id, anachannel, devicename, deviceip, deviceport, devicechan, devicemanu, loginuser, loginpass, ptzipos, ptztpos, hlimit
			sprintf_s( szSql, 1024, "UPDATE %s SET devicename='%s', deviceip = '%s', deviceport = %d, devicechan = %d, devicemanu = '%s', loginuser = '%s', loginpass = '%s', ptzipos = %d, ptztpos = %d, hlimit = %d WHERE anachannel = %d",
				TBCHAN, pChan->szDevIP, pChan->szDevIP, pChan->nDevPort, pChan->nDevChan, pChan->szDevManu, pChan->szLoginUser, pChan->szLoginPass, pChan->nPtziPos, pChan->nPtztPos, pChan->nHLimit, nChanId ) ;
		}
	}


	long lRet = m_db->SQLiteExec( szSql );

	m_csCommit.Unlock();

	m_bCommit = TRUE;

	return lRet;
}

//释放缓冲区
void CDBManage::FreeLinkInfo( char* &pBuffer )
{
	if ( pBuffer!=NULL )
	{
		delete []pBuffer;
		pBuffer = NULL;
	}
}

//添加规则
long CDBManage::AddRuleInfo( LPVOID param )
{
	m_csCommit.Lock();

	if( NULL==param || NULL==m_db )
	{
		m_csCommit.Unlock();
		return -1;
	}

	LINK_DATA* pChan = (LINK_DATA*)param;

	char szSql[1024] = {'\0'};

	//id, anachannel, anamaintype, anasubtype, linenum, line1_1, line1_2, line1_3, line1_4, line1_5, line2_1, line2_2, line2_3, line2_4, line2_5, direct_1, direct_2, direct_3, direct_4, direct_5
	sprintf_s( szSql, 1024, "INSERT INTO \"%s\" VALUES( NULL, %d, %d, %d, %d, \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\" )",
		TBCHAN, pChan->nAnaChannel, pChan->nAnaMainType, pChan->nAnaSubType, pChan->nLineNum, pChan->szBZSLineType[0], pChan->szBZSLineType[1], pChan->szBZSLineType[2], pChan->szBZSLineType[3],
		pChan->szBZSLineType[4], pChan->szBZSLineType_2[0], pChan->szBZSLineType_2[1], pChan->szBZSLineType_2[2], pChan->szBZSLineType_2[3], pChan->szBZSLineType_2[4], pChan->szAlarmDirect[0], pChan->szAlarmDirect[1],
		pChan->szAlarmDirect[2], pChan->szAlarmDirect[3], pChan->szAlarmDirect[4] ) ;

	long  lRet = m_db->SQLiteExec( szSql );

	m_csCommit.Unlock();

	m_bCommit = TRUE;

	long lNewId = m_db->getLastInsertId();

	return lNewId;
}

//获取规则
long CDBManage::GetRuleInfo( LPVOID param, long &lResCount, LPVOID &pResult )
{
	m_csCommit.Lock();

	if( NULL==param || NULL==m_db || pResult )
	{
		m_csCommit.Unlock();

		return -1;
	}

	lResCount = 0;

	long lRet = 0;
	BOOL bMulti = FALSE;

	char szSql[1024] = {'\0'};	

	LINK_DATA* pInfo = (LINK_DATA*)param;

	if( pResult==NULL )//返回 lResCount 结果到 pResult 中
	{
		//
		//查询通道规则rules_table
		//
		sprintf_s( szSql, 1024, "SELECT * FROM %s", TBRULE );

		if( pInfo->id>0 )
		{//索引号
			sprintf_s( szSql, 1024, "%s WHERE id = %d", szSql, pInfo->id );

			bMulti = TRUE;
		}

		if( pInfo->nAnaChannel>0 )
		{//分析通道号
			if( bMulti )
			{
				sprintf_s( szSql, 1024, "%s AND anachannel = %d", szSql, pInfo->nAnaChannel );
			}
			else
			{
				sprintf_s( szSql, 1024, "%s WHERE anachannel = %d", szSql, pInfo->nAnaChannel );

				bMulti = TRUE;
			}
		}

		if( pInfo->nAnaMainType>0 )
		{//算法类型
			if( bMulti )
			{
				sprintf_s( szSql, 1024, "%s AND anamaintype = %d", szSql, pInfo->nAnaMainType );
			}
			else
			{
				sprintf_s( szSql, 1024, "%s WHERE anamaintype = %d", szSql, pInfo->nAnaSubType );

				bMulti = TRUE;
			}
		}

		if( pInfo->nAnaSubType>0 )
		{//识别子类型
			if( bMulti )
			{
				sprintf_s( szSql, 1024, "%s AND anasubtype = %d", szSql, pInfo->nAnaSubType );
			}
			else
			{
				sprintf_s( szSql, 1024, "%s WHERE anasubtype = %d", szSql, pInfo->nAnaSubType );

				bMulti = TRUE;
			}
		}

		//执行查询
		lRet = m_db->SQLiteSearch( szSql );

		if ( m_db->GetTableRow()<=0 || m_db->GetTableColumn()<20 )
		{
			lResCount = 0;
			m_csCommit.Unlock();

			return 0;
		}

		//获取查询结果
		lResCount = m_db->GetTableRow();

		LINK_DATA  *pLink = new LINK_DATA[lResCount];
		memset( pLink, 0, lResCount*sizeof(LINK_DATA) );

		for( int i=0; i<lResCount; i++ )
		{
			int j = 0;//列

			pLink[i].id = atoi( m_db->GetTableData(j, i+1) );//id
			j++;
			pLink[i].nAnaChannel = atoi( m_db->GetTableData(j, i+1) );//识别通道
			j++;
			pLink[i].nAnaMainType = atoi( m_db->GetTableData(j, i+1) );//算法类型
			j++;
			pLink[i].nAnaSubType = atoi( m_db->GetTableData(j, i+1) );//识别子类型
			j++;
			pLink[i].nLineNum = atoi( m_db->GetTableData(j, i+1));//规则个数
			j++;
			strcpy_s(pLink[i].szBZSLineType[0], 256, m_db->GetTableData(j, i+1));//Line1规则1
			j++;
			strcpy_s(pLink[i].szBZSLineType[1], 256, m_db->GetTableData(j, i+1));//Line1规则2
			j++;
			strcpy_s(pLink[i].szBZSLineType[2], 256, m_db->GetTableData(j, i+1));//Line1规则3
			j++;
			strcpy_s(pLink[i].szBZSLineType[3], 256, m_db->GetTableData(j, i+1));//Line1规则4
			j++;
			strcpy_s(pLink[i].szBZSLineType[4], 256, m_db->GetTableData(j, i+1));//Line1规则5
			j++;
			strcpy_s(pLink[i].szBZSLineType_2[0], 256, m_db->GetTableData(j, i+1));//Line2规则1
			j++;
			strcpy_s(pLink[i].szBZSLineType_2[1], 256, m_db->GetTableData(j, i+1));//Line2规则2
			j++;
			strcpy_s(pLink[i].szBZSLineType_2[2], 256, m_db->GetTableData(j, i+1));//Line2规则3
			j++;
			strcpy_s(pLink[i].szBZSLineType_2[3], 256, m_db->GetTableData(j, i+1));//Line2规则4
			j++;
			strcpy_s(pLink[i].szBZSLineType_2[4], 256, m_db->GetTableData(j, i+1));//Line2规则5
			j++;
			strcpy_s(pLink[i].szAlarmDirect[0], 256, m_db->GetTableData(j, i+1));//警戒方向1
			j++;
			strcpy_s(pLink[i].szAlarmDirect[1], 256, m_db->GetTableData(j, i+1));//警戒方向2
			j++;
			strcpy_s(pLink[i].szAlarmDirect[2], 256, m_db->GetTableData(j, i+1));//警戒方向3
			j++;
			strcpy_s(pLink[i].szAlarmDirect[3], 256, m_db->GetTableData(j, i+1));//警戒方向4
			j++;
			strcpy_s(pLink[i].szAlarmDirect[4], 256, m_db->GetTableData(j, i+1));//警戒方向5
			j++;
			pLink[i].nHLimit = atoi( m_db->GetTableData(j, i+1));//高度标尺
		}

		m_db->FreeTableData();

		pResult = pLink;//返回查询结果
	}

	m_csCommit.Unlock();

	return 0;
}

//更新规则
long CDBManage::SetRuleInfo( int nChanId, LPVOID param )
{
	m_csCommit.Lock();

	if( NULL==param || NULL==m_db || 0>nChanId )
	{
		m_csCommit.Unlock();
		return -1;
	}

	int nRow = 0;

	LINK_DATA  *pChan = (LINK_DATA*)param;

	char szSql[1024] = {'\0'};

	//查询是否记录
	sprintf_s(szSql, 1024, "SELECT * FROM %s WHERE anachannel=%d AND anamaintype=%d AND anasubtype=%d", TBRULE, nChanId, pChan->nAnaMainType, pChan->nAnaSubType);
	m_db->SQLiteSearch( szSql );
	nRow = m_db->GetTableRow();
	m_db->FreeTableData();

	
	if ( nRow<=0 )
	{
		//插入数据
		//id, anachannel, anamaintype, anasubtype, linenum, line1_1, line1_2, line1_3, line1_4, line1_5, line2_1, line2_2, line2_3, line2_4, line2_5, direct_1, direct_2, direct_3, direct_4, direct_5, hlimit
		sprintf_s( szSql, 1024, "INSERT INTO %s VALUES (NULL, %d, %d, %d, %d, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %d)",
			TBRULE, nChanId, pChan->nAnaMainType, pChan->nAnaSubType, pChan->nLineNum, pChan->szBZSLineType[0], pChan->szBZSLineType[1], pChan->szBZSLineType[2], pChan->szBZSLineType[3], pChan->szBZSLineType[4], pChan->szBZSLineType_2[0], pChan->szBZSLineType_2[1], pChan->szBZSLineType_2[2],
			pChan->szBZSLineType_2[3], pChan->szBZSLineType_2[4], pChan->szAlarmDirect[0], pChan->szAlarmDirect[1], pChan->szAlarmDirect[2], pChan->szAlarmDirect[3], pChan->szAlarmDirect[4], pChan->nHLimit ) ;
	}
	else
	{
		//更新数据
		//id, anachannel, anamaintype, anasubtype, linenum, line1_1, line1_2, line1_3, line1_4, line1_5, line2_1, line2_2, line2_3, line2_4, line2_5, direct_1, direct_2, direct_3, direct_4, direct_5, hlimit
		sprintf_s( szSql, 1024, "UPDATE %s SET linenum=%d, line1_1='%s', line1_2='%s', line1_3='%s', line1_4='%s', line1_5='%s', line2_1='%s', line2_2='%s', line2_3='%s', line2_4='%s', line2_5='%s', direct_1='%s', direct_2='%s', direct_3='%s', direct_4='%s', direct_5='%s', hlimit=%d WHERE anachannel = %d AND anamaintype=%d AND anasubtype=%d",
			TBRULE, pChan->nLineNum, pChan->szBZSLineType[0], pChan->szBZSLineType[1], pChan->szBZSLineType[2], pChan->szBZSLineType[3], pChan->szBZSLineType[4], pChan->szBZSLineType_2[0], pChan->szBZSLineType_2[1], pChan->szBZSLineType_2[2],
			pChan->szBZSLineType_2[3], pChan->szBZSLineType_2[4], pChan->szAlarmDirect[0], pChan->szAlarmDirect[1], pChan->szAlarmDirect[2], pChan->szAlarmDirect[3], pChan->szAlarmDirect[4], pChan->nHLimit, nChanId, pChan->nAnaMainType, pChan->nAnaSubType ) ;
	}

	long lRet = m_db->SQLiteExec( szSql );

	m_csCommit.Unlock();

	m_bCommit = TRUE;

	return lRet;
}

//删除通道规则
long CDBManage::DelRuleInfo( LPVOID param )
{
	m_csCommit.Lock();

	if( NULL==param || NULL==m_db )
	{
		m_csCommit.Unlock();
		return -1;
	}

	BOOL bMulti = FALSE;

	char szSql[1024] = {'\0'};	

	LINK_DATA  *pInfo = (LINK_DATA*)param;

	sprintf_s( szSql, 1024, "DELETE FROM \"%s\"", TBRULE ) ;

	if( pInfo->id>0 )
	{//索引号
		sprintf_s( szSql, 1024, "%s WHERE id = %d", szSql, pInfo->id ) ;
		bMulti = TRUE;
	}

	if( pInfo->nAnaChannel>0 )
	{//分析类型
		if( bMulti )
		{
			sprintf_s( szSql, 1024, "%s AND anachannel = %d", szSql, pInfo->nAnaChannel );
		}
		else
		{
			sprintf_s( szSql, 1024, "%s WHERE anachannel = %d", szSql, pInfo->nAnaChannel );
			bMulti = TRUE;
		}
	}

	if ( pInfo->nAnaMainType>0 && pInfo->nAnaSubType>0 )
	{
		if( bMulti )
		{
			sprintf_s( szSql, 1024, "%s AND anamaintype=%d and anasubtype=%d", szSql, pInfo->nAnaMainType, pInfo->nAnaSubType );
		}
		else
		{
			sprintf_s( szSql, 1024, "%s WHERE anamaintype=%d and anasubtype=%d", szSql, pInfo->nAnaMainType, pInfo->nAnaSubType );
			bMulti = TRUE;
		}
	}

	long lRet = m_db->SQLiteExec( szSql );

	m_csCommit.Unlock();

	m_bCommit = TRUE;

	return lRet;
}


