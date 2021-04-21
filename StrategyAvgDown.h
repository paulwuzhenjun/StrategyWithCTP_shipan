#pragma once
#include "strategy.h"
#include <map>
using namespace std;
// 均线回调策略，昨日阳线，今日开多，昨日阴线，今日开空,不隔交易日
class StrategyAvgDown : public CStrategy
{
	struct mParasType
	{
		char InstCodeName[30];
		double ProfitPoint;
		double StoplossPoint;
		char StartTime[30];
		char EndTime[30];
		bool OpenBuyAllow;
		bool OpenSellAllow;
		//bool CloseAllow;
		// 开盘时间
		char OpenTime[30];
		// 收盘时间
		char CloseTime[30];
		// 开盘价格
		double OpenPrice;
		int LoopTimes;
		// 大于合约昨日收盘价格的百分比
		double m_fTP;
		double m_fTS;
	};
public:
	StrategyAvgDown(char InstrumentID[30]);
	~StrategyAvgDown(void);

	void ReqOrderDelete(int pOrderId, int pOrderLocalRef, int pFrontID, int pSessionID);
	void ReqOpenOrderInsert(MyOpenOrderType* pOpenOrder);
	void ReqCloseOrderInsert(MyCloseOrderType* pCloseOrder, char OpenOrderTime[21]);

	void OnRtnDepthMarketData(TickInfo* pDepthMarketData);
	void OnRtnOrder(OrderTradeMsg* pOrderTradeMsg);
	void OnRtnTrade(OrderTradeMsg* pOrderTradeMsg);
	void OnRspOrderInsert(ShmRspOrderInsert* pRspOrderInsert);
	void WriteMsgToLogList(char logline[200]);
	void SetParamValue(ParamNode node);

	void InitAction();
	void ResetAction();
	void exitAction();

	void SetInstanceName(char xInstanceName[50]);
	void GetInstanceName(char* xInstanceName);

	void SetStrategyID(char strategyId[50]);
private:
	// 添加5minBar
	void AddToRatesX(int tickCount, TickInfo* curTick, BarRateInfo* xBarFile, int* xBarIndex, int xBarTimeSize);
	void InitVariables();
	void AddtoTipMsgListBox(char msgline[200]);
	void DisplayTradeOnScreen(OrderTradeMsg* pOrderTradeMsg, int mDirection, int mOpenOrClose, int mCloseProfitOrNot);
	bool timeRuleOK(char datatime[10]);
	void FlushStrategyInfoToFile();
	int GetShmindex() { return shmindex; }
	void SetShmindex(int xshmindex) { shmindex = xshmindex; }
	void ReleaseDataArray();
	void InitDataArray();
	int CreateStrategyMapOfView();
	int OpenStrategyMapOfView();
	wstring s2ws(const string& s);
	void CloseMap();
	// 读取昨日价格
	void LoadHisData();
	// A50读取开盘价格和收盘价格
	void LoadCNData();

	bool timeRuleForOpen(char datadate[10], char datatime[10]);
	bool timeRuleForClose(char datadate[10], char datatime[10]);
public:
	char mInstanceName[50];
	// 最大持仓
	static int MaxOnHandPositionCount;
	// 策略在手持仓
	static int OnHandPositionCount;

private:
	bool m_bIsRunning;
	TickInfo m_ThisData;
	int mBarIndex;
	double m_Price;//最新价格
	double m_Buy1;  //买1价
	double m_Sell1; //卖1价
	int mTickCount;
	BarRateInfo* mBarFile;
	BarRateInfo* mOldBarFile;
	// 周期
	int mBarSize;
	int lastmBarIndex;
	mParasType mStrategyParams;
	char  m_InstID[30];  //远期合约类型
	char  m_CommodityNo[30];
	char m_ExchangeID[30];
	int shmindex;
	list<MyCloseOrderType> CloseOrderList;
	list<MyOpenOrderType> OpenOrderList;
	bool m_bCrossTradingDay;
	string mStrategyAndInstance;
	int InstanceOnHandPositionCount;
	int openBarIndex;
	mSerializeHeader header;

	double mOpenPrice;	// 今日开盘价格
	char* StrategyfilePoint;
	HANDLE StrategyInfoFileMap;
	map<string, string> matchnomap;

	time_t tmt_StartTime;
	time_t tmt_EndTime;
	int mOpenTimes;			// 一天只开仓一次
	double mYOpen;
	double mYHigh;
	double mYLow;
	double mYClose;
	double mAvg;
	bool bOpenRet;
};
