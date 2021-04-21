#pragma once
#include ".\ThostTraderApi\ThostFtdcMdApi.h"

class CThostMdSpi : public CThostFtdcMdSpi
{
public:
	CThostMdSpi(CThostFtdcMdApi* xMdApi, char xBROKER_ID[20], char xINVESTOR_ID[20], char xPASSWORD[20]);

	///错误应答
	virtual void OnRspError(CThostFtdcRspInfoField* pRspInfo,
		int nRequestID, bool bIsLast);

	virtual void OnFrontDisconnected(int nReason);

	virtual void OnHeartBeatWarning(int nTimeLapse);

	///当客户端与交易后台建立起通信连接时
	virtual void OnFrontConnected();

	///登录请求响应
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///订阅行情应答
	virtual void OnRspSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///取消订阅行情应答
	virtual void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///深度行情通知
	virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData);

	bool timeRuleForOpen(TThostFtdcTimeType datatime);
	int m_count;

	char beginrun_date[10];
	char tradingday[15];

	void Release();

	int AddSubscribeMarketData(char* ppNewInstrumentID[100], int iNewInstrumentID);
	int UnSubscribeMarketData(char* ppNewInstrumentID[100], int iNewInstrumentID);
private:
	CThostFtdcMdApi* pMdApi;
	TThostFtdcBrokerIDType	mBROKER_ID;
	TThostFtdcInvestorIDType mINVESTOR_ID;
	TThostFtdcPasswordType	mPASSWORD;

	void ReqUserLogin();

	void SubscribeMarketData();
	//
	bool IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo);
	//void ReqOrderInsert(int openOrClose,int buyOrSell,double LimitPrice,CThostFtdcDepthMarketDataField *pDepthMarketData);
};