#include "StdAfx.h"
#include "ado_conn.h"
#include "MyStruct.h"

bool ado_conn::initState = false;
int connectNum = 1;

ado_conn::ado_conn(void)
{
	connState = false;
	rsState = false;
	cmdState = false;
	pool_mode = false;
	used = false;

	//sql_log = new msg_log("","sql_err.log");

	if (!initState)
	{
		::CoInitialize(NULL);
		initState = true;
	}
}

ado_conn::ado_conn(string username, string password, string conn_str)
{
	connState = false;
	rsState = false;
	cmdState = false;
	pool_mode = false;
	used = false;

	//sql_log = new msg_log("","sql_err.log");

	if (!initState)
	{
		::CoInitializeEx(NULL, COINIT_MULTITHREADED);
		initState = true;
	}
	conn(username, password, conn_str);
}

ado_conn::~ado_conn(void)
{
	release();

	::CoUninitialize();
}
bool    ado_conn::close(void)
{
	end_execute();

	this->is_used(false);

	return true;
}
bool    ado_conn::release(void)
{
	end_execute();

	end_conn();

	//if(sql_log != NULL) delete sql_log;

	this->is_used(false);

	return true;
}

bool    ado_conn::conn(string username, string password, string conn_str)
{
	if (!end_execute()) return false;

	this->username = username;
	this->password = password;
	this->conn_str = conn_str;

	/*
	 string str = "Provider=MSDAORA.1;Password="
			   + password
			   + ";User ID="
			   + username
			   + ";Data Source="
			   + conn_str
			   + ";Persist Security Info=True";
*/
	string str = "Provider=OraOLEDB.Oracle;Data Source=" + conn_str + ";User Id=" + username + ";Password=" + password;
	try //建立数据库连接
	{
		m_pConnection.CreateInstance(__uuidof(Connection));
		//m_pConnection.CreateInstance("ADODB.Connection"+connectNum);
		connectNum++;

		m_pConnection->Open(str.c_str(), "", "", adModeUnknown);

		//AfxMessageBox("连接数据库成功！");
		connState = true;
		//临时
		//sql_log->log2file_str("连接数据库成功！");
	}
	catch (_com_error e)
	{
		CString str("数据库连接失败！【错误信息】：");
		str += e.ErrorMessage();
		connState = false;
		//AfxMessageBox(LPCTSTR(str.c_str()));
		//sql_log->log2file_str(str.c_str());
	}
	catch (...)
	{
		//报警
		CString str("数据库连接失败！");
		//AfxMessageBox(LPCTSTR(str.c_str()));
		connState = false;
		//sql_log->log2file_str(str.c_str());
	}

	return connState;
}
bool    ado_conn::conn(void)
{
	return false;//不提供
}
bool    ado_conn::db_query(const char* commandTxt)
{
	string sql_str = commandTxt;

	if (sql_str.length() == 0) return false;

	if (!connState) return false;

	if (!end_execute()) return false;

	try
	{
		m_pCommand.CreateInstance(__uuidof(Command));

		cmdState = true;

		m_pRecordset.CreateInstance(__uuidof(Recordset));

		rsState = true;

		m_pCommand->ActiveConnection = m_pConnection;

		m_pCommand->CommandText = sql_str.c_str();

		m_pRecordset = m_pCommand->Execute(NULL, NULL, adCmdText); // 执行SQL语句，返回记录集
	}
	catch (_com_error e)
	{
		//意外处理
		end_execute();

		CString str("数据库查询失败！【错误信息】：");

		str += e.ErrorMessage();
		//sql_log->log2file_str(str.c_str());

		return false;
	}
	catch (...)
	{
		//意外处理
		end_execute();
		string str = "数据库查询失败！【错误信息】：";
		str += sql_str;
		//sql_log->log2file_str(str.c_str());
		return false;
	}

	return true;
}
bool    ado_conn::db_update(const char* commandTxt)
{
	string sql_str = commandTxt;

	if (sql_str.length() == 0) return false;

	if (!connState) return false;

	if (!end_execute()) return false;

	try
	{
		m_pConnection->Execute(sql_str.c_str(), NULL, adCmdText);
	}
	catch (_com_error e)
	{
		//意外处理
		end_execute();

		CString str("数据库更新失败！【错误信息】：");

		str += e.ErrorMessage();
		//sql_log->log2file_str(str.c_str());

		return false;
	}
	catch (...)
	{
		//意外处理
		end_execute();

		string str = "数据库更新失败！【错误信息】：";
		str += sql_str;
		//sql_log->log2file_str(str.c_str());

		return false;
	}
	return true;
}

