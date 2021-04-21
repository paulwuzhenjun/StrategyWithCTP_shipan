// TraderThread.cpp : implementation file
//

#include "stdafx.h"
#include "HitTraderApi.h"
#include "MyThread.h"
#include "MdSpi.h"
#include "EsunTraderSpi.h"
#include "ThostTraderSpi.h"
#include "SgitTraderSpi.h"
#include "StrategyOpenPriceOpening.h"
#include "StrategyLastTTimeOpen.h"
#include "StrategyKDJ.h"
#include "StrategyGridOpen.h"
#include "ScreenDisplayThread.h"
#include "LogMsgThread.h"
#include "DispatchTdMsgThread.h"
#include "OrderDataList.h"
#include <map>
#include <io.h>
#include "StrategyRecoverDlg.h"
#include "GlobalFunc.h"
#include "MessageList.h"
#include "StrategyBandMomnt.h"
#include "EsunMdSpi.h"
#include "MDDataDispatchThread.h"
#include "ThostMdSpi.h"
#include "SgitMdSpi.h"
#include "StrategyWaveOpen.h"
#include "StrategyWaveOpenAdd.h"
#include "StrategyBar.h"
#include "StrategyUpDownR.h"
#include "StrategyAvgLine.h"
#include "StrategyAvgDown.h"
#include "StrategyDT.h"
#include "StrategyBaseGridOpen.h"
#include "StrategyBaseGridOpen_plus.h"
#include "StrategyBaoChe.h"
#include "StrategyOpenPriceOpeningNew.h"
#include "StrategyOpenPriceOpeningAsia.h"
#include "StrategyBaseGridMAStopOpen.h"
#include "StrategyBaseGridOpenCffex.h"
#include "StrategyBaseGridMAStopOpenCffex.h"
#include "StrategyThreeK.h"
#include "StrategyOpenPriceOpeningNight.h"
#include "StrategyGridMAStopGTCChop.h"
#include "database.h"
#include "TradeDataList.h"
#include "RiskManagementTcp.h"

using namespace std;
// CMyThread
//extern char InstrumentID[];
//extern char CodeName[];
extern bool gEndSendUdp;
extern char LoginUserTDEsun[];
extern char LoginPwdTDEsun[];
extern char LoginUserTDCTP[];
extern char LoginPwdCTP[];
extern char LoginMDUser[];
extern char LoginMDPwd[];
extern char LoginMDUserCTP[];
extern char LoginMDPwdCTP[];

extern CListBox* pPubMsg;
extern CListBox* pMdPubMsg;
extern EsunTraderSpi* pEsunTraderSpi;
extern ThostTraderSpi* pThostTraderSpi;
extern SgitTraderSpi* pSgitTraderSpi;
extern CMdSpi* pMdSpi;
extern CStrategy* gStrategyImpl[];
extern int gStrategyImplIndex;

extern CListCtrl* pQryOrdersList;
extern CListCtrl* pPositionDetailsList;
extern CListCtrl* pParamsList;

extern CListCtrl* pStrategyToBeRecover;

extern char  FRONT_TRADER_ADDR[];
extern char  FRONT_MD_ADDR[];
extern MapViewType* gMapView;
extern list<ModelNode> ModelList;
extern TradeDataList TradeList;
extern CString ClassNameShowing;
extern CString StrategyIDShowing;
extern HANDLE CreateThreadSem;
extern HANDLE DispatchTdSem;
extern HANDLE tradeSemaphore;
extern OrderDataList OrderList;
extern map<string, InstrumentInfo> InstrumentsSubscribed;
extern list<string> InstrumentsSubscribedList;
extern CMyThread* pEsunTraderThread;
extern CMyThread* pThostTraderThread;
extern CMyThread* pQryTradeThread;
extern bool OnOpenSuccess;
extern CMyThread* pQryActionThread;
extern bool g_bQryOrderSentByRecoverDlg;
extern char TDConnectionLostTime[];

extern bool EsunAPIUsed;
extern bool CTPAPIUsed;
extern bool SgitAPIUsed;

extern GlobalFunc globalFuncUtil;
extern MessageList LogMessageList;
extern HANDLE logSemaphore;

extern HANDLE ambushordersem;
extern list<InsertOrderField> ambushOrderList;
extern MessageList ScreenDisplayMsgList;
extern HANDLE ScreenDisplaySem;
extern HANDLE RecoverStrategyDlgSem;
///------------------------------------------------
extern char LoginMDUser[];
extern char LoginMDPwd[];
extern char LoginMDUserCTP[];
extern char LoginMDPwdCTP[];

extern CListBox* pMdPubMsg;
extern int iRequestIDMD;

extern CEsunMdSpi* pEsunMdSpi;
extern CThostMdSpi* pThostMdSpi;
extern CSgitMdSpi* pSgitMdSpi;

extern CMyThread* pEsunMdThread;
extern CMyThread* pCTPMdThread;
extern CMyThread* pSgitMdThread;

extern list<InstrumentsName> OverSeaInstSubscribed;
extern list<InstrumentsName> DomesticInstSubscribed;
extern list<InstrumentsName> SGEInstSubscribed;

extern list<InstrumentsName> OverSeaInstSubscribedNew;
extern list<InstrumentsName> DomesticInstSubscribedNew;
extern list<InstrumentsName> SGEInstSubscribedNew;

extern char* ppInstrumentID[];
extern int iInstrumentID;
extern char* SgitppInstrumentID[];
extern int SgitiInstrumentID;

extern MapViewType* gMapView;
extern addrtype EsunMdAddr[MAXMDADDRNO];
extern addrtype EsunTdAddr[MAXTDADDRNO];
extern char CTPTradingDay[];
extern char RSServerIP[];
extern int RSServerPort;
extern RiskManagementTcp* pRMTcp;

IMPLEMENT_DYNCREATE(CMyThread, CWinThread)

CMyThread::CMyThread()
{
}

CMyThread::~CMyThread()
{
}

BOOL CMyThread::InitInstance()
{
	// TODO:  perform and per-thread initialization here
	return TRUE;
}

int CMyThread::ExitInstance()
{
	// TODO:  perform any per-thread cleanup here
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CMyThread, CWinThread)
	ON_THREAD_MESSAGE(WM_CREATE_ESUN_MD, OnCreateEsunMd)
	ON_THREAD_MESSAGE(WM_RESTART_ESUN_MD, OnRestartEsunMd)
	ON_THREAD_MESSAGE(WM_CREATE_CTP_MD, OnCreateCTPMd)
	ON_THREAD_MESSAGE(WM_RESTART_CTP_MD, OnRestartCTPMd)
	ON_THREAD_MESSAGE(WM_CREATE_SGIT_MD, OnCreateSgitMd)
	ON_THREAD_MESSAGE(WM_CREATE_MD_DISPATHER, OnCreateMDDispatchThread)
	ON_THREAD_MESSAGE(WM_ADD_NEW_SUBINST, OnAddNewSubInst)
	ON_THREAD_MESSAGE(WM_CREATE_ESUN_TRADER, OnCreateEsunTrader)
	ON_THREAD_MESSAGE(WM_RESTART_ESUN_TRADER, OnRestartEsunTrader)
	ON_THREAD_MESSAGE(WM_START_STRATEGY, OnStartStrategy)
	ON_THREAD_MESSAGE(WM_RESTART_STRATEGY_RECOVERDLG, OnReStartStrategyFromRecoverDlg)
	ON_THREAD_MESSAGE(WM_STOP_STRATEGY, OnStopStrategy)
	ON_THREAD_MESSAGE(WM_START_ACTION_THREAD, OnStartProcessActionThread)
	ON_THREAD_MESSAGE(WM_STOP, OnStop)
	ON_THREAD_MESSAGE(WM_QRY_POSITION_DETAILS, OnSendQryPositionDetailsMsg)
	ON_THREAD_MESSAGE(WM_QRY_ORDERS, OnSendQryOrdersMsg)
	ON_THREAD_MESSAGE(WM_RECOVER_QRY_ORDERS, OnSendRecoverQryOrdersMsg)
	ON_THREAD_MESSAGE(WM_QRY_TRADES, OnSendQryTradesMsg)
	ON_THREAD_MESSAGE(WM_TDLOST_QRY_TRADES, OnTDLostSendQryTradesMsg)
	ON_THREAD_MESSAGE(WM_QRY_MONEY, OnSendQryMoneyMsg)
	ON_THREAD_MESSAGE(WM_WITHDRAW_ORDER, OnWithdrawOrder)
	ON_THREAD_MESSAGE(WM_CREATE_LOG_THREAD, OnCreateLogThread)
	ON_THREAD_MESSAGE(WM_CREATE_TDSHM_THREAD, OnCreateTdDispatchThread)
	ON_THREAD_MESSAGE(WM_CREATE_CTP_TRADER, OnCreateCTPTrader)
	ON_THREAD_MESSAGE(WM_RESTART_CTP_TRADER, OnRestartCTPTrader)
	ON_THREAD_MESSAGE(WM_CREATE_SGIT_TRADER, OnCreateSgitTrader)
	ON_THREAD_MESSAGE(WM_CHECK_INSTANCE_CFG, CheckAllInstanceStatus)
	ON_THREAD_MESSAGE(WM_UPLOAD_TRADE_LOG, OnUpLoadTradeLog)
	ON_THREAD_MESSAGE(WM_UPLOAD_VALUE, OnUpLoadValue)
	ON_THREAD_MESSAGE(WM_UPLOAD_CTP_ACCVALUE, OnUpLoadCTPAccValue)
	ON_THREAD_MESSAGE(WM_SEND_UDP_MESSAGE, OnSendUdpMessage)
	ON_THREAD_MESSAGE(WM_CREATE_RM_TCP, OnCreateRMTcpThread)
END_MESSAGE_MAP()

// CMyThread message handlers
wstring CMyThread::s2ws(const string& s)
{
	setlocale(LC_ALL, "chs");
	const char* _Source = s.c_str();
	size_t _Dsize = s.size() + 1;
	wchar_t* _Dest = new wchar_t[_Dsize];
	wmemset(_Dest, 0, _Dsize);
	mbstowcs(_Dest, _Source, _Dsize);
	wstring result = _Dest;
	delete[]_Dest;
	setlocale(LC_ALL, "C");
	return result;
}

void CMyThread::OnCreateEsunTrader(WPARAM wParam, LPARAM lParam)
{
	CString pubMsg("Create Trader Thread..");
	pPubMsg->AddString(pubMsg);
	globalFuncUtil.WriteMsgToLogList("Create Trader Thread..");

	struct tm* ptTm;
	time_t nowtime;
	char cur_datetime[20];
	int nHour, nMin, nSec;
	int iRet = -1;
	time(&nowtime);
	ptTm = localtime(&nowtime);
	strftime(cur_datetime, 20, "%X", ptTm);
	sscanf_s(cur_datetime, "%d:%d:%d", &nHour, &nMin, &nSec);
	while ((ptTm->tm_wday == 0) || (ptTm->tm_wday == 6 && nHour > 5)
		|| (ptTm->tm_wday == 1 && nHour == 5 && nMin <= 45) || (ptTm->tm_wday == 1 && nHour < 5)
		) {
		//当前时间为停市期间,故5:30后再重新连接
		time(&nowtime);
		ptTm = localtime(&nowtime);
		strftime(cur_datetime, 20, "%X", ptTm);
		sscanf_s(cur_datetime, "%d:%d:%d", &nHour, &nMin, &nSec);

		Sleep(300000);
	}

	pEsunTraderSpi = new EsunTraderSpi();
	ESForeign::TEsCertInfoType cert;
	memset(&cert, 0, sizeof(cert));
#ifdef _TEST
	strcpy(cert, "C7833C7AE258DB7174AADD371C6BCC7C659DEA0C1BE1425CA76D75D96007A7267AFE4CBF4CAA52FD1EFE57BEF5308F299D029BD60BC9F5EE821B117B6D4AB53C60B32393324E3ED890F37FEBC9E0E8EF2B669E2E03BFC314581A4E022D774963581A4E022D774963581A4E022D774963581A4E022D774963581A4E022D774963D050C47BED3ED850EB24DD23CBAEB147374BDA8B40E4214C581A4E022D774963581A4E022D774963581A4E022D774963581A4E022D774963581A4E022D77496354E606ACCBE47772");
#else
	strcpy(cert, "C7833C7AE258DB7174AADD371C6BCC7C659DEA0C1BE1425CA76D75D96007A7267AFE4CBF4CAA52FD1EFE57BEF5308F291CA3F7E476DB49DE7352AC365C958C3BC8881E9E872836B02C0C7F3FF3700177F5DA4C82A4A0A4BFB1071141D5CD3BC6581A4E022D774963581A4E022D774963581A4E022D774963581A4E022D774963D050C47BED3ED850EB24DD23CBAEB147374BDA8B40E4214C581A4E022D774963581A4E022D774963581A4E022D774963581A4E022D774963581A4E022D77496354E606ACCBE47772");//实盘
#endif
	ESForeign::TEsAppIDType appID;
	memset(&appID, 0, sizeof(appID));
#ifdef _TEST
	strcpy(appID, "EA661783134408CD581A4E022D774963581A4E022D774963581A4E022D774963581A4E022D774963581A4E022D7749635E59");
#else
	strcpy(appID, "0DEBC4F751894ADB1609DE32E9ECF7A8581A4E022D774963581A4E022D774963581A4E022D774963581A4E022D7749635E59");//实盘
#endif

	ESForeign::TEsLogPathType path;
	memset(&path, 0, sizeof(path));
	sprintf(path, ".");

	int ret;
	pEsunTraderSpi->m_Api = CreateEsunnyForeignTradeApi(cert, &ret, path, appID);

	CString str1("Esun Trader Initialize..");
	pPubMsg->AddString(str1);
	globalFuncUtil.WriteMsgToLogList("Esun Trader Initialize..");

	pEsunTraderSpi->Initialize();

	//	ESForeign::TEsAddressField addr;
	//#ifdef _TEST
	//	strcpy(addr.Ip, "122.224.221.238");
	//	addr.Port = 27802;
	//#else
	//	strcpy(addr.Ip, "122.224.221.238");
	//	addr.Port = 17901;
	//#endif

		//strcpy(LoginUserTDEsun,"C1171");
		//strcpy(LoginPwdTDEsun,"Lh851171");
		// 连接交易服务器
	str1 = _T("Esun Trader Open..");
	globalFuncUtil.WriteMsgToLogList("Esun Trader Open..");
	pPubMsg->AddString(str1);

	//pEsunTraderSpi->m_bLoginSuccess=false;
	//pEsunTraderSpi->m_bLoginDone=false;
	//bool bRet = pEsunTraderSpi->m_Api->Open(addr);

	//Sleep(3000);
	////CString str(((bRet==0) ? "Trader Open Success" : "Trader Open Failed"));
	////pPubMsg->AddString(str);
	//while(!pEsunTraderSpi->m_bLoginSuccess){
	//	if(!pEsunTraderSpi->m_bLoginDone){
	//		bRet =pEsunTraderSpi->m_Api->Open(addr);
	//	}
	//	Sleep(1000*30);
	//}
	//
 //   if (!bRet)
 //   {
 //       return;
 //   }

	pEsunTraderSpi->m_bLoginSuccess = false;
	pEsunTraderSpi->m_bLoginDone = false;
	int EsunTDAddrType = 0;
	bool bRet = false;
	ESForeign::TEsAddressField addr;
	strcpy(addr.Ip, EsunTdAddr[EsunTDAddrType].serverip);
	addr.Port = EsunTdAddr[EsunTDAddrType].port;
	bRet = pEsunTraderSpi->m_Api->Open(addr);
	while (!pEsunTraderSpi->m_bLoginSuccess) {
		if (pEsunTraderSpi->m_bLoginDone && !pEsunTraderSpi->m_bLoginSuccess) {
			EsunTDAddrType++;
			EsunTDAddrType = EsunTDAddrType % MAXTDADDRNO;
			strcpy(addr.Ip, EsunTdAddr[EsunTDAddrType].serverip);
			addr.Port = EsunTdAddr[EsunTDAddrType].port;

			bRet = pEsunTraderSpi->m_Api->Open(addr);
		}
		else {
			Sleep(10000);
			if (!pEsunTraderSpi->m_bLoginDone && !pEsunTraderSpi->m_bLoginSuccess) {
				EsunTDAddrType++;
				EsunTDAddrType = EsunTDAddrType % MAXTDADDRNO;
				strcpy(addr.Ip, EsunTdAddr[EsunTDAddrType].serverip);
				addr.Port = EsunTdAddr[EsunTDAddrType].port;

				bRet = pEsunTraderSpi->m_Api->Open(addr);
			}
		}
	}
}

