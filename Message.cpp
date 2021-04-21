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

//������������Ϣ�д洢��λ�ã����ڵ�1��2�����ݿ��ܽϴ�ֵĿռ�϶�
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