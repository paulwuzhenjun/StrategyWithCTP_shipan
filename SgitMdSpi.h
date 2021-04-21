#pragma once
#include ".\sgittraderapi\SgitFtdcMdApi.h"

class CSgitMdSpi : public fstech::CThostFtdcMdSpi
{
public:
	CSgitMdSpi(fstech::CThostFtdcMdApi* xMdApi, char xBROKER_ID[20], char xINVESTOR_ID[20], char xPASSWORD[20]);

	///����Ӧ��
	virtual void OnRspError(fstech::CThostFtdcRspInfoField* pRspInfo,
		int nRequestID, bool bIsLast);

	virtual void OnFrontDisconnected(int nReason);

	virtual void OnHeartBeatWarning(int nTimeLapse);

	///���ͻ����뽻�׺�̨������ͨ������ʱ
	virtual void OnFrontConnected();

	///��¼������Ӧ
	virtual void OnRspUserLogin(fstech::CThostFtdcRspUserLoginField* pRspUserLogin, fstech::CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///��������Ӧ��
	virtual void OnRspSubMarketData(fstech::CThostFtdcSpecificInstrumentField* pSpecificInstrument, fstech::CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///ȡ����������Ӧ��
	virtual void OnRspUnSubMarketData(fstech::CThostFtdcSpecificInstrumentField* pSpecificInstrument, fstech::CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�������֪ͨ
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