#pragma once
#include "strategy.h"
#include <map>
using namespace std;

// 开仓和平仓撤单后不会重新挂单

class StrategyBaseGridOpen : public CStrategy
{
	struct mParasType
	{
		char InstCodeName[30];
		char StartTime[30];
		char EndTime[30];
		char OpenTime[30];
		char CloseTime[30];
		bool OpenBuyAllow;
		bool OpenSellAllow;
		int LoopTimes;
		int UpGridCount;				// 上网格数量
		int UpGridDistPoint;			// 上网格间距
		int DnGridCount;				// 下网格数量
		int DnGridDistPoint;			// 下网格间距
		int GridVol;
		double BasePrice;
		//int UpTwoGridTickDist[5];
		int ProfitPoint;				// 网格止盈点
		int StoplossPoint;				// 网格止损点
		//double TradeFee;				// 交易手续费
		bool IsBasePrice;				// 基准价是否固定
		//int DnTwoGridTickDist[5];
	};

private:
	char  m_InstID[30];  //远期合约类型
	char  m_CommodityNo[30];
	char m_ExchangeID[30];
	char m_InstCodeName[30];	// 合约全名 NYMEX CL 1809/SHFE au au1812

	double m_Price;//此时的价格
	double m_Buy1;  //远期买1价
	double m_Sell1; //远期卖1价

	time_t curTickTime;

	bool timeRuleForOpen(char datadate[10],char datatime[10]);
	bool timeRuleForOpenNew(char datadate[10],char datatime[10]);
	bool timeRuleForClose(char datadate[10],char datatime[10]);
	bool timeRuleForCancel(char datadate[10],char datatime[10]);		// 收盘前1分钟撤单
	//bool timeRuleForCancel(char datadate[10],char datatime[10]);		// 凌晨撤单
	bool timeRuleForClosePrice(char datadate[10],char datatime[10]);

	bool m_bStoplossClose;
	int mOpenTimes;
	//int GridCloseOrderCount[10];
	bool mOpenRet;
	double mClosePrice;
	double mOpenPrice;
public:
	StrategyBaseGridOpen(char InstrumentID[30]);
	~StrategyBaseGridOpen(void);
	bool m_bIsRunning; //初始化为ture
	bool m_bSampleReady;

	void InitVariables();
	void ResetStrategy();
	void SetCloseLocalOrder(LocalCLForDisplayField *pCLOrder);
	void SetParamValue(ParamNode node);
	//void MessageProcess();

	void InitAction();

	int mTickCount;
	int GridCloseOrderCount[60];

	time_t tmt_StartTime;
	time_t tmt_EndTime;

	mParasType mStrategyParams;
	//void SetParamValue(ParamNode node);

	char mInstanceName[50];
	string mStrategyAndInstance;
	static int MaxOnHandPositionCount;
	static int OnHandPositionCount;

	map<string,string> matchnomap;
private:
	list<MyCloseOrderType> CloseOrderList;
	list<MyOpenOrderType> OpenOrderList;
	int shmindex;
	bool m_bCrossTradingDay;
	char curTradingDay[30];

	void UpdateOrderIdToOrderList(int OrderId);
	void DeleteErrorOrderIdFromOrderList();
	void SetInstanceName(char xInstanceName[50]);
	void GetInstanceName(char *xInstanceName);
	int GetShmindex();
	void SetShmindex(int xshmindex);
	void SetStrategyID(char strategyId[50]);

	void ReqOrderDelete(int pOrderId,int pOrderLocalRef,int pFrontID,int pSessionID);
	void ReqOpenOrderInsert(MyOpenOrderType *pOpenOrder);
	void ReqCloseOrderInsert(MyCloseOrderType *pCloseOrder,char OpenOrderTime[21]);

	void OnRtnDepthMarketData(TickInfo *pDepthMarketData);
	void OnRtnOrder(OrderTradeMsg *pOrderTradeMsg);
	void OnRtnTrade(OrderTradeMsg *pOrderTradeMsg);
	void OnRspOrderInsert(ShmRspOrderInsert *pRspOrderInsert);
	void OnDisplayLocalCloseOrderList();
	void WideCharToMultiChar(CString str,char *dest_str);
	void WriteMsgToLogList(char logline[200]);
	void AddtoTipMsgListBox(char msgline[200]);
	void DisplayTradeOnScreen(OrderTradeMsg *pOrderTradeMsg,int mDirection,int mOpenOrClose,int mCloseProfitOrNot);

	void RecoverInstance(char cfgFileName[500]);
	void CloseMap();

	mSerializeHeader header;
	wstring s2ws(const string& s);
	int CreateStrategyMapOfView();
	int OpenStrategyMapOfView();
	void FlushStrategyInfoToFile();
	HANDLE StrategyInfoFileMap;
	char *StrategyfilePoint;
};

