#include "StdAfx.h"
#include "StrategyGridMAStopGTCChop.h"
#include "Message.h"
#include "MessageList.h"
#include "TickDataList.h"
#include "OrderDataList.h"
#include "ado_conn.h"
#include "EsunTraderSpi.h"
#include "ThostTraderSpi.h"
#include "SgitTraderSpi.h"
#include <Afxtempl.h>
#include <iostream>
#include <time.h>
#include <tchar.h>
#include <math.h>
#include <map>
#include "LockVariable.h"
#include "TradeDataList.h"
#include "GlobalFunc.h"

using namespace std;

int StrategyGridMAStopGTCChop::MaxOnHandPositionCount = 0;
int StrategyGridMAStopGTCChop::OnHandPositionCount = 0;

extern CListBox* pPubMsg;

extern GlobalFunc globalFuncUtil;

extern EsunTraderSpi* pEsunTraderSpi;
extern ThostTraderSpi* pThostTraderSpi;
extern SgitTraderSpi* pSgitTraderSpi;

extern MessageList ScreenDisplayMsgList;
extern MessageList LogMessageList;
extern OrderDataList OrderList;
extern TradeDataList TradeList;

extern int iNextOrderRef;    //下一单引用
extern char CTPTradingDay[];
extern char LoginUserTDEsun[];

//extern LONGLONG nStart; //微妙
//extern DWORD nMillSecStart;//毫秒
//extern LARGE_INTEGER liQPF;
//extern double g_PF_us;
//extern double g_spread;
extern TickDataList TickList;
extern map<int, int> OrderIdToShmIndex;
extern map<int, string> OrderIdToStrategyNameForDisplay;
extern map<string, string> MatchNoToStrategyNameForDisplay;
extern map<int, int> ReqIdToShmIndex;
extern HANDLE logSemaphore;
extern HANDLE MatchNoMapSem;
extern SRWLOCK  g_srwLockReqId;
extern SRWLOCK  g_srwLockOrderId;
extern int TotalOnHandPosition; //原子操作,故不用锁
extern int MaxTotalOnHandPosition;
extern MessageList ScreenDisplayMsgList;
extern HANDLE ScreenDisplaySem;
extern HANDLE MainScreenFreshSem;
extern HANDLE OrderInsertSem;
extern HANDLE tradeSemaphore;
extern CListCtrl* pTradesDetailsList;
extern list<ModelNode> ModelList;
extern bool ClearInstanceCfgFile;
//For Thost Trader API
extern CLockVariable gLockVariable;
extern SRWLOCK g_srwLockOrderLocalRef;
extern map<int, int> OrderLocalRefToShmIndex;

StrategyGridMAStopGTCChop::StrategyGridMAStopGTCChop(char InstrumentID[30])
{
	iNextOrderRef = 1;
	mTickCount = 0;
	m_dOneTick = 0.01;
	m_bSampleReady = false;
	mOpenTimes = 0;

	m_bIsRunning = true;
	m_bCrossTradingDay = false;

	mClosePrice = 0;
	mPrice = 0;
	mRet = false;
	mOrderRet = false;

	mInitBasePrice = 0;
	mCurBasePrice = 0;

	memset(&mStrategyParams, 0, sizeof(mParasType));
	memset(GridCloseOrderCount, 0, sizeof(GridCloseOrderCount));

	m_bStoplossClose = false;

	strcpy(mStrategyName, "GridMAStopGTCChop");
	strcpy(mInstanceName, "");
	WriteMsgToLogList("StrategyGridMAStopGTCChop Init..");
	//memset(curTradingDay,0,sizeof(curTradingDay));
	//CTime ct=CTime::GetCurrentTime();
	//struct tm* ptTm;
	//time_t nowTime;
	//nowTime=time(NULL);
	//ptTm=localtime(&nowTime);
	//strftime(curTradingDay,30,"%X",ptTm);
}

StrategyGridMAStopGTCChop::~StrategyGridMAStopGTCChop(void)
{
}

int StrategyGridMAStopGTCChop::GetShmindex()
{
	return shmindex;
}

void StrategyGridMAStopGTCChop::SetShmindex(int xshmindex)
{
	shmindex = xshmindex;
}

void StrategyGridMAStopGTCChop::SetInstanceName(char xInstanceName[50])
{
	strcpy(mInstanceName, xInstanceName);
	mStrategyAndInstance = mStrategyID;
	mStrategyAndInstance.append("_");
	mStrategyAndInstance.append(mInstanceName);
}

void StrategyGridMAStopGTCChop::GetInstanceName(char* xInstanceName) {
	strcpy(xInstanceName, mInstanceName);
}

void StrategyGridMAStopGTCChop::SetParamValue(ParamNode node) {
	bool find = false;
	std::list<ParamNode>::iterator param_it;
	if (!mParamslist.empty()) {
		for (param_it = mParamslist.begin(); param_it != mParamslist.end(); ++param_it) {
			if (strcmp(param_it->ParamName, node.ParamName) == 0) { find = true; break; }
		}
	}
	if (!find) {
		mParamslist.push_back(node);
	}

	if (strcmp(node.ParamName, "InstCodeName") == 0) {
		string strInstCodeName(node.ParamValue);
		string strExchangeID = strInstCodeName.substr(0, strInstCodeName.find_first_of(" "));
		string strCommodityNo = strInstCodeName.substr(strInstCodeName.find_first_of(" ") + 1, strInstCodeName.find_last_of(" ") - strInstCodeName.find_first_of(" ") - 1);
		string strInstID = strInstCodeName.substr(strInstCodeName.find_last_of(" ") + 1, strInstCodeName.length() - strInstCodeName.find_last_of(" "));

		if (strcmp(strExchangeID.c_str(), "CFFEX") == 0 || strcmp(strExchangeID.c_str(), "SHFE") == 0
			|| strcmp(strExchangeID.c_str(), "DCE") == 0 || strcmp(strExchangeID.c_str(), "CZCE") == 0 || strcmp(strExchangeID.c_str(), "SGE") == 0) {
			if (strcmp(strExchangeID.c_str(), "SGE") == 0) {
				SgitTraderAPI = true;
			}
			else {
				ThostTraderAPI = true;
			}
			strcpy(mStrategyParams.InstCodeName, strInstID.c_str());
			strcpy(InstCodeName, strInstID.c_str());
		}
		else {
			ThostTraderAPI = false;
			strcpy(mStrategyParams.InstCodeName, node.ParamValue);
			strcpy(InstCodeName, node.ParamValue);
		}

		strcpy(m_InstID, strInstID.c_str());
		strcpy(m_CommodityNo, strCommodityNo.c_str());
		strcpy(m_ExchangeID, strExchangeID.c_str());
		strcpy(m_InstCodeName, strInstCodeName.c_str());
	}
	else if (strcmp(node.ParamName, "UpGridCount") == 0) {
		mStrategyParams.UpGridCount = atoi(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "UpGridDistPoint") == 0) {
		mStrategyParams.UpGridDistPoint = atoi(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "DnGridCount") == 0) {
		mStrategyParams.DnGridCount = atoi(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "DnGridDistPoint") == 0) {
		mStrategyParams.DnGridDistPoint = atoi(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "ProfitPoint") == 0) {
		mStrategyParams.ProfitPoint = atoi(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "StoplossPoint") == 0) {
		mStrategyParams.StoplossPoint = atoi(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "GridVol") == 0) {
		mStrategyParams.GridVol = atoi(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "BasePrice") == 0) {
		mStrategyParams.BasePrice = atof(node.ParamValue);
		mPrice = mStrategyParams.BasePrice;
	}
	else if (strcmp(node.ParamName, "StopPrice") == 0) {
		mStrategyParams.StopPrice = atof(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "StartTime") == 0) {
		strcpy(mStrategyParams.StartTime, node.ParamValue);
	}
	else if (strcmp(node.ParamName, "EndTime") == 0) {
		strcpy(mStrategyParams.EndTime, node.ParamValue);
	}
	else if (strcmp(node.ParamName, "OpenTime") == 0) {
		strcpy(mStrategyParams.OpenTime, node.ParamValue);
	}
	else if (strcmp(node.ParamName, "CloseTime") == 0) {
		strcpy(mStrategyParams.CloseTime, node.ParamValue);
	}
	else if (strcmp(node.ParamName, "OpenBuyAllow") == 0) {
		if (atoi(node.ParamValue) >= 1)mStrategyParams.OpenBuyAllow = true;
		else mStrategyParams.OpenBuyAllow = false;
	}
	else if (strcmp(node.ParamName, "OpenSellAllow") == 0) {
		if (atoi(node.ParamValue) >= 1)mStrategyParams.OpenSellAllow = true;
		else mStrategyParams.OpenSellAllow = false;
	}
	else if (strcmp(node.ParamName, "IsBasePrice") == 0) {
		if (atoi(node.ParamValue) >= 1)mStrategyParams.IsBasePrice = true;
		else mStrategyParams.IsBasePrice = false;
	}
	else if (strcmp(node.ParamName, "BackPerc") == 0) {
		mStrategyParams.BackPerc = atof(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "OffFlagPoint") == 0) {
		mStrategyParams.OffFlagPoint = atoi(node.ParamValue);
	}

	//根据开始结束时间设置委托单是否跨日有效
	if (strcmp(mStrategyParams.StartTime, "") > 0 && strcmp(mStrategyParams.EndTime, "") > 0) {
		string strStart(mStrategyParams.StartTime);
		string strEnd(mStrategyParams.EndTime);
		string timepart = strStart.substr(strStart.find_first_of(" ") + 1, strStart.length() - strStart.find_first_of(" "));
		string datepart = strStart.substr(0, strStart.find_first_of(" "));
		int nYearTmp = 0, nMonthTmp = 0, nDateTmp = 0, nHourTmp = 0, nMinTmp = 0, nSecTmp = 0;
		int nYearEndTmp = 0, nMonthEndTmp = 0, nDateEndTmp = 0, nHourEndTmp = 0, nMinEndTmp = 0, nSecEndTmp = 0;
		sscanf_s(strStart.c_str(), "%d-%d-%d %d:%d:%d", &nYearTmp, &nMonthTmp, &nDateTmp, &nHourTmp, &nMinTmp, &nSecTmp);
		sscanf_s(strEnd.c_str(), "%d-%d-%d %d:%d:%d", &nYearEndTmp, &nMonthEndTmp, &nDateEndTmp, &nHourEndTmp, &nMinEndTmp, &nSecEndTmp);
		if (nHourTmp < 3) {
			//开始时间在03:00前,当前交易日结束时间为当天的03:00:00
			CTime endct(nYearTmp, nMonthTmp, nDateTmp, 3, 0, 0);
			time_t ct = endct.GetTime();
			CTime straendct(nYearEndTmp, nMonthEndTmp, nDateEndTmp, nHourEndTmp, nMinEndTmp, nSecEndTmp);
			time_t stract = straendct.GetTime();
			if (stract > ct) {
				//跨日
				m_bCrossTradingDay = true;
			}
			else {
				m_bCrossTradingDay = false;
			}
		}
		else {
			//开始时间在03:00后,当前交易日结束时间为第二天的03:00:00
			CTime endct(nYearTmp, nMonthTmp, nDateTmp, 3, 0, 0);
			time_t ct = endct.GetTime() + 86400;

			CTime straendct(nYearEndTmp, nMonthEndTmp, nDateEndTmp, nHourEndTmp, nMinEndTmp, nSecEndTmp);
			time_t stract = straendct.GetTime();
			if (stract > ct) {
				//跨日
				m_bCrossTradingDay = true;
			}
			else {
				m_bCrossTradingDay = false;
			}
		}
	}
}

void StrategyGridMAStopGTCChop::InitVariables() {
	int nYearS, nMonthS, nDateS, nHourS, nMinS, nSecS;
	int nYearE, nMonthE, nDateE, nHourE, nMinE, nSecE;

	sscanf(mStrategyParams.StartTime, "%d-%d-%d %d:%d:%d", &nYearS, &nMonthS, &nDateS, &nHourS, &nMinS, &nSecS);
	sscanf(mStrategyParams.EndTime, "%d-%d-%d %d:%d:%d", &nYearE, &nMonthE, &nDateE, &nHourE, &nMinE, &nSecE);

	CTime tms(nYearS, nMonthS, nDateS, nHourS, nMinS, nSecS);
	tmt_StartTime = tms.GetTime();

	CTime tme(nYearE, nMonthE, nDateE, nHourE, nMinE, nSecE);
	tmt_EndTime = tme.GetTime();

	mTickCount = 0;
	string strInstCodeName(mStrategyParams.InstCodeName);

	m_bSampleReady = false;
	mOpenTimes = 0;
	mOpenRet = false;
}
void StrategyGridMAStopGTCChop::ResetStrategy() {
}

