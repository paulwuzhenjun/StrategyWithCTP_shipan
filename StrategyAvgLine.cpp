#include "StdAfx.h"
#include "StrategyAvgLine.h"

#include "EsunTraderSpi.h"
#include "ThostTraderSpi.h"
#include "SgitTraderSpi.h"

#include "MessageList.h"
#include "TickDataList.h"
#include "OrderDataList.h"

#include "LockVariable.h"
#include "database.h"

extern CListBox* pPubMsg;

extern EsunTraderSpi* pEsunTraderSpi;
extern ThostTraderSpi* pThostTraderSpi;
extern SgitTraderSpi* pSgitTraderSpi;

extern MessageList LogMessageList;
extern OrderDataList OrderList;

extern TickDataList TickList;
extern map<int, int> OrderIdToShmIndex;
extern map<int, string> OrderIdToStrategyNameForDisplay;
extern map<string, string> MatchNoToStrategyNameForDisplay;
extern map<int, int> ReqIdToShmIndex;
extern HANDLE logSemaphore;
extern HANDLE MatchNoMapSem;
extern SRWLOCK  g_srwLockReqId;
extern SRWLOCK  g_srwLockOrderId;
extern int TotalOnHandPosition; //ԭ�Ӳ���,�ʲ�����
extern int MaxTotalOnHandPosition;
extern MessageList ScreenDisplayMsgList;
extern map<string, double> instlastpricemap;
extern HANDLE ScreenDisplaySem;
extern HANDLE MainScreenFreshSem;
extern HANDLE OrderInsertSem;
extern CListCtrl* pTradesDetailsList;
extern list<ModelNode> ModelList;
extern bool ClearInstanceCfgFile;
extern CLockVariable gLockVariable;
extern char CTPTradingDay[];
extern char SgitTradingDay[];

int StrategyAvgLine::MaxOnHandPositionCount = 0;
int StrategyAvgLine::OnHandPositionCount = 0;

StrategyAvgLine::StrategyAvgLine(char InstrumentID[30])
{
	CTime mCurrTime = CTime::GetCurrentTime();
	//ȫ�ֱ���
	m_dOneTick = 0.01;

	mTickCount = 0;
	lastmBarIndex = 0;
	//mTodayTickCount=0;

	int m_dallocBarNum = 10000;
	mBarFile = (BarRateInfo*)malloc(sizeof(BarRateInfo) * m_dallocBarNum);
	//mBarClose=(double*)malloc(sizeof(double)*m_dallocBarNum);
	mBarIndex = 0;
	// 1min
	mBarSize = 60;

	m_bIsRunning = true;
	m_bCrossTradingDay = false;

	memset(&mStrategyParams, 0, sizeof(mParasType));
	memset(&m_ThisData, 0, sizeof(TickInfo));

	strcpy(mStrategyName, "AvgLine");
	strcpy(mInstanceName, "");
	mStrategyAndInstance = mStrategyName;

	mOpenPrice = 0;
	m_bBuyRet = false;
	m_bSellRet = false;
	m_dHigh = 0;
	m_dLow = 0;

	InstanceOnHandPositionCount = 0;
	WriteMsgToLogList("Strategy Init..");
}

StrategyAvgLine::~StrategyAvgLine(void)
{
}

