#pragma once
#include "StdAfx.h"
#include "SgitTraderSpi.h"
#include "MyStruct.h"
#include "OrderDataList.h"
#include "MessageList.h"
#include "LockVariable.h"
#include "SgitTraderApi\SgitFtdcUserApiDataType.h"
#include <map>

using namespace std;

extern CListBox* pPubMsg;
extern CStatic* pMoneyCTP;
extern CStatic* pBalanceCTP;
extern CStatic* pUsernameCTP;
extern int g_iRequestIDThost;
extern OrderDataList OrderList;
extern HANDLE DispatchTdSem;

extern MessageList ScreenDisplayMsgList;
extern bool g_bQryOrderSentByRecoverDlg;
extern HANDLE ScreenDisplaySem;
extern CLockVariable gLockVariable;
extern char SgitTradingDay[];
extern map<int, int> OrderLocalRefToShmIndex;
extern SRWLOCK g_srwLockOrderLocalRef;

bool SgitIsFlowControl(int iResult)
{
	return ((iResult == -2) || (iResult == -3));
}

SgitTraderSpi::SgitTraderSpi(fstech::CThostFtdcTraderApi* xTraderApi, char xBROKER_ID[20], char xINVESTOR_ID[20], char xPASSWORD[20])
{
	mTraderApi = xTraderApi;
	strcpy(mBROKER_ID, xBROKER_ID);
	strcpy(mINVESTOR_ID, xINVESTOR_ID);
	strcpy(mPASSWORD, xPASSWORD);
}
SgitTraderSpi::~SgitTraderSpi(void)
{
}

void SgitTraderSpi::OnFrontConnected()
{
	CString str("Sgit交易服务器连接成功:");

	CTime mCurrTime = CTime::GetCurrentTime();
	CString str_mCurrTime = mCurrTime.Format("%Y-%m-%d %X");
	int len = WideCharToMultiByte(CP_ACP, 0, str_mCurrTime, str_mCurrTime.GetLength(), NULL, 0, NULL, NULL);
	char* c_str_mCurrTime = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, str_mCurrTime, str_mCurrTime.GetLength(), c_str_mCurrTime, len, NULL, NULL);
	c_str_mCurrTime[len] = '\0';

	CString csTime(c_str_mCurrTime);
	str.Append(csTime);

	free(c_str_mCurrTime);
	pPubMsg->AddString(str);
	ReqUserLogin();
}
void SgitTraderSpi::ReqUserLogin()
{
	fstech::CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy_s(req.BrokerID, mBROKER_ID);
	strcpy_s(req.UserID, mINVESTOR_ID);
	strcpy_s(req.Password, mPASSWORD);
	int iResult = mTraderApi->ReqUserLogin(&req, ++g_iRequestIDThost);
	CString str1(((iResult == 0) ? "Sgit交易服务器登陆请求发送成功" : "Sgit交易服务器登陆请求发送失败"));
	pPubMsg->AddString(str1);
}

void SgitTraderSpi::OnRspUserLogin(fstech::CThostFtdcRspUserLoginField* pRspUserLogin,
	fstech::CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	CString str("Sgit交易服务器登陆成功");
	pPubMsg->AddString(str);

	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		// 保存会话参数
		mFRONT_ID = pRspUserLogin->FrontID;
		mSESSION_ID = pRspUserLogin->SessionID;
		int iNextOrderRefx = atoi(pRspUserLogin->MaxOrderRef);
		iNextOrderRefx++;
		gLockVariable.setThostNextOrderRef(max(iNextOrderRefx, gLockVariable.getThostNextOrderRef() + 1));
		//sprintf_s(MAX_ORDER_REF, "%d", iNextOrderRef);
		///投资者结算结果确认
		CString str1("当前交易日 ");
		CString str2(mTraderApi->GetTradingDay());
		CString str3(str1 + str2);
		CString csMaxRef("");
		csMaxRef.Format(_T(" MaxRef=%d"), gLockVariable.getThostNextOrderRef());
		str3.Append(csMaxRef);
		pPubMsg->AddString(str3);
		strcpy(SgitTradingDay, mTraderApi->GetTradingDay());
		ReqSettlementInfoConfirm();
	}
}

