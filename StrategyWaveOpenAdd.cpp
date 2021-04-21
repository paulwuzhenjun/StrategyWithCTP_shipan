#include "StdAfx.h"
#include "StrategyWaveOpenAdd.h"
#include "Message.h"
#include "MessageList.h"
#include "TickDataList.h"
#include "OrderDataList.h"
#include "ado_conn.h"
#include "EsunTraderSpi.h"
#include <Afxtempl.h>
#include <iostream>
#include <time.h>
#include <tchar.h>
#include <math.h>
#include <map>
#include "ThostTraderSpi.h"
#include "SgitTraderSpi.h"
#include "LockVariable.h"

using namespace std;

int StrategyWaveOpenAdd::MaxOnHandPositionCount = 0;
int StrategyWaveOpenAdd::OnHandPositionCount = 0;

extern CListBox* pPubMsg;

extern EsunTraderSpi* pEsunTraderSpi;
extern ThostTraderSpi* pThostTraderSpi;
extern SgitTraderSpi* pSgitTraderSpi;

extern MessageList ScreenDisplayMsgList;
extern MessageList LogMessageList;
extern OrderDataList OrderList;

extern int iNextOrderRef;    //下一单引用

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
extern CListCtrl* pTradesDetailsList;
extern list<ModelNode> ModelList;
extern bool ClearInstanceCfgFile;
extern CLockVariable gLockVariable;
extern SRWLOCK g_srwLockOrderLocalRef;
extern map<int, int> OrderLocalRefToShmIndex;

StrategyWaveOpenAdd::StrategyWaveOpenAdd(char InstrumentID[30]) :m_Price(0)
{
	//	strcpy(m_InstID,InstrumentID);

		//全局变量
	iNextOrderRef = 1;
	mTickCount = 0;
	m_dOneTick = 0.01;
	m_bSampleReady = false;
	mOpenTimes = 0;

	m_bIsRunning = true;
	m_bCrossTradingDay = false;

	mPairID = 1;

	memset(&mStrategyParams, 0, sizeof(mParasType));
	memset(upMaxTime, 0, sizeof(upMaxTime));
	memset(upMinTime, 0, sizeof(upMinTime));
	memset(dnMaxTime, 0, sizeof(dnMaxTime));
	memset(dnMinTime, 0, sizeof(dnMinTime));
	mCloseOrderSeqNo = 0;
	InstanceOnHandPositionCount = 0;
	strcpy(mStrategyName, "WaveOpenAdd");
	strcpy(mInstanceName, "");
	WriteMsgToLogList("Strategy Init..");
}

StrategyWaveOpenAdd::~StrategyWaveOpenAdd(void)
{
	//fclose(foutbardata);
}

int StrategyWaveOpenAdd::GetShmindex()
{
	return shmindex;
}

void StrategyWaveOpenAdd::SetShmindex(int xshmindex)
{
	shmindex = xshmindex;
}

void StrategyWaveOpenAdd::SetInstanceName(char xInstanceName[50])
{
	strcpy(mInstanceName, xInstanceName);
	mStrategyAndInstance = mStrategyID;
	mStrategyAndInstance.append("_");
	mStrategyAndInstance.append(mInstanceName);
}

void StrategyWaveOpenAdd::GetInstanceName(char* xInstanceName) {
	strcpy(xInstanceName, mInstanceName);
}

