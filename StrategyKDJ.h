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
class StrategyKDJ : public CStrategy
{
	struct mParasType
	{
		char InstCodeName[30];
		int NPeriod;
		int KPeriod;
		int DPeriod;
		int JPeriod;
		double ProfitPoint;
		double StoplossPoint;
		double MAProfitPoint;
		double BackPerc;
		int OpenVol;
		char StartTime[30];
		char EndTime[30];
		bool OpenBuyAllow;
		bool OpenSellAllow;
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

	//double m_dOneTick;
	//int mOpenTimes;

	map<string, string> matchnomap;

public:
	StrategyKDJ(char InstrumentID[30]);
	~StrategyKDJ(void);
	bool m_bIsRunning; //初始化为ture

	void InitVariables();
	void ResetStrategy();

	void InitAction();
	void ResetAction();
	//void exitAction();
	//void MessageProcess();

	MIndicator mIndicator;

	BarRateInfo* mBarFile;
	int mBarIndex;
	int mBarSize;
	int lastmBarIndex;
	int mTodayTickCount;

	double* RSVBuffer;
	double* KBuffer;
	double* DBuffer;
	double* JBuffer;

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
	int shmindex;

	void SetCloseLocalOrder(LocalCLForDisplayField* pCLOrder);
private:
	list<MyCloseOrderType> CloseOrderList;
	list<MyOpenOrderType> OpenOrderList;
	bool m_bCrossTradingDay;
	bool m_bType1StoplossClose;

	//void UpdateOrderIdToOrderList(int OrderId);
	//void DeleteErrorOrderIdFromOrderList();
	void SetInstanceName(char xInstanceName[50]);
	void GetInstanceName(char* xInstanceName);
	char m_ExchangeID[30];
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
	//FILE *findicator;

	bool timeRuleOK(char datatime[10]);
	char TickRealTimeDataDate[12];
	void CheckAndResubmitOutOfDateOrder(char cTickDataDate[11], char cTickUpdateTime[11]);
};