void StrategyGridMAStopGTCChop::OnRtnDepthMarketData(TickInfo* pDepthMarketData)
{
	///读取行情价格，设置当前本地的当前价格
	m_Price = pDepthMarketData->price;
	m_Buy1 = pDepthMarketData->bid1;// m_Price;//
	m_Sell1 = pDepthMarketData->ask1;//m_Buy1+1.0;//

	int nYear_m, nMonth_m, nDate_m;
	nYear_m = atoi(pDepthMarketData->datadate) / 10000;
	nMonth_m = (atoi(pDepthMarketData->datadate) % 10000) / 100;
	nDate_m = (atoi(pDepthMarketData->datadate) % 10000) % 100;
	int  nHour_m, nMin_m, nSec_m;

	sscanf(pDepthMarketData->updatetime, "%d:%d:%d", &nHour_m, &nMin_m, &nSec_m);
	CTime tm_m(nYear_m, nMonth_m, nDate_m, nHour_m, nMin_m, nSec_m);
	curTickTime = tm_m.GetTime();

	int nSecond = GetShmindex();

	// 写文件
	if (14 == nHour_m && 58 == nMin_m && !mRet) {
		char date[30];
		sprintf_s(date, "%04d%02d%02d", nYear_m, nMonth_m, nDate_m);
		mCurBasePrice = mStrategyParams.BasePrice;
		WriteBasePrice(date, pDepthMarketData->updatetime);
		mRet = true;
	}
	int nOpenHour, nOpenMin, nOpenSec;
	sscanf(mStrategyParams.OpenTime, "%d:%d:%d", &nOpenHour, &nOpenMin, &nOpenSec);

	// 获取开盘价
	if (!mStrategyParams.IsBasePrice) {
		if (CloseOrderList.empty()) {
			if (nOpenHour == nHour_m && nMin_m >= nOpenMin && !mOpenRet) {
				char buff[20];
				sprintf(buff, "%.4f", pDepthMarketData->price);
				mStrategyParams.BasePrice = atof(buff);
				mPrice = mStrategyParams.BasePrice;
				mInitBasePrice = mStrategyParams.BasePrice;
				mOpenRet = true;
			}
		}
	}

	//Processing Opened Order
	// 收盘前没有仓位时要撤掉开仓挂单,开盘后以上日收盘价为基准价重新挂单
	if (CloseOrderList.empty()) {
		std::list<MyOpenOrderType>::iterator openorder_it;
		for (openorder_it = OpenOrderList.begin(); openorder_it != OpenOrderList.end(); ++openorder_it) {
			if (openorder_it->VolumeTotal != 0 && openorder_it->OpenOrderCanbeCanceled && timeRuleForCancel(pDepthMarketData->datadate, pDepthMarketData->updatetime)) {
				//结束时间已到,未成交的开仓单撤掉
				char line[200];
				sprintf(line, "%s,%s,TimeEnd,CancelOpen,Id=%d", pDepthMarketData->datadate, pDepthMarketData->updatetime, openorder_it->OrderId);
				WriteMsgToLogList(line);

				ReqOrderDelete(openorder_it->OrderId, openorder_it->OrderLocalRef, openorder_it->FrontID, openorder_it->SessionID);
				openorder_it->OpenOrderCanbeCanceled = false;
			}
		}//end for Opened loop
	}

	//Processing Closed Order
	/*std::list<MyCloseOrderType>::iterator closeorder_it;
	for(closeorder_it = CloseOrderList.begin(); closeorder_it != CloseOrderList.end(); ++closeorder_it){
		if((closeorder_it->OrderStatus==MORDER_PART_TRADED||closeorder_it->OrderStatus==MORDER_ACCEPTED||closeorder_it->OrderStatus==MORDER_QUEUED)
			&&timeRuleForCancel(pDepthMarketData->datadate,pDepthMarketData->updatetime)
			&&closeorder_it->CanbeCanceled){
			//结束时间已到,未成交的平仓单撤掉
			char line[200];
			sprintf(line,"%s,%s,TimeEnd,CancelClose,Id=%d",pDepthMarketData->datadate,pDepthMarketData->updatetime,closeorder_it->OrderId);
			WriteMsgToLogList(line);

			ReqOrderDelete(closeorder_it->OrderId,closeorder_it->OrderLocalRef,closeorder_it->FrontID,closeorder_it->SessionID);
			closeorder_it->CanbeCanceled=false;
		}
	}//end for Closed loop*/
	bool BuyOrderNeedStopExist = false;
	bool SellOrderNeedStopExist = false;
	std::list<MyCloseOrderType>::iterator closeorder_it;
	if (!CloseOrderList.empty()) {
		int CloseOrderCount = 0;
		//Loop Close Order List
		for (closeorder_it = CloseOrderList.begin(); closeorder_it != CloseOrderList.end();) {
			std::list<MyCloseOrderType>::iterator iter_e = closeorder_it++;
			if (iter_e->MOrderType == -1) BuyOrderNeedStopExist = true;
			if (iter_e->Direction == MORDER_SELL &&
				(iter_e->OrderStatus == MORDER_PART_TRADED || iter_e->OrderStatus == MORDER_ACCEPTED || iter_e->OrderStatus == MORDER_QUEUED)) {
				iter_e->maxProfit = max(iter_e->maxProfit, m_Price - iter_e->OpenOrderTradePrice);
				if (iter_e->maxProfit + 0.00001 > mStrategyParams.ProfitPoint* m_dOneTick) iter_e->MAStop = true;
				if (iter_e->CanbeCanceled && iter_e->IsClosePofitOrder && iter_e->MOrderType == -1) {
					// 砍仓操作
					char line[200];
					sprintf(line, "%s,Buy Order Chop Stoploss Close,%.4f,%.4f,%.4f,OrderId=%d", mStrategyAndInstance.c_str(), iter_e->OpenOrderTradePrice, iter_e->maxProfit, m_Price, iter_e->OrderId);
					WriteMsgToLogList(line);

					MyCloseOrderType newThostOrder;

					newThostOrder.OrderLocalRetReqID = 0;
					newThostOrder.OpenOrderID = iter_e->OpenOrderID;
					newThostOrder.CloseOrderSeqNo = 0;
					strcpy(newThostOrder.OpenTime, iter_e->OpenTime);
					newThostOrder.IsStoplessOrder = true;
					newThostOrder.CanbeCanceled = true;
					newThostOrder.IsClosePofitOrder = false;
					newThostOrder.OpenOrderTradePrice = iter_e->OpenOrderTradePrice;
					newThostOrder.MOrderType = iter_e->MOrderType;
					newThostOrder.Direction = iter_e->Direction;
					newThostOrder.LimitPrice = m_Buy1;
					newThostOrder.OrigSubmitPrice = newThostOrder.LimitPrice;
					newThostOrder.Offset = iter_e->Offset;
					newThostOrder.VolumeTotalOriginal = iter_e->VolumeTotal;
					newThostOrder.VolumeTotal = newThostOrder.VolumeTotalOriginal;
					newThostOrder.VolumeTraded = 0;

					ReqCloseOrderInsert(&newThostOrder, newThostOrder.OpenTime);

					CloseOrderList.erase(iter_e);
					CloseOrderCount++;
					if (CloseOrderCount >= 6) break;
					m_bStoplossClose = true;
				}
				else if ((iter_e->LimitPrice - 0.00001) > m_Sell1&& iter_e->IsStoplessOrder&& iter_e->CanbeCanceled) {
					char line[200];
					sprintf(line, "%s,Buy Order Close ReSubmit,OrderId=%d", mStrategyAndInstance.c_str(), iter_e->OrderId);
					WriteMsgToLogList(line);
					iter_e->NextCloseOrderPrice = m_Buy1;
					ReqOrderDelete(iter_e->OrderId, iter_e->OrderLocalRef, iter_e->FrontID, iter_e->SessionID);
					iter_e->CanbeCanceled = false;
				}
				else if (iter_e->IsClosePofitOrder && ((m_Price - 0.00001) < (iter_e->OpenOrderTradePrice + iter_e->maxProfit * (1 - mStrategyParams.BackPerc))) && iter_e->MAStop) {
					// 追踪止盈利
					char line[200];
					sprintf(line, "%s,Buy Order Trace Stoploss Close,%.4f,%.4f,%.4f,OrderId=%d", mStrategyAndInstance.c_str(), iter_e->OpenOrderTradePrice, iter_e->maxProfit, m_Price, iter_e->OrderId);
					WriteMsgToLogList(line);

					MyCloseOrderType newThostOrder;

					newThostOrder.OrderLocalRetReqID = 0;
					newThostOrder.OpenOrderID = iter_e->OpenOrderID;
					newThostOrder.CloseOrderSeqNo = 0;
					strcpy(newThostOrder.OpenTime, iter_e->OpenTime);
					newThostOrder.IsStoplessOrder = true;
					newThostOrder.CanbeCanceled = true;
					newThostOrder.IsClosePofitOrder = false;
					newThostOrder.OpenOrderTradePrice = iter_e->OpenOrderTradePrice;
					newThostOrder.MOrderType = iter_e->MOrderType;
					newThostOrder.Direction = iter_e->Direction;
					newThostOrder.LimitPrice = m_Buy1;
					newThostOrder.OrigSubmitPrice = newThostOrder.LimitPrice;
					newThostOrder.Offset = iter_e->Offset;
					newThostOrder.VolumeTotalOriginal = iter_e->VolumeTotal;
					newThostOrder.VolumeTotal = newThostOrder.VolumeTotalOriginal;
					newThostOrder.VolumeTraded = 0;

					ReqCloseOrderInsert(&newThostOrder, newThostOrder.OpenTime);

					CloseOrderList.erase(iter_e);
					CloseOrderCount++;
					if (CloseOrderCount >= 6) break;
				}
			}//End IF Closing buy order
			else if (iter_e->Direction == MORDER_BUY &&
				(iter_e->OrderStatus == MORDER_PART_TRADED || iter_e->OrderStatus == MORDER_ACCEPTED || iter_e->OrderStatus == MORDER_QUEUED)) {
				if (iter_e->MOrderType == -1) SellOrderNeedStopExist = true;
				if (iter_e->CanbeCanceled && iter_e->IsClosePofitOrder && iter_e->MOrderType == -1) {
					char line[200];
					sprintf(line, "%s,Sell Order Chop Stoploss Close,%.4f,%.4f,%.4f,OrderId=%d", mStrategyAndInstance.c_str(), iter_e->OpenOrderTradePrice, iter_e->maxProfit, m_Price, iter_e->OrderId);
					WriteMsgToLogList(line);

					MyCloseOrderType newThostOrder;

					newThostOrder.OrderLocalRetReqID = 0;
					newThostOrder.OpenOrderID = iter_e->OpenOrderID;
					newThostOrder.CloseOrderSeqNo = 0;
					strcpy(newThostOrder.OpenTime, iter_e->OpenTime);
					newThostOrder.IsStoplessOrder = true;
					newThostOrder.CanbeCanceled = true;
					newThostOrder.IsClosePofitOrder = false;
					newThostOrder.OpenOrderTradePrice = iter_e->OpenOrderTradePrice;
					newThostOrder.MOrderType = iter_e->MOrderType;
					newThostOrder.Direction = iter_e->Direction;
					newThostOrder.LimitPrice = m_Sell1;
					newThostOrder.OrigSubmitPrice = newThostOrder.LimitPrice;
					newThostOrder.Offset = iter_e->Offset;
					newThostOrder.VolumeTotalOriginal = iter_e->VolumeTotal;
					newThostOrder.VolumeTotal = newThostOrder.VolumeTotalOriginal;
					newThostOrder.VolumeTraded = 0;

					ReqCloseOrderInsert(&newThostOrder, newThostOrder.OpenTime);
					CloseOrderList.erase(iter_e);
					CloseOrderCount++;
					if (CloseOrderCount >= 6) break;
					m_bStoplossClose = true;
				}
				else if (iter_e->LimitPrice < (m_Buy1 - 0.00001) && iter_e->IsStoplessOrder && iter_e->CanbeCanceled) {
					char line[200];
					sprintf(line, "%s,Sell Order Close ReSubmit,OrderId=%d", mStrategyAndInstance.c_str(), iter_e->OrderId);
					WriteMsgToLogList(line);
					iter_e->NextCloseOrderPrice = m_Sell1;
					ReqOrderDelete(iter_e->OrderId, iter_e->OrderLocalRef, iter_e->FrontID, iter_e->SessionID);
					iter_e->CanbeCanceled = false;
				}
				else if (iter_e->IsClosePofitOrder && ((m_Price + 0.00001) > (iter_e->OpenOrderTradePrice - iter_e->maxProfit * (1 - mStrategyParams.BackPerc))) && iter_e->MAStop) {
					char line[200];
					sprintf(line, "%s,Sell Order Trace Stoploss Close,%.4f,%.4f,%.4f,OrderId=%d", mStrategyAndInstance.c_str(), iter_e->OpenOrderTradePrice, iter_e->maxProfit, m_Price, iter_e->OrderId);
					WriteMsgToLogList(line);

					MyCloseOrderType newThostOrder;

					newThostOrder.OrderLocalRetReqID = 0;
					newThostOrder.OpenOrderID = iter_e->OpenOrderID;
					newThostOrder.CloseOrderSeqNo = 0;
					strcpy(newThostOrder.OpenTime, iter_e->OpenTime);
					newThostOrder.IsStoplessOrder = true;
					newThostOrder.CanbeCanceled = true;
					newThostOrder.IsClosePofitOrder = false;
					newThostOrder.OpenOrderTradePrice = iter_e->OpenOrderTradePrice;
					newThostOrder.MOrderType = iter_e->MOrderType;
					newThostOrder.Direction = iter_e->Direction;
					newThostOrder.LimitPrice = m_Sell1;
					newThostOrder.OrigSubmitPrice = newThostOrder.LimitPrice;
					newThostOrder.Offset = iter_e->Offset;
					newThostOrder.VolumeTotalOriginal = iter_e->VolumeTotal;
					newThostOrder.VolumeTotal = newThostOrder.VolumeTotalOriginal;
					newThostOrder.VolumeTraded = 0;

					ReqCloseOrderInsert(&newThostOrder, newThostOrder.OpenTime);
					CloseOrderList.erase(iter_e);
					CloseOrderCount++;
					if (CloseOrderCount >= 6) break;
				}
			}//End IF Closing sell order
			//}//End for Strategy 1
		}//End Looping close order list
	}//End if the close order list is not null

	int nBuyOrderType = -1;
	int nSellOrderType = -1;
	int nGrid = ES_GRIDCOUNT >> 1;
	if (OpenOrderList.empty() && !CloseOrderList.empty()) {
		std::list<MyCloseOrderType>::iterator closeorder_it;

		int nSellMaxType = 0;
		for (closeorder_it = CloseOrderList.begin(); closeorder_it != CloseOrderList.end(); ++closeorder_it) {
			if (MORDER_BUY == closeorder_it->Direction && closeorder_it->MOrderType != -1) {
				nSellMaxType = max(closeorder_it->MOrderType, nSellMaxType);
			}
		}
		nSellOrderType = nSellMaxType + 1;

		int nBuyMaxType = 9999;
		for (closeorder_it = CloseOrderList.begin(); closeorder_it != CloseOrderList.end(); ++closeorder_it) {
			if (MORDER_SELL == closeorder_it->Direction && closeorder_it->MOrderType != -1) {
				nBuyMaxType = min(closeorder_it->MOrderType, nBuyMaxType);
			}
		}
		nBuyOrderType = nBuyMaxType - 1;
	}
	else if (OpenOrderList.empty() && CloseOrderList.empty()) {
		nBuyOrderType = nGrid - 1;
		nSellOrderType = nGrid;
	}

	// 砍买仓位
	if (mStrategyParams.OpenBuyAllow && nBuyOrderType < 30 - mStrategyParams.DnGridCount && mStrategyParams.DnGridCount>0 && !BuyOrderNeedStopExist) {
		int nCount = 1000;
		if (0 == mStrategyParams.OffFlagPoint) nCount = nGrid - nBuyOrderType;
		else nCount = nGrid - nBuyOrderType - 1;

		double StopPrice = mStrategyParams.BasePrice - mStrategyParams.DnGridDistPoint * nCount * m_dOneTick;
		if ((m_Price + 0.00001 < StopPrice)) {
			int MoveGridCount = mStrategyParams.DnGridCount / 2;//基准价往上移动的网格数目
			double oldBasePrice = mStrategyParams.BasePrice;
			mStrategyParams.BasePrice = mStrategyParams.BasePrice - mStrategyParams.DnGridDistPoint * MoveGridCount * m_dOneTick;
			//mClosePrice=mStrategyParams.BasePrice;

			std::list<MyCloseOrderType>::iterator itr_tmp;
			for (itr_tmp = CloseOrderList.begin(); itr_tmp != CloseOrderList.end(); ++itr_tmp) {
				if ((itr_tmp->OrderStatus == MORDER_ACCEPTED || itr_tmp->OrderStatus == MORDER_QUEUED)
					&& itr_tmp->Direction == MORDER_SELL && (itr_tmp->MOrderType >= 30 - MoveGridCount && itr_tmp->MOrderType <= 29))
					itr_tmp->MOrderType = -1;//待平仓
			}

			for (itr_tmp = CloseOrderList.begin(); itr_tmp != CloseOrderList.end(); ++itr_tmp) {
				if ((itr_tmp->OrderStatus == MORDER_ACCEPTED || itr_tmp->OrderStatus == MORDER_QUEUED) && itr_tmp->Direction == MORDER_SELL
					&& (itr_tmp->MOrderType >= 30 - mStrategyParams.DnGridCount && itr_tmp->MOrderType < 30 - MoveGridCount)) {
					//GridCloseOrderCount[itr_tmp->MOrderType]=0;
					itr_tmp->MOrderType = itr_tmp->MOrderType + MoveGridCount;
				}
			}

			for (int i = 29 - (mStrategyParams.DnGridCount - MoveGridCount); i >= 0; --i) GridCloseOrderCount[i] = 0;
		}
	}
	// 砍卖仓位
	if (mStrategyParams.OpenSellAllow && nSellOrderType > 29 + mStrategyParams.UpGridCount && mStrategyParams.UpGridCount > 0 && !SellOrderNeedStopExist) {
		int nCount = 1000;
		if (0 == mStrategyParams.OffFlagPoint) nCount = nSellOrderType - nGrid + 1;
		else nCount = nSellOrderType - nGrid;

		double StopPrice = mStrategyParams.BasePrice + mStrategyParams.UpGridDistPoint * nCount * m_dOneTick;
		if ((m_Price + 0.00001 > StopPrice)) {
			int MoveGridCount = mStrategyParams.UpGridCount / 2;//基准价往下移动的网格数目
			mStrategyParams.BasePrice = mStrategyParams.BasePrice + mStrategyParams.UpGridDistPoint * MoveGridCount * m_dOneTick;

			std::list<MyCloseOrderType>::iterator itr_tmp;
			for (itr_tmp = CloseOrderList.begin(); itr_tmp != CloseOrderList.end(); ++itr_tmp) {
				if ((itr_tmp->OrderStatus == MORDER_ACCEPTED || itr_tmp->OrderStatus == MORDER_QUEUED) && itr_tmp->Direction == MORDER_BUY
					&& (itr_tmp->MOrderType >= 30 && itr_tmp->MOrderType < 30 + MoveGridCount))
					itr_tmp->MOrderType = -1;//待平仓
			}

			for (itr_tmp = CloseOrderList.begin(); itr_tmp != CloseOrderList.end(); ++itr_tmp) {
				if ((itr_tmp->OrderStatus == MORDER_ACCEPTED || itr_tmp->OrderStatus == MORDER_QUEUED) && itr_tmp->Direction == MORDER_BUY
					&& (itr_tmp->MOrderType >= 30 + MoveGridCount && itr_tmp->MOrderType < 30 + mStrategyParams.UpGridCount)) {
					//GridCloseOrderCount[itr_tmp->MOrderType]=0;
					itr_tmp->MOrderType = itr_tmp->MOrderType - MoveGridCount;
				}
			}

			for (int i = 30 + mStrategyParams.UpGridCount - MoveGridCount; i < 60; ++i) GridCloseOrderCount[i] = 0;
		}
	}

	//if (nHour_m==nOpenHour&&nMin_m==nOpenMin&&!mOrderRet) mOrderRet=true;

	// 挂上网格
	if ((nSellOrderType >= nGrid) && (nSellOrderType <= (nGrid + mStrategyParams.UpGridCount - 1)) && nSec_m > nSecond) {
		if (0 == GridCloseOrderCount[nSellOrderType]) {
			int nCount = nSellOrderType - 30 + 1;
			double dPrice = mStrategyParams.BasePrice - mStrategyParams.OffFlagPoint * m_dOneTick + mStrategyParams.UpGridDistPoint * nCount * m_dOneTick;
			// 开盘没持仓时进单
			bool bFlag = false;
			if (nHour_m == nOpenHour && nMin_m == nOpenMin && !mOrderRet && CloseOrderList.empty() && OpenOrderList.empty()) {
				bFlag = true;
				mOrderRet = true;
			}
			else {
				if (m_Price + 0.00001 > dPrice) bFlag = true;
			}
			if (timeRuleForOpen(pDepthMarketData->datadate, pDepthMarketData->updatetime)
				&& mStrategyParams.OpenSellAllow && (mStrategyParams.BasePrice - 0.00001 > 0)
				&& OnHandPositionCount <= MaxOnHandPositionCount && TotalOnHandPosition <= MaxTotalOnHandPosition && bFlag) {
				//Open Sell Order
				MyOpenOrderType openThostOrder;
				iNextOrderRef++;
				openThostOrder.OrderLocalRetReqID = 0;
				openThostOrder.OrderId = -1;
				openThostOrder.OrderLocalRef = -1;
				int nCount = nSellOrderType - nGrid + 1;
				//openThostOrder.LimitPrice=mStrategyParams.BasePrice+mStrategyParams.UpGridDistPoint*nCount*m_dOneTick-mStrategyParams.OffFlagPoint*m_dOneTick;
				openThostOrder.LimitPrice = pDepthMarketData->lowerLimitPrice;
				openThostOrder.OrigSubmitPrice = openThostOrder.LimitPrice;
				openThostOrder.Direction = MORDER_SELL;
				openThostOrder.Offset = MORDER_OPEN;
				openThostOrder.VolumeTotal = mStrategyParams.GridVol;
				openThostOrder.VolumeTotalOriginal = openThostOrder.VolumeTotal;
				openThostOrder.VolumeTraded = 0;
				openThostOrder.ProfitPrice = openThostOrder.LimitPrice - mStrategyParams.ProfitPoint * m_dOneTick;
				openThostOrder.OpenOrderCanbeCanceled = true;
				openThostOrder.MOrderType = nSellOrderType;
				openThostOrder.maxProfit = 0.0;
				openThostOrder.MAStop = false;

				GridCloseOrderCount[nSellOrderType] = openThostOrder.VolumeTotal;
				mOpenTimes++;
				openThostOrder.mStoplossPoint = mStrategyParams.StoplossPoint;

				ReqOpenOrderInsert(&openThostOrder);

				OnHandPositionCount += openThostOrder.VolumeTotal;
				TotalOnHandPosition += openThostOrder.VolumeTotal;

				char log[200];
				sprintf(log, "%s,上网格开仓,卖=%.5f,时间=%s,网格:%d", mStrategyAndInstance.c_str(), openThostOrder.LimitPrice, pDepthMarketData->updatetime, openThostOrder.MOrderType);
				AddtoTipMsgListBox(log);
				WriteMsgToLogList(log);
			}
		}
	}

	// 挂下网格
	if (nBuyOrderType >= (nGrid - mStrategyParams.DnGridCount) && (nBuyOrderType <= nGrid - 1) && nSec_m > nSecond) {
		if (0 == GridCloseOrderCount[nBuyOrderType]) {
			int nCount = 29 - nBuyOrderType + 1;
			double dPrice = mStrategyParams.BasePrice + mStrategyParams.OffFlagPoint * m_dOneTick - mStrategyParams.DnGridDistPoint * nCount * m_dOneTick;
			// 开盘进单
			bool bFlag = false;
			if (nHour_m == nOpenHour && nMin_m == nOpenMin && !mOrderRet && CloseOrderList.empty() && OpenOrderList.empty()) {
				bFlag = true;
				mOrderRet = true;
			}
			else {
				if (m_Price - 0.00001 < dPrice) bFlag = true;
			}
			if (timeRuleForOpen(pDepthMarketData->datadate, pDepthMarketData->updatetime)
				&& mStrategyParams.OpenBuyAllow && (mStrategyParams.BasePrice - 0.00001 > 0)
				&& OnHandPositionCount <= MaxOnHandPositionCount && TotalOnHandPosition <= MaxTotalOnHandPosition && bFlag) {
				MyOpenOrderType openThostOrder;
				iNextOrderRef++;
				openThostOrder.OrderLocalRetReqID = 0;
				openThostOrder.OrderId = -1;
				openThostOrder.OrderLocalRef = -1;
				int nCount = nGrid - nBuyOrderType;
				//openThostOrder.LimitPrice=mStrategyParams.BasePrice-mStrategyParams.DnGridDistPoint*nCount*m_dOneTick+mStrategyParams.OffFlagPoint*m_dOneTick;
				openThostOrder.LimitPrice = pDepthMarketData->upperLimitPrice;
				openThostOrder.OrigSubmitPrice = openThostOrder.LimitPrice;
				openThostOrder.Direction = MORDER_BUY;
				openThostOrder.Offset = MORDER_OPEN;
				openThostOrder.VolumeTotal = mStrategyParams.GridVol;;
				openThostOrder.VolumeTotalOriginal = openThostOrder.VolumeTotal;
				openThostOrder.VolumeTraded = 0;
				openThostOrder.ProfitPrice = openThostOrder.LimitPrice + mStrategyParams.ProfitPoint * m_dOneTick;
				openThostOrder.OpenOrderCanbeCanceled = true;
				openThostOrder.MOrderType = nBuyOrderType;
				openThostOrder.MAStop = false;
				openThostOrder.maxProfit = 0.0;

				GridCloseOrderCount[nBuyOrderType] = openThostOrder.VolumeTotal;
				mOpenTimes++;
				openThostOrder.mStoplossPoint = mStrategyParams.StoplossPoint;

				ReqOpenOrderInsert(&openThostOrder);

				OnHandPositionCount += openThostOrder.VolumeTotal;
				TotalOnHandPosition += openThostOrder.VolumeTotal;

				char log[200];
				sprintf(log, "%s,下网格开仓,买=%.5f,时间=%s,网格:%d", mStrategyAndInstance.c_str(), openThostOrder.LimitPrice, pDepthMarketData->updatetime, openThostOrder.MOrderType);
				AddtoTipMsgListBox(log);
				WriteMsgToLogList(log);
			}
		}
	}

	//strcpy(curTradingDay,CTPTradingDay);
	//if (nHour_m==nOpenHour&&nMin_m>nOpenMin&&mOrderRet) mOrderRet=false;
	FlushStrategyInfoToFile();
}

