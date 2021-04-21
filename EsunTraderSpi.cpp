#pragma once
#include "StdAfx.h"
#include "EsunTraderSpi.h"
#include "MyThread.h"
#include "MessageList.h"
#include "OrderDataList.h"
#include "MyStruct.h"
#include "Global.h"

using namespace std;

extern CListBox* pPubMsg;
extern CStatic* pDay;
extern CStatic* pMoney;
extern CStatic* pBalance;
extern CStatic* pUsername;

extern MessageList ScreenDisplayMsgList;
extern OrderDataList OrderList;

extern char LoginUserTDEsun[];
extern char LoginPwdTDEsun[];

extern int g_iRequestIDEsun;
extern char PASSWORD[];

extern char InstrumentID[];
extern CMyThread* pRestartTraderThread;
extern CMyThread* pTDConnLostQryTradeThread;
//extern map<int,int> OrderIdToShmIndex;
extern MapViewType* gMapView;
bool LoginSuccess = false;
bool OnOpenSuccess = false;
extern bool EsunTDReleasedAction;
extern bool g_bQryOrderSentByRecoverDlg;
extern HANDLE RecoverScreenDisplaySem;
extern char TDConnectionLostTime[];
extern HANDLE RecoverStrategyDlgSem;
extern GlobalFunc globalFuncUtil;
extern bool g_bQryMoney;

bool EsunIsFlowControl(int iResult)
{
	return ((iResult == -2) || (iResult == -3));
}

void EsunTraderSpi::Initialize()
{
	m_Api->SetSpi(this);
}

void EsunTraderSpi::Release()
{
	CString str("Esun Release 交易服务器");
	pPubMsg->AddString(str);
	EsunTDReleasedAction = true;
	m_Api->Free();

	char cLog[200];
	globalFuncUtil.ConvertCStringToCharArray(str, cLog);
	globalFuncUtil.WriteMsgToLogList(cLog);
}

void EsunTraderSpi::Close()
{
	m_Api->Close();
}

void __cdecl EsunTraderSpi::OnClose()
{
	m_nLoginState = STATE_DISCONNECTED;

	TDConnectionLost = true;
	LoginSuccess = false;
	OnOpenSuccess = false;
	m_bLoginSuccess = false;

	CTime mCurrTime = CTime::GetCurrentTime();
	CString str_mCurrTime = mCurrTime.Format("%Y-%m-%d %X");
	int len = WideCharToMultiByte(CP_ACP, 0, str_mCurrTime, str_mCurrTime.GetLength(), NULL, 0, NULL, NULL);
	char* c_str_mCurrTime = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, str_mCurrTime, str_mCurrTime.GetLength(), c_str_mCurrTime, len, NULL, NULL);
	c_str_mCurrTime[len] = '\0';
	strcpy(TDConnectionLostTime, c_str_mCurrTime);
	free(c_str_mCurrTime);

	CString str1("Esun交易服务器断开..");
	CString csTime(TDConnectionLostTime);
	str1.Append(csTime);
	pPubMsg->AddString(str1);
	if (!EsunTDReleasedAction)
	{
		pRestartTraderThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
		Sleep(100);
		pRestartTraderThread->PostThreadMessage(WM_RESTART_ESUN_TRADER, NULL, NULL);
	}
}

void EsunTraderSpi::OnOpen()
{
	OnOpenSuccess = true;
	m_bLoginSuccess = false;
	m_bLoginDone = false;

	m_iConnState = STATE_CONNECTED;
	m_nLoginState = STATE_CONNECTED;

	ESForeign::TEsLoginReqField native;
	memset(&native, 0, sizeof(ESForeign::TEsLoginReqField));

	native.IsCaLogin = ESForeign::NOT_CA_LOGIN;
	native.Identity = ESForeign::IDENTITY_CLIENT;	//client;
	//native.Identity = ESForeign::IDENTITY_TRADER;	//trader;
	//native.Identity = ESForeign::IDENTITY_TCLIENT;	//
	native.IsForcePwd = ESForeign::NOT_FORCE_PWD;
	strcpy(native.ClientNo, LoginUserTDEsun);
	strcpy(native.LoginPwd, LoginPwdTDEsun);
	int Req = 0;
	int ret = m_Api->Login(native, Req);
	CString str(((ret == 0) ? "Esun Trader OnOpen Success" : "Esun Trader OnOpen Failed"));
	pPubMsg->AddString(str);
}

