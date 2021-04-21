#include "StdAfx.h"
#include "OrderDataList.h"

CCriticalSection ordercritical_section;

OrderDataList::OrderDataList(void)
{
}

OrderDataList::~OrderDataList(void)
{
}

void OrderDataList::AddTail(OrderTradeMsg m)
{
	ordercritical_section.Lock();
	DataListCore.AddTail(m);
	ordercritical_section.Unlock();
}

OrderTradeMsg OrderDataList::GetHead()
{
	ordercritical_section.Lock();
	if (DataListCore.IsEmpty())
	{
		ordercritical_section.Unlock();
		OrderTradeMsg m;
		memset(&m, 0, sizeof(OrderTradeMsg));
		return m;
	}
	else
	{
		OrderTradeMsg m = DataListCore.GetHead();
		RemoveHead();
		ordercritical_section.Unlock();
		return m;
	}
}

void OrderDataList::RemoveHead()
{
	ordercritical_section.Lock();
	if (!DataListCore.IsEmpty())
	{
		DataListCore.RemoveHead();
	}
	ordercritical_section.Unlock();
}