bool SgitTraderSpi::IsErrorRspInfo(fstech::CThostFtdcRspInfoField* pRspInfo)
{
	// 如果ErrorID != 0, 说明收到了错误的响应
	bool bResult = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	CString str("Sgit返回错误:");
	CString str1("Sgit返回正确");
	if (bResult) {
		//cerr << "--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << endl;
		pPubMsg->AddString(str);
		CString errormsg(pRspInfo->ErrorMsg);
		pPubMsg->AddString(errormsg);
	}
	//else
	//	pPubMsg->AddString(str1);
	return bResult;
}
void SgitTraderSpi::OnRtnOrder(fstech::CThostFtdcOrderField* pOrder)
{
	CTime mCurrTime = CTime::GetCurrentTime();
	int nHourT, nMinT, nSecT;
	CString str_mCurrTime = mCurrTime.Format("%X");
	int len = WideCharToMultiByte(CP_ACP, 0, str_mCurrTime, str_mCurrTime.GetLength(), NULL, 0, NULL, NULL);
	char* c_str_mCurrTime = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, str_mCurrTime, str_mCurrTime.GetLength(), c_str_mCurrTime, len, NULL, NULL);
	c_str_mCurrTime[len] = '\0';
	sscanf_s(c_str_mCurrTime, "%d:%d:%d", &nHourT, &nMinT, &nSecT);
	if ((nHourT == 15 && nMinT >= 15) || (nHourT > 15 && nHourT < 20))return;
	delete c_str_mCurrTime;

	if (pOrder->OrderStatus == THOST_FTDC_OST_NoTradeNotQueueing || pOrder->OrderStatus == THOST_FTDC_OST_NoTradeQueueing || pOrder->OrderStatus == THOST_FTDC_OST_AllTraded
		|| pOrder->OrderStatus == THOST_FTDC_OST_PartTradedQueueing || pOrder->OrderStatus == THOST_FTDC_OST_Canceled) {
		OrderTradeMsg order;
		order.OrderSysId = atoi(pOrder->OrderSysID);
		order.OrderLocalRef = atoi(pOrder->OrderRef);
		order.OrderType = ON_THOST_RTN_ORDER;
		if (pOrder->OrderStatus == THOST_FTDC_OST_NoTradeNotQueueing) {
			order.OrderStatus = MORDER_ACCEPTED;
		}
		else if (pOrder->OrderStatus == THOST_FTDC_OST_NoTradeQueueing) {
			order.OrderStatus = MORDER_QUEUED;
		}
		else if (pOrder->OrderStatus == THOST_FTDC_OST_PartTradedQueueing) {
			order.OrderStatus = MORDER_PART_TRADED;
		}
		else if (pOrder->OrderStatus == THOST_FTDC_OST_AllTraded) {
			order.OrderStatus = MORDER_FULL_TRADED;
		}
		else if (pOrder->OrderStatus == THOST_FTDC_OST_Canceled) {
			order.OrderStatus = MORDER_CANCELLED;
		}
		order.LimitPrice = pOrder->LimitPrice;
		order.VolumeTotalOriginal = pOrder->VolumeTotalOriginal;
		order.VolumeTraded = pOrder->VolumeTraded;
		order.VolumeTotal = pOrder->VolumeTotalOriginal - pOrder->VolumeTraded;

		string strInsertDateTime(pOrder->InsertDate);
		strInsertDateTime.append(" ");
		strInsertDateTime.append(pOrder->InsertTime);

		strcpy(order.InsertOrTradeTime, strInsertDateTime.c_str());

		OrderList.AddTail(order);
		ReleaseSemaphore(DispatchTdSem, 2, NULL);
	}
}
void SgitTraderSpi::OnRtnTrade(fstech::CThostFtdcTradeField* pTrade)
{
	OrderTradeMsg order;
	order.OrderSysId = atoi(pTrade->OrderSysID);
	order.OrderLocalRef = atoi(pTrade->OrderRef);
	order.OrderType = ON_THOST_RTN_TRADE;

	order.Price = pTrade->Price;
	order.Volume = pTrade->Volume;
	order.MatchFee = 0;

	string strTradeDateTime(pTrade->TradingDay);
	strTradeDateTime.append(" ");
	strTradeDateTime.append(pTrade->TradeTime);
	strcpy(order.InsertOrTradeTime, strTradeDateTime.c_str());

	strcpy(order.MatchNo, pTrade->TradeID);
	OrderList.AddTail(order);
	ReleaseSemaphore(DispatchTdSem, 2, NULL);
}
bool SgitTraderSpi::IsMyOrder(fstech::CThostFtdcOrderField* pOrder)
{
	return ((pOrder->FrontID == mFRONT_ID) &&
		(pOrder->SessionID == mSESSION_ID));
}