void __cdecl EsunTraderSpi::OnLogin(const ESForeign::TEsLoginRspField* rsp, int errCode, const int iReqID)
{
	if (errCode != 0)
	{
		char data[500];
		sprintf_s(data, "Esun Trader OnLogin Error,errCode=%d", errCode);
		CString str(data);
		pPubMsg->AddString(str);
		m_bLoginSuccess = false;
	}
	else {
		char data[500];
		sprintf_s(data, "Esun Trader OnLogin Success");
		CString str(data);
		pPubMsg->AddString(str);
		m_bLoginSuccess = true;

		if (TDConnectionLost) {
			pTDConnLostQryTradeThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
			Sleep(100);
			pTDConnLostQryTradeThread->PostThreadMessage(WM_TDLOST_QRY_TRADES, NULL, NULL);
			TDConnectionLost = false;
		}
		//Sleep(3000);
		//ReqQryMoney();

//#ifdef _MONEY
//		pQryActionThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
//		Sleep(200);
//		pQryActionThread->PostThreadMessage(WM_QRY_MONEY,NULL,NULL);
//#endif
//#ifdef _POSITION
//		pQryActionThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
//		Sleep(200);
//		pQryActionThread->PostThreadMessage(WM_QRY_POSITION_DETAILS,NULL,NULL);
//#endif
	}
	m_bLoginDone = true;
	ReleaseSemaphore(RecoverStrategyDlgSem, 1, NULL);
	/*
	if(errCode!=0)
	{
	AccountState *p = new AccountState;
	strcpy(p->atype, "esun");
	strcpy(p->loginid, m_pTrader->loginid);
	sprintf(p->msg, "%s", m_Api->GetErrcodeDesc(errCode));
	char data[500];
	sprintf_s(data, "response=交易账户状态;content=");
	int len = strlen(data);
	memcpy(&data[len], p, sizeof(AccountState));
	len += sizeof(AccountState);
	theApp.SendUdpMessage(theApp.m_udpSock, "127.0.0.1", theApp.m_iPort, data, len);
	delete p;

	m_pTrader->m_nLoginState = 登录失败;
	}
	else
	{
	AccountState *p = new AccountState;
	strcpy(p->atype, "esun");
	strcpy(p->loginid, m_pTrader->loginid);
	strcpy(p->msg, "登录成功");
	char data[500];
	sprintf_s(data, "response=交易账户状态;content=");
	int len = strlen(data);
	memcpy(&data[len], p, sizeof(AccountState));
	len += sizeof(AccountState);
	theApp.SendUdpMessage(theApp.m_udpSock, "127.0.0.1", theApp.m_iPort, data, len);
	delete p;

	m_pTrader->m_nLoginState = 登录成功;
	}
	*/
}
/////////////

void EsunTraderSpi::OnRspQryCurrency(const ESForeign::TEsCurrencyQryRspField* rsp, ESForeign::TEsIsLastType islast, int errCode, const int iReqID)
{
	if (rsp != NULL && errCode == 0) {
		if (strcmp(rsp->CurrencyNo, "RMB") == 0) {
			m_RMBExchangeRate = rsp->ExchangeRate;
			TRACE("%s,%.4f\n", rsp->CurrencyNo, rsp->ExchangeRate);
		}
	}
	if (islast) {
		ESForeign::TEsMoneyQryReqField req;
		memset(&req, 0, sizeof(req));
		strcpy(req.ClientNo, LoginUserTDEsun);
		//g_iRequestID++;
		int iReqID = 0;
		int ret = m_Api->QryMoney(req, iReqID);
		//struct REQ *pNew = new REQ;
		//pNew->MyReq = g_iRequestID;
		//pNew->EsReq = iReqID;
		//strcpy(pNew->ClientID, req.ClientNo);
		//g_Req.push_back(pNew);

		//	ReqQryTrade("NYMEX","CL","1707","00:00:01");
		m_dAvailable = 0;
		m_Balance = 0;
	}
}

void EsunTraderSpi::ReqQryCurrency()
{
	ESForeign::TEsCurrencyQryReqField req;
	memset(&req, 0, sizeof(req));
	int iReqID = 0;
	int ret = m_Api->QryCurrency(req, iReqID);
}

