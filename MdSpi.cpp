#include "StdAfx.h"
#include "MdSpi.h"
//#include <iostream>
#include "Message.h"
#include "MyStruct.h"
#include "TickDataList.h"
#include "MyThread.h"

using namespace std;

#pragma warning(disable : 4996)

// USER_API参数
extern CListBox* pPubMsg;
extern CListBox* pPubMsg;

extern CListBox* pRealTimePrice;

extern TickDataList TickList;

// 配置参数
extern char FRONT_ADDR[];
extern double gOpenPrice;

extern char LoginUserTDEsun[];

//extern LONGLONG nStart; //微妙
//extern LARGE_INTEGER liQPF;
//extern double g_PF_us;

extern int iNextOrderRef;    //下一单引用

CMdSpi::CMdSpi()
{
	m_bInitOK = false;
	m_pQuoteAPI = CreateEsunnyQuotClient(this);
}

CMdSpi::~CMdSpi(void)
{
	DelEsunnyQuotClient(m_pQuoteAPI);
}
int	CMdSpi::Connect(const char* ip, int port)
{
	return m_pQuoteAPI->Connect(ip, port);
}
void CMdSpi::DisConnect()
{
	return m_pQuoteAPI->DisConnect();
}
int	CMdSpi::Login(const char* user, const char* password)
{
	return m_pQuoteAPI->Login(user, password);
}
int	CMdSpi::RequestQuot(const char* market, const char* stk, int need)
{
	return m_pQuoteAPI->RequestQuot(market, stk, need);
}
int	CMdSpi::RequestHistory(const char* market, const char* stk, int period)
{
	return m_pQuoteAPI->RequestHistory(market, stk, period);
}
int	CMdSpi::RequestTrace(const char* market, const char* stk, const char* date)
{
	return m_pQuoteAPI->RequestTrace(market, stk, date);
}
int	CMdSpi::AddReqStk(const char* market, const char* stk, int need)
{
	return m_pQuoteAPI->AddReqStk(market, stk, need);
}
int	CMdSpi::SendReqStk()
{
	return m_pQuoteAPI->SendReqStk();
}
int	CMdSpi::InitSecretKey(const char* secretkey, int option)
{
	return m_pQuoteAPI->InitSecretKey(secretkey, option);
}

int CMdSpi::OnRspLogin(int err, const char* errtext)
{
	if (err == 0) {
		char data[500];
		sprintf(data, "MD OnRspLogin:loginaccount=%s;content=登录成功", LoginUserTDEsun);
		CString str(data);
		pPubMsg->AddString(str);
	}
	else {
		char data[500];
		sprintf(data, "MD OnRspLogin:loginaccount=%s;content=%s", LoginUserTDEsun, errtext);
		CString str(data);
		pPubMsg->AddString(str);
	}

	struct tm* ptTm;
	time_t nowtime;
	memset(&beginrun_date, 0, 10);
	time(&nowtime);
	ptTm = localtime(&nowtime);
	strftime(beginrun_date, 10, "%Y%m%d", ptTm);

	return 0;
}
int CMdSpi::OnChannelLost(int err, const char* errtext)
{
	CString str("MD OnChannelLost...");
	pPubMsg->AddString(str);
	return 1;
}
int CMdSpi::OnStkQuot(struct STKDATA* pData)
{
	int nHour, nMin, nSec;
	nHour = (int)(pData->updatetime) / 10000;
	nMin = ((int)(pData->updatetime) % 10000) / 100;
	nSec = ((int)(pData->updatetime) % 10000) % 100;

	//if(nHour<=8||(nHour>=15&&nHour<=20)||(nHour>=23&&nMin>=29)) return -1;

	TickInfo tickData;

	strcpy(tickData.ordername, pData->Code);
	strcpy(tickData.datadate, beginrun_date);
	sprintf(tickData.updatetime, "%02d:%02d:%02d", nHour, nMin, nSec);
	tickData.millsec = 0;
	tickData.bid1 = pData->Bid[0];
	tickData.bidvol1 = pData->BidVol[0];
	tickData.ask1 = pData->Ask[0];
	tickData.askvol1 = pData->AskVol[0];
	tickData.price = pData->New;
	TRACE("%s,%.5f,%s\n", tickData.ordername, pData->New, tickData.updatetime);

	char pr[30];
	double price = tickData.price;
	if (price > 0 && price < 99999)sprintf_s(pr, "%.5f", price);
	else sprintf_s(pr, "%.5f", 0.0);
	CString priceString(pr);

	TickList.AddTail(tickData);
	/*
	CString thisInstName(tickData.ordername);
	pPriceTag1->SetWindowTextW(thisInstName);
	pPrice1->SetWindowTextW(priceString);
	*/
	return 1;
}
int CMdSpi::OnRspHistoryQuot(struct STKHISDATA* pHisData)
{
	return 1;
}
int CMdSpi::OnRspTraceData(struct STKTRACEDATA* pTraceData)
{
	return 1;
}
int CMdSpi::OnRspMarketInfo(struct MarketInfo* pMarketInfo, int bLast)
{
	return 1;
}

//datadate format: yyyymmdd , datatime formate hh24:mi:ss
//true - can open new order , false - shouldnot open new order
bool CMdSpi::timeRuleForOpen(char datatime[10]) {
	int nHour, nMin, nSec;

	sscanf_s(datatime, "%d:%d:%d", &nHour, &nMin, &nSec);

	if (nHour == 9 && nMin <= 14) return false;
	if (nHour == 11 && nMin == 29) return false;
	if (nHour == 15 && nMin >= 14) return false;

	return true;
}