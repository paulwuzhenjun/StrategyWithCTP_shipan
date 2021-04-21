#include "StdAfx.h"
#include "EsunMdSpi.h"
//#include <iostream>
#include "Message.h"
#include "MyStruct.h"
#include "TickDataList.h"
#include "MessageList.h"
#include "MyThread.h"
#include "GlobalFunc.h"

using namespace std;

#pragma warning(disable : 4996)

extern CListBox* pMdPubMsg;
extern TickDataList TickList;
// 配置参数

extern char LoginMDUser[];
// 请求编号
extern int iRequestID;
extern HANDLE MdTickSem;
extern CMyThread* pRestartEsunMdThread;
extern MessageList LogMessageList;
extern HANDLE logSemaphore;
extern bool EsunMDReleasedAction;
extern GlobalFunc globalFuncUtil;

CEsunMdSpi::CEsunMdSpi()
{
	m_bInitOK = false;
	m_pQuoteAPI = CreateEsunnyQuotClient(this);
}

CEsunMdSpi::~CEsunMdSpi(void)
{
	DelEsunnyQuotClient(m_pQuoteAPI);
}
int	CEsunMdSpi::Connect(const char* ip, int port)
{
	return m_pQuoteAPI->Connect(ip, port);
}
void CEsunMdSpi::DisConnect()
{
	globalFuncUtil.WriteMsgToLogList("CEsunMdSpi::DisConnect()");
	return m_pQuoteAPI->DisConnect();
}
int	CEsunMdSpi::Login(const char* user, const char* password)
{
	m_bLoginDone = false;
	m_bLoginSuccess = false;
	return m_pQuoteAPI->Login(user, password);
}
int	CEsunMdSpi::RequestQuot(const char* market, const char* stk, int need)
{
	return m_pQuoteAPI->RequestQuot(market, stk, need);
}
int	CEsunMdSpi::RequestHistory(const char* market, const char* stk, int period)
{
	return m_pQuoteAPI->RequestHistory(market, stk, period);
}
int	CEsunMdSpi::RequestTrace(const char* market, const char* stk, const char* date)
{
	return m_pQuoteAPI->RequestTrace(market, stk, date);
}
int	CEsunMdSpi::AddReqStk(const char* market, const char* stk, int need)
{
	return m_pQuoteAPI->AddReqStk(market, stk, need);
}
int	CEsunMdSpi::SendReqStk()
{
	return m_pQuoteAPI->SendReqStk();
}
int	CEsunMdSpi::InitSecretKey(const char* secretkey, int option)
{
	return m_pQuoteAPI->InitSecretKey(secretkey, option);
}

int CEsunMdSpi::OnRspLogin(int err, const char* errtext)
{
	char logline[200];

	if (err == 0) {
		char data[500];
		sprintf(data, "易胜行情登录返回成功:loginaccount=%s;", LoginMDUser);
		CString str(data);
		pMdPubMsg->AddString(str);
		sprintf(logline, "Esun MD RspLogin Success.");
		m_bLoginSuccess = true;
	}
	else {
		char data[500];
		sprintf(data, "易胜行情登录返回失败:loginaccount=%s;content=%s", LoginMDUser, errtext);
		CString str(data);
		pMdPubMsg->AddString(str);
		sprintf(logline, "Esun MD RspLogin Failured.");
		m_bLoginSuccess = false;
	}

	struct tm* ptTm;
	time_t nowtime;
	memset(&beginrun_date, 0, 10);
	time(&nowtime);
	ptTm = localtime(&nowtime);
	strftime(beginrun_date, 10, "%Y%m%d", ptTm);

	Message logMsg;
	logMsg.type = MD_LOG;
	logMsg.AddData(logline, 0, sizeof(char) * 200);
	LogMessageList.AddTail(logMsg);
	ReleaseSemaphore(logSemaphore, 1, NULL);

	m_bLoginDone = true;
	return 0;
}
int CEsunMdSpi::OnChannelLost(int err, const char* errtext)
{
	CString str("易胜行情连接丢失...");
	pMdPubMsg->AddString(str);

	globalFuncUtil.WriteMsgToLogList("易胜行情连接丢失...");

	if (!EsunMDReleasedAction) {
		globalFuncUtil.WriteMsgToLogList("易胜行情重连...");
		pRestartEsunMdThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
		Sleep(500);
		pRestartEsunMdThread->PostThreadMessage(WM_RESTART_ESUN_MD, NULL, NULL);
	}

	return 1;
}
int CEsunMdSpi::OnStkQuot(struct STKDATA* pData)
{
	int nHour, nMin, nSec;
	nHour = (int)(pData->updatetime) / 10000;
	nMin = ((int)(pData->updatetime) % 10000) / 100;
	nSec = ((int)(pData->updatetime) % 10000) % 100;

	//if(nHour<=8||(nHour>=15&&nHour<=20)||(nHour>=23&&nMin>=29)) return -1;

	TickInfo tickData;
	strcpy(tickData.ordername, pData->Code);
	//strcpy(tickData.datadate,beginrun_date);
	sprintf(tickData.updatetime, "%02d:%02d:%02d", nHour, nMin, nSec);
	TRACE("%s,%.5f,%s\n", tickData.ordername, pData->New, tickData.updatetime);
	//tickData.millsec=0;
	tickData.bid1 = pData->Bid[0];
	tickData.ask1 = pData->Ask[0];
	tickData.price = pData->New;
	if (tickData.bid1 > 0 && tickData.bid1 < 999999 && tickData.ask1>0 && tickData.ask1 < 999999 && tickData.price>0 && tickData.price < 999999)
		TickList.AddTail(tickData);

	ReleaseSemaphore(MdTickSem, 2, NULL);
	return 1;
}
int CEsunMdSpi::OnRspHistoryQuot(struct STKHISDATA* pHisData)
{
	return 1;
}
int CEsunMdSpi::OnRspTraceData(struct STKTRACEDATA* pTraceData)
{
	return 1;
}
int CEsunMdSpi::OnRspMarketInfo(struct MarketInfo* pMarketInfo, int bLast)
{
	return 1;
}

//datadate format: yyyymmdd , datatime formate hh24:mi:ss
//true - can open new order , false - shouldnot open new order
bool CEsunMdSpi::timeRuleForOpen(char datatime[10]) {
	int nHour, nMin, nSec;

	sscanf_s(datatime, "%d:%d:%d", &nHour, &nMin, &nSec);

	if (nHour == 9 && nMin <= 14) return false;
	if (nHour == 11 && nMin == 29) return false;
	if (nHour == 15 && nMin >= 14) return false;

	return true;
}