void StrategyWaveOpenAdd::SetParamValue(ParamNode node) {
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
	}
	else if (strcmp(node.ParamName, "WaveLengthPoint") == 0) {
		mStrategyParams.WaveLengthPoint = atoi(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "OpenVol1") == 0) {
		mStrategyParams.OpenVol1 = atoi(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "OpenVol2") == 0) {
		mStrategyParams.OpenVol2 = atoi(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "WaveBackPoint") == 0) {
		mStrategyParams.WaveBackPoint = atoi(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "FirstProfitPoint") == 0) {
		mStrategyParams.FirstProfitPoint = atoi(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "FirstStoplossPoint") == 0) {
		mStrategyParams.FirstStoplossPoint = atoi(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "SecondProfitPoint") == 0) {
		mStrategyParams.SecondProfitPoint = atoi(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "SecondStoplossPoint") == 0) {
		mStrategyParams.SecondStoplossPoint = atoi(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "StartTime") == 0) {
		strcpy(mStrategyParams.StartTime, node.ParamValue);
	}
	else if (strcmp(node.ParamName, "EndTime") == 0) {
		strcpy(mStrategyParams.EndTime, node.ParamValue);
	}
	else if (strcmp(node.ParamName, "OpenBuyAllow") == 0) {
		if (atoi(node.ParamValue) >= 1)mStrategyParams.OpenBuyAllow = true;
		else mStrategyParams.OpenBuyAllow = false;
	}
	else if (strcmp(node.ParamName, "OpenSellAllow") == 0) {
		if (atoi(node.ParamValue) >= 1)mStrategyParams.OpenSellAllow = true;
		else mStrategyParams.OpenSellAllow = false;
	}
	else if (strcmp(node.ParamName, "LoopTimes") == 0) {
		mStrategyParams.LoopTimes = atoi(node.ParamValue);
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

void StrategyWaveOpenAdd::InitVariables() {
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
}
void StrategyWaveOpenAdd::ResetStrategy() {
}
//检查消息队列，并将得到的消息转给相应的处理函数
/*
void StrategyWaveOpenAdd::MessageProcess()
{
	TickInfo *pDepthMarketData=0;
	OrderTradeMsg *pOrderTrade=0;

	while(m_bIsRunning)
	{
		if(!TickList.DataListCore.IsEmpty())
		{
			pDepthMarketData =(TickInfo*)malloc(sizeof(TickInfo));
			TickInfo tick=TickList.GetHead();
			memcpy(pDepthMarketData,&tick,sizeof(TickInfo));
			//if(strstr(pDepthMarketData->ordername,m_InstID)!=NULL)
			OnRtnDepthMarketData(pDepthMarketData);
			delete pDepthMarketData;
			pDepthMarketData=0;
		}
		if(!OrderList.DataListCore.IsEmpty())
		{
			pOrderTrade =(OrderTradeMsg*)malloc(sizeof(OrderTradeMsg));
			OrderTradeMsg order=OrderList.GetHead();
			memcpy(pOrderTrade,&order,sizeof(OrderTradeMsg));
			if(pOrderTrade->OrderType==ON_RTN_ORDER)OnRtnOrder(pOrderTrade);
			else if(pOrderTrade->OrderType==ON_RTN_TRADE)OnRtnTrade(pOrderTrade);
			delete pOrderTrade;
			pOrderTrade=0;
		}

		//DeleteErrorOrderIdFromOrderList();
	}//end while
}
*/
void StrategyWaveOpenAdd::OnRtnDepthMarketData(TickInfo* pDepthMarketData)
{
	///读取行情价格，设置当前本地的当前价格
	m_Price = pDepthMarketData->price;
	m_Buy1 = pDepthMarketData->bid1;// m_Price;//
	m_Sell1 = pDepthMarketData->ask1;	//m_Buy1+1.0;//
	int EnterSellTrade = 0, EnterBuyTrade = 0;

	int nYear_m, nMonth_m, nDate_m;
	nYear_m = atoi(pDepthMarketData->datadate) / 10000;
	nMonth_m = (atoi(pDepthMarketData->datadate) % 10000) / 100;
	nDate_m = (atoi(pDepthMarketData->datadate) % 10000) % 100;
	int  nHour_m, nMin_m, nSec_m;
	sscanf(pDepthMarketData->updatetime, "%d:%d:%d", &nHour_m, &nMin_m, &nSec_m);

	CTime tm_m(nYear_m, nMonth_m, nDate_m, nHour_m, nMin_m, nSec_m);
	curTickTime = tm_m.GetTime();

	if (!m_bSampleReady) {
		if (nHour_m >= 7) {
			mOpenPrice = m_Price;
			m_bSampleReady = true;
			upWaveMax = mOpenPrice;
			upWaveMin = mOpenPrice;
			dnWaveMax = mOpenPrice;
			dnWaveMin = mOpenPrice;

			strcpy(upMaxTime, pDepthMarketData->updatetime);
			strcpy(upMinTime, pDepthMarketData->updatetime);
			strcpy(dnMaxTime, pDepthMarketData->updatetime);
			strcpy(dnMinTime, pDepthMarketData->updatetime);
		}
	}

	if (m_bSampleReady) {
		if (m_Price < upWaveMin) {
			upWaveMin = m_Price;
			upWaveMax = m_Price;
			strcpy(upMaxTime, pDepthMarketData->updatetime);
			strcpy(upMinTime, pDepthMarketData->updatetime);
		}
		else if (m_Price > upWaveMax) {
			upWaveMax = m_Price;
			strcpy(upMaxTime, pDepthMarketData->updatetime);
		}

		if (m_Price > dnWaveMax) {
			dnWaveMax = m_Price;
			dnWaveMin = m_Price;
			strcpy(dnMaxTime, pDepthMarketData->updatetime);
			strcpy(dnMinTime, pDepthMarketData->updatetime);
		}
		else if (m_Price < dnWaveMin) {
			dnWaveMin = m_Price;
			strcpy(dnMinTime, pDepthMarketData->updatetime);
		}
	}

	if (curTickTime > tmt_StartTime) {
		mTickCount++;
	}

	if (m_bSampleReady && (dnWaveMax - dnWaveMin + 0.00001) > mStrategyParams.WaveLengthPoint* m_dOneTick && (m_Price - dnWaveMin + 0.00001) > mStrategyParams.WaveBackPoint* m_dOneTick) {
		EnterSellTrade = 1;
		mOpenPrice = m_Price;
		char log[200];
		sprintf(log, "%s,时间=%s,%s,dnMax=%.4f,dnMin=%.4f,%s,%s", mStrategyAndInstance.c_str(), pDepthMarketData->datadate, pDepthMarketData->updatetime, dnWaveMax, dnWaveMin, dnMaxTime, dnMinTime);
		AddtoTipMsgListBox(log);
		WriteMsgToLogList(log);
		//重置波段
		dnWaveMax = m_Price;
		dnWaveMin = m_Price;
		strcpy(dnMaxTime, pDepthMarketData->updatetime);
		strcpy(dnMinTime, pDepthMarketData->updatetime);
	}
	else if (m_bSampleReady && (upWaveMax - upWaveMin + 0.00001) > mStrategyParams.WaveLengthPoint* m_dOneTick && (upWaveMax - m_Price + 0.00001) > mStrategyParams.WaveBackPoint* m_dOneTick) {
		EnterBuyTrade = 1;
		mOpenPrice = m_Price;
		char log[200];
		sprintf(log, "%s,时间=%s,%s,upMax=%.4f,upMin=%.4f,%s,%s", mStrategyAndInstance.c_str(), pDepthMarketData->datadate, pDepthMarketData->updatetime, upWaveMax, upWaveMin, upMaxTime, upMinTime);
		AddtoTipMsgListBox(log);
		WriteMsgToLogList(log);
		upWaveMax = m_Price;
		upWaveMin = m_Price;
		strcpy(upMaxTime, pDepthMarketData->updatetime);
		strcpy(upMinTime, pDepthMarketData->updatetime);
	}

	//Processing Opened Order
	std::list<MyOpenOrderType>::iterator openorder_it;
	if (!OpenOrderList.empty()) {
		for (openorder_it = OpenOrderList.begin(); openorder_it != OpenOrderList.end(); ++openorder_it) {
			if (openorder_it->VolumeTotal != 0 && openorder_it->OpenOrderCanbeCanceled) {
				if ((((openorder_it->LimitPrice - 0.00001 - m_dOneTick * 2.0) > m_Sell1&& openorder_it->Direction == MORDER_SELL)
					|| ((m_Buy1 - 0.00001 - m_dOneTick * 2.0) > openorder_it->LimitPrice&& openorder_it->Direction == MORDER_BUY)
					) && openorder_it->MOrderType == 1
					) {
					char line[200];
					sprintf(line, "%s,Cancel Open Order,OrderId=%d", mStrategyAndInstance.c_str(), openorder_it->OrderId);
					WriteMsgToLogList(line);

					ReqOrderDelete(openorder_it->OrderId, openorder_it->OrderLocalRef, openorder_it->FrontID, openorder_it->SessionID);
					openorder_it->OpenOrderCanbeCanceled = false;
				}
			}
		}//end for loop
	}
	//End for Opened Order processing

	std::list<MyCloseOrderType>::iterator closeorder_it;
	if (!CloseOrderList.empty()) {
		//Loop Close Order List
		for (closeorder_it = CloseOrderList.begin(); closeorder_it != CloseOrderList.end();) {
			std::list<MyCloseOrderType>::iterator iter_e = closeorder_it++;

			if (iter_e->Direction == MORDER_SELL &&
				(iter_e->OrderStatus == MORDER_PART_TRADED || iter_e->OrderStatus == MORDER_ACCEPTED || iter_e->OrderStatus == MORDER_QUEUED)) {
				//Closing the buy order
				iter_e->maxProfit = max(iter_e->maxProfit, m_Price - iter_e->OpenOrderTradePrice);

				//if((iter_e->maxProfit+0.00001)>mStrategyParams.MAStopStartPoint*m_dOneTick){
					//iter_e->mStoplossPoint=min(0,10-iter_e->maxProfit);
				//	iter_e->MAStop=true;
				//}

				if (iter_e->IsClosePofitOrder && (iter_e->OpenOrderTradePrice - m_Price) >= (iter_e->mStoplossPoint * m_dOneTick - 0.00001)
					&& iter_e->MOrderType == 1) {
					char line[200];
					sprintf(line, "%s,Type 1 Buy Stoploss Close,OrderId=%d", mStrategyAndInstance.c_str(), iter_e->OrderId);
					WriteMsgToLogList(line);

					iter_e->NextCloseOrderPrice = m_Sell1 - m_dOneTick;
					iter_e->CancelType = 1;
					ReqOrderDelete(iter_e->OrderId, iter_e->OrderLocalRef, iter_e->FrontID, iter_e->SessionID);
				}
				else if (iter_e->IsClosePofitOrder && (iter_e->OpenOrderTradePrice - m_Price) >= (mStrategyParams.SecondStoplossPoint * m_dOneTick - 0.00001)
					&& iter_e->MOrderType == 2) {
					char line[200];
					sprintf(line, "%s,Type 2 Buy Stoploss Close,OrderId=%d", mStrategyAndInstance.c_str(), iter_e->OrderId);
					WriteMsgToLogList(line);

					iter_e->NextCloseOrderPrice = m_Sell1 - m_dOneTick;
					iter_e->CancelType = 1;
					ReqOrderDelete(iter_e->OrderId, iter_e->OrderLocalRef, iter_e->FrontID, iter_e->SessionID);
				}
				else if (timeRuleForClose(pDepthMarketData->datadate, pDepthMarketData->updatetime)
					&& iter_e->IsClosePofitOrder) {
					char line[200];
					sprintf(line, "%s,Buy Order timeRuleForClose Close,OrderId=%d", mStrategyAndInstance.c_str(), iter_e->OrderId);
					WriteMsgToLogList(line);

					closeorder_it->NextCloseOrderPrice = m_Sell1 - m_dOneTick;
					iter_e->CancelType = 1;
					ReqOrderDelete(iter_e->OrderId, iter_e->OrderLocalRef, iter_e->FrontID, iter_e->SessionID);
				}
				else if ((iter_e->LimitPrice - 0.00001) > m_Sell1&& iter_e->IsStoplessOrder&& iter_e->CanbeCanceled) {
					char line[200];
					sprintf(line, "%s,Buy Order Close ReSubmit,OrderId=%d", mStrategyAndInstance.c_str(), iter_e->OrderId);
					WriteMsgToLogList(line);

					//TRACE("Cancel the stopless order for buy order,stopless price=%.1f,sell1=%.1f\n",closeorder_it->LimitPrice,m_Sell1);
					iter_e->NextCloseOrderPrice = m_Sell1;
					iter_e->CancelType = 1;
					ReqOrderDelete(iter_e->OrderId, iter_e->OrderLocalRef, iter_e->FrontID, iter_e->SessionID);
					iter_e->CanbeCanceled = false;
				}
			}//End IF Closing buy order

			if (iter_e->Direction == MORDER_BUY &&
				(iter_e->OrderStatus == MORDER_PART_TRADED || iter_e->OrderStatus == MORDER_ACCEPTED || iter_e->OrderStatus == MORDER_QUEUED)) {
				//onhandSellOrder++;
				//Closing sell order
				iter_e->maxProfit = max(iter_e->maxProfit, iter_e->OpenOrderTradePrice - m_Price);
				//if((iter_e->maxProfit+0.00001)>mStrategyParams.MAStopStartPoint*m_dOneTick){
					//iter_e->mStoplossPoint=min(0,10-iter_e->maxProfit);
				//	iter_e->MAStop=true;
				//}
				if (iter_e->IsClosePofitOrder && (m_Price - iter_e->OpenOrderTradePrice) >= (iter_e->mStoplossPoint * m_dOneTick - 0.00001)
					&& iter_e->MOrderType == 1) {
					char line[200];
					sprintf(line, "%s,Type 1 Sell Stoploss Close,OrderId=%d", mStrategyAndInstance.c_str(), iter_e->OrderId);
					WriteMsgToLogList(line);

					iter_e->NextCloseOrderPrice = m_Buy1 + m_dOneTick;
					iter_e->CancelType = 1;
					ReqOrderDelete(iter_e->OrderId, iter_e->OrderLocalRef, iter_e->FrontID, iter_e->SessionID);
				}
				else if (iter_e->IsClosePofitOrder && (m_Price - iter_e->OpenOrderTradePrice) >= (mStrategyParams.SecondStoplossPoint * m_dOneTick - 0.00001)
					&& iter_e->MOrderType == 2) {
					char line[200];
					sprintf(line, "%s,Type 2 Sell Stoploss Close,OrderId=%d", mStrategyAndInstance.c_str(), iter_e->OrderId);
					WriteMsgToLogList(line);

					iter_e->NextCloseOrderPrice = m_Buy1 + m_dOneTick;
					iter_e->CancelType = 1;
					ReqOrderDelete(iter_e->OrderId, iter_e->OrderLocalRef, iter_e->FrontID, iter_e->SessionID);
				}
				else if (timeRuleForClose(pDepthMarketData->datadate, pDepthMarketData->updatetime)
					&& iter_e->IsClosePofitOrder) {
					char line[200];
					sprintf(line, "%s,Sell Order timeRuleForClose Close,OrderId=%d", mStrategyAndInstance.c_str(), iter_e->OrderId);
					WriteMsgToLogList(line);

					closeorder_it->NextCloseOrderPrice = m_Buy1 + m_dOneTick;
					iter_e->CancelType = 1;
					ReqOrderDelete(closeorder_it->OrderId, closeorder_it->OrderLocalRef, closeorder_it->FrontID, closeorder_it->SessionID);
				}
				else if (iter_e->LimitPrice < (m_Buy1 - 0.00001) && iter_e->IsStoplessOrder && iter_e->CanbeCanceled) {
					char line[200];
					sprintf(line, "%s,Sell Order Close ReSubmit,OrderId=%d", mStrategyAndInstance.c_str(), iter_e->OrderId);
					WriteMsgToLogList(line);
					//TRACE("Cancel the stopless order for sell order,stopless price=%.1f,buy1=%.1f\n",closeorder_it->LimitPrice,m_Buy1);
					iter_e->NextCloseOrderPrice = m_Buy1;
					iter_e->CancelType = 1;
					ReqOrderDelete(iter_e->OrderId, iter_e->OrderLocalRef, iter_e->FrontID, iter_e->SessionID);
					iter_e->CanbeCanceled = false;
				}
			}//End IF Closing sell order
			//}//End for Strategy 1
		}//End Looping close order list
	}//End if the close order list is not null

	if (EnterBuyTrade == 1 && timeRuleForOpen(pDepthMarketData->datadate, pDepthMarketData->updatetime) && OpenOrderList.empty() && CloseOrderList.empty()
		&& InstanceOnHandPositionCount < mStrategyParams.LoopTimes
		&& mStrategyParams.OpenBuyAllow
		&& (OnHandPositionCount + mStrategyParams.OpenVol1) <= MaxOnHandPositionCount && (TotalOnHandPosition + mStrategyParams.OpenVol1) <= MaxTotalOnHandPosition) {
		//Open Buy Order
		MyOpenOrderType openThostOrder;
		iNextOrderRef++;
		openThostOrder.OrderLocalRetReqID = 0;
		openThostOrder.OrderId = -1;
		openThostOrder.LimitPrice = m_Price + m_dOneTick;
		openThostOrder.OrigSubmitPrice = openThostOrder.LimitPrice;
		openThostOrder.Direction = MORDER_BUY;
		openThostOrder.Offset = MORDER_OPEN;
		openThostOrder.VolumeTotal = mStrategyParams.OpenVol1;
		openThostOrder.VolumeTotalOriginal = openThostOrder.VolumeTotal;
		openThostOrder.VolumeTraded = 0;
		openThostOrder.ProfitPrice = openThostOrder.LimitPrice + mStrategyParams.FirstProfitPoint * m_dOneTick;
		openThostOrder.OpenOrderCanbeCanceled = true;
		openThostOrder.MOrderType = 1;
		openThostOrder.MAStop = false;
		openThostOrder.maxProfit = 0.0;

		//重置波段
		upWaveMax = m_Price;
		upWaveMin = m_Price;

		openThostOrder.mStoplossPoint = mStrategyParams.FirstStoplossPoint;
		ReqOpenOrderInsert(&openThostOrder);

		OnHandPositionCount += openThostOrder.VolumeTotal;
		TotalOnHandPosition += openThostOrder.VolumeTotal;
		InstanceOnHandPositionCount += openThostOrder.VolumeTotal;

		char log[200];
		sprintf(log, "%s,开仓,买=%.5f,时间=%s,%s", mStrategyAndInstance.c_str(), openThostOrder.LimitPrice, pDepthMarketData->datadate, pDepthMarketData->updatetime);
		AddtoTipMsgListBox(log);
		WriteMsgToLogList(log);
	}

	if (EnterSellTrade == 1 && timeRuleForOpen(pDepthMarketData->datadate, pDepthMarketData->updatetime) && OpenOrderList.empty() && CloseOrderList.empty()
		&& InstanceOnHandPositionCount < mStrategyParams.LoopTimes
		&& mStrategyParams.OpenSellAllow
		&& (OnHandPositionCount + mStrategyParams.OpenVol1) <= MaxOnHandPositionCount && (TotalOnHandPosition + mStrategyParams.OpenVol1) <= MaxTotalOnHandPosition) {
		//Open Sell Order
		MyOpenOrderType openThostOrder;
		iNextOrderRef++;
		openThostOrder.OrderLocalRetReqID = 0;
		openThostOrder.OrderId = -1;
		openThostOrder.LimitPrice = m_Price - m_dOneTick;
		openThostOrder.OrigSubmitPrice = openThostOrder.LimitPrice;
		openThostOrder.Direction = MORDER_SELL;
		openThostOrder.Offset = MORDER_OPEN;
		openThostOrder.VolumeTotal = mStrategyParams.OpenVol1;
		openThostOrder.VolumeTotalOriginal = openThostOrder.VolumeTotal;
		openThostOrder.VolumeTraded = 0;
		openThostOrder.ProfitPrice = openThostOrder.LimitPrice - mStrategyParams.FirstProfitPoint * m_dOneTick;
		openThostOrder.OpenOrderCanbeCanceled = true;
		openThostOrder.MOrderType = 1;
		openThostOrder.maxProfit = 0.0;
		openThostOrder.MAStop = false;

		//重置波段
		dnWaveMax = m_Price;
		dnWaveMin = m_Price;

		mOpenTimes++;
		openThostOrder.mStoplossPoint = mStrategyParams.FirstStoplossPoint;
		ReqOpenOrderInsert(&openThostOrder);

		OnHandPositionCount += openThostOrder.VolumeTotal;
		TotalOnHandPosition += openThostOrder.VolumeTotal;
		InstanceOnHandPositionCount += openThostOrder.VolumeTotal;

		char log[200];
		sprintf(log, "%s,开仓,卖=%.5f,时间=%s,%s", mStrategyAndInstance.c_str(), openThostOrder.LimitPrice, pDepthMarketData->datadate, pDepthMarketData->updatetime);
		AddtoTipMsgListBox(log);
		WriteMsgToLogList(log);
	}

	FlushStrategyInfoToFile();
}
/*
void StrategyWaveOpenAdd::DeleteErrorOrderIdFromOrderList(){
if(!ErrorOrderIdToReqId.empty()){
std::map<int,int>::iterator errormapiter;
for(errormapiter = ErrorOrderIdToReqId.begin(); errormapiter != ErrorOrderIdToReqId.end();){
int thisOrderId=errormapiter->first;
map<int,int>::iterator iter;
iter=OrderIdToReqId.find(thisOrderId);
if(iter!=OrderIdToReqId.end()){
int iReqID=iter->second;
map<int,int>::iterator iter2;
iter2=ReqIdToOrderLocalRef.find(iReqID);
if(iter2!=ReqIdToOrderLocalRef.end()){
int OrderLocalRef=iter2->second;
if(!OpenOrderList.empty()){
std::list<MyOpenOrderType>::iterator openorder_it;
for(openorder_it = OpenOrderList.begin(); openorder_it != OpenOrderList.end();){
std::list<MyOpenOrderType>::iterator iter_e=openorder_it++;
if(iter_e->OrderLocalRef==OrderLocalRef){
//iter_e->OrderId=thisOrderId;
ReqIdToOrderLocalRef.erase(iter2);
OrderIdToReqId.erase(iter);
OpenOrderList.erase(iter_e);
return ;
}
}
}

if(!CloseOrderList.empty()){
std::list<MyCloseOrderType>::iterator closeorder_it;
for(closeorder_it = CloseOrderList.begin(); closeorder_it != CloseOrderList.end();){
std::list<MyCloseOrderType>::iterator iter_ec=closeorder_it++;
if(iter_ec->OrderLocalRef==OrderLocalRef){
iter_ec->OrderId=thisOrderId;
ReqIdToOrderLocalRef.erase(iter2);
OrderIdToReqId.erase(iter);
CloseOrderList.erase(iter_ec);
return;
}
}
}
}
}
}
}
}
*/

void StrategyWaveOpenAdd::OnRtnOrder(OrderTradeMsg* pOrderTradeMsg)
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
				if (pOrderTradeMsg->OrderStatus == MORDER_PART_CANCELLED || pOrderTradeMsg->OrderStatus == MORDER_CANCELLED
					) {
					/*
					//开仓单被撤,重新报单
					MyOpenOrderType newOpenOrder;
					iNextOrderRef++;
					newOpenOrder.OrderLocalRetReqID=0;
					newOpenOrder.OrderId=-1;
					newOpenOrder.OrigSubmitPrice=iter_e->OrigSubmitPrice;
					if(iter_e->Direction==MORDER_BUY){
						newOpenOrder.LimitPrice=m_Sell1;
						newOpenOrder.ProfitPrice=newOpenOrder.LimitPrice+mStrategyParams.ProfitPoint;
					}else{
						newOpenOrder.LimitPrice=m_Buy1;
						newOpenOrder.ProfitPrice=newOpenOrder.LimitPrice-mStrategyParams.ProfitPoint;
					}
					newOpenOrder.Direction=iter_e->Direction;
					newOpenOrder.Offset=MORDER_OPEN;
					newOpenOrder.VolumeTotal=iter_e->VolumeTotal;
					newOpenOrder.VolumeTotalOriginal=newOpenOrder.VolumeTotal;
					newOpenOrder.VolumeTraded=0;
					newOpenOrder.OpenOrderCanbeCanceled=true;
					newOpenOrder.MOrderType=1;
					newOpenOrder.maxProfit=0.0;
					newOpenOrder.mStoplossPoint=mStrategyParams.StoplossPoint;
					ReqOpenOrderInsert(&newOpenOrder);
					*/
					//开仓单被撤,不重新报单
					OnHandPositionCount -= pOrderTradeMsg->VolumeTotal;
					TotalOnHandPosition -= pOrderTradeMsg->VolumeTotal;
					InstanceOnHandPositionCount -= pOrderTradeMsg->VolumeTotal;
					mOpenTimes--;

					OpenOrderList.erase(openorder_it);
					break;
				}
			} //End if openorder_it->OrderSysId==thisTradeRef
		}//End While
	}

	std::list<MyCloseOrderType>::iterator closeorder_it;
	if (!CloseOrderList.empty()) {
		for (closeorder_it = CloseOrderList.begin(); closeorder_it != CloseOrderList.end(); closeorder_it++) {
			//std::list<MyCloseOrderType>::iterator iter_ec=closeorder_it++;
			if (closeorder_it->OrderId == thisOrderId || (closeorder_it->OrderLocalRef == pOrderTradeMsg->OrderLocalRef && closeorder_it->OrderLocalRef > 0)) {
				//printf("CloseOrderList exist order \n");
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
				if ((pOrderTradeMsg->OrderStatus == MORDER_PART_CANCELLED || pOrderTradeMsg->OrderStatus == MORDER_CANCELLED)
					)
				{
					if (closeorder_it->CancelType == 1) {
						//类型1撤单，止损价触碰或者止损单未成交重新报单
						if (closeorder_it->IsClosePofitOrder) {
							//报到交易所的止盈单被撤
							if (closeorder_it->Direction == MORDER_SELL)
							{
								char line[200];
								sprintf(line, "%s,Buy Profit Cancel,OrderId=%d", mStrategyAndInstance.c_str(), closeorder_it->OrderId);
								WriteMsgToLogList(line);

								MyCloseOrderType newThostOrder;
								iNextOrderRef++;
								newThostOrder.OrderLocalRetReqID = 0;
								newThostOrder.OrderId = -1;
								newThostOrder.OrderLocalRef = -1;
								newThostOrder.OpenOrderID = closeorder_it->OpenOrderID;
								strcpy(newThostOrder.OpenTime, closeorder_it->OpenTime);
								newThostOrder.IsStoplessOrder = true;
								newThostOrder.CanbeCanceled = true;
								newThostOrder.IsClosePofitOrder = false;
								newThostOrder.OpenOrderTradePrice = closeorder_it->OpenOrderTradePrice;
								newThostOrder.MOrderType = closeorder_it->MOrderType;
								newThostOrder.Direction = closeorder_it->Direction;
								newThostOrder.LimitPrice = m_Buy1;//以最新价进行报止损单
								newThostOrder.OrigSubmitPrice = newThostOrder.LimitPrice;
								newThostOrder.Offset = closeorder_it->Offset;
								newThostOrder.VolumeTotalOriginal = closeorder_it->VolumeTotal;
								newThostOrder.VolumeTotal = newThostOrder.VolumeTotalOriginal;
								newThostOrder.VolumeTraded = 0;

								ReqCloseOrderInsert(&newThostOrder, newThostOrder.OpenTime);
							}
							else if (closeorder_it->Direction == MORDER_BUY)
							{
								char line[200];
								sprintf(line, "%s,Sell Profit Cancel,OrderId=%d", mStrategyAndInstance.c_str(), closeorder_it->OrderId);
								WriteMsgToLogList(line);

								MyCloseOrderType newThostOrder;
								iNextOrderRef++;
								newThostOrder.OrderLocalRetReqID = 0;
								newThostOrder.OrderId = -1;
								newThostOrder.OrderLocalRef = -1;
								newThostOrder.OpenOrderID = closeorder_it->OpenOrderID;
								strcpy(newThostOrder.OpenTime, closeorder_it->OpenTime);
								newThostOrder.IsStoplessOrder = true;
								newThostOrder.CanbeCanceled = true;
								newThostOrder.IsClosePofitOrder = false;
								newThostOrder.OpenOrderTradePrice = closeorder_it->OpenOrderTradePrice;
								newThostOrder.MOrderType = closeorder_it->MOrderType;
								newThostOrder.Direction = closeorder_it->Direction;
								newThostOrder.LimitPrice = m_Sell1;//以最新价进行报止损单
								newThostOrder.OrigSubmitPrice = newThostOrder.LimitPrice;
								newThostOrder.Offset = closeorder_it->Offset;
								newThostOrder.VolumeTotalOriginal = closeorder_it->VolumeTotal;
								newThostOrder.VolumeTotal = newThostOrder.VolumeTotalOriginal;
								newThostOrder.VolumeTraded = 0;

								ReqCloseOrderInsert(&newThostOrder, newThostOrder.OpenTime);
							}
							CloseOrderList.erase(closeorder_it);
							break;
						}
						else if (closeorder_it->IsStoplessOrder) {
							//止损单被撤(或报到交易所的止盈单被撤)
							if (closeorder_it->Direction == MORDER_SELL)
							{
								MyCloseOrderType newThostOrder;
								iNextOrderRef++;
								newThostOrder.OrderLocalRetReqID = 0;
								newThostOrder.OrderId = -1;
								newThostOrder.OrderLocalRef = -1;
								newThostOrder.OpenOrderID = closeorder_it->OpenOrderID;
								strcpy(newThostOrder.OpenTime, closeorder_it->OpenTime);
								newThostOrder.IsStoplessOrder = true;
								newThostOrder.MAStop = closeorder_it->MAStop;
								newThostOrder.CanbeCanceled = true;
								newThostOrder.IsClosePofitOrder = false;
								newThostOrder.ManualStopPrice = closeorder_it->ManualStopPrice;
								newThostOrder.OpenOrderTradePrice = closeorder_it->OpenOrderTradePrice;
								newThostOrder.MOrderType = closeorder_it->MOrderType;
								newThostOrder.Direction = closeorder_it->Direction;
								newThostOrder.LimitPrice = m_Buy1;//iter_ec->NextCloseOrderPrice;//以原先设定的价格进行报单
								newThostOrder.OrigSubmitPrice = closeorder_it->OrigSubmitPrice;
								newThostOrder.Offset = closeorder_it->Offset;
								newThostOrder.VolumeTotalOriginal = closeorder_it->VolumeTotal;
								newThostOrder.VolumeTotal = newThostOrder.VolumeTotalOriginal;
								newThostOrder.VolumeTraded = 0;

								ReqCloseOrderInsert(&newThostOrder, closeorder_it->OpenTime);
							}
							else if (closeorder_it->Direction == MORDER_BUY)
							{
								MyCloseOrderType newThostOrder;
								iNextOrderRef++;
								newThostOrder.OrderLocalRetReqID = 0;
								newThostOrder.OrderId = -1;
								newThostOrder.OrderLocalRef = -1;
								newThostOrder.OpenOrderID = closeorder_it->OpenOrderID;
								strcpy(newThostOrder.OpenTime, closeorder_it->OpenTime);
								newThostOrder.IsStoplessOrder = true;
								newThostOrder.MAStop = closeorder_it->MAStop;
								newThostOrder.CanbeCanceled = true;
								newThostOrder.IsClosePofitOrder = false;
								newThostOrder.ManualStopPrice = closeorder_it->ManualStopPrice;
								newThostOrder.OpenOrderTradePrice = closeorder_it->OpenOrderTradePrice;
								newThostOrder.MOrderType = closeorder_it->MOrderType;
								newThostOrder.Direction = closeorder_it->Direction;
								newThostOrder.LimitPrice = m_Sell1;//iter_ec->NextCloseOrderPrice;//以原先设定的价格进行报单
								newThostOrder.OrigSubmitPrice = closeorder_it->OrigSubmitPrice;
								newThostOrder.Offset = closeorder_it->Offset;
								newThostOrder.VolumeTotalOriginal = closeorder_it->VolumeTotal;
								newThostOrder.VolumeTotal = newThostOrder.VolumeTotalOriginal;
								newThostOrder.VolumeTraded = 0;

								ReqCloseOrderInsert(&newThostOrder, closeorder_it->OpenTime);
							}
						}
						CloseOrderList.erase(closeorder_it);
						break;
					}
					else if (closeorder_it->CancelType == 2 && closeorder_it->MOrderType == 1) {
						//类型1平仓单的撤单,由类型2开仓成功后撤单

						//加仓单开仓成交,撤掉类型1的止盈报单
						//looping the close order list to find the new profit price;
						double ProfitPrice = -1;
						double type2open = -1;
						int type2stoplosspoint = 0;
						for (std::list<MyCloseOrderType>::iterator co_it = CloseOrderList.begin(); co_it != CloseOrderList.end();) {
							std::list<MyCloseOrderType>::iterator iter_etmp = co_it++;
							if (iter_etmp->PairID == closeorder_it->PairID && iter_etmp->MOrderType == 2
								) {
								ProfitPrice = iter_etmp->ProfitPrice;
								type2open = iter_etmp->OpenOrderTradePrice;
								type2stoplosspoint = iter_etmp->mStoplossPoint;
								break;
							}
						}

						MyCloseOrderType newThostOrder;
						iNextOrderRef++;
						newThostOrder.OrderLocalRetReqID = 0;
						newThostOrder.OrderId = -1;
						newThostOrder.OrderLocalRef = -1;
						newThostOrder.CloseOrderSeqNo = ++mCloseOrderSeqNo;
						newThostOrder.OpenOrderID = closeorder_it->OpenOrderID;
						strcpy(newThostOrder.OpenTime, closeorder_it->OpenTime);
						newThostOrder.IsStoplessOrder = false;
						newThostOrder.CanbeCanceled = true;
						newThostOrder.IsClosePofitOrder = true;
						newThostOrder.OpenOrderTradePrice = closeorder_it->OpenOrderTradePrice;
						newThostOrder.MOrderType = closeorder_it->MOrderType;
						newThostOrder.Direction = closeorder_it->Direction;
						newThostOrder.LimitPrice = ProfitPrice;//以最新价进行报止损单
						newThostOrder.OrigSubmitPrice = newThostOrder.LimitPrice;
						newThostOrder.ProfitPrice = newThostOrder.LimitPrice;
						newThostOrder.mStoplossPoint = floor(fabs(closeorder_it->OpenOrderTradePrice - type2open) / m_dOneTick + 0.00001) + type2stoplosspoint;
						newThostOrder.ManualStopPrice = 0;
						newThostOrder.Offset = closeorder_it->Offset;
						newThostOrder.VolumeTotalOriginal = closeorder_it->VolumeTotal;
						newThostOrder.VolumeTotal = newThostOrder.VolumeTotalOriginal;
						newThostOrder.VolumeTraded = 0;

						ReqCloseOrderInsert(&newThostOrder, newThostOrder.OpenTime);

						CloseOrderList.erase(closeorder_it);
						break;
					}
				}//if it is cancelled order
			}//End if closeorder_it->OrderSysId==thisOrderRef for Strategy 2
		}//End While
	}

	FlushStrategyInfoToFile();
}