void StrategyGridMAStopGTCChop::OnRtnOrder(OrderTradeMsg* pOrderTradeMsg)
{
	int thisOrderId = (pOrderTradeMsg->OrderSysId);
	//UpdateOrderIdToOrderList(thisOrderId);
	char line[200];
	sprintf(line, "%s,OnRtnOrder,OrderId=%d,Status=%c,VolRemain=%d", mStrategyAndInstance.c_str(), thisOrderId, pOrderTradeMsg->OrderStatus, pOrderTradeMsg->VolumeTotal);
	WriteMsgToLogList(line);

	std::list<MyOpenOrderType>::iterator openorder_it;
	if (!OpenOrderList.empty()) {
		for (openorder_it = OpenOrderList.begin(); openorder_it != OpenOrderList.end(); openorder_it++) {
			//std::list<MyOpenOrderType>::iterator iter_e=openorder_it++;
			if (openorder_it->OrderId == thisOrderId || (openorder_it->OrderLocalRef == pOrderTradeMsg->OrderLocalRef && openorder_it->OrderLocalRef > 0)) {
				//printf("OpenOrderList exist order \n");
				if (openorder_it->OrderLocalRef == pOrderTradeMsg->OrderLocalRef && openorder_it->OrderLocalRef > 0)openorder_it->OrderId = thisOrderId;
				//CTP未返回OnRspOrderInsert,在此处更新map
				map<int, string>::iterator iter;
				AcquireSRWLockExclusive(&g_srwLockReqId);
				iter = OrderIdToStrategyNameForDisplay.find(thisOrderId);
				if (iter == OrderIdToStrategyNameForDisplay.end()) {
					string sfinal(mStrategyAndInstance);
					sfinal.append("_open");
					OrderIdToStrategyNameForDisplay.insert(std::pair<int, string>(thisOrderId, sfinal));
				}
				ReleaseSRWLockExclusive(&g_srwLockReqId);

				openorder_it->OrderStatus = pOrderTradeMsg->OrderStatus;
				if (pOrderTradeMsg->OrderStatus == MORDER_PART_CANCELLED || pOrderTradeMsg->OrderStatus == MORDER_CANCELLED) {
					//开仓单被撤,不重新报单
					OnHandPositionCount -= pOrderTradeMsg->VolumeTotal;
					TotalOnHandPosition -= pOrderTradeMsg->VolumeTotal;
					// 更新网格数量
					GridCloseOrderCount[openorder_it->MOrderType] -= pOrderTradeMsg->VolumeTotal;
					mOpenTimes--;

					OpenOrderList.erase(openorder_it);
					break;
				}
			} //End if openorder_it->OrderRef==thisTradeRef
		}//End While
	}

	std::list<MyCloseOrderType>::iterator closeorder_it;
	if (!CloseOrderList.empty()) {
		for (closeorder_it = CloseOrderList.begin(); closeorder_it != CloseOrderList.end(); closeorder_it++) {
			//std::list<MyCloseOrderType>::iterator iter_ec=closeorder_it++;
			if (closeorder_it->OrderId == thisOrderId || (closeorder_it->OrderLocalRef == pOrderTradeMsg->OrderLocalRef && closeorder_it->OrderLocalRef > 0)) {
				if (closeorder_it->OrderLocalRef == pOrderTradeMsg->OrderLocalRef && closeorder_it->OrderLocalRef > 0)closeorder_it->OrderId = thisOrderId;
				//CTP未返回OnRspOrderInsert,在此处更新map
				map<int, string>::iterator iter;
				AcquireSRWLockExclusive(&g_srwLockReqId);
				iter = OrderIdToStrategyNameForDisplay.find(thisOrderId);
				if (iter == OrderIdToStrategyNameForDisplay.end()) {
					string sfinal(mStrategyAndInstance);
					sfinal.append("_close");
					OrderIdToStrategyNameForDisplay.insert(std::pair<int, string>(thisOrderId, sfinal));
				}
				ReleaseSRWLockExclusive(&g_srwLockReqId);

				closeorder_it->OrderStatus = pOrderTradeMsg->OrderStatus;
				if (pOrderTradeMsg->OrderStatus == MORDER_PART_CANCELLED || pOrderTradeMsg->OrderStatus == MORDER_CANCELLED)
				{
					//pOrderTradeMsg->OrderStatus=MORDER_WAITFORSUBMIT;
					if (closeorder_it->IsClosePofitOrder || closeorder_it->IsStoplessOrder) {
						if (closeorder_it->Direction == MORDER_SELL) {
							char line[200];
							sprintf(line, "%s,Buy Order Stoploss Close,OrderId=%d", mStrategyAndInstance.c_str(), closeorder_it->OrderId);
							WriteMsgToLogList(line);

							MyCloseOrderType newThostOrder;
							iNextOrderRef++;
							newThostOrder.OrderLocalRetReqID = 0;
							strcpy(newThostOrder.OpenTime, closeorder_it->OpenTime);
							newThostOrder.IsStoplessOrder = true;
							newThostOrder.CanbeCanceled = true;
							newThostOrder.IsClosePofitOrder = false;
							newThostOrder.OpenOrderTradePrice = closeorder_it->OpenOrderTradePrice;
							newThostOrder.MOrderType = closeorder_it->MOrderType;
							newThostOrder.Direction = closeorder_it->Direction;
							newThostOrder.LimitPrice = m_Buy1 - 20 * m_dOneTick;//以最新价进行报止损单
							newThostOrder.OrigSubmitPrice = newThostOrder.LimitPrice;
							newThostOrder.Offset = closeorder_it->Offset;
							newThostOrder.VolumeTotalOriginal = closeorder_it->VolumeTotal;
							newThostOrder.VolumeTotal = newThostOrder.VolumeTotalOriginal;
							newThostOrder.VolumeTraded = 0;
							newThostOrder.OpenOrderID = closeorder_it->OpenOrderID;

							ReqCloseOrderInsert(&newThostOrder, newThostOrder.OpenTime);
						}
						else if (closeorder_it->Direction == MORDER_BUY) {
							char line[200];
							sprintf(line, "%s,Sell Order Stoploss Close,OrderId=%d", mStrategyAndInstance.c_str(), closeorder_it->OrderId);
							WriteMsgToLogList(line);

							MyCloseOrderType newThostOrder;
							iNextOrderRef++;
							newThostOrder.OrderLocalRetReqID = 0;
							strcpy(newThostOrder.OpenTime, closeorder_it->OpenTime);
							newThostOrder.IsStoplessOrder = true;
							newThostOrder.CanbeCanceled = true;
							newThostOrder.IsClosePofitOrder = false;
							newThostOrder.OpenOrderTradePrice = closeorder_it->OpenOrderTradePrice;
							newThostOrder.MOrderType = closeorder_it->MOrderType;
							newThostOrder.Direction = closeorder_it->Direction;
							newThostOrder.LimitPrice = m_Sell1 + 20 * m_dOneTick;//以最新价进行报止损单
							newThostOrder.OrigSubmitPrice = newThostOrder.LimitPrice;
							newThostOrder.Offset = closeorder_it->Offset;
							newThostOrder.VolumeTotalOriginal = closeorder_it->VolumeTotal;
							newThostOrder.VolumeTotal = newThostOrder.VolumeTotalOriginal;
							newThostOrder.VolumeTraded = 0;
							newThostOrder.OpenOrderID = closeorder_it->OpenOrderID;

							ReqCloseOrderInsert(&newThostOrder, newThostOrder.OpenTime);
						}
						CloseOrderList.erase(closeorder_it);
						break;
					}
				}
			}//End if closeorder_it->OrderRef==thisOrderRef for Strategy 2
		}//End While
	}

	if (pOrderTradeMsg->OrderStatus == MORDER_PART_CANCELLED || pOrderTradeMsg->OrderStatus == MORDER_CANCELLED) {
		map<int, int>::iterator iter;
		AcquireSRWLockExclusive(&g_srwLockOrderId);
		iter = OrderIdToShmIndex.find(pOrderTradeMsg->OrderSysId);
		if (iter != OrderIdToShmIndex.end()) {
			OrderIdToShmIndex.erase(iter);
		}
		ReleaseSRWLockExclusive(&g_srwLockOrderId);
	}

	FlushStrategyInfoToFile();
}

