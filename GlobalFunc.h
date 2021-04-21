#pragma once
#include "MyStruct.h"
class GlobalFunc
{
public:
	GlobalFunc(void);
	~GlobalFunc(void);

	//根据行情程序,初始化订阅和约列表
	void InitInstrumentSubList();
	void WriteMsgToLogList(char logline[200]);
	void ConvertCStringToCharArray(CString csSource, char* rtnCharArray);
	int Split(CString source, CString ch, CStringArray& strarr);
	char* getModuleFilePath(char* path);
	void getUDPSendLog(TradeLogType* pTrade, SendLogType* pSend);
};
