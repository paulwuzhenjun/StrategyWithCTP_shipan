#pragma once

#include ".\ThostTraderApi\ThostFtdcTraderApi.h"

class TestSpi :
	public CThostFtdcTraderSpi
{
public:
	TestSpi(void);
	~TestSpi(void);

	virtual void OnFrontConnected();
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void TestSpi::OnRtnOrder(CThostFtdcOrderField *pOrder);
	virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);
private:
	//void ReqUserLogin(void);
private:
	void ReqUserLogin();
		///��¼������Ӧ
	bool TestSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);
		// �Ƿ��ҵı����ر�
	bool IsMyOrder(CThostFtdcOrderField *pOrder);
	// �Ƿ����ڽ��׵ı���
	bool IsTradingOrder(CThostFtdcOrderField *pOrder);
	void ReqQryTradingAccount();
	void ReqSettlementInfoConfirm();
	void ReqQryInstrument();
};