void StrategyGridMAStopGTCChop::OnRtnTrade(OrderTradeMsg* pOrderTradeMsg)
{
	int thisTradeRef = pOrderTradeMsg->OrderSysId;
	string strmatchno(pOrderTradeMsg->MatchNo);
	map<string, string>::iterator iter;
	iter = matchnomap.find(strmatchno);
	if (iter == matchnomap.end()) {
		matchnomap.insert(std::pair<string, string>(strmatchno, strmatchno));

		//UpdateOrderIdToOrderList(thisTradeRef);
		char line[200];
		sprintf(line, "%s,OnRtnTrade,OrderId=%d,vol=%d,price=%.5f", mStrategyAndInstance.c_str(), thisTradeRef, pOrderTradeMsg->Volume, pOrderTradeMsg->Price);
		WriteMsgToLogList(line);

		char DataDate[32] = { 0 };
		char* DataTime;
		strcpy(DataDate, pOrderTradeMsg->InsertOrTradeTime);
		if (strlen(DataDate) >= 17) {
			DataTime = strstr(DataDate, " ");
			*(DataTime++) = '\0';
		}

		double submitprice = 0;
		int tradedirection = -1;
		int openorclose = -1;
		std::list<MyOpenOrderType>::iterator openorder_it;
		if (!OpenOrderList.empty()) {
			for (openorder_it = OpenOrderList.begin(); openorder_it != OpenOrderList.end(); openorder_it++) {
				//std::list<MyOpenOrderType>::iterator iter_e=openorder_it++;
				if (openorder_it->OrderId == thisTradeRef || (openorder_it->OrderLocalRef == pOrderTradeMsg->OrderLocalRef && openorder_it->OrderLocalRef > 0)) {
					if (openorder_it->OrderLocalRef == pOrderTradeMsg->OrderLocalRef && openorder_it->OrderLocalRef > 0)openorder_it->OrderId = thisTradeRef;
					openorclose = MORDER_OPEN;
					openorder_it->VolumeTraded = openorder_it->VolumeTraded + pOrderTradeMsg->Volume;
					openorder_it->VolumeTotal = openorder_it->VolumeTotal - pOrderTradeMsg->Volume;
					strcpy(openorder_it->OpenTime, pOrderTradeMsg->InsertOrTradeTime);

					if (openorder_it->Direction == MORDER_BUY)
					{
						tradedirection = MORDER_BUY;
						MyCloseOrderType thostOrder;
						thostOrder.OrderId = -1;
						thostOrder.OrderLocalRef = -1;
						thostOrder.OpenOrderID = thisTradeRef;
						thostOrder.OpenOrderSubmitPrice = openorder_it->OrigSubmitPrice;
						thostOrder.OpenOrderTradePrice = pOrderTradeMsg->Price;
						thostOrder.IsClosePofitOrder = true;
						thostOrder.IsStoplessOrder = false;
						thostOrder.CanbeCanceled = true;
						thostOrder.dwCloseOrderStart = GetTickCount();
						thostOrder.MOrderType = openorder_it->MOrderType;
						thostOrder.MAStop = false;
						strcpy(thostOrder.OpenTime, pOrderTradeMsg->InsertOrTradeTime);
						iNextOrderRef++;
						thostOrder.OrderLocalRetReqID = 0;
						thostOrder.CloseOrderSeqNo = ++mCloseOrderSeqNo;
						thostOrder.Offset = MORDER_CLOSE;
						thostOrder.OrderStatus = MORDER_ACCEPTED;
						thostOrder.Direction = MORDER_SELL;
						thostOrder.LimitPrice = openorder_it->ProfitPrice;
						thostOrder.OrigSubmitPrice = thostOrder.LimitPrice;
						thostOrder.ProfitPrice = openorder_it->ProfitPrice;
						thostOrder.VolumeTotalOriginal = pOrderTradeMsg->Volume;
						thostOrder.VolumeTraded = 0;
						thostOrder.VolumeTotal = pOrderTradeMsg->Volume;//
						thostOrder.maxProfit = 0;
						thostOrder.mStoplossPoint = openorder_it->mStoplossPoint;
						thostOrder.ManualStopPrice = 0;

						//ReqCloseOrderInsert(&thostOrder,openorder_it->OpenTime);
						CloseOrderList.push_back(thostOrder);

						submitprice = openorder_it->OrigSubmitPrice;

						if (openorder_it->VolumeTotal == 0) {
							OpenOrderList.erase(openorder_it);

							map<int, int>::iterator iter;
							AcquireSRWLockExclusive(&g_srwLockOrderId);
							iter = OrderIdToShmIndex.find(pOrderTradeMsg->OrderSysId);
							if (iter != OrderIdToShmIndex.end()) {
								OrderIdToShmIndex.erase(iter);
							}
							ReleaseSRWLockExclusive(&g_srwLockOrderId);
						}
						break;
					}
					else if (openorder_it->Direction == MORDER_SELL) {
						tradedirection = MORDER_SELL;
						MyCloseOrderType thostOrder;
						thostOrder.OrderId = -1;
						thostOrder.OrderLocalRef = -1;
						thostOrder.OpenOrderID = thisTradeRef;
						thostOrder.OpenOrderSubmitPrice = openorder_it->OrigSubmitPrice;
						thostOrder.OpenOrderTradePrice = pOrderTradeMsg->Price;
						thostOrder.IsClosePofitOrder = true;
						thostOrder.IsStoplessOrder = false;
						thostOrder.CanbeCanceled = true;
						thostOrder.dwCloseOrderStart = GetTickCount();
						thostOrder.MOrderType = openorder_it->MOrderType;
						thostOrder.MAStop = false;
						strcpy(thostOrder.OpenTime, pOrderTradeMsg->InsertOrTradeTime);
						iNextOrderRef++;
						thostOrder.OrderLocalRetReqID = 0;
						thostOrder.CloseOrderSeqNo = ++mCloseOrderSeqNo;
						thostOrder.Offset = MORDER_CLOSE;
						thostOrder.OrderStatus = MORDER_ACCEPTED;
						thostOrder.Direction = MORDER_BUY;
						thostOrder.LimitPrice = openorder_it->ProfitPrice;
						thostOrder.OrigSubmitPrice = thostOrder.LimitPrice;
						thostOrder.ProfitPrice = openorder_it->ProfitPrice;
						thostOrder.VolumeTotalOriginal = pOrderTradeMsg->Volume;
						thostOrder.VolumeTraded = 0;
						thostOrder.VolumeTotal = pOrderTradeMsg->Volume;//
						thostOrder.maxProfit = 0;
						thostOrder.mStoplossPoint = openorder_it->mStoplossPoint;
						thostOrder.ManualStopPrice = 0;

						//ReqCloseOrderInsert(&thostOrder,openorder_it->OpenTime);
						CloseOrderList.push_back(thostOrder);
						submitprice = openorder_it->OrigSubmitPrice;

						if (openorder_it->VolumeTotal == 0) {
							OpenOrderList.erase(openorder_it);

							map<int, int>::iterator iter;
							AcquireSRWLockExclusive(&g_srwLockOrderId);
							iter = OrderIdToShmIndex.find(pOrderTradeMsg->OrderSysId);
							if (iter != OrderIdToShmIndex.end()) {
								OrderIdToShmIndex.erase(iter);
							}
							ReleaseSRWLockExclusive(&g_srwLockOrderId);
						}
						break;
					}
				} //End if openorder_it->OrderRef==thisTradeRef
			}
		}

		int closeprofitornot = -1;
		int openid = -1;
		std::list<MyCloseOrderType>::iterator closeorder_it;
		if (!CloseOrderList.empty()) {
			for (closeorder_it = CloseOrderList.begin(); closeorder_it != CloseOrderList.end(); closeorder_it++) {
				//std::list<MyCloseOrderType>::iterator iter_ec=closeorder_it++;
				if (closeorder_it->OrderId == thisTradeRef || (closeorder_it->OrderLocalRef == pOrderTradeMsg->OrderLocalRef && closeorder_it->OrderLocalRef > 0)) {
					if (closeorder_it->OrderLocalRef == pOrderTradeMsg->OrderLocalRef && closeorder_it->OrderLocalRef > 0)closeorder_it->OrderId = thisTradeRef;
					openorclose = MORDER_CLOSE;
					openid = closeorder_it->OpenOrderID;
					//printf("CloseOrderList exist order \n");
					submitprice = closeorder_it->OrigSubmitPrice;
					if (closeorder_it->IsClosePofitOrder) {
						closeprofitornot = 1;
					}
					else closeprofitornot = 0;

					if (closeorder_it->Direction == MORDER_BUY) {
						tradedirection = MORDER_BUY;
					}
					else if (closeorder_it->Direction == MORDER_SELL) {
						tradedirection = MORDER_SELL;
					}

					closeorder_it->VolumeTraded = closeorder_it->VolumeTraded + pOrderTradeMsg->Volume;
					closeorder_it->VolumeTotal = closeorder_it->VolumeTotal - pOrderTradeMsg->Volume;

					OnHandPositionCount -= pOrderTradeMsg->Volume;
					TotalOnHandPosition -= pOrderTradeMsg->Volume;

					GridCloseOrderCount[closeorder_it->MOrderType] = GridCloseOrderCount[closeorder_it->MOrderType] - pOrderTradeMsg->Volume; //更新网格平仓单的未平数量

					if (pOrderTradeMsg->Volume == closeorder_it->VolumeTotalOriginal) {
						CloseOrderList.erase(closeorder_it);

						break;
						// 继续挂当前网格
						//int nGrid = closeorder_it->MOrderType;
						//int nGridCount=ES_GRIDCOUNT>>1;
						//if(timeRuleForOpen(DataDate,DataTime)
						//	&&((nGrid>=nGridCount)&&nGrid<=(nGridCount+mStrategyParams.UpGridCount-1))
						//	&&MORDER_BUY==closeorder_it->Direction
						//	&&0==GridCloseOrderCount[nGrid]&&mStrategyParams.OpenSellAllow
						//	&&OnHandPositionCount<=MaxOnHandPositionCount&&TotalOnHandPosition<=MaxTotalOnHandPosition){
						//		MyOpenOrderType openThostOrder;
						//		iNextOrderRef++;
						//		openThostOrder.OrderLocalRetReqID=0;
						//		openThostOrder.OrderId=-1;
						//		openThostOrder.OrderLocalRef=-1;
						//		openThostOrder.MOrderType=nGrid;
						//		int nCount=nGrid-nGridCount+1;
						//		//openThostOrder.LimitPrice=mStrategyParams.BasePrice+(mStrategyParams.UpGridDistPoint*nCount)*m_dOneTick-mStrategyParams.OffFlagPoint*m_dOneTick;
						//		openThostOrder.LimitPrice=m_Buy1-20*m_dOneTick;
						//		openThostOrder.OrigSubmitPrice=openThostOrder.LimitPrice;
						//		openThostOrder.Direction=MORDER_SELL;
						//		openThostOrder.Offset=MORDER_OPEN;
						//		openThostOrder.VolumeTotal=mStrategyParams.GridVol;
						//		openThostOrder.VolumeTotalOriginal=openThostOrder.VolumeTotal;
						//		openThostOrder.VolumeTraded=0;

						//		openThostOrder.ProfitPrice=openThostOrder.LimitPrice-mStrategyParams.ProfitPoint*m_dOneTick;
						//		openThostOrder.mStoplossPoint=mStrategyParams.StoplossPoint;

						//		openThostOrder.OpenOrderCanbeCanceled=true;

						//		openThostOrder.maxProfit=0.0;
						//		openThostOrder.MAStop=false;
						//		GridCloseOrderCount[nGrid]=openThostOrder.VolumeTotal;
						//		ReqOpenOrderInsert(&openThostOrder);
						//		OnHandPositionCount+=openThostOrder.VolumeTotal;
						//		TotalOnHandPosition+=openThostOrder.VolumeTotal;

						//		char log[200];
						//		sprintf(log,"%s,开仓,卖=%.5f,时间=%s,网格:%d",mStrategyAndInstance.c_str(),openThostOrder.LimitPrice,pOrderTradeMsg->InsertOrTradeTime,openThostOrder.MOrderType);
						//		AddtoTipMsgListBox(log);
						//		WriteMsgToLogList(log);
						//		// 撤销下一网格挂单
						//		std::list<MyOpenOrderType>::iterator openorder_itother;
						//		for(openorder_itother = OpenOrderList.begin(); openorder_itother != OpenOrderList.end();openorder_itother++){
						//			if(((nGrid+1>=nGridCount)&&(nGrid+1)<=(nGridCount+mStrategyParams.UpGridCount-1))
						//				&&openorder_itother->Direction==MORDER_SELL&&GridCloseOrderCount[nGrid+1]>0
						//				&&openorder_itother->OpenOrderCanbeCanceled){
						//					ReqOrderDelete(openorder_itother->OrderId,openorder_itother->OrderLocalRef,openorder_itother->FrontID,openorder_itother->SessionID);
						//					openorder_itother->OpenOrderCanbeCanceled=false;
						//					break;
						//			}
						//		}

						//}else if(timeRuleForOpen(DataDate,DataTime)
						//	&&((nGrid>=(nGridCount-mStrategyParams.DnGridCount))&&(nGrid<=nGridCount-1))
						//	&&MORDER_SELL==closeorder_it->Direction&&0==GridCloseOrderCount[nGrid]&&mStrategyParams.OpenBuyAllow
						//	&&OnHandPositionCount<=MaxOnHandPositionCount&&TotalOnHandPosition<=MaxTotalOnHandPosition){
						//		MyOpenOrderType openThostOrder;
						//		iNextOrderRef++;
						//		openThostOrder.OrderLocalRetReqID=0;
						//		openThostOrder.OrderId=-1;
						//		openThostOrder.OrderLocalRef=-1;
						//		openThostOrder.MOrderType=nGrid;
						//		int nCount=nGridCount-nGrid;
						//		//openThostOrder.LimitPrice=mStrategyParams.BasePrice-(mStrategyParams.DnGridDistPoint*nCount)*m_dOneTick+mStrategyParams.OffFlagPoint*m_dOneTick;
						//		openThostOrder.LimitPrice=m_Sell1+20*m_dOneTick;
						//		openThostOrder.OrigSubmitPrice=openThostOrder.LimitPrice;
						//		openThostOrder.Direction=MORDER_BUY;
						//		openThostOrder.Offset=MORDER_OPEN;
						//		openThostOrder.VolumeTotal=mStrategyParams.GridVol;
						//		openThostOrder.VolumeTotalOriginal=openThostOrder.VolumeTotal;
						//		openThostOrder.VolumeTraded=0;

						//		openThostOrder.ProfitPrice=openThostOrder.LimitPrice+mStrategyParams.ProfitPoint*m_dOneTick;
						//		openThostOrder.mStoplossPoint=mStrategyParams.StoplossPoint;

						//		openThostOrder.OpenOrderCanbeCanceled=true;

						//		openThostOrder.maxProfit=0.0;
						//		openThostOrder.MAStop=false;
						//		GridCloseOrderCount[nGrid]=openThostOrder.VolumeTotal;
						//		ReqOpenOrderInsert(&openThostOrder);
						//		OnHandPositionCount+=openThostOrder.VolumeTotal;
						//		TotalOnHandPosition+=openThostOrder.VolumeTotal;

						//		char log[200];
						//		sprintf(log,"%s,开仓,买=%.5f,时间=%s,网格:%d",mStrategyAndInstance.c_str(),openThostOrder.LimitPrice,pOrderTradeMsg->InsertOrTradeTime,openThostOrder.MOrderType);
						//		AddtoTipMsgListBox(log);
						//		WriteMsgToLogList(log);

						//		// 撤销下一网格挂单
						//		std::list<MyOpenOrderType>::iterator openorder_itother;
						//		for(openorder_itother = OpenOrderList.begin(); openorder_itother != OpenOrderList.end();openorder_itother++){
						//			if(((nGrid-1>=(nGridCount-mStrategyParams.DnGridCount))&&(nGrid-1<=nGridCount-1))
						//				&&openorder_itother->Direction==MORDER_BUY&&GridCloseOrderCount[nGrid-1]>0
						//				&&openorder_itother->OpenOrderCanbeCanceled){
						//					ReqOrderDelete(openorder_itother->OrderId,openorder_itother->OrderLocalRef,openorder_itother->FrontID,openorder_itother->SessionID);
						//					openorder_itother->OpenOrderCanbeCanceled=false;
						//					break;
						//			}
						//		}
						//}

						//CloseOrderList.erase(closeorder_it);

						//map<int,int>::iterator iter;
						//AcquireSRWLockExclusive(&g_srwLockOrderId);
						//iter=OrderIdToShmIndex.find(pOrderTradeMsg->OrderSysId);
						//if(iter!=OrderIdToShmIndex.end()){
						//	OrderIdToShmIndex.erase(iter);
						//}
						//ReleaseSRWLockExclusive(&g_srwLockOrderId);

						//break;
					}
					else {
						if (closeorder_it->VolumeTotal == 0) {
							CloseOrderList.erase(closeorder_it);
							break;
						}
					}
				} //End if closeorder_it->OrderRef==thisTradeRef
			}//End While for Close Order List
		}

		if (tradedirection != -1) {
			TradeLogType trade;
			strcpy(trade.AccountID, LoginUserTDEsun);
			strcpy(trade.InstanceName, mInstanceName);
			strcpy(trade.StrategyID, mStrategyID);
			//int nYear,nMonth,nDate,nHour,nMin,nSec;
			string strfulltradetime(pOrderTradeMsg->InsertOrTradeTime);
			string strtimepart = strfulltradetime.substr(strfulltradetime.find_first_of(" ") + 1, strfulltradetime.length() - strfulltradetime.find_first_of(" ") - 1);
			//char ctimepart[10];
			//strcpy(ctimepart,strtimepart.c_str());
			string strdatepart = strfulltradetime.substr(0, strfulltradetime.find_first_of(" "));
			//nYear=atoi(strdatepart.c_str())/10000;
			//nMonth=(atoi(strdatepart.c_str())%10000)/100;
			//nDate=(atoi(strdatepart.c_str())%10000)%100;
			//sscanf_s(ctimepart, "%d:%d:%d",&nYear, &nMonth, &nDate,&nHour,&nMin,&nSec);
			//strcpy(trade.tradingday,strdatepart.c_str());
			CTime tm = CTime::GetCurrentTime();
			sprintf(trade.tradingday, "%04d%02d%02d", tm.GetYear(), tm.GetMonth(), tm.GetDay());
			strcpy(trade.tradingtime, strtimepart.c_str());
			//strcpy(trade.CodeName,InstCodeName);
			strcpy(trade.CodeName, m_InstCodeName);
			trade.tradeprice = pOrderTradeMsg->Price;
			trade.submitprice = submitprice;
			if (tradedirection == MORDER_BUY) {
				trade.qty = pOrderTradeMsg->Volume;
			}
			else trade.qty = -pOrderTradeMsg->Volume;
			trade.fee = pOrderTradeMsg->MatchFee;
			trade.openorclose = openorclose;
			if (openorclose == MORDER_OPEN) {
				trade.openid = thisTradeRef;
				trade.closeid = -1;
			}
			else {
				trade.openid = openid;
				trade.closeid = thisTradeRef;
			}

			strcpy(trade.MatchNo, pOrderTradeMsg->MatchNo);

			Message logMsg;
			logMsg.type = TRADE_LOG;
			logMsg.AddData(&trade, 0, sizeof(TradeLogType));
			LogMessageList.AddTail(logMsg);
			ReleaseSemaphore(logSemaphore, 1, NULL);

			// send udp message
			SendLogType send;
			globalFuncUtil.getUDPSendLog(&trade, &send);
			TradeList.AddTail(send);
			ReleaseSemaphore(tradeSemaphore, 1, NULL);
			// end send udp message

			WaitForSingleObject(MatchNoMapSem, INFINITE);
			string matchno(pOrderTradeMsg->MatchNo);
			MatchNoToStrategyNameForDisplay.insert(std::pair<string, string>(matchno, mStrategyAndInstance));
			ReleaseSemaphore(MatchNoMapSem, 1, NULL);

			DisplayTradeOnScreen(pOrderTradeMsg, tradedirection, openorclose, closeprofitornot);

			FlushStrategyInfoToFile();
		}
	}
}

