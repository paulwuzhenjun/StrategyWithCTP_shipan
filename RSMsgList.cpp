#include "StdAfx.h"
#include "RSMsgList.h"
#include "afxmt.h"

CCriticalSection rscritical_section;

RSMsgList::RSMsgList(void)
{
}

RSMsgList::~RSMsgList(void)
{
}

void RSMsgList::AddTail(rs_msg m)
{
	rscritical_section.Lock();
	DataListCore.AddTail(m);
	rscritical_section.Unlock();
}

rs_msg RSMsgList::GetHead()
{
	rscritical_section.Lock();
	if (DataListCore.IsEmpty())
	{
		rscritical_section.Unlock();
		rs_msg m;
		memset(&m, 0, sizeof(rs_msg));
		return m;
	}
	else
	{
		rs_msg m = DataListCore.GetHead();
		RemoveHead();
		rscritical_section.Unlock();
		return m;
	}
}

void RSMsgList::RemoveHead()
{
	//rscritical_section.Lock();
	if (!DataListCore.IsEmpty())
	{
		DataListCore.RemoveHead();
	}
	//rscritical_section.Unlock();
}