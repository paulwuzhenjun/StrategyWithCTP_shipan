#pragma once
#include "strategy.h"
#include <map>
using namespace std;
// 均线回调策略，昨日阳线，今日开多，昨日阴线，今日开空,不隔交易日
class StrategyDT : public CStrategy
{
	struct mParasType
	{
		char InstCodeName[30];
		double ProfitPoint1;
		double ProfitPoint2;
		double StoplossPoint;
		double StoplossPoint1;
		double StoplossPoint2;
		int MA1;
		int MA2;
		double MA1Value;
		double MA2Value;
		double DT;
		int Ktime;
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
		int Vol;
		double OpenPrice;
		int LoopTimes;
		// 大于合约昨日收盘价格的百分比
		double m_fTP;
		double m_fTS;
	};
public:
	StrategyDT(char InstrumentID[30]);
	~StrategyDT(void);

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
	void exitAction(int i);

	void SetInstanceName(char xInstanceName[50]);
	void GetInstanceName(char* xInstanceName);

	void SetStrategyID(char strategyId[50]);
	void RecoverInstance(char cfgFileName[500]);
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
	void LoadHisData(BarRateInfo* xBarFile);
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
	char  m_ExchangeID[30];
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
	double MA1Avg;
	double MA2Avg;
	double PreMA1Avg;
	double PreMA2Avg;
	double MA1sum;
	double MA2sum;
	double DT;
	bool bOpenRet;
};
