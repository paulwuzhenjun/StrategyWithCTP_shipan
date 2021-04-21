#ifndef __CDATA_BASE_H__
#define __CDATA_BASE_H__

#include <iostream>
#include <stdio.h>
#include <ICRSINT.H>
#include <string>
using namespace std;
#import "c:\\program files\\common files\\system\\ado\\msado15.dll"  no_namespace rename("EOF", "adoEOF")

class COracle
{
public:
	//初始化数据库操作需要的对象
	COracle(void);
	~COracle(void);

	//连接至数据库  "Provider=OraOLEDB.Oracle.1;Persist Security Info=True;Data Source=QJDATA" 驱动问题
	// 安装了client和server 所以驱动默认是server,是64位，所以驱动选择时报错，so采用微软自带驱动
	bool ConnToDB(char* ConnectionString, char* UserID, char* Password);

	//数据库操作函数
	//查询操作 删除以及添加
	_RecordsetPtr ExecuteWithResSQL(const char*);

private:
	void PrintErrorInfo(_com_error&);

private:
	//初始化数据库连接、命令、记录集
	_ConnectionPtr CreateConnPtr();
	_CommandPtr CreateCommPtr();
	_RecordsetPtr CreateRecsetPtr();

private:
	//数据库连接需要的连接、命令操作对象
	_ConnectionPtr m_pConnection;
	_CommandPtr m_pCommand;
};

class CDataBase
{
public:
	CDataBase();
	virtual ~CDataBase();
	virtual CDataBase* createBase() = 0;
};

#endif