bool SgitTraderSpi::IsTradingOrder(fstech::CThostFtdcOrderField* pOrder)
{
	return ((pOrder->OrderStatus != THOST_FTDC_OST_PartTradedNotQueueing) &&
		(pOrder->OrderStatus != THOST_FTDC_OST_Canceled) &&
		(pOrder->OrderStatus != THOST_FTDC_OST_AllTraded));
}

void SgitTraderSpi::ReqSettlementInfoConfirm()
{
	fstech::CThostFtdcSettlementInfoConfirmField req;
	memset(&req, 0, sizeof(req));
	strcpy_s(req.BrokerID, mBROKER_ID);
	strcpy_s(req.InvestorID, mINVESTOR_ID);
	int iResult = mTraderApi->ReqSettlementInfoConfirm(&req, ++g_iRequestIDThost);
	CString str("Sgit投资者结算结果确认");
	pPubMsg->AddString(str);
}

void SgitTraderSpi::OnRspSettlementInfoConfirm(fstech::CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, fstech::CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	//	memcpy(pStrategy->pSettlementInfoConfirm,pSettlementInfoConfirm,sizeof(CThostFtdcSettlementInfoConfirmField));
	//	pStrategyThread->PostThreadMessage(WM_RspSettlementInfoCon

		//pObserver->OnRspSettlementInfoConfirm(pSettlementInfoConfirm,pRspInfo,nRequestID,bIsLast);
		//cerr << "--->>> " << "OnRspSettlementInfoConfirm" << endl;
	CString str("Sgit投资者结算结果确认成功");
	pPubMsg->AddString(str);
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		///请求查询账户
		ReqQryTradingAccount();
	}
}

void SgitTraderSpi::OnRspQryInstrument(fstech::CThostFtdcInstrumentField* pInstrument, fstech::CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	CString str("Sgit查询合约成功");
	pPubMsg->AddString(str);
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		///请求查询
	}
}

void SgitTraderSpi::ReqQryTradingAccount()
{
	fstech::CThostFtdcQryTradingAccountField req;
	memset(&req, 0, sizeof(req));
	strcpy_s(req.BrokerID, mBROKER_ID);
	strcpy_s(req.InvestorID, mINVESTOR_ID);
	while (true)
	{
		int iResult = mTraderApi->ReqQryTradingAccount(&req, ++g_iRequestIDThost);
		if (!SgitIsFlowControl(iResult))
		{
			CString str("Sgit查询资金账户成功....等待回复");
			pPubMsg->AddString(str);
			break;
		}
		else
		{
			CString str("Sgit查询资金账户流控.....等待中");
			Sleep(1000);
		}
	} // while
}
void SgitTraderSpi::OnRspQryTradingAccount(fstech::CThostFtdcTradingAccountField* pTradingAccount, fstech::CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	/*
	IsErrorRspInfo(pRspInfo);
	double money=0;
	if(pTradingAccount==0)
	{
		CString str("Sgit查询资金账户返回错误");
		pPubMsg->AddString(str);
		CString str1(pRspInfo->ErrorMsg);
		pPubMsg->AddString(str1);
	}
	else
	{
		money=pTradingAccount->Available;
		CString str1;
		str1.Format(_T("%.3f"),money);
		CString str2("Sgit可用 ");
		CString str3(str2+str1);
		pPubMsg->AddString(str3);

		char pm[20];
		sprintf_s(pm,"%.3f",pTradingAccount->Available);
		CString money(pm);
		pMoneyCTP->SetWindowTextW(money);

		char tb[20];
		sprintf_s(tb,"%.3f",pTradingAccount->Balance);
		CString balance(tb);
		pBalanceCTP->SetWindowTextW(balance);

		CString csLoginInvestor(mINVESTOR_ID);
		pUsernameCTP->SetWindowTextW(csLoginInvestor);
	}
	*/
}

