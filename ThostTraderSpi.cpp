#pragma once
#include "StdAfx.h"
#include "ThostTraderSpi.h"
#include "MyStruct.h"
#include "OrderDataList.h"
#include "MessageList.h"
#include "LockVariable.h"
#include <map>
#include "GlobalFunc.h"
#include "windows.h"

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
extern HANDLE RecoverStrategyDlgSem;

extern MessageList LogMessageList;
extern HANDLE logSemaphore;
extern char CTPTradingDay[];
extern map<int, int> OrderLocalRefToShmIndex;
extern SRWLOCK g_srwLockOrderLocalRef;
extern GlobalFunc globalFuncUtil;
extern int TotalCancelTimes;

extern int gBuyPosition;
extern int gSellPosition;
extern map<string, Posi> gPosiMap;

bool ThostIsFlowControl(int iResult)
{
	return ((iResult == -2) || (iResult == -3));
}

ThostTraderSpi::ThostTraderSpi(CThostFtdcTraderApi* xTraderApi, char xBROKER_ID[20], char xINVESTOR_ID[20], char xPASSWORD[20])
{
	mTraderApi = xTraderApi;
	strcpy(mBROKER_ID, xBROKER_ID);
	strcpy(mINVESTOR_ID, xINVESTOR_ID);
	strcpy(mPASSWORD, xPASSWORD);
}
ThostTraderSpi::~ThostTraderSpi(void)
{
}

void ThostTraderSpi::Release()
{
	mTraderApi->Release();
	CString str("CTP退出交易服务器");
	pPubMsg->AddString(str);
}

void ThostTraderSpi::OnFrontConnected()
{
	CString str("CTP交易服务器连接成功:");

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
#ifdef _TEST
	char m_appId[11], m_userId[16], m_AuthCode[40], m_AppID[33];
	strcpy(m_appId, "8888");
	strcpy(m_userId, "932535");
	strcpy(m_AuthCode, "EKPAU690P3W36I06");
	strcpy(m_AppID, "client_CTPLinux_1.0.1");
	ReqAuthenticate(m_appId, m_userId, m_AuthCode, m_AppID);
#else
	ReqUserLogin();
#endif
}

void ThostTraderSpi::ReqAuthenticate(TThostFtdcBrokerIDType	appId,
	TThostFtdcUserIDType	userId, TThostFtdcAuthCodeType	AuthCode, TThostFtdcAppIDType	AppID)
{
	CThostFtdcReqAuthenticateField field;
	memset(&field, 0, sizeof(field));
	strcpy(field.BrokerID, appId);
	strcpy(field.UserID, userId);

	strcpy(field.AuthCode, AuthCode);
	strcpy(field.AppID, AppID);
	mTraderApi->ReqAuthenticate(&field, 5);
}

///客户端认证响应
void ThostTraderSpi::OnRspAuthenticate(CThostFtdcRspAuthenticateField* pRspAuthenticateField, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	printf("OnRspAuthenticate\n");
	if (pRspInfo != NULL && pRspInfo->ErrorID == 0)
	{
		printf("认证成功,ErrorID=0x%04x, ErrMsg=%s\n\n", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
		ReqUserLogin();
	}
	else
	{
		cout << "认证失败，" << "ErrorID=" << pRspInfo->ErrorID << "  ,ErrMsg=" << pRspInfo->ErrorMsg << endl;
	}
};
void ThostTraderSpi::ReqUserLogin()
{
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy_s(req.BrokerID, mBROKER_ID);
	strcpy_s(req.UserID, mINVESTOR_ID);
	strcpy_s(req.Password, mPASSWORD);
	int iResult = mTraderApi->ReqUserLogin(&req, ++g_iRequestIDThost);
	CString str1(((iResult == 0) ? "CTP交易服务器登陆请求发送成功" : "CTP交易服务器登陆请求发送失败"));
	pPubMsg->AddString(str1);
}

void ThostTraderSpi::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
	CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	CString str("CTP交易服务器登陆成功");
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
		CString str1("CTP当前交易日 ");
		CString str2(mTraderApi->GetTradingDay());
		CString str3(str1 + str2);
		pPubMsg->AddString(str3);

		if (0 != strcmp(CTPTradingDay, mTraderApi->GetTradingDay())) TotalCancelTimes = 0;
		strcpy(CTPTradingDay, mTraderApi->GetTradingDay());
		ReqSettlementInfoConfirm();
		//ReqQryTradingAccount();
		//gBuyPosition=0;
		//gSellPosition=0;
		//for(map<string,Posi>::iterator itor=gPosiMap.begin();itor!=gPosiMap.end();++itor){
		//	itor->second.buyposition=0;
		//	itor->second.sellposition=0;
		//}
	}

	ReleaseSemaphore(RecoverStrategyDlgSem, 1, NULL);
}