void StrategyGridMAStopGTCChop::OnRspOrderInsert(ShmRspOrderInsert* pRspOrderInsert)
{
	int iRetReqID = pRspOrderInsert->iRetReqID;
	int OrderId = pRspOrderInsert->OrderID;
	if (!OpenOrderList.empty()) {
		std::list<MyOpenOrderType>::iterator openorder_it;
		for (openorder_it = OpenOrderList.begin(); openorder_it != OpenOrderList.end(); openorder_it++) {
			//std::list<MyOpenOrderType>::iterator iter_e=openorder_it++;
			if (openorder_it->OrderLocalRetReqID == iRetReqID) {
				openorder_it->OrderId = OrderId;

				map<int, int>::iterator iter;
				AcquireSRWLockExclusive(&g_srwLockReqId);
				iter = ReqIdToShmIndex.find(iRetReqID);
				if (iter != ReqIdToShmIndex.end()) {
					ReqIdToShmIndex.erase(iter);

					//string strategyname(mStrategyName);
					string sfinal(mStrategyAndInstance);
					sfinal.append("_open");
					OrderIdToStrategyNameForDisplay.insert(std::pair<int, string>(OrderId, sfinal));
				}
				ReleaseSRWLockExclusive(&g_srwLockReqId);
				return;
			}
		}
	}

	if (!CloseOrderList.empty()) {
		std::list<MyCloseOrderType>::iterator closeorder_it;
		for (closeorder_it = CloseOrderList.begin(); closeorder_it != CloseOrderList.end(); closeorder_it++) {
			//std::list<MyCloseOrderType>::iterator iter_ec=closeorder_it++;
			if (closeorder_it->OrderLocalRetReqID == iRetReqID) {
				closeorder_it->OrderId = OrderId;
				map<int, int>::iterator iter;
				AcquireSRWLockExclusive(&g_srwLockReqId);
				iter = ReqIdToShmIndex.find(iRetReqID);
				if (iter != ReqIdToShmIndex.end()) {
					ReqIdToShmIndex.erase(iter);
					//AcquireSRWLockExclusive(&g_srwLockOrderId);
					//OrderIdToShmIndex.insert(std::pair<int,int>(OrderId,shmindex));
					//ReleaseSRWLockExclusive(&g_srwLockOrderId);
					//string strategyname(mStrategyName);
					string sfinal(mStrategyAndInstance);
					sfinal.append("_close");
					OrderIdToStrategyNameForDisplay.insert(std::pair<int, string>(OrderId, sfinal));
				}
				ReleaseSRWLockExclusive(&g_srwLockReqId);
				return;
			}
		}
	}
}

