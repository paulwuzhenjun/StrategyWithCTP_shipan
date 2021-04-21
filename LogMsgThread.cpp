#include "StdAfx.h"
#include "LogMsgThread.h"
#include "MessageList.h"
#include "MyStruct.h"
#include <io.h>

extern MessageList LogMessageList;
extern HANDLE logSemaphore;
extern char CTPTradingDay[];

LogMsgThread::LogMsgThread(void)
{
	m_bIsRunning = true;

	CString strPath;
	::GetModuleFileName(NULL, strPath.GetBuffer(MAX_PATH), MAX_PATH);
	strPath.ReleaseBuffer();
	strPath = strPath.Left(strPath.ReverseFind(_T('\\')));
	strPath += "\\XXQIJ";
	CTime todayday = CTime::GetCurrentTime();
	strPath.Append(todayday.Format("%Y%m%d"));
	strPath += "Test.log";
	outFile.Open(strPath, CFile::modeNoTruncate | CFile::modeCreate | CFile::modeWrite | CFile::typeText | CFile::shareDenyWrite);

	CString strLogInfo(_T("Log File Opened.."));
	CString strData;
	CTime tt = CTime::GetCurrentTime();
	strData = tt.Format("[%Y-%B-%d %A, %H:%M:%S] ") + strLogInfo;
	strData += "\n";

	outFile.SeekToEnd();
	outFile.WriteString(strData);
	outFile.Flush();

	CString strPathTradeFile;
	::GetModuleFileName(NULL, strPathTradeFile.GetBuffer(MAX_PATH), MAX_PATH);
	strPathTradeFile.ReleaseBuffer();
	strPathTradeFile = strPathTradeFile.Left(strPathTradeFile.ReverseFind(_T('\\')));
	strPathTradeFile += "\\Trades";

	int len = WideCharToMultiByte(CP_ACP, 0, strPathTradeFile, strPathTradeFile.GetLength(), NULL, 0, NULL, NULL);
	char* c_str_filename = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, strPathTradeFile, strPathTradeFile.GetLength(), c_str_filename, len, NULL, NULL);
	c_str_filename[len] = '\0';
	if (_access(c_str_filename, 0) != 0) {
		CreateDirectory(strPathTradeFile, NULL);
	}
	strcpy(m_path, c_str_filename);
	free(c_str_filename);

	//strPathTradeFile += "\\Trades";
	//strPathTradeFile.Append(todayday.Format("%Y%m%d"));
	//strPathTradeFile += ".log";
	//tradeFile.Open(strPathTradeFile,CFile::modeNoTruncate | CFile::modeCreate | CFile::modeWrite | CFile::typeText | CFile::shareDenyWrite);
}

LogMsgThread::~LogMsgThread(void)
{
	outFile.Close();
	//tradeFile.Close();
}

void LogMsgThread::ProcessLogMsg()
{
	char logLine[200];
	TradeLogType pTrade;
	while (m_bIsRunning) {
		WaitForSingleObject(logSemaphore, INFINITE);

		if (!LogMessageList.MessageListCore.IsEmpty())
		{
			Message message = LogMessageList.GetHead();
			switch (message.type)
			{
			case STRATEGY_LOG:
				memset(logLine, 0, sizeof(logLine));
				message.GetData(logLine, 0, sizeof(char) * 200);
				WriteIntoFile(logLine);
				break;
			case TRADE_LOG:
				memset(&pTrade, 0, sizeof(TradeLogType));
				message.GetData(&pTrade, 0, sizeof(TradeLogType));
				WriteTradeRecIntoFile(&pTrade);
				break;
			case MD_LOG:
				memset(logLine, 0, sizeof(logLine));
				message.GetData(logLine, 0, sizeof(char) * 200);
				WriteIntoFile(logLine);
				break;
			}
		}
	}
}

void LogMsgThread::WriteIntoFile(char* logline)
{
	CString strLogInfo(logline);
	CString strData;
	CTime tt = CTime::GetCurrentTime();
	strData = tt.Format("[%Y-%B-%d %A, %H:%M:%S] ") + strLogInfo;
	strData += "\n";

	TCHAR* old_locale = _tcsdup(_tsetlocale(LC_CTYPE, NULL));
	_tsetlocale(LC_CTYPE, _T("chs"));
	outFile.SeekToEnd();
	outFile.WriteString(strData);
	outFile.Flush();
	_tsetlocale(LC_CTYPE, old_locale);
	free(old_locale);
}

void LogMsgThread::WriteTradeRecIntoFile(TradeLogType* pTrade)
{
	//CTime todayday = CTime::GetCurrentTime();
	//CString cDay = todayday.Format("%Y%m%d");
	//CStdioFile tradeFile;
	//CreateTradeFile(tradeFile,cDay);
	//char recline[300];
	//sprintf(recline,"%s,%s,%s,%s,%s,%d,%d,%d,%d,%.5f,%.5f,%.5f\n",pTrade->StrategyID,pTrade->InstanceName,pTrade->CodeName,pTrade->tradingday,pTrade->tradingtime,pTrade->openorclose,pTrade->openid,pTrade->closeid,pTrade->qty,pTrade->submitprice,pTrade->tradeprice,pTrade->fee);
	//CString strLogInfo(recline);
	//tradeFile.SeekToEnd();
	//tradeFile.WriteString(strLogInfo);
	//tradeFile.Flush();
	//tradeFile.Close();

	// 0点到6点的交易写在上一个文件,本地时间比交易时间慢的话会到上一个交易日
	int nHour, nMin, nSec;
	CTime todayday;
	sscanf_s(pTrade->tradingtime, "%d:%d:%d", &nHour, &nMin, &nSec);
	if (nHour >= 0 && nHour < 6)
		todayday = CTime::GetCurrentTime() + CTimeSpan(0, 1, 0, 0) - CTimeSpan(1, 0, 0, 0);
	else
		todayday = CTime::GetCurrentTime();

	ofstream iFile;
	char fPath[256] = { 0 };
	char cDate[32] = { 0 };
	sprintf(cDate, "%04d%02d%02d", todayday.GetYear(), todayday.GetMonth(), todayday.GetDay());
	strcpy(fPath, m_path);
	strcat(fPath, "\\Trades");
	strcat(fPath, cDate);
	strcat(fPath, ".log");
	iFile.open(fPath, ios::app | ios::out);
	if (iFile.is_open())
	{
		char recline[300];
		sprintf(recline, "%s,%s,%s,%s,%s,%d,%d,%d,%d,%.5f,%.5f,%.5f,%s,%s\n", pTrade->StrategyID, pTrade->InstanceName, pTrade->CodeName, pTrade->tradingday, pTrade->tradingtime, pTrade->openorclose, pTrade->openid, pTrade->closeid, pTrade->qty, pTrade->submitprice, pTrade->tradeprice, pTrade->fee, pTrade->MatchNo, CTPTradingDay);
		iFile << recline << endl;
		iFile.flush();
		iFile.close();
	}
}