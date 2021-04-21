#pragma once
#include "strategy.h"
#include <map>

using namespace std;

class StrategyAvgLine : public CStrategy
{
	struct mParasType
	{
		char InstCodeName[30];
		double ProfitPoint;
		double StoplossPoint;
		//int OpenVolMulti;
		char StartTime[30];
		char EndTime[30];
		bool OpenBuyAllow;
		bool OpenSellAllow;
		//bool CloseAllow;
		char OpenTime[10];
		char CloseTime[10];
		int LoopTimes;
		// ���Ȱٷֱ�
		double m_fX;
		// �ص����Ȱٷֱ�
		double m_fY;
		double m_fTP;
		double m_fTS;
	};
public:
	StrategyAvgLine(char InstrumentID[30]);
	~StrategyAvgLine(void);

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
	// ����10�վ���
	void calcAvg(int size, char* table);
	bool timeRuleForOpen(char datadate[10], char datatime[10]);
	bool timeRuleForClose(char datadate[10], char datatime[10]);
	int timeToSecond(char datetime[10]);
public:
	char mInstanceName[50];
	// ���ֲ�
	static int MaxOnHandPositionCount;
	// �������ֲֳ�
	static int OnHandPositionCount;

private:
	bool m_bIsRunning;
	TickInfo m_ThisData;
	int mBarIndex;
	double m_Price;//���¼۸�
	double m_Buy1;  //��1��
	double m_Sell1; //��1��
	int mTickCount;
	BarRateInfo* mBarFile;
	// ����
	int mBarSize;
	int lastmBarIndex;
	mParasType mStrategyParams;
	char  m_InstID[30];  //Զ�ں�Լ����
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

	double mOpenPrice;
	char* StrategyfilePoint;
	HANDLE StrategyInfoFileMap;
	map<string, string> matchnomap;

	time_t tmt_StartTime;
	time_t tmt_EndTime;
	double m_fAvg;			// 10�վ���
	int m_nDate[10];
	double m_fStop;			// ��¼ֹ��۸�
	bool m_bBuyRet;
	bool m_bSellRet;
	double m_dHigh;
	double m_dLow;
};
