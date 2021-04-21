#pragma once
//#include "..\log\msg_log.h"
#include<afxole.h>
#import "C:\Program Files\Common Files\System\ado\msado15.dll" no_namespace rename("EOF","adoEOF")

class ado_conn
{
private:

	string username;
	string password;
	string conn_str;

	_ConnectionPtr m_pConnection;
	_CommandPtr    m_pCommand;
	_RecordsetPtr  m_pRecordset;

public:
	ado_conn(void);
	ado_conn(string username, string password, string conn_str);
	~ado_conn(void);
	ado_conn* handle(void) { return this; };

	//----------------------------------------------------------
	bool  conn(string username, string password, string conn_str);
	bool  db_query(const char* commandTxt);
	bool  db_update(const char* commandTxt);
	bool  next(void);
	bool  is_empty(void);
	//----------------------------------------------------------
	bool    get_double(int Index, double* value);
	bool    get_string(int Index, string* value);
	bool    get_int(int Index, int* value);
	double  get_double(const char* FieldName);
	int     get_int(const char* FieldName);
	string  get_string(const char* FieldName);
	//----------------------------------------------------------
	bool    is_open(void) { return connState; };
	bool    is_used(void) { return used; };
	void    is_used(bool used) { this->used = used; };

	bool    pool_mode;
	//----------------------------------------------------------
	bool    close(void);
	bool    release(void);

private:
	bool    conn(void);
	bool    end_execute(void);
	bool    end_rs(void);
	bool    end_conn(void);
	bool    end_cmd(void);
	bool    connState;
	bool    rsState;
	bool    cmdState;
	bool    used;
	static  bool    initState;
	//msg_log *sql_log;
};