void SgitTraderSpi::OnRspOrderInsert(fstech::CThostFtdcInputOrderField* pInputOrder, fstech::CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo != NULL && pRspInfo->ErrorID != 0) {
		CString str("OnRspOrderInsert,Error:");
		CString str1(pRspInfo->ErrorMsg);
		str.Append(str1);
		pPubMsg->AddString(str);
	}
}

void SgitTraderSpi::OnRspOrderAction(fstech::CThostFtdcInputOrderActionField* pInputOrderAction, fstech::CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo != NULL && pRspInfo->ErrorID != 0) {
		CString str("OnRspOrderAction,Error:");
		CString str1(pRspInfo->ErrorMsg);
		str.Append(str1);
		pPubMsg->AddString(str);
	}
}

void SgitTraderSpi::OnRspQryOrder(fstech::CThostFtdcOrderField* pOrder, fstech::CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	IsErrorRspInfo(pRspInfo);

	if ((pRspInfo == NULL || (pRspInfo != NULL && pRspInfo->ErrorID == 0)) && pOrder != NULL && pOrder->OrderStatus != THOST_FTDC_OST_AllTraded) {
		OrderDetailField mOrder;
		strcpy(mOrder.CommodityNo, pOrder->InstrumentID);
		strcpy(mOrder.InstrumentID, pOrder->InstrumentID);
		mOrder.OrderId = atoi(pOrder->OrderSysID);

		string strInsertDateTime(pOrder->InsertDate);
		strInsertDateTime.append(" ");
		strInsertDateTime.append(pOrder->InsertTime);
		strcpy(mOrder.InsertDateTime, strInsertDateTime.c_str());
		if (pOrder->Direction == THOST_FTDC_D_Buy) {
			mOrder.Direction = MORDER_BUY;
		}
		else if (pOrder->Direction == THOST_FTDC_D_Sell) {
			mOrder.Direction = MORDER_SELL;
		}
		if (pOrder->CombOffsetFlag[0] == THOST_FTDC_OF_Open) {
			mOrder.Offset = MORDER_OPEN;
		}
		else {
			mOrder.Offset = MORDER_CLOSE;
		}

		if (pOrder->OrderStatus == THOST_FTDC_OST_NoTradeNotQueueing) {
			mOrder.OrderStatus = MORDER_ACCEPTED;
		}
		else if (pOrder->OrderStatus == THOST_FTDC_OST_NoTradeQueueing) {
			mOrder.OrderStatus = MORDER_QUEUED;
		}
		else if (pOrder->OrderStatus == THOST_FTDC_OST_PartTradedQueueing) {
			mOrder.OrderStatus = MORDER_PART_TRADED;
		}
		else if (pOrder->OrderStatus == THOST_FTDC_OST_AllTraded) {
			mOrder.OrderStatus = MORDER_FULL_TRADED;
		}
		else if (pOrder->OrderStatus == THOST_FTDC_OST_Canceled) {
			mOrder.OrderStatus = MORDER_CANCELLED;
		}
		else {
			mOrder.OrderStatus = MORDER_STATE_OTHER;
		}

		mOrder.SubmitPrice = pOrder->LimitPrice;
		mOrder.VolumeTotalOriginal = pOrder->VolumeTotalOriginal;
		mOrder.TradePrice = mOrder.SubmitPrice;
		mOrder.VolumeTraded = pOrder->VolumeTraded;

		mOrder.OrderLocalRef = atoi(pOrder->OrderRef);
		mOrder.FrontID = pOrder->FrontID;
		mOrder.SessionID = pOrder->SessionID;

		Message pOrderMsg;

		if (g_bQryOrderSentByRecoverDlg) {
			pOrderMsg.type = ON_RECOVER_QRY_ORDER;
		}
		else pOrderMsg.type = ON_RSP_QRY_ORDER;

		pOrderMsg.AddData(&mOrder, 0, sizeof(OrderDetailField));
		ScreenDisplayMsgList.AddTail(pOrderMsg);

		ReleaseSemaphore(ScreenDisplaySem, 1, NULL);
	}
}

