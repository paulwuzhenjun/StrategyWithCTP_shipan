#pragma once
#include "strategy.h"
#include <map>
using namespace std;
// ���߻ص����ԣ��������ߣ����տ��࣬�������ߣ����տ���,����������
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
		// ����ʱ��
		char OpenTime[30];
		// ����ʱ��
		char CloseTime[30];
		// ���̼۸�
		double OpenPrice;
		int LoopTimes;
		// ���ں�Լ�������̼۸�İٷֱ�
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
	// ���5minBar
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
	// ��ȡ���ռ۸�
	void LoadHisData();
	// A50��ȡ���̼۸�����̼۸�
	void LoadCNData();

	bool timeRuleForOpen(char datadate[10], char datatime[10]);
	bool timeRuleForClose(char datadate[10], char datatime[10]);
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
	BarRateInfo* mOldBarFile;
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

	double mOpenPrice;	// ���տ��̼۸�
	char* StrategyfilePoint;
	HANDLE StrategyInfoFileMap;
	map<string, string> matchnomap;

	time_t tmt_StartTime;
	time_t tmt_EndTime;
	int mOpenTimes;			// һ��ֻ����һ��
	double mYOpen;
	double mYHigh;
	double mYLow;
	double mYClose;
	double mAvg;
	bool bOpenRet;
};