void StrategyGridMAStopGTCChop::WideCharToMultiChar(CString str, char* dest_str)
{
	//获取缓冲区的大小，并申请空间，缓冲区大小是按字节计算的
	int len = WideCharToMultiByte(CP_ACP, 0, str, str.GetLength(), NULL, 0, NULL, NULL);
	dest_str = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, str, str.GetLength(), dest_str, len, NULL, NULL);
	dest_str[len] = '\0';
}

//datadate format: yyyymmdd , datatime formate hh24:mi:ss
//true - can open new order , false - shouldnot open new order
bool StrategyGridMAStopGTCChop::timeRuleForOpen(char datadate[10], char datatime[10]) {
	int nYear, nMonth, nDate;
	nYear = atoi(datadate) / 10000;
	nMonth = (atoi(datadate) % 10000) / 100;
	nDate = (atoi(datadate) % 10000) % 100;

	// 收盘和开盘之间,以及收盘前10min禁止开仓
	int nOpenHour, nOpenMin, nOpenSec, nCloseHour, nCloseMin, nCloseSec, nHour, nMin, nSec;
	sscanf_s(mStrategyParams.OpenTime, "%d:%d:%d", &nOpenHour, &nOpenMin, &nOpenSec);
	sscanf_s(mStrategyParams.CloseTime, "%d:%d:%d", &nCloseHour, &nCloseMin, &nCloseSec);
	sscanf_s(datatime, "%d:%d:%d", &nHour, &nMin, &nSec);
	if (nHour >= nCloseHour && nHour < nOpenHour) return false;

	CTime tm(nYear, nMonth, nDate, nCloseHour, nCloseMin, nCloseSec);
	time_t closeTime = tm.GetTime();

	sscanf_s(datatime, "%d:%d:%d", &nHour, &nMin, &nSec);

	CTime tm2(nYear, nMonth, nDate, nHour, nMin, nSec);
	time_t currTime = tm2.GetTime();

	if (closeTime - currTime >= 0 && closeTime - currTime < 600) return false;

	return true;
}

//datadate format: yyyymmdd , datatime formate hh24:mi:ss
//true - must close current order , false - no need to close current order
bool StrategyGridMAStopGTCChop::timeRuleForClose(char datadate[10], char datatime[10]) {
	//int nYear, nMonth, nDate;
	//nYear=atoi(datadate)/10000;
	//nMonth=(atoi(datadate)%10000)/100;
	//nDate=(atoi(datadate)%10000)%100;
	//int nHour,nMin,nSec;
	//sscanf_s(datatime, "%d:%d:%d",&nHour, &nMin, &nSec);
	//CTime tm(nYear, nMonth, nDate,nHour,nMin,nSec);
	//time_t curTime=tm.GetTime();

	//if(curTime>tmt_EndTime) return true;

	//return false;
	return false;
}

bool StrategyGridMAStopGTCChop::timeRuleForCancel(char datadate[10], char datatime[10]) {
	int nYear, nMonth, nDate;
	nYear = atoi(datadate) / 10000;
	nMonth = (atoi(datadate) % 10000) / 100;
	nDate = (atoi(datadate) % 10000) % 100;

	int nCHour, nCMin, nCSec;
	sscanf_s(mStrategyParams.CloseTime, "%d:%d:%d", &nCHour, &nCMin, &nCSec);

	CTime tm(nYear, nMonth, nDate, nCHour, nCMin, nCSec);
	time_t closeTime = tm.GetTime();

	int nHour, nMin, nSec;
	sscanf_s(datatime, "%d:%d:%d", &nHour, &nMin, &nSec);

	CTime tm2(nYear, nMonth, nDate, nHour, nMin, nSec);
	time_t currTime = tm2.GetTime();

	if (closeTime - currTime >= 0 && closeTime - currTime < 600) return true;

	return false;
}

bool StrategyGridMAStopGTCChop::timeRuleForClosePrice(char datadate[10], char datatime[10]) {
	int nHour, nMin, nSec, nCloseHour, nCloseMin, nCloseSec;
	sscanf_s(datatime, "%d:%d:%d", &nHour, &nMin, &nSec);
	sscanf_s(mStrategyParams.CloseTime, "%d:%d:%d", &nCloseHour, &nCloseMin, &nCloseSec);
	int nYear, nMonth, nDate;
	nYear = atoi(datadate) / 10000;
	nMonth = (atoi(datadate) % 10000) / 100;
	nDate = (atoi(datadate) % 10000) % 100;

	CTime tm(nYear, nMonth, nDate, nCloseHour, nCloseMin, nCloseSec);
	time_t closeTime = tm.GetTime();

	CTime tm2(nYear, nMonth, nDate, nHour, nMin, nSec);
	time_t currTime = tm2.GetTime();
	// 收盘前半小时读取价格
	if (closeTime - currTime > 0 && closeTime - currTime <= 1800) return true;
	return false;
}

void StrategyGridMAStopGTCChop::ReqOpenOrderInsert(MyOpenOrderType* pOpenOrder) {
	InsertOrderField cOrder;
	strcpy(cOrder.ClientNo, "");
	strcpy(cOrder.CommodityNo, m_CommodityNo);
	strcpy(cOrder.InstrumentID, m_InstID);
	cOrder.Direction = pOpenOrder->Direction;
	cOrder.Offset = pOpenOrder->Offset;
	cOrder.OrderPrice = floor((pOpenOrder->LimitPrice + 0.00001) / m_dOneTick) * m_dOneTick;
	cOrder.OrderVol = pOpenOrder->VolumeTotal;
	//cOrder.OrderLocalRef=pOpenOrder->OrderLocalRef;
	if (ThostTraderAPI || SgitTraderAPI) {
		pOpenOrder->OrderId = -1;
		if (ThostTraderAPI)pThostTraderSpi->ReqOrderInsert(&cOrder, m_bCrossTradingDay, false, shmindex);
		else pSgitTraderSpi->ReqOrderInsert(&cOrder, m_bCrossTradingDay, false, shmindex);

		pOpenOrder->OrderLocalRef = cOrder.OrderLocalRef;
		pOpenOrder->FrontID = cOrder.FrontID;
		pOpenOrder->SessionID = cOrder.SessionID;

		CTime mCurrTime = CTime::GetCurrentTime();
		CString str_mCurrTime = mCurrTime.Format("%Y%m%d %X");
		int len = WideCharToMultiByte(CP_ACP, 0, str_mCurrTime, str_mCurrTime.GetLength(), NULL, 0, NULL, NULL);
		char* c_str_mCurrTime = new char[len + 1];
		WideCharToMultiByte(CP_ACP, 0, str_mCurrTime, str_mCurrTime.GetLength(), c_str_mCurrTime, len, NULL, NULL);
		c_str_mCurrTime[len] = '\0';
		strcpy(pOpenOrder->SubmitDateAndTime, c_str_mCurrTime);
		strcpy(pOpenOrder->OpenTime, c_str_mCurrTime);
		delete c_str_mCurrTime;

		OpenOrderList.push_back((*pOpenOrder));
	}
	else {
		int iRetReqID;
		pEsunTraderSpi->ReqOrderInsert(&cOrder, &iRetReqID, m_bCrossTradingDay);
		pOpenOrder->OrderLocalRef = -1;
		pOpenOrder->OrderLocalRetReqID = iRetReqID;
		pOpenOrder->FrontID = cOrder.FrontID;
		pOpenOrder->SessionID = cOrder.SessionID;

		CTime mCurrTime = CTime::GetCurrentTime();
		CString str_mCurrTime = mCurrTime.Format("%Y%m%d %X");
		int len = WideCharToMultiByte(CP_ACP, 0, str_mCurrTime, str_mCurrTime.GetLength(), NULL, 0, NULL, NULL);
		char* c_str_mCurrTime = new char[len + 1];
		WideCharToMultiByte(CP_ACP, 0, str_mCurrTime, str_mCurrTime.GetLength(), c_str_mCurrTime, len, NULL, NULL);
		c_str_mCurrTime[len] = '\0';
		strcpy(pOpenOrder->SubmitDateAndTime, c_str_mCurrTime);
		strcpy(pOpenOrder->OpenTime, c_str_mCurrTime);
		delete c_str_mCurrTime;

		OpenOrderList.push_back((*pOpenOrder));

		AcquireSRWLockExclusive(&g_srwLockReqId);
		ReqIdToShmIndex.insert(std::pair<int, int>(iRetReqID, shmindex));
		ReleaseSRWLockExclusive(&g_srwLockReqId);

		ReleaseSemaphore(OrderInsertSem, 1, NULL);
	}
}

void StrategyGridMAStopGTCChop::ReqCloseOrderInsert(MyCloseOrderType* pCloseOrder, char OpenOrderTime[21]) {
	InsertOrderField cOrder;
	strcpy(cOrder.ClientNo, "");
	strcpy(cOrder.CommodityNo, m_CommodityNo);
	strcpy(cOrder.InstrumentID, m_InstID);
	cOrder.Direction = pCloseOrder->Direction;
	cOrder.Offset = pCloseOrder->Offset;
	cOrder.OrderPrice = floor((pCloseOrder->LimitPrice + 0.00001) / m_dOneTick) * m_dOneTick;
	cOrder.OrderVol = pCloseOrder->VolumeTotal;
	//cOrder.OrderLocalRef=pCloseOrder->OrderLocalRef;
	if (ThostTraderAPI || SgitTraderAPI) {
		CTime mCurrTime = CTime::GetCurrentTime();
		CString str_mCurrDate = mCurrTime.Format("%Y%m%d");
		int lenDate = WideCharToMultiByte(CP_ACP, 0, str_mCurrDate, str_mCurrDate.GetLength(), NULL, 0, NULL, NULL);
		char* c_str_mCurrDate = new char[lenDate + 1];
		WideCharToMultiByte(CP_ACP, 0, str_mCurrDate, str_mCurrDate.GetLength(), c_str_mCurrDate, lenDate, NULL, NULL);
		c_str_mCurrDate[lenDate] = '\0';

		string strOpenOrderTime(OpenOrderTime);
		string strOpenDataDate = strOpenOrderTime.substr(0, strOpenOrderTime.find_first_of(" "));
		bool CloseToday = false;
		// 对比开仓日期和交易日
		if (0 == strcmp(strOpenDataDate.c_str(), CTPTradingDay)) CloseToday = true;
		//if(strcmp(strOpenDataDate.c_str(),c_str_mCurrDate)==0){
		//	CloseToday=true;
		//}
		free(c_str_mCurrDate);

		pCloseOrder->OrderId = -1;
		if (ThostTraderAPI)pThostTraderSpi->ReqOrderInsert(&cOrder, m_bCrossTradingDay, CloseToday, shmindex);
		else pSgitTraderSpi->ReqOrderInsert(&cOrder, m_bCrossTradingDay, CloseToday, shmindex);
		pCloseOrder->OrderLocalRef = cOrder.OrderLocalRef;
		pCloseOrder->FrontID = cOrder.FrontID;
		pCloseOrder->SessionID = cOrder.SessionID;

		CString str_mCurrTime = mCurrTime.Format("%Y%m%d %X");
		int len = WideCharToMultiByte(CP_ACP, 0, str_mCurrTime, str_mCurrTime.GetLength(), NULL, 0, NULL, NULL);
		char* c_str_mCurrTime = new char[len + 1];
		WideCharToMultiByte(CP_ACP, 0, str_mCurrTime, str_mCurrTime.GetLength(), c_str_mCurrTime, len, NULL, NULL);
		c_str_mCurrTime[len] = '\0';
		strcpy(pCloseOrder->SubmitDateAndTime, c_str_mCurrTime);
		delete c_str_mCurrTime;

		CloseOrderList.push_back((*pCloseOrder));
	}
	else {
		int iRetReqID;
		pEsunTraderSpi->ReqOrderInsert(&cOrder, &iRetReqID, m_bCrossTradingDay);
		pCloseOrder->OrderLocalRef = -1;
		pCloseOrder->OrderLocalRetReqID = iRetReqID;
		pCloseOrder->FrontID = cOrder.FrontID;
		pCloseOrder->SessionID = cOrder.SessionID;

		CTime mCurrTime = CTime::GetCurrentTime();
		CString str_mCurrTime = mCurrTime.Format("%Y%m%d %X");
		int len = WideCharToMultiByte(CP_ACP, 0, str_mCurrTime, str_mCurrTime.GetLength(), NULL, 0, NULL, NULL);
		char* c_str_mCurrTime = new char[len + 1];
		WideCharToMultiByte(CP_ACP, 0, str_mCurrTime, str_mCurrTime.GetLength(), c_str_mCurrTime, len, NULL, NULL);
		c_str_mCurrTime[len] = '\0';
		strcpy(pCloseOrder->OpenTime, c_str_mCurrTime);
		strcpy(pCloseOrder->SubmitDateAndTime, c_str_mCurrTime);
		delete c_str_mCurrTime;

		CloseOrderList.push_back((*pCloseOrder));

		AcquireSRWLockExclusive(&g_srwLockReqId);
		ReqIdToShmIndex.insert(std::pair<int, int>(iRetReqID, shmindex));
		ReleaseSRWLockExclusive(&g_srwLockReqId);

		ReleaseSemaphore(OrderInsertSem, 1, NULL);
	}
}

void StrategyGridMAStopGTCChop::ReqOrderDelete(int pOrderId, int pOrderLocalRef, int pFrontID, int pSessionID)
{
	if (ThostTraderAPI || SgitTraderAPI) {
		if (ThostTraderAPI)pThostTraderSpi->ReqOrderDeletePerOrderLocalRef(pOrderLocalRef, m_InstID, m_ExchangeID, pFrontID, pSessionID);
		else pSgitTraderSpi->ReqOrderDeletePerOrderLocalRef(pOrderLocalRef, m_InstID, pFrontID, pSessionID);
	}
	else {
		pEsunTraderSpi->ReqOrderDelete(pOrderId);
	}

	char line[200];
	sprintf(line, "%s,ReqOrderDelete,%d", mStrategyAndInstance.c_str(), pOrderId);
	WriteMsgToLogList(line);
}

