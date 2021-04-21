#include "StdAfx.h"
#include "Message.h"

Message::Message(void)
{
}

Message::~Message(void)
{
}

void Message::AddData(void* src, int rank, int length)
{
	int location = GetLocation(rank);
	memcpy(data + location, src, length);
}

void Message::GetData(void* des, int rank, int length)
{
	int location = GetLocation(rank);
	memcpy(des, data + location, length);
}

//返回数据在消息中存储的位置，由于第1，2个数据可能较大分的空间较多
int Message::GetLocation(int rank)
{
	switch (rank)
	{
	case 0:
		return 0;
	case 1:
		return 1000;
	case 2:
		return 2500;
	default:
		return 0;
	}
}