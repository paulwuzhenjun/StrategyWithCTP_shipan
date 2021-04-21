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

//递归指标策略
class StrategySpeedBreak : public CStrategy
{
	struct mParasType
	{
		char InstCodeName[30];
		int BreakHour;//异动开始时间：小时
		int BreakMinute;//异动开始时间：分钟
		int RangeSecond;//持续时间：单位秒
		int RangeTick;//波动距离：单位tick
		double BackPercent;//回调比例：单位0.01
		double ProfitPoint;//止盈点数：单位tick
		double LossPoint;//止损点数：单位tick
		int ProfitCount;//最大止盈次数
		int LossCount;//最大止损次数
		bool OpenBuyAllow;//允许开多单
		bool OpenSellAllow;//允许开空单
		bool HoldAllow;//允许隔夜持仓
		int OpenVol;//单次开仓手数
		//int BandsPeriod;
		//int BandsDeviations;
		//double HLDist;

		char StartTime[30];
		char EndTime[30];
		char StartTimeNight[30];
		char EndTimeNight[30];

		//int LoopTimes;
	};

private:
	char  m_InstID[30];  //远期合约类型
	char  m_CommodityNo[30];
	char m_ExchangeID[30];

	double m_Price;//此时的价格
	double m_Buy1;  //远期买1价
	double m_Sell1; //远期卖1价

	bool timeRuleForOpen(char datadate[10], char datatime[10]);
	bool timeRuleForClose(char datadate[10], char datatime[10]);
	bool timeRuleOK(char datatime[10]);

	//double m_dOneTick;
	//int mOpenTimes;

	map<string, string> matchnomap;

public:
	StrategySpeedBreak(char InstrumentID[30]);
	~StrategySpeedBreak(void);
	bool m_bIsRunning; //初始化为ture

	void InitVariables();
	void ResetStrategy();
	void ResetAction();
	//void MessageProcess();

	MIndicator mIndicator;

	BarRateInfo* mBarFile;
	int mBarIndex;
	int mBarSize;
	int lastmBarIndex;
	int mTodayTickCount;
	double* mCloseBuffer;

	double* MovingBuffer;
	double* UpperBuffer;
	double* LowerBuffer;

	int mTickCount;
	double mOpenPrice;
	//time_t curTickTime;
	int openBarIndex;

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
	int InstanceOnHandPositionCount;
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
	int GetShmindex();
	void SetShmindex(int xshmindex);
	void SetStrategyID(char strategyId[50]);

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

	void AddToRatesX(int tickCount, TickInfo* curTick, int gRatesXSize);
	void AddInToDesRatesX(BarRateInfo* gCurrtRatesX, int gSourceRatesXIndex, BarRateInfo* gDesRatesX, int* gDesRatesXIndex, int gDesRatesXSize);
	int initHstDataFromCSV();

	FILE* fmddata;
	FILE* findicator;

	char TickRealTimeDataDate[12];
	void CheckAndResubmitOutOfDateOrder(char cTickDataDate[11], char cTickUpdateTime[11]);
};