void EsunTraderSpi::ReqQryMoney()
{
	m_RMBExchangeRate = 0.0;
	ReqQryCurrency();
}

void __cdecl EsunTraderSpi::OnQryMoney(const ESForeign::TEsMoneyQryRspField* rsp, ESForeign::TEsIsLastType islast, int errCode, const int iReqID)
{
	if (errCode != 0) {
		char data[500];
		sprintf_s(data, "Esun Response=查询资金返回错误,errorCode=%d", errCode);
		CString str(data);
		pPubMsg->AddString(str);
		return;
	}
	else {
		if (rsp != NULL && (strcmp(rsp->CurrencyNo, "USD") == 0 || strcmp(rsp->CurrencyNo, "RMB") == 0)) {
			if (strcmp(rsp->CurrencyNo, "RMB") == 0) {
				m_dAvailable += rsp->TAvailable * m_RMBExchangeRate;
				m_Balance += rsp->TBalance * m_RMBExchangeRate;
				m_AccountValue += rsp->AccountMarketValue * m_RMBExchangeRate;
			}
			else {
				m_dAvailable += rsp->TAvailable;
				m_Balance += rsp->TBalance;
				m_AccountValue += rsp->AccountMarketValue;
			}

			char data[500];
			sprintf_s(data, "Esun Response=查询资金返回;%s,AvailMoney=%.3f", rsp->CurrencyNo, rsp->TAvailable);
			CString str(data);
			pPubMsg->AddString(str);

			char pm[20];
			sprintf_s(pm, "%.3f", m_dAvailable);
			CString money(pm);
			pMoney->SetWindowTextW(money);

			char tb[20];
			sprintf_s(tb, "%.3f", m_Balance);
			CString balance(tb);
			pBalance->SetWindowTextW(balance);

			g_bQryMoney = true;
		}
#ifdef _MONEY
		if (islast) {
			char buff[200] = { 0 };
			sprintf_s(buff, "ClientNo:%s,AccountMarketValue:%.4f,ExchangeRate:%.4f", LoginUserTDEsun, m_AccountValue, 0 == m_RMBExchangeRate ? 1 : 1 / m_RMBExchangeRate);
			globalFuncUtil.WriteMsgToLogList(buff);
			m_AccountValue = 0;
		}
#endif
	}
}

void __cdecl EsunTraderSpi::OnRtnMoney(const ESForeign::TEsMoneyChgNoticeField& rsp)
{
}

void __cdecl EsunTraderSpi::OnQryHold(const ESForeign::TEsHoldQryRspField* rsp, ESForeign::TEsIsLastType islast, int errCode, const int iReqID)
{
	if (rsp != NULL && errCode == 0) {
		PositionDetailField position;
		strcpy(position.CommodityNo, rsp->CommodityNo);
		strcpy(position.InstrumentID, rsp->ContractNo);
		if (rsp->Direct == ESForeign::DIRECT_BUY) {
			position.Direction = MORDER_BUY;
		}
		else if (rsp->Direct == ESForeign::DIRECT_SELL) {
			position.Direction = MORDER_SELL;
		}
		strcpy(position.TradeDateTime, rsp->MatchDateTime);
		position.TradePrice = rsp->TradePrice;
		position.TradeVol = rsp->TradeVol;
		strcpy(position.MatchNo, rsp->MatchNo);
		Message posiMsg;
		posiMsg.type = ON_RSP_QRY_POSITION;
		posiMsg.AddData(&position, 0, sizeof(PositionDetailField));
		ScreenDisplayMsgList.AddTail(posiMsg);

		ReleaseSemaphore(ScreenDisplaySem, 2, NULL);
#ifdef _POSITION
		char buff[200] = { 0 };
		sprintf_s(buff, "ClientNo:%s,Inst:%s %s,Direct:%c,TradePrice:%.4f,TradeVol:%d,YSettlePrice:%.4f,TNewPrice:%.4f,MatchDateTime:%s,Deposit:%.4f",
			rsp->ClientNo, rsp->CommodityNo, rsp->ContractNo, rsp->Direct, rsp->TradePrice, rsp->TradeVol, rsp->YSettlePrice, rsp->TNewPrice, rsp->MatchDateTime,
			rsp->Deposit);
		globalFuncUtil.WriteMsgToLogList(buff);

#endif
	}
}

