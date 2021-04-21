#include "StdAfx.h"
#include "TickDataList.h"

CCriticalSection mdcritical_section;

TickDataList::TickDataList(void)
{
}

TickDataList::~TickDataList(void)
{
}

void TickDataList::AddTail(TickInfo m)
{
	mdcritical_section.Lock();
	DataListCore.AddTail(m);
	mdcritical_section.Unlock();
}

TickInfo TickDataList::GetHead()
{
	mdcritical_section.Lock();
	if (DataListCore.IsEmpty())
	{
		mdcritical_section.Unlock();
		TickInfo m;
		memset(&m, 0, sizeof(TickInfo));
		return m;
	}
	else
	{
		TickInfo m = DataListCore.GetHead();
		RemoveHead();
		mdcritical_section.Unlock();
		return m;
	}
}

void TickDataList::RemoveHead()
{
	mdcritical_section.Lock();
	if (!DataListCore.IsEmpty())
	{
		DataListCore.RemoveHead();
	}
	mdcritical_section.Unlock();
}