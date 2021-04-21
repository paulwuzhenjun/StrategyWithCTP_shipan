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
class StrategyWaveOpenAdd : public CStrategy
{
	struct mParasType
	{
		char InstCodeName[30];
		int WaveLengthPoint;
		int WaveBackPoint;
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
		int LoopTimes;
	};

private:
	char  m_InstID[30];  //远期合约类型
	char  m_CommodityNo[30];
	char m_ExchangeID[30];

	double m_Price;//此时的价格
	double m_Buy1;  //远期买1价
	double m_Sell1; //远期卖1价

	time_t curTickTime;

	bool timeRuleForOpen(char datadate[10], char datatime[10]);
	bool timeRuleForClose(char datadate[10], char datatime[10]);

	double mOpenPrice;
	int mOpenTimes;
	//---------------------------------------
	double upWaveMax;
	double upWaveMin;
	char upMaxTime[20];
	char upMinTime[20];

	double dnWaveMax;
	double dnWaveMin;
	char dnMaxTime[20];
	char dnMinTime[20];

	int mPairID;
public:
	StrategyWaveOpenAdd(char InstrumentID[30]);
	~StrategyWaveOpenAdd(void);
	bool m_bIsRunning; //初始化为ture
	bool m_bSampleReady;

	void InitVariables();
	void ResetStrategy();
	//void MessageProcess();

	int mTickCount;

	time_t tmt_StartTime;
	time_t tmt_EndTime;

	mParasType mStrategyParams;
	void SetParamValue(ParamNode node);
	void SetCloseLocalOrder(LocalCLForDisplayField* pCLOrder);

	char mInstanceName[50];
	string mStrategyAndInstance;
	static int MaxOnHandPositionCount;
	static int OnHandPositionCount;
	int InstanceOnHandPositionCount;

	map<string, string> matchnomap;
private:
	list<MyCloseOrderType> CloseOrderList;
	list<MyOpenOrderType> OpenOrderList;
	int shmindex;
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
	void WideCharToMultiChar(CString str, char* dest_str);
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
};