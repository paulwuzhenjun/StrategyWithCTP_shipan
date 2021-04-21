#pragma once
#include ".\sgittraderapi\SgitFtdcTraderApi.h"
#include "MyStruct.h"

class SgitTraderSpi :
	public fstech::CThostFtdcTraderSpi
{
public:
	//BaseStrategyObserver* m_pObserver;
public:
	SgitTraderSpi(fstech::CThostFtdcTraderApi* xTraderApi, char xBROKER_ID[20], char xINVESTOR_ID[20], char xPASSWORD[20]);
	~SgitTraderSpi(void);

	//virtual void OnFrontConnected();
	//virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	//virtual void OnRtnOrder(CThostFtdcOrderField *pOrder);
	//virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	//virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	//virtual void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

//	virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);

		///���ͻ����뽻�׺�̨������ͨ������ʱ����δ��¼ǰ�����÷��������á�
	virtual void OnFrontConnected();

	///��¼������Ӧ
	virtual void OnRspUserLogin(fstech::CThostFtdcRspUserLoginField* pRspUserLogin, fstech::CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///Ͷ���߽�����ȷ����Ӧ
	virtual void OnRspSettlementInfoConfirm(fstech::CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, fstech::CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯ��Լ��Ӧ
	virtual void OnRspQryInstrument(fstech::CThostFtdcInstrumentField* pInstrument, fstech::CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯ�ʽ��˻���Ӧ
	virtual void OnRspQryTradingAccount(fstech::CThostFtdcTradingAccountField* pTradingAccount, fstech::CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯͶ���ֲ߳���Ӧ
//	virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///����¼��������Ӧ
	virtual void OnRspOrderInsert(fstech::CThostFtdcInputOrderField* pInputOrder, fstech::CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);//Commented by Sam

	///��������������Ӧ
	virtual void OnRspOrderAction(fstech::CThostFtdcInputOrderActionField* pInputOrderAction, fstech::CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);//Commented by Sam

	///����Ӧ��
//	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///���ͻ����뽻�׺�̨ͨ�����ӶϿ�ʱ���÷��������á���������������API���Զ��������ӣ��ͻ��˿ɲ�������
	//virtual void OnFrontDisconnected(int nReason);

	///������ʱ���档����ʱ��δ�յ�����ʱ���÷��������á�
	//virtual void OnHeartBeatWarning(int nTimeLapse);

	///����֪ͨ
	virtual void OnRtnOrder(fstech::CThostFtdcOrderField* pOrder);

	///�ɽ�֪ͨ
	virtual void OnRtnTrade(fstech::CThostFtdcTradeField* pTrade);
	//void BindObserver(BaseStrategyObserver* observer);

	///ί�в�ѯ֪ͨ
	virtual void OnRspQryOrder(fstech::CThostFtdcOrderField* pOrder, fstech::CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯ�ɽ���Ӧ
	virtual void OnRspQryTrade(fstech::CThostFtdcTradeField* pTrade, fstech::CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�ֲ���ϸ��ѯ֪ͨ
	virtual void OnRspQryInvestorPositionDetail(fstech::CThostFtdcInvestorPositionDetailField* pInvestorPositioDetail, fstech::CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
private:
	fstech::CThostFtdcTraderApi* mTraderApi;
	fstech::TThostFtdcBrokerIDType	mBROKER_ID;
	fstech::TThostFtdcInvestorIDType mINVESTOR_ID;
	fstech::TThostFtdcPasswordType	mPASSWORD;
	fstech::TThostFtdcFrontIDType mFRONT_ID;
	fstech::TThostFtdcSessionIDType mSESSION_ID;

	void ReqUserLogin();
	///��¼������Ӧ
	bool IsErrorRspInfo(fstech::CThostFtdcRspInfoField* pRspInfo);
	// �Ƿ��ҵı����ر�
	bool IsMyOrder(fstech::CThostFtdcOrderField* pOrder);
	// �Ƿ����ڽ��׵ı���
	bool IsTradingOrder(fstech::CThostFtdcOrderField* pOrder);

	void ReqSettlementInfoConfirm();
	void ReqQryInstrument();

public:
	void ReqQryTradingAccount();

	int ReqOrderDelete(OrderDetailField* pOrderField);
	int ReqOrderDeletePerOrderLocalRef(int x_dOrderLocalRef, char x_cInstrumentID[30], int x_FrontID, int x_SessionID);
	int ReqOrderInsert(InsertOrderField* pReq, bool x_bCrossTradingDay, bool CloseToday, int xshmindex);
	void ReqQryTrade(char ExchangeNo[30], char CommodityNo[30], char InstrumentID[30], char BeginMatchDateTime[21]);
	void ReqQryOrder(char ExchangeNo[30], char CommodityNo[30], char InstrumentID[30]);
	void ReqQryPosition(char ExchangeNo[30], char CommodityNo[30], char InstrumentID[30]);
};
