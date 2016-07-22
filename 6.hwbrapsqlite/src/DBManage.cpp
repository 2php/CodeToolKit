#include "StdAfx.h"
#include "DBManage.h"

#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include <atlstr.h>


//�����ύ�߳�
DWORD  WINAPI  CommitTransactionProc( LPVOID param );

//
//���ݿ������
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


//��ʼ����
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

//��������
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


//��ȡ�����¼
char* CDBManage::GetDBLastError()
{
	if( NULL==m_db )
	{
		return NULL;
	}

	return m_db->GetErrorMsg();
}


//�豸��ʾ��Ϣ�ص�
long CDBManage::SetMsgCallback( PZENOSQLITECALLBACK pMsgCallback, void *pContext )
{
	m_pMsgCallback = pMsgCallback;
	m_pContext     = pContext;

	return 0;
}


//�ύ�����߳�
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
			//�ύ��������
			pMain->EndTransaction();

			//������һ������
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


//�������ݿ�
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
	//_getcwd(pBuf, 256);//���ַ����õ���·����ÿ�δ�һ���µĶԻ����絼��ͼƬ���õ���·���ͻ���ͼƬ���ڵ�·�������ǳ�����·��
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


	//�������������ݿ�
	long lRet = m_db->SQLiteConnect( pSqlPath );
	
	//��������
	BeginTransaction();

	m_csCommit.Unlock();

	//ֹͣ�ѿ������������߳�
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

	//���������߳�
	m_hEvent     = CreateEvent( NULL, FALSE, FALSE, NULL );
	m_hCommitPro = CreateThread( NULL, 0, CommitTransactionProc, this, 0, NULL );

	return lRet;
}

//�ر����ݿ⣬�ύ����
long CDBManage::DisconnectSQL()
{
	if( NULL==m_db )
	{
		return -1;
	}

	//�ύ����
	EndTransaction();

	//ֹͣ�����߳�
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


//��ʼ���������ݱ�
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

	//����ͨ���豸��
	sprintf_s( szSql, 1024, "CREATE TABLE %s ( id INTEGER PRIMARY KEY autoincrement, anachannel INTEGER, devicename VARCHAR(64), deviceip VARCHAR(64), deviceport INTEGER, devicechan INTEGER, devicemanu VARCHAR(32), loginuser VARCHAR(64), loginpass VARCHAR(64), ptzipos INTEGER, ptztpos INTEGER, hlimit INTEGER );", TBCHAN ) ;

	lRet = m_db->SQLiteExec( szSql );

	//����ͨ�������
	sprintf_s( szSql, 1024, "CREATE TABLE %s ( id INTEGER PRIMARY KEY autoincrement, anachannel INTEGER, anamaintype INTEGER, anasubtype INTEGER, linenum INTEGER, line1_1 VARCHAR(256), line1_2 VARCHAR(256), line1_3 VARCHAR(256), line1_4 VARCHAR(256), line1_5 VARCHAR(256), line2_1 VARCHAR(256), line2_2 VARCHAR(256), line2_3 VARCHAR(256), line2_4 VARCHAR(256), line2_5 VARCHAR(256), direct_1 VARCHAR(256), direct_2 VARCHAR(256), direct_3 VARCHAR(256), direct_4 VARCHAR(256), direct_5 VARCHAR(256), hlimit INTEGER );", TBRULE ) ;

	lRet = m_db->SQLiteExec( szSql );

	m_csCommit.Unlock();

	m_bCommit = TRUE;

	return lRet;
}


//ɾ�����ݱ�X
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
//����������¼
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

