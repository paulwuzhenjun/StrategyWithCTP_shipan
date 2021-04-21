#pragma once
#include ".\sgittraderapi\SgitFtdcMdApi.h"

class CSgitMdSpi : public fstech::CThostFtdcMdSpi
{
public:
	CSgitMdSpi(fstech::CThostFtdcMdApi* xMdApi, char xBROKER_ID[20], char xINVESTOR_ID[20], char xPASSWORD[20]);

	///错误应答
	virtual void OnRspError(fstech::CThostFtdcRspInfoField* pRspInfo,
		int nRequestID, bool bIsLast);

	virtual void OnFrontDisconnected(int nReason);

	virtual void OnHeartBeatWarning(int nTimeLapse);

	///当客户端与交易后台建立起通信连接时
	virtual void OnFrontConnected();

	///登录请求响应
	virtual void OnRspUserLogin(fstech::CThostFtdcRspUserLoginField* pRspUserLogin, fstech::CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///订阅行情应答
	virtual void OnRspSubMarketData(fstech::CThostFtdcSpecificInstrumentField* pSpecificInstrument, fstech::CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///取消订阅行情应答
	virtual void OnRspUnSubMarketData(fstech::CThostFtdcSpecificInstrumentField* pSpecificInstrument, fstech::CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///深度行情通知
	virtual void OnRtnDepthMarketData(fstech::CThostFtdcDepthMarketDataField* pDepthMarketData);

	bool timeRuleForOpen(fstech::TThostFtdcTimeType datatime);
	int m_count;

	char beginrun_date[10];
	char tradingday[15];
	void Release();
private:
	fstech::CThostFtdcMdApi* pMdApi;
	fstech::TThostFtdcBrokerIDType	mBROKER_ID;
	fstech::TThostFtdcInvestorIDType mINVESTOR_ID;
	fstech::TThostFtdcPasswordType	mPASSWORD;

	void ReqUserLogin();
	void SubscribeMarketData();
	//
	bool IsErrorRspInfo(fstech::CThostFtdcRspInfoField* pRspInfo);
	//void ReqOrderInsert(int openOrClose,int buyOrSell,double LimitPrice,CThostFtdcDepthMarketDataField *pDepthMarketData);
};