bool ThostTraderSpi::IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo)
{
	// 如果ErrorID != 0, 说明收到了错误的响应
	bool bResult = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	CString str("CTP返回错误:");
	CString str1("CTP返回正确");
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
void ThostTraderSpi::OnRtnOrder(CThostFtdcOrderField* pOrder)
{
	if (atoi(pOrder->OrderRef) > 100000) return;

	//CTime mCurrTime=CTime::GetCurrentTime();
	//int nHourT,nMinT,nSecT;
	//CString str_mCurrTime=mCurrTime.Format("%X");
	//int len=WideCharToMultiByte(CP_ACP,0,str_mCurrTime,str_mCurrTime.GetLength(),NULL,0,NULL,NULL);
	//char *c_str_mCurrTime=new char[len+1];
	//WideCharToMultiByte(CP_ACP,0,str_mCurrTime,str_mCurrTime.GetLength(),c_str_mCurrTime,len,NULL,NULL);
	//c_str_mCurrTime[len]='\0';
	//sscanf_s(c_str_mCurrTime, "%d:%d:%d",&nHourT, &nMinT, &nSecT);
	//if((nHourT==15&&nMinT>=15)||(nHourT>15&&nHourT<=20))return;
	//delete c_str_mCurrTime;
	//
	if (pOrder->OrderStatus == THOST_FTDC_OST_NoTradeNotQueueing || pOrder->OrderStatus == THOST_FTDC_OST_NoTradeQueueing || pOrder->OrderStatus == THOST_FTDC_OST_AllTraded
		|| pOrder->OrderStatus == THOST_FTDC_OST_PartTradedQueueing || pOrder->OrderStatus == THOST_FTDC_OST_Canceled) {
		OrderTradeMsg order;
		order.OrderSysId = pOrder->SequenceNo;
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
			TotalCancelTimes++;
		}
		order.LimitPrice = pOrder->LimitPrice;
		order.VolumeTotalOriginal = pOrder->VolumeTotalOriginal;
		order.VolumeTraded = pOrder->VolumeTraded;
		order.VolumeTotal = pOrder->VolumeTotal;

		string strInsertDateTime(pOrder->InsertDate);
		strInsertDateTime.append(" ");
		strInsertDateTime.append(pOrder->InsertTime);

		strcpy(order.InsertOrTradeTime, strInsertDateTime.c_str());

		OrderList.AddTail(order);
		ReleaseSemaphore(DispatchTdSem, 2, NULL);
	}
}
void ThostTraderSpi::OnRtnTrade(CThostFtdcTradeField* pTrade)
{
	if (atoi(pTrade->OrderRef) > 100000) return;

	OrderTradeMsg order;
	//order.OrderSysId=pTrade->SequenceNo;
	char buff[50];
	int nDay = atoi(CTPTradingDay) % 100;
	sprintf_s(buff, "%d%d", nDay, pTrade->SequenceNo);
	order.OrderSysId = atoi(buff);
	order.OrderLocalRef = atoi(pTrade->OrderRef);
	order.OrderType = ON_THOST_RTN_TRADE;

	order.Price = pTrade->Price;
	order.Volume = pTrade->Volume;
	order.MatchFee = 0;
	order.OffFlag = pTrade->OffsetFlag;
	order.Direction = pTrade->Direction;

	string strTradeDateTime(pTrade->TradingDay);
	strTradeDateTime.append(" ");
	strTradeDateTime.append(pTrade->TradeTime);
	strcpy(order.InsertOrTradeTime, strTradeDateTime.c_str());

	strcpy(order.MatchNo, pTrade->TradeID);
	OrderList.AddTail(order);
	ReleaseSemaphore(DispatchTdSem, 2, NULL);
}
bool ThostTraderSpi::IsMyOrder(CThostFtdcOrderField* pOrder)
{
	return ((pOrder->FrontID == mFRONT_ID) &&
		(pOrder->SessionID == mSESSION_ID));
}

