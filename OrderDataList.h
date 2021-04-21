#pragma once
#include "MyStruct.h"

class OrderDataList
{
public:
	CList<OrderTradeMsg, OrderTradeMsg&> DataListCore;
public:
	OrderDataList(void);
	~OrderDataList(void);
	void AddTail(OrderTradeMsg m);
	OrderTradeMsg GetHead();
private:
	void RemoveHead();
};
