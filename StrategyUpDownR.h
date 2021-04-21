#pragma once
#include "Strategy.h"
#include <map>

using namespace std;

class StrategyUpDownR : public CStrategy
{
	struct mParasType
	{
		char InstCodeName[30];
		double StoplossPoint;
		//int OpenVolMulti;
		char StartTime[30];
		char EndTime[30];
		bool OpenBuyAllow;
		bool OpenSellAllow;
		// 开盘时间
		char OpenTime[30];
		// 收盘时间
		char CloseTime[30];
		int LoopTimes;
		// 上涨或者下跌幅度
		double m_fX;
		// 回调幅度百分比
		double m_fY;
		// 盈利百分比(相对于幅度)
		double m_fY2;
		// 交易时段(1全天2美盘)
		int Period;
	};
public:
	StrategyUpDownR(char InstrumentID[30]);
	~StrategyUpDownR(void);

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
	void AddToRatesX(int tickCount, TickInfo* curTick, int gRatesXSize);
	void InitVariables();
	void AddtoTipMsgListBox(char msgline[200]);
	void DisplayTradeOnScreen(OrderTradeMsg* pOrderTradeMsg, int mDirection, int mOpenOrClose, int mCloseProfitOrNot);
	bool timeRuleOK(char datatime[10]);
	void FlushStrategyInfoToFile();
	char m_ExchangeID[30];
	int GetShmindex() { return shmindex; }
	void SetShmindex(int xshmindex) { shmindex = xshmindex; }
	void ReleaseDataArray();
	void InitDataArray();
	int CreateStrategyMapOfView();
	int OpenStrategyMapOfView();
	wstring s2ws(const string& s);
	void CloseMap();
	void getDistance(double& highPrice, double& lowPrice, time_t& t1, time_t& t2);

	bool timeRuleForOpen(char datatime[10]);
	bool timeRuleForClose(char datadate[10], char datatime[10]);
	bool isOpenTime(char datetime[10]);
	int  timeToSecond(char datetime[10]);
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
	// 周期
	int mBarSize;
	// 记录平仓的barIndex;
	int mCloseBarIndex;
	double mdTHigh;
	double mdTLow;

	int lastmBarIndex;
	mParasType mStrategyParams;
	char  m_InstID[30];  //远期合约类型
	char  m_CommodityNo[30];
	int shmindex;
	list<MyCloseOrderType> CloseOrderList;
	list<MyOpenOrderType> OpenOrderList;
	bool m_bCrossTradingDay;
	string mStrategyAndInstance;
	int InstanceOnHandPositionCount;
	int openBarIndex;
	mSerializeHeader header;

	double mOpenPrice;
	char* StrategyfilePoint;
	HANDLE StrategyInfoFileMap;
	map<string, string> matchnomap;

	time_t tmt_StartTime;
	time_t tmt_EndTime;
};