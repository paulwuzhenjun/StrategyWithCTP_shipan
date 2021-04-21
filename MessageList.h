#pragma once

#include "Message.h"

class MessageList
{
public:
	CList<Message, Message&> MessageListCore;
public:
	MessageList(void);
	~MessageList(void);
	void AddTail(Message m);
	Message GetHead();
private:
	void RemoveHead();
};
