/************************************************************************************ 
     * Copyright (c) houwenbin All Rights Reserved. 
     * ��˾���ƣ�������ŵ�Ƽ��ɷ����޹�˾
     * �ļ�����  ZenoSQL.h
     * �����ˣ�  ���ı�
     * �������䣺houwenbin1986@gmail.com
     * ����    : ��װsqlite3���ݿ�api���ṩ���򵥡����ˡ��ѺõĽӿ�
*/

#ifndef ZENOSQL_H_
#define ZENOSQL_H_

#include "sqlite3.h"


//��װSQLite3����
class CZenoSQL
{
public:
	CZenoSQL(void);
	~CZenoSQL(void);

public:
	 long  SQLiteConnect(char *pFileName);//�������ݿ�
	 long  SQLiteDisconnect(void);//�Ͽ����ݿ�����
	 long  SQLiteExec(char *pSQL);//ִ��SQL����
	 long  SQLiteSearch(char *pSearch);//��ѯ����

	 
	 int  GetTableRow()//��ѯ��ȡ�ñ��С���
	 {
		 return m_nRow;
	 }
	 
     int  GetTableColumn()//��ѯ��ȡ�ñ��С���
	 {
		 return m_nCol;
	 }
	 
     char *GetErrorMsg()//ȡ�õ�ǰ������ʾ
	 {
		 return m_szErrMsg;
	 }
	 
     char *GetTableData( int col, int row )//��ѯ��ȡ�ñ���ĳ����Ԫֵ
	 {
		 if( m_pResult==NULL )
		 {
			 return NULL;
		 }

		 //m_pResult����Ǵ���ͷ�����Ŷ�ά�����������row��1��ʼ
		 return *( m_pResult + col + row*m_nCol );
	 }
	 
	 void FreeTableData()//�ͷŲ�ѯ���
	 {
		 if( m_pResult!=NULL )
		 {
			 sqlite3_free_table( m_pResult );
			 m_pResult = NULL;
			 m_nCol = 0;
			 m_nRow = 0;
			 m_szErrMsg[0] = '\0';
		 }
	 }

	 long getLastInsertId()//��ȡ�������ID
	 {
		 if( NULL==m_db )
		 {
			 return -1;
		 }
		 return long(sqlite3_last_insert_rowid(m_db));
	 }

private:	 
	char       **m_pResult;//��ά�����Ž��     
	char         m_szErrMsg[512];//���������Ϣ
	char        *m_pError;

	int          m_nRow;//��������
	int          m_nCol;//��������

private:
	sqlite3     *m_db;//���ݿ���
};

#endif