//ɾ������ͨ��
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
	{//����ͨ����
		sprintf_s( szSql, 1024, "%s WHERE anachannel = %d", szSql, pInfo->id ) ;
		bMulti = TRUE;
	}

	if( pInfo->nAnaChannel>0 )
	{//��������
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

////��ѯ����ͨ��
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
//	if( pResult==NULL )//���� lResCount ����� pResult ��
//	{
//
//		//////////////////////////////////////////////////////////////////////////
//		//�Ȳ�ѯchannels_table�е�ͨ�������豸��Ϣ
//
//		sprintf_s( szSql, 1024, "SELECT * from %s", CHANTAB );
//		if( pInfo->id>0 )
//		{//������
//			sprintf_s( szSql, 1024, "%s WHERE id = %d", szSql, pInfo->id );
//			bMulti = TRUE;
//		}
//
//		if( pInfo->nAnaChannel>0 )
//		{//����ͨ����
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
//		//ִ�в�ѯ
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
//			int j = 0;//��
//
//			//j++;
//			pChanInfo[i].id = atoi( m_db->GetTableData(j, i+1) );//id
//			j++;
//			pChanInfo[i].nAnaChannel = atoi( m_db->GetTableData(j, i+1) );//ʶ��ͨ��
//			j++;
//			strcpy_s(pChanInfo[i].szDevName, 64, m_db->GetTableData(j, i+1) );//�豸Name
//			j++;
//			strcpy_s(pChanInfo[i].szDevIP, 64, m_db->GetTableData(j, i+1) );//�豸IP
//			j++;
//			pChanInfo[i].nDevPort = atoi( m_db->GetTableData(j, i+1) );//�豸PORT
//			j++;
//			pChanInfo[i].nDevChan = atoi( m_db->GetTableData(j, i+1) );//�豸Channel
//			j++;			
//			strcpy_s(pChanInfo[i].szDevManu, 32, m_db->GetTableData(j, i+1) );//�豸manu
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
//		//�ٲ�ѯͨ������rules_table
//		bMulti = FALSE;
//		memset(szSql, 0, 1024);
//		sprintf_s( szSql, 1024, "SELECT * FROM %s", RULETAB );
//		
//		if( pInfo->id>0 )
//		{//������
//			sprintf_s( szSql, 1024, "%s WHERE id = %d", szSql, pInfo->id );
//
//			bMulti = TRUE;
//		}
//
//		if( pInfo->nAnaChannel>0 )
//		{//����ͨ����
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
//		{//�㷨����
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
//		{//ʶ��������
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
//		//ִ�в�ѯ
//		lRet = m_db->SQLiteSearch( szSql );
//
//		if ( m_db->GetTableRow()<=0 || m_db->GetTableColumn()<15 )
//		{
//			//����ͨ����Ϣ
//			lResCount = nChanNum;
//			pResult   = pChanInfo;
//
//			m_csCommit.Unlock();
//
//			return 0;
//		}
//
//		//��ȡ��ѯ���
//		lResCount = m_db->GetTableRow();
//
//		LINK_DATA  *pLink = new LINK_DATA[lResCount];
//		memset( pLink, 0, lResCount*sizeof(LINK_DATA) );
//
//		for( int i=0; i<lResCount; i++ )
//		{
//			int j = 0;//��
//
//			//j++;
//			pLink[i].id = atoi( m_db->GetTableData(j, i+1) );//id
//			j++;
//			pLink[i].nAnaChannel = atoi( m_db->GetTableData(j, i+1) );//ʶ��ͨ��
//			j++;
//			pLink[i].nAnaMainType = atoi( m_db->GetTableData(j, i+1) );//�㷨����
//			j++;
//			pLink[i].nAnaSubType = atoi( m_db->GetTableData(j, i+1) );//ʶ��������
//			j++;
//			pLink[i].nLineNum = atoi( m_db->GetTableData(j, i+1));//�������
//			j++;
//			strcpy_s(pLink[i].szBZSLineType[0], 256, m_db->GetTableData(j, i+1));//Line1����1
//			j++;
//			strcpy_s(pLink[i].szBZSLineType[1], 256, m_db->GetTableData(j, i+1));//Line1����2
//			j++;
//			strcpy_s(pLink[i].szBZSLineType[2], 256, m_db->GetTableData(j, i+1));//Line1����3
//			j++;
//			strcpy_s(pLink[i].szBZSLineType[3], 256, m_db->GetTableData(j, i+1));//Line1����4
//			j++;
//			strcpy_s(pLink[i].szBZSLineType[4], 256, m_db->GetTableData(j, i+1));//Line1����5
//			j++;
//			strcpy_s(pLink[i].szBZSLineType_2[0], 256, m_db->GetTableData(j, i+1));//Line2����1
//			j++;
//			strcpy_s(pLink[i].szBZSLineType_2[1], 256, m_db->GetTableData(j, i+1));//Line2����2
//			j++;
//			strcpy_s(pLink[i].szBZSLineType_2[2], 256, m_db->GetTableData(j, i+1));//Line2����3
//			j++;
//			strcpy_s(pLink[i].szBZSLineType_2[3], 256, m_db->GetTableData(j, i+1));//Line2����4
//			j++;
//			strcpy_s(pLink[i].szBZSLineType_2[4], 256, m_db->GetTableData(j, i+1));//Line2����5
//
//
//			//���ͨ����Ϣ
//			for ( int k=0; k<nChanNum; k++ )
//			{
//				if ( pChanInfo[k].nAnaChannel==pLink[i].nAnaChannel )
//				{
//					strcpy_s(pLink[i].szDevName, 64, pChanInfo[k].szDevName);//�豸Name
//					strcpy_s(pLink[i].szDevIP,   64, pChanInfo[k].szDevIP);//�豸IP
//					pLink[i].nDevPort = pChanInfo[k].nDevPort;//�豸PORT
//					pLink[i].nDevChan = pChanInfo[k].nDevChan;//�豸Channel			
//					strcpy_s(pLink[i].szDevManu,   32, pChanInfo[k].szDevManu);//�豸manu		
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

//��ѯ����ͨ��
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

	if( pResult==NULL )//���� lResCount ����� pResult ��
	{
		//
		//��ѯchannels_table�е�ͨ�������豸��Ϣ

		sprintf_s( szSql, 1024, "SELECT * FROM %s", TBCHAN );
		if( pInfo->id>0 )
		{//������
			sprintf_s( szSql, 1024, "%s WHERE id = %d", szSql, pInfo->id );
			bMulti = TRUE;
		}

		if( pInfo->nAnaChannel>0 )
		{//����ͨ����
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

		//ִ�в�ѯ
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
			int j = 0;//��

			pChanInfo[i].id = atoi( m_db->GetTableData(j, i+1) );//id
			j++;
			pChanInfo[i].nAnaChannel = atoi( m_db->GetTableData(j, i+1) );//ʶ��ͨ��
			j++;
			strcpy_s(pChanInfo[i].szDevName, 64, m_db->GetTableData(j, i+1) );//�豸Name
			j++;
			strcpy_s(pChanInfo[i].szDevIP, 64, m_db->GetTableData(j, i+1) );//�豸IP
			j++;
			pChanInfo[i].nDevPort = atoi( m_db->GetTableData(j, i+1) );//�豸PORT
			j++;
			pChanInfo[i].nDevChan = atoi( m_db->GetTableData(j, i+1) );//�豸Channel
			j++;			
			strcpy_s(pChanInfo[i].szDevManu, 32, m_db->GetTableData(j, i+1) );//�豸manu
			j++;			
			strcpy_s(pChanInfo[i].szLoginUser, 64, m_db->GetTableData(j, i+1) );//LoginUser
			j++;
			strcpy_s(pChanInfo[i].szLoginPass, 64, m_db->GetTableData(j, i+1) );//LoginPass
			j++;
			pChanInfo[i].nPtziPos = atoi( m_db->GetTableData(j, i+1) );//��̨��ʼԤ�õ�
			j++;
			pChanInfo[i].nPtztPos = atoi( m_db->GetTableData(j, i+1) );//��̨Ŀ��Ԥ�õ�
			j++;
			pChanInfo[i].nHLimit = atoi( m_db->GetTableData(j, i+1) );//�߶ȱ��
		}

		m_db->FreeTableData();

		pResult = pChanInfo;
	}

	m_csCommit.Unlock();

	return 0;
}

