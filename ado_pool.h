#pragma once
#include <list>
#include "ado_conn.h"
#include "afxtempl.h"
#include "afxmt.h"

class ado_pool//:my_thread
{
private:
	CCriticalSection CriticalSection;

	list<ado_conn*> conn_list;// from 1 to max_conn

	int max_conn;
	int min_conn;
	int inc_conn;

	string user_name;
	string password;
	string tnsname;

	int conn_count;

public:
	ado_pool(int max_conn, int min_conn, int inc_conn, string user_name, string password, string tnsname);
	~ado_pool(void);
	ado_conn* get_conn(void);
	//void run(void);//release conn
	bool is_open(void) { return open; };
private:
	int make_conn(void);
	bool remove(int index);
	bool open;
};
