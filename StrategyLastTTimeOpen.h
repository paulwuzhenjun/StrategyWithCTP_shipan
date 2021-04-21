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
class StrategyLastTTimeOpen : public CStrategy
{
	struct mParasType
	{
		char InstCodeName[30];
		double OpenDistPoint;	// �ǵ�����
		int SamplePeriod;
		double ProfitPoint;
		double StoplossPoint;
		int OpenVol1;
		int MAStopStartPoint;	// ׷��ֹӮ��λ
		int BackPoint;			// �ص���λ
		double DrawPercent;		// �س��ٷֱ�
		char StartTime[30];
		char EndTime[30];
		bool OpenBuyAllow;
		bool OpenSellAllow;
		//bool CloseAllow;
		int LoopTimes;
	};

private:
	char  m_InstID[30];  //Զ�ں�Լ����
	char  m_CommodityNo[30];

	double m_Price;//��ʱ�ļ۸�
	double m_Buy1;  //Զ����1��
	double m_Sell1; //Զ����1��

	time_t curTickTime;

	bool timeRuleForOpen(char datadate[10], char datatime[10]);
	bool timeRuleForClose(char datadate[10], char datatime[10]);

	//double m_dOneTick;
	void AddToRatesX(int tickCount, TickInfo* curTick, int gRatesXSize);

	BarRateInfo* mBarFile;
	int mBarIndex;
	double mOpenPrice;
	int mOpenTimes;
	bool StoplossClose;
	double mdTHigh;		// Tʱ������߼۸�
	double mdTLow;		// Tʱ������ͼ۸�
public:
	StrategyLastTTimeOpen(char InstrumentID[30]);
	~StrategyLastTTimeOpen(void);
	bool m_bIsRunning; //��ʼ��Ϊture
	bool m_bSampleReady;

	void InitVariables();
	void ResetStrategy();
	//void MessageProcess();

	void ReleaseDataArray();
	void InitDataArray();

	void InitAction();
	void ResetAction();
	void exitAction();

	int mTickCount;
	int mCloseBarIndex;

	time_t tmt_StartTime;
	time_t tmt_EndTime;

	mParasType mStrategyParams;
	void SetParamValue(ParamNode node);

	char mInstanceName[50];
	string mStrategyAndInstance;
	static int MaxOnHandPositionCount;
	static int OnHandPositionCount;

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
	char m_ExchangeID[30];
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
	void getDistance(double price, int& nBuy, int& nSell);

	mSerializeHeader header;
	wstring s2ws(const string& s);
	int CreateStrategyMapOfView();
	int OpenStrategyMapOfView();
	void FlushStrategyInfoToFile();
	HANDLE StrategyInfoFileMap;
	char* StrategyfilePoint;
};