void StrategyWaveOpenAdd::OnRtnTrade(OrderTradeMsg* pOrderTradeMsg)
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

		double submitprice = 0;
		int tradedirection = -1;
		int openorclose = -1;
		std::list<MyOpenOrderType>::iterator openorder_it;
		if (!OpenOrderList.empty()) {
			for (openorder_it = OpenOrderList.begin(); openorder_it != OpenOrderList.end(); openorder_it++) {
				//std::list<MyOpenOrderType>::iterator iter_e=openorder_it++;
				if (openorder_it->OrderId == thisTradeRef || (openorder_it->OrderLocalRef == pOrderTradeMsg->OrderLocalRef && openorder_it->OrderLocalRef > 0)) {
					//printf("OpenOrderList exist order \n");
					if (openorder_it->OrderLocalRef == pOrderTradeMsg->OrderLocalRef && openorder_it->OrderLocalRef > 0)openorder_it->OrderId = thisTradeRef;
					openorclose = MORDER_OPEN;
					openorder_it->VolumeTraded = openorder_it->VolumeTraded + pOrderTradeMsg->Volume;
					openorder_it->VolumeTotal = openorder_it->VolumeTotal - pOrderTradeMsg->Volume;
					strcpy(openorder_it->OpenTime, pOrderTradeMsg->InsertOrTradeTime);

					if (openorder_it->Direction == MORDER_BUY && openorder_it->MOrderType == 1)
					{
						tradedirection = MORDER_BUY;
						MyCloseOrderType thostOrder;//平仓止盈单
						thostOrder.OpenOrderID = thisTradeRef;
						thostOrder.OpenOrderSubmitPrice = openorder_it->OrigSubmitPrice;
						thostOrder.OpenOrderTradePrice = pOrderTradeMsg->Price;
						thostOrder.IsClosePofitOrder = true;
						thostOrder.IsStoplessOrder = false;
						thostOrder.CanbeCanceled = false;
						thostOrder.dwCloseOrderStart = GetTickCount();
						thostOrder.MOrderType = openorder_it->MOrderType;
						thostOrder.MAStop = false;
						strcpy(thostOrder.OpenTime, pOrderTradeMsg->InsertOrTradeTime);
						iNextOrderRef++;
						thostOrder.OrderLocalRetReqID = 0;
						thostOrder.OrderId = -1;
						thostOrder.OrderLocalRef = -1;
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
						thostOrder.PairID = mPairID;
						//strcpy(thostOrder.OpenTime,gEqualVolRatesX[gEqualVolRatesXIndex].datatime);
						//CloseOrderList.push_back(thostOrder);
						ReqCloseOrderInsert(&thostOrder, openorder_it->OpenTime);

						submitprice = openorder_it->OrigSubmitPrice;

						//报第二个开仓单到交易所
						if (openorder_it->MOrderType == 1 && InstanceOnHandPositionCount < mStrategyParams.LoopTimes
							&& mOpenPrice>0.00001
							&& (OnHandPositionCount + mStrategyParams.OpenVol2) <= MaxOnHandPositionCount
							&& (TotalOnHandPosition + mStrategyParams.OpenVol2) <= MaxTotalOnHandPosition
							&& openorder_it->VolumeTotal == 0
							) {
							char line[200];
							sprintf(line, "%s,Open Type 2 Order,Buy,T1OrderId=%d", mStrategyAndInstance.c_str(), openorder_it->OrderId);
							WriteMsgToLogList(line);

							MyOpenOrderType openThostOrder;
							iNextOrderRef++;
							openThostOrder.OrderLocalRetReqID = 0;
							openThostOrder.OrderId = -1;
							openThostOrder.OrderLocalRef = -1;
							openThostOrder.LimitPrice = pOrderTradeMsg->Price - mStrategyParams.WaveBackPoint * m_dOneTick;//加仓价
							openThostOrder.OrigSubmitPrice = openThostOrder.LimitPrice;
							openThostOrder.Direction = MORDER_BUY;
							openThostOrder.Offset = MORDER_OPEN;
							openThostOrder.VolumeTotal = mStrategyParams.OpenVol2;
							openThostOrder.VolumeTotalOriginal = openThostOrder.VolumeTotal;
							openThostOrder.VolumeTraded = 0;
							openThostOrder.ProfitPrice = openThostOrder.LimitPrice + mStrategyParams.SecondProfitPoint * m_dOneTick;
							openThostOrder.mStoplossPoint = mStrategyParams.SecondProfitPoint;
							openThostOrder.OpenOrderCanbeCanceled = true;
							openThostOrder.MOrderType = 2;
							openThostOrder.maxProfit = 0.0;
							openThostOrder.PairID = mPairID;

							OnHandPositionCount += openThostOrder.VolumeTotal;
							TotalOnHandPosition += openThostOrder.VolumeTotal;
							InstanceOnHandPositionCount += openThostOrder.VolumeTotal;

							openThostOrder.mStoplossPoint = mStrategyParams.SecondStoplossPoint;
							ReqOpenOrderInsert(&openThostOrder);
						}

						if (openorder_it->VolumeTotal == 0) {
							mPairID++;
							OpenOrderList.erase(openorder_it);
						}
						break;
					}
					else if (openorder_it->Direction == MORDER_SELL && openorder_it->MOrderType == 1) {
						tradedirection = MORDER_SELL;
						MyCloseOrderType thostOrder;//平仓止盈单
						thostOrder.OpenOrderID = thisTradeRef;
						thostOrder.OpenOrderSubmitPrice = openorder_it->OrigSubmitPrice;
						thostOrder.OpenOrderTradePrice = pOrderTradeMsg->Price;
						thostOrder.IsClosePofitOrder = true;
						thostOrder.IsStoplessOrder = false;
						thostOrder.CanbeCanceled = false;
						thostOrder.dwCloseOrderStart = GetTickCount();
						thostOrder.MOrderType = openorder_it->MOrderType;
						thostOrder.MAStop = false;
						strcpy(thostOrder.OpenTime, pOrderTradeMsg->InsertOrTradeTime);
						iNextOrderRef++;
						thostOrder.OrderLocalRetReqID = 0;
						thostOrder.OrderId = -1;
						thostOrder.OrderLocalRef = -1;
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
						thostOrder.PairID = mPairID;
						//CloseOrderList.push_back(thostOrder);
						ReqCloseOrderInsert(&thostOrder, openorder_it->OpenTime);

						submitprice = openorder_it->OrigSubmitPrice;

						//报第二个开仓单到交易所
						if (openorder_it->MOrderType == 1 && InstanceOnHandPositionCount < mStrategyParams.LoopTimes
							&& (OnHandPositionCount + mStrategyParams.OpenVol2) <= MaxOnHandPositionCount
							&& (TotalOnHandPosition + mStrategyParams.OpenVol2) <= MaxTotalOnHandPosition
							&& openorder_it->VolumeTotal == 0
							) {
							char line[200];
							sprintf(line, "%s,Open Type 2 Order,Sell,T1OrderId=%d", mStrategyAndInstance.c_str(), openorder_it->OrderId);
							WriteMsgToLogList(line);

							MyOpenOrderType openThostOrder;
							iNextOrderRef++;
							openThostOrder.OrderLocalRetReqID = 0;
							openThostOrder.OrderId = -1;
							openThostOrder.OrderLocalRef = -1;
							openThostOrder.LimitPrice = pOrderTradeMsg->Price + mStrategyParams.WaveBackPoint * m_dOneTick;//加仓价
							openThostOrder.OrigSubmitPrice = openThostOrder.LimitPrice;
							openThostOrder.Direction = MORDER_SELL;
							openThostOrder.Offset = MORDER_OPEN;
							openThostOrder.VolumeTotal = mStrategyParams.OpenVol2;
							openThostOrder.VolumeTotalOriginal = openThostOrder.VolumeTotal;
							openThostOrder.VolumeTraded = 0;
							openThostOrder.ProfitPrice = openThostOrder.LimitPrice - mStrategyParams.SecondProfitPoint * m_dOneTick;
							openThostOrder.mStoplossPoint = mStrategyParams.SecondProfitPoint;
							openThostOrder.OpenOrderCanbeCanceled = true;
							openThostOrder.MOrderType = 2;
							openThostOrder.maxProfit = 0.0;
							openThostOrder.PairID = mPairID;

							OnHandPositionCount += openThostOrder.VolumeTotal;
							TotalOnHandPosition += openThostOrder.VolumeTotal;
							InstanceOnHandPositionCount += openThostOrder.VolumeTotal;

							openThostOrder.mStoplossPoint = mStrategyParams.SecondStoplossPoint;
							ReqOpenOrderInsert(&openThostOrder);
						}

						if (openorder_it->VolumeTotal == 0) {
							mPairID++;
							OpenOrderList.erase(openorder_it);
						}
						break;
					}
					else if (openorder_it->Direction == MORDER_BUY && openorder_it->MOrderType == 2)
					{
						char line[200];
						sprintf(line, "%s,Type 2,Buy,OpenOrderId=%d", mStrategyAndInstance.c_str(), openorder_it->OrderId);
						WriteMsgToLogList(line);

						tradedirection = MORDER_BUY;
						MyCloseOrderType thostOrder;//加仓的平仓止盈单
						thostOrder.OpenOrderID = thisTradeRef;
						thostOrder.OpenOrderSubmitPrice = openorder_it->OrigSubmitPrice;
						thostOrder.OpenOrderTradePrice = pOrderTradeMsg->Price;
						thostOrder.IsClosePofitOrder = true;
						thostOrder.IsStoplessOrder = false;
						thostOrder.CanbeCanceled = false;
						thostOrder.dwCloseOrderStart = GetTickCount();
						thostOrder.MOrderType = openorder_it->MOrderType;
						thostOrder.MAStop = false;
						strcpy(thostOrder.OpenTime, pOrderTradeMsg->InsertOrTradeTime);
						iNextOrderRef++;
						thostOrder.OrderLocalRetReqID = 0;
						thostOrder.OrderId = -1;
						thostOrder.OrderLocalRef = -1;
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
						//strcpy(thostOrder.OpenTime,gEqualVolRatesX[gEqualVolRatesXIndex].datatime);
						//CloseOrderList.push_back(thostOrder);
						ReqCloseOrderInsert(&thostOrder, openorder_it->OpenTime);

						submitprice = openorder_it->OrigSubmitPrice;

						//looping the close order list to cancel the type 1 profit order
						for (std::list<MyCloseOrderType>::iterator co_it = CloseOrderList.begin(); co_it != CloseOrderList.end();) {
							std::list<MyCloseOrderType>::iterator iter_etmp = co_it++;
							if (iter_etmp->Direction == MORDER_SELL && iter_etmp->PairID == openorder_it->PairID && iter_etmp->MOrderType == 1
								&& (iter_etmp->OrderStatus == MORDER_PART_TRADED || iter_etmp->OrderStatus == MORDER_ACCEPTED || iter_etmp->OrderStatus == MORDER_QUEUED)
								&& iter_etmp->CanbeCanceled
								) {
								char line[200];
								sprintf(line, "%s,Cancel Type 1 Profit,T1POrderId=%d", mStrategyAndInstance.c_str(), iter_etmp->OrderId);
								WriteMsgToLogList(line);

								iter_etmp->CanbeCanceled = false;
								iter_etmp->CancelType = 2;
								ReqOrderDelete(iter_etmp->OrderId, iter_etmp->OrderLocalRef, iter_etmp->FrontID, iter_etmp->SessionID);
							}
						}

						if (openorder_it->VolumeTotal == 0) {
							OpenOrderList.erase(openorder_it);
						}
						break;
					}
					else if (openorder_it->Direction == MORDER_SELL && openorder_it->MOrderType == 2) {
						char line[200];
						sprintf(line, "%s,Type 2,Sell,OpenOrderId=%d", mStrategyAndInstance.c_str(), openorder_it->OrderId);
						WriteMsgToLogList(line);

						tradedirection = MORDER_SELL;
						MyCloseOrderType thostOrder;//加仓的平仓止盈单
						thostOrder.OpenOrderID = thisTradeRef;
						thostOrder.OpenOrderSubmitPrice = openorder_it->OrigSubmitPrice;
						thostOrder.OpenOrderTradePrice = pOrderTradeMsg->Price;
						thostOrder.IsClosePofitOrder = true;
						thostOrder.IsStoplessOrder = false;
						thostOrder.CanbeCanceled = false;
						thostOrder.dwCloseOrderStart = GetTickCount();
						thostOrder.MOrderType = openorder_it->MOrderType;
						thostOrder.MAStop = false;
						strcpy(thostOrder.OpenTime, pOrderTradeMsg->InsertOrTradeTime);
						iNextOrderRef++;
						thostOrder.OrderLocalRetReqID = 0;
						thostOrder.OrderId = -1;
						thostOrder.OrderLocalRef = -1;
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
						//CloseOrderList.push_back(thostOrder);
						ReqCloseOrderInsert(&thostOrder, openorder_it->OpenTime);

						submitprice = openorder_it->OrigSubmitPrice;

						//looping the close order list to cancel the type 1 profit order
						for (std::list<MyCloseOrderType>::iterator co_it = CloseOrderList.begin(); co_it != CloseOrderList.end();) {
							std::list<MyCloseOrderType>::iterator iter_etmp = co_it++;
							if (iter_etmp->Direction == MORDER_BUY && iter_etmp->PairID == openorder_it->PairID && iter_etmp->MOrderType == 1
								&& (iter_etmp->OrderStatus == MORDER_PART_TRADED || iter_etmp->OrderStatus == MORDER_ACCEPTED || iter_etmp->OrderStatus == MORDER_QUEUED)
								&& iter_etmp->CanbeCanceled
								) {
								char line[200];
								sprintf(line, "%s,Cancel Type 1 Profit,T1POrderId=%d", mStrategyAndInstance.c_str(), iter_etmp->OrderId);
								WriteMsgToLogList(line);

								iter_etmp->CanbeCanceled = false;
								iter_etmp->CancelType = 2;
								ReqOrderDelete(iter_etmp->OrderId, iter_etmp->OrderLocalRef, iter_etmp->FrontID, iter_etmp->SessionID);
							}
						}

						if (openorder_it->VolumeTotal == 0) {
							OpenOrderList.erase(openorder_it);
						}
						break;
					}
					else {
						printf("openDirection Excetion!!! =%i\n", pOrderTradeMsg->Direction);
					}
				} //End if openorder_it->OrderSysId==thisTradeRef
			}
		}

		int closeprofitornot = -1;
		std::list<MyCloseOrderType>::iterator closeorder_it;
		if (!CloseOrderList.empty()) {
			for (closeorder_it = CloseOrderList.begin(); closeorder_it != CloseOrderList.end(); closeorder_it++) {
				//std::list<MyCloseOrderType>::iterator iter_ec=closeorder_it++;
				if (closeorder_it->OrderId == thisTradeRef || (closeorder_it->OrderLocalRef == pOrderTradeMsg->OrderLocalRef && closeorder_it->OrderLocalRef > 0)) {
					if (closeorder_it->OrderLocalRef == pOrderTradeMsg->OrderLocalRef && closeorder_it->OrderLocalRef > 0)closeorder_it->OrderId = thisTradeRef;
					openorclose = MORDER_CLOSE;
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

					OnHandPositionCount -= pOrderTradeMsg->Volume;
					TotalOnHandPosition -= pOrderTradeMsg->Volume;
					InstanceOnHandPositionCount -= pOrderTradeMsg->Volume;

					if (pOrderTradeMsg->Volume == closeorder_it->VolumeTotalOriginal) {
						//looping the open order list to cancel the type 2 open order
						for (std::list<MyOpenOrderType>::iterator op_it = OpenOrderList.begin(); op_it != OpenOrderList.end();) {
							std::list<MyOpenOrderType>::iterator iter_etmp = op_it++;
							if (iter_etmp->PairID == closeorder_it->PairID && iter_etmp->MOrderType == 2 && closeorder_it->MOrderType == 1
								) {
								char line[200];
								sprintf(line, "%s,Cancel Type 2 Open,OrderId=%d", mStrategyAndInstance.c_str(), iter_etmp->OrderId);
								WriteMsgToLogList(line);

								ReqOrderDelete(iter_etmp->OrderId, iter_etmp->OrderLocalRef, iter_etmp->FrontID, iter_etmp->SessionID);
							}
						}

						CloseOrderList.erase(closeorder_it);
						break;
					}
					else {
						closeorder_it->VolumeTraded = closeorder_it->VolumeTraded + pOrderTradeMsg->Volume;
						closeorder_it->VolumeTotal = closeorder_it->VolumeTotal - pOrderTradeMsg->Volume;
						if (closeorder_it->VolumeTotal == 0) {
							//looping the open order list to cancel the type 2 open order
							for (std::list<MyOpenOrderType>::iterator op_it = OpenOrderList.begin(); op_it != OpenOrderList.end();) {
								std::list<MyOpenOrderType>::iterator iter_etmp = op_it++;
								if (iter_etmp->PairID == closeorder_it->PairID && iter_etmp->MOrderType == 2 && closeorder_it->MOrderType == 1
									) {
									char line[200];
									sprintf(line, "%s,Cancel Type 2 Open,OrderId=%d", mStrategyAndInstance.c_str(), iter_etmp->OrderId);
									WriteMsgToLogList(line);

									ReqOrderDelete(iter_etmp->OrderId, iter_etmp->OrderLocalRef, iter_etmp->FrontID, iter_etmp->SessionID);
								}
							}

							CloseOrderList.erase(closeorder_it);
							break;
						}
					}
				} //End if closeorder_it->OrderSysId==thisTradeRef
			}//End While for Close Order List
		}

		if (tradedirection != -1) {
			TradeLogType trade;
			strcpy(trade.InstanceName, mInstanceName);
			strcpy(trade.StrategyID, mStrategyID);
			//int nYear,nMonth,nDate,nHour,nMin,nSec;
			string strfulltradetime(pOrderTradeMsg->InsertOrTradeTime);
			//string strtimepart=strfulltradetime.substr(strfulltradetime.find_first_of(" ")+1,strfulltradetime.length()-strfulltradetime.find_first_of(" ")-1);
			//char ctimepart[10];
			//strcpy(ctimepart,strtimepart.c_str());
			string strdatepart = strfulltradetime.substr(0, strfulltradetime.find_first_of(" "));
			//nYear=atoi(strdatepart.c_str())/10000;
			//nMonth=(atoi(strdatepart.c_str())%10000)/100;
			//nDate=(atoi(strdatepart.c_str())%10000)%100;
			//sscanf_s(ctimepart, "%d:%d:%d",&nYear, &nMonth, &nDate,&nHour,&nMin,&nSec);
			strcpy(trade.tradingday, strdatepart.c_str());
			strcpy(trade.CodeName, InstCodeName);
			trade.tradeprice = pOrderTradeMsg->Price;
			trade.submitprice = submitprice;
			if (tradedirection == MORDER_BUY) {
				trade.qty = pOrderTradeMsg->Volume;
			}
			else trade.qty = -pOrderTradeMsg->Volume;
			trade.fee = pOrderTradeMsg->MatchFee;

			Message logMsg;
			logMsg.type = TRADE_LOG;
			logMsg.AddData(&trade, 0, sizeof(TradeLogType));
			LogMessageList.AddTail(logMsg);
			ReleaseSemaphore(logSemaphore, 1, NULL);

			WaitForSingleObject(MatchNoMapSem, INFINITE);
			string matchno(pOrderTradeMsg->MatchNo);
			MatchNoToStrategyNameForDisplay.insert(std::pair<string, string>(matchno, mStrategyAndInstance));
			ReleaseSemaphore(MatchNoMapSem, 1, NULL);

			DisplayTradeOnScreen(pOrderTradeMsg, tradedirection, openorclose, closeprofitornot);

			FlushStrategyInfoToFile();
		}
	}
}