void __cdecl EsunTraderSpi::OnRspQryOrder(const ESForeign::TEsOrderDataQryRspField* rsp, ESForeign::TEsIsLastType islast, int errCode, const int iReqID)
{
	if (rsp != NULL && errCode == 0) {
		OrderDetailField pOrder;
		strcpy(pOrder.CommodityNo, rsp->ReqData.CommodityNo);
		strcpy(pOrder.InstrumentID, rsp->ReqData.ContractNo);
		pOrder.OrderId = rsp->OrderId;

		strcpy(pOrder.InsertDateTime, rsp->InsertDateTime);
		if (rsp->ReqData.Direct == ESForeign::DIRECT_BUY) {
			pOrder.Direction = MORDER_BUY;
		}
		else if (rsp->ReqData.Direct == ESForeign::DIRECT_SELL) {
			pOrder.Direction = MORDER_SELL;
		}
		if (rsp->ReqData.Offset == ESForeign::OFFSET_OPEN) {
			pOrder.Offset = MORDER_OPEN;
		}
		else if (rsp->ReqData.Offset == ESForeign::OFFSET_COVER || rsp->ReqData.Offset == ESForeign::OFFSET_COVER_TODAY) {
			pOrder.Offset = MORDER_CLOSE;
		}
		else if (rsp->ReqData.Offset == ESForeign::OFFSET_NONE) {
			pOrder.Offset = MORDER_OPENCLOSENONE;
		}

		if (rsp->OrderState == ESForeign::ORDER_STATE_FAIL) {
			pOrder.OrderStatus = MORDER_FAIL;
		}
		else if (rsp->OrderState == ESForeign::ORDER_STATE_ACCEPT) {
			pOrder.OrderStatus = MORDER_ACCEPTED;
		}
		else if (rsp->OrderState == ESForeign::ORDER_STATE_QUEUED) {
			pOrder.OrderStatus = MORDER_QUEUED;
		}
		else if (rsp->OrderState == ESForeign::ORDER_STATE_PARTFINISHED) {
			pOrder.OrderStatus = MORDER_PART_TRADED;
		}
		else if (rsp->OrderState == ESForeign::ORDER_STATE_FINISHED) {
			pOrder.OrderStatus = MORDER_FULL_TRADED;
		}
		else if (rsp->OrderState == ESForeign::ORDER_STATE_PARTDELETED) {
			pOrder.OrderStatus = MORDER_PART_CANCELLED;
		}
		else if (rsp->OrderState == ESForeign::ORDER_STATE_DELETED) {
			pOrder.OrderStatus = MORDER_CANCELLED;
		}
		else {
			pOrder.OrderStatus = MORDER_STATE_OTHER;
		}

		pOrder.SubmitPrice = rsp->ReqData.OrderPrice;
		pOrder.VolumeTotalOriginal = rsp->ReqData.OrderVol;
		pOrder.TradePrice = rsp->MatchPrice;
		pOrder.VolumeTraded = rsp->MatchVol;
		pOrder.OrderLocalRef = -1;
		pOrder.FrontID = -1;
		pOrder.SessionID = -1;

		Message pOrderMsg;

		if (g_bQryOrderSentByRecoverDlg) {
			pOrderMsg.type = ON_RECOVER_QRY_ORDER;
		}
		else pOrderMsg.type = ON_RSP_QRY_ORDER;

		pOrderMsg.AddData(&pOrder, 0, sizeof(OrderDetailField));
		ScreenDisplayMsgList.AddTail(pOrderMsg);

		ReleaseSemaphore(ScreenDisplaySem, 1, NULL);
	}
};

void __cdecl EsunTraderSpi::OnRspOrderInsert(const ESForeign::TEsOrderInsertRspField* rsp, int errCode, const int iReqID)
{
	if (errCode == 0) {
		if (strcmp(LoginUserTDEsun, rsp->ReqData.ClientNo) == 0) {
			OrderTradeMsg order;
			order.OrderSysId = rsp->OrderId;
			order.ActionLocalNo = iReqID;
			order.OrderType = ON_TD_RSP_ORDER_INSERT;
			OrderList.AddTail(order);
			ReleaseSemaphore(DispatchTdSem, 1, NULL);
			//OrderIdToReqId.insert(std::pair<int, int>(rsp->OrderId,iReqID));
		}
	}
	else {
		//if(strcmp(LoginUserTDEsun, rsp->ReqData.ClientNo)==0) ErrorOrderIdToReqId.insert(std::pair<int, int>(rsp->OrderId,iReqID));
		char data[500];
		sprintf_s(data, "订单指令失败,ErrorCode=%d,OrderId=%d", errCode, rsp->OrderId);
		CString str(data);
		pPubMsg->AddString(str);
	}
}