void CMyThread::OnCreateCTPTrader(WPARAM wParam, LPARAM lParam) {
	CString pubMsg("Create CTPTrader Thread..");
	pPubMsg->AddString(pubMsg);
	globalFuncUtil.WriteMsgToLogList("Create CTPTrader Thread..");

	//struct tm *ptTm;
	//time_t nowtime;
	//char cur_datetime[20];
	//int nHour,nMin,nSec;

	//int iRet = -1;
	//while(iRet!=0){
	//	time(&nowtime);
	//	ptTm=localtime(&nowtime);
	//	strftime(cur_datetime,20,"%X",ptTm);
	//	sscanf_s(cur_datetime, "%d:%d:%d",&nHour, &nMin, &nSec);
	//	while((nHour==2&&nMin>=30)||(nHour>2&&nHour<8)||(nHour==8&&nMin<35)||(nHour==15&&nMin>30)
	//		||(nHour>15&&nHour<20)||(nHour==20&&nMin<35)||(ptTm->tm_wday==0)||(ptTm->tm_wday==6&&nHour>2)
	//		||(ptTm->tm_wday==1&&nHour<8)){
	//			//当前时间为停市期间,故8:45&20:50后再重新连接
	//			time(&nowtime);
	//			ptTm=localtime(&nowtime);
	//			strftime(cur_datetime,20,"%X",ptTm);
	//			sscanf_s(cur_datetime, "%d:%d:%d",&nHour, &nMin, &nSec);

	//			Sleep(60000);
	//	}
	//	CString csTime(_T("连接CTP当前时间:"));
	//	time(&nowtime);
	//	ptTm=localtime(&nowtime);
	//	char cur_datetimeX[21];
	//	strftime(cur_datetimeX,21,"%Y-%m-%d %X",ptTm);
	//	CString cscur_datetime(cur_datetimeX);
	//	csTime.Append(cscur_datetime);
	//	pPubMsg->AddString(csTime);

	//	char cLog[200];
	//	globalFuncUtil.ConvertCStringToCharArray(csTime,cLog);
	//	globalFuncUtil.WriteMsgToLogList(cLog);

	//	iRet=0;
	//}

#ifdef _TEST
	CThostFtdcTraderApi* pCTPTraderApi = CThostFtdcTraderApi::CreateFtdcTraderApi();
	pThostTraderSpi = new ThostTraderSpi(pCTPTraderApi, "8888", LoginUserTDCTP, LoginPwdCTP);
	pCTPTraderApi->RegisterSpi((CThostFtdcTraderSpi*)pThostTraderSpi);
	pCTPTraderApi->SubscribePublicTopic(THOST_TERT_QUICK);
	pCTPTraderApi->SubscribePrivateTopic(THOST_TERT_QUICK);
	pCTPTraderApi->RegisterFront("tcp://118.242.3.179:43205");
	pCTPTraderApi->Init();
	pCTPTraderApi->Join();
#else
	CThostFtdcTraderApi* pCTPTraderApi = CThostFtdcTraderApi::CreateFtdcTraderApi();
	pThostTraderSpi = new ThostTraderSpi(pCTPTraderApi, "", LoginUserTDCTP, LoginPwdCTP);
	pCTPTraderApi->RegisterSpi((CThostFtdcTraderSpi*)pThostTraderSpi);
	pCTPTraderApi->SubscribePublicTopic(THOST_TERT_QUICK);
	pCTPTraderApi->SubscribePrivateTopic(THOST_TERT_QUICK);
	//"tcp://192.168.1.130:9113"
	pCTPTraderApi->RegisterFront("tcp://106.15.193.26:9113");
	pCTPTraderApi->Init();
	pCTPTraderApi->Join();
#endif
}

void CMyThread::OnRestartCTPTrader(WPARAM wParam, LPARAM lParam) {
#ifdef _TEST
	CThostFtdcTraderApi* pCTPTraderApi = CThostFtdcTraderApi::CreateFtdcTraderApi();
	pThostTraderSpi = new ThostTraderSpi(pCTPTraderApi, "8888", LoginUserTDCTP, LoginPwdCTP);
	pCTPTraderApi->RegisterSpi((CThostFtdcTraderSpi*)pThostTraderSpi);
	pCTPTraderApi->SubscribePublicTopic(THOST_TERT_QUICK);
	pCTPTraderApi->SubscribePrivateTopic(THOST_TERT_QUICK);
	pCTPTraderApi->RegisterFront("tcp://118.242.3.182:47205");
	//pCTPTraderApi->RegisterFront("tcp://218.202.237.33:10102");
	pCTPTraderApi->Init();
	pCTPTraderApi->Join();
#else
	CThostFtdcTraderApi* pCTPTraderApi = CThostFtdcTraderApi::CreateFtdcTraderApi();
	pThostTraderSpi = new ThostTraderSpi(pCTPTraderApi, "", LoginUserTDCTP, LoginPwdCTP);
	pCTPTraderApi->RegisterSpi((CThostFtdcTraderSpi*)pThostTraderSpi);
	pCTPTraderApi->SubscribePublicTopic(THOST_TERT_QUICK);
	pCTPTraderApi->SubscribePrivateTopic(THOST_TERT_QUICK);
	pCTPTraderApi->RegisterFront("tcp://106.15.193.26:9113");
	pCTPTraderApi->Init();
	pCTPTraderApi->Join();
#endif
}

void CMyThread::OnCreateSgitTrader(WPARAM wParam, LPARAM lParam) {
	fstech::CThostFtdcTraderApi* pSgitTraderApi = fstech::CThostFtdcTraderApi::CreateFtdcTraderApi();
	pSgitTraderSpi = new SgitTraderSpi(pSgitTraderApi, "", "06000045", "888888");
	pSgitTraderApi->RegisterSpi((fstech::CThostFtdcTraderSpi*)pSgitTraderSpi);
	pSgitTraderApi->SubscribePublicTopic(fstech::THOST_TERT_QUICK);
	pSgitTraderApi->SubscribePrivateTopic(fstech::THOST_TERT_QUICK);
	pSgitTraderApi->RegisterFront("tcp://140.206.81.6:27776");
	pSgitTraderApi->Init();
	pSgitTraderApi->Join();
}

void CMyThread::CheckAllInstanceStatus(WPARAM wParam, LPARAM lParam) {
	//WaitForSingleObject(RecoverStrategyDlgSem, INFINITE);

	bool AbnormalExit = false;
	//检查各实例是否正常退出,异常退出的实例将在磁盘上保留 实例名.cfg 文件
	CString strPathFile;
	::GetModuleFileName(NULL, strPathFile.GetBuffer(MAX_PATH), MAX_PATH);
	strPathFile.ReleaseBuffer();
	strPathFile = strPathFile.Left(strPathFile.ReverseFind(_T('\\')));
	strPathFile += "\\Strategies";

	int ItermNum = 0;
	std::list<ModelNode>::iterator model_itr;
	if (!ModelList.empty()) {
		for (model_itr = ModelList.begin(); model_itr != ModelList.end(); ++model_itr) {
			CString ModelName(model_itr->ModelName);
			CString csModel1Path;
			csModel1Path.Append(strPathFile);
			csModel1Path += "\\";
			csModel1Path.Append(ModelName);
			std::list<StrategyNode>::iterator strategy_itr;
			if (!model_itr->StrategyList.empty()) {
				for (strategy_itr = model_itr->StrategyList.begin(); strategy_itr != model_itr->StrategyList.end(); ++strategy_itr) {
					CString StrategyName(strategy_itr->StrategyID);
					CString csStrategy1Path;
					csStrategy1Path.Append(csModel1Path);
					csStrategy1Path += "\\";
					csStrategy1Path.Append(StrategyName);
					std::list<StrategyInstanceNode>::iterator instance_itr;
					for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
						CString csInstanceName(instance_itr->InstanceName);

						CString csInstanceCfg;
						csInstanceCfg.Append(csStrategy1Path);
						csInstanceCfg += "\\";
						csInstanceCfg.Append(csInstanceName);
						csInstanceCfg += ".cfg";

						int len = WideCharToMultiByte(CP_ACP, 0, csInstanceCfg, csInstanceCfg.GetLength(), NULL, 0, NULL, NULL);
						char* c_str_filename = new char[len + 1];
						WideCharToMultiByte(CP_ACP, 0, csInstanceCfg, csInstanceCfg.GetLength(), c_str_filename, len, NULL, NULL);
						c_str_filename[len] = '\0';
						if (_access(c_str_filename, 0) != -1) {
							//文件存在,实例属于异常退出
							g_bQryOrderSentByRecoverDlg = true;
							pQryActionThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
							Sleep(500);
							pQryActionThread->PostThreadMessage(WM_QRY_ORDERS, NULL, NULL);

							AbnormalExit = true;
						}
						free(c_str_filename);
					}
				}
			}
		}
	}

	if (AbnormalExit) {
		StrategyRecoverDlg m_StrategyRecoverDlg;
		if (m_StrategyRecoverDlg.DoModal() != IDOK)
		{
			ExitProcess(0);
		}
	}
}

void CMyThread::OnRestartEsunTrader(WPARAM wParam, LPARAM lParam)
{
	CString pubMsg("重启易胜交易服务器连接..");
	pPubMsg->AddString(pubMsg);
	globalFuncUtil.WriteMsgToLogList("重启易胜交易服务器连接..");
	Sleep(1000);

	//	bool bRet=false;
	//	ESForeign::TEsAddressField addr;
	//#ifdef _TEST
	//	strcpy(addr.Ip, "122.224.221.238");
	//	addr.Port = 27802;
	//#else
	//	strcpy(addr.Ip, "122.224.221.238");
	//	addr.Port = 17901;
	//#endif
	//
	//	/*
	//	DWORD retCode;
	//	GetExitCodeThread(pEsunTraderThread->m_hThread,&retCode);
	//	if(retCode==STILL_ACTIVE){
	//		TRACE("Trader Process Active..");
	//		TerminateThread(pEsunTraderThread->m_hThread,0);
	//	}
	//	bool bRet=false;
	//	int ret=-1;
	//
	//		pEsunTraderSpi=new EsunTraderSpi();
	//		ESForeign::TEsCertInfoType cert;
	//		memset(&cert, 0, sizeof(cert));
	//		strcpy(cert, "C7833C7AE258DB7174AADD371C6BCC7C659DEA0C1BE1425CA76D75D96007A7267AFE4CBF4CAA52FD1EFE57BEF5308F2920D09AD1299D8C356F3E5E706B27202DA0268BB292C9F89E910938D056701E382B669E2E03BFC314581A4E022D774963581A4E022D774963581A4E022D774963581A4E022D774963581A4E022D774963D050C47BED3ED850EB24DD23CBAEB147374BDA8B40E4214C581A4E022D774963581A4E022D774963581A4E022D774963581A4E022D774963581A4E022D77496354E606ACCBE47772");
	//		ESForeign::TEsAppIDType appID;
	//		memset(&appID, 0, sizeof(appID));
	//		strcpy(appID, "EA661783134408CD581A4E022D774963581A4E022D774963581A4E022D774963581A4E022D774963581A4E022D7749635E59");
	//		ESForeign::TEsLogPathType path;
	//		memset(&path, 0, sizeof(path));
	//		sprintf(path, "C:\\%s\\", LoginUserTDEsun);
	//		pEsunTraderSpi->m_Api = CreateEsunnyForeignTradeApi(cert, &ret, path, appID);
	//
	//		//重新初始化
	//		pEsunTraderSpi->Initialize();
	//
	//		ret=-1;
	//		*/
	//	struct tm *ptTm;
	//	time_t nowtime;
	//	char cur_datetime[20];
	//	int nHour,nMin,nSec;
	//
	//	bRet = pEsunTraderSpi->m_Api->Open(addr);
	//	while(!pEsunTraderSpi->m_bLoginSuccess){
	//
	//		if(pEsunTraderSpi->m_bLoginDone){
	//			bRet =pEsunTraderSpi->m_Api->Open(addr);
	//		}else{
	//			Sleep(3000);
	//		}
	//
	//		time(&nowtime);
	//		ptTm=localtime(&nowtime);
	//		strftime(cur_datetime,20,"%X",ptTm);
	//		sscanf_s(cur_datetime, "%d:%d:%d",&nHour, &nMin, &nSec);
	//		while((nHour==5&&nMin<30)||(ptTm->tm_wday==0)||(ptTm->tm_wday==6&&nHour>5)
	//			||(ptTm->tm_wday==1&&nHour<6)
	//			){
	//			time(&nowtime);
	//			ptTm=localtime(&nowtime);
	//			strftime(cur_datetime,20,"%X",ptTm);
	//			sscanf_s(cur_datetime, "%d:%d:%d",&nHour, &nMin, &nSec);
	//
	//			Sleep(300000);
	//		}
	//		Sleep(5000);
	//	}
		//return 0;

	int EsunTDAddrType = 0;
	bool bRet = false;
	ESForeign::TEsAddressField addr;
	strcpy(addr.Ip, EsunTdAddr[EsunTDAddrType].serverip);
	addr.Port = EsunTdAddr[EsunTDAddrType].port;
	struct tm* ptTm;
	time_t nowtime;
	char cur_datetime[20];
	int nHour, nMin, nSec;

	bRet = pEsunTraderSpi->m_Api->Open(addr);
	while (!pEsunTraderSpi->m_bLoginSuccess) {
		if (pEsunTraderSpi->m_bLoginDone && !pEsunTraderSpi->m_bLoginSuccess) {
			EsunTDAddrType++;
			EsunTDAddrType = EsunTDAddrType % MAXTDADDRNO;
			strcpy(addr.Ip, EsunTdAddr[EsunTDAddrType].serverip);
			addr.Port = EsunTdAddr[EsunTDAddrType].port;

			bRet = pEsunTraderSpi->m_Api->Open(addr);
		}
		else {
			Sleep(10000);
			if (!pEsunTraderSpi->m_bLoginDone && !pEsunTraderSpi->m_bLoginSuccess) {
				EsunTDAddrType++;
				EsunTDAddrType = EsunTDAddrType % MAXTDADDRNO;
				strcpy(addr.Ip, EsunTdAddr[EsunTDAddrType].serverip);
				addr.Port = EsunTdAddr[EsunTDAddrType].port;

				bRet = pEsunTraderSpi->m_Api->Open(addr);
			}
		}

		time(&nowtime);
		ptTm = localtime(&nowtime);
		strftime(cur_datetime, 20, "%X", ptTm);
		sscanf_s(cur_datetime, "%d:%d:%d", &nHour, &nMin, &nSec);
		while ((nHour == 5 && nMin < 30) || (ptTm->tm_wday == 0) || (ptTm->tm_wday == 6 && nHour > 5)
			|| (ptTm->tm_wday == 1 && nHour < 6)
			) {
			time(&nowtime);
			ptTm = localtime(&nowtime);
			strftime(cur_datetime, 20, "%X", ptTm);
			sscanf_s(cur_datetime, "%d:%d:%d", &nHour, &nMin, &nSec);

			Sleep(300000);
		}
		Sleep(5000);
	}
}

