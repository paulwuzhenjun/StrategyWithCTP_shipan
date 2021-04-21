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
	//��ʼ�����ݿ������Ҫ�Ķ���
	COracle(void);
	~COracle(void);

	//���������ݿ�  "Provider=OraOLEDB.Oracle.1;Persist Security Info=True;Data Source=QJDATA" ��������
	// ��װ��client��server ��������Ĭ����server,��64λ����������ѡ��ʱ����so����΢���Դ�����
	bool ConnToDB(char* ConnectionString, char* UserID, char* Password);

	//���ݿ��������
	//��ѯ���� ɾ���Լ����
	_RecordsetPtr ExecuteWithResSQL(const char*);

private:
	void PrintErrorInfo(_com_error&);

private:
	//��ʼ�����ݿ����ӡ������¼��
	_ConnectionPtr CreateConnPtr();
	_CommandPtr CreateCommPtr();
	_RecordsetPtr CreateRecsetPtr();

private:
	//���ݿ�������Ҫ�����ӡ������������
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