void StrategyGridMAStopGTCChop::WriteMsgToLogList(char logline[200])
{
	Message logMsg;
	logMsg.type = STRATEGY_LOG;
	logMsg.AddData(logline, 0, sizeof(char) * 200);
	LogMessageList.AddTail(logMsg);
	ReleaseSemaphore(logSemaphore, 1, NULL);
}

void StrategyGridMAStopGTCChop::OnDisplayLocalCloseOrderList()
{
	char log[200];
	sprintf(log, "%s,收盘价=%.4f,基准价=%.4f,收盘时间:%s", mStrategyAndInstance.c_str(), mClosePrice, mStrategyParams.BasePrice,
		mStrategyParams.CloseTime);
	CString str(log);
	pPubMsg->AddString(str);

	std::list<MyOpenOrderType>::iterator openorder_it;
	for (openorder_it = OpenOrderList.begin(); openorder_it != OpenOrderList.end(); openorder_it++) {
		LocalCLForDisplayField lcposition;
		strcpy(lcposition.StrategyID, mStrategyID);
		strcpy(lcposition.InstanceName, mInstanceName);
		strcpy(lcposition.InstCodeName, InstCodeName);

		lcposition.OrderID = openorder_it->OrderId;
		lcposition.CloseOrderSeqNo = 0;
		lcposition.Direction = openorder_it->Direction;

		strcpy(lcposition.OpenTime, openorder_it->OpenTime);
		lcposition.LimitPrice = openorder_it->LimitPrice;
		lcposition.OpenOrderTradePrice = openorder_it->LimitPrice;
		lcposition.VolumeTotal = openorder_it->VolumeTotal;
		lcposition.OffSet = MORDER_OPEN;
		lcposition.ManualStopPrice = 0.0;
		lcposition.MOrderType = openorder_it->MOrderType;

		Message posiMsg;
		posiMsg.type = ON_RSP_LOCAL_CL;
		posiMsg.AddData(&lcposition, 0, sizeof(LocalCLForDisplayField));
		ScreenDisplayMsgList.AddTail(posiMsg);

		ReleaseSemaphore(ScreenDisplaySem, 1, NULL);
	}

	std::list<MyCloseOrderType>::iterator closeorder_it;
	for (closeorder_it = CloseOrderList.begin(); closeorder_it != CloseOrderList.end(); closeorder_it++) {
		if (closeorder_it->IsClosePofitOrder) {
			LocalCLForDisplayField lcposition;
			strcpy(lcposition.StrategyID, mStrategyID);
			strcpy(lcposition.InstanceName, mInstanceName);
			strcpy(lcposition.InstCodeName, InstCodeName);

			lcposition.OrderID = -1;
			lcposition.CloseOrderSeqNo = closeorder_it->CloseOrderSeqNo;
			lcposition.Direction = closeorder_it->Direction;

			strcpy(lcposition.OpenTime, closeorder_it->OpenTime);
			if (closeorder_it->Direction == MORDER_BUY) {
				lcposition.LimitPrice = closeorder_it->OpenOrderTradePrice + closeorder_it->mStoplossPoint * m_dOneTick;
				lcposition.OpenOrderTradePrice = closeorder_it->OpenOrderTradePrice;
			}
			else if (closeorder_it->Direction == MORDER_SELL) {
				lcposition.LimitPrice = closeorder_it->OpenOrderTradePrice - closeorder_it->mStoplossPoint * m_dOneTick;
				lcposition.OpenOrderTradePrice = closeorder_it->OpenOrderTradePrice;
			}

			lcposition.VolumeTotal = closeorder_it->VolumeTotal;
			lcposition.OffSet = MORDER_STOPLOSSCLOSE;
			lcposition.ManualStopPrice = closeorder_it->ManualStopPrice;
			lcposition.MOrderType = closeorder_it->MOrderType;

			Message posiMsg;
			posiMsg.type = ON_RSP_LOCAL_CL;
			posiMsg.AddData(&lcposition, 0, sizeof(LocalCLForDisplayField));
			ScreenDisplayMsgList.AddTail(posiMsg);

			ReleaseSemaphore(ScreenDisplaySem, 1, NULL);
		}
		LocalCLForDisplayField lcposition;
		strcpy(lcposition.StrategyID, mStrategyID);
		strcpy(lcposition.InstanceName, mInstanceName);
		strcpy(lcposition.InstCodeName, InstCodeName);

		lcposition.OrderID = closeorder_it->OrderId;
		lcposition.CloseOrderSeqNo = closeorder_it->CloseOrderSeqNo;
		lcposition.Direction = closeorder_it->Direction;

		strcpy(lcposition.OpenTime, closeorder_it->OpenTime);
		lcposition.LimitPrice = closeorder_it->LimitPrice;
		lcposition.OpenOrderTradePrice = closeorder_it->OpenOrderTradePrice;
		lcposition.VolumeTotal = closeorder_it->VolumeTotal;
		lcposition.OffSet = MORDER_CLOSE;
		lcposition.ManualStopPrice = 0.0;
		lcposition.MOrderType = closeorder_it->MOrderType;

		Message posiMsg;
		posiMsg.type = ON_RSP_LOCAL_CL;
		posiMsg.AddData(&lcposition, 0, sizeof(LocalCLForDisplayField));
		ScreenDisplayMsgList.AddTail(posiMsg);

		ReleaseSemaphore(ScreenDisplaySem, 1, NULL);
	}
}

void StrategyGridMAStopGTCChop::AddtoTipMsgListBox(char msgline[200])
{
	WaitForSingleObject(MainScreenFreshSem, INFINITE);
	CString str(msgline);
	pPubMsg->AddString(str);
	ReleaseSemaphore(MainScreenFreshSem, 1, NULL);
}

void StrategyGridMAStopGTCChop::DisplayTradeOnScreen(OrderTradeMsg* pOrderTradeMsg, int mDirection, int mOpenOrClose, int mCloseProfitOrNot)
{
	WaitForSingleObject(MainScreenFreshSem, INFINITE);
	int orderIndex = pTradesDetailsList->GetItemCount();
	CString itemname("");
	itemname.Format(_T("%d"), orderIndex);
	pTradesDetailsList->InsertItem(orderIndex, (LPCTSTR)itemname);

	//CString csStrategyName(mStrategyName);
	CString csStrategyID(mStrategyID);
	CString csInstanceName(mInstanceName);

	pTradesDetailsList->SetItemText(orderIndex, 0, (LPCTSTR)csStrategyID);
	//pTradesDetailsList->SetItemText(orderIndex,0,(LPCTSTR)csStrategyName);
	pTradesDetailsList->SetItemText(orderIndex, 1, (LPCTSTR)csInstanceName);

	CString csInstCodeName(mStrategyParams.InstCodeName);
	pTradesDetailsList->SetItemText(orderIndex, 2, (LPCTSTR)csInstCodeName);

	CString csTradeTime(pOrderTradeMsg->InsertOrTradeTime);
	pTradesDetailsList->SetItemText(orderIndex, 3, (LPCTSTR)csTradeTime);

	CString direction("");
	if (mDirection == MORDER_SELL) {
		direction = _T("卖");
	}
	else if (mDirection == MORDER_BUY) {
		direction = _T("买");
	}
	pTradesDetailsList->SetItemText(orderIndex, 4, (LPCTSTR)direction);

	CString csTradePrice;
	csTradePrice.Format(_T("%.5f"), pOrderTradeMsg->Price);
	pTradesDetailsList->SetItemText(orderIndex, 5, (LPCTSTR)csTradePrice);

	CString vol;
	vol.Format(_T("%d"), pOrderTradeMsg->Volume);
	pTradesDetailsList->SetItemText(orderIndex, 6, (LPCTSTR)vol);

	CString csOpenOrClose("");
	if (mOpenOrClose == MORDER_OPEN) {
		csOpenOrClose = _T("开");
	}
	else if (mOpenOrClose == MORDER_CLOSE) {
		if (mCloseProfitOrNot == 1) {
			csOpenOrClose = _T("止盈平");
		}
		else csOpenOrClose = _T("止损平");
	}
	pTradesDetailsList->SetItemText(orderIndex, 7, (LPCTSTR)csOpenOrClose);

	pTradesDetailsList->EnsureVisible(pTradesDetailsList->GetItemCount() - 1, FALSE);
	ReleaseSemaphore(MainScreenFreshSem, 1, NULL);
}

void StrategyGridMAStopGTCChop::RecoverInstance(char cfgFileName[500])
{
	FILE* fptr = fopen(cfgFileName, "rb");
	fseek(fptr, 0, SEEK_SET);
	fread(&header, sizeof(mSerializeHeader), 1, fptr);
	int OpenOrderCount = header.OpenOrderCount;
	int CloseOrderCount = header.CloseOrderCount;
	memcpy(&mOpenTimes, header.SpecificArea, sizeof(int));
	memcpy(&m_bStoplossClose, header.SpecificArea + sizeof(int), sizeof(bool));
	memcpy(&mPrice, header.SpecificArea + sizeof(int) + sizeof(bool), sizeof(double));
	memcpy(&GridCloseOrderCount, header.SpecificArea + sizeof(int) + sizeof(bool) + sizeof(double), sizeof(GridCloseOrderCount));
	//	strcpy(header.StartTime,mStrategyParams.StartTime);
	//	strcpy(header.EndTime,mStrategyParams.EndTime);
	mStrategyParams.BasePrice = mPrice;

	MyOpenOrderType openOrder;
	for (int i = 0; i < OpenOrderCount; i++) {
		fread(&openOrder, sizeof(MyOpenOrderType), 1, fptr);
		OpenOrderList.push_back(openOrder);
		OrderIdToShmIndex.insert(std::pair<int, int>(openOrder.OrderId, GetShmindex()));
		string sfinal(mStrategyAndInstance);
		sfinal.append("_open");
		OrderIdToStrategyNameForDisplay.insert(std::pair<int, string>(openOrder.OrderId, sfinal));
	}
	MyCloseOrderType closeOrder;
	for (int i = 0; i < CloseOrderCount; i++) {
		fread(&closeOrder, sizeof(MyCloseOrderType), 1, fptr);
		CloseOrderList.push_back(closeOrder);
		OrderIdToShmIndex.insert(std::pair<int, int>(closeOrder.OrderId, GetShmindex()));
		string sfinal(mStrategyAndInstance);
		sfinal.append("_close");
		OrderIdToStrategyNameForDisplay.insert(std::pair<int, string>(closeOrder.OrderId, sfinal));
	}

	fclose(fptr);

	CTime mCurrTime = CTime::GetCurrentTime();

	CString str_mCurrTime = mCurrTime.Format("%X");
	int len = WideCharToMultiByte(CP_ACP, 0, str_mCurrTime, str_mCurrTime.GetLength(), NULL, 0, NULL, NULL);
	char* c_str_mCurrTime = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, str_mCurrTime, str_mCurrTime.GetLength(), c_str_mCurrTime, len, NULL, NULL);
	c_str_mCurrTime[len] = '\0';

	char log[300];
	sprintf(log, "%s,StrategyGridOpen,重启恢复,当前时间=%s", mStrategyAndInstance.c_str(), c_str_mCurrTime);
	CString str(log);
	pPubMsg->AddString(str);
	WriteMsgToLogList(log);
	free(c_str_mCurrTime);

	FlushStrategyInfoToFile();
}

void StrategyGridMAStopGTCChop::FlushStrategyInfoToFile() {
	header.OpenOrderCount = OpenOrderList.size();
	header.CloseOrderCount = CloseOrderList.size();
	CTime mCurrTime = CTime::GetCurrentTime();
	CString str_mCurrTime = mCurrTime.Format("%Y-%m-%d %X");
	int len = WideCharToMultiByte(CP_ACP, 0, str_mCurrTime, str_mCurrTime.GetLength(), NULL, 0, NULL, NULL);
	char* c_str_mCurrTime = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, str_mCurrTime, str_mCurrTime.GetLength(), c_str_mCurrTime, len, NULL, NULL);
	c_str_mCurrTime[len] = '\0';
	strcpy(header.StopTime, c_str_mCurrTime);
	free(c_str_mCurrTime);
	memcpy(header.SpecificArea, &mOpenTimes, sizeof(int));
	memcpy(header.SpecificArea + sizeof(int), &m_bStoplossClose, sizeof(bool));
	memcpy(header.SpecificArea + sizeof(int) + sizeof(bool), &mStrategyParams.BasePrice, sizeof(double));
	memcpy(header.SpecificArea + sizeof(int) + sizeof(bool) + sizeof(double), &GridCloseOrderCount, sizeof(GridCloseOrderCount));

	memcpy(StrategyfilePoint, &header, sizeof(mSerializeHeader));

	memset(StrategyfilePoint + sizeof(mSerializeHeader), 0, (sizeof(MyCloseOrderType) + sizeof(MyOpenOrderType)) * 100);

	int OpenOrderIndex = 0;
	if (!OpenOrderList.empty()) {
		std::list<MyOpenOrderType>::iterator openorder_ittmp;
		for (openorder_ittmp = OpenOrderList.begin(); openorder_ittmp != OpenOrderList.end(); openorder_ittmp++) {
			memcpy(StrategyfilePoint + sizeof(mSerializeHeader) + OpenOrderIndex * sizeof(MyOpenOrderType), &(*openorder_ittmp), sizeof(MyOpenOrderType));
			OpenOrderIndex++;
		}
	}
	int CloseOrderIndex = 0;
	if (!CloseOrderList.empty()) {
		std::list<MyCloseOrderType>::iterator closeorder_ittmp;
		for (closeorder_ittmp = CloseOrderList.begin(); closeorder_ittmp != CloseOrderList.end(); closeorder_ittmp++) {
			memcpy(StrategyfilePoint + sizeof(mSerializeHeader) + OpenOrderIndex * sizeof(MyOpenOrderType) + CloseOrderIndex * sizeof(MyCloseOrderType), &(*closeorder_ittmp), sizeof(MyCloseOrderType));
			CloseOrderIndex++;
		}
	}
	FlushViewOfFile(StrategyfilePoint, sizeof(mSerializeHeader) + (sizeof(MyCloseOrderType) + sizeof(MyOpenOrderType)) * 100);
}

wstring StrategyGridMAStopGTCChop::s2ws(const string& s)
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

