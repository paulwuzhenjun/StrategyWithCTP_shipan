#pragma once
#include "MdSpi.h"
#include "Message.h"
#include "StrategyOpenPriceOpening.h"
#include "MessageList.h"
#include "TickDataList.h"
#include "OrderDataList.h"
#include "MyThread.h"
#include <map>
#include "EsunTraderSpi.h"
#include "ThostTraderSpi.h"
#include "SgitTraderSpi.h"
#include "LockVariable.h"
#include "GlobalFunc.h"
#include "EsunMdSpi.h"
#include "ThostMdSpi.h"
#include "SgitMdSpi.h"
#include "TradeDataList.h"
#include "RiskManagementTcp.h"

using namespace std;

// controls
CListBox* pPubMsg;  //提示消息

CListCtrl* pPositionDetailsList;
CListCtrl* pTradesDetailsList; //委托单
CListCtrl* pMDMsgDisplay;
CListCtrl* pQryOrdersList;
CListCtrl* pParamsList;
CListCtrl* pStraLocalCLList;

CStatic* pDay;      //
CStatic* pMoney;
CStatic* pBalance;
CStatic* pUsername; //用户名
CStatic* pMoneyCTP;
CStatic* pBalanceCTP;
CStatic* pUsernameCTP;
CButton* pConnectTDBTN;
//  api
EsunTraderSpi* pEsunTraderSpi;
ThostTraderSpi* pThostTraderSpi;
SgitTraderSpi* pSgitTraderSpi;

CMdSpi* pMdSpi;

// 记录IC仓位
int gBuyPosition = 0;
int gSellPosition = 0;
map<string, Posi> gPosiMap;

//  thread
CMyThread* pEsunTraderThread;
CMyThread* pThostTraderThread;
CMyThread* pSgitTraderThread;

CMyThread* pRestartTraderThread;
CMyThread* pMdThread;
CMyThread* pTdDispatcher;
CMyThread* pStrategyThread;
CMyThread* pRecoverStrategyThread;
CMyThread* pQryActionThread;
CMyThread* pQryTradeThread;
CMyThread* pTDConnLostQryTradeThread;
CMyThread* pLogThread;
CMyThread* pUpLoadThread;
CMyThread* pUpLoadValueThread;
CMyThread* pRSTcpLoadThread;
CStrategy* gStrategyImpl[MAX_RUNNING_STRATEGY] = { 0 };
int gStrategyImplIndex = 0;
CString StrategyIDShowing("None");
CString ClassNameShowing("None");
//以下为设置项
char  FRONT_TRADER_ADDR[] = "tcp://180.166.13.17:41205";	 //在这里设置交易服务前置地址
char  FRONT_MD_ADDR[] = "tcp://180.166.13.17:41213";//"tcp://180.168.212.75:41213";    //在这里设置行情服务前置地址

char LoginUserTDEsun[21] = "12345";
char LoginPwdTDEsun[21] = "12345";
char LoginUserTDCTP[21] = "12345";
char LoginPwdCTP[21] = "12345";
//char InstrumentID[10]="1706";  //远期合约类型
//char CodeName[50]="";

int g_iRequestIDEsun = 0;
int g_iRequestIDThost = 0;
bool gEndSendUdp = false;	// 发送udp消息标志

TickDataList TickList;
OrderDataList OrderList;
MessageList ScreenDisplayMsgList;
MessageList LogMessageList;
TradeDataList TradeList;

int iNextOrderRef;    //下一单引用

//map<int,int> ErrorOrderIdToReqId;
map<int, int> OrderIdToShmIndex;
map<int, string> OrderIdToStrategyNameForDisplay;
map<string, string> MatchNoToStrategyNameForDisplay;
map<int, int> ReqIdToShmIndex;
map<int, int> OrderLocalRefToShmIndex;//为CTP报单逻辑准备, OrderLocalRef到策略缓存队列号shmindex的map

map<int, ManualOrder> OrderIdToStrategyManualOrder;
map<int, ManualOrder> ReqIdToStrategyManualOrder;

list<ParamNode> paramslist;
list<ModelNode> ModelList;
HANDLE logSemaphore;
HANDLE ScreenDisplaySem;
HANDLE DispatchTdSem;
HANDLE MainScreenFreshSem;
HANDLE MDMainScreenFreshSem;
HANDLE OrderInsertSem;
HANDLE tradeSemaphore;

int TotalOnHandPosition = 0; //原子操作,故不用锁
int TotalCancelTimes = 0;
int MaxTotalOnHandPosition = 6;
int MaxTotalCancelTimes = 400;

HANDLE CreateThreadSem;
HANDLE TraderOrderInsertSem;
HANDLE MatchNoMapSem;
HANDLE RecoverStrategyDlgSem;
HANDLE localCLSem;

//double m_dOneTick=0.01;
map<string, InstrumentInfo> InstrumentsSubscribed;
list<string> InstrumentsSubscribedList;
bool ParamsChangedOrNot = false;
bool g_bQryOrderSentByRecoverDlg = false;
map<int, OrderDetailField> RecoverRspOrderMap;
RecoverStrategy gRecoverStrategy;
bool ClearInstanceCfgFile = false;
char TDConnectionLostTime[21];
bool TDConnectionLost = false;

map<string, int> InstanceToShmindexMap;

CLockVariable gLockVariable;

OrderDetailField gOrderDelete;

bool EsunAPIUsed = true;
bool CTPAPIUsed = true;
bool SgitAPIUsed = false;

GlobalFunc globalFuncUtil;
char CTPTradingDay[10];
char SgitTradingDay[10];

HANDLE ambushordersem;
list<InsertOrderField> ambushOrderList;
//map<string,double> instlastpricemap;
//-------For Md Dlg
CListBox* pMdPubMsg;
CListBox* pMdInstList;
list<InstrumentsName> OverSeaInstSubscribed;
list<InstrumentsName> DomesticInstSubscribed;
list<InstrumentsName> SGEInstSubscribed;

list<InstrumentsName> OverSeaInstSubscribedNew;
list<InstrumentsName> DomesticInstSubscribedNew;
list<InstrumentsName> SGEInstSubscribedNew;

CMyThread* pEsunMdThread;
CMyThread* pCTPMdThread;
CMyThread* pSgitMdThread;
CMyThread* pRestartEsunMdThread;
CMyThread* pRestartCTPMdThread;
CMyThread* pMDDispatcherThread;
CMyThread* pAddNewSubInst;
CMyThread* pSendUdpMsThread;
bool MDServerConnected = false;
HANDLE MdTickSem;
//HANDLE logSemaphore;
char LoginMDUser[30];
char LoginMDPwd[30];
char LoginMDUserCTP[30];
char LoginMDPwdCTP[30];
CEsunMdSpi* pEsunMdSpi;
CThostMdSpi* pThostMdSpi;
CSgitMdSpi* pSgitMdSpi;
RiskManagementTcp* pRMTcp;
char RSServerIP[50] = "192.168.130.9";
int RSServerPort = 7777;
#ifdef _TEST
addrtype EsunMdAddr[MAXMDADDRNO] = { {"122.224.221.238",27801},{"124.160.66.126",27801} };
addrtype EsunTdAddr[MAXTDADDRNO] = { {"122.224.221.238",27802},{"124.160.66.126",27802} };
#else
addrtype EsunMdAddr[MAXMDADDRNO] = { {"122.224.221.238",17902},{"124.160.66.126",17902} };
addrtype EsunTdAddr[MAXTDADDRNO] = { {"122.224.221.238",17901},{"124.160.66.126",17901} };
#endif