void __cdecl EsunTraderSpi::OnRspOrderDelete(const ESForeign::TEsOrderDeleteRspField* rsp, int errCode, const int iReqID) {
	if (errCode == 0) {
		char data[500];
		sprintf_s(data, "Response=撤单操作成功,委托号=%d", rsp->OrderStateField.OrderId);
		CString str(data);
		pPubMsg->AddString(str);
	}
	else {
		if (rsp != NULL) {
			char data[500];
			sprintf_s(data, "Response=撤单操作返回错误,委托号=%d,ErrorCode=%d", rsp->OrderStateField.OrderId, errCode);
			CString str(data);
			pPubMsg->AddString(str);
		}
	}
}

void __cdecl EsunTraderSpi::OnRtnOrderState(const ESForeign::TEsOrderStateNoticeField& rsp)
{
	if (rsp.OrderState == ESForeign::ORDER_STATE_ACCEPT || rsp.OrderState == ESForeign::ORDER_STATE_QUEUED || rsp.OrderState == ESForeign::ORDER_STATE_PARTFINISHED || rsp.OrderState == ESForeign::ORDER_STATE_FINISHED
		|| rsp.OrderState == ESForeign::ORDER_STATE_PARTDELETED || rsp.OrderState == ESForeign::ORDER_STATE_DELETED) {
		OrderTradeMsg order;
		order.OrderSysId = rsp.OrderId;
		order.ActionLocalNo = atoi(rsp.ActionLocalNo);
		order.OrderType = ON_RTN_ORDER;
		order.OrderLocalRef = -1;
		if (rsp.OrderState == ESForeign::ORDER_STATE_ACCEPT) {
			order.OrderStatus = MORDER_ACCEPTED;
		}
		else if (rsp.OrderState == ESForeign::ORDER_STATE_QUEUED) {
			order.OrderStatus = MORDER_QUEUED;
		}
		else if (rsp.OrderState == ESForeign::ORDER_STATE_PARTFINISHED) {
			order.OrderStatus = MORDER_PART_TRADED;
		}
		else if (rsp.OrderState == ESForeign::ORDER_STATE_FINISHED) {
			order.OrderStatus = MORDER_FULL_TRADED;
		}
		else if (rsp.OrderState == ESForeign::ORDER_STATE_PARTDELETED) {
			order.OrderStatus = MORDER_PART_CANCELLED;
		}
		else if (rsp.OrderState == ESForeign::ORDER_STATE_DELETED) {
			order.OrderStatus = MORDER_CANCELLED;
		}
		order.LimitPrice = rsp.OrderPrice;
		order.VolumeTotalOriginal = rsp.OrderVol;
		order.VolumeTraded = rsp.MatchVol;
		order.VolumeTotal = order.VolumeTotalOriginal - order.VolumeTraded;
		strcpy(order.InsertOrTradeTime, rsp.UpdateDateTime);

		/*
		map<int,int>::iterator iter;
		iter=OrderIdToShmIndex.find(order.OrderSysId);
		if(iter!=OrderIdToShmIndex.end()){
			order.shmindex=iter->second;
			OrderList.AddTail(order);
		}
		*/
		OrderList.AddTail(order);
		ReleaseSemaphore(DispatchTdSem, 2, NULL);
	}
}

