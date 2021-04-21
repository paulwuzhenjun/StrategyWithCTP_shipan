#include "StdAfx.h"
#include "TradeDataList.h"

CCriticalSection tdcritical_section;

TradeDataList::TradeDataList(void)
{
}

TradeDataList::~TradeDataList(void)
{
}

void TradeDataList::AddTail(SendLogType m)
{
	tdcritical_section.Lock();
	DataListCore.AddTail(m);
	tdcritical_section.Unlock();
}

SendLogType TradeDataList::GetHead()
{
	tdcritical_section.Lock();
	if (DataListCore.IsEmpty())
	{
		tdcritical_section.Unlock();
		SendLogType m;
		memset(&m, 0, sizeof(SendLogType));
		return m;
	}
	else
	{
		SendLogType m = DataListCore.GetHead();
		RemoveHead();
		tdcritical_section.Unlock();
		return m;
	}
}

void TradeDataList::RemoveHead()
{
	tdcritical_section.Lock();
	if (!DataListCore.IsEmpty())
	{
		DataListCore.RemoveHead();
	}
	tdcritical_section.Unlock();
}

int TradeDataList::GetCount() {
	return DataListCore.GetCount();
}