bool    ado_conn::next(void)
{
	if (!connState) return false;

	if (!rsState) return false;

	m_pRecordset->MoveNext();

	return true;
}
bool    ado_conn::is_empty(void)
{
	if (!connState) return false;

	if (!rsState) return false;

	if (m_pRecordset->adoEOF)	return true;

	return false;
}
bool    ado_conn::end_execute(void)
{
	end_rs();
	end_cmd();

	return !(rsState || cmdState);
}

bool    ado_conn::end_rs(void)
{
	if (m_pRecordset != NULL && (m_pRecordset->GetState()) != adStateClosed)
		//if(rsState)
	{
		rsState = false;
		m_pRecordset->Close();
		m_pRecordset.Release();
		//m_pRecordset = NULL;
	}
	return true;
}

bool    ado_conn::end_conn(void)
{
	if (connState)
	{
		connState = false;
		m_pConnection->Close();
		m_pConnection.Release();
		//m_pConnection = NULL;
	}
	return true;
}

bool    ado_conn::end_cmd(void)
{
	if (m_pCommand != NULL && (m_pCommand->GetState()) != adStateClosed)
		//if(cmdState)
	{
		cmdState = false;
		m_pCommand.Release();
	}
	return true;
}

bool    ado_conn::get_double(int Index, double* value)
{
	_variant_t	tmp_Val;

	if (!rsState) return false;

	try
	{
		tmp_Val = m_pRecordset->GetCollect(_variant_t((long)Index));

		if (tmp_Val.vt == VT_NULL)
		{
			*value = 0;
			return false;
		}
		else
		{
			*value = (double)tmp_Val;
			return true;
		}
	}
	catch (_com_error e)
	{
		*value = 0;
		return false;
	}
	catch (...)
	{
		//意外处理
		*value = 0;
		return false;
	}
}

bool    ado_conn::get_string(int Index, string* value)
{
	_variant_t	tmp_Val;

	if (!rsState) return false;

	try
	{
		tmp_Val = m_pRecordset->GetCollect(_variant_t((long)Index));

		if (tmp_Val.vt == VT_NULL)
		{
			*value = "";
			return false;
		}
		else
		{
			*value = (char*)(_bstr_t)tmp_Val;
			return true;
		}
	}
	catch (_com_error e)
	{
		*value = "";
		return false;
	}
	catch (...)
	{
		//意外处理
		*value = "";
		return false;
	}
}
bool    ado_conn::get_int(int Index, int* value)
{
	_variant_t	tmp_Val;

	if (!rsState) return false;

	try
	{
		tmp_Val = m_pRecordset->GetCollect(_variant_t((long)Index));

		if (tmp_Val.vt == VT_NULL)
		{
			*value = 0;
			return false;
		}
		else
		{
			*value = (int)tmp_Val;
			return true;
		}
	}
	catch (_com_error e)
	{
		*value = 0;
		return false;
	}
	catch (...)
	{
		//意外处理
		*value = 0;
		return false;
	}
}

double  ado_conn::get_double(const char* FieldName)
{
	_variant_t	tmp_Val;

	if (!rsState) return -1;

	try
	{
		tmp_Val = m_pRecordset->GetCollect(FieldName);
		if (tmp_Val.vt == VT_NULL) return 0;
	}
	catch (_com_error e)
	{
		return -1;
	}
	catch (...)
	{
		//意外处理
		return -1;
	}
	return (double)tmp_Val;
}
int     ado_conn::get_int(const char* FieldName)
{
	_variant_t	tmp_Val;

	if (!rsState) return -1;

	try
	{
		tmp_Val = m_pRecordset->GetCollect(FieldName);
		if (tmp_Val.vt == VT_NULL) return 0;
	}
	catch (_com_error e)
	{
		return -1;
	}
	catch (...)
	{
		//意外处理
		return -1;
	}

	return (int)(double)tmp_Val;
}
string  ado_conn::get_string(const char* FieldName)
{
	_variant_t	tmp_Val;

	if (!rsState) return "";

	string value;

	try
	{
		tmp_Val = m_pRecordset->GetCollect(FieldName);

		if (tmp_Val.vt == VT_NULL)
		{
			return "";
		}
		else
		{
			value = (char*)(_bstr_t)tmp_Val;
			return value;
		}
	}
	catch (_com_error e)
	{
		return "";
	}
	catch (...)
	{
		//意外处理
		return "";
	}
}