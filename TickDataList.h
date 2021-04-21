#pragma once
#include "MyStruct.h"

class TickDataList
{
public:
	CList<TickInfo, TickInfo&> DataListCore;
public:
	TickDataList(void);
	~TickDataList(void);
	void AddTail(TickInfo m);
	TickInfo GetHead();
private:
	void RemoveHead();
};
