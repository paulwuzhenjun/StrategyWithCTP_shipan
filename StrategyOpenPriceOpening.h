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

// ���̼����¹ҵ�,����ֹ�����ֹ����;2��30���Ժ��ֹ����,�ڶ���12�㿳��,�Ե��쿪�̼����¹ҵ�
class StrategyOpenPriceOpening : public CStrategy
{
	struct mParasType
	{
		char InstCodeName[30];
		double OpenDistPoint;
		double SecondOpenDistPoint;
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
		double OpenPrice;		// �ⲿ���趨���̼�
		char OpenTime[30];		// ����ʱ��
		char CloseTime[30];		// ����ʱ��
		char ChopTime[30];		// ����ʱ��
		int LoopTimes;
	};

private:
	char  m_InstID[30];  //Զ�ں�Լ����
	char  m_CommodityNo[30];

	double m_Price;//��ʱ�ļ۸�
	double m_Buy1;  //Զ����1��
	double m_Sell1; //Զ����1��

	bool timeRuleForOpen(char datadate[10], char datatime[10]);
	bool timeRuleForClose(char datadate[10], char datatime[10]);
	bool timeRuleForCancelOpen(char datadate[10], char datatime[10]);//2:30~05:00�������ֵ�
	bool timeRuleForStop(int nHour, int nMin, int nSec);			// ��ֹ���ֵ�
	bool timeRuleForCloseNew(char openTime[30], char datadate[10], char datatime[10]);	// ���ֵ�

	//double m_dOneTick;
	int mOpenTimes;
	bool Type1OrderExist;
	bool Type2OrderExist;
	bool Type2StoplossClose;

	map<string, string> matchnomap;

public:
	StrategyOpenPriceOpening(char InstrumentID[30]);
	~StrategyOpenPriceOpening(void);
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
};