void CMyThread::OnStartProcessActionThread(UINT wParam, LONG lParam)
{
	ScreenDisplayThread* screenProcessAction = new ScreenDisplayThread();
	screenProcessAction->ProcessRspMsg();
}

void CMyThread::ConvertCStringToCharArray(CString csSource, char* rtnCharArray)
{
	int cslen = WideCharToMultiByte(CP_ACP, 0, csSource, csSource.GetLength(), NULL, 0, NULL, NULL);
	char* carray = new char[cslen + 1];
	WideCharToMultiByte(CP_ACP, 0, csSource, csSource.GetLength(), carray, cslen, NULL, NULL);
	carray[cslen] = '\0';
	strcpy(rtnCharArray, carray);
	delete carray;
}

void CMyThread::OnStartStrategy(UINT wParam, LONG lParam) {
	int Row = (int)wParam;//策略实例列表中的行数
	CString csInstanceName = pParamsList->GetItemText(Row, 1);
	bool StrategyInitDone = false;
	////----------------------------------------------------------------
	//int lineCount = pParamsList->GetItemCount();
	WaitForSingleObject(CreateThreadSem, INFINITE);
	int shmindexOk = -1, strategyindex = -1;
	for (int i = 0; i < NMAXSTRA; i++) {
		if (gMapView->strategyarray[i].StrategyStarted == false) {
			shmindexOk = i;
			break;
		}
	}
	for (int i = 0; i < NMAXSTRA; i++) {
		if (gStrategyImpl[i] == NULL || (gStrategyImpl[i] != NULL && gStrategyImpl[i]->GetShmindex() == -1)
			|| (gStrategyImpl[i] != NULL && gStrategyImpl[i]->GetShmindex() >= 0 && gMapView->strategyarray[gStrategyImpl[i]->GetShmindex()].StrategyStarted == false)) {
			strategyindex = i;
			break;
		}
	}
	if (shmindexOk == -1 || strategyindex == -1) {
		char strategyID[50];
		char instanceName[50];
		ConvertCStringToCharArray(StrategyIDShowing, strategyID);
		ConvertCStringToCharArray(csInstanceName, instanceName);
		char data[500];
		sprintf_s(data, "没有空闲的缓存区,创建实例失败,策略名=%s,实例名=%s", strategyID, instanceName);
		CString str(data);
		pPubMsg->AddString(str);

		CString strtemp;
		strtemp.Format(_T("没有空闲的缓存区,创建实例失败!")); // pNMListView->iSubItem
		AfxMessageBox(strtemp, MB_OK, 0);
		return;
	}
	std::list<ModelNode>::iterator model_itr;
	for (model_itr = ModelList.begin(); model_itr != ModelList.end(); ++model_itr) {
		CString ModelName(model_itr->ModelName);
		std::list<StrategyNode>::iterator strategy_itr;
		for (strategy_itr = model_itr->StrategyList.begin(); strategy_itr != model_itr->StrategyList.end(); ++strategy_itr) {
			CString mStrategyID(strategy_itr->StrategyID);
			if (mStrategyID.CompareNoCase(StrategyIDShowing) == 0) {
				std::list<StrategyInstanceNode>::iterator instance_itr;
				for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
					CString mInstanceName(instance_itr->InstanceName);
					if (mInstanceName.CompareNoCase(csInstanceName) == 0) {
						if (ClassNameShowing.CompareNoCase(_T("OpenPriceOpening")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyOpenPriceOpening("");
							StrategyOpenPriceOpening::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (ClassNameShowing.CompareNoCase(_T("ThreeK")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyThreeK("");
							StrategyThreeK::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (ClassNameShowing.CompareNoCase(_T("KDJ")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyKDJ("");
							StrategyKDJ::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (ClassNameShowing.CompareNoCase(_T("Bar")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyBar("");
							StrategyBar::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (ClassNameShowing.CompareNoCase(_T("BandMomnt")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyBandMomnt("");
							StrategyBandMomnt::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (ClassNameShowing.CompareNoCase(_T("LastTTimeOpen")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyLastTTimeOpen("");
							StrategyLastTTimeOpen::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (ClassNameShowing.CompareNoCase(_T("GridOpen")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyGridOpen("");
							StrategyGridOpen::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (ClassNameShowing.CompareNoCase(_T("WaveOpen")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyWaveOpen("");
							StrategyWaveOpen::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (ClassNameShowing.CompareNoCase(_T("WaveOpenAdd")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyWaveOpenAdd("");
							StrategyWaveOpenAdd::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (ClassNameShowing.CompareNoCase(_T("UpDownR")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyUpDownR("");
							StrategyUpDownR::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (ClassNameShowing.CompareNoCase(_T("AvgLine")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyAvgLine("");
							StrategyAvgLine::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (ClassNameShowing.CompareNoCase(_T("AvgDown")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyAvgDown("");
							StrategyAvgDown::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (ClassNameShowing.CompareNoCase(_T("DT")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyDT("");
							StrategyDT::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (ClassNameShowing.CompareNoCase(_T("BaseGridOpen")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyBaseGridOpen("");
							StrategyBaseGridOpen::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (ClassNameShowing.CompareNoCase(_T("BaseGridOpen_plus")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyBaseGridOpen_plus("");
							StrategyBaseGridOpen_plus::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (ClassNameShowing.CompareNoCase(_T("BaoChe")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyBaoChe("");
							StrategyBaoChe::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (ClassNameShowing.CompareNoCase(_T("OpenPriceOpeningNew")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyOpenPriceOpeningNew("");
							StrategyOpenPriceOpeningNew::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (ClassNameShowing.CompareNoCase(_T("OpenPriceOpeningAsia")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyOpenPriceOpeningAsia("");
							StrategyOpenPriceOpeningAsia::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (ClassNameShowing.CompareNoCase(_T("BaseGridMAStopOpen")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyBaseGridMAStopOpen("");
							StrategyBaseGridMAStopOpen::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (ClassNameShowing.CompareNoCase(_T("BaseGridOpenCffex")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyBaseGridOpenCffex("");
							StrategyBaseGridOpenCffex::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (ClassNameShowing.CompareNoCase(_T("BaseGridMAStopOpenCffex")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyBaseGridMAStopOpenCffex("");
							StrategyBaseGridMAStopOpenCffex::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (ClassNameShowing.CompareNoCase(_T("OpenPriceOpeningNight")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyOpenPriceOpeningNight("");
							StrategyOpenPriceOpeningNight::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (ClassNameShowing.CompareNoCase(_T("GridMAStopGTCChop")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyGridMAStopGTCChop("");
							StrategyGridMAStopGTCChop::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else {
							CString strtemp;
							strtemp.Format(_T("策略库中没有此策略,请检查设置!")); // pNMListView->iSubItem
							AfxMessageBox(strtemp, MB_OK, 0);
							return;
						}

						if (gStrategyImpl[strategyindex] != NULL) {
							//赋值策略参数
							std::list<ParamNode>::iterator param_itr;
							for (param_itr = instance_itr->ParamList.begin(); param_itr != instance_itr->ParamList.end(); ++param_itr) {
								ParamNode node;
								strcpy(node.ParamName, param_itr->ParamName);
								strcpy(node.ParamChineseName, param_itr->ParamChineseName);
								strcpy(node.ParamValue, param_itr->ParamValue);
								gStrategyImpl[strategyindex]->SetParamValue(node);
								if (strcmp(node.ParamName, "InstCodeName") == 0) {
									//strcpy(gStrategyImpl[strategyindex]->InstCodeName,node.ParamValue);
									string strInstCodeName(node.ParamValue);
									std::map<string, InstrumentInfo>::iterator itrmap;
									itrmap = InstrumentsSubscribed.find(strInstCodeName);
									if (itrmap != InstrumentsSubscribed.end()) {
										double m_dOneTickx = itrmap->second.OneTick;
										gStrategyImpl[strategyindex]->m_dOneTick = m_dOneTickx;
										gStrategyImpl[strategyindex]->Multiplier = itrmap->second.Multiplier;
									}
									else {
										AfxMessageBox(_T("未订阅该合约,请确认策略合约:") + mInstanceName, MB_OK | MB_ICONQUESTION, 0);
										gStrategyImpl[strategyindex]->SetShmindex(-1);
										StrategyInitDone = false;
										ReleaseSemaphore(CreateThreadSem, 1, NULL);
										return;
									}

									if ((strcmp(itrmap->second.ExchangeID, "CFFEX") == 0 || strcmp(itrmap->second.ExchangeID, "SHFE") == 0
										|| strcmp(itrmap->second.ExchangeID, "DCE") == 0 || strcmp(itrmap->second.ExchangeID, "CZCE") == 0 || strcmp(itrmap->second.ExchangeID, "INE") == 0)
										&& !CTPAPIUsed) {
										AfxMessageBox(_T("当前内盘连接不存在,请确认策略合约:") + mInstanceName, MB_OK | MB_ICONQUESTION, 0);
										gStrategyImpl[strategyindex]->SetShmindex(-1);
										StrategyInitDone = false;
										ReleaseSemaphore(CreateThreadSem, 1, NULL);
										return;
									}
									else if (!(strcmp(itrmap->second.ExchangeID, "CFFEX") == 0 || strcmp(itrmap->second.ExchangeID, "SHFE") == 0
										|| strcmp(itrmap->second.ExchangeID, "DCE") == 0 || strcmp(itrmap->second.ExchangeID, "CZCE") == 0 || strcmp(itrmap->second.ExchangeID, "SGE") == 0
										|| strcmp(itrmap->second.ExchangeID, "INE") == 0) && !EsunAPIUsed) {
										AfxMessageBox(_T("当前外盘连接不存在,请确认策略合约:") + mInstanceName, MB_OK | MB_ICONQUESTION, 0);
										gStrategyImpl[strategyindex]->SetShmindex(-1);
										StrategyInitDone = false;
										ReleaseSemaphore(CreateThreadSem, 1, NULL);
										return;
									}
									else if (strcmp(itrmap->second.ExchangeID, "SGE") == 0 && !SgitAPIUsed) {
										AfxMessageBox(_T("当前黄金交易所连接不存在,请确认策略合约:") + mInstanceName, MB_OK | MB_ICONQUESTION, 0);
										gStrategyImpl[strategyindex]->SetShmindex(-1);
										StrategyInitDone = false;
										ReleaseSemaphore(CreateThreadSem, 1, NULL);
										return;
									}
								}

								if (strcmp(node.ParamName, "BasePrice") == 0 && ClassNameShowing.CompareNoCase(_T("GridOpen")) == 0) {
									CString csBasePriceTip(_T("当前品种:"));
									CString csCode(gStrategyImpl[strategyindex]->InstCodeName);
									CString csPrice(node.ParamValue);
									csBasePriceTip.Append(csCode);
									csBasePriceTip.Append(_T(" 网格基准价为="));
									csBasePriceTip.Append(csPrice);
									if (IDOK == AfxMessageBox(csBasePriceTip + _T("是否确认开启策略"), MB_OKCANCEL | MB_ICONQUESTION, 0)) {
									}
									else {
										gStrategyImpl[strategyindex]->SetShmindex(-1);
										StrategyInitDone = false;
										ReleaseSemaphore(CreateThreadSem, 1, NULL);
										return;
									}
								}
							}
							////更新信息到左边的树形菜单
							instance_itr->shmindex = shmindexOk;
							instance_itr->StrategyStarted = true;
						}
						gStrategyImpl[strategyindex]->SetShmindex(shmindexOk);
						gStrategyImpl[strategyindex]->m_bIsRunning = true;
						gStrategyImpl[strategyindex]->SetStrategyID(strategy_itr->StrategyID);

						int cslen = WideCharToMultiByte(CP_ACP, 0, mInstanceName, mInstanceName.GetLength(), NULL, 0, NULL, NULL);
						char* carray = new char[cslen + 1];
						WideCharToMultiByte(CP_ACP, 0, mInstanceName, mInstanceName.GetLength(), carray, cslen, NULL, NULL);
						carray[cslen] = '\0';
						gStrategyImpl[strategyindex]->SetInstanceName(carray);
						delete carray;

						//策略的其它运行参数的初始化
						StrategyInitDone = true;
						gStrategyImpl[strategyindex]->InitAction();

						//设置实例启动状态
						string strInstanceStarted(InstanceStarted);
						wstring widstr;
						widstr = s2ws(strInstanceStarted);
						pParamsList->SetItemText(Row, 0, (LPCTSTR)widstr.c_str());

						//将策略启动时间写入Trade文件
						TradeLogType trade;
						CString csStrategyAndInstance("");
						csStrategyAndInstance.Append(mStrategyID);
						csStrategyAndInstance.Append(_T("_"));
						csStrategyAndInstance.Append(mInstanceName);
						strcpy(trade.StrategyID, strategy_itr->StrategyID);
						gStrategyImpl[strategyindex]->GetInstanceName(trade.InstanceName);
						//globalFuncUtil.ConvertCStringToCharArray(csStrategyAndInstance,trade.StrategyID);
						//strcpy(trade.InstanceName,strategy_itr->StrategyName);
						struct tm* ptTm;
						time_t nowtime;
						time(&nowtime);
						nowtime += 60;
						ptTm = localtime(&nowtime);
						strftime(trade.tradingday, 20, "%Y%m%d", ptTm);
						strcat(trade.tradingday, "S");
						strftime(trade.tradingtime, 20, "%X", ptTm);
						strcpy(trade.CodeName, gStrategyImpl[strategyindex]->InstCodeName);
						trade.tradeprice = -1;
						trade.submitprice = -1;
						trade.qty = 0;
						trade.fee = 0;
						trade.openorclose = -1;
						trade.openid = -1;
						trade.closeid = -1;
						strcpy(trade.MatchNo, "0000");
						Message logMsg;
						logMsg.type = TRADE_LOG;
						logMsg.AddData(&trade, 0, sizeof(TradeLogType));
						LogMessageList.AddTail(logMsg);
						ReleaseSemaphore(logSemaphore, 1, NULL);
						break;
					}//End for this instance
				}//End for instance loop
			}//End for this strategy
		}//End for strategy loop
	}//End For loop all the model
	//
	if (StrategyInitDone) {
		if (strategyindex == gStrategyImplIndex)gStrategyImplIndex++;
		//gMapView->strategyarray[shmindexOk].StrategyStarted=true;
		//添加启动策略的动作信息到策略消息队列
		OrderTradeMsg order;
		order.shmindex = shmindexOk;
		order.OrderType = ON_TD_STRATEGY_START;
		OrderList.AddTail(order);
		ReleaseSemaphore(DispatchTdSem, 1, NULL);
		gStrategyImpl[strategyindex]->CreateStrategyMapOfView();
		//----Send Strategy to Server
				//rs_msg cur_msg;
				//strategy_reg_info strategy_info;
				//memset(&cur_msg,0,sizeof(rs_msg));
				//cur_msg.info_socket_id=-1;
				//strcpy(cur_msg.info_from,"TL");
				//strcpy(cur_msg.info_to,"server");
				//cur_msg.info_type=STAT_LOGIN_MSG;
				//memset(&strategy_info,0,sizeof(strategy_reg_info));
				//strcpy(strategy_info.strategy_id,gStrategyImpl[strategyindex]->mStrategyID);
				//gStrategyImpl[strategyindex]->GetInstanceName(strategy_info.instance_name);
				//char xAcctID[50];
				//sprintf(xAcctID,"%s-%s",LoginUserTDEsun,LoginUserTDCTP);
				//strcpy(strategy_info.acct_id,xAcctID);
				//char xCodeName[100];
				//sprintf(xCodeName,"%s",gStrategyImpl[strategyindex]->InstCodeName);
				//strcpy(strategy_info.inst_name,xCodeName);
				//memcpy(cur_msg.info_content,&strategy_info,sizeof(strategy_info));
				//cur_msg.info_length=sizeof(strategy_info);
				//pRMTcp->sendlist.AddTail(cur_msg);
				//----End

		gStrategyImpl[strategyindex]->MessageProcess();
	}
	else ReleaseSemaphore(CreateThreadSem, 1, NULL);

	return;
}

void CMyThread::OnReStartStrategyFromRecoverDlg(UINT wParam, LONG lParam) {
	int Row = (int)wParam;//策略实例列表中的行数
	CString csmInstanceName = pStrategyToBeRecover->GetItemText(Row, 2);
	CString csmStrategyName = pStrategyToBeRecover->GetItemText(Row, 0);
	CString csmStrategyID = pStrategyToBeRecover->GetItemText(Row, 1);

	bool StrategyInitDone = false;
	////----------------------------------------------------------------
	//int lineCount = pParamsList->GetItemCount();
	WaitForSingleObject(CreateThreadSem, INFINITE);
	int shmindexOk = -1, strategyindex = -1;
	for (int i = 0; i < NMAXSTRA; i++) {
		if (gMapView->strategyarray[i].StrategyStarted == false) {
			shmindexOk = i;
			break;
		}
	}
	for (int i = 0; i < NMAXSTRA; i++) {
		if (gStrategyImpl[i] == NULL || (gStrategyImpl[i] != NULL && gStrategyImpl[i]->GetShmindex() == -1)
			|| (gStrategyImpl[i] != NULL && gStrategyImpl[i]->GetShmindex() > 0 && gMapView->strategyarray[gStrategyImpl[i]->GetShmindex()].StrategyStarted == false)) {
			strategyindex = i;
			break;
		}
	}
	if (shmindexOk == -1 || strategyindex == -1) {
		char strategyName[50];
		char instanceName[50];
		ConvertCStringToCharArray(csmStrategyName, strategyName);
		ConvertCStringToCharArray(csmInstanceName, instanceName);
		char data[500];
		sprintf_s(data, "没有空闲的缓存区,创建实例失败,策略名=%s,实例名=%s", strategyName, instanceName);
		CString str(data);
		pPubMsg->AddString(str);

		CString strtemp;
		strtemp.Format(_T("没有空闲的缓存区,创建实例失败!")); // pNMListView->iSubItem
		AfxMessageBox(strtemp, MB_OK, 0);
		return;
	}
	std::list<ModelNode>::iterator model_itr;
	for (model_itr = ModelList.begin(); model_itr != ModelList.end(); ++model_itr) {
		CString ModelName(model_itr->ModelName);
		std::list<StrategyNode>::iterator strategy_itr;
		for (strategy_itr = model_itr->StrategyList.begin(); strategy_itr != model_itr->StrategyList.end(); ++strategy_itr) {
			CString mClassName(strategy_itr->StrategyName);
			CString mStrategyID(strategy_itr->StrategyID);
			if (mClassName.CompareNoCase(csmStrategyName) == 0 && mStrategyID.CompareNoCase(csmStrategyID) == 0) {
				std::list<StrategyInstanceNode>::iterator instance_itr;
				for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
					CString mInstanceName(instance_itr->InstanceName);
					if (mInstanceName.CompareNoCase(csmInstanceName) == 0) {
						if (mClassName.CompareNoCase(_T("OpenPriceOpening")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyOpenPriceOpening("");
							StrategyOpenPriceOpening::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (mClassName.CompareNoCase(_T("ThreeK")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyThreeK("");
							StrategyThreeK::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (mClassName.CompareNoCase(_T("KDJ")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyKDJ("");
							StrategyKDJ::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (mClassName.CompareNoCase(_T("Bar")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyBar("");
							StrategyBar::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (mClassName.CompareNoCase(_T("BandMomnt")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyBandMomnt("");
							StrategyBandMomnt::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (mClassName.CompareNoCase(_T("LastTTimeOpen")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyLastTTimeOpen("");
							StrategyLastTTimeOpen::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (mClassName.CompareNoCase(_T("GridOpen")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyGridOpen("");
							StrategyGridOpen::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (mClassName.CompareNoCase(_T("WaveOpen")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyWaveOpen("");
							StrategyWaveOpen::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (mClassName.CompareNoCase(_T("UpDownR")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyUpDownR("");
							StrategyUpDownR::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (mClassName.CompareNoCase(_T("AvgLine")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyAvgLine("");
							StrategyAvgLine::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (mClassName.CompareNoCase(_T("AvgDown")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyAvgDown("");
							StrategyAvgDown::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (mClassName.CompareNoCase(_T("DT")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyDT("");
							StrategyDT::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (mClassName.CompareNoCase(_T("BaseGridOpen")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyBaseGridOpen("");
							StrategyBaseGridOpen::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (mClassName.CompareNoCase(_T("BaseGridOpen_plus")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyBaseGridOpen_plus("");
							StrategyBaseGridOpen_plus::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (mClassName.CompareNoCase(_T("BaoChe")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyBaoChe("");
							StrategyBaoChe::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (mClassName.CompareNoCase(_T("OpenPriceOpeningNew")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyOpenPriceOpeningNew("");
							StrategyOpenPriceOpeningNew::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (mClassName.CompareNoCase(_T("OpenPriceOpeningAsia")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyOpenPriceOpeningAsia("");
							StrategyOpenPriceOpeningAsia::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (mClassName.CompareNoCase(_T("BaseGridMAStopOpen")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyBaseGridMAStopOpen("");
							StrategyBaseGridMAStopOpen::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (mClassName.CompareNoCase(_T("BaseGridOpenCffex")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyBaseGridOpenCffex("");
							StrategyBaseGridOpenCffex::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (mClassName.CompareNoCase(_T("BaseGridMAStopOpenCffex")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyBaseGridMAStopOpenCffex("");
							StrategyBaseGridMAStopOpenCffex::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (mClassName.CompareNoCase(_T("OpenPriceOpeningNight")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyOpenPriceOpeningNight("");
							StrategyOpenPriceOpeningNight::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (mClassName.CompareNoCase(_T("GridMAStopGTCChop")) == 0) {
							gStrategyImpl[strategyindex] = new StrategyGridMAStopGTCChop("");
							StrategyGridMAStopGTCChop::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else {
							CString strtemp;
							strtemp.Format(_T("策略库中没有此策略,请检查设置!")); // pNMListView->iSubItem
							AfxMessageBox(strtemp, MB_OK, 0);
							return;
						}

						CString csInstanceStartTime = pStrategyToBeRecover->GetItemText(Row, 4);
						CString csInstanceEndTime = pStrategyToBeRecover->GetItemText(Row, 5);
						char cInstanceStartTime[30];
						char cInstanceEndTime[30];
						ConvertCStringToCharArray(csInstanceStartTime, cInstanceStartTime);
						ConvertCStringToCharArray(csInstanceEndTime, cInstanceEndTime);

						if (gStrategyImpl[strategyindex] != NULL) {
							//赋值策略参数
							std::list<ParamNode>::iterator param_itr;
							for (param_itr = instance_itr->ParamList.begin(); param_itr != instance_itr->ParamList.end(); ++param_itr) {
								ParamNode node;
								strcpy(node.ParamName, param_itr->ParamName);
								strcpy(node.ParamChineseName, param_itr->ParamChineseName);
								strcpy(node.ParamValue, param_itr->ParamValue);

								if (strcmp(node.ParamName, "InstCodeName") == 0) {
									strcpy(gStrategyImpl[strategyindex]->InstCodeName, node.ParamValue);
									string strInstCodeName(gStrategyImpl[strategyindex]->InstCodeName);
									std::map<string, InstrumentInfo>::iterator itrmap;
									itrmap = InstrumentsSubscribed.find(strInstCodeName);
									if (itrmap != InstrumentsSubscribed.end()) {
										double m_dOneTickx = itrmap->second.OneTick;
										gStrategyImpl[strategyindex]->m_dOneTick = m_dOneTickx;
										gStrategyImpl[strategyindex]->Multiplier = itrmap->second.Multiplier;
									}
									gStrategyImpl[strategyindex]->SetParamValue(node);
								}
								else if (strcmp(node.ParamName, "StartTime") == 0) {
									strcpy(param_itr->ParamValue, cInstanceStartTime);
									strcpy(node.ParamValue, param_itr->ParamValue);
									gStrategyImpl[strategyindex]->SetParamValue(node);
								}
								else if (strcmp(node.ParamName, "EndTime") == 0) {
									strcpy(param_itr->ParamValue, cInstanceEndTime);
									strcpy(node.ParamValue, param_itr->ParamValue);
									gStrategyImpl[strategyindex]->SetParamValue(node);
								}
								else {
									gStrategyImpl[strategyindex]->SetParamValue(node);
								}
							}
							////更新信息到左边的树形菜单
							instance_itr->shmindex = shmindexOk;
							instance_itr->StrategyStarted = true;
						}
						gStrategyImpl[strategyindex]->SetShmindex(shmindexOk);
						gStrategyImpl[strategyindex]->m_bIsRunning = true;
						gStrategyImpl[strategyindex]->SetStrategyID(strategy_itr->StrategyID);

						int cslen = WideCharToMultiByte(CP_ACP, 0, mInstanceName, mInstanceName.GetLength(), NULL, 0, NULL, NULL);
						char* carray = new char[cslen + 1];
						WideCharToMultiByte(CP_ACP, 0, mInstanceName, mInstanceName.GetLength(), carray, cslen, NULL, NULL);
						carray[cslen] = '\0';
						gStrategyImpl[strategyindex]->SetInstanceName(carray);
						delete carray;

						//策略的其它运行参数的初始化
						gStrategyImpl[strategyindex]->InitAction();
						StrategyInitDone = true;
						//实例的开平仓状态文件的恢复
						CString csInstanceCfgFile = pStrategyToBeRecover->GetItemText(Row, 8);
						csInstanceCfgFile.Replace(_T("\\"), _T("\\\\"));
						int cscfglen = WideCharToMultiByte(CP_ACP, 0, csInstanceCfgFile, csInstanceCfgFile.GetLength(), NULL, 0, NULL, NULL);
						char* ccfgarray = new char[cscfglen + 1];
						WideCharToMultiByte(CP_ACP, 0, csInstanceCfgFile, csInstanceCfgFile.GetLength(), ccfgarray, cscfglen, NULL, NULL);
						ccfgarray[cscfglen] = '\0';
						gStrategyImpl[strategyindex]->OpenStrategyMapOfView();
						gStrategyImpl[strategyindex]->RecoverInstance(ccfgarray);
						delete ccfgarray;

						//设置实例启动状态
						string strInstanceStarted(InstanceStarted);
						wstring widstr;
						widstr = s2ws(strInstanceStarted);
						pStrategyToBeRecover->SetItemText(Row, 7, (LPCTSTR)widstr.c_str());

						//将策略启动时间写入Trade文件
						TradeLogType trade;
						CString csStrategyAndInstance("");
						csStrategyAndInstance.Append(mStrategyID);
						csStrategyAndInstance.Append(_T("_"));
						csStrategyAndInstance.Append(mInstanceName);
						strcpy(trade.StrategyID, strategy_itr->StrategyID);
						gStrategyImpl[strategyindex]->GetInstanceName(trade.InstanceName);
						//globalFuncUtil.ConvertCStringToCharArray(csStrategyAndInstance,trade.StrategyID);
						struct tm* ptTm;
						time_t nowtime;
						time(&nowtime);
						nowtime += 60;
						ptTm = localtime(&nowtime);
						strftime(trade.tradingday, 20, "%Y%m%d", ptTm);
						strcat(trade.tradingday, "S");
						strftime(trade.tradingtime, 10, "%X", ptTm);
						strcpy(trade.CodeName, gStrategyImpl[strategyindex]->InstCodeName);
						trade.tradeprice = -1;
						trade.submitprice = -1;
						trade.qty = 0;
						trade.fee = 0;
						trade.openorclose = -1;
						trade.openid = -1;
						trade.closeid = -1;
						strcpy(trade.MatchNo, "0000");
						Message logMsg;
						logMsg.type = TRADE_LOG;
						logMsg.AddData(&trade, 0, sizeof(TradeLogType));
						LogMessageList.AddTail(logMsg);
						ReleaseSemaphore(logSemaphore, 1, NULL);
						break;
					}//End for this instance
				}//End for instance loop
			}//End for this strategy
		}//End for strategy loop
	}//End For loop all the model
	//
	if (StrategyInitDone) {
		//重新保存一下Model.ini
		//Save all Params to file
		FILE* fp2 = fopen("Model.ini", "w+");
		if (fp2 == NULL) { TRACE("Error in Open ini file"); TRACE("%s\n", strerror(errno)); }
		for (std::list<ModelNode>::iterator model_itr = ModelList.begin(); model_itr != ModelList.end(); ++model_itr) {
			std::list<StrategyNode>::iterator strategy_itr;
			for (strategy_itr = model_itr->StrategyList.begin(); strategy_itr != model_itr->StrategyList.end(); ++strategy_itr) {
				std::list<StrategyInstanceNode>::iterator instance_itr;
				for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
					std::list<ParamNode>::iterator param_itr;
					for (param_itr = instance_itr->ParamList.begin(); param_itr != instance_itr->ParamList.end(); ++param_itr) {
						char line[500];
						sprintf(line, "%s,%s,%s,%d,%s,%s,%s,%s\n", model_itr->ModelName, strategy_itr->StrategyName, strategy_itr->StrategyID, strategy_itr->MaxOnHandPositionCount, instance_itr->InstanceName, param_itr->ParamName, param_itr->ParamChineseName, param_itr->ParamValue);
						fwrite(line, strlen(line), 1, fp2);
					}
				}
			}
		}
		fclose(fp2);
		if (strategyindex == gStrategyImplIndex)gStrategyImplIndex++;
		//添加启动策略的动作信息到策略消息队列
		OrderTradeMsg order;
		order.shmindex = shmindexOk;
		order.OrderType = ON_TD_STRATEGY_START;
		OrderList.AddTail(order);
		ReleaseSemaphore(DispatchTdSem, 1, NULL);
		//针对此策略启动成交信息查询
		pQryTradeThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
		Sleep(500);
		pQryTradeThread->PostThreadMessage(WM_QRY_TRADES, (WPARAM)Row, NULL);
		//----Send Stategy Info To Server
		//rs_msg cur_msg;
		//strategy_reg_info strategy_info;
		//memset(&cur_msg,0,sizeof(rs_msg));
		//cur_msg.info_socket_id=-1;
		//strcpy(cur_msg.info_from,"TL");
		//strcpy(cur_msg.info_to,"server");
		//cur_msg.info_type=STAT_LOGIN_MSG;
		//memset(&strategy_info,0,sizeof(strategy_reg_info));
		//strcpy(strategy_info.strategy_id,gStrategyImpl[strategyindex]->mStrategyID);
		//gStrategyImpl[strategyindex]->GetInstanceName(strategy_info.instance_name);
		//char xAcctID[50];
		//sprintf(xAcctID,"%s-%s",LoginUserTDEsun,LoginUserTDCTP);
		//strcpy(strategy_info.acct_id,xAcctID);
		//char xCodeName[100];
		//sprintf(xCodeName,"%s",gStrategyImpl[strategyindex]->InstCodeName);
		//strcpy(strategy_info.inst_name,xCodeName);
		//memcpy(cur_msg.info_content,&strategy_info,sizeof(strategy_info));
		//cur_msg.info_length=sizeof(strategy_info);
		//pRMTcp->sendlist.AddTail(cur_msg);
		//---End

		gStrategyImpl[strategyindex]->MessageProcess();
	}
	else ReleaseSemaphore(CreateThreadSem, 1, NULL);
	return;
}

void CMyThread::OnCreateLogThread(UINT wParam, LONG lParam)
{
	LogMsgThread* myLogMsgThread = new LogMsgThread();
	myLogMsgThread->ProcessLogMsg();
}

void CMyThread::OnCreateTdDispatchThread(UINT wParam, LONG lParam)
{
	DispatchTdMsgThread* myDispathTdMsg = new DispatchTdMsgThread();
	myDispathTdMsg->DispatchTdMsg();
}

void CMyThread::OnStopStrategy(UINT wParam, LONG lParam)
{
	int Row = (int)wParam;//策略实例列表中的行数
	CString csInstanceName = pParamsList->GetItemText(Row, 1);
	bool StrategyInitDone = false;

	std::list<ModelNode>::iterator model_itr;
	for (model_itr = ModelList.begin(); model_itr != ModelList.end(); ++model_itr) {
		CString ModelName(model_itr->ModelName);
		std::list<StrategyNode>::iterator strategy_itr;
		for (strategy_itr = model_itr->StrategyList.begin(); strategy_itr != model_itr->StrategyList.end(); ++strategy_itr) {
			CString mStrategyID(strategy_itr->StrategyID);
			if (mStrategyID.CompareNoCase(StrategyIDShowing) == 0) {
				std::list<StrategyInstanceNode>::iterator instance_itr;
				for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
					CString mInstanceName(instance_itr->InstanceName);
					if (mInstanceName.CompareNoCase(csInstanceName) == 0) {
						if (instance_itr->shmindex >= 0) {
							for (int i = 0; i < gStrategyImplIndex; i++) {
								if (gStrategyImpl[i] != NULL && gStrategyImpl[i]->GetShmindex() == instance_itr->shmindex && gStrategyImpl[i]->m_bIsRunning) {
									//设置实例为未启动状态
									char InstCodeNameTemp[50];
									strcpy(InstCodeNameTemp, gStrategyImpl[i]->InstCodeName);

									string strInstanceNotStarted(InstanceNotStarted);
									wstring widstr;
									widstr = s2ws(strInstanceNotStarted);
									pParamsList->SetItemText(Row, 0, (LPCTSTR)widstr.c_str());
									//添加退出策略的动作信息到策略消息队列
									OrderTradeMsg order;
									order.shmindex = instance_itr->shmindex;
									order.OrderType = ON_TD_STRATEGY_EXIT;
									OrderList.AddTail(order);
									instance_itr->StrategyStarted = false;
									ReleaseSemaphore(DispatchTdSem, 1, NULL);

									//将策略结束时间写入Trade文件
									TradeLogType trade;
									CString csStrategyAndInstance("");
									csStrategyAndInstance.Append(mStrategyID);
									csStrategyAndInstance.Append(_T("_"));
									csStrategyAndInstance.Append(mInstanceName);
									strcpy(trade.StrategyID, strategy_itr->StrategyID);
									gStrategyImpl[i]->GetInstanceName(trade.InstanceName);
									//globalFuncUtil.ConvertCStringToCharArray(csStrategyAndInstance,trade.StrategyID);
									struct tm* ptTm;
									time_t nowtime;
									time(&nowtime);
									nowtime += 60;
									ptTm = localtime(&nowtime);
									strftime(trade.tradingday, 20, "%Y%m%d", ptTm);
									strcat(trade.tradingday, "E");
									strftime(trade.tradingtime, 10, "%X", ptTm);
									strcpy(trade.CodeName, InstCodeNameTemp);
									trade.tradeprice = -1;
									trade.submitprice = -1;
									trade.qty = 0;
									trade.fee = 0;
									trade.openid = -1;
									trade.closeid = -1;
									trade.openorclose = -1;
									strcpy(trade.MatchNo, "0");
									Message logMsg;
									logMsg.type = TRADE_LOG;
									logMsg.AddData(&trade, 0, sizeof(TradeLogType));
									LogMessageList.AddTail(logMsg);
									ReleaseSemaphore(logSemaphore, 1, NULL);

									break;
								}
							}
						}
					}
				}
			}
		}
	}
}

//这个函数最好写在窗口里，待整理
void CMyThread::OnStop(UINT wParam, LONG lParam)
{
	if (pEsunTraderSpi && (pEsunTraderSpi != 0))
	{
		//pEsunTraderSpi->Release();
	}

	if (pMdSpi && (pMdSpi != 0))
	{
		//pMdApi->Release();
	}
}

void CMyThread::OnSendQryOrdersMsg(WPARAM wParam, LPARAM lParam)
{
	//char *inserttime="15:30:00";
	//strcpy_s(req.InsertTimeStart,inserttime);
	//int iResult = pTraderApi->ReqQryOrder(&req,++iRequestID);
	CString str("委托单查询");
	pPubMsg->AddString(str);

	WaitForSingleObject(ambushordersem, INFINITE);
	std::list<InsertOrderField>::iterator openorder_it;
	if (!ambushOrderList.empty()) {
		for (openorder_it = ambushOrderList.begin(); openorder_it != ambushOrderList.end(); ++openorder_it) {
			OrderDetailField mOrder;
			strcpy(mOrder.CommodityNo, openorder_it->CommodityNo);
			strcpy(mOrder.InstrumentID, openorder_it->InstrumentID);
			mOrder.OrderId = -1;
			strcpy(mOrder.InsertDateTime, "");
			mOrder.Direction = openorder_it->Direction;
			mOrder.OrderStatus = MORDER_AMBUSH;
			mOrder.SubmitPrice = openorder_it->OrderPrice;
			mOrder.VolumeTotalOriginal = openorder_it->OrderVol;
			mOrder.TradePrice = -1;
			mOrder.VolumeTraded = 0;

			mOrder.OrderLocalRef = -1;
			mOrder.FrontID = -1;
			mOrder.SessionID = -1;
			Message pOrderMsg;

			pOrderMsg.type = ON_RSP_QRY_ORDER;

			pOrderMsg.AddData(&mOrder, 0, sizeof(OrderDetailField));
			ScreenDisplayMsgList.AddTail(pOrderMsg);
			ReleaseSemaphore(ScreenDisplaySem, 1, NULL);
		}
	}
	ReleaseSemaphore(ambushordersem, 1, NULL);

	bool CTPQrySent = false;
	if (StrategyIDShowing.CompareNoCase(_T("None")) == 0) {
		std::map<string, InstrumentInfo>::iterator map_itr;
		for (map_itr = InstrumentsSubscribed.begin(); map_itr != InstrumentsSubscribed.end(); ++map_itr) {
			if (strcmp(map_itr->second.ExchangeID, "CFFEX") == 0 || strcmp(map_itr->second.ExchangeID, "SHFE") == 0
				|| strcmp(map_itr->second.ExchangeID, "DCE") == 0 || strcmp(map_itr->second.ExchangeID, "CZCE") == 0 || strcmp(map_itr->second.ExchangeID, "SGE") == 0
				|| strcmp(map_itr->second.ExchangeID, "INE") == 0) {
				if (strcmp(map_itr->second.ExchangeID, "SGE") == 0 && pSgitTraderSpi != NULL) {
					pSgitTraderSpi->ReqQryOrder(map_itr->second.ExchangeID, map_itr->second.CommodityNo, map_itr->second.InstrumentID);
				}
				else {
					if (CTPAPIUsed && !CTPQrySent && pThostTraderSpi != NULL) {
						pThostTraderSpi->ReqQryOrder(map_itr->second.ExchangeID, map_itr->second.CommodityNo, map_itr->second.InstrumentID);
						CTPQrySent = true;
					}
				}
			}
			else {
				if (EsunAPIUsed && pEsunTraderSpi != NULL)pEsunTraderSpi->ReqQryOrder(map_itr->second.ExchangeID, map_itr->second.CommodityNo, map_itr->second.InstrumentID);
			}

			//pEsunTraderSpi->ReqQryOrder(map_itr->second.ExchangeID,map_itr->second.CommodityNo,map_itr->second.InstrumentID);
		}
	}
	else {
		std::map<string, string> strategyinstmap;
		bool find = false;
		std::list<ModelNode>::iterator model_itr;
		for (model_itr = ModelList.begin(); model_itr != ModelList.end(); ++model_itr) {
			std::list<StrategyNode>::iterator strategy_itr;
			for (strategy_itr = model_itr->StrategyList.begin(); strategy_itr != model_itr->StrategyList.end(); ++strategy_itr) {
				CString mClassName(strategy_itr->StrategyName);
				CString mStrategyID(strategy_itr->StrategyID);
				if (mClassName.CompareNoCase(ClassNameShowing) == 0 && mStrategyID.CompareNoCase(StrategyIDShowing) == 0) {
					std::list<StrategyInstanceNode>::iterator instance_itr;
					for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
						CString mInstanceName(instance_itr->InstanceName);
						std::list<ParamNode>::iterator param_it;
						for (param_it = instance_itr->ParamList.begin(); param_it != instance_itr->ParamList.end(); ++param_it) {
							if (strcmp(param_it->ParamName, "InstCodeName") == 0) {
								string strinst(param_it->ParamValue);
								map<string, string>::iterator iter;
								iter = strategyinstmap.find(strinst);
								if (iter == strategyinstmap.end()) {
									strategyinstmap.insert(std::pair<string, string>(strinst, strinst));
								}
							}
						}
					}
					break;
					find = true;
				}
			}//End for Strategy loop
			if (find)break;
		}//End For Model

		std::map<string, string>::iterator map_itr1;
		for (map_itr1 = strategyinstmap.begin(); map_itr1 != strategyinstmap.end(); ++map_itr1) {
			std::map<string, InstrumentInfo>::iterator map_itrall;
			map_itrall = InstrumentsSubscribed.find(map_itr1->first);
			if (map_itrall != InstrumentsSubscribed.end()) {
				//TRACE("%s,%s,%s\n",map_itrall->second.ExchangeID,map_itrall->second.CommodityNo,map_itrall->second.InstrumentID);
				if (strcmp(map_itrall->second.ExchangeID, "CFFEX") == 0 || strcmp(map_itrall->second.ExchangeID, "SHFE") == 0
					|| strcmp(map_itrall->second.ExchangeID, "DCE") == 0 || strcmp(map_itrall->second.ExchangeID, "CZCE") == 0 || strcmp(map_itrall->second.ExchangeID, "SGE") == 0
					|| strcmp(map_itrall->second.ExchangeID, "INE") == 0) {
					if (strcmp(map_itrall->second.ExchangeID, "SGE") == 0 && pSgitTraderSpi != NULL) {
						pSgitTraderSpi->ReqQryOrder(map_itrall->second.ExchangeID, map_itrall->second.CommodityNo, map_itrall->second.InstrumentID);
					}
					else {
						if (CTPAPIUsed && !CTPQrySent && pThostTraderSpi != NULL) {
							pThostTraderSpi->ReqQryOrder(map_itrall->second.ExchangeID, map_itrall->second.CommodityNo, map_itrall->second.InstrumentID);
							CTPQrySent = true;
						}
					}
				}
				else {
					if (EsunAPIUsed && pEsunTraderSpi != NULL)pEsunTraderSpi->ReqQryOrder(map_itrall->second.ExchangeID, map_itrall->second.CommodityNo, map_itrall->second.InstrumentID);
				}
			}
		}
	}
}

void CMyThread::OnSendRecoverQryOrdersMsg(WPARAM wParam, LPARAM lParam)
{
	int Row = (int)wParam;//策略实例列表中的行数
	CString csStrategyName = pStrategyToBeRecover->GetItemText(Row, 0);//
	CString csInstanceName = pStrategyToBeRecover->GetItemText(Row, 2);//
	CString csInstName = pStrategyToBeRecover->GetItemText(Row, 3);//
	CString csStopTime = pStrategyToBeRecover->GetItemText(Row, 6);
	char cInstName[50];
	char cStopTime[21];
	ConvertCStringToCharArray(csInstName, cInstName);
	ConvertCStringToCharArray(csStopTime, cStopTime);

	string strInstName(cInstName);

	bool find = false;
	map<string, InstrumentInfo>::iterator iter;
	for (iter = InstrumentsSubscribed.begin(); iter != InstrumentsSubscribed.end(); iter++) {
		if (strcmp(iter->first.c_str(), strInstName.c_str()) == 0 || iter->first.find_first_of(strInstName, 0) >= 0) {
			if (strcmp(iter->second.ExchangeID, "CFFEX") == 0 || strcmp(iter->second.ExchangeID, "SHFE") == 0
				|| strcmp(iter->second.ExchangeID, "DCE") == 0 || strcmp(iter->second.ExchangeID, "CZCE") == 0 || strcmp(iter->second.ExchangeID, "SGE") == 0
				|| strcmp(iter->second.ExchangeID, "INE") == 0) {
				if (strcmp(iter->second.ExchangeID, "SGE") == 0 && pSgitTraderSpi != NULL) {
					pSgitTraderSpi->ReqQryOrder(iter->second.ExchangeID, iter->second.CommodityNo, iter->second.InstrumentID);
				}
				else {
					if (CTPAPIUsed && pThostTraderSpi != NULL)pThostTraderSpi->ReqQryOrder(iter->second.ExchangeID, iter->second.CommodityNo, iter->second.InstrumentID);
				}
			}
			else {
				if (EsunAPIUsed && pEsunTraderSpi != NULL)pEsunTraderSpi->ReqQryOrder(iter->second.ExchangeID, iter->second.CommodityNo, iter->second.InstrumentID);
			}
			find = true;
			break;
		}
	}

	if (!find) {
		CString str("合约未订阅,无法查询委托信息,");
		str.Append(csStrategyName);
		str.Append(_T(","));
		str.Append(csInstanceName);
		pPubMsg->AddString(str);
	}
}
void CMyThread::OnSendQryTradesMsg(WPARAM wParam, LPARAM lParam)
{
	int Row = (int)wParam;//策略实例列表中的行数
	CString csStrategyName = pStrategyToBeRecover->GetItemText(Row, 0);//
	CString csInstanceName = pStrategyToBeRecover->GetItemText(Row, 2);//
	CString csInstName = pStrategyToBeRecover->GetItemText(Row, 3);//
	CString csStopTime = pStrategyToBeRecover->GetItemText(Row, 6);
	char cInstName[50];
	char cStopTime[21];
	ConvertCStringToCharArray(csInstName, cInstName);
	ConvertCStringToCharArray(csStopTime, cStopTime);

	string strInstName(cInstName);

	bool find = false;
	map<string, InstrumentInfo>::iterator iter;
	for (iter = InstrumentsSubscribed.begin(); iter != InstrumentsSubscribed.end(); iter++) {
		if (strcmp(iter->first.c_str(), strInstName.c_str()) == 0 || strstr(iter->first.c_str(), strInstName.c_str())) {
			if (strcmp(iter->second.ExchangeID, "CFFEX") == 0 || strcmp(iter->second.ExchangeID, "SHFE") == 0
				|| strcmp(iter->second.ExchangeID, "DCE") == 0 || strcmp(iter->second.ExchangeID, "CZCE") == 0 || strcmp(iter->second.ExchangeID, "SGE") == 0
				|| strcmp(iter->second.ExchangeID, "INE") == 0) {
				string strStopTime(cStopTime);
				string strStopTimePart = strStopTime.substr(strStopTime.find_last_of(" ") + 1, strStopTime.length() - strStopTime.find_last_of(" "));
				char cStopTimePart[9];
				strcpy(cStopTimePart, strStopTimePart.c_str());
				if (strcmp(iter->second.ExchangeID, "SGE") == 0 && pSgitTraderSpi != NULL) {
					pSgitTraderSpi->ReqQryTrade(iter->second.ExchangeID, iter->second.CommodityNo, iter->second.InstrumentID, cStopTimePart);
				}
				else {
					if (CTPAPIUsed && pThostTraderSpi != NULL)pThostTraderSpi->ReqQryTrade(iter->second.ExchangeID, iter->second.CommodityNo, iter->second.InstrumentID, cStopTimePart);
				}
			}
			else {
				if (EsunAPIUsed && pEsunTraderSpi != NULL)pEsunTraderSpi->ReqQryTrade(iter->second.ExchangeID, iter->second.CommodityNo, iter->second.InstrumentID, cStopTime);
			}
			find = true;
			break;
		}
	}
	if (!find) {
		CString str("合约未订阅,无法查询成交信息,");
		str.Append(csStrategyName);
		str.Append(_T(","));
		str.Append(csInstanceName);
		pPubMsg->AddString(str);
	}
}

void CMyThread::OnTDLostSendQryTradesMsg(WPARAM wParam, LPARAM lParam)
{
	std::list<ModelNode>::iterator model_itr;
	for (model_itr = ModelList.begin(); model_itr != ModelList.end(); ++model_itr) {
		std::list<StrategyNode>::iterator strategy_itr;
		for (strategy_itr = model_itr->StrategyList.begin(); strategy_itr != model_itr->StrategyList.end(); ++strategy_itr) {
			std::list<StrategyInstanceNode>::iterator instance_itr;
			for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
				if (instance_itr->StrategyStarted) {
					std::list<ParamNode>::iterator param_itr;
					for (param_itr = instance_itr->ParamList.begin(); param_itr != instance_itr->ParamList.end(); ++param_itr) {
						if (strcmp(param_itr->ParamName, "InstCodeName") == 0) {
							string strInstName(param_itr->ParamValue);

							map<string, InstrumentInfo>::iterator iter;
							iter = InstrumentsSubscribed.find(strInstName);
							if (iter != InstrumentsSubscribed.end()) {
								if (EsunAPIUsed && pEsunTraderSpi != NULL)pEsunTraderSpi->ReqQryTrade(iter->second.ExchangeID, iter->second.CommodityNo, iter->second.InstrumentID, "00:00:01");
							}
							else {
								CString str("TD掉线重连,因合约未订阅,无法查询成交信息,");
								CString csStrategyName(strategy_itr->StrategyName);
								CString csInstanceName(instance_itr->InstanceName);
								str.Append(csStrategyName);
								str.Append(_T(","));
								str.Append(csInstanceName);
								pPubMsg->AddString(str);
							}
						}
					}
				}//End if Instance is Started
			}
		}
	}
}

void CMyThread::OnSendQryMoneyMsg(WPARAM wParam, LPARAM lParam)
{
	//char *inserttime="15:30:00";
	//strcpy_s(req.InsertTimeStart,inserttime);
	//int iResult = pTraderApi->ReqQryOrder(&req,++iRequestID);
	CString str("可用资金查询");
	pPubMsg->AddString(str);
	if (pEsunTraderSpi != NULL && EsunAPIUsed)pEsunTraderSpi->ReqQryMoney();
	if (pThostTraderSpi != NULL && CTPAPIUsed)pThostTraderSpi->ReqQryTradingAccount();
	//	if(pSgitTraderSpi!=NULL&&SgitAPIUsed)pSgitTraderSpi->ReqQryTradingAccount();
}

void CMyThread::OnSendQryPositionDetailsMsg(WPARAM wParam, LPARAM lParam)
{
	bool CTPQuerySent = false;
	CString str("持仓明细查询");
	pPubMsg->AddString(str);
	std::map<string, InstrumentInfo>::iterator map_itr;
	for (map_itr = InstrumentsSubscribed.begin(); map_itr != InstrumentsSubscribed.end(); ++map_itr) {
		if (strcmp(map_itr->second.ExchangeID, "CFFEX") == 0 || strcmp(map_itr->second.ExchangeID, "SHFE") == 0
			|| strcmp(map_itr->second.ExchangeID, "DCE") == 0 || strcmp(map_itr->second.ExchangeID, "CZCE") == 0 || strcmp(map_itr->second.ExchangeID, "SGE") == 0
			|| strcmp(map_itr->second.ExchangeID, "INE") == 0) {
			if (strcmp(map_itr->second.ExchangeID, "SGE") == 0 && pSgitTraderSpi != NULL) {
				pSgitTraderSpi->ReqQryPosition(map_itr->second.ExchangeID, map_itr->second.CommodityNo, map_itr->second.InstrumentID);
			}
			else {
				if (CTPAPIUsed && !CTPQuerySent && pThostTraderSpi != NULL) {
					pThostTraderSpi->ReqQryPosition(map_itr->second.ExchangeID, map_itr->second.CommodityNo, "");
					CTPQuerySent = true;
				}
			}
		}
		else {
			if (EsunAPIUsed && pEsunTraderSpi != NULL)pEsunTraderSpi->ReqQryPosition(map_itr->second.ExchangeID, map_itr->second.CommodityNo, map_itr->second.InstrumentID);
		}
	}
}

void CMyThread::OnWithdrawOrder(WPARAM wParam, LPARAM lParam)
{
	//int orderId=(int)wParam;
	int EsunApiOrNot = (int)wParam;
	OrderDetailField* pOrderField = (OrderDetailField*)lParam;
	if (EsunApiOrNot == 1) {
		if (EsunAPIUsed && pEsunTraderSpi != NULL)pEsunTraderSpi->ReqOrderDelete(pOrderField->OrderId);
	}
	else if (EsunApiOrNot == 0) {
		if (CTPAPIUsed && pThostTraderSpi != NULL)pThostTraderSpi->ReqOrderDelete(pOrderField);
	}
	else if (EsunApiOrNot == 3 && pEsunTraderSpi != NULL) {
		pSgitTraderSpi->ReqOrderDelete(pOrderField);
	}
	//int iResult = pTraderApi->ReqOrderAction(&req, ++iRequestID);
	//cerr << "--->>> 报单操作请求: "  << iResult << ((iResult == 0) ? ", 成功" : ", 失败") << endl;
}
///--------------------------------------------------------------
// CMyThread message handlers
void CMyThread::OnCreateEsunMd(WPARAM wParam, LPARAM lParam)
{
	CString pubMsg("Create Esun Md Thread..");
	pMdPubMsg->AddString(pubMsg);

	pEsunMdSpi = new CEsunMdSpi();

	int iRet = 0;
	//初始化密钥
	iRet = pEsunMdSpi->InitSecretKey("2F7971747F6D27664656516E463EE402046C59FD", 1);
	//连接行情服务器
#ifdef _TEST
	iRet = pEsunMdSpi->Connect("122.224.221.238", 27801);
#else
	iRet = pEsunMdSpi->Connect("122.224.221.238", 17902);
#endif

	iRet = pEsunMdSpi->Login(LoginMDUser, LoginMDPwd);
	Sleep(3000);
	while (!pEsunMdSpi->m_bLoginSuccess) {
		if (pEsunMdSpi->m_bLoginDone && !pEsunMdSpi->m_bLoginSuccess) {
			iRet = pEsunMdSpi->Login(LoginMDUser, LoginMDPwd);
		}
		else {
			Sleep(5000);
			if (!pEsunMdSpi->m_bLoginDone && !pEsunMdSpi->m_bLoginSuccess) {
#ifdef _TEST
				iRet = pEsunMdSpi->Connect("122.224.221.238", 27801);
#else
				iRet = pEsunMdSpi->Connect("122.224.221.238", 17902);
#endif
				iRet = pEsunMdSpi->Login(LoginMDUser, LoginMDPwd);
			}
		}
	}

	CString str1(((iRet == 0) ? "Esun MD Login Success" : "Esun MD Login Failed"));
	pMdPubMsg->AddString(str1);

	Sleep(30000);
	time_t t = time(NULL);
	struct tm* tm1 = localtime(&t);
	char InstrumentID[32];
	int year = (tm1->tm_year + 1900) % 100;
	int mon = tm1->tm_mon + 1;
	/*
	for(int i=0;i<1;i++)
	{
		mon++;
		if(mon>12){
			mon=1;
			year++;
		}
		int date = (year%100)*100+mon;
		sprintf(InstrumentID, "NYMEX CL %d", date);
		iRet=pEsunMdSpi->AddReqStk("NYMEX", InstrumentID, 1);
		int m=1;
	}
	*/
	//int date = 1706;
	//sprintf(InstrumentID, "NYMEX CL %d", date);
	//iRet=pEsunMdSpi->AddReqStk("NYMEX", InstrumentID, 1);

	//std::list<string>::iterator inst_itr;
	char tipMsg[100];
	std::list<InstrumentsName>::iterator inst_itr;
	for (inst_itr = OverSeaInstSubscribed.begin(); inst_itr != OverSeaInstSubscribed.end(); inst_itr++) {
		//InstrumentsName inst=instlistscribed.front();

		if (strcmp(inst_itr->ExchangeID, "COMEX") == 0) {
			sprintf(InstrumentID, "%s %s %s", inst_itr->ExchangeID, inst_itr->CommodityNo, inst_itr->InstrumentID);
		}
		else {
			sprintf(InstrumentID, "%s %s %s", inst_itr->ExchangeID, inst_itr->CommodityNo, inst_itr->InstrumentID);
		}
		iRet = pEsunMdSpi->AddReqStk(inst_itr->ExchangeID, InstrumentID, 1);
		if (iRet == 0) {
			sprintf(tipMsg, "成功添加合约:%s", InstrumentID);
		}
		else {
			sprintf(tipMsg, "添加合约:%s 失败,Error Ret Code=%d", InstrumentID, iRet);
		}
		CString csRet(tipMsg);
		pMdPubMsg->AddString(csRet);
		//instlistscribed.pop_front();
	}
	//iRet = pEsunMdSpi->AddReqStk("NYMEX","NYMEX CL1705",1);
	iRet = pEsunMdSpi->SendReqStk();
	CString str(((iRet == 0) ? "易胜行情订阅合约成功" : "易胜行情订阅合约失败"));
	pMdPubMsg->AddString(str);

	//iRet = pEsunMdSpi->RequestQuot("NYMEX","NYMEX CL1705",TRUE);
	//Sleep(10000);
}

void CMyThread::OnRestartEsunMd(WPARAM wParam, LPARAM lParam)
{
	CString pubMsg("重连易胜行情连接..");
	pPubMsg->AddString(pubMsg);

	globalFuncUtil.WriteMsgToLogList("重连易胜行情连接..");

	struct tm* ptTm;
	time_t nowtime;
	char cur_datetime[20];
	int nHour, nMin, nSec;

	int iRet = -1;
	while (iRet != 0) {
		time(&nowtime);
		ptTm = localtime(&nowtime);
		strftime(cur_datetime, 20, "%X", ptTm);
		sscanf_s(cur_datetime, "%d:%d:%d", &nHour, &nMin, &nSec);
		while ((nHour == 2 && nMin >= 30) || (nHour >= 3 && nHour < 5) || (nHour == 5 && nMin < 30)) {
			//当前时间为停市期间,故5:30后再重新连接
			time(&nowtime);
			ptTm = localtime(&nowtime);
			strftime(cur_datetime, 20, "%X", ptTm);
			sscanf_s(cur_datetime, "%d:%d:%d", &nHour, &nMin, &nSec);

			Sleep(300000);
		}
		CString csTime(_T("重连易胜当前时间:"));
		time(&nowtime);
		ptTm = localtime(&nowtime);
		char cur_datetimeX[21];
		strftime(cur_datetimeX, 21, "%Y-%m-%d %X", ptTm);
		CString cscur_datetime(cur_datetimeX);
		csTime.Append(cscur_datetime);
		pPubMsg->AddString(csTime);

		//初始化密钥
		iRet = pEsunMdSpi->InitSecretKey("2F7971747F6D27664656516E463EE402046C59FD", 1);
		//连接行情服务器
		int EsunMDAddrType = 0;
		iRet = pEsunMdSpi->Connect(EsunMdAddr[EsunMDAddrType].serverip, EsunMdAddr[EsunMDAddrType].port);
		iRet = pEsunMdSpi->Login(LoginMDUser, LoginMDPwd);
		Sleep(3000);
		while (!pEsunMdSpi->m_bLoginSuccess) {
			if (pEsunMdSpi->m_bLoginDone && !pEsunMdSpi->m_bLoginSuccess) {
				iRet = pEsunMdSpi->Login(LoginMDUser, LoginMDPwd);
			}
			else {
				Sleep(5000);
				if (!pEsunMdSpi->m_bLoginDone && !pEsunMdSpi->m_bLoginSuccess) {
					EsunMDAddrType++;
					EsunMDAddrType = EsunMDAddrType % MAXMDADDRNO;
					iRet = pEsunMdSpi->Connect(EsunMdAddr[EsunMDAddrType].serverip, EsunMdAddr[EsunMDAddrType].port);

					iRet = pEsunMdSpi->Login(LoginMDUser, LoginMDPwd);
				}
			}
		}

		CString str1(((iRet == 0) ? "易胜行情重连登陆成功" : "易胜行情重连登陆失败"));
		pPubMsg->AddString(str1);

		if (iRet == 0)
			globalFuncUtil.WriteMsgToLogList("易胜行情重连登陆成功..");
		else
			globalFuncUtil.WriteMsgToLogList("易胜行情重连登陆失败..");

		Sleep(30000);

		char InstrumentID[32];
		char tipMsg[100];
		std::list<InstrumentsName>::iterator inst_itr;
		for (inst_itr = OverSeaInstSubscribed.begin(); inst_itr != OverSeaInstSubscribed.end(); inst_itr++) {
			if (strcmp(inst_itr->ExchangeID, "COMEX") == 0) {
				sprintf(InstrumentID, "%s %s %s", inst_itr->ExchangeID, inst_itr->CommodityNo, inst_itr->InstrumentID);
			}
			else {
				sprintf(InstrumentID, "%s %s %s", inst_itr->ExchangeID, inst_itr->CommodityNo, inst_itr->InstrumentID);
			}
			iRet = pEsunMdSpi->AddReqStk(inst_itr->ExchangeID, InstrumentID, 1);
			if (iRet == 0) {
				sprintf(tipMsg, "成功添加合约:%s", InstrumentID);
			}
			else {
				sprintf(tipMsg, "添加合约:%s 失败,Error Ret Code=%d", InstrumentID, iRet);
			}
			CString csRet(tipMsg);
			pPubMsg->AddString(csRet);
		}
		iRet = pEsunMdSpi->SendReqStk();
		CString str(((iRet == 0) ? "易胜行情重连订阅合约成功" : "易胜行情重连订阅合约失败"));
		pPubMsg->AddString(str);
		//每隔1分钟就重连一次直至成功
		if (iRet == 0)
			globalFuncUtil.WriteMsgToLogList("易胜行情重连订阅合约成功..");
		else
			globalFuncUtil.WriteMsgToLogList("易胜行情重连订阅合约失败..");
		Sleep(60000);
	}
	//iRet = pEsunMdSpi->RequestQuot("NYMEX","NYMEX CL1705",TRUE);
	//Sleep(10000);
}

void CMyThread::OnCreateCTPMd(UINT wParam, LONG lParam)
{
	char CTP_FRONT_MD_ADDR[100];
	strcpy(CTP_FRONT_MD_ADDR, "tcp://118.242.3.179:43173");
	iInstrumentID = 0;
	std::list<InstrumentsName>::iterator inst_itr;
	for (inst_itr = DomesticInstSubscribed.begin(); inst_itr != DomesticInstSubscribed.end(); inst_itr++) {
		ppInstrumentID[iInstrumentID] = new char[20];
		string strInst(inst_itr->InstrumentID);
		strcpy(ppInstrumentID[iInstrumentID], strInst.c_str());
		iInstrumentID++;
	}

	CThostFtdcMdApi* pMdApi = CThostFtdcMdApi::CreateFtdcMdApi();
	pThostMdSpi = new CThostMdSpi(pMdApi, "7070", LoginMDUserCTP, LoginMDPwdCTP);
	pMdApi->RegisterSpi((CThostFtdcMdSpi*)pThostMdSpi);
	pMdApi->RegisterFront(CTP_FRONT_MD_ADDR);
	pMdApi->Init();
	pMdApi->Join();

	TRACE("CTP MD End.");
}

void CMyThread::OnCreateSgitMd(UINT wParam, LONG lParam)
{
	SgitiInstrumentID = 0;
	std::list<InstrumentsName>::iterator inst_itr;
	for (inst_itr = SGEInstSubscribed.begin(); inst_itr != SGEInstSubscribed.end(); inst_itr++) {
		SgitppInstrumentID[SgitiInstrumentID] = new char[20];
		string strInst(inst_itr->InstrumentID);
		strcpy(SgitppInstrumentID[SgitiInstrumentID], strInst.c_str());
		SgitiInstrumentID++;
	}

	char SGIT_FRONT_MD_ADDR[100];
	strcpy(SGIT_FRONT_MD_ADDR, "tcp://140.206.81.6:27777");
	fstech::CThostFtdcMdApi* pMdApi = fstech::CThostFtdcMdApi::CreateFtdcMdApi();
	pSgitMdSpi = new CSgitMdSpi(pMdApi, "06000045", "06000045", "888888");
	pMdApi->RegisterSpi((fstech::CThostFtdcMdSpi*)pSgitMdSpi);
	pMdApi->RegisterFront(SGIT_FRONT_MD_ADDR);
	pMdApi->Init();
	pMdApi->Join();
}

void CMyThread::OnRestartCTPMd(WPARAM wParam, LPARAM lParam)
{
	CString pubMsg("重连CTP行情连接..");
	pPubMsg->AddString(pubMsg);

	struct tm* ptTm;
	time_t nowtime;
	char cur_datetime[20];
	int nHour, nMin, nSec;

	int iRet = -1;
	while (iRet != 0) {
		time(&nowtime);
		ptTm = localtime(&nowtime);
		strftime(cur_datetime, 20, "%X", ptTm);
		sscanf_s(cur_datetime, "%d:%d:%d", &nHour, &nMin, &nSec);
		while ((nHour == 2 && nMin >= 30) || (nHour > 2 && nHour < 8) || (nHour == 8 && nMin < 30)
			|| (nHour > 15 && nHour < 20) || (nHour == 20 && nMin < 30)) {
			//当前时间为停市期间,故8:30&20:30后再重新连接
			time(&nowtime);
			ptTm = localtime(&nowtime);
			strftime(cur_datetime, 20, "%X", ptTm);
			sscanf_s(cur_datetime, "%d:%d:%d", &nHour, &nMin, &nSec);

			Sleep(300000);
		}
		CString csTime(_T("重连CTP当前时间:"));
		time(&nowtime);
		ptTm = localtime(&nowtime);
		char cur_datetimeX[21];
		strftime(cur_datetimeX, 21, "%Y-%m-%d %X", ptTm);
		CString cscur_datetime(cur_datetimeX);
		csTime.Append(cscur_datetime);
		pPubMsg->AddString(csTime);

		iRet = 0;
	}

	char CTP_FRONT_MD_ADDR[100];
	strcpy(CTP_FRONT_MD_ADDR, "tcp://116.236.239.136:42213");
	iInstrumentID = 0;
	std::list<InstrumentsName>::iterator inst_itr;
	for (inst_itr = DomesticInstSubscribed.begin(); inst_itr != DomesticInstSubscribed.end(); inst_itr++) {
		ppInstrumentID[iInstrumentID] = new char[20];
		string strInst(inst_itr->InstrumentID);
		strcpy(ppInstrumentID[iInstrumentID], strInst.c_str());
		iInstrumentID++;
	}

	CThostFtdcMdApi* pMdApi = CThostFtdcMdApi::CreateFtdcMdApi();
	pThostMdSpi = new CThostMdSpi(pMdApi, "7070", LoginMDUserCTP, LoginMDPwdCTP);
	pMdApi->RegisterSpi((CThostFtdcMdSpi*)pThostMdSpi);
	pMdApi->RegisterFront(CTP_FRONT_MD_ADDR);
	pMdApi->Init();
	pMdApi->Join();
}

void CMyThread::OnCreateMDDispatchThread(UINT wParam, LONG lParam)
{
	MDDataDispatchThread* myMDDataDispatchThread = new MDDataDispatchThread();
	if (myMDDataDispatchThread->openThreadShm() == -1) {
		CString str("共享区创建/打开失败,启动失败..");
		pPubMsg->AddString(str);
		return;
	}
	else myMDDataDispatchThread->DispatchMdMsg();
}

void CMyThread::OnAddNewSubInst(UINT wParam, LONG lParam)
{
	if (pEsunMdSpi != NULL && !OverSeaInstSubscribedNew.empty()) {
		//Esun MD Api 已处于连接状态
		char InstrumentID[32];
		std::map<string, string> OverSeaInstSubscribedNewMap;
		std::list<InstrumentsName>::iterator inst_itrnew;
		for (inst_itrnew = OverSeaInstSubscribedNew.begin(); inst_itrnew != OverSeaInstSubscribedNew.end(); inst_itrnew++) {
			sprintf(InstrumentID, "%s %s %s", inst_itrnew->ExchangeID, inst_itrnew->CommodityNo, inst_itrnew->InstrumentID);
			string strInstrumentID(InstrumentID);
			OverSeaInstSubscribedNewMap.insert(std::pair<string, string>(strInstrumentID, strInstrumentID));
		}
		std::map<string, string> OverSeaInstSubscribedMap;
		std::list<InstrumentsName>::iterator inst_itrold;
		for (inst_itrold = OverSeaInstSubscribed.begin(); inst_itrold != OverSeaInstSubscribed.end(); inst_itrold++) {
			sprintf(InstrumentID, "%s %s %s", inst_itrold->ExchangeID, inst_itrold->CommodityNo, inst_itrold->InstrumentID);
			string strInstrumentID(InstrumentID);
			OverSeaInstSubscribedMap.insert(std::pair<string, string>(strInstrumentID, strInstrumentID));
		}

		char tipMsg[100];
		int iRet = -1;
		bool addnew = false;
		for (inst_itrnew = OverSeaInstSubscribedNew.begin(); inst_itrnew != OverSeaInstSubscribedNew.end(); inst_itrnew++) {
			sprintf(InstrumentID, "%s %s %s", inst_itrnew->ExchangeID, inst_itrnew->CommodityNo, inst_itrnew->InstrumentID);
			string strInstrumentID(InstrumentID);

			map<string, string>::iterator iter;
			iter = OverSeaInstSubscribedMap.find(strInstrumentID);
			if (iter == OverSeaInstSubscribedMap.end()) {
				iRet = pEsunMdSpi->AddReqStk(inst_itrnew->ExchangeID, InstrumentID, 1);
				if (iRet == 0) {
					sprintf(tipMsg, "成功添加新增合约:%s", InstrumentID);
					addnew = true;
				}
				else {
					sprintf(tipMsg, "添加新增合约:%s 失败,Error Ret Code=%d", InstrumentID, iRet);
				}
				CString csRet(tipMsg);
				pPubMsg->AddString(csRet);
			}
		}
		if (addnew) {
			iRet = pEsunMdSpi->SendReqStk();
			CString str(((iRet == 0) ? "易胜行情新增订阅合约成功" : "易胜行情新增订阅合约失败"));
			pPubMsg->AddString(str);
		}
		bool remove = false;
		for (inst_itrold = OverSeaInstSubscribed.begin(); inst_itrold != OverSeaInstSubscribed.end(); inst_itrold++) {
			sprintf(InstrumentID, "%s %s %s", inst_itrold->ExchangeID, inst_itrold->CommodityNo, inst_itrold->InstrumentID);
			string strInstrumentID(InstrumentID);

			map<string, string>::iterator iter;
			iter = OverSeaInstSubscribedNewMap.find(strInstrumentID);
			if (iter == OverSeaInstSubscribedNewMap.end()) {
				iRet = pEsunMdSpi->AddReqStk(inst_itrold->ExchangeID, InstrumentID, 0);
				if (iRet == 0) {
					sprintf(tipMsg, "成功删除合约:%s", InstrumentID);
					remove = true;
				}
				else {
					sprintf(tipMsg, "删除合约:%s 失败,Error Ret Code=%d", InstrumentID, iRet);
				}
				CString csRet(tipMsg);
				pPubMsg->AddString(csRet);
			}
		}
		if (remove) {
			iRet = pEsunMdSpi->SendReqStk();
			CString str(((iRet == 0) ? "易胜行情删除订阅合约成功" : "易胜行情删除订阅合约失败"));
			pPubMsg->AddString(str);
		}

		//同步NewList到ResultList
		OverSeaInstSubscribed.clear();
		for (inst_itrnew = OverSeaInstSubscribedNew.begin(); inst_itrnew != OverSeaInstSubscribedNew.end(); inst_itrnew++) {
			OverSeaInstSubscribed.push_back((*inst_itrnew));
		}
	}
	else if (!OverSeaInstSubscribedNew.empty()) {
		//同步NewList到ResultList
		OverSeaInstSubscribed.clear();
		std::list<InstrumentsName>::iterator inst_itrnew;
		for (inst_itrnew = OverSeaInstSubscribedNew.begin(); inst_itrnew != OverSeaInstSubscribedNew.end(); inst_itrnew++) {
			OverSeaInstSubscribed.push_back((*inst_itrnew));
		}

		pEsunMdThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
		Sleep(500);
		pEsunMdThread->PostThreadMessage(WM_CREATE_ESUN_MD, NULL, NULL);
	}
	else if (OverSeaInstSubscribedNew.empty()) {
		int iRet = -1;
		char InstrumentID[32];
		char tipMsg[100];
		bool remove = false;
		std::list<InstrumentsName>::iterator inst_itrold;
		for (inst_itrold = OverSeaInstSubscribed.begin(); inst_itrold != OverSeaInstSubscribed.end(); inst_itrold++) {
			sprintf(InstrumentID, "%s %s %s", inst_itrold->ExchangeID, inst_itrold->CommodityNo, inst_itrold->InstrumentID);

			iRet = pEsunMdSpi->AddReqStk(inst_itrold->ExchangeID, InstrumentID, 0);
			if (iRet == 0) {
				sprintf(tipMsg, "成功删除合约:%s", InstrumentID);
				remove = true;
			}
			else {
				sprintf(tipMsg, "删除合约:%s 失败,Error Ret Code=%d", InstrumentID, iRet);
			}
			CString csRet(tipMsg);
			pPubMsg->AddString(csRet);
		}

		if (remove) {
			iRet = pEsunMdSpi->SendReqStk();
			CString str(((iRet == 0) ? "易胜行情删除订阅合约成功" : "易胜行情删除订阅合约失败"));
			pPubMsg->AddString(str);
		}

		OverSeaInstSubscribed.clear();
	}

	if (pThostMdSpi != NULL && !DomesticInstSubscribedNew.empty()) {
		//Esun MD Api 已处于连接状态
		char InstrumentID[32];
		std::map<string, string> DomesticInstSubscribedNewMap;
		std::list<InstrumentsName>::iterator inst_itrnew;
		for (inst_itrnew = DomesticInstSubscribedNew.begin(); inst_itrnew != DomesticInstSubscribedNew.end(); inst_itrnew++) {
			sprintf(InstrumentID, "%s %s %s", inst_itrnew->ExchangeID, inst_itrnew->CommodityNo, inst_itrnew->InstrumentID);
			string strInstrumentID(InstrumentID);
			DomesticInstSubscribedNewMap.insert(std::pair<string, string>(strInstrumentID, strInstrumentID));
		}
		std::map<string, string> DomesticInstSubscribedMap;
		std::list<InstrumentsName>::iterator inst_itrold;
		for (inst_itrold = DomesticInstSubscribed.begin(); inst_itrold != DomesticInstSubscribed.end(); inst_itrold++) {
			sprintf(InstrumentID, "%s %s %s", inst_itrold->ExchangeID, inst_itrold->CommodityNo, inst_itrold->InstrumentID);
			string strInstrumentID(InstrumentID);
			DomesticInstSubscribedMap.insert(std::pair<string, string>(strInstrumentID, strInstrumentID));
		}
		//---------------------
		//char tipMsg[100];
		int iRet = -1;
		char* ppNewInstrumentID[100];
		int iNewInstrumentID = 0;
		for (inst_itrnew = DomesticInstSubscribedNew.begin(); inst_itrnew != DomesticInstSubscribedNew.end(); inst_itrnew++) {
			sprintf(InstrumentID, "%s %s %s", inst_itrnew->ExchangeID, inst_itrnew->CommodityNo, inst_itrnew->InstrumentID);
			string strInstrumentID(InstrumentID);

			map<string, string>::iterator iter;
			iter = DomesticInstSubscribedMap.find(strInstrumentID);
			if (iter == DomesticInstSubscribedMap.end()) {
				ppNewInstrumentID[iNewInstrumentID] = new char[20];
				strcpy(ppNewInstrumentID[iNewInstrumentID], inst_itrnew->InstrumentID);
				CString str(_T("CTP行情新增合约:"));
				CString csInst(inst_itrnew->InstrumentID);
				str.Append(csInst);
				pPubMsg->AddString(str);
				iNewInstrumentID++;

				ppInstrumentID[iInstrumentID] = new char[20];
				strcpy(ppInstrumentID[iInstrumentID], inst_itrnew->InstrumentID);
				iInstrumentID++;
			}
		}
		if (iNewInstrumentID > 0) {
			iRet = pThostMdSpi->AddSubscribeMarketData(ppNewInstrumentID, iNewInstrumentID);
			CString str(((iRet == 0) ? "CTP行情新增订阅合约成功" : "CTP行情新增订阅合约失败"));
			pPubMsg->AddString(str);
		}

		char* ppRemoveInstrumentID[100];
		int iRemoveInstrumentID = 0;
		for (inst_itrold = DomesticInstSubscribed.begin(); inst_itrold != DomesticInstSubscribed.end(); inst_itrold++) {
			sprintf(InstrumentID, "%s %s %s", inst_itrold->ExchangeID, inst_itrold->CommodityNo, inst_itrold->InstrumentID);
			string strInstrumentID(InstrumentID);

			map<string, string>::iterator iter;
			iter = DomesticInstSubscribedNewMap.find(strInstrumentID);
			if (iter == DomesticInstSubscribedNewMap.end()) {
				ppRemoveInstrumentID[iRemoveInstrumentID] = new char[20];
				strcpy(ppRemoveInstrumentID[iRemoveInstrumentID], inst_itrold->InstrumentID);
				CString str(_T("CTP行情删除合约:"));
				CString csInst(inst_itrold->InstrumentID);
				str.Append(csInst);
				pPubMsg->AddString(str);

				iRemoveInstrumentID++;
			}
		}
		if (iRemoveInstrumentID > 0) {
			iRet = pThostMdSpi->UnSubscribeMarketData(ppRemoveInstrumentID, iRemoveInstrumentID);
			CString str(((iRet == 0) ? "CTP行情删除订阅合约成功" : "CTP行情删除订阅合约失败"));
			pPubMsg->AddString(str);
		}

		//同步NewList到ResultList
		DomesticInstSubscribed.clear();
		for (inst_itrnew = DomesticInstSubscribedNew.begin(); inst_itrnew != DomesticInstSubscribedNew.end(); inst_itrnew++) {
			DomesticInstSubscribed.push_back((*inst_itrnew));
		}
	}
	else if (!DomesticInstSubscribedNew.empty()) {
		//同步NewList到ResultList
		DomesticInstSubscribed.clear();
		std::list<InstrumentsName>::iterator inst_itrnew;
		for (inst_itrnew = DomesticInstSubscribedNew.begin(); inst_itrnew != DomesticInstSubscribedNew.end(); inst_itrnew++) {
			DomesticInstSubscribed.push_back((*inst_itrnew));
		}

		pCTPMdThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
		Sleep(500);
		pCTPMdThread->PostThreadMessage(WM_CREATE_CTP_MD, NULL, NULL);
	}
	else if (DomesticInstSubscribedNew.empty()) {
		char* ppRemoveInstrumentID[100];
		int iRemoveInstrumentID = 0;
		std::list<InstrumentsName>::iterator inst_itrold;
		for (inst_itrold = DomesticInstSubscribed.begin(); inst_itrold != DomesticInstSubscribed.end(); inst_itrold++) {
			ppRemoveInstrumentID[iRemoveInstrumentID] = new char[20];
			strcpy(ppRemoveInstrumentID[iRemoveInstrumentID], inst_itrold->InstrumentID);
			CString str(_T("CTP行情删除合约:"));
			CString csInst(inst_itrold->InstrumentID);
			str.Append(csInst);
			pPubMsg->AddString(str);

			iRemoveInstrumentID++;
		}

		int iRet = -1;
		if (iRemoveInstrumentID > 0) {
			iRet = pThostMdSpi->UnSubscribeMarketData(ppRemoveInstrumentID, iRemoveInstrumentID);
			CString str(((iRet == 0) ? "CTP行情删除订阅合约成功" : "CTP行情删除订阅合约失败"));
			pPubMsg->AddString(str);
		}
		DomesticInstSubscribed.clear();
	}

	if (pSgitMdSpi != NULL) {
	}

	//写入文件
	std::list<InstrumentsName>::iterator inst_itr;
	//char InstFullName[80];
	FILE* fp = fopen("InstrumentsIDSub.ini", "w+");
	if (fp == NULL) { TRACE("Error in Open ini file"); TRACE("%s\n", strerror(errno)); }
	for (inst_itr = DomesticInstSubscribed.begin(); inst_itr != DomesticInstSubscribed.end(); ++inst_itr) {
		char line[500];
		sprintf(line, "%s,%s,%s,%.5f\n", inst_itr->ExchangeID, inst_itr->CommodityNo, inst_itr->InstrumentID, inst_itr->OneTick);
		fwrite(line, strlen(line), 1, fp);
	}
	for (inst_itr = SGEInstSubscribed.begin(); inst_itr != SGEInstSubscribed.end(); ++inst_itr) {
		char line[500];
		sprintf(line, "%s,%s,%s,%.5f\n", inst_itr->ExchangeID, inst_itr->CommodityNo, inst_itr->InstrumentID, inst_itr->OneTick);
		fwrite(line, strlen(line), 1, fp);
	}
	for (inst_itr = OverSeaInstSubscribed.begin(); inst_itr != OverSeaInstSubscribed.end(); ++inst_itr) {
		char line[500];
		sprintf(line, "%s,%s,%s,%.5f\n", inst_itr->ExchangeID, inst_itr->CommodityNo, inst_itr->InstrumentID, inst_itr->OneTick);
		fwrite(line, strlen(line), 1, fp);
	}
	fclose(fp);

	//刷新缓存中的合约列表
	if (gMapView != NULL) {
		int instarrayindex = 0;
		std::list<InstrumentsName>::iterator inst_itr;
		for (inst_itr = OverSeaInstSubscribed.begin(); inst_itr != OverSeaInstSubscribed.end(); inst_itr++) {
			char instFullName[50];
			sprintf(instFullName, "%s %s %s", (*inst_itr).ExchangeID, (*inst_itr).CommodityNo, (*inst_itr).InstrumentID);

			strcpy(gMapView->instarray[instarrayindex], instFullName);
			instarrayindex++;
			gMapView->instnum = instarrayindex;
		}
		for (inst_itr = DomesticInstSubscribed.begin(); inst_itr != DomesticInstSubscribed.end(); inst_itr++) {
			char instFullName[50];
			sprintf(instFullName, "%s %s %s", (*inst_itr).ExchangeID, (*inst_itr).CommodityNo, (*inst_itr).InstrumentID);

			strcpy(gMapView->instarray[instarrayindex], instFullName);
			instarrayindex++;
			gMapView->instnum = instarrayindex;
		}
		for (inst_itr = SGEInstSubscribed.begin(); inst_itr != SGEInstSubscribed.end(); inst_itr++) {
			char instFullName[50];
			sprintf(instFullName, "%s %s %s", (*inst_itr).ExchangeID, (*inst_itr).CommodityNo, (*inst_itr).InstrumentID);

			strcpy(gMapView->instarray[instarrayindex], instFullName);
			instarrayindex++;
			gMapView->instnum = instarrayindex;
		}
	}

	globalFuncUtil.InitInstrumentSubList();
}

void CMyThread::OnUpLoadTradeLog(UINT wParam, LONG lParam)
{
	COracle* m_pDB = new COracle();
	char cnn[1024] = { 0 };
	bool m_bRet = m_pDB->ConnToDB("Provider=OraOLEDB.Oracle.1;Persist Security Info=True;Data Source=192.168.1.71:1521/orcl", "jrgc", "qjtz1234");
	if (m_bRet)
	{
		// 读取数据
		ifstream  ifs;
		char filePath[128] = { 0 };
		CTime m_Date = CTime::GetCurrentTime() - CTimeSpan(1, 0, 0, 0);
		CString strPathTradeFile;
		::GetModuleFileName(NULL, strPathTradeFile.GetBuffer(MAX_PATH), MAX_PATH);
		strPathTradeFile.ReleaseBuffer();
		strPathTradeFile = strPathTradeFile.Left(strPathTradeFile.ReverseFind(_T('\\')));
		strPathTradeFile += "\\Trades";
		strPathTradeFile += "\\Trades";
		strPathTradeFile += m_Date.Format("%Y%m%d");
		strPathTradeFile += ".log";

		int len = WideCharToMultiByte(CP_ACP, 0, strPathTradeFile, strPathTradeFile.GetLength(), NULL, 0, NULL, NULL);
		char* c_str_filename = new char[len + 1];
		WideCharToMultiByte(CP_ACP, 0, strPathTradeFile, strPathTradeFile.GetLength(), c_str_filename, len, NULL, NULL);
		c_str_filename[len] = '\0';

		ifs.open(c_str_filename, ios::in);
		if (ifs.is_open())
		{
			char buff[1024] = { 0 };
			while (!ifs.eof())
			{
				ifs.getline(buff, 1024);
				char sql[1024] = { 0 };
				CStringArray arry;
				CString sBuff(buff);
				CString sCh(",");
				globalFuncUtil.Split(sBuff, sCh, arry);
				if (arry.GetCount() >= 12)
				{
					// 过滤启动/暂停日志
					CString strTmp = arry.GetAt(3);
					if (-1 != strTmp.Find('S') || -1 != strTmp.Find('E'))
						continue;
					char strategy[256], insName[256], openDate[256], openTime[256], tradingDay[256], openId[256], closeId[256], qty[256], price[256], fee[256], orgPrice[256], instance[256], tradeid[256];
					globalFuncUtil.ConvertCStringToCharArray(arry.GetAt(0), strategy);
					globalFuncUtil.ConvertCStringToCharArray(arry.GetAt(2), insName);
					globalFuncUtil.ConvertCStringToCharArray(arry.GetAt(3), openDate);
					globalFuncUtil.ConvertCStringToCharArray(arry.GetAt(4), openTime);
					globalFuncUtil.ConvertCStringToCharArray(arry.GetAt(13), tradingDay);
					globalFuncUtil.ConvertCStringToCharArray(arry.GetAt(6), openId);
					globalFuncUtil.ConvertCStringToCharArray(arry.GetAt(7), closeId);
					globalFuncUtil.ConvertCStringToCharArray(arry.GetAt(8), qty);
					globalFuncUtil.ConvertCStringToCharArray(arry.GetAt(10), price);
					globalFuncUtil.ConvertCStringToCharArray(arry.GetAt(11), fee);
					globalFuncUtil.ConvertCStringToCharArray(arry.GetAt(10), orgPrice);
					globalFuncUtil.ConvertCStringToCharArray(arry.GetAt(1), instance);
					globalFuncUtil.ConvertCStringToCharArray(arry.GetAt(12), tradeid);

					char UserId[30] = { 0 };
					if (EsunAPIUsed) strcpy(UserId, LoginUserTDEsun);
					else if (CTPAPIUsed) strcpy(UserId, LoginUserTDCTP);

					// 帐号，策略，合约，开仓日期，开仓时间，交易日，开仓id，平仓id，方向数量，成交价格，手续费，成交编号,原始成交价格,实例名称
#ifdef _TEST
					sprintf(sql, "insert into QIJ_TRADE_LOG_BAK (ACCOUNT_NAME,STRATEGY_NAME,INST_NAME,DATADATE,DATATIME,TRADINGDAY,OPEN_ID,CLOSE_ID,TRADE_VOL,TRADE_PRICE,TRADE_FEE,TRADE_ID,ORIG_TRADE_PRICE,INSTANCE_NAME) values('%s','%s','%s','%s','%s','%s',%s,%s,%s,%s,%s,'%s',%s,'%s')",
						UserId, strategy, insName, openDate, openTime, tradingDay, openId, closeId, qty, price, fee, tradeid, orgPrice, instance);
#else
					sprintf(sql, "insert into QIJ_TRADE_LOG (ACCOUNT_NAME,STRATEGY_NAME,INST_NAME,DATADATE,DATATIME,TRADINGDAY,OPEN_ID,CLOSE_ID,TRADE_VOL,TRADE_PRICE,TRADE_FEE,TRADE_ID,ORIG_TRADE_PRICE,INSTANCE_NAME) values('%s','%s','%s','%s','%s','%s','%s','%s',%s,%s,%s,'%s',%s,'%s')",
						UserId, strategy, insName, openDate, openTime, tradingDay, openId, closeId, qty, price, fee, tradeid, orgPrice, instance);
#endif
					m_pDB->ExecuteWithResSQL(sql);
				}
			}
		}
		ifs.close();
		free(c_str_filename);
		delete m_pDB;
		m_pDB = NULL;
	}
}

void CMyThread::OnUpLoadValue(UINT wParam, LONG lParam)
{
	CTime mCurrTime = CTime::GetCurrentTime() - CTimeSpan(1, 0, 0, 0);
	char cDate[32] = { 0 };
	sprintf(cDate, "%04d%02d%02d", mCurrTime.GetYear(), mCurrTime.GetMonth(), mCurrTime.GetDay());
	ofstream m_of;
	m_of.open("value.txt", ios::out | ios::app);
	double f = pEsunTraderSpi->m_Balance;
	double fRate = 1 / pEsunTraderSpi->m_RMBExchangeRate;
	if (m_of.is_open())
	{
		char buff[256] = { 32 };
		// 账户,日期,时间,权益,汇率
		sprintf(buff, "%s,%s,%s,%.2f,%.4f", LoginUserTDEsun, cDate, "23:59:59", f, fRate);
		m_of << buff << endl;
		m_of.flush();
		m_of.close();
	}
	// 写数据库
	COracle* m_pDB = new COracle();
	char cnn[1024] = { 0 };
	bool m_bRet = m_pDB->ConnToDB("Provider=OraOLEDB.Oracle.1;Persist Security Info=True;Data Source=192.168.1.71:1521/orcl", "jrgc", "qjtz1234");
	if (m_bRet)
	{
		char sql[256] = { 0 };
		// 帐号，策略，合约，开仓日期，开仓时间，交易日，开仓id，平仓id，方向数量，成交价格，手续费，成交编号
		char tradingDay[32] = { 0 };
		sprintf(tradingDay, "%d", 11);
#ifdef _TEST
		sprintf(sql, "insert into qij_acct_info_bak (ACCT_ID,TRADE_DATE,ACCT_VALUE,DATATIME,RMB_EXCHANGE_RATE) values('%s','%s',%.2f,'%s','%.4f')", LoginUserTDEsun,
			cDate, f, "23:59:59", fRate);
#else
		sprintf(sql, "insert into qij_acct_info (ACCT_ID,TRADE_DATE,ACCT_VALUE,DATATIME,RMB_EXCHANGE_RATE) values('%s','%s',%.2f,'%s','%.4f')", LoginUserTDEsun,
			cDate, f, "23:59:59", fRate);
#endif

		m_pDB->ExecuteWithResSQL(sql);
		delete m_pDB;
		m_pDB = NULL;
	}
}

void CMyThread::OnUpLoadCTPAccValue(UINT wParam, LONG lParam)
{
	double f = pThostTraderSpi->m_Balance;
	double fRate = 1;
	// 写数据库
	COracle* m_pDB = new COracle();
	char cnn[1024] = { 0 };
	bool m_bRet = m_pDB->ConnToDB("Provider=OraOLEDB.Oracle.1;Persist Security Info=True;Data Source=192.168.1.71:1521/orcl", "jrgc", "qjtz1234");
	if (m_bRet)
	{
		char sql[256] = { 0 };
		// 帐号，策略，合约，开仓日期，开仓时间，交易日，开仓id，平仓id，方向数量，成交价格，手续费，成交编号
#ifdef _TEST
		sprintf(sql, "insert into qij_acct_info_bak (ACCT_ID,TRADE_DATE,ACCT_VALUE,DATATIME,RMB_EXCHANGE_RATE) values('%s','%s',%.2f,'%s','%.4f')", LoginUserTDCTP,
			CTPTradingDay, f, "15:00:00", fRate);
#else
		sprintf(sql, "insert into qij_acct_info (ACCT_ID,TRADE_DATE,ACCT_VALUE,DATATIME,RMB_EXCHANGE_RATE,MONEY_UNIT) values('%s','%s',%.2f,'%s','%.4f','%s')", LoginUserTDCTP,
			CTPTradingDay, f, "15:00:00", fRate, "RMB");
#endif
		m_pDB->ExecuteWithResSQL(sql);
		delete m_pDB;
		m_pDB = NULL;
	}
}

void CMyThread::OnSendUdpMessage(WPARAM wParam, LPARAM lParam) {
	WSADATA data;
	WORD wVersionRequested = MAKEWORD(2, 0);
	WSAStartup(wVersionRequested, &data);

	SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(5566);
	addr.sin_addr.S_un.S_addr = inet_addr("192.168.3.196");

	//TRACE("Client is setup and now trying to connect server...\n");

	sockaddr_in addrServer;
	int nSockAddrSize = sizeof(addrServer);
	//sendto(s, szSendToServer, sizeof(szSendToServer), 0, (sockaddr*)&addr, nSockAddrSize);
	while (true) {
		WaitForSingleObject(tradeSemaphore, INFINITE);
		if (!TradeList.DataListCore.IsEmpty()) {
			SendLogType log = TradeList.GetHead();
			int len = sendto(s, (char*)&log, sizeof(SendLogType), 0, (sockaddr*)&addr, nSockAddrSize);
			char xline[200];
			memset(xline, 0, sizeof(xline));
			sprintf(xline, "send:%s,%s,%s,%s,%s,%d,%s,%s;length:%d", log.AccountID, log.StrategyID, log.InstanceName, log
				.CodeName, log.MatchNo, log.qty, log.openid, log.closeid, len);
			globalFuncUtil.WriteMsgToLogList(xline);
		}
		if (gEndSendUdp) break;
	}
	//recvfrom(s, szBuff, sizeof(szBuff), 0, (sockaddr*)&addrServer, &nSockAddrSize);
	//TRACE("client receive:%s\n", szBuff);

	closesocket(s);
	WSACleanup();

	return;
}

void CMyThread::OnCreateRMTcpThread(UINT wParam, LONG lParam)
{
	pRMTcp = new RiskManagementTcp(RSServerIP, RSServerPort);
	pRMTcp->m_bRunning = true;
	pRMTcp->ProcessTCPMsg();
}