//���¹���ͨ��
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

	//��ѯ�Ƿ��¼
	sprintf_s(szSql, 1024, "SELECT * FROM %s WHERE anachannel=%d", TBCHAN, nChanId, pChan->nAnaMainType, pChan->nAnaSubType);

	m_db->SQLiteSearch( szSql );
	nRow = m_db->GetTableRow();
	m_db->FreeTableData();

	if ( nRow<=0 )
	{
		//��������
		//id, anachannel, devicename, deviceip, deviceport, devicechan, devicemanu, loginuser, loginpass, ptzipos, ptztpos, hlimit
		sprintf_s( szSql, 1024, "INSERT INTO \"%s\" VALUES( NULL, %d, \"%s\", \"%s\", %d, %d, \"%s\", \"%s\", \"%s\", %d, %d, %d )",
			TBCHAN, pChan->nAnaChannel, pChan->szDevName, pChan->szDevIP, pChan->nDevPort, pChan->nDevChan, pChan->szDevManu, pChan->szLoginUser, pChan->szLoginPass,
			pChan->nPtziPos, pChan->nPtztPos, pChan->nHLimit ) ;
	}
	else
	{
		//��������
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

//�ͷŻ�����
void CDBManage::FreeLinkInfo( char* &pBuffer )
{
	if ( pBuffer!=NULL )
	{
		delete []pBuffer;
		pBuffer = NULL;
	}
}

//��ӹ���
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

//��ȡ����
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

	if( pResult==NULL )//���� lResCount ����� pResult ��
	{
		//
		//��ѯͨ������rules_table
		//
		sprintf_s( szSql, 1024, "SELECT * FROM %s", TBRULE );

		if( pInfo->id>0 )
		{//������
			sprintf_s( szSql, 1024, "%s WHERE id = %d", szSql, pInfo->id );

			bMulti = TRUE;
		}

		if( pInfo->nAnaChannel>0 )
		{//����ͨ����
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
		{//�㷨����
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
		{//ʶ��������
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

		//ִ�в�ѯ
		lRet = m_db->SQLiteSearch( szSql );

		if ( m_db->GetTableRow()<=0 || m_db->GetTableColumn()<20 )
		{
			lResCount = 0;
			m_csCommit.Unlock();

			return 0;
		}

		//��ȡ��ѯ���
		lResCount = m_db->GetTableRow();

		LINK_DATA  *pLink = new LINK_DATA[lResCount];
		memset( pLink, 0, lResCount*sizeof(LINK_DATA) );

		for( int i=0; i<lResCount; i++ )
		{
			int j = 0;//��

			pLink[i].id = atoi( m_db->GetTableData(j, i+1) );//id
			j++;
			pLink[i].nAnaChannel = atoi( m_db->GetTableData(j, i+1) );//ʶ��ͨ��
			j++;
			pLink[i].nAnaMainType = atoi( m_db->GetTableData(j, i+1) );//�㷨����
			j++;
			pLink[i].nAnaSubType = atoi( m_db->GetTableData(j, i+1) );//ʶ��������
			j++;
			pLink[i].nLineNum = atoi( m_db->GetTableData(j, i+1));//�������
			j++;
			strcpy_s(pLink[i].szBZSLineType[0], 256, m_db->GetTableData(j, i+1));//Line1����1
			j++;
			strcpy_s(pLink[i].szBZSLineType[1], 256, m_db->GetTableData(j, i+1));//Line1����2
			j++;
			strcpy_s(pLink[i].szBZSLineType[2], 256, m_db->GetTableData(j, i+1));//Line1����3
			j++;
			strcpy_s(pLink[i].szBZSLineType[3], 256, m_db->GetTableData(j, i+1));//Line1����4
			j++;
			strcpy_s(pLink[i].szBZSLineType[4], 256, m_db->GetTableData(j, i+1));//Line1����5
			j++;
			strcpy_s(pLink[i].szBZSLineType_2[0], 256, m_db->GetTableData(j, i+1));//Line2����1
			j++;
			strcpy_s(pLink[i].szBZSLineType_2[1], 256, m_db->GetTableData(j, i+1));//Line2����2
			j++;
			strcpy_s(pLink[i].szBZSLineType_2[2], 256, m_db->GetTableData(j, i+1));//Line2����3
			j++;
			strcpy_s(pLink[i].szBZSLineType_2[3], 256, m_db->GetTableData(j, i+1));//Line2����4
			j++;
			strcpy_s(pLink[i].szBZSLineType_2[4], 256, m_db->GetTableData(j, i+1));//Line2����5
			j++;
			strcpy_s(pLink[i].szAlarmDirect[0], 256, m_db->GetTableData(j, i+1));//���䷽��1
			j++;
			strcpy_s(pLink[i].szAlarmDirect[1], 256, m_db->GetTableData(j, i+1));//���䷽��2
			j++;
			strcpy_s(pLink[i].szAlarmDirect[2], 256, m_db->GetTableData(j, i+1));//���䷽��3
			j++;
			strcpy_s(pLink[i].szAlarmDirect[3], 256, m_db->GetTableData(j, i+1));//���䷽��4
			j++;
			strcpy_s(pLink[i].szAlarmDirect[4], 256, m_db->GetTableData(j, i+1));//���䷽��5
			j++;
			pLink[i].nHLimit = atoi( m_db->GetTableData(j, i+1));//�߶ȱ��
		}

		m_db->FreeTableData();

		pResult = pLink;//���ز�ѯ���
	}

	m_csCommit.Unlock();

	return 0;
}

//���¹���
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

	//��ѯ�Ƿ��¼
	sprintf_s(szSql, 1024, "SELECT * FROM %s WHERE anachannel=%d AND anamaintype=%d AND anasubtype=%d", TBRULE, nChanId, pChan->nAnaMainType, pChan->nAnaSubType);
	m_db->SQLiteSearch( szSql );
	nRow = m_db->GetTableRow();
	m_db->FreeTableData();

	
	if ( nRow<=0 )
	{
		//��������
		//id, anachannel, anamaintype, anasubtype, linenum, line1_1, line1_2, line1_3, line1_4, line1_5, line2_1, line2_2, line2_3, line2_4, line2_5, direct_1, direct_2, direct_3, direct_4, direct_5, hlimit
		sprintf_s( szSql, 1024, "INSERT INTO %s VALUES (NULL, %d, %d, %d, %d, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %d)",
			TBRULE, nChanId, pChan->nAnaMainType, pChan->nAnaSubType, pChan->nLineNum, pChan->szBZSLineType[0], pChan->szBZSLineType[1], pChan->szBZSLineType[2], pChan->szBZSLineType[3], pChan->szBZSLineType[4], pChan->szBZSLineType_2[0], pChan->szBZSLineType_2[1], pChan->szBZSLineType_2[2],
			pChan->szBZSLineType_2[3], pChan->szBZSLineType_2[4], pChan->szAlarmDirect[0], pChan->szAlarmDirect[1], pChan->szAlarmDirect[2], pChan->szAlarmDirect[3], pChan->szAlarmDirect[4], pChan->nHLimit ) ;
	}
	else
	{
		//��������
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

//ɾ��ͨ������
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
	{//������
		sprintf_s( szSql, 1024, "%s WHERE id = %d", szSql, pInfo->id ) ;
		bMulti = TRUE;
	}

	if( pInfo->nAnaChannel>0 )
	{//��������
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


