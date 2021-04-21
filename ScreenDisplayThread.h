#pragma once
#include "MyStruct.h"

class ScreenDisplayThread
{
public:
	ScreenDisplayThread(void);
	~ScreenDisplayThread(void);

	bool m_bIsRunning;
	void ProcessRspMsg();

	void OnRspQryLocalCLDetail(LocalCLForDisplayField* pCLDetail);
	void OnRspQryPositionDetail(PositionDetailField* pPosi);
	void OnRecoverRspQryOrder(OrderDetailField* pOrder);
	void OnRspQryOrder(OrderDetailField* pOrder);
};