void SgitTraderSpi::OnRspQryTrade(fstech::CThostFtdcTradeField* pTrade, fstech::CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	if ((pRspInfo == NULL || (pRspInfo != NULL && pRspInfo->ErrorID == 0)) && pTrade != NULL) {
		OrderTradeMsg order;
		order.OrderSysId = atoi(pTrade->OrderSysID);//与上述的ActionLocalNo不同，这是交易API生成的号码
		order.OrderType = ON_RSP_QRY_TRADE;

		order.Price = pTrade->Price;
		order.Volume = pTrade->Volume;
		order.MatchFee = 0;

		string strInsertDateTime(pTrade->TradeDate);
		strInsertDateTime.append(" ");
		strInsertDateTime.append(pTrade->TradeTime);
		strcpy(order.InsertOrTradeTime, strInsertDateTime.c_str());

		strcpy(order.MatchNo, pTrade->TradeID);
		OrderList.AddTail(order);
		ReleaseSemaphore(DispatchTdSem, 2, NULL);
	}
}

void SgitTraderSpi::OnRspQryInvestorPositionDetail(fstech::CThostFtdcInvestorPositionDetailField* pInvestorPositioDetail, fstech::CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	IsErrorRspInfo(pRspInfo);
	if ((pRspInfo == NULL || (pRspInfo != NULL && pRspInfo->ErrorID == 0)) && pInvestorPositioDetail != NULL) {
		if (pInvestorPositioDetail->Volume > 0) {
			PositionDetailField position;
			strcpy(position.CommodityNo, pInvestorPositioDetail->InstrumentID);
			strcpy(position.InstrumentID, pInvestorPositioDetail->InstrumentID);
			if (pInvestorPositioDetail->Direction == THOST_FTDC_D_Buy) {
				position.Direction = MORDER_BUY;
			}
			else if (pInvestorPositioDetail->Direction == THOST_FTDC_D_Sell) {
				position.Direction = MORDER_SELL;
			}
			strcpy(position.TradeDateTime, pInvestorPositioDetail->OpenDate);
			position.TradePrice = pInvestorPositioDetail->OpenPrice;
			position.TradeVol = pInvestorPositioDetail->Volume;
			strcpy(position.MatchNo, pInvestorPositioDetail->TradeID);
			Message posiMsg;
			posiMsg.type = ON_RSP_QRY_POSITION;
			posiMsg.AddData(&position, 0, sizeof(PositionDetailField));
			ScreenDisplayMsgList.AddTail(posiMsg);

			ReleaseSemaphore(ScreenDisplaySem, 2, NULL);
		}
	}
}

