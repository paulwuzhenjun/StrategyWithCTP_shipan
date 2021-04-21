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

//3K线策略 参数说明,当StoplossPoint设置大于1000表示止损不启作用
class StrategyThreeK : public CStrategy
{
	struct mParasType
	{
		char InstCodeName[30];
		char StartTime[30];
		char EndTime[30];
		bool OpenBuyAllow;
		bool OpenSellAllow;
		//double ProfitPoint;		// 止盈点位
		int OppositeClose;      //反向信号平仓
		double StoplossPerc;	// 固定止损比例
		double TrackStopPerc;	// 追踪止赢百分比
		double BackPerc;		// 回撤百分比
		int BarSize;			// K线周期 300=5min
		int OpenVol;
		char OpenTime[30];		// 开盘时间
		char CloseTime[30];		// 收盘时间
		int LoopTimes;
	};

private:
	char  m_InstID[30];  //远期合约类型
	char  m_CommodityNo[30];
	char m_InstCodeName[30];
	char m_ExchangeID[30];

	double m_Price;//此时的价格
	double m_Buy1;  //远期买1价
	double m_Sell1; //远期卖1价

	bool timeRuleForOpen(char datatime[10]);
	bool timeRuleForClose(char datadate[10], char datatime[10]);
	bool timeRuleForCancel(char datadate[10], char datatime[10]);

	//double m_dOneTick;
	int mOpenTimes;

	map<string, string> matchnomap;

public:
	StrategyThreeK(char InstrumentID[30]);
	~StrategyThreeK(void);
	bool m_bIsRunning; //初始化为ture

	void InitVariables();
	void ResetStrategy();
	//void MessageProcess();

	void InitAction();
	void ResetAction();
	void exitAction();

	int mTickCount;
	int mBarIndex;
	int lastmBarIndex;
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
	BarRateInfo* mBarFile;
	int InstanceOnHandPositionCount;

	void InitDataArray();
	void ReleaseDataArray();
	void AddToRatesX(int tickCount, TickInfo* curTick, int gRatesXSize);
	//void UpdateOrderIdToOrderList(int OrderId);
	//void DeleteErrorOrderIdFromOrderList();
	void SetInstanceName(char xInstanceName[50]);
	void GetInstanceName(char* xInstanceName);
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

	int initHstDataFromCSV();
	void AddInToDesRatesX(BarRateInfo* gCurrtRatesX, int gSourceRatesXIndex, BarRateInfo* gDesRatesX, int* gDesRatesXIndex, int gDesRatesXSize);

	char TickRealTimeDataDate[12];
	//void CheckAndResubmitOutOfDateOrder(char cTickDataDate[11],char cTickUpdateTime[11]);
	int getKSingnal();
};