bool ThostTraderSpi::IsTradingOrder(CThostFtdcOrderField* pOrder)
{
	return ((pOrder->OrderStatus != THOST_FTDC_OST_PartTradedNotQueueing) &&
		(pOrder->OrderStatus != THOST_FTDC_OST_Canceled) &&
		(pOrder->OrderStatus != THOST_FTDC_OST_AllTraded));
}

void ThostTraderSpi::ReqSettlementInfoConfirm()
{
	CThostFtdcSettlementInfoConfirmField req;
	memset(&req, 0, sizeof(req));
	strcpy_s(req.BrokerID, mBROKER_ID);
	strcpy_s(req.InvestorID, mINVESTOR_ID);
	int iResult = mTraderApi->ReqSettlementInfoConfirm(&req, ++g_iRequestIDThost);
	CString str("CTP投资者结算结果确认");
	pPubMsg->AddString(str);
}

void ThostTraderSpi::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	//	memcpy(pStrategy->pSettlementInfoConfirm,pSettlementInfoConfirm,sizeof(CThostFtdcSettlementInfoConfirmField));
	//	pStrategyThread->PostThreadMessage(WM_RspSettlementInfoCon

		//pObserver->OnRspSettlementInfoConfirm(pSettlementInfoConfirm,pRspInfo,nRequestID,bIsLast);
		//cerr << "--->>> " << "OnRspSettlementInfoConfirm" << endl;
	CString str("CTP投资者结算结果确认成功");
	pPubMsg->AddString(str);
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		///请求查询账户
		ReqQryTradingAccount();
	}
}

void ThostTraderSpi::OnRspQryInstrument(CThostFtdcInstrumentField* pInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	CString str("CTP查询合约成功");
	pPubMsg->AddString(str);
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		///请求查询
	}
}

void ThostTraderSpi::ReqQryTradingAccount()
{
	CThostFtdcQryTradingAccountField req;
	memset(&req, 0, sizeof(req));
	strcpy_s(req.BrokerID, "");
	strcpy_s(req.InvestorID, "");
	while (true)
	{
		int iResult = mTraderApi->ReqQryTradingAccount(&req, ++g_iRequestIDThost);
		if (!ThostIsFlowControl(iResult))
		{
			CString str("CTP查询资金账户成功....等待回复");
			pPubMsg->AddString(str);
			break;
		}
		else
		{
			CString str("CTP查询资金账户流控.....等待中");
			Sleep(1000);
		}
	} // while
}
void ThostTraderSpi::OnRspQryTradingAccount(CThostFtdcTradingAccountField* pTradingAccount, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	IsErrorRspInfo(pRspInfo);
	double money = 0;
	if (pTradingAccount == 0)
	{
		CString str("CTP查询资金账户返回错误");
		pPubMsg->AddString(str);
		CString str1(pRspInfo->ErrorMsg);
		pPubMsg->AddString(str1);
	}
	else
	{
		money = pTradingAccount->Available;
		CString str1;
		str1.Format(_T("%.3f"), money);
		CString str2("CTP可用 ");
		CString str3(str2 + str1);
		pPubMsg->AddString(str3);

		char pm[20];
		sprintf_s(pm, "%.3f", pTradingAccount->Available);
		CString money(pm);
		pMoneyCTP->SetWindowTextW(money);

		char tb[20];
		sprintf_s(tb, "%.3f", pTradingAccount->Balance);
		CString balance(tb);
		pBalanceCTP->SetWindowTextW(balance);

		CString csLoginInvestor(mINVESTOR_ID);
		pUsernameCTP->SetWindowTextW(csLoginInvestor);

		m_Balance = pTradingAccount->Balance;

		// 记录日志
		char buff[200] = { 0 };
		sprintf_s(buff, "AccountID:%s,Balance:%.3f", pTradingAccount->AccountID, pTradingAccount->Balance);
		globalFuncUtil.WriteMsgToLogList(buff);
	}
	if (bIsLast) {
		gBuyPosition = 0;
		gSellPosition = 0;
		//for(map<string,Posi>::iterator itor=gPosiMap.begin();itor!=gPosiMap.end();++itor){
		//	itor->second.buyposition=0;
		//	itor->second.sellposition=0;
		//	itor->second.bBuy=false;
		//	itor->second.bSell=false;
		//}
		gPosiMap.clear();
		ReqQryPosition("", "", "");
	}
}