int SgitTraderSpi::ReqOrderDelete(OrderDetailField* pOrderField)
{
	fstech::CThostFtdcInputOrderActionField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, mBROKER_ID);
	strcpy(req.InvestorID, mINVESTOR_ID);
	strcpy(req.UserID, mINVESTOR_ID);
	gLockVariable.setThostNextOrderRef(gLockVariable.getThostNextOrderRef() + 1);

	//sprintf(req.OrderRef,"%d",gLockVariable.getThostNextOrderRef());
	req.OrderActionRef = gLockVariable.getThostNextOrderRef();

	sprintf(req.OrderSysID, "%012ld", pOrderField->OrderId);
	req.FrontID = pOrderField->FrontID;
	req.SessionID = pOrderField->SessionID;
	sprintf(req.OrderRef, "%012ld", pOrderField->OrderLocalRef);
	req.ActionFlag = THOST_FTDC_AF_Delete;
	strcpy(req.InstrumentID, pOrderField->InstrumentID);
	//strcpy(req.ExchangeID,pOrderField->e);
	int iResult = mTraderApi->ReqOrderAction(&req, ++g_iRequestIDThost);
	return iResult;
}

int SgitTraderSpi::ReqOrderDeletePerOrderLocalRef(int x_dOrderLocalRef, char x_cInstrumentID[30], int x_FrontID, int x_SessionID)
{
	fstech::CThostFtdcInputOrderActionField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, mBROKER_ID);
	strcpy(req.InvestorID, mINVESTOR_ID);
	strcpy(req.UserID, mINVESTOR_ID);
	gLockVariable.setThostNextOrderRef(gLockVariable.getThostNextOrderRef() + 1);

	//sprintf(req.OrderRef,"%d",gLockVariable.getThostNextOrderRef());
	req.OrderActionRef = gLockVariable.getThostNextOrderRef();

	//sprintf(req.OrderSysID,"%d",pOrderField->OrderId);
	req.FrontID = x_FrontID;
	req.SessionID = x_SessionID;
	sprintf(req.OrderRef, "%012ld", x_dOrderLocalRef);
	req.ActionFlag = THOST_FTDC_AF_Delete;
	strcpy(req.InstrumentID, x_cInstrumentID);
	//strcpy(req.ExchangeID,x_cInstrumentID);
	int iResult = mTraderApi->ReqOrderAction(&req, ++g_iRequestIDThost);
	return iResult;
}

int SgitTraderSpi::ReqOrderInsert(InsertOrderField* pReq, bool x_bCrossTradingDay, bool CloseToday, int xshmindex)  //0 open 1 close,0 buy 1 sell
{
	fstech::CThostFtdcInputOrderField req;
	memset(&req, 0, sizeof(req));
	///经纪公司代码
	strcpy_s(req.BrokerID, mBROKER_ID);
	///投资者代码
	strcpy_s(req.InvestorID, mINVESTOR_ID);
	strcpy_s(req.UserID, mINVESTOR_ID);
	///合约代码
	strcpy_s(req.InstrumentID, pReq->InstrumentID);
	///报单引用
	gLockVariable.setThostNextOrderRef(gLockVariable.getThostNextOrderRef() + 1);
	pReq->OrderLocalRef = gLockVariable.getThostNextOrderRef();
	sprintf_s(req.OrderRef, "%012ld", pReq->OrderLocalRef);
	if (xshmindex >= 0) {
		AcquireSRWLockExclusive(&g_srwLockOrderLocalRef);
		OrderLocalRefToShmIndex.insert(std::pair<int, int>(pReq->OrderLocalRef, xshmindex));
		ReleaseSRWLockExclusive(&g_srwLockOrderLocalRef);
	}
	if (pReq->Offset == MORDER_OPEN)   //如果是开仓的话
	{
		req.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
	}
	else if (pReq->Offset == MORDER_CLOSE)   //如果是平仓的话
	{
		if (CloseToday)req.CombOffsetFlag[0] = THOST_FTDC_OF_CloseToday;//平今THOST_FTDC_OF_CloseToday，其他交易所不区分
		else req.CombOffsetFlag[0] = THOST_FTDC_OF_Close;
	}

	///买卖方向:
	if (pReq->Direction == MORDER_BUY)
	{
		req.Direction = THOST_FTDC_D_Buy;
	}
	else if (pReq->Direction == MORDER_SELL)
	{
		req.Direction = THOST_FTDC_D_Sell;
	}
	///用户代码
//	TThostFtdcUserIDType	UserID;
	///报单价格条件: 限价
	req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;

	///组合投机套保标志
	req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
	///价格
	req.LimitPrice = pReq->OrderPrice;
	///数量: 1
	req.VolumeTotalOriginal = pReq->OrderVol;
	///有效期类型: 当日有效
	if (x_bCrossTradingDay) {
		//取消前有效
		req.TimeCondition = THOST_FTDC_TC_GTC;
	}
	else {
		//当日有效
		req.TimeCondition = THOST_FTDC_TC_GFD;
	}
	//	TThostFtdcDateType	GTDDate;
		///成交量类型: 任何数量
	req.VolumeCondition = THOST_FTDC_VC_AV;
	///最小成交量: 1
	req.MinVolume = 1;
	///触发条件: 立即
	req.ContingentCondition = THOST_FTDC_CC_Immediately;
	///止损价
//	TThostFtdcPriceType	StopPrice;
	///强平原因: 非强平
	req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
	///自动挂起标志: 否
	req.IsAutoSuspend = 0;
	///业务单元
//	TThostFtdcBusinessUnitType	BusinessUnit;
	///请求编号
//	TThostFtdcRequestIDType	RequestID;
	///用户强评标志: 否
	req.UserForceClose = 0;

	//	TRACE("Order Insert!! openOrClose=%i,buyOrSell=%i,price=%f,ref=%i,type=%d \n",openOrClose,buyOrSell,LimitPrice,iNextOrderRef,openThostOrder.StrategyType);

	if (req.LimitPrice == 0) //如果是市价单
	{
		req.OrderPriceType = THOST_FTDC_OPT_AnyPrice;
		req.TimeCondition = THOST_FTDC_TC_IOC;
	}

	int iResult = mTraderApi->ReqOrderInsert(&req, ++g_iRequestIDThost);

	pReq->FrontID = mFRONT_ID;
	pReq->SessionID = mSESSION_ID;

	return iResult;
}