void __cdecl EsunTraderSpi::OnRtnMatchState(const ESForeign::TEsMatchStateNoticeField& rsp)
{
	OrderTradeMsg order;
	order.OrderSysId = rsp.OrderId;//与上述的ActionLocalNo不同，这是交易API生成的号码
	order.OrderType = ON_RTN_TRADE;
	order.OrderLocalRef = -1;
	//order.Direction=rsp.
	order.Price = rsp.MatchPrice;
	order.Volume = rsp.MatchVol;
	order.MatchFee = rsp.MatchFee;
	string strMatchDateTime(rsp.MatchDateTime);//yyyy-MM-dd hh:nn:ss -> yyyymmdd hh:nn:ss
	strMatchDateTime = strMatchDateTime.replace(strMatchDateTime.find("-"), 1, "");
	strMatchDateTime = strMatchDateTime.replace(strMatchDateTime.find("-"), 1, "");

	strcpy(order.InsertOrTradeTime, strMatchDateTime.c_str());
	/*
	map<int,int>::iterator iter;
	iter=OrderIdToShmIndex.find(order.OrderSysId);
	if(iter!=OrderIdToShmIndex.end()){
		order.shmindex=iter->second;
		OrderList.AddTail(order);
	}*/
	strcpy(order.MatchNo, rsp.MatchNo);
	OrderList.AddTail(order);
	ReleaseSemaphore(DispatchTdSem, 2, NULL);
}

void __cdecl EsunTraderSpi::OnRspMatchQry(const ESForeign::TEsMatchDataQryRspField* rsp, ESForeign::TEsIsLastType islast, int errCode, const int iReqID)
{
	if (rsp != NULL && errCode == 0) {
		OrderTradeMsg order;
		order.OrderSysId = rsp->StateData.OrderId;//与上述的ActionLocalNo不同，这是交易API生成的号码
		order.OrderType = ON_RSP_QRY_TRADE;

		order.Price = rsp->StateData.MatchPrice;
		order.Volume = rsp->StateData.MatchVol;
		order.MatchFee = rsp->StateData.MatchFee;
		strcpy(order.InsertOrTradeTime, rsp->StateData.MatchDateTime);
		strcpy(order.MatchNo, rsp->StateData.MatchNo);
		OrderList.AddTail(order);
		ReleaseSemaphore(DispatchTdSem, 2, NULL);
	}
}

/*
4.2. 报单流程说明
1、 用户通过OrderInsert 发送报单请求，其中请求域中的RequestID 字段需要用户自己
生成用于标示发出的不同报单。
2、 交易API 将用户的请求数据封装后发送到易盛交易平台，易盛交易平台收到请求并
验证后给用户响应该报单请求。
3、 交易API 收到易盛交易平台的响应数据，进行数据转换后调用OnRspOrderInsert 函
数通知报单已经处理，对于易盛交易平台处理成功的报单同时会返回委托编号
（OrderNo）字段，用户程序在此处将OrderNo 和第一步生成的RequestID（应答域
的ReqData. RequestID）进行关联，后续该笔委托的状态变化通知通过OrderNo 字段
来识别本报单， RequestID 不再有效。
4、 易盛交易平台应答用户报单同时，将报单请求提交到报单对应的交易所，交易所收
到该笔报单进行内部处理后将处理结果返回给易盛交易平台。
5、 易盛交易平台收到交易所的处理结果分为成功和失败两种情况；对于成功的报单会
生成有效的系统号（SystemNo）字段，失败的报单返回失败原因，不会生成系统号。

*/

/*
void EsunTraderSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
CString str("OnRspOrderInsert");
CString str1(pRspInfo->ErrorMsg);
pPubMsg->AddString(str1);
}
*/
/*
void EsunTraderSpi::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
CString str("OnRspOrderAction");
pPubMsg->AddString(str);
CString str1(pRspInfo->ErrorMsg);
TRACE("OnRspOrderAction,orderref=",pInputOrderAction->OrderSysId);
pPubMsg->AddString(str1);
}
*/