void ThostTraderSpi::OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo != NULL && pRspInfo->ErrorID != 0) {
		CString str("OnRspOrderInsert,Error:");
		CString str1(pRspInfo->ErrorMsg);
		str.Append(str1);
		pPubMsg->AddString(str);
	}
}

void ThostTraderSpi::OnRspOrderAction(CThostFtdcInputOrderActionField* pInputOrderAction, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo != NULL && pRspInfo->ErrorID != 0) {
		CString str("OnRspOrderAction,Error:");
		CString str1(pRspInfo->ErrorMsg);
		str.Append(str1);
		pPubMsg->AddString(str);
	}
}

void ThostTraderSpi::OnRspQryOrder(CThostFtdcOrderField* pOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	IsErrorRspInfo(pRspInfo);

	if ((pRspInfo == NULL || (pRspInfo != NULL && pRspInfo->ErrorID == 0)) && pOrder != NULL && pOrder->OrderStatus != THOST_FTDC_OST_AllTraded) {
		OrderDetailField mOrder;
		strcpy(mOrder.CommodityNo, pOrder->InstrumentID);
		strcpy(mOrder.InstrumentID, pOrder->InstrumentID);
		mOrder.OrderId = pOrder->SequenceNo;

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

void ThostTraderSpi::OnRspQryTrade(CThostFtdcTradeField* pTrade, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	if ((pRspInfo == NULL || (pRspInfo != NULL && pRspInfo->ErrorID == 0)) && pTrade != NULL) {
		OrderTradeMsg order;
		order.OrderSysId = pTrade->SequenceNo;//与上述的ActionLocalNo不同，这是交易API生成的号码
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

void ThostTraderSpi::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField* pInvestorPositioDetail, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	IsErrorRspInfo(pRspInfo);
	if ((pRspInfo == NULL || (pRspInfo != NULL && pRspInfo->ErrorID == 0)) && pInvestorPositioDetail != NULL) {
		if (pInvestorPositioDetail->Volume > 0) {
			PositionDetailField position;
			strcpy(position.CommodityNo, pInvestorPositioDetail->InstrumentID);
			strcpy(position.InstrumentID, pInvestorPositioDetail->InstrumentID);
			if (pInvestorPositioDetail->Direction == THOST_FTDC_D_Buy) {
				position.Direction = MORDER_BUY;
				string strIns(pInvestorPositioDetail->InstrumentID);
				if (0 != strcmp(pInvestorPositioDetail->OpenDate, pInvestorPositioDetail->TradingDay)) {		// 昨仓
					map<string, Posi>::iterator itor = gPosiMap.find(strIns);
					if (itor != gPosiMap.end() && !itor->second.bBuy) itor->second.buyposition += pInvestorPositioDetail->Volume;
					else {
						Posi p;
						memset(&p, 0, sizeof(Posi));
						p.buyposition = pInvestorPositioDetail->Volume;
						p.bBuy = false;
						gPosiMap.insert(map<string, Posi>::value_type(strIns, p));
					}
					//gBuyPosition+=pInvestorPositioDetail->Volume;
				}
				else {	// 今仓表示不能平仓
					map<string, Posi>::iterator itor = gPosiMap.find(strIns);
					if (itor != gPosiMap.end()) {
						itor->second.buyposition = 0;
						itor->second.bBuy = true;
					}
					else {
						Posi p;
						memset(&p, 0, sizeof(Posi));
						p.buyposition = 0;
						p.bBuy = true;
						gPosiMap.insert(map<string, Posi>::value_type(strIns, p));
					}
				}
			}
			else if (pInvestorPositioDetail->Direction == THOST_FTDC_D_Sell) {
				position.Direction = MORDER_SELL;
				string strIns(pInvestorPositioDetail->InstrumentID);
				if (0 != strcmp(pInvestorPositioDetail->OpenDate, pInvestorPositioDetail->TradingDay)) {
					map<string, Posi>::iterator itor = gPosiMap.find(strIns);
					if (itor != gPosiMap.end()) itor->second.sellposition += pInvestorPositioDetail->Volume;
					else {
						Posi p;
						memset(&p, 0, sizeof(Posi));
						p.sellposition = pInvestorPositioDetail->Volume;
						p.bSell = false;
						gPosiMap.insert(map<string, Posi>::value_type(strIns, p));
					}
					//gSellPosition+=pInvestorPositioDetail->Volume;
				}
				else {
					map<string, Posi>::iterator itor = gPosiMap.find(strIns);
					if (itor != gPosiMap.end()) {
						itor->second.sellposition = 0;
						itor->second.bSell = true;
					}
					else {
						Posi p;
						memset(&p, 0, sizeof(Posi));
						p.sellposition = 0;
						p.bSell = true;
						gPosiMap.insert(map<string, Posi>::value_type(strIns, p));
					}
				}
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

			char line[200];
			sprintf(line, "ClientNo:%s,Inst:%s,Direct:%d,TradePrice:%.2f,TradeVol:%d,SettlePrice:%.2f,%.2f,MatchDateTime:%s", mINVESTOR_ID, position.InstrumentID, position.Direction, position.TradePrice,
				position.TradeVol, pInvestorPositioDetail->LastSettlementPrice, pInvestorPositioDetail->PositionProfitByTrade, position.TradeDateTime);
			globalFuncUtil.WriteMsgToLogList(line);
		}
	}
}

int ThostTraderSpi::ReqOrderDelete(OrderDetailField* pOrderField)
{
	CThostFtdcInputOrderActionField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, mBROKER_ID);
	strcpy(req.InvestorID, mINVESTOR_ID);
	gLockVariable.setThostNextOrderRef(gLockVariable.getThostNextOrderRef() + 1);

	//sprintf(req.OrderRef,"%d",gLockVariable.getThostNextOrderRef());
	req.OrderActionRef = gLockVariable.getThostNextOrderRef();

	//sprintf(req.OrderSysID,"%d",pOrderField->OrderId);
	req.FrontID = pOrderField->FrontID;
	req.SessionID = pOrderField->SessionID;
	sprintf(req.OrderRef, "%d", pOrderField->OrderLocalRef);
	req.ActionFlag = THOST_FTDC_AF_Delete;
	strcpy(req.InstrumentID, pOrderField->InstrumentID);
	int iResult = mTraderApi->ReqOrderAction(&req, ++g_iRequestIDThost);
	return iResult;
}

int ThostTraderSpi::ReqOrderDeletePerOrderLocalRef(int x_dOrderLocalRef, char x_cInstrumentID[30], char x_cExchangeID[30], int x_FrontID, int x_SessionID)
{
	CThostFtdcInputOrderActionField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, mBROKER_ID);
	strcpy(req.InvestorID, mINVESTOR_ID);
	strcpy(req.ExchangeID, x_cExchangeID);
	gLockVariable.setThostNextOrderRef(gLockVariable.getThostNextOrderRef() + 1);

	//sprintf(req.OrderRef,"%d",gLockVariable.getThostNextOrderRef());
	req.OrderActionRef = gLockVariable.getThostNextOrderRef();

	//sprintf(req.OrderSysID,"%d",pOrderField->OrderId);
	req.FrontID = x_FrontID;
	req.SessionID = x_SessionID;
	sprintf(req.OrderRef, "%d", x_dOrderLocalRef);
	req.ActionFlag = THOST_FTDC_AF_Delete;
	strcpy(req.InstrumentID, x_cInstrumentID);
	int iResult = mTraderApi->ReqOrderAction(&req, ++g_iRequestIDThost);
	return iResult;
}

int ThostTraderSpi::ReqOrderInsert(InsertOrderField* pReq, bool x_bCrossTradingDay, bool CloseToday, int xshmindex)  //0 open 1 close,0 buy 1 sell
{
	CThostFtdcInputOrderField req;
	memset(&req, 0, sizeof(req));
	///经纪公司代码
	strcpy_s(req.BrokerID, mBROKER_ID);
	strcpy_s(req.ExchangeID, pReq->ExchangeID);
	///投资者代码
	strcpy_s(req.InvestorID, mINVESTOR_ID);
	///合约代码
	strcpy_s(req.InstrumentID, pReq->InstrumentID);
	///报单引用

	gLockVariable.setThostNextOrderRef(gLockVariable.getThostNextOrderRef() + 1);
	pReq->OrderLocalRef = gLockVariable.getThostNextOrderRef();
	sprintf_s(req.OrderRef, "%d", pReq->OrderLocalRef);
	pReq->FrontID = mFRONT_ID;
	pReq->SessionID = mSESSION_ID;

	if (xshmindex >= 0) {
		AcquireSRWLockExclusive(&g_srwLockOrderLocalRef);
		OrderLocalRefToShmIndex.insert(std::pair<int, int>(pReq->OrderLocalRef, xshmindex));
		ReleaseSRWLockExclusive(&g_srwLockOrderLocalRef);
	}
	if (pReq->Offset == MORDER_OPEN)   //如果是开仓的话
	{
		req.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
	}
	else if (pReq->Offset == MORDER_CLOSE || pReq->Offset == MORDER_CLOSETODAY)   //如果是平仓的话
	{
		if (CloseToday)req.CombOffsetFlag[0] = THOST_FTDC_OF_CloseToday;
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
	//if(x_bCrossTradingDay){
		 //取消前有效
	//	req.TimeCondition = THOST_FTDC_TC_GTC;
	//}else{
		//当日有效
	req.TimeCondition = THOST_FTDC_TC_GFD;
	//}
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

	char log[200];
	sprintf(log, "CTP Insert!! %d,openOrClose=%d,buyOrSell=%d,price=%.4f,vol=%d,inst=%s,iRet=%d", pReq->OrderLocalRef, pReq->Offset, pReq->Direction, pReq->OrderPrice, pReq->OrderVol, pReq->InstrumentID, iResult);
	WriteMsgToLogList(log);

	if (iResult == -1) {
		CString str("Req Order Insert Ret=-1.Error.");
		pPubMsg->AddString(str);
	}
	return iResult;
}

int ThostTraderSpi::ReqOrderInsertCffex(InsertOrderField* pReq, bool x_bCrossTradingDay, bool CloseToday, int xshmindex, int& yBuyPosition, int& ySellPosition)  //0 open 1 close,0 buy 1 sell
{
	CThostFtdcInputOrderField req;
	memset(&req, 0, sizeof(req));
	///经纪公司代码
	strcpy_s(req.BrokerID, mBROKER_ID);
	///投资者代码
	strcpy_s(req.InvestorID, mINVESTOR_ID);
	///合约代码
	strcpy_s(req.InstrumentID, pReq->InstrumentID);
	///报单引用

	gLockVariable.setThostNextOrderRef(gLockVariable.getThostNextOrderRef() + 1);
	pReq->OrderLocalRef = gLockVariable.getThostNextOrderRef();
	sprintf_s(req.OrderRef, "%d", pReq->OrderLocalRef);
	pReq->FrontID = mFRONT_ID;
	pReq->SessionID = mSESSION_ID;

	if (xshmindex >= 0) {
		AcquireSRWLockExclusive(&g_srwLockOrderLocalRef);
		OrderLocalRefToShmIndex.insert(std::pair<int, int>(pReq->OrderLocalRef, xshmindex));
		ReleaseSRWLockExclusive(&g_srwLockOrderLocalRef);
	}

	//if(pReq->Offset==MORDER_OPEN)   //如果是开仓的话
	//{
	//	req.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
	//}else if(pReq->Offset == MORDER_CLOSE||pReq->Offset == MORDER_CLOSETODAY)   //如果是平仓的话
	//{
	//	if(CloseToday)req.CombOffsetFlag[0] = THOST_FTDC_OF_CloseToday;
	//	else req.CombOffsetFlag[0] = THOST_FTDC_OF_Close;
	//}

	if (yBuyPosition > 0 && MORDER_SELL == pReq->Direction) {
		req.CombOffsetFlag[0] = THOST_FTDC_OF_Close;
		yBuyPosition--;
	}
	else if (ySellPosition > 0 && MORDER_BUY == pReq->Direction) {
		req.CombOffsetFlag[0] = THOST_FTDC_OF_Close;
		ySellPosition--;
	}
	else {
		req.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
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
	//if(x_bCrossTradingDay){
	//取消前有效
	//	req.TimeCondition = THOST_FTDC_TC_GTC;
	//}else{
	//当日有效
	req.TimeCondition = THOST_FTDC_TC_GFD;
	//}
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

	char log[200];
	sprintf(log, "CTP Insert!! %d,openOrClose=%d,buyOrSell=%d,price=%.4f,vol=%d,inst=%s,iRet=%d", pReq->OrderLocalRef, pReq->Offset, pReq->Direction, pReq->OrderPrice, pReq->OrderVol, pReq->InstrumentID, iResult);
	WriteMsgToLogList(log);

	if (iResult == -1) {
		CString str("Req Order Insert Ret=-1.Error.");
		pPubMsg->AddString(str);
	}
	return iResult;
}

void ThostTraderSpi::ReqQryOrder(char ExchangeNo[30], char CommodityNo[30], char InstrumentID[30])
{
	CThostFtdcQryOrderField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, mBROKER_ID);
	strcpy(req.InvestorID, mINVESTOR_ID);
	//	strcpy(req.ExchangeID,ExchangeNo);
	//	strcpy(req.InstrumentID,InstrumentID);
	int ret = mTraderApi->ReqQryOrder(&req, ++g_iRequestIDThost);
}

void ThostTraderSpi::ReqQryPosition(char ExchangeNo[30], char CommodityNo[30], char InstrumentID[30])
{
	CThostFtdcQryInvestorPositionDetailField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, mBROKER_ID);
	strcpy(req.InvestorID, mINVESTOR_ID);
	strcpy(req.InstrumentID, InstrumentID);
	int reqCount = 0;
	while (true)
	{
		int ret = mTraderApi->ReqQryInvestorPositionDetail(&req, ++g_iRequestIDThost);
		if (!ThostIsFlowControl(ret)) {
			CString str(((ret == 0) ? "CTP请求查询持仓成功" : "CTP请求查询持仓失败"));
			pPubMsg->AddString(str);
			break;
		}
		else {
			CString str(_T("CTP请求查询持仓收到流控,Ret:"));
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

void ThostTraderSpi::ReqQryTrade(char ExchangeNo[30], char CommodityNo[30], char InstrumentID[30], char BeginMatchDateTime[21])//格式 hh:nn:ss
{
	CThostFtdcQryTradeField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, mBROKER_ID);
	strcpy(req.InvestorID, mINVESTOR_ID);
	strcpy(req.InstrumentID, InstrumentID);
	strcpy(req.TradeTimeStart, "09:00:00");
	int ret = mTraderApi->ReqQryTrade(&req, ++g_iRequestIDThost);
}

void ThostTraderSpi::WriteMsgToLogList(char logline[200])
{
	Message logMsg;
	logMsg.type = STRATEGY_LOG;
	logMsg.AddData(logline, 0, sizeof(char) * 200);
	LogMessageList.AddTail(logMsg);
	ReleaseSemaphore(logSemaphore, 1, NULL);
}