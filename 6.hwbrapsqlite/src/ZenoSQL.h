/************************************************************************************ 
     * Copyright (c) houwenbin All Rights Reserved. 
     * 公司名称：杭州智诺科技股份有限公司
     * 文件名：  ZenoSQL.h
     * 创建人：  侯文斌
     * 电子邮箱：houwenbin1986@gmail.com
     * 描述    : 封装sqlite3数据库api，提供更简单、明了、友好的接口
*/

#ifndef ZENOSQL_H_
#define ZENOSQL_H_

#include "sqlite3.h"


//封装SQLite3操作
class CZenoSQL
{
public:
	CZenoSQL(void);
	~CZenoSQL(void);

public:
	 long  SQLiteConnect(char *pFileName);//连接数据库
	 long  SQLiteDisconnect(void);//断开数据库连接
	 long  SQLiteExec(char *pSQL);//执行SQL命令
	 long  SQLiteSearch(char *pSearch);//查询命令

	 
	 int  GetTableRow()//查询后，取得表“行”数
	 {
		 return m_nRow;
	 }
	 
     int  GetTableColumn()//查询后，取得表“列”数
	 {
		 return m_nCol;
	 }
	 
     char *GetErrorMsg()//取得当前错误提示
	 {
		 return m_szErrMsg;
	 }
	 
     char *GetTableData( int col, int row )//查询后，取得表内某个单元值
	 {
		 if( m_pResult==NULL )
		 {
			 return NULL;
		 }

		 //m_pResult结果是带表头的整张二维表格，所以数据row从1开始
		 return *( m_pResult + col + row*m_nCol );
	 }
	 
	 void FreeTableData()//释放查询结果
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

	 long getLastInsertId()//获取最近插入ID
	 {
		 if( NULL==m_db )
		 {
			 return -1;
		 }
		 return long(sqlite3_last_insert_rowid(m_db));
	 }

private:	 
	char       **m_pResult;//二维数组存放结果     
	char         m_szErrMsg[512];//保存错误信息
	char        *m_pError;

	int          m_nRow;//数据行数
	int          m_nCol;//数据列数

private:
	sqlite3     *m_db;//数据库句柄
};

#endif

