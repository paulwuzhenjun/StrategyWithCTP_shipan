#pragma once
#include "MyStruct.h"
#include "MIndicator.h"
#include "TickDataList.h"
#include <stdio.h>
#include <string.h>
#include <list>
#include "Strategy.h"
#include <map>

using namespace std;

// 开盘价上下挂单,两单止损当天禁止开仓;2点30分以后禁止开仓,第二天12点砍仓,以当天开盘价重新挂单
class StrategyOpenPriceOpening : public CStrategy
{
	struct mParasType
	{
		char InstCodeName[30];
		double OpenDistPoint;
		double SecondOpenDistPoint;
		double FirstProfitPoint;
		double FirstStoplossPoint;
		double SecondProfitPoint;
		double SecondStoplossPoint;
		int OpenVol1;
		int OpenVol2;
		char StartTime[30];
		char EndTime[30];
		bool OpenBuyAllow;
		bool OpenSellAllow;
		double OpenPrice;		// 外部可设定开盘价
		char OpenTime[30];		// 开盘时间
		char CloseTime[30];		// 收盘时间
		char ChopTime[30];		// 砍仓时间
		int LoopTimes;
	};

private:
	char  m_InstID[30];  //远期合约类型
	char  m_CommodityNo[30];

	double m_Price;//此时的价格
	double m_Buy1;  //远期买1价
	double m_Sell1; //远期卖1价

	bool timeRuleForOpen(char datadate[10], char datatime[10]);
	bool timeRuleForClose(char datadate[10], char datatime[10]);
	bool timeRuleForCancelOpen(char datadate[10], char datatime[10]);//2:30~05:00撤掉开仓单
	bool timeRuleForStop(int nHour, int nMin, int nSec);			// 禁止开仓点
	bool timeRuleForCloseNew(char openTime[30], char datadate[10], char datatime[10]);	// 砍仓点

	//double m_dOneTick;
	int mOpenTimes;
	bool Type1OrderExist;
	bool Type2OrderExist;
	bool Type2StoplossClose;

	map<string, string> matchnomap;

public:
	StrategyOpenPriceOpening(char InstrumentID[30]);
	~StrategyOpenPriceOpening(void);
	bool m_bIsRunning; //初始化为ture

	void InitVariables();
	void ResetStrategy();
	//void MessageProcess();

	void InitAction();
	//void ResetAction(){}
	//void exitAction(){}

	int mTickCount;
	double mOpenPrice;
	bool mOpenRet;
	time_t curTickTime;

	time_t tmt_StartTime;
	time_t tmt_EndTime;

	mParasType mStrategyParams;
	void SetParamValue(ParamNode node);

	char mInstanceName[50];
	string mStrategyAndInstance;
	static int MaxOnHandPositionCount;
	static int OnHandPositionCount;

	bool MarketOpenExecuted;
	char MarketOpenTime[11];//HH24:MI:SS
	void MarketOpenOperation();
	bool MarketCloseExecuted;
	char MarketCloseTime[11];//HH24:MI:SS
	void MarketCloseOperation();
private:
	list<MyCloseOrderType> CloseOrderList;
	list<MyOpenOrderType> OpenOrderList;
	int shmindex;
	bool m_bCrossTradingDay;
	bool m_bType1StoplossClose;

	//void UpdateOrderIdToOrderList(int OrderId);
	//void DeleteErrorOrderIdFromOrderList();
	void SetInstanceName(char xInstanceName[50]);
	void GetInstanceName(char* xInstanceName);
	char m_ExchangeID[30];
	int GetShmindex();
	void SetShmindex(int xshmindex);
	void SetStrategyID(char strategyId[50]);			// 新增加 CL-GD-00006

	void ReqOrderDelete(int pOrderId, int pOrderLocalRef, int pFrontID, int pSessionID);
	void ReqOpenOrderInsert(MyOpenOrderType* pOpenOrder);
	void ReqCloseOrderInsert(MyCloseOrderType* pCloseOrder, char OpenOrderTime[21]);

	void OnRtnDepthMarketData(TickInfo* pDepthMarketData);
	void OnRtnOrder(OrderTradeMsg* pOrderTradeMsg);
	void OnRtnTrade(OrderTradeMsg* pOrderTradeMsg);
	void OnRspOrderInsert(ShmRspOrderInsert* pRspOrderInsert);
	void OnDisplayLocalCloseOrderList();

	void WriteMsgToLogList(char logline[200]);
	void AddtoTipMsgListBox(char msgline[200]);
	void DisplayTradeOnScreen(OrderTradeMsg* pOrderTradeMsg, int mDirection, int mOpenOrClose, int mCloseProfitOrNot);

	void RecoverInstance(char cfgFileName[500]);
	void CloseMap();

	mSerializeHeader header;
	wstring s2ws(const string& s);
	int CreateStrategyMapOfView();
	int OpenStrategyMapOfView();
	void FlushStrategyInfoToFile();
	HANDLE StrategyInfoFileMap;
	char* StrategyfilePoint;

	char TickRealTimeDataDate[12];
	void CheckAndResubmitOutOfDateOrder(char cTickDataDate[11], char cTickUpdateTime[11]);
};