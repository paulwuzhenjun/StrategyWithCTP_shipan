#pragma once
#include <afx.h>
#include "MyStruct.h"

class LogMsgThread
{
public:
	LogMsgThread(void);
	~LogMsgThread(void);

	bool m_bIsRunning;
	void ProcessLogMsg();

private:
	CStdioFile outFile;
	void WriteIntoFile(char* logline);

	//CStdioFile tradeFile;
	void WriteTradeRecIntoFile(TradeLogType* pTrade);
	char m_path[256];
};
