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

class StrategyBar : public CStrategy
{
	struct mParasType
	{
		char InstCodeName[30];
		int n1;
		double ProfitPoint;
		double StoplossPoint;
		int OpenVolMulti;
		char StartTime[30];
		char EndTime[30];
		bool OpenBuyAllow;
		bool OpenSellAllow;
		bool CloseAllow;
		int LoopTimes;
	};

private:
	char  m_InstID[30];  //远期合约类型
	char  m_CommodityNo[30];
	char m_ExchangeID[30];
	TickInfo m_ThisData;

	double m_Price;//此时的价格
	double m_Buy1;  //远期买1价
	double m_Sell1; //远期卖1价

	bool timeRuleForOpen(char datadate[10], char datatime[10]);
	bool timeRuleForClose(char datadate[10], char datatime[10]);

	//double m_dOneTick;
	//int mOpenTimes;

	map<string, string> matchnomap;

	int initHstData();
public:
	StrategyBar(char InstrumentID[30]);
	~StrategyBar(void);
	bool m_bIsRunning; //初始化为ture

	void InitDataArray();
	void ReleaseDataArray();

	void InitVariables();

	void InitAction();
	void ResetAction();
	void exitAction();
	//void MessageProcess();

	MIndicator mIndicator;

	BarRateInfo* mBarFile;
	double* mBarClose;
	int mBarIndex;
	int mBarSize;
	int lastmBarIndex;

	int mTodayTickCount;

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

	int InstanceOnHandPositionCount;
	int shmindex;
	void SetCloseLocalOrder(LocalCLForDisplayField* pCLOrder);
private:
	list<MyCloseOrderType> CloseOrderList;
	list<MyOpenOrderType> OpenOrderList;
	bool m_bCrossTradingDay;

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

	void AddToRatesX(int tickCount, TickInfo* curTick, BarRateInfo* xBarFile, int* xBarIndex, int xBarTimeSize);
	void AddInToDesRatesX(BarRateInfo* gCurrtRatesX, int gSourceRatesXIndex, BarRateInfo* gDesRatesX, int* gDesRatesXIndex, int gDesRatesXSize);
	int initHstDataFromCSV();

	//FILE *fmddata;
	//FILE *findicator;

	bool timeRuleOK(char datatime[10]);
	char TickRealTimeDataDate[12];
	void CheckAndResubmitOutOfDateOrder(char cTickDataDate[11], char cTickUpdateTime[11]);
};