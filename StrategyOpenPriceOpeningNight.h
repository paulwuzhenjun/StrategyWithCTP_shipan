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

//�ݹ�ָ�����
class StrategyOpenPriceOpeningNight : public CStrategy
{
	struct mParasType
	{
		char InstCodeName[30];
		double OpenDistPoint;
		double FirstProfitPoint;
		double FirstStoplossPoint;
		int OpenVol1;
		char StartTime[30];
		char EndTime[30];
		bool OpenBuyAllow;
		bool OpenSellAllow;
		double OpenPrice;
		char OpenTime[30];		// ����ʱ��
		char CloseTime[30];		// ����ʱ��
		char NCloseTime[30];	// ҹ������ʱ��
		bool bNight;			// ҹ��ƽ��
		int LoopTimes;
	};

private:
	char  m_InstID[30];  //Զ�ں�Լ����
	char  m_CommodityNo[30];

	double m_Price;//��ʱ�ļ۸�
	double m_Buy1;  //Զ����1��
	double m_Sell1; //Զ����1��

	bool timeRuleForOpen(char datatime[10]);
	bool timeRuleForClose(char datadate[10], char datatime[10]);
	bool timeRuleForCancel(char datadate[10], char datatime[10]);

	//double m_dOneTick;
	int mOpenTimes;
	bool Type1OrderExist;
	bool Type2OrderExist;
	bool Type2StoplossClose;

	map<string, string> matchnomap;

public:
	StrategyOpenPriceOpeningNight(char InstrumentID[30]);
	~StrategyOpenPriceOpeningNight(void);
	bool m_bIsRunning; //��ʼ��Ϊture

	void InitVariables();
	void ResetStrategy();
	//void MessageProcess();

	void InitAction();
	//void ResetAction(){}
	//void exitAction(){}

	int mTickCount;
	double mOpenPrice;
	bool mOpenRet;
	time_t curTickTime;

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
	char m_ExchangeID[30];
	int GetShmindex();
	void SetShmindex(int xshmindex);
	void SetStrategyID(char strategyId[50]);			// ������ CL-GD-00006

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

	char TickRealTimeDataDate[12];
	void CheckAndResubmitOutOfDateOrder(char cTickDataDate[11], char cTickUpdateTime[11]);

	// ����ƽ��ӯ���ͳֲ�ӯ��
	double mCloseProfit;
	double mPositionProfit;
	time_t mPositionTime;
	void FlushPositionProfit(char* dataDate, char* dataTime, int dir, int position);
};