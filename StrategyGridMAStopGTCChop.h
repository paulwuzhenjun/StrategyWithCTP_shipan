#pragma once
#include "strategy.h"
#include <map>
using namespace std;
//#define ES_GRIDCOUNT 120		// ����������������

// ���̳�����Ч���̶��������̶�ֹӯ
// ��һ������
// ȡ���ҵ���ʽ

class StrategyGridMAStopGTCChop : public CStrategy
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
		int UpGridCount;				// ����������
		int UpGridDistPoint;			// ��������
		int DnGridCount;				// ����������
		int DnGridDistPoint;			// ��������
		int GridVol;
		double BasePrice;
		double StopPrice;				// �̶�ֹ��۸�
		bool StopAfterOpen;				// ֹ����Ƿ񿪲�
		//int UpTwoGridTickDist[5];
		int ProfitPoint;				// ����ֹӯ��
		int StoplossPoint;				// ����ֹ���(ֹ��㲻������)
		bool IsBasePrice;				// ��׼���Ƿ�̶�
		double BackPerc;				// ׷��ֹӮ�س��ٷֱ�
		//int DnTwoGridTickDist[5];
		int OffFlagPoint;				// ��׼ƫ��
	};

private:
	char  m_InstID[30];  //Զ�ں�Լ����
	char  m_CommodityNo[30];
	char m_ExchangeID[30];
	char m_InstCodeName[30];

	double m_Price;//��ʱ�ļ۸�
	double m_Buy1;  //Զ����1��
	double m_Sell1; //Զ����1��

	time_t curTickTime;

	bool timeRuleForOpen(char datadate[10], char datatime[10]);
	bool timeRuleForClose(char datadate[10], char datatime[10]);
	bool timeRuleForCancel(char datadate[10], char datatime[10]);		// ����ǰ10���ӳ���
	bool timeRuleForClosePrice(char datadate[10], char datatime[10]);

	bool m_bStoplossClose;
	int mOpenTimes;
	//int GridCloseOrderCount[10];
	bool mOpenRet;
	bool mCloseRet;
	double mClosePrice;
	double mPrice;				// ���Ա���ָ��۸�
	bool mRet;
	bool mOrderRet;

public:
	StrategyGridMAStopGTCChop(char InstrumentID[30]);
	~StrategyGridMAStopGTCChop(void);
	bool m_bIsRunning; //��ʼ��Ϊture
	bool m_bSampleReady;

	void InitVariables();
	void ResetStrategy();
	void SetCloseLocalOrder(LocalCLForDisplayField* pCLOrder);
	void SetParamValue(ParamNode node);
	//void MessageProcess();

	void InitAction();

	int mTickCount;
	int GridCloseOrderCount[ES_GRIDCOUNT];			// ���֧�����¸�10������

	time_t tmt_StartTime;
	time_t tmt_EndTime;

	mParasType mStrategyParams;
	//void SetParamValue(ParamNode node);

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
	//char curTradingDay[30];

	void UpdateOrderIdToOrderList(int OrderId);
	void DeleteErrorOrderIdFromOrderList();
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
	void WriteBasePrice(char* date, char* time);

	mSerializeHeader header;
	wstring s2ws(const string& s);
	int CreateStrategyMapOfView();
	int OpenStrategyMapOfView();
	void FlushStrategyInfoToFile();
	HANDLE StrategyInfoFileMap;
	char* StrategyfilePoint;
	bool IsOpenFlag();
};