void StrategyWaveOpenAdd::OnRspOrderInsert(ShmRspOrderInsert* pRspOrderInsert)
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

void StrategyWaveOpenAdd::WideCharToMultiChar(CString str, char* dest_str)
{
	//获取缓冲区的大小，并申请空间，缓冲区大小是按字节计算的
	int len = WideCharToMultiByte(CP_ACP, 0, str, str.GetLength(), NULL, 0, NULL, NULL);
	dest_str = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, str, str.GetLength(), dest_str, len, NULL, NULL);
	dest_str[len] = '\0';
}

//datadate format: yyyymmdd , datatime formate hh24:mi:ss
//true - can open new order , false - shouldnot open new order
bool StrategyWaveOpenAdd::timeRuleForOpen(char datadate[10], char datatime[10]) {
	int nYear, nMonth, nDate;
	nYear = atoi(datadate) / 10000;
	nMonth = (atoi(datadate) % 10000) / 100;
	nDate = (atoi(datadate) % 10000) % 100;
	int nHour, nMin, nSec;
	sscanf(datatime, "%d:%d:%d", &nHour, &nMin, &nSec);
	CTime tm(nYear, nMonth, nDate, nHour, nMin, nSec);
	time_t curTime = tm.GetTime();
	if (curTime > tmt_StartTime&& curTime < tmt_EndTime) return true;
	return false;
}