int StrategyGridMAStopGTCChop::CreateStrategyMapOfView() {
	CString strPathFile;
	::GetModuleFileName(NULL, strPathFile.GetBuffer(MAX_PATH), MAX_PATH);
	strPathFile.ReleaseBuffer();
	strPathFile = strPathFile.Left(strPathFile.ReverseFind(_T('\\')));
	strPathFile += "\\Strategies\\TTimeOpenModel\\";
	char filename[500];
	ConvertCStringToCharArray(strPathFile, filename);
	string strFileName(filename);
	//strFileName.append(mStrategyName);
	strFileName.append(mStrategyID);
	strFileName.append("\\");
	strFileName.append(mInstanceName);
	strFileName.append(".cfg");
	wstring widstr2;
	widstr2 = s2ws(strFileName);

	//fpinfo=fopen(strFileName.c_str(),"w+");

	// LPCWSTR MT4filename=L"E:\\Program Files\\x.txt";

	// 步骤1 打开文件FILE_FLAG_WRITE_THROUGH
	HANDLE StrahFile = CreateFile(
		widstr2.c_str(),
		GENERIC_WRITE | GENERIC_READ,// 如果要映射文件：此处必设置为只读(GENERIC_READ)或读写
		FILE_SHARE_READ | FILE_SHARE_WRITE,//0,    // 此设为打开文件的任何尝试均将失败
		NULL,
		CREATE_ALWAYS,//OPEN_EXISTING,OPEN_ALWAYS,TRUNCATE_EXISTING,CREATE_ALWAYS
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, //|FILE_FLAG_WRITE_THROUGH,【解1】
		NULL);
	if (StrahFile == INVALID_HANDLE_VALUE)// 文件打开失败返回句柄为-1
		// 这步必须测试，详细见步骤2
	{
		return (-1);
	}

	// 步骤2 建立内存映射文件
	//DWORD dwFileSize = GetFileSize(hFile, NULL);

	header.OpenOrderCount = 0; header.CloseOrderCount = 0;
	strcpy(header.CodeName, InstCodeName);
	strcpy(header.StartTime, mStrategyParams.StartTime);
	strcpy(header.EndTime, mStrategyParams.EndTime);

	CTime mCurrTime = CTime::GetCurrentTime();
	CString str_mCurrTime = mCurrTime.Format("%Y-%m-%d %X");
	int len = WideCharToMultiByte(CP_ACP, 0, str_mCurrTime, str_mCurrTime.GetLength(), NULL, 0, NULL, NULL);
	char* c_str_mCurrTime = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, str_mCurrTime, str_mCurrTime.GetLength(), c_str_mCurrTime, len, NULL, NULL);
	c_str_mCurrTime[len] = '\0';
	strcpy(header.StopTime, c_str_mCurrTime);
	free(c_str_mCurrTime);

	int xmShmindex = GetShmindex();
	CString csxmShmindex("");
	csxmShmindex.Format(_T("%d"), xmShmindex);

	CString FileMappingName(mStrategyAndInstance.c_str());
	StrategyInfoFileMap = CreateFileMapping(
		StrahFile, // 如果这值为INVALID_HANDLE_VALUE,是合法的，上步一定测试啊
		NULL,   // 默认安全性
		PAGE_READWRITE,   // 可读写
		0, // 2个32位数示1个64位数，最大文件字节数，
		// 高字节，文件大小小于4G时，高字节永远为0
		sizeof(mSerializeHeader) + (sizeof(MyCloseOrderType) + sizeof(MyOpenOrderType)) * 100,//dwFileSize, // 此为低字节，也就是最主要的参数，如果为0，取文件真实大小
		_T("XX" + FileMappingName + "Map" + csxmShmindex));
	if (StrategyInfoFileMap == NULL)
	{
		return (-1);
	}

	CloseHandle(StrahFile);    // 关闭文件

	// 步骤3：将文件数据映射到进程的地址空间
	StrategyfilePoint = (char*)MapViewOfFile( //filePoint就是得到的指针，用它来直接操作文件
		StrategyInfoFileMap,
		FILE_MAP_WRITE,    // 可写
		0,     // 文件指针头位置 高字节
		0, // 文件指针头位置 低字节 必为分配粒度的整倍数,windows的粒度为64K
		0);   // 要映射的文件尾，如果为0，则从指针头到真实文件尾
	if (StrategyfilePoint == NULL)
	{
		return (-1);
	}

	// 步骤4: 像操作内存一样操作文件
	memcpy(StrategyfilePoint, &header, sizeof(mSerializeHeader));
	FlushViewOfFile(StrategyfilePoint, sizeof(mSerializeHeader));

	return 0;
}

int StrategyGridMAStopGTCChop::OpenStrategyMapOfView() {
	CString strPathFile;
	::GetModuleFileName(NULL, strPathFile.GetBuffer(MAX_PATH), MAX_PATH);
	strPathFile.ReleaseBuffer();
	strPathFile = strPathFile.Left(strPathFile.ReverseFind(_T('\\')));
	strPathFile += "\\Strategies\\TTimeOpenModel\\";
	char filename[500];
	ConvertCStringToCharArray(strPathFile, filename);
	string strFileName(filename);
	//strFileName.append(mStrategyName);
	strFileName.append(mStrategyID);
	strFileName.append("\\");
	strFileName.append(mInstanceName);
	strFileName.append(".cfg");
	wstring widstr2;
	widstr2 = s2ws(strFileName);

	//fpinfo=fopen(strFileName.c_str(),"w+");

	// LPCWSTR MT4filename=L"E:\\Program Files\\x.txt";

	// 步骤1 打开文件FILE_FLAG_WRITE_THROUGH
	HANDLE StrahFile = CreateFile(
		widstr2.c_str(),
		GENERIC_WRITE | GENERIC_READ,// 如果要映射文件：此处必设置为只读(GENERIC_READ)或读写
		FILE_SHARE_READ | FILE_SHARE_WRITE,//0,    // 此设为打开文件的任何尝试均将失败
		NULL,
		OPEN_EXISTING,//OPEN_EXISTING,OPEN_ALWAYS,TRUNCATE_EXISTING,CREATE_ALWAYS
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, //|FILE_FLAG_WRITE_THROUGH,【解1】
		NULL);
	if (StrahFile == INVALID_HANDLE_VALUE)// 文件打开失败返回句柄为-1
		// 这步必须测试，详细见步骤2
	{
		return (-1);
	}

	// 步骤2 建立内存映射文件
	//DWORD dwFileSize = GetFileSize(hFile, NULL);
	int xmShmindex = GetShmindex();
	CString csxmShmindex("");
	csxmShmindex.Format(_T("%d"), xmShmindex);

	CString FileMappingName(mStrategyAndInstance.c_str());
	StrategyInfoFileMap = CreateFileMapping(
		StrahFile, // 如果这值为INVALID_HANDLE_VALUE,是合法的，上步一定测试啊
		NULL,   // 默认安全性
		PAGE_READWRITE,   // 可读写
		0, // 2个32位数示1个64位数，最大文件字节数，
		// 高字节，文件大小小于4G时，高字节永远为0
		sizeof(mSerializeHeader) + (sizeof(MyCloseOrderType) + sizeof(MyOpenOrderType)) * 100,//dwFileSize, // 此为低字节，也就是最主要的参数，如果为0，取文件真实大小
		_T("XX" + FileMappingName + "Map" + csxmShmindex));
	if (StrategyInfoFileMap == NULL)
	{
		return (-1);
	}

	CloseHandle(StrahFile);    // 关闭文件

	// 步骤3：将文件数据映射到进程的地址空间
	StrategyfilePoint = (char*)MapViewOfFile( //filePoint就是得到的指针，用它来直接操作文件
		StrategyInfoFileMap,
		FILE_MAP_WRITE,    // 可写
		0,     // 文件指针头位置 高字节
		0, // 文件指针头位置 低字节 必为分配粒度的整倍数,windows的粒度为64K
		0);   // 要映射的文件尾，如果为0，则从指针头到真实文件尾
	if (StrategyfilePoint == NULL)
	{
		return (-1);
	}

	return 0;
}

void StrategyGridMAStopGTCChop::CloseMap()
{
	//删除对应的cfg文件
	UnmapViewOfFile(StrategyfilePoint);
	CloseHandle(StrategyInfoFileMap);

	if (ClearInstanceCfgFile) {
		CString strategyName(mStrategyName);
		char cInstanceName[50];
		GetInstanceName(cInstanceName);
		CString instranceName(cInstanceName);
		CString strPathFile;
		::GetModuleFileName(NULL, strPathFile.GetBuffer(MAX_PATH), MAX_PATH);
		strPathFile.ReleaseBuffer();
		strPathFile = strPathFile.Left(strPathFile.ReverseFind(_T('\\')));
		strPathFile += "\\Strategies";

		std::list<ModelNode>::iterator model_itr;
		for (model_itr = ModelList.begin(); model_itr != ModelList.end(); ++model_itr) {
			CString ModelName(model_itr->ModelName);
			CString csModel1Path;
			csModel1Path.Append(strPathFile);
			csModel1Path += "\\";
			csModel1Path.Append(ModelName);
			std::list<StrategyNode>::iterator strategy_itr;
			for (strategy_itr = model_itr->StrategyList.begin(); strategy_itr != model_itr->StrategyList.end(); ++strategy_itr) {
				CString csStrategyName(strategy_itr->StrategyName);
				if (csStrategyName.CompareNoCase(strategyName) == 0) {
					CString csStrategy1Path;
					csStrategy1Path.Append(csModel1Path);
					csStrategy1Path += "\\";
					csStrategy1Path.Append(csStrategyName);
					std::list<StrategyInstanceNode>::iterator instance_itr;
					for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
						CString csInstanceName(instance_itr->InstanceName);
						if (csInstanceName.CompareNoCase(instranceName) == 0) {
							CString csInstanceCfg;
							csInstanceCfg.Append(csStrategy1Path);
							csInstanceCfg += "\\";
							csInstanceCfg.Append(csInstanceName);
							csInstanceCfg += ".cfg";
							DeleteFile((LPCWSTR)csInstanceCfg);
						}
					}
				}
			}
		}
	}
}

void StrategyGridMAStopGTCChop::SetCloseLocalOrder(LocalCLForDisplayField* pCLOrder)
{
}

void StrategyGridMAStopGTCChop::SetStrategyID(char strategyId[50])
{
	strcpy(mStrategyID, strategyId);
}

void StrategyGridMAStopGTCChop::InitAction() {
	mTickCount = 0;
	m_bSampleReady = false;
	mOpenRet = false;
	mRet = false;
	mOrderRet = false;
}

void StrategyGridMAStopGTCChop::WriteBasePrice(char* date, char* time) {
	char line[200];
	sprintf(line, "%s,WriteBasePrice", mStrategyAndInstance.c_str());
	WriteMsgToLogList(line);

	char filePath[256] = { 0 };
	globalFuncUtil.getModuleFilePath(filePath);
	strcat(filePath, "\\Profit\\");

	strcat(filePath, mStrategyID);
	strcat(filePath, "_");
	strcat(filePath, mInstanceName);
	strcat(filePath, "_");
	strcat(filePath, date);
	strcat(filePath, ".posi");
	FILE* fout = fopen(filePath, "a");
	char mdline[200];
	memset(mdline, 0, sizeof(mdline));
	sprintf(mdline, "%s,%s,%s,%s,%s,%.2f\n", mStrategyID, mInstanceName, date, time, "BasePrice", mInitBasePrice);
	fwrite(mdline, strlen(mdline), 1, fout);
	memset(mdline, 0, sizeof(mdline));
	sprintf(mdline, "%s,%s,%s,%s,%s,%.2f\n", mStrategyID, mInstanceName, date, time, "CurBasePrice", mCurBasePrice);
	fwrite(mdline, strlen(mdline), 1, fout);

	map<int, BasePriceData> pMap;
	if (!CloseOrderList.empty()) {
		std::list<MyCloseOrderType>::iterator closeorder_it;
		for (closeorder_it = CloseOrderList.begin(); closeorder_it != CloseOrderList.end(); ++closeorder_it) {
			map<int, BasePriceData>::iterator pIntor = pMap.find(closeorder_it->MOrderType);
			if (pIntor == pMap.end()) {
				BasePriceData bData;
				memset(&bData, 0, sizeof(BasePriceData));
				strcpy_s(bData.StrategyID, mStrategyID);
				strcpy_s(bData.InstanceName, mInstanceName);
				strcpy_s(bData.DataDate, date);
				strcpy_s(bData.ItemData, "OpenSpread-Vol");
				bData.Price = closeorder_it->OpenOrderTradePrice * closeorder_it->VolumeTotal;
				bData.Vol = closeorder_it->VolumeTotal;
				bData.Dir = closeorder_it->Direction;
				pMap.insert(map<int, BasePriceData>::value_type(closeorder_it->MOrderType, bData));
			}
			else {
				pIntor->second.Price += closeorder_it->OpenOrderTradePrice * closeorder_it->VolumeTotal;
				pIntor->second.Vol += closeorder_it->VolumeTotal;
			}
		}
	}
	for (map<int, BasePriceData>::iterator itor = pMap.begin(); itor != pMap.end(); ++itor)
	{
		if (0 == itor->second.Dir)
			sprintf(mdline, "%s,%s,%s,%s,%s,%.2f_%d\n", mStrategyID, mInstanceName, date, time, "OpenSpread-Vol", itor->second.Price / itor->second.Vol, -itor->second.Vol);
		else
			sprintf(mdline, "%s,%s,%s,%s,%s,%.2f_%d\n", mStrategyID, mInstanceName, date, time, "OpenSpread-Vol", itor->second.Price / itor->second.Vol, itor->second.Vol);
		fwrite(mdline, strlen(mdline), 1, fout);
	}

	//if(!CloseOrderList.empty()){
	//	std::list<MyCloseOrderType>::iterator closeorder_it;
	//	for(closeorder_it = CloseOrderList.begin(); closeorder_it != CloseOrderList.end();++closeorder_it){
	//		sprintf(mdline,"%s,%s,%s,%s,%s,%.2f\n",mStrategyID,mInstanceName,date,time,"OpenSpread",closeorder_it->OpenOrderTradePrice);
	//		fwrite(mdline,strlen(mdline),1,fout);
	//	}
	//}

	pMap.clear();
	fclose(fout);
}