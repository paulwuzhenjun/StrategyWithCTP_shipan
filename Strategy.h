#pragma once
#include "MyStruct.h"
#define ES_GRIDCOUNT 60	
class CStrategy
{
public:
	CStrategy(void);
	~CStrategy(void);

	//int shmindex;
	char InstCodeName[30];
	double m_dOneTick;
	int Multiplier;
	int mCloseOrderSeqNo;

	char mStrategyName[50];
	char mStrategyID[50];
	list<ParamNode> mParamslist;

	bool ThostTraderAPI;
	bool SgitTraderAPI;
	//	list<MyCloseOrderType> CloseOrderList;
	//	list<MyOpenOrderType> OpenOrderList;

	double mInitBasePrice;		// 基准价
	double mCurBasePrice;		// 砍仓操作后基准价会有变化

	virtual void InitVariables() {};
	virtual void OnRtnDepthMarketData(TickInfo* pDepthMarketData) {};

	virtual	void OnRtnOrder(OrderTradeMsg* pOrderTradeMsg) {};
	virtual void OnRtnTrade(OrderTradeMsg* pOrderTradeMsg) {};
	virtual void OnRspOrderInsert(ShmRspOrderInsert* pRspOrderInsert) {};
	//virtual void DeleteErrorOrderIdFromOrderList(){};
	virtual void SetParamValue(ParamNode node) {};
	virtual void SetInstanceName(char xInstanceName[50]) {};
	virtual void GetInstanceName(char* xInstanceName) {};
	virtual void OnDisplayLocalCloseOrderList() {};
	virtual void RecoverInstance(char cfgFileName[500]) {};
	virtual void SetCloseLocalOrder(LocalCLForDisplayField* pCLOrder) {};

	virtual void ResetAction() {};
	virtual void InitAction() {};
	virtual void exitAction() {};

	bool m_bIsRunning; //初始化为ture
	void MessageProcess();
	char beginrun_date[10];
	char yesterdaydate[10];
	bool DateChange;

	virtual int GetShmindex() { return 0; };
	virtual void SetShmindex(int xshmindex) {};

	virtual int CreateStrategyMapOfView() { return -1; };
	virtual int OpenStrategyMapOfView() { return -1; };

	virtual void CloseMap() {};

	void ConvertCStringToCharArray(CString csSource, char* rtnCharArray);
	virtual void SetStrategyID(char strategyId[50]) {};

	virtual void  WriteBasePrice(char* date, char* time) {};

	FILE* fpinfo;
};