int EsunTraderSpi::ReqOrderInsert(InsertOrderField* pReq, int* iRetReqID, bool x_bCrossTradingDay)
{
	ESForeign::TEsOrderInsertReqField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.ClientNo, LoginUserTDEsun);

	strcpy(req.ContractNo, pReq->InstrumentID);
	strcpy(req.CommodityNo, pReq->CommodityNo);

	//if(strstr(pReq->InstrumentID, "LME")==pReq->InstrumentID){
		//限价单
	req.OrderType = ESForeign::ORDER_TYPE_LIMIT;
	//}
	//else{
		//市价单
		//req.OrderType = ESForeign::ORDER_TYPE_MARKET;
	//}
	//自主电子单
	req.OrderWay = ESForeign::ORDER_WAY_SELF_ETRADER;

	if (x_bCrossTradingDay) {
		//取消前有效
		req.OrderMode = ESForeign::ORDER_MODE_GTC;
	}
	else {
		//当日有效
		req.OrderMode = ESForeign::ORDER_MODE_GFD;
	}
	//风险报单
	//req.IsRiskOrder = ESForeign::RISK_ORDER_YES;

	if (pReq->Direction == MORDER_SELL) {
		req.Direct = ESForeign::DIRECT_SELL;
	}
	else {
		req.Direct = ESForeign::DIRECT_BUY;
	}
	/*
	if(pReq->Offset==MORDER_OPEN){
	//开平仓：无
	req.Offset = ESForeign::OFFSET_OPEN;
	}
	else{
	req.Offset = ESForeign::OFFSET_COVER;
	}
	*/
	req.Offset = ESForeign::OFFSET_NONE;
	//投机
	req.Hedge = ESForeign::HEDGE_T;
	//委托价格
	req.OrderPrice = pReq->OrderPrice;

	req.OrderVol = pReq->OrderVol;
	req.MinMatchVol = 1;
	int iReqID = -1;
	//WaitForSingleObject(TraderOrderInsertSem, INFINITE);
	int ret = -1;
	if (req.OrderPrice > 0 && req.OrderPrice < 999999)
	{
		ret = m_Api->OrderInsert(req, iReqID/*pReq->RequestID*/);
	}

	//ReleaseSemaphore(TraderOrderInsertSem, 1, NULL);
	(*iRetReqID) = iReqID;
	pReq->FrontID = 0;
	pReq->SessionID = 0;

	//	ReqIdToOrderLocalRef.insert(std::pair<int,int>(iReqID,pReq->OrderLocalRef));
	return ret;
}

void EsunTraderSpi::ReqQryPosition(char ExchangeNo[30], char CommodityNo[30], char InstrumentID[30])
{
	ESForeign::TEsHoldQryReqField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.ClientNo, LoginUserTDEsun);
	strcpy(req.CommodityNo, CommodityNo);
	strcpy(req.ContractNo, InstrumentID);
	//strcpy(req.ClientNo, loginid
	//strcpy(req.BrokerID, m_BrokerID);
	//strcpy(req.InvestorID, loginid);
	int ret = m_Api->QryHold(req, ++g_iRequestIDEsun);
}

void EsunTraderSpi::ReqQryOrder(char ExchangeNo[30], char CommodityNo[30], char InstrumentID[30])
{
	ESForeign::TEsOrderQryReqField req;
	memset(&req, 0, sizeof(req));
	req.OrderStreamId = 0;
	strcpy(req.ClientNo, LoginUserTDEsun);
	strcpy(req.ExchangeNo, ExchangeNo);
	strcpy(req.CommodityNo, CommodityNo);
	strcpy(req.ContractNo, InstrumentID);
	int ret = m_Api->QryOrder(req, ++g_iRequestIDEsun);
}

void EsunTraderSpi::ReqQryTrade(char ExchangeNo[30], char CommodityNo[30], char InstrumentID[30], char BeginMatchDateTime[21])//格式 yyyy-MM-dd hh:nn:ss
{
	ESForeign::TMatchQryReqField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.ClientNo, LoginUserTDEsun);
	strcpy(req.ExchangeNo, ExchangeNo);
	strcpy(req.CommodityNo, CommodityNo);
	strcpy(req.ContractNo, InstrumentID);
	strcpy(req.BeginMatchDateTime, BeginMatchDateTime);
	int ret = m_Api->QryMatch(req, ++g_iRequestIDEsun);
}

void EsunTraderSpi::ReqOrderDelete(int OrderId)
{
	ESForeign::TEsOrderDeleteReqField req;
	req.OrderId = OrderId;
	int iReqID = 0;
	m_Api->OrderDelete(req, iReqID);
}

void EsunTraderSpi::OnInitFinished(int errCode) {
	if (0 == errCode) {
#ifdef _MONEY
		pQryActionThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
		Sleep(200);
		pQryActionThread->PostThreadMessage(WM_QRY_MONEY, NULL, NULL);
#endif
#ifdef _POSITION
		pQryActionThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
		Sleep(200);
		pQryActionThread->PostThreadMessage(WM_QRY_POSITION_DETAILS, NULL, NULL);
#endif
	}
}