void SgitTraderSpi::ReqQryOrder(char ExchangeNo[30], char CommodityNo[30], char InstrumentID[30])
{
	fstech::CThostFtdcQryOrderField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, mBROKER_ID);
	strcpy(req.InvestorID, mINVESTOR_ID);
	strcpy(req.ExchangeID, ExchangeNo);
	strcpy(req.InstrumentID, InstrumentID);
	int ret = mTraderApi->ReqQryOrder(&req, ++g_iRequestIDThost);
}

void SgitTraderSpi::ReqQryPosition(char ExchangeNo[30], char CommodityNo[30], char InstrumentID[30])
{
	fstech::CThostFtdcQryInvestorPositionDetailField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, mBROKER_ID);
	strcpy(req.InvestorID, mINVESTOR_ID);
	strcpy(req.InstrumentID, InstrumentID);
	int reqCount = 0;
	while (true)
	{
		int ret = mTraderApi->ReqQryInvestorPositionDetail(&req, ++g_iRequestIDThost);
		if (!SgitIsFlowControl(ret)) {
			CString str(((ret == 0) ? "CTP请求查询持仓成功" : "CTP请求查询持仓失败"));
			pPubMsg->AddString(str);
			break;
		}
		else {
			CString str(_T("请求查询持仓收到流控,Ret:"));
			CString strret("");
			strret.Format(_T("%d"), ret);
			str.Append(strret);
			pPubMsg->AddString(str);
			Sleep(1000);
			reqCount++;
			if (reqCount >= 3)break;
		}
	}
}

void SgitTraderSpi::ReqQryTrade(char ExchangeNo[30], char CommodityNo[30], char InstrumentID[30], char BeginMatchDateTime[21])//格式 hh:nn:ss
{
	fstech::CThostFtdcQryTradeField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, mBROKER_ID);
	strcpy(req.InvestorID, mINVESTOR_ID);
	strcpy(req.InstrumentID, InstrumentID);
	strcpy(req.TradeTimeStart, "09:00:00");
	int ret = mTraderApi->ReqQryTrade(&req, ++g_iRequestIDThost);
}