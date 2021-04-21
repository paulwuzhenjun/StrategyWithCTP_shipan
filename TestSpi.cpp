#include "StdAfx.h"
#include "TestSpi.h"
#include "TestTraderApiDlg.h"

extern CString PublicMsg;
extern CString PrivateMsg;
extern CTraderThread* pTraderThread;
extern CListBox* pPubMsg;
extern CListBox* pPriMsg;
extern CThostFtdcTraderApi* pUserApi;
extern CStatic* pDay;
extern CStatic* pMoney;
extern CStatic* pUsername;


int iRequestID = 0;
TThostFtdcFrontIDType	FRONT_ID;	//前置编号
TThostFtdcSessionIDType	SESSION_ID;	//会话编号
TThostFtdcOrderRefType	ORDER_REF;	//报单引用

void AddPublicMsg(CString Msg)
{
}

bool IsFlowControl(int iResult)
{
	return ((iResult == -2) || (iResult == -3));
}

TestSpi::TestSpi(void)
{
}

TestSpi::~TestSpi(void)
{
}

void TestSpi::OnFrontConnected()
{
	//int nTmpt=0;

	//::PostMessage((HWND)(AfxGetMainWnd()->GetSafeHwnd()),WM_PUBMSG,nTmpt,NULL);

	//AfxMessageBox(_T("hello1"),MB_OK);
	//pThreadThread->
	//::PostMessage((HWND)(pTraderThread->GetMainWnd()->GetSafeHwnd()),WM_PUBMSG,NULL,NULL);
	CString str("HTTP Connected");
	pPubMsg->AddString(str);
	ReqUserLogin();
}
void TestSpi::ReqUserLogin()
{
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.UserID, INVESTOR_ID);
	strcpy(req.Password, PASSWORD);
	int iResult = pUserApi->ReqUserLogin(&req, ++iRequestID);
	//cerr << "--->>> 发送用户登录请求: " << iResult <<  << endl;
	//CString str("");
	CString str1(((iResult == 0) ? "sending user login request success" : "sending user login request fail"));
	//pPubMsg->AddString(str);
	pPubMsg->AddString(str1);

}

void TestSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	CString str("User Login success" );
	pPubMsg->AddString(str);
	
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		// 保存会话参数
		CString str4(pRspUserLogin->UserID);
		pUsername->SetWindowTextW(str4);
		FRONT_ID = pRspUserLogin->FrontID;
		SESSION_ID = pRspUserLogin->SessionID;
		int iNextOrderRef = atoi(pRspUserLogin->MaxOrderRef);
		iNextOrderRef++;
		sprintf(ORDER_REF, "%d", iNextOrderRef);
		///获取当前交易日
		//cerr << "--->>> 获取当前交易日 = " << pUserApi->GetTradingDay() << endl;
		///投资者结算结果确认
		CString str1("Current trading day is  ");
		CString str2(pUserApi->GetTradingDay());
		CString str3(str1+str2);
		pPubMsg->AddString(str3);
		pDay->SetWindowTextW(str2);
		//pPubMsg->AddString(str);
		//ReqQryTradingAccount();
		ReqSettlementInfoConfirm();
		char* Instrument[]={"IF1105","IF1106"};
		//pUserApi->SubscribeMarketData(Instrument,2);
		//pUserApi->sub
	}
}

bool TestSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
	// 如果ErrorID != 0, 说明收到了错误的响应
	bool bResult = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	CString str("response error");
	CString str1("response correct");
	if (bResult)
		//cerr << "--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << endl;
		pPubMsg->AddString(str);
	else
		pPubMsg->AddString(str1);
	return bResult;
}
void TestSpi::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
	CString str( "--->>> OnRtnOrder" );
	pPriMsg->AddString(str);
	//pPubMsg->SetCurSel(pPubMsg->GetCount()-1);
//	if (IsMyOrder(pOrder))
//	{
//		if (IsTradingOrder(pOrder))
			//ReqOrderAction(pOrder);
//		else if (pOrder->OrderStatus == THOST_FTDC_OST_Canceled)
//		{
			//cout << "--->>> 撤单成功" << endl;
