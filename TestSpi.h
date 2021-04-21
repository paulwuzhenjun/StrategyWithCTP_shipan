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
		///登录请求响应
	bool TestSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);
		// 是否我的报单回报
	bool IsMyOrder(CThostFtdcOrderField *pOrder);
	// 是否正在交易的报单
	bool IsTradingOrder(CThostFtdcOrderField *pOrder);
	void ReqQryTradingAccount();
	void ReqSettlementInfoConfirm();
	void ReqQryInstrument();
};
