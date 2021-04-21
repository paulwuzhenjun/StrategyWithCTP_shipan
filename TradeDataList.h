#pragma once
#include "MyStruct.h"

class TradeDataList
{
public:
	CList<SendLogType, SendLogType&> DataListCore;
public:
	TradeDataList(void);
	~TradeDataList(void);
	void AddTail(SendLogType m);
	SendLogType GetHead();
	int GetCount();
private:
	void RemoveHead();
};
