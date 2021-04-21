#pragma once
#include ".\thosttraderapi\thostftdctraderapi.h"
#include "MyStruct.h"

class ThostTraderSpi :
	public CThostFtdcTraderSpi
{
public:
	//BaseStrategyObserver* m_pObserver;
public:
	ThostTraderSpi(CThostFtdcTraderApi* xTraderApi, char xBROKER_ID[20], char xINVESTOR_ID[20], char xPASSWORD[20]);
	~ThostTraderSpi(void);

	//virtual void OnFrontConnected();
	//virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	//virtual void OnRtnOrder(CThostFtdcOrderField *pOrder);
	//virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	//virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	//virtual void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

//	virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);

		///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
	virtual void OnFrontConnected();

	///登录请求响应
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///投资者结算结果确认响应
	virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///请求查询合约响应
	virtual void OnRspQryInstrument(CThostFtdcInstrumentField* pInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///请求查询资金账户响应
	virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField* pTradingAccount, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///请求查询投资者持仓响应
//	virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///报单录入请求响应
	virtual void OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);//Commented by Sam

	///报单操作请求响应
	virtual void OnRspOrderAction(CThostFtdcInputOrderActionField* pInputOrderAction, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);//Commented by Sam

	///错误应答
//	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
	//virtual void OnFrontDisconnected(int nReason);

	///心跳超时警告。当长时间未收到报文时，该方法被调用。
	//virtual void OnHeartBeatWarning(int nTimeLapse);

	///报单通知
	virtual void OnRtnOrder(CThostFtdcOrderField* pOrder);

	///成交通知
	virtual void OnRtnTrade(CThostFtdcTradeField* pTrade);
	//void BindObserver(BaseStrategyObserver* observer);

	///委托查询通知
	virtual void OnRspQryOrder(CThostFtdcOrderField* pOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///请求查询成交响应
	virtual void OnRspQryTrade(CThostFtdcTradeField* pTrade, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///持仓明细查询通知
	virtual void OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField* pInvestorPositioDetail, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	void Release();
private:
	CThostFtdcTraderApi* mTraderApi;
	TThostFtdcBrokerIDType	mBROKER_ID;
	TThostFtdcInvestorIDType mINVESTOR_ID;
	TThostFtdcPasswordType	mPASSWORD;
	TThostFtdcFrontIDType mFRONT_ID;
	TThostFtdcSessionIDType mSESSION_ID;
	void ReqAuthenticate(TThostFtdcBrokerIDType	appId, TThostFtdcUserIDType	userId, TThostFtdcAuthCodeType	AuthCode, TThostFtdcAppIDType	AppID);
	void OnRspAuthenticate(CThostFtdcRspAuthenticateField* pRspAuthenticateField, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	void ReqUserLogin();
	///登录请求响应
	bool IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo);
	// 是否我的报单回报
	bool IsMyOrder(CThostFtdcOrderField* pOrder);
	// 是否正在交易的报单
	bool IsTradingOrder(CThostFtdcOrderField* pOrder);

	void ReqSettlementInfoConfirm();
	void ReqQryInstrument();

public:
	void ReqQryTradingAccount();

	int ReqOrderDelete(OrderDetailField* pOrderField);
	int ReqOrderDeletePerOrderLocalRef(int x_dOrderLocalRef, char x_cInstrumentID[30], char x_cExchangeID[30], int x_FrontID, int x_SessionID);
	int ReqOrderInsert(InsertOrderField* pReq, bool x_bCrossTradingDay, bool CloseToday, int xshmindex);
	void ReqQryTrade(char ExchangeNo[30], char CommodityNo[30], char InstrumentID[30], char BeginMatchDateTime[21]);
	void ReqQryOrder(char ExchangeNo[30], char CommodityNo[30], char InstrumentID[30]);
	void ReqQryPosition(char ExchangeNo[30], char CommodityNo[30], char InstrumentID[30]);
	int ThostTraderSpi::ReqOrderInsertCffex(InsertOrderField* pReq, bool x_bCrossTradingDay, bool CloseToday, int xshmindex, int& yBuyPosition, int& ySellPosition);

	void WriteMsgToLogList(char logline[200]);

	double m_Balance;
};