void StrategyAvgLine::OnRtnDepthMarketData(TickInfo* pDepthMarketData)
{
	//if(!timeRuleOK(pDepthMarketData->updatetime)) return;
	memcpy(&m_ThisData, pDepthMarketData, sizeof(TickInfo));

	///��ȡ����۸����õ�ǰ���صĵ�ǰ�۸�
	m_Price = pDepthMarketData->price;
	m_Buy1 = pDepthMarketData->bid1;
	m_Sell1 = pDepthMarketData->ask1;
	int EnterSellTrade = 0, EnterBuyTrade = 0;

	int nYear_m, nMonth_m, nDate_m;
	nYear_m = atoi(pDepthMarketData->datadate) / 10000;
	nMonth_m = (atoi(pDepthMarketData->datadate) % 10000) / 100;
	nDate_m = (atoi(pDepthMarketData->datadate) % 10000) % 100;

	int  nHour_m, nMin_m, nSec_m;
	sscanf(pDepthMarketData->updatetime, "%d:%d:%d", &nHour_m, &nMin_m, &nSec_m);

	int nOpenHour, nOpenMin, nOpenSec, nCloseHour, nCloseMin, nCloseSec;
	sscanf_s(mStrategyParams.OpenTime, "%d:%d:%d", &nOpenHour, &nOpenMin, &nOpenSec);
	sscanf_s(mStrategyParams.CloseTime, "%d:%d:%d", &nCloseHour, &nCloseMin, &nCloseSec);

	//if(nHour_m>=nCloseHour&&nHour_m<nOpenHour) return;

	//CTime tm_m(nYear_m, nMonth_m, nDate_m,nHour_m,nMin_m,nSec_m);
	//curTickTime=tm_m.GetTime();
	//strcpy(TickRealTimeDataDate,pDepthMarketData->datadate);

	//if(curTickTime>tmt_StartTime) {
	AddToRatesX(mTickCount, pDepthMarketData, mBarFile, &mBarIndex, mBarSize);
	mTickCount++;

	if (m_fAvg > 0 && InstanceOnHandPositionCount <= 0) {
		if (pDepthMarketData->price >= m_fAvg * (1 + mStrategyParams.m_fX) && !m_bSellRet) {
			m_dHigh = pDepthMarketData->price;
			m_bSellRet = true;
		}
		else if (pDepthMarketData->price <= m_fAvg * (1 - mStrategyParams.m_fX) && !m_bBuyRet) {
			m_dLow = pDepthMarketData->price;
			m_bBuyRet = true;
		}
	}
	if (m_bSellRet) {
		if (pDepthMarketData->price > m_dHigh) {
			m_dHigh = pDepthMarketData->price;
			return;
		}
		if ((m_dHigh - pDepthMarketData->price) / m_fAvg >= mStrategyParams.m_fY) {
			EnterSellTrade = 1;		// ���򿪵�
			m_fStop = m_dHigh;		// ����ֹ��۸�
			char line[200];
			sprintf(line, "%s,Buy,%s", mStrategyAndInstance.c_str(), mBarFile[mBarIndex].datatime);
			WriteMsgToLogList(line);
			m_bSellRet = false;
		}
	}
	if (m_bBuyRet) {
		if (pDepthMarketData->price < m_dLow) {
			m_dLow = pDepthMarketData->price;
			return;
		}
		if ((pDepthMarketData->price - m_dLow) / m_fAvg >= mStrategyParams.m_fY) {
			EnterBuyTrade = 1;		// ���򿪵�
			m_fStop = m_dLow;		// ����ֹ��۸�
			char line[200];
			sprintf(line, "%s,Buy,%s", mStrategyAndInstance.c_str(), mBarFile[mBarIndex].datatime);
			WriteMsgToLogList(line);
			m_bBuyRet = false;
		}
	}

	//Processing Opened Order	������
	std::list<MyOpenOrderType>::iterator openorder_it;
	if (!OpenOrderList.empty()) {
		for (openorder_it = OpenOrderList.begin(); openorder_it != OpenOrderList.end(); ++openorder_it) {
			if (openorder_it->VolumeTotal != 0 && openorder_it->OpenOrderCanbeCanceled) {
				if (((openorder_it->LimitPrice - 0.00001) > m_Sell1&& openorder_it->Direction == MORDER_SELL)
					|| ((m_Buy1 - 0.00001) > openorder_it->LimitPrice&& openorder_it->Direction == MORDER_BUY)
					//&&(openBarIndex-mBarIndex)>0
					) {
					char line[200];
					sprintf(line, "%s,Cancel Open Order,Ref=%d,OrderId=%d", mStrategyAndInstance.c_str(), openorder_it->OrderLocalRef, openorder_it->OrderId);
					WriteMsgToLogList(line);

					ReqOrderDelete(openorder_it->OrderId, openorder_it->OrderLocalRef, openorder_it->FrontID, openorder_it->SessionID);
					openorder_it->OpenOrderCanbeCanceled = false;
				}
			}
		}//end for loop
	}
	//End for Opened Order processing

	int BuyOrderOnHand = 0, SellOrderOnHand = 0;
	std::list<MyCloseOrderType>::iterator closeorder_it;
	if (!CloseOrderList.empty()) {
		//Loop Close Order List
		int CloseOrderCount = 0;
		for (closeorder_it = CloseOrderList.begin(); closeorder_it != CloseOrderList.end();) {
			std::list<MyCloseOrderType>::iterator iter_e = closeorder_it++;

			if (iter_e->Direction == MORDER_SELL &&
				(iter_e->OrderStatus == MORDER_PART_TRADED || iter_e->OrderStatus == MORDER_ACCEPTED || iter_e->OrderStatus == MORDER_QUEUED)) {
				//Closing the buy order
				BuyOrderOnHand++;
				iter_e->maxProfit = max(iter_e->maxProfit, m_Price - iter_e->OpenOrderTradePrice);
				// ��ӯ�����ȴ���MA(10)�İٷֱȿ�ʼ׷��
				if ((iter_e->maxProfit + 0.00001) > m_fAvg* mStrategyParams.m_fTP) {
					iter_e->MAStop = true;
				}
				if ((m_Buy1 - 0.00001) < iter_e->ManualStopPrice && iter_e->IsClosePofitOrder && iter_e->ManualStopPrice > 0.001) {
					//�ֶ�ֹ����,ͨ���ֶ����ô�ֵ,����ʵ���ֶ�ֹ��
					char line[200];
					sprintf(line, "%s,Buy Order Fix Stop Manually,%.4f,%.1f,%.4f,Ref=%d", mStrategyAndInstance.c_str(), iter_e->OpenOrderTradePrice, iter_e->ManualStopPrice, m_Buy1, iter_e->OrderLocalRef);
					WriteMsgToLogList(line);

					MyCloseOrderType newThostOrder;

					newThostOrder.OrderLocalRetReqID = 0;
					newThostOrder.CloseOrderSeqNo = 0;
					newThostOrder.OpenOrderID = iter_e->OpenOrderID;
					newThostOrder.OpenOrderATR = iter_e->OpenOrderATR;
					strcpy(newThostOrder.OpenTime, iter_e->OpenTime);
					newThostOrder.IsStoplessOrder = true;
					newThostOrder.CanbeCanceled = true;
					newThostOrder.IsClosePofitOrder = false;
					newThostOrder.OpenOrderTradePrice = iter_e->OpenOrderTradePrice;
					newThostOrder.MOrderType = iter_e->MOrderType;
					newThostOrder.Direction = iter_e->Direction;
					newThostOrder.LimitPrice = iter_e->ManualStopPrice;//���ֶ�ֹ��۽��б���
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
				else if (iter_e->IsClosePofitOrder && m_Price < (iter_e->OpenOrderTradePrice + iter_e->maxProfit * (1 - mStrategyParams.m_fTS) + 0.00001) && iter_e->MAStop) {
					char line[200];
					sprintf(line, "%s,Sell Order Signal C,%.4f,%.1f,Ref=%d", mStrategyAndInstance.c_str(), iter_e->OpenOrderTradePrice, mBarFile[mBarIndex - 1].close, iter_e->OrderLocalRef);
					WriteMsgToLogList(line);

					MyCloseOrderType newThostOrder;

					newThostOrder.OrderLocalRetReqID = 0;
					newThostOrder.CloseOrderSeqNo = 0;
					newThostOrder.OpenOrderID = iter_e->OpenOrderID;
					newThostOrder.OpenOrderATR = iter_e->OpenOrderATR;
					strcpy(newThostOrder.OpenTime, iter_e->OpenTime);
					newThostOrder.IsStoplessOrder = true;
					newThostOrder.CanbeCanceled = true;
					newThostOrder.IsClosePofitOrder = false;
					newThostOrder.OpenOrderTradePrice = iter_e->OpenOrderTradePrice;
					newThostOrder.MOrderType = iter_e->MOrderType;
					newThostOrder.Direction = iter_e->Direction;
					newThostOrder.LimitPrice = m_Price;//�����¼۽��б���
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
				else if (iter_e->IsClosePofitOrder && m_Price > 0 && m_Price < m_fStop) {
					char line[200];
					sprintf(line, "%s,Buy Order Stop C,%.4f,%.1f,%.4f,Ref=%d", mStrategyAndInstance.c_str(), iter_e->OpenOrderTradePrice, iter_e->mStoplossPoint, m_Price, iter_e->OrderLocalRef);
					WriteMsgToLogList(line);

					MyCloseOrderType newThostOrder;

					newThostOrder.OrderLocalRetReqID = 0;
					newThostOrder.CloseOrderSeqNo = 0;
					newThostOrder.OpenOrderID = iter_e->OpenOrderID;
					newThostOrder.OpenOrderATR = iter_e->OpenOrderATR;
					strcpy(newThostOrder.OpenTime, iter_e->OpenTime);
					newThostOrder.IsStoplessOrder = true;
					newThostOrder.CanbeCanceled = true;
					newThostOrder.IsClosePofitOrder = false;
					newThostOrder.OpenOrderTradePrice = iter_e->OpenOrderTradePrice;
					newThostOrder.MOrderType = iter_e->MOrderType;
					newThostOrder.Direction = iter_e->Direction;
					newThostOrder.LimitPrice = m_fStop;//��ֹ��۽��б�ֹ��
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
				else if (timeRuleForClose(pDepthMarketData->datadate, pDepthMarketData->updatetime)) {
					char line[200];
					sprintf(line, "%s,Buy Order closeTime C,%.4f,%.1f,%.4f,Ref=%d", mStrategyAndInstance.c_str(), iter_e->OpenOrderTradePrice, iter_e->mStoplossPoint, m_Price, iter_e->OrderLocalRef);
					WriteMsgToLogList(line);

					MyCloseOrderType newThostOrder;

					newThostOrder.OrderLocalRetReqID = 0;
					newThostOrder.CloseOrderSeqNo = 0;
					newThostOrder.OpenOrderID = iter_e->OpenOrderID;
					newThostOrder.OpenOrderATR = iter_e->OpenOrderATR;
					strcpy(newThostOrder.OpenTime, iter_e->OpenTime);
					newThostOrder.IsStoplessOrder = true;
					newThostOrder.CanbeCanceled = true;
					newThostOrder.IsClosePofitOrder = false;
					newThostOrder.OpenOrderTradePrice = iter_e->OpenOrderTradePrice;
					newThostOrder.MOrderType = iter_e->MOrderType;
					newThostOrder.Direction = iter_e->Direction;
					newThostOrder.LimitPrice = m_Price;
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
				else if ((iter_e->LimitPrice - 0.00001) > m_Sell1&& iter_e->IsStoplessOrder&& iter_e->CanbeCanceled) {
					char line[200];
					sprintf(line, "%s,Buy Order Close ReSubmit,Ref=%d", mStrategyAndInstance.c_str(), iter_e->OrderLocalRef);
					WriteMsgToLogList(line);

					//TRACE("Cancel the stopless order for buy order,stopless price=%.1f,sell1=%.1f\n",iter_e->LimitPrice,m_Sell1);
					iter_e->NextCloseOrderPrice = m_Buy1;
					ReqOrderDelete(iter_e->OrderId, iter_e->OrderLocalRef, iter_e->FrontID, iter_e->SessionID);
					iter_e->CanbeCanceled = false;
					CloseOrderCount++;
					if (CloseOrderCount >= 6) break;
				}
			}//End IF Closing buy order
			else if (iter_e->Direction == MORDER_BUY &&
				(iter_e->OrderStatus == MORDER_PART_TRADED || iter_e->OrderStatus == MORDER_ACCEPTED || iter_e->OrderStatus == MORDER_QUEUED)) {
				SellOrderOnHand++;
				//Closing sell order
				iter_e->maxProfit = max(iter_e->maxProfit, iter_e->OpenOrderTradePrice - m_Price);
				// ��ӯ�����ȴ���MA(10)�İٷֱȿ�ʼ׷��
				if ((iter_e->maxProfit + 0.00001) > m_fAvg* mStrategyParams.m_fTP) {
					iter_e->MAStop = true;
				}

				if ((m_Sell1 + 0.00001) > iter_e->ManualStopPrice&& iter_e->IsClosePofitOrder&& iter_e->ManualStopPrice > 0.001) {
					//�ֶ�ֹ����,ͨ���ֶ����ô�ֵ,����ʵ���ֶ�ֹ��
					char line[200];
					sprintf(line, "%s,Sell Order Fix Stop Manually,%.4f,%.1f,%.4f,Ref=%d", mStrategyAndInstance.c_str(), iter_e->OpenOrderTradePrice, iter_e->ManualStopPrice, m_Buy1, iter_e->OrderLocalRef);
					WriteMsgToLogList(line);

					MyCloseOrderType newThostOrder;

					newThostOrder.OrderLocalRetReqID = 0;
					newThostOrder.CloseOrderSeqNo = 0;
					newThostOrder.OpenOrderID = iter_e->OpenOrderID;
					newThostOrder.OpenOrderATR = iter_e->OpenOrderATR;
					strcpy(newThostOrder.OpenTime, iter_e->OpenTime);
					newThostOrder.IsStoplessOrder = true;
					newThostOrder.CanbeCanceled = true;
					newThostOrder.IsClosePofitOrder = false;
					newThostOrder.OpenOrderTradePrice = iter_e->OpenOrderTradePrice;
					newThostOrder.MOrderType = iter_e->MOrderType;
					newThostOrder.Direction = iter_e->Direction;
					newThostOrder.LimitPrice = iter_e->ManualStopPrice;//���ֶ�ֹ��۽��б���
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
				else if (iter_e->IsClosePofitOrder
					&& m_Price > (iter_e->OpenOrderTradePrice - iter_e->maxProfit * (1 - mStrategyParams.m_fTS) + 0.00001) && iter_e->MAStop) {
					char line[200];
					sprintf(line, "%s,Buy Order Signal C,%.4f,%.1f,Ref=%d", mStrategyAndInstance.c_str(), iter_e->OpenOrderTradePrice, mBarFile[mBarIndex - 1].close, iter_e->OrderLocalRef);
					WriteMsgToLogList(line);

					MyCloseOrderType newThostOrder;

					newThostOrder.OrderLocalRetReqID = 0;
					newThostOrder.CloseOrderSeqNo = 0;
					newThostOrder.OpenOrderID = iter_e->OpenOrderID;
					newThostOrder.OpenOrderATR = iter_e->OpenOrderATR;
					strcpy(newThostOrder.OpenTime, iter_e->OpenTime);
					newThostOrder.IsStoplessOrder = true;
					newThostOrder.CanbeCanceled = true;
					newThostOrder.IsClosePofitOrder = false;
					newThostOrder.OpenOrderTradePrice = iter_e->OpenOrderTradePrice;
					newThostOrder.MOrderType = iter_e->MOrderType;
					newThostOrder.Direction = iter_e->Direction;
					newThostOrder.LimitPrice = m_Price;//�����¼۽��б���
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
				else if (iter_e->IsClosePofitOrder && m_Price > 0 && m_Price > m_fStop) {
					char line[200];
					sprintf(line, "%s,Sell Order Stop C,%.4f,%.1f,%.4f,Ref=%d", mStrategyAndInstance.c_str(), iter_e->OpenOrderTradePrice, iter_e->mStoplossPoint, m_Price, iter_e->OrderLocalRef);
					WriteMsgToLogList(line);

					MyCloseOrderType newThostOrder;

					newThostOrder.OrderLocalRetReqID = 0;
					newThostOrder.CloseOrderSeqNo = 0;
					newThostOrder.OpenOrderID = iter_e->OpenOrderID;
					newThostOrder.OpenOrderATR = iter_e->OpenOrderATR;
					strcpy(newThostOrder.OpenTime, iter_e->OpenTime);
					newThostOrder.IsStoplessOrder = true;
					newThostOrder.CanbeCanceled = true;
					newThostOrder.IsClosePofitOrder = false;
					newThostOrder.OpenOrderTradePrice = iter_e->OpenOrderTradePrice;
					newThostOrder.MOrderType = iter_e->MOrderType;
					newThostOrder.Direction = iter_e->Direction;
					newThostOrder.LimitPrice = m_fStop;//��ֹ��۽��б�ֹ��
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
				else if (timeRuleForClose(pDepthMarketData->datadate, pDepthMarketData->updatetime)) {
					char line[200];
					sprintf(line, "%s,Sell Order closeTime C,%.4f,%.1f,%.4f,Ref=%d", mStrategyAndInstance.c_str(), iter_e->OpenOrderTradePrice, iter_e->mStoplossPoint, m_Price, iter_e->OrderLocalRef);
					WriteMsgToLogList(line);

					MyCloseOrderType newThostOrder;

					newThostOrder.OrderLocalRetReqID = 0;
					newThostOrder.CloseOrderSeqNo = 0;
					newThostOrder.OpenOrderID = iter_e->OpenOrderID;
					newThostOrder.OpenOrderATR = iter_e->OpenOrderATR;
					strcpy(newThostOrder.OpenTime, iter_e->OpenTime);
					newThostOrder.IsStoplessOrder = true;
					newThostOrder.CanbeCanceled = true;
					newThostOrder.IsClosePofitOrder = false;
					newThostOrder.OpenOrderTradePrice = iter_e->OpenOrderTradePrice;
					newThostOrder.MOrderType = iter_e->MOrderType;
					newThostOrder.Direction = iter_e->Direction;
					newThostOrder.LimitPrice = m_Price;
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
				else if (iter_e->LimitPrice < (m_Buy1 - 0.00001) && iter_e->IsStoplessOrder && iter_e->CanbeCanceled) {
					char line[200];
					sprintf(line, "%s,Sell Order Close ReSubmit,Ref=%d", mStrategyAndInstance.c_str(), iter_e->OrderLocalRef);
					WriteMsgToLogList(line);
					//TRACE("Cancel the stopless order for sell order,stopless price=%.1f,buy1=%.1f\n",iter_e->LimitPrice,m_Buy1);
					iter_e->NextCloseOrderPrice = m_Sell1;
					ReqOrderDelete(iter_e->OrderId, iter_e->OrderLocalRef, iter_e->FrontID, iter_e->SessionID);
					iter_e->CanbeCanceled = false;
					CloseOrderCount++;
					if (CloseOrderCount >= 6) break;
				}
			}//End IF Closing sell order
			//}//End for Strategy 1
		}//End Looping close order list
	}//End if the close order list is not null

	if (timeRuleForOpen(pDepthMarketData->datadate, pDepthMarketData->updatetime)) {
		bool BuyOrderSubmitted = false; bool SellOrderSubmitted = false;
		if (EnterBuyTrade == 1
			&& BuyOrderOnHand == 0
			&& InstanceOnHandPositionCount < mStrategyParams.LoopTimes
			&& mStrategyParams.OpenBuyAllow) {
			// �޲�1��
			if (OnHandPositionCount <= 0 && (OnHandPositionCount + 1) <= MaxOnHandPositionCount && (TotalOnHandPosition + 1) <= MaxTotalOnHandPosition) {
				//Open Buy Order
				MyOpenOrderType openThostOrder;

				openThostOrder.OrderLocalRetReqID = 0;
				openThostOrder.OrderId = -1;
				openThostOrder.LimitPrice = m_Buy1;
				openThostOrder.OrigSubmitPrice = openThostOrder.LimitPrice;
				openThostOrder.Direction = MORDER_BUY;
				openThostOrder.Offset = MORDER_OPEN;
				openThostOrder.VolumeTotal = 1;
				openThostOrder.VolumeTotalOriginal = openThostOrder.VolumeTotal;
				openThostOrder.VolumeTraded = 0;
				openThostOrder.ProfitPrice = openThostOrder.LimitPrice + mStrategyParams.ProfitPoint * m_dOneTick;
				openThostOrder.OpenOrderCanbeCanceled = true;
				openThostOrder.MOrderType = 1;
				openThostOrder.OpenOrderATR = 0;
				openThostOrder.maxProfit = 0.0;
				openThostOrder.mStoplossPoint = mStrategyParams.StoplossPoint;
				ReqOpenOrderInsert(&openThostOrder);

				OnHandPositionCount += openThostOrder.VolumeTotal;
				InstanceOnHandPositionCount += openThostOrder.VolumeTotal;
				TotalOnHandPosition += openThostOrder.VolumeTotal;

				BuyOrderSubmitted = true;

				openBarIndex = mBarIndex;

				char log[200];
				sprintf(log, "%s,����,��=%.5f,ʱ��=%s,����=%d,%d", mStrategyAndInstance.c_str(), openThostOrder.LimitPrice, pDepthMarketData->updatetime, openThostOrder.VolumeTotal, shmindex);
				AddtoTipMsgListBox(log);
				WriteMsgToLogList(log);
			}
		}

		if (EnterSellTrade == 1
			&& SellOrderOnHand == 0
			&& InstanceOnHandPositionCount < mStrategyParams.LoopTimes
			&& mStrategyParams.OpenSellAllow) {
			if (OnHandPositionCount <= 0 && (OnHandPositionCount + 1) <= MaxOnHandPositionCount && (TotalOnHandPosition + 1) <= MaxTotalOnHandPosition) {
				//Open Sell Order
				MyOpenOrderType openThostOrder;

				openThostOrder.OrderLocalRetReqID = 0;
				openThostOrder.OrderId = -1;
				openThostOrder.LimitPrice = m_Sell1;
				openThostOrder.OrigSubmitPrice = openThostOrder.LimitPrice;
				openThostOrder.Direction = MORDER_SELL;
				openThostOrder.Offset = MORDER_OPEN;
				openThostOrder.VolumeTotal = 1;
				openThostOrder.VolumeTotalOriginal = openThostOrder.VolumeTotal;
				openThostOrder.VolumeTraded = 0;
				openThostOrder.ProfitPrice = openThostOrder.LimitPrice - mStrategyParams.ProfitPoint * m_dOneTick;
				openThostOrder.OpenOrderCanbeCanceled = true;
				openThostOrder.MOrderType = 1;
				openThostOrder.maxProfit = 0.0;
				openThostOrder.OpenOrderATR = 0;
				openThostOrder.mStoplossPoint = mStrategyParams.StoplossPoint;
				ReqOpenOrderInsert(&openThostOrder);

				OnHandPositionCount += openThostOrder.VolumeTotal;
				InstanceOnHandPositionCount += openThostOrder.VolumeTotal;
				TotalOnHandPosition += openThostOrder.VolumeTotal;

				SellOrderSubmitted = true;

				openBarIndex = mBarIndex;

				char log[200];
				sprintf(log, "%s,����,��=%.5f,ʱ��=%s,����=%d,%d", mStrategyAndInstance.c_str(), openThostOrder.LimitPrice, pDepthMarketData->updatetime, openThostOrder.VolumeTotal, shmindex);
				AddtoTipMsgListBox(log);
				WriteMsgToLogList(log);
			}
		}
	}

	// ��bar��ֵ
	lastmBarIndex = mBarIndex;

	FlushStrategyInfoToFile();
}

void StrategyAvgLine::OnRtnOrder(OrderTradeMsg* pOrderTradeMsg)
{
	int thisOrderId = (pOrderTradeMsg->OrderSysId);
	//UpdateOrderIdToOrderList(thisOrderId);
	char line[200];
	sprintf(line, "%s,OnRtnOrder,Ref=%d,OrderId=%d,Status=%c,VolRemain=%d", mStrategyAndInstance.c_str(), pOrderTradeMsg->OrderLocalRef, thisOrderId, pOrderTradeMsg->OrderStatus, pOrderTradeMsg->VolumeTotal);
	WriteMsgToLogList(line);

	std::list<MyOpenOrderType>::iterator openorder_it;
	if (!OpenOrderList.empty()) {
		for (openorder_it = OpenOrderList.begin(); openorder_it != OpenOrderList.end();) {
			std::list<MyOpenOrderType>::iterator iter_e = openorder_it++;
			if (iter_e->OrderId == thisOrderId || (iter_e->OrderLocalRef == pOrderTradeMsg->OrderLocalRef && iter_e->OrderLocalRef > 0 && (ThostTraderAPI || SgitTraderAPI))) {
				//printf("OpenOrderList exist order \n");
				//if(iter_e->OrderLocalRef==pOrderTradeMsg->OrderLocalRef&&iter_e->OrderLocalRef>0)iter_e->OrderId=thisOrderId;
				//CTPδ����OnRspOrderInsert,�ڴ˴�����map
				map<int, string>::iterator iter;
				AcquireSRWLockExclusive(&g_srwLockReqId);
				iter = OrderIdToStrategyNameForDisplay.find(thisOrderId);
				if (iter == OrderIdToStrategyNameForDisplay.end()) {
					string sfinal(mStrategyAndInstance);
					sfinal.append("_open");
					OrderIdToStrategyNameForDisplay.insert(std::pair<int, string>(thisOrderId, sfinal));
				}
				ReleaseSRWLockExclusive(&g_srwLockReqId);

				iter_e->OrderStatus = pOrderTradeMsg->OrderStatus;
				if (pOrderTradeMsg->OrderStatus == MORDER_PART_CANCELLED || pOrderTradeMsg->OrderStatus == MORDER_CANCELLED) {
					//���ֵ�����,���±���
					MyOpenOrderType newOpenOrder;

					newOpenOrder.OrderLocalRetReqID = 0;
					newOpenOrder.OrderId = -1;
					newOpenOrder.OrigSubmitPrice = iter_e->OrigSubmitPrice;
					newOpenOrder.OpenOrderATR = iter_e->OpenOrderATR;
					if (iter_e->Direction == MORDER_BUY) {
						newOpenOrder.LimitPrice = m_Sell1;
						newOpenOrder.ProfitPrice = iter_e->ProfitPrice;
					}
					else {
						newOpenOrder.LimitPrice = m_Buy1;
						newOpenOrder.ProfitPrice = iter_e->ProfitPrice;
					}
					newOpenOrder.Direction = iter_e->Direction;
					newOpenOrder.Offset = MORDER_OPEN;
					newOpenOrder.VolumeTotal = iter_e->VolumeTotal;
					newOpenOrder.VolumeTotalOriginal = newOpenOrder.VolumeTotal;
					newOpenOrder.VolumeTraded = 0;
					newOpenOrder.OpenOrderCanbeCanceled = true;
					newOpenOrder.MOrderType = 1;
					newOpenOrder.maxProfit = 0.0;
					newOpenOrder.mStoplossPoint = iter_e->mStoplossPoint;
					ReqOpenOrderInsert(&newOpenOrder);

					OpenOrderList.erase(iter_e);
					break;
				}
			} //End if openorder_it->OrderSysId==thisTradeRef
		}//End While
	}

	std::list<MyCloseOrderType>::iterator closeorder_it;
	if (!CloseOrderList.empty()) {
		for (closeorder_it = CloseOrderList.begin(); closeorder_it != CloseOrderList.end();) {
			std::list<MyCloseOrderType>::iterator iter_ec = closeorder_it++;
			if (iter_ec->OrderId == thisOrderId || (iter_ec->OrderLocalRef == pOrderTradeMsg->OrderLocalRef && iter_ec->OrderLocalRef > 0 && (ThostTraderAPI || SgitTraderAPI))) {
				//printf("CloseOrderList exist order \n");
				//if(iter_ec->OrderLocalRef==pOrderTradeMsg->OrderLocalRef&&iter_ec->OrderLocalRef>0)iter_ec->OrderId=thisOrderId;
				//CTPδ����OnRspOrderInsert,�ڴ˴�����map
				map<int, string>::iterator iter;
				AcquireSRWLockExclusive(&g_srwLockReqId);
				iter = OrderIdToStrategyNameForDisplay.find(thisOrderId);
				if (iter == OrderIdToStrategyNameForDisplay.end()) {
					string sfinal(mStrategyAndInstance);
					sfinal.append("_close");
					OrderIdToStrategyNameForDisplay.insert(std::pair<int, string>(thisOrderId, sfinal));
				}
				ReleaseSRWLockExclusive(&g_srwLockReqId);

				iter_ec->OrderStatus = pOrderTradeMsg->OrderStatus;
				if (pOrderTradeMsg->OrderStatus == MORDER_PART_CANCELLED || pOrderTradeMsg->OrderStatus == MORDER_CANCELLED)
				{
					if (iter_ec->IsStoplessOrder) {
						char linex[200];
						sprintf(linex, "%s,OnRtnOrder IsStoplessOrder,Ref=%d", mStrategyAndInstance.c_str(), pOrderTradeMsg->OrderLocalRef);
						WriteMsgToLogList(linex);
						//ֹ�𵥱���
						if (iter_ec->Direction == MORDER_SELL)
						{
							MyCloseOrderType newThostOrder;

							newThostOrder.OrderLocalRetReqID = 0;
							newThostOrder.CloseOrderSeqNo = 0;
							newThostOrder.OpenOrderID = iter_ec->OpenOrderID;
							newThostOrder.OpenOrderATR = iter_ec->OpenOrderATR;
							strcpy(newThostOrder.OpenTime, iter_ec->OpenTime);
							newThostOrder.IsStoplessOrder = true;
							newThostOrder.MAStop = iter_ec->MAStop;
							newThostOrder.CanbeCanceled = true;
							newThostOrder.IsClosePofitOrder = false;
							newThostOrder.ManualStopPrice = iter_ec->ManualStopPrice;
							newThostOrder.OpenOrderTradePrice = iter_ec->OpenOrderTradePrice;
							newThostOrder.MOrderType = iter_ec->MOrderType;
							newThostOrder.Direction = iter_ec->Direction;
							newThostOrder.LimitPrice = m_Buy1;//iter_ec->NextCloseOrderPrice;//��ԭ���趨�ļ۸���б���
							newThostOrder.OrigSubmitPrice = iter_ec->OrigSubmitPrice;
							newThostOrder.Offset = iter_ec->Offset;
							newThostOrder.VolumeTotalOriginal = iter_ec->VolumeTotal;
							newThostOrder.VolumeTotal = newThostOrder.VolumeTotalOriginal;
							newThostOrder.VolumeTraded = 0;

							ReqCloseOrderInsert(&newThostOrder, newThostOrder.OpenTime);
						}
						else if (iter_ec->Direction == MORDER_BUY)
						{
							MyCloseOrderType newThostOrder;

							newThostOrder.OrderLocalRetReqID = 0;
							newThostOrder.CloseOrderSeqNo = 0;
							newThostOrder.OpenOrderID = iter_ec->OpenOrderID;
							newThostOrder.OpenOrderATR = iter_ec->OpenOrderATR;
							strcpy(newThostOrder.OpenTime, iter_ec->OpenTime);
							newThostOrder.IsStoplessOrder = true;
							newThostOrder.MAStop = iter_ec->MAStop;
							newThostOrder.CanbeCanceled = true;
							newThostOrder.IsClosePofitOrder = false;
							newThostOrder.ManualStopPrice = iter_ec->ManualStopPrice;
							newThostOrder.OpenOrderTradePrice = iter_ec->OpenOrderTradePrice;
							newThostOrder.MOrderType = iter_ec->MOrderType;
							newThostOrder.Direction = iter_ec->Direction;
							newThostOrder.LimitPrice = m_Sell1;//iter_ec->NextCloseOrderPrice;//��ԭ���趨�ļ۸���б���
							newThostOrder.OrigSubmitPrice = iter_ec->OrigSubmitPrice;
							newThostOrder.Offset = iter_ec->Offset;
							newThostOrder.VolumeTotalOriginal = iter_ec->VolumeTotal;
							newThostOrder.VolumeTotal = newThostOrder.VolumeTotalOriginal;
							newThostOrder.VolumeTraded = 0;

							ReqCloseOrderInsert(&newThostOrder, newThostOrder.OpenTime);
						}
					}
					CloseOrderList.erase(iter_ec);
					break;
				}
			}//End if closeorder_it->OrderSysId==thisOrderRef
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

void StrategyAvgLine::OnRtnTrade(OrderTradeMsg* pOrderTradeMsg)
{
	int thisTradeRef = pOrderTradeMsg->OrderSysId;
	string strmatchno(pOrderTradeMsg->MatchNo);
	map<string, string>::iterator iter;
	iter = matchnomap.find(strmatchno);
	if (iter == matchnomap.end()) {
		matchnomap.insert(std::pair<string, string>(strmatchno, strmatchno));
		//UpdateOrderIdToOrderList(thisTradeRef);
		char line[200];
		sprintf(line, "%s,OnRtnTrade,Ref=%d,OrderId=%d,vol=%d,price=%.5f", mStrategyAndInstance.c_str(), pOrderTradeMsg->OrderLocalRef, thisTradeRef, pOrderTradeMsg->Volume, pOrderTradeMsg->Price);
		WriteMsgToLogList(line);

		double submitprice = 0;
		int tradedirection = -1;
		int openorclose = -1;
		std::list<MyOpenOrderType>::iterator openorder_it;
		if (!OpenOrderList.empty()) {
			for (openorder_it = OpenOrderList.begin(); openorder_it != OpenOrderList.end(); openorder_it++) {
				//std::list<MyOpenOrderType>::iterator iter_e=openorder_it++;
				char xline[200];
				sprintf(xline, "%s,OnRtnTrade,ORef=%d,OOrderId=%d", mStrategyAndInstance.c_str(), openorder_it->OrderLocalRef, openorder_it->OrderId);
				WriteMsgToLogList(xline);
				if ((openorder_it->OrderLocalRef == pOrderTradeMsg->OrderLocalRef && openorder_it->OrderLocalRef > 0 && (ThostTraderAPI || SgitTraderAPI)) || openorder_it->OrderId == thisTradeRef) {
					//printf("OpenOrderList exist order \n");
					//if(openorder_it->OrderLocalRef==pOrderTradeMsg->OrderLocalRef&&openorder_it->OrderLocalRef>0)openorder_it->OrderId=thisTradeRef;

					openorclose = MORDER_OPEN;
					openorder_it->VolumeTraded = openorder_it->VolumeTraded + pOrderTradeMsg->Volume;
					openorder_it->VolumeTotal = openorder_it->VolumeTotal - pOrderTradeMsg->Volume;
					strcpy(openorder_it->OpenTime, pOrderTradeMsg->InsertOrTradeTime);

					if (openorder_it->Direction == MORDER_BUY)
					{
						tradedirection = MORDER_BUY;
						MyCloseOrderType thostOrder;//�����ƽ��ֹӯ��,δʵ���ύ��������,������ֹӯ�����ύֹӯ��
						thostOrder.OpenOrderSubmitPrice = openorder_it->OrigSubmitPrice;
						thostOrder.OpenOrderATR = openorder_it->OpenOrderATR;
						thostOrder.OpenOrderTradePrice = pOrderTradeMsg->Price;
						thostOrder.IsClosePofitOrder = true;
						thostOrder.IsStoplessOrder = false;
						thostOrder.CanbeCanceled = false;
						thostOrder.dwCloseOrderStart = GetTickCount();
						thostOrder.MOrderType = openorder_it->MOrderType;
						thostOrder.MAStop = false;
						strcpy(thostOrder.OpenTime, pOrderTradeMsg->InsertOrTradeTime);
						strcpy(thostOrder.SubmitDateAndTime, thostOrder.OpenTime);
						thostOrder.OrderLocalRetReqID = 0;
						thostOrder.OrderId = -1;
						thostOrder.OrderLocalRef = -1;
						thostOrder.CloseOrderSeqNo = ++mCloseOrderSeqNo;
						thostOrder.OpenOrderID = thisTradeRef;
						thostOrder.Offset = MORDER_CLOSE;
						thostOrder.OrderStatus = MORDER_ACCEPTED;
						thostOrder.Direction = MORDER_SELL;
						//
						thostOrder.LimitPrice = openorder_it->ProfitPrice;
						thostOrder.OrigSubmitPrice = thostOrder.LimitPrice;
						thostOrder.ProfitPrice = openorder_it->ProfitPrice;
						thostOrder.VolumeTotalOriginal = pOrderTradeMsg->Volume;
						thostOrder.VolumeTraded = 0;
						thostOrder.VolumeTotal = pOrderTradeMsg->Volume;//
						thostOrder.maxProfit = 0;
						thostOrder.ManualStopPrice = 0;
						thostOrder.mStoplossPoint = openorder_it->mStoplossPoint;
						thostOrder.OpenOrderID = thisTradeRef;

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
					}
					else if (openorder_it->Direction == MORDER_SELL) {
						tradedirection = MORDER_SELL;
						MyCloseOrderType thostOrder;//�����ƽ��ֹӯ��,δʵ���ύ��������,������ֹӯ�����ύֹӯ��
						thostOrder.OpenOrderSubmitPrice = openorder_it->OrigSubmitPrice;
						thostOrder.OpenOrderATR = openorder_it->OpenOrderATR;
						thostOrder.OpenOrderTradePrice = pOrderTradeMsg->Price;
						thostOrder.IsClosePofitOrder = true;
						thostOrder.IsStoplessOrder = false;
						thostOrder.CanbeCanceled = false;
						thostOrder.dwCloseOrderStart = GetTickCount();
						thostOrder.MOrderType = openorder_it->MOrderType;
						thostOrder.MAStop = false;
						strcpy(thostOrder.OpenTime, pOrderTradeMsg->InsertOrTradeTime);
						strcpy(thostOrder.SubmitDateAndTime, thostOrder.OpenTime);
						thostOrder.OrderLocalRetReqID = 0;
						thostOrder.OrderId = -1;
						thostOrder.OrderLocalRef = -1;
						thostOrder.CloseOrderSeqNo = ++mCloseOrderSeqNo;
						thostOrder.OpenOrderID = thisTradeRef;
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
						thostOrder.ManualStopPrice = 0;
						thostOrder.mStoplossPoint = openorder_it->mStoplossPoint;
						thostOrder.OpenOrderID = thisTradeRef;
						//strcpy(thostOrder.OpenTime,gEqualVolRatesX[gEqualVolRatesXIndex].datatime);
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
					}
					else {
						printf("openDirection Excetion!!! =%i\n", pOrderTradeMsg->Direction);
					}
					break;
				} //End if openorder_it->OrderSysId==thisTradeRef
			}
		}

		std::list<MyCloseOrderType>::iterator closeorder_it;
		int closeprofitornot = -1;
		int openid = -1;
		if (!CloseOrderList.empty()) {
			for (closeorder_it = CloseOrderList.begin(); closeorder_it != CloseOrderList.end(); closeorder_it++) {
				//std::list<MyCloseOrderType>::iterator iter_ec=closeorder_it++;
				if (closeorder_it->OrderId == thisTradeRef || (closeorder_it->OrderLocalRef == pOrderTradeMsg->OrderLocalRef && closeorder_it->OrderLocalRef > 0 && (ThostTraderAPI || SgitTraderAPI))) {
					//printf("CloseOrderList exist order \n");
					if (closeorder_it->OrderLocalRef == pOrderTradeMsg->OrderLocalRef && closeorder_it->OrderLocalRef > 0)closeorder_it->OrderId = thisTradeRef;

					openorclose = MORDER_CLOSE;
					submitprice = closeorder_it->OrigSubmitPrice;
					openid = closeorder_it->OpenOrderID;

					if (closeorder_it->Direction == MORDER_BUY) {
						tradedirection = MORDER_BUY;
						if (closeorder_it->OpenOrderTradePrice > pOrderTradeMsg->Price) {
							closeprofitornot = 1;
						}
						else closeprofitornot = 0;
					}
					else if (closeorder_it->Direction == MORDER_SELL) {
						tradedirection = MORDER_SELL;
						if (closeorder_it->OpenOrderTradePrice < pOrderTradeMsg->Price) {
							closeprofitornot = 1;
						}
						else closeprofitornot = 0;
					}

					closeorder_it->VolumeTraded = closeorder_it->VolumeTraded + pOrderTradeMsg->Volume;
					closeorder_it->VolumeTotal = closeorder_it->VolumeTotal - pOrderTradeMsg->Volume;

					OnHandPositionCount -= pOrderTradeMsg->Volume;
					InstanceOnHandPositionCount -= pOrderTradeMsg->Volume;
					TotalOnHandPosition -= pOrderTradeMsg->Volume;

					if (pOrderTradeMsg->Volume == closeorder_it->VolumeTotalOriginal) {
						CloseOrderList.erase(closeorder_it);

						map<int, int>::iterator iter;
						AcquireSRWLockExclusive(&g_srwLockOrderId);
						iter = OrderIdToShmIndex.find(pOrderTradeMsg->OrderSysId);
						if (iter != OrderIdToShmIndex.end()) {
							OrderIdToShmIndex.erase(iter);
						}
						ReleaseSRWLockExclusive(&g_srwLockOrderId);
					}
					else {
						closeorder_it->VolumeTraded = closeorder_it->VolumeTraded + pOrderTradeMsg->Volume;
						closeorder_it->VolumeTotal = closeorder_it->VolumeTotal - pOrderTradeMsg->Volume;
						if (closeorder_it->VolumeTotal == 0) {
							CloseOrderList.erase(closeorder_it);

							map<int, int>::iterator iter;
							AcquireSRWLockExclusive(&g_srwLockOrderId);
							iter = OrderIdToShmIndex.find(pOrderTradeMsg->OrderSysId);
							if (iter != OrderIdToShmIndex.end()) {
								OrderIdToShmIndex.erase(iter);
							}
							ReleaseSRWLockExclusive(&g_srwLockOrderId);
						}
					}
					break;
				} //End if closeorder_it->OrderSysId==thisTradeRef
			}//End While for Close Order List
		}
		//ѭ��ƽ�ֶ���
		if (tradedirection != -1) {
			TradeLogType trade;
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
			strcpy(trade.tradingday, strdatepart.c_str());
			strcpy(trade.tradingtime, strtimepart.c_str());
			strcpy(trade.CodeName, InstCodeName);
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

			Message logMsg;
			logMsg.type = TRADE_LOG;
			logMsg.AddData(&trade, 0, sizeof(TradeLogType));
			LogMessageList.AddTail(logMsg);

			ReleaseSemaphore(logSemaphore, 1, NULL);

			WaitForSingleObject(MatchNoMapSem, INFINITE);
			string matchno(pOrderTradeMsg->MatchNo);
			MatchNoToStrategyNameForDisplay.insert(std::pair<string, string>(matchno, mStrategyAndInstance));
			ReleaseSemaphore(MatchNoMapSem, 1, NULL);

			map<int, int>::iterator iter;
			AcquireSRWLockExclusive(&g_srwLockOrderId);
			iter = OrderIdToShmIndex.find(pOrderTradeMsg->OrderSysId);
			if (iter != OrderIdToShmIndex.end()) {
				OrderIdToShmIndex.erase(iter);
			}
			ReleaseSRWLockExclusive(&g_srwLockOrderId);

			DisplayTradeOnScreen(pOrderTradeMsg, tradedirection, openorclose, closeprofitornot);

			FlushStrategyInfoToFile();
		}
	}
}

void StrategyAvgLine::OnRspOrderInsert(ShmRspOrderInsert* pRspOrderInsert)
{
	int iRetReqID = pRspOrderInsert->iRetReqID;
	int OrderId = pRspOrderInsert->OrderID;
	TRACE("OnRspOrderInsert=%d,%d\n", iRetReqID, OrderId);
	if (!OpenOrderList.empty()) {
		std::list<MyOpenOrderType>::iterator openorder_it;
		for (openorder_it = OpenOrderList.begin(); openorder_it != OpenOrderList.end(); openorder_it++) {
			//std::list<MyOpenOrderType>::iterator iter_e=openorder_it++;
			//TRACE("OnRspOrderInsert,OpenOrderList,LocalRetReqID=%d,%d \n",openorder_it->OrderLocalRetReqID,iRetReqID);
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

void StrategyAvgLine::AddToRatesX(int tickCount, TickInfo* curTick, BarRateInfo* xBarFile, int* xBarIndex, int xBarTimeSize)
{
	int nYear, nMonth, nDate, nHour, nMin, nSec;
	int nXYear, nXMonth, nXDate, nXHour, nXMin, nXSec;
	double d_open, d_high, d_low, d_close, d_vol;
	time_t d_time;

	nYear = atoi(curTick->datadate) / 10000;
	nMonth = (atoi(curTick->datadate) % 10000) / 100;
	nDate = (atoi(curTick->datadate) % 10000) % 100;
	sscanf_s(curTick->updatetime, "%d:%d:%d", &nHour, &nMin, &nSec);

	CTime tm(nYear, nMonth, nDate, nHour, nMin, nSec);
	time_t curTime = tm.GetTime();
	//char timet[64];
	//strftime(timet, sizeof(timet), "%Y-%m-%d %X",localtime(&curTime));
	if (tickCount == 0) {
		//��ʼ����������
		d_open = curTick->price;
		d_high = curTick->price;
		d_low = curTick->price;
		d_close = curTick->price;
		d_time = curTime / xBarTimeSize;
		d_time = d_time * xBarTimeSize;
		//update to MX Bar Array
		mBarIndex = 0;
	}
	else {
		nXYear = atoi(xBarFile[*xBarIndex].datadate) / 10000;
		nXMonth = (atoi(xBarFile[*xBarIndex].datadate) % 10000) / 100;
		nXDate = (atoi(xBarFile[*xBarIndex].datadate) % 10000) % 100;
		sscanf_s(xBarFile[*xBarIndex].datatime, "%d:%d:%d", &nXHour, &nXMin, &nXSec);
		CTime tmX(nXYear, nXMonth, nXDate, nXHour, nXMin, nXSec);
		d_time = tmX.GetTime();

		if (curTime >= (d_time + xBarTimeSize)) {
			if (strcmp(curTick->datadate, xBarFile[*xBarIndex].datadate) != 0) {
				//New Date
				d_time = curTime / xBarTimeSize;
				d_time = d_time * xBarTimeSize;
			}
			else {
				d_time = curTime / xBarTimeSize;
				d_time = d_time * xBarTimeSize;
			}

			//New Bar
			d_open = curTick->price;
			d_high = curTick->price;
			d_low = curTick->price;
			d_close = curTick->price;

			(*xBarIndex)++;
		}
		else {
			if (curTick->price < xBarFile[*xBarIndex].low) xBarFile[*xBarIndex].low = curTick->price;
			if (curTick->price > xBarFile[*xBarIndex].high)xBarFile[*xBarIndex].high = curTick->price;
			xBarFile[*xBarIndex].close = curTick->price;
			return;
		}
	}
	char timet[10];
	strftime(timet, sizeof(timet), "%Y%m%d", localtime(&d_time));
	strcpy_s(xBarFile[*xBarIndex].datadate, 10, timet);
	strftime(timet, sizeof(timet), "%X", localtime(&d_time));
	strcpy_s(xBarFile[*xBarIndex].datatime, 10, timet);
	xBarFile[*xBarIndex].open = d_open;
	xBarFile[*xBarIndex].high = d_high;
	xBarFile[*xBarIndex].low = d_low;
	xBarFile[*xBarIndex].close = d_close;
}

void StrategyAvgLine::InitVariables()
{
	int nYearS, nMonthS, nDateS, nHourS, nMinS, nSecS;
	int nYearE, nMonthE, nDateE, nHourE, nMinE, nSecE;

	sscanf(mStrategyParams.StartTime, "%d-%d-%d %d:%d:%d", &nYearS, &nMonthS, &nDateS, &nHourS, &nMinS, &nSecS);
	sscanf(mStrategyParams.EndTime, "%d-%d-%d %d:%d:%d", &nYearE, &nMonthE, &nDateE, &nHourE, &nMinE, &nSecE);

	CTime tms(nYearS, nMonthS, nDateS, nHourS, nMinS, nSecS);
	tmt_StartTime = tms.GetTime();

	CTime tme(nYearE, nMonthE, nDateE, nHourE, nMinE, nSecE);
	tmt_EndTime = tme.GetTime();

	openBarIndex = 0;

	mTickCount = 0;
	lastmBarIndex = 0;
	m_fAvg = 0;
	//mTodayTickCount=0;
	//calcAvg(10,"QIJ_NYMEX_CL_ESUN_DAY");
}

void StrategyAvgLine::ReqOpenOrderInsert(MyOpenOrderType* pOpenOrder) {
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

void StrategyAvgLine::ReqCloseOrderInsert(MyCloseOrderType* pCloseOrder, char OpenOrderTime[21]) {
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
		string strOpenOrderTime(OpenOrderTime);
		string strOpenTradeingDay = strOpenOrderTime.substr(0, strOpenOrderTime.find_first_of(" "));
		bool CloseToday = false;
		if (ThostTraderAPI) {
			if (strcmp(strOpenTradeingDay.c_str(), CTPTradingDay) == 0) {
				CloseToday = true;
			}
		}
		else if (SgitTraderAPI) {
			if (strcmp(strOpenTradeingDay.c_str(), SgitTradingDay) == 0) {
				CloseToday = true;
			}
		}

		pCloseOrder->OrderId = -1;
		if (ThostTraderAPI)pThostTraderSpi->ReqOrderInsert(&cOrder, m_bCrossTradingDay, CloseToday, shmindex);
		else pSgitTraderSpi->ReqOrderInsert(&cOrder, m_bCrossTradingDay, CloseToday, shmindex);

		pCloseOrder->OrderLocalRef = cOrder.OrderLocalRef;
		pCloseOrder->FrontID = cOrder.FrontID;
		pCloseOrder->SessionID = cOrder.SessionID;

		CTime mCurrTime = CTime::GetCurrentTime();
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

void StrategyAvgLine::ReqOrderDelete(int pOrderId, int pOrderLocalRef, int pFrontID, int pSessionID)
{
	if (ThostTraderAPI || SgitTraderAPI) {
		if (ThostTraderAPI)pThostTraderSpi->ReqOrderDeletePerOrderLocalRef(pOrderLocalRef, m_InstID, m_ExchangeID, pFrontID, pSessionID);
		else pSgitTraderSpi->ReqOrderDeletePerOrderLocalRef(pOrderLocalRef, m_InstID, pFrontID, pSessionID);
	}
	else {
		pEsunTraderSpi->ReqOrderDelete(pOrderId);
	}

	char line[200];
	sprintf(line, "ReqOrderDelete,%d,%d", pOrderLocalRef, pOrderId);
	WriteMsgToLogList(line);
}

void StrategyAvgLine::WriteMsgToLogList(char logline[200])
{
	Message logMsg;
	logMsg.type = STRATEGY_LOG;
	logMsg.AddData(logline, 0, sizeof(char) * 200);
	LogMessageList.AddTail(logMsg);
	ReleaseSemaphore(logSemaphore, 1, NULL);
}

void StrategyAvgLine::InitAction()
{
	InitVariables();
	calcAvg(10, "QIJ_NYMEX_CL_ESUN_DAY");
}

void StrategyAvgLine::ResetAction()
{
	char line[200];
	sprintf(line, "%s,ResetAction", mStrategyAndInstance.c_str());
	WriteMsgToLogList(line);
	matchnomap.clear();

	//string strHstFileName(mStrategyName);
	//char xBarSize[20];
	//sprintf(xBarSize,"%d",mBarSize);
	//strHstFileName.append(xBarSize);
	//strHstFileName.append(".txt");
	string strmdfilename(mStrategyName);
	CTime tm = CTime::GetCurrentTime() - CTimeSpan(1, 0, 0, 0);
	char day[32] = { 0 };
	sprintf(day, "%04d%02d%02d", tm.GetYear(), tm.GetMonth(), tm.GetDay());
	char xBarSize[20];
	sprintf(xBarSize, "%d", mBarSize);
	strmdfilename.append("_");
	strmdfilename.append(InstCodeName);
	strmdfilename.append("_");
	strmdfilename.append(day);
	strmdfilename.append("_");
	strmdfilename.append(xBarSize);
	strmdfilename.append(".txt");
	FILE* fout = fopen(strmdfilename.c_str(), "w+");
	char mdline[200];
	if (mBarIndex > 1) {
		for (int i = 0; i <= mBarIndex; i++) {
			memset(mdline, 0, sizeof(mdline));
			sprintf(mdline, "%s,%s,%.4f,%.4f,%.4f,%.4f\n", mBarFile[i].datadate, mBarFile[i].datatime, mBarFile[i].open, mBarFile[i].high, mBarFile[i].low, mBarFile[i].close);
			fwrite(mdline, strlen(mdline), 1, fout);
		}
	}
	fclose(fout);

	ReleaseDataArray();
	InitDataArray();

	sprintf(line, "%s,ResetAction Done.", mStrategyAndInstance.c_str());
	WriteMsgToLogList(line);
}

void StrategyAvgLine::exitAction()
{
	string strmdfilename(mStrategyName);
	CTime tm = CTime::GetCurrentTime();
	char day[32] = { 0 };
	sprintf(day, "%04d%02d%02d", tm.GetYear(), tm.GetMonth(), tm.GetDay());
	char xBarSize[20];
	sprintf(xBarSize, "%d", mBarSize);
	strmdfilename.append("_");
	strmdfilename.append(InstCodeName);
	strmdfilename.append("_");
	strmdfilename.append(day);
	strmdfilename.append("_");
	strmdfilename.append(xBarSize);
	strmdfilename.append(".txt");
	FILE* fmddata = fopen(strmdfilename.c_str(), "w+");

	if (mBarIndex > 1) {
		for (int i = 0; i <= mBarIndex; i++) {
			//��¼��������
			char dataline[200];
			sprintf(dataline, "%s,%s,%.2f,%.2f,%.2f,%.2f\n", mBarFile[i].datadate, mBarFile[i].datatime, mBarFile[i].open, mBarFile[i].high, mBarFile[i].low, mBarFile[i].close);
			fwrite(dataline, strlen(dataline), 1, fmddata);
			fflush(fmddata);
		}
	}
	fflush(fmddata);
	fclose(fmddata);
}

void StrategyAvgLine::AddtoTipMsgListBox(char msgline[200])
{
	WaitForSingleObject(MainScreenFreshSem, INFINITE);
	CString str(msgline);
	pPubMsg->AddString(str);
	ReleaseSemaphore(MainScreenFreshSem, 1, NULL);
}

void StrategyAvgLine::DisplayTradeOnScreen(OrderTradeMsg* pOrderTradeMsg, int mDirection, int mOpenOrClose, int mCloseProfitOrNot)
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
		direction = _T("��");
	}
	else if (mDirection == MORDER_BUY) {
		direction = _T("��");
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
		csOpenOrClose = _T("��");
	}
	else if (mOpenOrClose == MORDER_CLOSE) {
		if (mCloseProfitOrNot == 1) {
			csOpenOrClose = _T("ֹӯƽ");
		}
		else csOpenOrClose = _T("ֹ��ƽ");
	}
	pTradesDetailsList->SetItemText(orderIndex, 7, (LPCTSTR)csOpenOrClose);

	pTradesDetailsList->EnsureVisible(pTradesDetailsList->GetItemCount() - 1, FALSE);
	ReleaseSemaphore(MainScreenFreshSem, 1, NULL);
}

bool StrategyAvgLine::timeRuleOK(char datatime[10])
{
	int nHour, nMin, nSec;
	sscanf(datatime, "%d:%d:%d", &nHour, &nMin, &nSec);
	if (5 == nHour)
		return false;
	return true;
}

void StrategyAvgLine::FlushStrategyInfoToFile() {
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
	memcpy(header.SpecificArea, &mOpenPrice, sizeof(double));
	memcpy(header.SpecificArea + sizeof(double), &InstanceOnHandPositionCount, sizeof(int));

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

void StrategyAvgLine::ReleaseDataArray() {
	if (mBarFile != NULL) {
		free(mBarFile);
		mBarFile = NULL;
	}
	mBarIndex = 0;
}

void StrategyAvgLine::InitDataArray() {
	int m_dallocBarNum = 10000;
	mBarFile = (BarRateInfo*)malloc(sizeof(BarRateInfo) * m_dallocBarNum);
	mBarIndex = 0;

	//InitVariables();
}

void StrategyAvgLine::SetParamValue(ParamNode node) {
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
			SgitTraderAPI = false;
			strcpy(mStrategyParams.InstCodeName, node.ParamValue);
			strcpy(InstCodeName, node.ParamValue);
		}

		strcpy(m_InstID, strInstID.c_str());
		strcpy(m_CommodityNo, strCommodityNo.c_str());
	}
	else if (strcmp(node.ParamName, "ProfitPoint") == 0) {
		mStrategyParams.ProfitPoint = atof(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "StoplossPoint") == 0) {
		mStrategyParams.StoplossPoint = atof(node.ParamValue);
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
	}/*else if(strcmp(node.ParamName,"CloseAllow")==0){
		if(atoi(node.ParamValue)>=1)mStrategyParams.CloseAllow=true;
		else mStrategyParams.CloseAllow=false;
	}*/else if (strcmp(node.ParamName, "OpenTime") == 0) {
		strcpy(mStrategyParams.OpenTime, node.ParamValue);
	}
	else if (strcmp(node.ParamName, "CloseTime") == 0) {
		strcpy(mStrategyParams.CloseTime, node.ParamValue);
	}
	else if (strcmp(node.ParamName, "LoopTimes") == 0) {
		mStrategyParams.LoopTimes = atoi(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "m_fX") == 0) {
		mStrategyParams.m_fX = atof(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "m_fY") == 0) {
		mStrategyParams.m_fY = atof(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "m_fTP") == 0) {
		mStrategyParams.m_fTP = atof(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "m_fTS") == 0) {
		mStrategyParams.m_fTS = atof(node.ParamValue);
	}

	//���ݿ�ʼ����ʱ������ί�е��Ƿ������Ч
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
			//��ʼʱ����03:00ǰ,��ǰ�����ս���ʱ��Ϊ�����03:00:00
			CTime endct(nYearTmp, nMonthTmp, nDateTmp, 3, 0, 0);
			time_t ct = endct.GetTime();
			CTime straendct(nYearEndTmp, nMonthEndTmp, nDateEndTmp, nHourEndTmp, nMinEndTmp, nSecEndTmp);
			time_t stract = straendct.GetTime();
			if (stract > ct) {
				//����
				m_bCrossTradingDay = true;
			}
			else {
				m_bCrossTradingDay = false;
			}
		}
		else {
			//��ʼʱ����03:00��,��ǰ�����ս���ʱ��Ϊ�ڶ����03:00:00
			CTime endct(nYearTmp, nMonthTmp, nDateTmp, 3, 0, 0);
			time_t ct = endct.GetTime() + 86400;

			CTime straendct(nYearEndTmp, nMonthEndTmp, nDateEndTmp, nHourEndTmp, nMinEndTmp, nSecEndTmp);
			time_t stract = straendct.GetTime();
			if (stract > ct) {
				//����
				m_bCrossTradingDay = true;
			}
			else {
				m_bCrossTradingDay = false;
			}
		}
	}
}

int StrategyAvgLine::CreateStrategyMapOfView() {
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

	// ����1 ���ļ�FILE_FLAG_WRITE_THROUGH
	HANDLE StrahFile = CreateFile(
		widstr2.c_str(),
		GENERIC_WRITE | GENERIC_READ,// ���Ҫӳ���ļ����˴�������Ϊֻ��(GENERIC_READ)���д
		FILE_SHARE_READ | FILE_SHARE_WRITE,//0,    // ����Ϊ���ļ����κγ��Ծ���ʧ��
		NULL,
		CREATE_ALWAYS,//OPEN_EXISTING,OPEN_ALWAYS,TRUNCATE_EXISTING,CREATE_ALWAYS
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, //|FILE_FLAG_WRITE_THROUGH,����1��
		NULL);
	if (StrahFile == INVALID_HANDLE_VALUE)// �ļ���ʧ�ܷ��ؾ��Ϊ-1
		// �ⲽ������ԣ���ϸ������2
	{
		return (-1);
	}

	// ����2 �����ڴ�ӳ���ļ�
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
		StrahFile, // �����ֵΪINVALID_HANDLE_VALUE,�ǺϷ��ģ��ϲ�һ�����԰�
		NULL,   // Ĭ�ϰ�ȫ��
		PAGE_READWRITE,   // �ɶ�д
		0, // 2��32λ��ʾ1��64λ��������ļ��ֽ�����
		// ���ֽڣ��ļ���СС��4Gʱ�����ֽ���ԶΪ0
		sizeof(mSerializeHeader) + (sizeof(MyCloseOrderType) + sizeof(MyOpenOrderType)) * 100,//dwFileSize, // ��Ϊ���ֽڣ�Ҳ��������Ҫ�Ĳ��������Ϊ0��ȡ�ļ���ʵ��С
		_T("XX" + FileMappingName + "Map" + csxmShmindex));
	if (StrategyInfoFileMap == NULL)
	{
		return (-1);
	}

	CloseHandle(StrahFile);    // �ر��ļ�

	// ����3�����ļ�����ӳ�䵽���̵ĵ�ַ�ռ�
	StrategyfilePoint = (char*)MapViewOfFile( //filePoint���ǵõ���ָ�룬������ֱ�Ӳ����ļ�
		StrategyInfoFileMap,
		FILE_MAP_WRITE,    // ��д
		0,     // �ļ�ָ��ͷλ�� ���ֽ�
		0, // �ļ�ָ��ͷλ�� ���ֽ� ��Ϊ�������ȵ�������,windows������Ϊ64K
		0);   // Ҫӳ����ļ�β�����Ϊ0�����ָ��ͷ����ʵ�ļ�β
	if (StrategyfilePoint == NULL)
	{
		return (-1);
	}

	// ����4: ������ڴ�һ�������ļ�
	memcpy(StrategyfilePoint, &header, sizeof(mSerializeHeader));
	FlushViewOfFile(StrategyfilePoint, sizeof(mSerializeHeader));

	return 0;
}

int StrategyAvgLine::OpenStrategyMapOfView() {
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

	// ����1 ���ļ�FILE_FLAG_WRITE_THROUGH
	HANDLE StrahFile = CreateFile(
		widstr2.c_str(),
		GENERIC_WRITE | GENERIC_READ,// ���Ҫӳ���ļ����˴�������Ϊֻ��(GENERIC_READ)���д
		FILE_SHARE_READ | FILE_SHARE_WRITE,//0,    // ����Ϊ���ļ����κγ��Ծ���ʧ��
		NULL,
		OPEN_EXISTING,//OPEN_EXISTING,OPEN_ALWAYS,TRUNCATE_EXISTING,CREATE_ALWAYS
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, //|FILE_FLAG_WRITE_THROUGH,����1��
		NULL);
	if (StrahFile == INVALID_HANDLE_VALUE)// �ļ���ʧ�ܷ��ؾ��Ϊ-1
		// �ⲽ������ԣ���ϸ������2
	{
		return (-1);
	}

	// ����2 �����ڴ�ӳ���ļ�
	//DWORD dwFileSize = GetFileSize(hFile, NULL);
	int xmShmindex = GetShmindex();
	CString csxmShmindex("");
	csxmShmindex.Format(_T("%d"), xmShmindex);

	CString FileMappingName(mStrategyAndInstance.c_str());
	StrategyInfoFileMap = CreateFileMapping(
		StrahFile, // �����ֵΪINVALID_HANDLE_VALUE,�ǺϷ��ģ��ϲ�һ�����԰�
		NULL,   // Ĭ�ϰ�ȫ��
		PAGE_READWRITE,   // �ɶ�д
		0, // 2��32λ��ʾ1��64λ��������ļ��ֽ�����
		// ���ֽڣ��ļ���СС��4Gʱ�����ֽ���ԶΪ0
		sizeof(mSerializeHeader) + (sizeof(MyCloseOrderType) + sizeof(MyOpenOrderType)) * 100,//dwFileSize, // ��Ϊ���ֽڣ�Ҳ��������Ҫ�Ĳ��������Ϊ0��ȡ�ļ���ʵ��С
		_T("XX" + FileMappingName + "Map" + csxmShmindex));
	if (StrategyInfoFileMap == NULL)
	{
		return (-1);
	}

	CloseHandle(StrahFile);    // �ر��ļ�

	// ����3�����ļ�����ӳ�䵽���̵ĵ�ַ�ռ�
	StrategyfilePoint = (char*)MapViewOfFile( //filePoint���ǵõ���ָ�룬������ֱ�Ӳ����ļ�
		StrategyInfoFileMap,
		FILE_MAP_WRITE,    // ��д
		0,     // �ļ�ָ��ͷλ�� ���ֽ�
		0, // �ļ�ָ��ͷλ�� ���ֽ� ��Ϊ�������ȵ�������,windows������Ϊ64K
		0);   // Ҫӳ����ļ�β�����Ϊ0�����ָ��ͷ����ʵ�ļ�β
	if (StrategyfilePoint == NULL)
	{
		return (-1);
	}

	return 0;
}

wstring StrategyAvgLine::s2ws(const string& s)
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

void StrategyAvgLine::SetInstanceName(char xInstanceName[50])
{
	strcpy(mInstanceName, xInstanceName);
	mStrategyAndInstance = mStrategyID;
	mStrategyAndInstance.append("_");
	mStrategyAndInstance.append(mInstanceName);
}

void StrategyAvgLine::GetInstanceName(char* xInstanceName) {
	strcpy(xInstanceName, mInstanceName);
}

void StrategyAvgLine::SetStrategyID(char strategyId[50])
{
	strcpy(mStrategyID, strategyId);
}

void StrategyAvgLine::calcAvg(int size, char* table)
{
	int iCount = 0;
	double sum = 0;
	// ���ݿ�ֻ��ȡ��9�����̼�
	CTime nDate = CTime::GetCurrentTime() - CTimeSpan(2, 0, 0, 0);
	while (iCount <= size - 2)
	{
		// �ж��Ƿ�Ϊ��ĩ
		if (nDate.GetDayOfWeek() != 1 && nDate.GetDayOfWeek() != 7)
		{
			char buff[32] = { 0 };
			sprintf(buff, "%04d%02d%02d", nDate.GetYear(), nDate.GetMonth(), nDate.GetDay());
			m_nDate[iCount++] = atoi(buff);
			nDate = nDate - CTimeSpan(1, 0, 0, 0);
		}
		else
			nDate = nDate - CTimeSpan(2, 0, 0, 0);
	}
	COracle* m_pDB = new COracle();
	char cnn[1024] = { 0 };
	bool m_bRet = m_pDB->ConnToDB("Provider=OraOLEDB.Oracle.1;Persist Security Info=True;Data Source=192.168.1.71:1521/orcl", "jrgc", "qjtz1234");
	if (m_bRet)
	{
		char sql[1024] = { 0 };
		sprintf(sql, "select CLOSE from %s t where (TRADINGDAY between %d and %d) and ORDERNAME='%s'", table, m_nDate[8], m_nDate[0], InstCodeName);
		_RecordsetPtr  record = m_pDB->ExecuteWithResSQL(sql);
		if (NULL != record)
		{
			while (!record->adoEOF)
			{
				_variant_t vClose;
				vClose = record->GetCollect("CLOSE");
				sum += atof((_bstr_t)vClose);
				record->MoveNext();
			}
		}
		delete m_pDB;
		m_pDB = NULL;
	}
	// ǰһ�����̼۴ӱ��ض�ȡ
	string strmdfilename(mStrategyName);
	CTime tm = CTime::GetCurrentTime() - CTimeSpan(1, 0, 0, 0);
	while (1 == tm.GetDayOfWeek() || 7 == tm.GetDayOfWeek())
		tm = tm - CTimeSpan(1, 0, 0, 0);

	char day[32] = { 0 };
	sprintf(day, "%04d%02d%02d", tm.GetYear(), tm.GetMonth(), tm.GetDay());
	char xBarSize[20];
	sprintf(xBarSize, "%d", mBarSize);
	strmdfilename.append("_");
	strmdfilename.append(InstCodeName);
	strmdfilename.append("_");
	strmdfilename.append(day);
	strmdfilename.append("_");
	strmdfilename.append(xBarSize);
	strmdfilename.append(".txt");
	FILE* fp = fopen(strmdfilename.c_str(), "r+");
	char buff[32] = { 0 };
	int i = 0;
	if (fp) {
		fseek(fp, -3L, SEEK_END);// �س�ԭ��
		char ch = fgetc(fp);
		while (ch != ',') {
			fseek(fp, -2L, SEEK_CUR);
			ch = fgetc(fp);
			i++;
		}
		fread(buff, i, 1, fp);
		iCount++;
		sum += atof(buff);
		fclose(fp);
	}
	if (iCount != 0 && iCount == size) {
		m_fAvg = sum / iCount;
		m_fAvg = floor((m_fAvg / m_dOneTick) + 0.00001) * m_dOneTick;
	}
	char line[200];
	sprintf(line, "%s,calcAvg,begindate:%d,enddate:%s,avg:%.4f,day:%d", mStrategyAndInstance.c_str(), m_nDate[8], day, m_fAvg, iCount);
	WriteMsgToLogList(line);
}

void StrategyAvgLine::CloseMap()
{
	//ɾ����Ӧ��cfg�ļ�
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

bool StrategyAvgLine::timeRuleForOpen(char datadate[10], char datatime[10]) {
	//int nYear, nMonth, nDate;
	//nYear=atoi(datadate)/10000;
	//nMonth=(atoi(datadate)%10000)/100;
	//nDate=(atoi(datadate)%10000)%100;
	//int nHour,nMin,nSec;
	//sscanf(datatime, "%d:%d:%d",&nHour, &nMin, &nSec);
	//CTime tm(nYear, nMonth, nDate,nHour,nMin,nSec);
	//time_t curTime=tm.GetTime();
	//if(curTime>tmt_StartTime) return true;
	//return false;
	int nSecond = timeToSecond(datatime);
	if (nSecond >= 21 * 3600 || nSecond >= 0 && nSecond <= 2 * 3600 + 45 * 60) return true;
	return false;
}

bool StrategyAvgLine::timeRuleForClose(char datadate[10], char datatime[10])
{
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

	if (closeTime - currTime > 0 && closeTime - currTime <= 600) return true;

	return false;
}

int StrategyAvgLine::timeToSecond(char datetime[10]) {
	int nHour, nMin, nSec;
	sscanf(datetime, "%d:%d:%d", &nHour, &nMin, &nSec);
	return nHour * 3600 + nMin * 60 + nSec;
}