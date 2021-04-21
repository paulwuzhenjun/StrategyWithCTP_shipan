#include "StdAfx.h"
#include "MessageList.h"
#include "Message.h"

CCriticalSection critical_section;

MessageList::MessageList(void)
{
}

MessageList::~MessageList(void)
{
}

void MessageList::AddTail(Message m)
{
	critical_section.Lock();
	MessageListCore.AddTail(m);
	critical_section.Unlock();
}

Message MessageList::GetHead()
{
	critical_section.Lock();
	if (MessageListCore.IsEmpty())
	{
		critical_section.Unlock();
		TRACE("message is empty in newclist gethead()\n");
		Message m;
		memset(&m, 0, sizeof(Message));
		return m;
	}
	else
	{
		Message m = MessageListCore.GetHead();
		RemoveHead();
		critical_section.Unlock();
		return m;
	}
}

void MessageList::RemoveHead()
{
	if (!MessageListCore.IsEmpty())
	{
		MessageListCore.RemoveHead();
	}
}