//				CString str1( "撤单成功" );
//				pPubMsg->AddString(str1);
//		}
			
//	}
}

bool TestSpi::IsMyOrder(CThostFtdcOrderField *pOrder)
{
	return ((pOrder->FrontID == FRONT_ID) &&
			(pOrder->SessionID == SESSION_ID) &&
			(strcmp(pOrder->OrderRef, ORDER_REF) == 0));
}

bool TestSpi::IsTradingOrder(CThostFtdcOrderField *pOrder)
{
	return ((pOrder->OrderStatus != THOST_FTDC_OST_PartTradedNotQueueing) &&
			(pOrder->OrderStatus != THOST_FTDC_OST_Canceled) &&
			(pOrder->OrderStatus != THOST_FTDC_OST_AllTraded));
}



void TestSpi::ReqSettlementInfoConfirm()
{
	CThostFtdcSettlementInfoConfirmField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVESTOR_ID);
	int iResult = pUserApi->ReqSettlementInfoConfirm(&req, ++iRequestID);
	//cerr << "--->>> 投资者结算结果确认: " << iResult << ((iResult == 0) ? ", 成功" : ", 失败") << endl;
	CString str("request settlement infor confirmation");
	pPubMsg->AddString(str);
}

void TestSpi::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	//cerr << "--->>> " << "OnRspSettlementInfoConfirm" << endl;
	CString str("OnRspSettlementInfoConfirm");
	pPubMsg->AddString(str);
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		///请求查询合约
		ReqQryInstrument();
	}
}

void TestSpi::ReqQryInstrument()
{
	CThostFtdcQryInstrumentField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.InstrumentID, INSTRUMENT_ID);
	while (true)
	{
		int iResult = pUserApi->ReqQryInstrument(&req, ++iRequestID);
		if (!IsFlowControl(iResult))
		{
			//cerr << "--->>> 请求查询合约: "  << iResult << ((iResult == 0) ? ", 成功" : ", 失败") << endl;
			break;
		}
		else
		{
			//cerr << "--->>> 请求查询合约: "  << iResult << ", 受到流控" << endl;
			Sleep(1000);
		}
	} // while
}

void TestSpi::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	//cerr << "--->>> " << "OnRspQryInstrument" << endl;
	CString str("OnRspQryInstrument");
	pPubMsg->AddString(str);
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		///请求查询合约
		ReqQryTradingAccount();
	}
}

void TestSpi::ReqQryTradingAccount()
{
	CThostFtdcQryTradingAccountField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVESTOR_ID);
	while (true)
	{
		int iResult = pUserApi->ReqQryTradingAccount(&req, ++iRequestID);
		if (!IsFlowControl(iResult))
		{
			//cerr << "--->>> 请求查询资金账户: "  << iResult << ((iResult == 0) ? ", 成功" : ", 失败") << endl;
			CString str("request trading account success....waiting for response");
			pPubMsg->AddString(str);
			break;
		}
		else
		{
			//cerr << "--->>> 请求查询资金账户: "  << iResult << ", 受到流控" << endl;
			CString str("request trading account blocked.....waiting");
			Sleep(1000);
		}
	} // while
}
void TestSpi::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	//cerr << "--->>> " << "OnRspQryTradingAccount" << endl;
	IsErrorRspInfo(pRspInfo);
	double money=0;
	if(pTradingAccount==0)
	{
		CString str("return trading account error");
		pPubMsg->AddString(str);
		CString str1(pRspInfo->ErrorMsg);
		pPubMsg->AddString(str1);
		CString str2;
	//	str2.Format(_T("%i"),pRspInfo->ErrorID);
		//pPubMsg->AddString(str2);
	}
	else
	{
		money=pTradingAccount->Available;
		CString str1;
		str1.Format(_T("%d"),money);
		CString str2("Available money is ");
		CString str3(str2+str1);
	//pPubMsg->AddString(str3);
	//pMoney->SetWindowTextW(str1);
	}
}

void TestSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	CString str("onrtndepth");
	pPubMsg->AddString(str);
}