//datadate format: yyyymmdd , datatime formate hh24:mi:ss
//true - must close current order , false - no need to close current order
bool StrategyWaveOpenAdd::timeRuleForClose(char datadate[10], char datatime[10]) {
	int nYear, nMonth, nDate;
	nYear = atoi(datadate) / 10000;
	nMonth = (atoi(datadate) % 10000) / 100;
	nDate = (atoi(datadate) % 10000) % 100;
	int nHour, nMin, nSec;
	sscanf_s(datatime, "%d:%d:%d", &nHour, &nMin, &nSec);
	CTime tm(nYear, nMonth, nDate, nHour, nMin, nSec);
	time_t curTime = tm.GetTime();

	if (curTime > tmt_EndTime) return true;

	return false;
}

void StrategyWaveOpenAdd::ReqOpenOrderInsert(MyOpenOrderType* pOpenOrder) {
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
		pOpenOrder->FrontID = cOrder.FrontID;
		pOpenOrder->SessionID = cOrder.SessionID;

		pOpenOrder->OrderLocalRetReqID = iRetReqID;
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

void StrategyWaveOpenAdd::ReqCloseOrderInsert(MyCloseOrderType* pCloseOrder, char OpenOrderTime[21]) {
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
		if (strcmp(strOpenDataDate.c_str(), c_str_mCurrDate) == 0) {
			CloseToday = true;
		}
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
		pCloseOrder->FrontID = cOrder.FrontID;
		pCloseOrder->SessionID = cOrder.SessionID;

		pCloseOrder->OrderLocalRetReqID = iRetReqID;
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

void StrategyWaveOpenAdd::ReqOrderDelete(int pOrderId, int pOrderLocalRef, int pFrontID, int pSessionID)
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

void StrategyWaveOpenAdd::WriteMsgToLogList(char logline[200])
{
	Message logMsg;
	logMsg.type = STRATEGY_LOG;
	logMsg.AddData(logline, 0, sizeof(char) * 200);
	LogMessageList.AddTail(logMsg);
	ReleaseSemaphore(logSemaphore, 1, NULL);
}

void StrategyWaveOpenAdd::OnDisplayLocalCloseOrderList()
{
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

	if (strlen(upMaxTime) > 0 && strlen(upMinTime) > 0 && strlen(dnMaxTime) > 0 && strlen(dnMinTime) > 0) {
		char log[200];
		sprintf(log, "%s,%d,%d,upMax=%.2f,upMin=%.2f,%s,%s,dnMax=%.2f,dnMin=%.2f,%s,%s", mStrategyAndInstance.c_str(), OpenOrderList.size(), CloseOrderList.size(), upWaveMax, upWaveMin, upMaxTime, upMinTime, dnWaveMax, dnWaveMin, dnMaxTime, dnMinTime);
		CString str(log);
		pPubMsg->AddString(str);
	}
}

void StrategyWaveOpenAdd::AddtoTipMsgListBox(char msgline[200])
{
	WaitForSingleObject(MainScreenFreshSem, INFINITE);
	CString str(msgline);
	pPubMsg->AddString(str);
	ReleaseSemaphore(MainScreenFreshSem, 1, NULL);
}

void StrategyWaveOpenAdd::DisplayTradeOnScreen(OrderTradeMsg* pOrderTradeMsg, int mDirection, int mOpenOrClose, int mCloseProfitOrNot)
{
	WaitForSingleObject(MainScreenFreshSem, INFINITE);
	int orderIndex = pTradesDetailsList->GetItemCount();
	CString itemname("");
	itemname.Format(_T("%d"), orderIndex);
	pTradesDetailsList->InsertItem(orderIndex, (LPCTSTR)itemname);

	CString csStrategyName(mStrategyName);
	CString csInstanceName(mInstanceName);

	pTradesDetailsList->SetItemText(orderIndex, 0, (LPCTSTR)csStrategyName);
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

void StrategyWaveOpenAdd::RecoverInstance(char cfgFileName[500])
{
	FILE* fptr = fopen(cfgFileName, "rb");
	fseek(fptr, 0, SEEK_SET);
	fread(&header, sizeof(mSerializeHeader), 1, fptr);
	int OpenOrderCount = header.OpenOrderCount;
	int CloseOrderCount = header.CloseOrderCount;
	memcpy(&mOpenTimes, header.SpecificArea, sizeof(int));
	InstanceOnHandPositionCount = 0;
	//	strcpy(header.StartTime,mStrategyParams.StartTime);
	//	strcpy(header.EndTime,mStrategyParams.EndTime);

	mCloseOrderSeqNo = 0;

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
		mCloseOrderSeqNo++;
		fread(&closeOrder, sizeof(MyCloseOrderType), 1, fptr);
		closeOrder.CloseOrderSeqNo = mCloseOrderSeqNo;
		OnHandPositionCount += closeOrder.VolumeTotal;
		InstanceOnHandPositionCount += closeOrder.VolumeTotal;
		TotalOnHandPosition += closeOrder.VolumeTotal;

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

	char log[200];
	sprintf(log, "%s,StrategyWaveOpenAdd,重启恢复,当前时间=%s", mStrategyAndInstance.c_str(), c_str_mCurrTime);
	CString str(log);
	pPubMsg->AddString(str);
	WriteMsgToLogList(log);
	free(c_str_mCurrTime);

	FlushStrategyInfoToFile();
}

void StrategyWaveOpenAdd::FlushStrategyInfoToFile() {
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

wstring StrategyWaveOpenAdd::s2ws(const string& s)
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

int StrategyWaveOpenAdd::CreateStrategyMapOfView() {
	CString strPathFile;
	::GetModuleFileName(NULL, strPathFile.GetBuffer(MAX_PATH), MAX_PATH);
	strPathFile.ReleaseBuffer();
	strPathFile = strPathFile.Left(strPathFile.ReverseFind(_T('\\')));
	strPathFile += "\\Strategies\\TTimeOpenModel\\";
	char filename[500];
	ConvertCStringToCharArray(strPathFile, filename);
	string strFileName(filename);
	strFileName.append(mStrategyName);
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

int StrategyWaveOpenAdd::OpenStrategyMapOfView() {
	CString strPathFile;
	::GetModuleFileName(NULL, strPathFile.GetBuffer(MAX_PATH), MAX_PATH);
	strPathFile.ReleaseBuffer();
	strPathFile = strPathFile.Left(strPathFile.ReverseFind(_T('\\')));
	strPathFile += "\\Strategies\\TTimeOpenModel\\";
	char filename[500];
	ConvertCStringToCharArray(strPathFile, filename);
	string strFileName(filename);
	strFileName.append(mStrategyName);
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

void StrategyWaveOpenAdd::CloseMap()
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

void StrategyWaveOpenAdd::SetCloseLocalOrder(LocalCLForDisplayField* pCLOrder)
{
}

void StrategyWaveOpenAdd::SetStrategyID(char strategyId[50])
{
	strcpy(mStrategyID, strategyId);
}