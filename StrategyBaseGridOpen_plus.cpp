#include "StdAfx.h"
#include "StrategyBaseGridOpen_plus.h"
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

using namespace std;

int StrategyBaseGridOpen_plus::MaxOnHandPositionCount=0;
int StrategyBaseGridOpen_plus::OnHandPositionCount=0;

extern CListBox* pPubMsg;

extern EsunTraderSpi*	pEsunTraderSpi;
extern ThostTraderSpi*	pThostTraderSpi;
extern SgitTraderSpi*	pSgitTraderSpi;

extern MessageList ScreenDisplayMsgList;
extern MessageList LogMessageList;
extern OrderDataList OrderList;

extern int iNextOrderRef;    //��һ������
extern char CTPTradingDay[];
extern int MaxTotalCancelTimes;

//extern LONGLONG nStart; //΢��
//extern DWORD nMillSecStart;//����
//extern LARGE_INTEGER liQPF;
//extern double g_PF_us;
//extern double g_spread;
extern TickDataList TickList;
extern map<int,int> OrderIdToShmIndex;
extern map<int,string> OrderIdToStrategyNameForDisplay;
extern map<string,string> MatchNoToStrategyNameForDisplay;
extern map<int,int> ReqIdToShmIndex;
extern HANDLE logSemaphore;
extern HANDLE MatchNoMapSem;
extern SRWLOCK  g_srwLockReqId;
extern SRWLOCK  g_srwLockOrderId;
extern int TotalOnHandPosition; //ԭ�Ӳ���,�ʲ�����
extern int MaxTotalOnHandPosition;
extern int TotalCancelTimes; //ԭ�Ӳ���,�ʲ�����
extern int MaxTotalCancelTimes;
extern MessageList ScreenDisplayMsgList;
extern HANDLE ScreenDisplaySem;
extern HANDLE MainScreenFreshSem;
extern HANDLE OrderInsertSem;
extern CListCtrl *pTradesDetailsList;
extern list<ModelNode> ModelList;
extern bool ClearInstanceCfgFile;
//For Thost Trader API
extern CLockVariable gLockVariable;
extern SRWLOCK g_srwLockOrderLocalRef;
extern map<int,int> OrderLocalRefToShmIndex;


StrategyBaseGridOpen_plus::StrategyBaseGridOpen_plus(char InstrumentID[30])
{
	iNextOrderRef=1;
	mTickCount=0;
	m_dOneTick=0.01;
	m_bSampleReady=false;	
	mOpenTimes=0;

	m_bIsRunning=true;
	m_bCrossTradingDay=false;

	mClosePrice=0;
	mOpenPrice=0;

	memset(&mStrategyParams,0,sizeof(mParasType));
	memset(GridCloseOrderCount,0,sizeof(GridCloseOrderCount));

	m_bStoplossClose=false;

	strcpy(mStrategyName,"BaseGridOpen_plus");
	strcpy(mInstanceName,"");
	WriteMsgToLogList("Strategy Init..");
	memset(curTradingDay,0,sizeof(curTradingDay));
}


StrategyBaseGridOpen_plus::~StrategyBaseGridOpen_plus(void)
{
}

int StrategyBaseGridOpen_plus::GetShmindex()
{
	return shmindex;
}

void StrategyBaseGridOpen_plus::SetShmindex(int xshmindex)
{
	shmindex=xshmindex;
}

void StrategyBaseGridOpen_plus::SetInstanceName(char xInstanceName[50])
{
	strcpy(mInstanceName,xInstanceName);
	mStrategyAndInstance=mStrategyID;
	mStrategyAndInstance.append("_");
	mStrategyAndInstance.append(mInstanceName);
}

void StrategyBaseGridOpen_plus::GetInstanceName(char *xInstanceName){
	strcpy(xInstanceName,mInstanceName);
}

void StrategyBaseGridOpen_plus::SetParamValue(ParamNode node){
	bool find=false;
	std::list<ParamNode>::iterator param_it;
	if(!mParamslist.empty()){
		for(param_it = mParamslist.begin(); param_it != mParamslist.end(); ++param_it){
			if(strcmp(param_it->ParamName,node.ParamName)==0){find=true;break;}
		}
	}
	if(!find){
		mParamslist.push_back(node);
	}

	if(strcmp(node.ParamName,"InstCodeName")==0){
		string strInstCodeName(node.ParamValue);
		string strExchangeID=strInstCodeName.substr(0,strInstCodeName.find_first_of(" "));
		string strCommodityNo=strInstCodeName.substr(strInstCodeName.find_first_of(" ")+1,strInstCodeName.find_last_of(" ")-strInstCodeName.find_first_of(" ")-1); 
		string strInstID=strInstCodeName.substr(strInstCodeName.find_last_of(" ")+1,strInstCodeName.length()-strInstCodeName.find_last_of(" ")); 

		if(strcmp(strExchangeID.c_str(),"CFFEX")==0||strcmp(strExchangeID.c_str(),"SHFE")==0
			||strcmp(strExchangeID.c_str(),"DCE")==0||strcmp(strExchangeID.c_str(),"CZCE")==0||strcmp(strExchangeID.c_str(),"SGE")==0
			||strcmp(strExchangeID.c_str(),"INE")==0){
				if(strcmp(strExchangeID.c_str(),"SGE")==0){
					SgitTraderAPI=true;
				}else{
					ThostTraderAPI=true;
				}
				strcpy(mStrategyParams.InstCodeName,strInstID.c_str());
				strcpy(InstCodeName,strInstID.c_str());
		}else {
			ThostTraderAPI=false;
			strcpy(mStrategyParams.InstCodeName,node.ParamValue);
			strcpy(InstCodeName,node.ParamValue);
		}

		strcpy(m_InstID,strInstID.c_str());
		strcpy(m_CommodityNo,strCommodityNo.c_str());
		strcpy(m_ExchangeID,strExchangeID.c_str());
		strcpy(m_InstCodeName,strInstCodeName.c_str());
	}else if(strcmp(node.ParamName,"UpGridCount")==0){
		mStrategyParams.UpGridCount=atoi(node.ParamValue);
	}else if(strcmp(node.ParamName,"UpGridDistPoint")==0){
		mStrategyParams.UpGridDistPoint=atoi(node.ParamValue);
	}else if(strcmp(node.ParamName,"DnGridCount")==0){
		mStrategyParams.DnGridCount=atoi(node.ParamValue);
	}else if(strcmp(node.ParamName,"DnGridDistPoint")==0){
		mStrategyParams.DnGridDistPoint=atoi(node.ParamValue);
	}else if(strcmp(node.ParamName,"ProfitPoint")==0){
		mStrategyParams.ProfitPoint=atoi(node.ParamValue);
	}else if(strcmp(node.ParamName,"StoplossPoint")==0){
		mStrategyParams.StoplossPoint=atoi(node.ParamValue);
	}else if(strcmp(node.ParamName,"GridVol")==0){
		mStrategyParams.GridVol=atoi(node.ParamValue);
	}else if(strcmp(node.ParamName,"BasePrice")==0){
		mStrategyParams.BasePrice=atof(node.ParamValue);
	}else if (strcmp(node.ParamName, "OffFlagPoint") == 0) {
		mStrategyParams.OffFlagPoint = atof(node.ParamValue);
	}else if(strcmp(node.ParamName,"StartTime")==0){
		strcpy(mStrategyParams.StartTime,node.ParamValue);
	}else if(strcmp(node.ParamName,"EndTime")==0){
		strcpy(mStrategyParams.EndTime,node.ParamValue);
	}else if(strcmp(node.ParamName,"OpenTime")==0){
		strcpy(mStrategyParams.OpenTime,node.ParamValue);
	}else if(strcmp(node.ParamName,"CloseTime")==0){
		strcpy(mStrategyParams.CloseTime,node.ParamValue);
	}else if(strcmp(node.ParamName,"OpenBuyAllow")==0){
		if(atoi(node.ParamValue)>=1)mStrategyParams.OpenBuyAllow=true;
		else mStrategyParams.OpenBuyAllow=false;
	}else if(strcmp(node.ParamName,"OpenSellAllow")==0){
		if(atoi(node.ParamValue)>=1)mStrategyParams.OpenSellAllow=true;
		else mStrategyParams.OpenSellAllow=false;
	}else if(strcmp(node.ParamName,"IsBasePrice")==0){
		if(atoi(node.ParamValue)>=1)mStrategyParams.IsBasePrice=true;
		else mStrategyParams.IsBasePrice=false;
	}else if(strcmp(node.ParamName,"LoopTimes")==0){
		mStrategyParams.LoopTimes=atoi(node.ParamValue);
	}

	//���ݿ�ʼ����ʱ������ί�е��Ƿ������Ч
	if(strcmp(mStrategyParams.StartTime,"")>0&&strcmp(mStrategyParams.EndTime,"")>0){
		string strStart(mStrategyParams.StartTime);
		string strEnd(mStrategyParams.EndTime);
		string timepart=strStart.substr(strStart.find_first_of(" ")+1,strStart.length()-strStart.find_first_of(" "));
		string datepart=strStart.substr(0,strStart.find_first_of(" "));
		int nYearTmp=0,nMonthTmp=0,nDateTmp=0,nHourTmp=0,nMinTmp=0,nSecTmp=0;
		int nYearEndTmp=0,nMonthEndTmp=0,nDateEndTmp=0,nHourEndTmp=0,nMinEndTmp=0,nSecEndTmp=0;
		sscanf_s(strStart.c_str(),"%d-%d-%d %d:%d:%d",&nYearTmp,&nMonthTmp,&nDateTmp,&nHourTmp,&nMinTmp,&nSecTmp);
		sscanf_s(strEnd.c_str(),"%d-%d-%d %d:%d:%d",&nYearEndTmp,&nMonthEndTmp,&nDateEndTmp,&nHourEndTmp,&nMinEndTmp,&nSecEndTmp);
		if(nHourTmp<3){
			//��ʼʱ����03:00ǰ,��ǰ�����ս���ʱ��Ϊ�����03:00:00
			CTime endct(nYearTmp, nMonthTmp, nDateTmp,3,0,0);	
			time_t ct=endct.GetTime();
			CTime straendct(nYearEndTmp, nMonthEndTmp, nDateEndTmp,nHourEndTmp,nMinEndTmp,nSecEndTmp);	
			time_t stract=straendct.GetTime();
			if(stract>ct){
				//����
				m_bCrossTradingDay=true;
			}else{
				m_bCrossTradingDay=false;
			}
		}else{
			//��ʼʱ����03:00��,��ǰ�����ս���ʱ��Ϊ�ڶ����03:00:00
			CTime endct(nYearTmp, nMonthTmp, nDateTmp,3,0,0);	
			time_t ct=endct.GetTime()+86400;

			CTime straendct(nYearEndTmp, nMonthEndTmp, nDateEndTmp,nHourEndTmp,nMinEndTmp,nSecEndTmp);	
			time_t stract=straendct.GetTime();
			if(stract>ct){
				//����
				m_bCrossTradingDay=true;
			}else{
				m_bCrossTradingDay=false;
			}
		}

	}
}

void StrategyBaseGridOpen_plus::InitVariables(){
	int nYearS,nMonthS,nDateS,nHourS,nMinS,nSecS;
	int nYearE,nMonthE,nDateE,nHourE,nMinE,nSecE;

	sscanf(mStrategyParams.StartTime, "%d-%d-%d %d:%d:%d",&nYearS,&nMonthS,&nDateS,&nHourS, &nMinS, &nSecS);	 	
	sscanf(mStrategyParams.EndTime, "%d-%d-%d %d:%d:%d",&nYearE,&nMonthE,&nDateE,&nHourE, &nMinE, &nSecE);

	CTime tms(nYearS, nMonthS, nDateS,nHourS,nMinS,nSecS);
	tmt_StartTime=tms.GetTime();

	CTime tme(nYearE, nMonthE, nDateE,nHourE,nMinE,nSecE);
	tmt_EndTime=tme.GetTime();

	mTickCount=0;
	string strInstCodeName(mStrategyParams.InstCodeName);

	m_bSampleReady=false;	
	mOpenTimes=0;
	mOpenRet=false;
}
void StrategyBaseGridOpen_plus::ResetStrategy(){

}

void StrategyBaseGridOpen_plus::OnRtnDepthMarketData(TickInfo *pDepthMarketData)
{
	///��ȡ����۸����õ�ǰ���صĵ�ǰ�۸�
	m_Price = pDepthMarketData->price;
	m_Buy1 =pDepthMarketData->bid1;// m_Price;//
	m_Sell1 =pDepthMarketData->ask1;	//m_Buy1+1.0;//

	int nYear_m, nMonth_m, nDate_m;
	nYear_m=atoi(pDepthMarketData->datadate)/10000;
	nMonth_m=(atoi(pDepthMarketData->datadate)%10000)/100;
	nDate_m=(atoi(pDepthMarketData->datadate)%10000)%100;
	int  nHour_m, nMin_m, nSec_m;		
	sscanf(pDepthMarketData->updatetime, "%d:%d:%d",&nHour_m, &nMin_m, &nSec_m);

	CTime tm_m(nYear_m, nMonth_m, nDate_m,nHour_m,nMin_m,nSec_m);
	curTickTime=tm_m.GetTime();

	// ��ȡ���̼�
	//if(timeRuleForClosePrice(pDepthMarketData->datadate,pDepthMarketData->updatetime)){
	//	mClosePrice=pDepthMarketData->price;
	//}
	//if(!mStrategyParams.IsBasePrice){
	//	if(CloseOrderList.empty()&&0!=strcmp(curTradingDay,CTPTradingDay)&&0!=strlen(curTradingDay)){
	//		mStrategyParams.BasePrice=mClosePrice;
	//	}
	//}

	// ��ȡ���̼�
	int nOpenHour,nOpenMin,nOpenSec;
	sscanf_s(mStrategyParams.OpenTime,"%d:%d:%d",&nOpenHour,&nOpenMin,&nOpenSec);
	if (nHour_m==nOpenHour&&nMin_m>=nOpenMin&&!mOpenRet){
		mOpenPrice=pDepthMarketData->price;
		mOpenRet=true;
	}

	if (!mStrategyParams.IsBasePrice) {
		if (CloseOrderList.empty()) 
		{
			std::list<MyOpenOrderType>::iterator openorder_it;
			for (openorder_it = OpenOrderList.begin(); openorder_it != OpenOrderList.end(); ++openorder_it) {
				if (openorder_it->VolumeTotal != 0 && openorder_it->OpenOrderCanbeCanceled) {
					//ȫ��ֹӯδ�ɽ��Ŀ��ֵ�����
					char line[200];
					sprintf(line, "%s,%s,AllProfit,CancelOpen,Id=%d,ref=%d", pDepthMarketData->datadate, pDepthMarketData->updatetime, openorder_it->OrderId, openorder_it->OrderLocalRef);
					WriteMsgToLogList(line);

					ReqOrderDelete(openorder_it->OrderId, openorder_it->OrderLocalRef, openorder_it->FrontID, openorder_it->SessionID);
					openorder_it->OpenOrderCanbeCanceled = false;
				}
			}//end for Opened loop
			if (mStrategyParams.OpenBuyAllow)
			{
				mStrategyParams.BasePrice = pDepthMarketData->price + mStrategyParams.DnGridDistPoint * int(mStrategyParams.DnGridCount/2) * m_dOneTick;
			}
			else if (mStrategyParams.OpenSellAllow)
			{
				mStrategyParams.BasePrice = pDepthMarketData->price - mStrategyParams.UpGridDistPoint * int(mStrategyParams.UpGridCount / 2) * m_dOneTick;;
			}

		}
	}

	int nSecond=GetShmindex();
	/*if(curTickTime>tmt_StartTime){
		mTickCount++;
	}

	if(!m_bSampleReady){
		if(mTickCount>3){
			m_bSampleReady=true;
		} 
	}*/
	
	//Processing Opened Order	
	std::list<MyOpenOrderType>::iterator openorder_it;
	for(openorder_it = OpenOrderList.begin(); openorder_it != OpenOrderList.end(); ++openorder_it){
		if(openorder_it->VolumeTotal!=0&&openorder_it->OpenOrderCanbeCanceled&&timeRuleForCancel(pDepthMarketData->datadate,pDepthMarketData->updatetime)){				
			//����ʱ���ѵ�,δ�ɽ��Ŀ��ֵ�����
			char line[200];
			sprintf(line,"%s,%s,TimeEnd,CancelOpen,Id=%d,ref=%d",pDepthMarketData->datadate,pDepthMarketData->updatetime,openorder_it->OrderId,openorder_it->OrderLocalRef);
			WriteMsgToLogList(line);

			ReqOrderDelete(openorder_it->OrderId,openorder_it->OrderLocalRef,openorder_it->FrontID,openorder_it->SessionID);
			openorder_it->OpenOrderCanbeCanceled=false;
		}
	}//end for Opened loop
	//Processing Closed Order
	std::list<MyCloseOrderType>::iterator closeorder_it2;
	for(closeorder_it2 = CloseOrderList.begin(); closeorder_it2 != CloseOrderList.end(); ++closeorder_it2){
		if((closeorder_it2->OrderStatus==MORDER_PART_TRADED||closeorder_it2->OrderStatus==MORDER_ACCEPTED||closeorder_it2->OrderStatus==MORDER_QUEUED)
			&&timeRuleForCancel(pDepthMarketData->datadate,pDepthMarketData->updatetime)
			&&closeorder_it2->CanbeCanceled){				
			//����ʱ���ѵ�,δ�ɽ���ƽ�ֵ�����
			char line[200];
			sprintf(line,"%s,%s,TimeEnd,CancelClose,Id=%d,ref=%d",pDepthMarketData->datadate,pDepthMarketData->updatetime,closeorder_it2->OrderId,closeorder_it2->OrderLocalRef);
			WriteMsgToLogList(line);

			ReqOrderDelete(closeorder_it2->OrderId,closeorder_it2->OrderLocalRef,closeorder_it2->FrontID,closeorder_it2->SessionID);
			closeorder_it2->CanbeCanceled=false;
			
		}
	}//end for Closed loop
	//End for Opened Order processing

	//std::list<MyCloseOrderType>::iterator closeorder_it;
	//if(!CloseOrderList.empty()){
	//	//Loop Close Order List
	//	int CloseOrderCount=0;
	//	for(closeorder_it = CloseOrderList.begin(); closeorder_it != CloseOrderList.end();++closeorder_it){			
	//		if( closeorder_it->Direction==MORDER_SELL
	//			&&(closeorder_it->OrderStatus==MORDER_PART_TRADED||closeorder_it->OrderStatus==MORDER_ACCEPTED||closeorder_it->OrderStatus==MORDER_QUEUED)
	//			&&(mStrategyParams.BackPerc-0.000001>0)){
	//				//Closing the buy order
	//				closeorder_it->maxProfit=max(closeorder_it->maxProfit,m_Price-closeorder_it->OpenOrderTradePrice);
	//				if(closeorder_it->maxProfit-0.00001>=mStrategyParams.ProfitPoint*m_dOneTick) closeorder_it->MAStop=true;
	//				if(closeorder_it->IsClosePofitOrder&&m_Price<(closeorder_it->OpenOrderTradePrice+closeorder_it->maxProfit*(1-mStrategyParams.BackPerc)+0.00001)
	//					&&closeorder_it->CanbeCanceled&&closeorder_it->MAStop){
	//					char line[200];
	//					sprintf(line,"%s,Buy Order Stoploss Close,OrderId=%d",mStrategyAndInstance.c_str(),closeorder_it->OrderId);
	//					WriteMsgToLogList(line);

	//					closeorder_it->NextCloseOrderPrice=m_Sell1;
	//					ReqOrderDelete(closeorder_it->OrderId,closeorder_it->OrderLocalRef,closeorder_it->FrontID,closeorder_it->SessionID);
	//					closeorder_it->CanbeCanceled=false;
	//						
	//				}/*else if( timeRuleForClose(pDepthMarketData->datadate,pDepthMarketData->updatetime)
	//					&&iter_e->IsClosePofitOrder){
	//						char line[200];
	//						sprintf(line,"%s,Buy Order timeRuleForClose Close,OrderId=%d",mStrategyAndInstance.c_str(),iter_e->OrderId);
	//						WriteMsgToLogList(line);

	//						iter_e->NextCloseOrderPrice=m_Sell1-m_dOneTick;
	//						ReqOrderDelete(iter_e->OrderId,iter_e->OrderLocalRef,iter_e->FrontID,iter_e->SessionID);

	//				}*/else if((closeorder_it->LimitPrice-0.00001)>m_Sell1&&closeorder_it->IsStoplessOrder&&closeorder_it->CanbeCanceled){
	//					char line[200];
	//					sprintf(line,"%s,Buy Order Close ReSubmit,OrderId=%d",mStrategyAndInstance.c_str(),closeorder_it->OrderId);
	//					WriteMsgToLogList(line);

	//					//TRACE("Cancel the stopless order for buy order,stopless price=%.1f,sell1=%.1f\n",closeorder_it->LimitPrice,m_Sell1);
	//					closeorder_it->NextCloseOrderPrice=m_Buy1;
	//					ReqOrderDelete(closeorder_it->OrderId,closeorder_it->OrderLocalRef,closeorder_it->FrontID,closeorder_it->SessionID);
	//					closeorder_it->CanbeCanceled=false;
	//				}

	//		}//End IF Closing buy order

	//		if( closeorder_it->Direction==MORDER_BUY
	//			&&(closeorder_it->OrderStatus==MORDER_PART_TRADED||closeorder_it->OrderStatus==MORDER_ACCEPTED||closeorder_it->OrderStatus==MORDER_QUEUED)
	//			&&(mStrategyParams.BackPerc-0.000001>0)){
	//				//Closing sell order
	//				closeorder_it->maxProfit=max(closeorder_it->maxProfit,closeorder_it->OpenOrderTradePrice-m_Price);
	//				if(closeorder_it->maxProfit-0.00001>=mStrategyParams.ProfitPoint*m_dOneTick) closeorder_it->MAStop=true;

	//				if( closeorder_it->IsClosePofitOrder&&m_Price<(closeorder_it->OpenOrderTradePrice-closeorder_it->maxProfit*(1-mStrategyParams.BackPerc)+0.00001)
	//					&&closeorder_it->CanbeCanceled&&closeorder_it->MAStop){
	//						char line[200];
	//						sprintf(line,"%s,Sell Order Stoploss Close,OrderId=%d",mStrategyAndInstance.c_str(),closeorder_it->OrderId);
	//						WriteMsgToLogList(line);

	//						closeorder_it->NextCloseOrderPrice=m_Buy1;
	//						ReqOrderDelete(closeorder_it->OrderId,closeorder_it->OrderLocalRef,closeorder_it->FrontID,closeorder_it->SessionID);							
	//						closeorder_it->CanbeCanceled=false;

	//				}/*else if( timeRuleForClose(pDepthMarketData->datadate,pDepthMarketData->updatetime)
	//					&&iter_e->IsClosePofitOrder){
	//						char line[200];
	//						sprintf(line,"%s,Sell Order timeRuleForClose Close,OrderId=%d",mStrategyAndInstance.c_str(),iter_e->OrderId);
	//						WriteMsgToLogList(line);

	//						iter_e->NextCloseOrderPrice=m_Buy1+m_dOneTick;
	//						ReqOrderDelete(iter_e->OrderId,iter_e->OrderLocalRef,iter_e->FrontID,iter_e->SessionID);
	//				}*/else if(closeorder_it->LimitPrice<(m_Buy1-0.00001)&&closeorder_it->IsStoplessOrder&&closeorder_it->CanbeCanceled){
	//					char line[200];
	//					sprintf(line,"%s,Sell Order Close ReSubmit,OrderId=%d",mStrategyAndInstance.c_str(),closeorder_it->OrderId);
	//					WriteMsgToLogList(line);
	//					closeorder_it->NextCloseOrderPrice=m_Sell1;
	//					ReqOrderDelete(closeorder_it->OrderId,closeorder_it->OrderLocalRef,closeorder_it->FrontID,closeorder_it->SessionID);
	//					closeorder_it->CanbeCanceled=false;							
	//				}

	//		}//End IF Closing sell order
	//		//}//End for Strategy 1

	//	}//End Looping close order list
	//}//End if the close order list is not null

	bool bOpenFlag=false;
	if(0==strcmp(m_ExchangeID,"CFFEX")){
		bOpenFlag=timeRuleForOpenNew(pDepthMarketData->datadate,pDepthMarketData->updatetime);
	}else{
		bOpenFlag=timeRuleForOpen(pDepthMarketData->datadate,pDepthMarketData->updatetime);
	}

	if(!CloseOrderList.empty()&&bOpenFlag&&nSec_m>nSecond){
		int CloseOrderCount=0;
		std::list<MyCloseOrderType>::iterator closeorder_it;
		for(closeorder_it = CloseOrderList.begin(); closeorder_it != CloseOrderList.end();){
			std::list<MyCloseOrderType>::iterator iter_e=closeorder_it++;

				if(MORDER_WAITFORSUBMIT==iter_e->OrderStatus){
					MyCloseOrderType newThostOrder;
					iNextOrderRef++;
					newThostOrder.OrderLocalRetReqID=0;
					newThostOrder.OrderId=-1;
					newThostOrder.OrderLocalRef=-1;
					newThostOrder.OpenOrderID=iter_e->OpenOrderID;
					strcpy(newThostOrder.OpenTime,iter_e->OpenTime);
					newThostOrder.IsStoplessOrder=false;
					newThostOrder.MAStop=iter_e->MAStop;
					newThostOrder.CanbeCanceled=true;
					newThostOrder.IsClosePofitOrder=true;
					newThostOrder.ManualStopPrice=iter_e->ManualStopPrice;
					newThostOrder.OpenOrderTradePrice=iter_e->OpenOrderTradePrice;
					newThostOrder.MOrderType=iter_e->MOrderType;
					newThostOrder.Direction=iter_e->Direction;
					newThostOrder.LimitPrice=iter_e->LimitPrice;
					if(MORDER_BUY==newThostOrder.Direction){
						if((newThostOrder.LimitPrice-0.00001)<pDepthMarketData->lowerLimitPrice) continue;
					}else if(MORDER_SELL==newThostOrder.Direction){
						if((newThostOrder.LimitPrice+0.00001)>pDepthMarketData->upperLimitPrice) continue;
					}
					newThostOrder.ProfitPrice=iter_e->ProfitPrice;
					newThostOrder.OrigSubmitPrice=iter_e->OrigSubmitPrice;
					newThostOrder.Offset=iter_e->Offset;
					newThostOrder.VolumeTotalOriginal=iter_e->VolumeTotalOriginal;
					newThostOrder.VolumeTotal=newThostOrder.VolumeTotalOriginal;
					newThostOrder.VolumeTraded=0;
					newThostOrder.maxProfit=iter_e->maxProfit;
					newThostOrder.mStoplossPoint=iter_e->mStoplossPoint;
					

					ReqCloseOrderInsert(&newThostOrder,newThostOrder.OpenTime); 

					CloseOrderList.erase(iter_e);
					CloseOrderCount++;
					if(CloseOrderCount>=4) break;
				}

		}
	}
	
	bool BuyOrderNeedStopExist = false;
	bool SellOrderNeedStopExist = false;
	std::list<MyCloseOrderType>::iterator closeorder_it;
	if (!CloseOrderList.empty()) {
		int CloseOrderCount = 0;
		//Loop Close Order List
		for (closeorder_it = CloseOrderList.begin(); closeorder_it != CloseOrderList.end(); closeorder_it++) {
			//std::list<MyCloseOrderType>::iterator iter_e=closeorder_it++;
			if (closeorder_it->Direction == MORDER_SELL &&
				(closeorder_it->OrderStatus == MORDER_PART_TRADED || closeorder_it->OrderStatus == MORDER_ACCEPTED || closeorder_it->OrderStatus == MORDER_QUEUED)) {
				if (closeorder_it->CanbeCanceled && closeorder_it->IsClosePofitOrder && closeorder_it->MOrderType == -1) {
					char line[200];
					sprintf(line, "%s,Buy Order Stoploss Close,%.5f,OrderId=%d", mStrategyAndInstance.c_str(), m_Price, closeorder_it->OrderId);
					WriteMsgToLogList(line);
					closeorder_it->NextCloseOrderPrice = m_Sell1 - m_dOneTick;
					ReqOrderDelete(closeorder_it->OrderId, closeorder_it->OrderLocalRef, closeorder_it->FrontID, closeorder_it->SessionID);
					closeorder_it->CanbeCanceled = false;
					m_bStoplossClose = true;

				}
				else if ((closeorder_it->LimitPrice - 0.00001) > m_Sell1&& closeorder_it->IsStoplessOrder&& closeorder_it->CanbeCanceled) {
					char line[200];
					sprintf(line, "%s,Buy Order Close ReSubmit,OrderId=%d", mStrategyAndInstance.c_str(), closeorder_it->OrderId);
					WriteMsgToLogList(line);
					closeorder_it->NextCloseOrderPrice = m_Buy1;
					ReqOrderDelete(closeorder_it->OrderId, closeorder_it->OrderLocalRef, closeorder_it->FrontID, closeorder_it->SessionID);
					closeorder_it->CanbeCanceled = false;
				}
				if (closeorder_it->MOrderType == -1)
				{
					BuyOrderNeedStopExist = true;
					//mStrategyParams.OpenBuyAllow = 0;
				}

			}//End IF Closing buy order
			else if (closeorder_it->Direction == MORDER_BUY &&
				(closeorder_it->OrderStatus == MORDER_PART_TRADED || closeorder_it->OrderStatus == MORDER_ACCEPTED || closeorder_it->OrderStatus == MORDER_QUEUED)) {
				if (closeorder_it->CanbeCanceled && closeorder_it->IsClosePofitOrder && closeorder_it->MOrderType == -1) {
					char line[200];
					sprintf(line, "%s,Sell Order Stoploss Close,%.5f,OrderId=%d", mStrategyAndInstance.c_str(), m_Price, closeorder_it->OrderId);
					WriteMsgToLogList(line);
					closeorder_it->NextCloseOrderPrice = m_Buy1 + m_dOneTick;
					ReqOrderDelete(closeorder_it->OrderId, closeorder_it->OrderLocalRef, closeorder_it->FrontID, closeorder_it->SessionID);
					closeorder_it->CanbeCanceled = false;
					m_bStoplossClose = true;
				}
				else if (closeorder_it->LimitPrice < (m_Buy1 - 0.00001) && closeorder_it->IsStoplessOrder && closeorder_it->CanbeCanceled) {
					char line[200];
					sprintf(line, "%s,Sell Order Close ReSubmit,OrderId=%d", mStrategyAndInstance.c_str(), closeorder_it->OrderId);
					WriteMsgToLogList(line);
					closeorder_it->NextCloseOrderPrice = m_Sell1;
					ReqOrderDelete(closeorder_it->OrderId, closeorder_it->OrderLocalRef, closeorder_it->FrontID, closeorder_it->SessionID);
					closeorder_it->CanbeCanceled = false;
				}
				if (closeorder_it->MOrderType == -1)
				{
					SellOrderNeedStopExist = true;
					//mStrategyParams.OpenSellAllow = 0;
				}

			}//End IF Closing sell order
			//}//End for Strategy 1
		}//End Looping close order list
	}//End if the close order list is not null
	if (mStrategyParams.OpenBuyAllow && BuyOrderNeedStopExist)
	{
		mStrategyParams.BasePrice = pDepthMarketData->price + mStrategyParams.DnGridDistPoint * int(mStrategyParams.DnGridCount / 3) * m_dOneTick;
	}
	else if (mStrategyParams.OpenSellAllow && SellOrderNeedStopExist)
	{
		mStrategyParams.BasePrice = pDepthMarketData->price - mStrategyParams.UpGridDistPoint * int(mStrategyParams.UpGridCount / 3) * m_dOneTick;;
	}

	int nBuyOrderType=-1;
	int nSellOrderType=-1;
	//if(OpenOrderList.empty()&&!CloseOrderList.empty()){
	//	std::list<MyCloseOrderType>::iterator closeorder_it;

	//	int nSellMaxType=0;
	//	for(closeorder_it = CloseOrderList.begin(); closeorder_it != CloseOrderList.end(); ++closeorder_it){
	//		if(MORDER_BUY==closeorder_it->Direction){
	//			nSellMaxType=max(closeorder_it->MOrderType,nSellMaxType);
	//		}
	//	}
	//	nSellOrderType=nSellMaxType+1;

	//	int nBuyMaxType=9999;
	//	for (closeorder_it = CloseOrderList.begin(); closeorder_it != CloseOrderList.end(); ++closeorder_it){
	//		if(MORDER_SELL==closeorder_it->Direction){
	//			nBuyMaxType=min(closeorder_it->MOrderType,nBuyMaxType);
	//		}
	//	}
	//	nBuyOrderType=nBuyMaxType-1;

	//}else if(OpenOrderList.empty()&&CloseOrderList.empty()){
	//	nBuyOrderType=29;
	//	nSellOrderType=30;
	//}


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

	// �����λ
	if (mStrategyParams.OpenBuyAllow && nBuyOrderType < nGrid - mStrategyParams.DnGridCount && mStrategyParams.DnGridCount>0 && !BuyOrderNeedStopExist) {
		int nCount = 1000;
		if (0 == mStrategyParams.OffFlagPoint) nCount = nGrid - nBuyOrderType;
		else nCount = nGrid - nBuyOrderType - 1;

		double StopPrice = mStrategyParams.BasePrice - mStrategyParams.DnGridDistPoint * nCount * m_dOneTick;
		if ((m_Price + 0.00001 < StopPrice)) {
			int MoveGridCount = mStrategyParams.DnGridCount / 2*3;//��׼�������ƶ���������Ŀ
			double oldBasePrice = mStrategyParams.BasePrice;
			mStrategyParams.BasePrice = mStrategyParams.BasePrice - mStrategyParams.DnGridDistPoint * MoveGridCount * m_dOneTick;
			//mClosePrice=mStrategyParams.BasePrice;

			std::list<MyCloseOrderType>::iterator itr_tmp;
			for (itr_tmp = CloseOrderList.begin(); itr_tmp != CloseOrderList.end(); ++itr_tmp) {
				if ((itr_tmp->OrderStatus == MORDER_ACCEPTED || itr_tmp->OrderStatus == MORDER_QUEUED)
					&& itr_tmp->Direction == MORDER_SELL && (itr_tmp->MOrderType >= nGrid - MoveGridCount && itr_tmp->MOrderType <= nGrid-1))
					itr_tmp->MOrderType = -1;//��ƽ��
			}

			for (itr_tmp = CloseOrderList.begin(); itr_tmp != CloseOrderList.end(); ++itr_tmp) {
				if ((itr_tmp->OrderStatus == MORDER_ACCEPTED || itr_tmp->OrderStatus == MORDER_QUEUED) && itr_tmp->Direction == MORDER_SELL) {
					if (itr_tmp->MOrderType >= nGrid - mStrategyParams.DnGridCount && itr_tmp->MOrderType < nGrid - MoveGridCount) {
						itr_tmp->MOrderType += MoveGridCount;
					}
				}
			}
			for (int i = nGrid -1 - (mStrategyParams.DnGridCount - MoveGridCount); i >= 0; --i) GridCloseOrderCount[i] = 0;
		}
	}
	// ������λ
	if (mStrategyParams.OpenSellAllow && nSellOrderType > nGrid-1 + mStrategyParams.UpGridCount && mStrategyParams.UpGridCount > 0 && !SellOrderNeedStopExist) {
		int nCount = 1000;
		if (0 == mStrategyParams.OffFlagPoint) nCount = nSellOrderType - nGrid + 1;
		else nCount = nSellOrderType - nGrid;

		double StopPrice = mStrategyParams.BasePrice + mStrategyParams.UpGridDistPoint * nCount * m_dOneTick;
		if ((m_Price + 0.00001 > StopPrice)) {
			int MoveGridCount = mStrategyParams.UpGridCount / 2*3;//��׼�������ƶ���������Ŀ
			mStrategyParams.BasePrice = mStrategyParams.BasePrice + mStrategyParams.UpGridDistPoint * MoveGridCount * m_dOneTick;

			std::list<MyCloseOrderType>::iterator itr_tmp;
			for (itr_tmp = CloseOrderList.begin(); itr_tmp != CloseOrderList.end(); ++itr_tmp) {
				if ((itr_tmp->OrderStatus == MORDER_ACCEPTED || itr_tmp->OrderStatus == MORDER_QUEUED) && itr_tmp->Direction == MORDER_BUY
					&& (itr_tmp->MOrderType >= nGrid && itr_tmp->MOrderType < nGrid + MoveGridCount))
					itr_tmp->MOrderType = -1;//��ƽ��
			}

			for (itr_tmp = CloseOrderList.begin(); itr_tmp != CloseOrderList.end(); ++itr_tmp) {
				if ((itr_tmp->OrderStatus == MORDER_ACCEPTED || itr_tmp->OrderStatus == MORDER_QUEUED) && itr_tmp->Direction == MORDER_BUY
					&& (itr_tmp->MOrderType >= nGrid + MoveGridCount && itr_tmp->MOrderType < nGrid + mStrategyParams.UpGridCount)) {
					itr_tmp->MOrderType = itr_tmp->MOrderType - MoveGridCount;
				}
			}

			for (int i = nGrid + mStrategyParams.UpGridCount - MoveGridCount; i < nGrid*2; ++i) GridCloseOrderCount[i] = 0;
		}
	}

	// ��������
	if(nSellOrderType>= nGrid &&nSellOrderType<=(nGrid +mStrategyParams.UpGridCount-1)){
		if(0==GridCloseOrderCount[nSellOrderType]){
			if(bOpenFlag&&mStrategyParams.OpenSellAllow&&mStrategyParams.BasePrice-0.00001>0
				&&OnHandPositionCount<=MaxOnHandPositionCount&&TotalOnHandPosition<=MaxTotalOnHandPosition
				&&TotalCancelTimes<=MaxTotalCancelTimes&&nSec_m>nSecond){
					//Open Sell Order
					MyOpenOrderType openThostOrder;
					iNextOrderRef++;
					openThostOrder.OrderLocalRetReqID=0;
					openThostOrder.OrderId=-1;
					openThostOrder.OrderLocalRef=-1;
					int nCount=nSellOrderType- nGrid +1;
					openThostOrder.LimitPrice=mStrategyParams.BasePrice+mStrategyParams.UpGridDistPoint*nCount*m_dOneTick;
					// �жϼ۸��Ƿ�����ͣ
					if((openThostOrder.LimitPrice-0.00001)>=pDepthMarketData->upperLimitPrice) return;
					openThostOrder.OrigSubmitPrice=openThostOrder.LimitPrice;
					openThostOrder.Direction=MORDER_SELL;
					openThostOrder.Offset=MORDER_OPEN;
					openThostOrder.VolumeTotal=mStrategyParams.GridVol;
					openThostOrder.VolumeTotalOriginal=openThostOrder.VolumeTotal;
					openThostOrder.VolumeTraded=0;
					openThostOrder.ProfitPrice=openThostOrder.LimitPrice-mStrategyParams.ProfitPoint*m_dOneTick;
					openThostOrder.OpenOrderCanbeCanceled=true;
					openThostOrder.MOrderType=nSellOrderType;
					openThostOrder.maxProfit=0.0;			
					openThostOrder.MAStop=false;

					GridCloseOrderCount[nSellOrderType]=openThostOrder.VolumeTotal;
					mOpenTimes++;
					openThostOrder.mStoplossPoint=mStrategyParams.StoplossPoint;

					ReqOpenOrderInsert(&openThostOrder);	

					OnHandPositionCount+=openThostOrder.VolumeTotal;
					TotalOnHandPosition+=openThostOrder.VolumeTotal;

					char log[200];
					sprintf(log,"%s,�����񿪲�,��=%.5f,ʱ��=%s,����:%d",mStrategyAndInstance.c_str(),openThostOrder.LimitPrice,pDepthMarketData->updatetime,openThostOrder.MOrderType);
					AddtoTipMsgListBox(log);
					WriteMsgToLogList(log);
			}
		}
	}

	// ����һ����
	if(nBuyOrderType>=(nGrid -1-mStrategyParams.DnGridCount+1)&&nBuyOrderType>=0&&nBuyOrderType<= nGrid-1){
		if(0==GridCloseOrderCount[nBuyOrderType]){
			if(bOpenFlag&&mStrategyParams.OpenBuyAllow&&mStrategyParams.BasePrice-0.00001>0
				&&OnHandPositionCount<=MaxOnHandPositionCount&&TotalOnHandPosition<=MaxTotalOnHandPosition
				&&TotalCancelTimes<=MaxTotalCancelTimes&&nSec_m>nSecond){
					MyOpenOrderType openThostOrder;
					iNextOrderRef++;
					openThostOrder.OrderLocalRetReqID=0;
					openThostOrder.OrderId=-1;
					openThostOrder.OrderLocalRef=-1;
					int nCount= nGrid -nBuyOrderType;
					openThostOrder.LimitPrice=mStrategyParams.BasePrice-mStrategyParams.DnGridDistPoint*nCount*m_dOneTick;
					// �жϼ۸��Ƿ��ڵ�ͣ
					if((openThostOrder.LimitPrice+0.00001)<=pDepthMarketData->lowerLimitPrice) return;
					openThostOrder.OrigSubmitPrice=openThostOrder.LimitPrice;
					openThostOrder.Direction=MORDER_BUY;
					openThostOrder.Offset=MORDER_OPEN;
					openThostOrder.VolumeTotal=mStrategyParams.GridVol;;
					openThostOrder.VolumeTotalOriginal=openThostOrder.VolumeTotal;
					openThostOrder.VolumeTraded=0;
					openThostOrder.ProfitPrice=openThostOrder.LimitPrice+mStrategyParams.ProfitPoint*m_dOneTick;
					openThostOrder.OpenOrderCanbeCanceled=true;
					openThostOrder.MOrderType=nBuyOrderType;
					openThostOrder.MAStop=false;
					openThostOrder.maxProfit=0.0;

					GridCloseOrderCount[nBuyOrderType]=openThostOrder.VolumeTotal;
					mOpenTimes++;
					openThostOrder.mStoplossPoint=mStrategyParams.StoplossPoint;

					ReqOpenOrderInsert(&openThostOrder);

					OnHandPositionCount+=openThostOrder.VolumeTotal;
					TotalOnHandPosition+=openThostOrder.VolumeTotal;

					char log[200];
					sprintf(log,"%s,�����񿪲�,��=%.5f,ʱ��=%s,����:%d",mStrategyAndInstance.c_str(),openThostOrder.LimitPrice,pDepthMarketData->updatetime,openThostOrder.MOrderType);
					AddtoTipMsgListBox(log);
					WriteMsgToLogList(log);
			}
		}
	}

	strcpy(curTradingDay,CTPTradingDay);
	FlushStrategyInfoToFile();
}

void StrategyBaseGridOpen_plus::OnRtnOrder(OrderTradeMsg *pOrderTradeMsg)
{
	int thisOrderId = (pOrderTradeMsg->OrderSysId);
	//UpdateOrderIdToOrderList(thisOrderId);
	char line[200];
	sprintf(line,"%s,OnRtnOrder,OrderId=%d,Status=%c,VolRemain=%d,OrderRef=%d",mStrategyAndInstance.c_str(),thisOrderId,pOrderTradeMsg->OrderStatus,
		pOrderTradeMsg->VolumeTotal,pOrderTradeMsg->OrderLocalRef);
	WriteMsgToLogList(line);

	std::list<MyOpenOrderType>::iterator openorder_it;
	if(!OpenOrderList.empty()){
		for(openorder_it = OpenOrderList.begin(); openorder_it != OpenOrderList.end();openorder_it++){
			//std::list<MyOpenOrderType>::iterator iter_e=openorder_it++;
			if(openorder_it->OrderId==thisOrderId||(openorder_it->OrderLocalRef==pOrderTradeMsg->OrderLocalRef&&openorder_it->OrderLocalRef>0)){
				//printf("OpenOrderList exist order \n");
				if(openorder_it->OrderLocalRef==pOrderTradeMsg->OrderLocalRef&&openorder_it->OrderLocalRef>0)openorder_it->OrderId=thisOrderId;
				//CTPδ����OnRspOrderInsert,�ڴ˴�����map
				map<int,string>::iterator iter;
				AcquireSRWLockExclusive(&g_srwLockReqId);
				iter=OrderIdToStrategyNameForDisplay.find(thisOrderId);
				if(iter==OrderIdToStrategyNameForDisplay.end()){
					string sfinal(mStrategyAndInstance);
					sfinal.append("_open");
					OrderIdToStrategyNameForDisplay.insert(std::pair<int,string>(thisOrderId,sfinal));
				}
				ReleaseSRWLockExclusive(&g_srwLockReqId);

				openorder_it->OrderStatus=pOrderTradeMsg->OrderStatus;
				if(pOrderTradeMsg->OrderStatus==MORDER_PART_CANCELLED||pOrderTradeMsg->OrderStatus==MORDER_CANCELLED) {

					//���ֵ�����,�����±���
					OnHandPositionCount-=pOrderTradeMsg->VolumeTotal;
					TotalOnHandPosition-=pOrderTradeMsg->VolumeTotal;
					// ������������
					GridCloseOrderCount[openorder_it->MOrderType]-=pOrderTradeMsg->VolumeTotal;
					mOpenTimes--;

					OpenOrderList.erase(openorder_it);
					break;
				}

			} //End if openorder_it->OrderRef==thisTradeRef
		}//End While
	}

	std::list<MyCloseOrderType>::iterator closeorder_it;
	if(!CloseOrderList.empty()){
		for(closeorder_it = CloseOrderList.begin(); closeorder_it != CloseOrderList.end();closeorder_it++){
			//std::list<MyCloseOrderType>::iterator iter_ec=closeorder_it++;
			if(closeorder_it->OrderId==thisOrderId||(closeorder_it->OrderLocalRef==pOrderTradeMsg->OrderLocalRef&&closeorder_it->OrderLocalRef>0)){
				if(closeorder_it->OrderLocalRef==pOrderTradeMsg->OrderLocalRef&&closeorder_it->OrderLocalRef>0)closeorder_it->OrderId=thisOrderId;
				//CTPδ����OnRspOrderInsert,�ڴ˴�����map
				map<int,string>::iterator iter;
				AcquireSRWLockExclusive(&g_srwLockReqId);
				iter=OrderIdToStrategyNameForDisplay.find(thisOrderId);
				if(iter==OrderIdToStrategyNameForDisplay.end()){
					string sfinal(mStrategyAndInstance);
					sfinal.append("_close");
					OrderIdToStrategyNameForDisplay.insert(std::pair<int,string>(thisOrderId,sfinal));
				}
				ReleaseSRWLockExclusive(&g_srwLockReqId);

				closeorder_it->OrderStatus=pOrderTradeMsg->OrderStatus;
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
							newThostOrder.LimitPrice = m_Buy1 - 10 * m_dOneTick;//�����¼۽��б�ֹ��
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
							newThostOrder.LimitPrice = m_Sell1 + 10 * m_dOneTick;//�����¼۽��б�ֹ��
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

				//if(pOrderTradeMsg->OrderStatus==MORDER_PART_CANCELLED||pOrderTradeMsg->OrderStatus==MORDER_CANCELLED)
				//	closeorder_it->OrderStatus=MORDER_WAITFORSUBMIT;
			}//End if closeorder_it->OrderRef==thisOrderRef for Strategy 2

		}//End While
	}

	if(pOrderTradeMsg->OrderStatus==MORDER_PART_CANCELLED||pOrderTradeMsg->OrderStatus==MORDER_CANCELLED){
		map<int,int>::iterator iter;
		AcquireSRWLockExclusive(&g_srwLockOrderId);
		iter=OrderIdToShmIndex.find(pOrderTradeMsg->OrderSysId);
		if(iter!=OrderIdToShmIndex.end()){
			OrderIdToShmIndex.erase(iter);
		}
		ReleaseSRWLockExclusive(&g_srwLockOrderId);
	}

	FlushStrategyInfoToFile();
}

void StrategyBaseGridOpen_plus::OnRtnTrade(OrderTradeMsg *pOrderTradeMsg)
{
	int baseGrid = ES_GRIDCOUNT >> 1;
	int thisTradeRef = pOrderTradeMsg->OrderSysId;
	string strmatchno(pOrderTradeMsg->MatchNo);
	map<string,string>::iterator iter;
	iter=matchnomap.find(strmatchno);
	if(iter==matchnomap.end()){
		matchnomap.insert(std::pair<string,string>(strmatchno,strmatchno));

		//UpdateOrderIdToOrderList(thisTradeRef);
		char line[200];
		sprintf(line,"%s,OnRtnTrade,OrderId=%d,vol=%d,price=%.5f",mStrategyAndInstance.c_str(),thisTradeRef,pOrderTradeMsg->Volume,pOrderTradeMsg->Price);
		WriteMsgToLogList(line);

		double submitprice=0;
		int tradedirection=-1;
		int openorclose=-1;
		std::list<MyOpenOrderType>::iterator openorder_it;
		if(!OpenOrderList.empty()){
			for(openorder_it = OpenOrderList.begin(); openorder_it != OpenOrderList.end();openorder_it++){
				//std::list<MyOpenOrderType>::iterator iter_e=openorder_it++;
				if(openorder_it->OrderId==thisTradeRef||(openorder_it->OrderLocalRef==pOrderTradeMsg->OrderLocalRef&&openorder_it->OrderLocalRef>0)){
					if(openorder_it->OrderLocalRef==pOrderTradeMsg->OrderLocalRef&&openorder_it->OrderLocalRef>0)openorder_it->OrderId=thisTradeRef;
					openorclose=MORDER_OPEN;
					openorder_it->VolumeTraded=openorder_it->VolumeTraded+pOrderTradeMsg->Volume;
					openorder_it->VolumeTotal=openorder_it->VolumeTotal-pOrderTradeMsg->Volume;		
					strcpy(openorder_it->OpenTime,pOrderTradeMsg->InsertOrTradeTime);
					if(openorder_it->Direction==MORDER_BUY)
					{
						tradedirection=MORDER_BUY;
						MyCloseOrderType thostOrder;//ƽ��ֹӯ��
						thostOrder.OrderId=-1;
						thostOrder.OrderLocalRef=-1;
						thostOrder.OpenOrderID=thisTradeRef;
						thostOrder.OpenOrderSubmitPrice=openorder_it->OrigSubmitPrice;
						thostOrder.OpenOrderTradePrice=pOrderTradeMsg->Price;
						thostOrder.IsClosePofitOrder=true;
						thostOrder.IsStoplessOrder=false;
						thostOrder.CanbeCanceled=true;
						thostOrder.dwCloseOrderStart=GetTickCount();
						thostOrder.MOrderType=openorder_it->MOrderType;
						thostOrder.MAStop=false;
						strcpy(thostOrder.OpenTime,pOrderTradeMsg->InsertOrTradeTime);
						iNextOrderRef++;
						thostOrder.OrderLocalRetReqID=0;
						thostOrder.CloseOrderSeqNo=++mCloseOrderSeqNo;
						thostOrder.Offset = MORDER_CLOSE;
						thostOrder.OrderStatus=MORDER_ACCEPTED;
						thostOrder.Direction=MORDER_SELL;
						thostOrder.LimitPrice=openorder_it->ProfitPrice;
						thostOrder.OrigSubmitPrice=thostOrder.LimitPrice;
						thostOrder.ProfitPrice=openorder_it->ProfitPrice;
						thostOrder.VolumeTotalOriginal=pOrderTradeMsg->Volume;
						thostOrder.VolumeTraded=0;
						thostOrder.VolumeTotal=pOrderTradeMsg->Volume;//
						thostOrder.maxProfit=0;
						thostOrder.mStoplossPoint=openorder_it->mStoplossPoint;
						thostOrder.ManualStopPrice=0;
						
						ReqCloseOrderInsert(&thostOrder,openorder_it->OpenTime); 
						submitprice=openorder_it->OrigSubmitPrice;

						if(openorder_it->VolumeTotal==0){
							OpenOrderList.erase(openorder_it);
						}
						break;

					}else if(openorder_it->Direction==MORDER_SELL){
						tradedirection=MORDER_SELL;
						MyCloseOrderType thostOrder;//ƽ��ֹӯ��
						thostOrder.OrderId=-1;
						thostOrder.OrderLocalRef=-1;
						thostOrder.OpenOrderID=thisTradeRef;
						thostOrder.OpenOrderSubmitPrice=openorder_it->OrigSubmitPrice;
						thostOrder.OpenOrderTradePrice=pOrderTradeMsg->Price;
						thostOrder.IsClosePofitOrder=true;
						thostOrder.IsStoplessOrder=false;
						thostOrder.CanbeCanceled=true;
						thostOrder.dwCloseOrderStart=GetTickCount();
						thostOrder.MOrderType=openorder_it->MOrderType;
						thostOrder.MAStop=false;
						strcpy(thostOrder.OpenTime,pOrderTradeMsg->InsertOrTradeTime);
						iNextOrderRef++;
						thostOrder.OrderLocalRetReqID=0;
						thostOrder.CloseOrderSeqNo=++mCloseOrderSeqNo;
						thostOrder.Offset = MORDER_CLOSE;
						thostOrder.OrderStatus=MORDER_ACCEPTED;
						thostOrder.Direction=MORDER_BUY;
						thostOrder.LimitPrice=openorder_it->ProfitPrice;
						thostOrder.OrigSubmitPrice=thostOrder.LimitPrice;
						thostOrder.ProfitPrice=openorder_it->ProfitPrice;
						thostOrder.VolumeTotalOriginal=pOrderTradeMsg->Volume;
						thostOrder.VolumeTraded=0;
						thostOrder.VolumeTotal=pOrderTradeMsg->Volume;//
						thostOrder.maxProfit=0;
						thostOrder.mStoplossPoint=openorder_it->mStoplossPoint;
						thostOrder.ManualStopPrice=0;
						ReqCloseOrderInsert(&thostOrder,openorder_it->OpenTime); 

						submitprice=openorder_it->OrigSubmitPrice;

						if(openorder_it->VolumeTotal==0){
							OpenOrderList.erase(openorder_it);
						}
						break;

					}else {
						//printf("openDirection Excetion!!! =%i\n",pOrderTradeMsg->Direction);
					}


				} //End if openorder_it->OrderRef==thisTradeRef
			}
		}

		int closeprofitornot=-1;
		int openid=-1;
		std::list<MyCloseOrderType>::iterator closeorder_it;
		if(!CloseOrderList.empty()){
			for(closeorder_it = CloseOrderList.begin(); closeorder_it != CloseOrderList.end();closeorder_it++){
				//std::list<MyCloseOrderType>::iterator iter_ec=closeorder_it++;
				if(closeorder_it->OrderId==thisTradeRef||(closeorder_it->OrderLocalRef==pOrderTradeMsg->OrderLocalRef&&closeorder_it->OrderLocalRef>0)){
					if(closeorder_it->OrderLocalRef==pOrderTradeMsg->OrderLocalRef&&closeorder_it->OrderLocalRef>0)closeorder_it->OrderId=thisTradeRef;
					openorclose=MORDER_CLOSE;
					openid=closeorder_it->OpenOrderID;
					//printf("CloseOrderList exist order \n");	
					submitprice=closeorder_it->OrigSubmitPrice;
					if(closeorder_it->IsClosePofitOrder){
						closeprofitornot=1;
					}else closeprofitornot=0;

					if(closeorder_it->Direction==MORDER_BUY){
						tradedirection=MORDER_BUY;
					}else if(closeorder_it->Direction==MORDER_SELL){
						tradedirection=MORDER_SELL;
					}

					closeorder_it->VolumeTraded=closeorder_it->VolumeTraded+pOrderTradeMsg->Volume;
					closeorder_it->VolumeTotal=closeorder_it->VolumeTotal-pOrderTradeMsg->Volume;
					//��������ƽ�ֵ���δƽ����
					GridCloseOrderCount[closeorder_it->MOrderType]=GridCloseOrderCount[closeorder_it->MOrderType]-pOrderTradeMsg->Volume;

					OnHandPositionCount-=pOrderTradeMsg->Volume;
					TotalOnHandPosition-=pOrderTradeMsg->Volume;

					if(pOrderTradeMsg->Volume==closeorder_it->VolumeTotalOriginal){
						// �����ҵ�ǰ����

						int nGrid = closeorder_it->MOrderType;
						if(nGrid>= baseGrid &&nGrid<= baseGrid +mStrategyParams.UpGridCount-1&&MORDER_BUY==closeorder_it->Direction&&0==GridCloseOrderCount[nGrid]&&mStrategyParams.OpenSellAllow
							&&OnHandPositionCount<=MaxOnHandPositionCount&&TotalOnHandPosition<=MaxTotalOnHandPosition
							&&TotalCancelTimes<=MaxTotalCancelTimes){
								MyOpenOrderType openThostOrder;
								iNextOrderRef++;
								openThostOrder.OrderLocalRetReqID=0;
								openThostOrder.OrderId=-1;
								openThostOrder.OrderLocalRef=-1;
								openThostOrder.MOrderType=nGrid;
								int nCount=nGrid- baseGrid +1;
								openThostOrder.LimitPrice=mStrategyParams.BasePrice+(mStrategyParams.UpGridDistPoint*nCount)*m_dOneTick;
								openThostOrder.OrigSubmitPrice=openThostOrder.LimitPrice;
								openThostOrder.Direction=MORDER_SELL;
								openThostOrder.Offset=MORDER_OPEN;
								openThostOrder.VolumeTotal=mStrategyParams.GridVol;
								openThostOrder.VolumeTotalOriginal=openThostOrder.VolumeTotal;
								openThostOrder.VolumeTraded=0;

								openThostOrder.ProfitPrice=openThostOrder.LimitPrice-mStrategyParams.ProfitPoint*m_dOneTick;
								openThostOrder.mStoplossPoint=mStrategyParams.StoplossPoint;

								openThostOrder.OpenOrderCanbeCanceled=true;
								
								openThostOrder.maxProfit=0.0;			
								openThostOrder.MAStop=false;
								GridCloseOrderCount[nGrid]=openThostOrder.VolumeTotal;
								ReqOpenOrderInsert(&openThostOrder);	
								OnHandPositionCount+=openThostOrder.VolumeTotal;
								TotalOnHandPosition+=openThostOrder.VolumeTotal;

								char log[200];
								sprintf(log,"%s,����,��=%.5f,ʱ��=%s,����:%d",mStrategyAndInstance.c_str(),openThostOrder.LimitPrice,pOrderTradeMsg->InsertOrTradeTime,openThostOrder.MOrderType);
								AddtoTipMsgListBox(log);
								WriteMsgToLogList(log);
								// ������һ����ҵ�
								std::list<MyOpenOrderType>::iterator openorder_itother;
								for(openorder_itother = OpenOrderList.begin(); openorder_itother != OpenOrderList.end();openorder_itother++){
									if(nGrid+1<= baseGrid*2-1 &&openorder_itother->Direction==MORDER_SELL&&GridCloseOrderCount[nGrid+1]>0){
										ReqOrderDelete(openorder_itother->OrderId,openorder_itother->OrderLocalRef,openorder_itother->FrontID,openorder_itother->SessionID);
										openorder_itother->OpenOrderCanbeCanceled=false;
										break;
									}
								}
								
						}else if(nGrid>= baseGrid -1-mStrategyParams.DnGridCount+1&&nGrid<= baseGrid -1&&MORDER_SELL==closeorder_it->Direction&&0==GridCloseOrderCount[nGrid]&&mStrategyParams.OpenBuyAllow
							&&OnHandPositionCount<=MaxOnHandPositionCount&&TotalOnHandPosition<=MaxTotalOnHandPosition
							&&TotalCancelTimes<=MaxTotalCancelTimes){
								MyOpenOrderType openThostOrder;
								iNextOrderRef++;
								openThostOrder.OrderLocalRetReqID=0;
								openThostOrder.OrderId=-1;
								openThostOrder.OrderLocalRef=-1;
								openThostOrder.MOrderType=nGrid;
								int nCount= baseGrid -1-nGrid+1;
								openThostOrder.LimitPrice=mStrategyParams.BasePrice-(mStrategyParams.DnGridDistPoint*nCount)*m_dOneTick;
								openThostOrder.OrigSubmitPrice=openThostOrder.LimitPrice;
								openThostOrder.Direction=MORDER_BUY;
								openThostOrder.Offset=MORDER_OPEN;
								openThostOrder.VolumeTotal=mStrategyParams.GridVol;
								openThostOrder.VolumeTotalOriginal=openThostOrder.VolumeTotal;
								openThostOrder.VolumeTraded=0;

								openThostOrder.ProfitPrice=openThostOrder.LimitPrice+mStrategyParams.ProfitPoint*m_dOneTick;
								openThostOrder.mStoplossPoint=mStrategyParams.StoplossPoint;

								openThostOrder.OpenOrderCanbeCanceled=true;
								
								openThostOrder.maxProfit=0.0;			
								openThostOrder.MAStop=false;
								GridCloseOrderCount[nGrid]=openThostOrder.VolumeTotal;
								ReqOpenOrderInsert(&openThostOrder);	
								OnHandPositionCount+=openThostOrder.VolumeTotal;
								TotalOnHandPosition+=openThostOrder.VolumeTotal;

								char log[200];
								sprintf(log,"%s,����,��=%.5f,ʱ��=%s,����:%d",mStrategyAndInstance.c_str(),openThostOrder.LimitPrice,pOrderTradeMsg->InsertOrTradeTime,openThostOrder.MOrderType);
								AddtoTipMsgListBox(log);
								WriteMsgToLogList(log);

								// ������һ����ҵ�
								std::list<MyOpenOrderType>::iterator openorder_itother;
								for(openorder_itother = OpenOrderList.begin(); openorder_itother != OpenOrderList.end();openorder_itother++){
									if(nGrid-1>=0&&openorder_itother->Direction==MORDER_BUY&&GridCloseOrderCount[nGrid-1]>0){
										ReqOrderDelete(openorder_itother->OrderId,openorder_itother->OrderLocalRef,openorder_itother->FrontID,openorder_itother->SessionID);
										openorder_itother->OpenOrderCanbeCanceled=false;
										break;
									}
								}
						}

						CloseOrderList.erase(closeorder_it);

						break;
					}else{
						if(closeorder_it->VolumeTotal==0){
							int nGrid = closeorder_it->MOrderType;
							if(nGrid>= baseGrid &&nGrid<= baseGrid +mStrategyParams.UpGridCount-1&&MORDER_BUY==closeorder_it->Direction&&0==GridCloseOrderCount[nGrid]&&mStrategyParams.OpenSellAllow
								&&OnHandPositionCount<=MaxOnHandPositionCount&&TotalOnHandPosition<=MaxTotalOnHandPosition
								&&TotalCancelTimes<=MaxTotalCancelTimes){
									MyOpenOrderType openThostOrder;
									iNextOrderRef++;
									openThostOrder.OrderLocalRetReqID=0;
									openThostOrder.OrderId=-1;
									openThostOrder.OrderLocalRef=-1;
									openThostOrder.MOrderType=nGrid;
									int nCount=nGrid- baseGrid +1;
									openThostOrder.LimitPrice=mStrategyParams.BasePrice+(mStrategyParams.UpGridDistPoint*nCount)*m_dOneTick;
									openThostOrder.OrigSubmitPrice=openThostOrder.LimitPrice;
									openThostOrder.Direction=MORDER_SELL;
									openThostOrder.Offset=MORDER_OPEN;
									openThostOrder.VolumeTotal=mStrategyParams.GridVol;
									openThostOrder.VolumeTotalOriginal=openThostOrder.VolumeTotal;
									openThostOrder.VolumeTraded=0;

									openThostOrder.ProfitPrice=openThostOrder.LimitPrice-mStrategyParams.ProfitPoint*m_dOneTick;
									openThostOrder.mStoplossPoint=mStrategyParams.StoplossPoint;

									openThostOrder.OpenOrderCanbeCanceled=true;
									
									openThostOrder.maxProfit=0.0;			
									openThostOrder.MAStop=false;
									GridCloseOrderCount[nGrid]=openThostOrder.VolumeTotal;
									ReqOpenOrderInsert(&openThostOrder);	
									OnHandPositionCount+=openThostOrder.VolumeTotal;
									TotalOnHandPosition+=openThostOrder.VolumeTotal;

									char log[200];
									sprintf(log,"%s,����,��=%.5f,ʱ��=%s,����:%d",mStrategyAndInstance.c_str(),openThostOrder.LimitPrice,pOrderTradeMsg->InsertOrTradeTime,openThostOrder.MOrderType);
									AddtoTipMsgListBox(log);
									WriteMsgToLogList(log);

									// ������һ����ҵ�
									std::list<MyOpenOrderType>::iterator openorder_itother;
									for(openorder_itother = OpenOrderList.begin(); openorder_itother != OpenOrderList.end();openorder_itother++){
										if(nGrid+1<= baseGrid *2-1&&openorder_itother->Direction==MORDER_SELL&&GridCloseOrderCount[nGrid+1]>0){
											ReqOrderDelete(openorder_itother->OrderId,openorder_itother->OrderLocalRef,openorder_itother->FrontID,openorder_itother->SessionID);
											openorder_itother->OpenOrderCanbeCanceled=false;
											break;
										}
									}

							}else if(nGrid>= baseGrid -1-mStrategyParams.DnGridCount+1&&nGrid<= baseGrid -1&&MORDER_SELL==closeorder_it->Direction&&0==GridCloseOrderCount[nGrid]&&mStrategyParams.OpenBuyAllow
								&&OnHandPositionCount<=MaxOnHandPositionCount&&TotalOnHandPosition<=MaxTotalOnHandPosition
								&&TotalCancelTimes<=MaxTotalCancelTimes){
									MyOpenOrderType openThostOrder;
									iNextOrderRef++;
									openThostOrder.OrderLocalRetReqID=0;
									openThostOrder.OrderId=-1;
									openThostOrder.OrderLocalRef=-1;
									openThostOrder.MOrderType=nGrid;
									int nCount= baseGrid -1-nGrid+1;
									openThostOrder.LimitPrice=mStrategyParams.BasePrice-(mStrategyParams.DnGridDistPoint*nCount)*m_dOneTick;
									openThostOrder.OrigSubmitPrice=openThostOrder.LimitPrice;
									openThostOrder.Direction=MORDER_BUY;
									openThostOrder.Offset=MORDER_OPEN;
									openThostOrder.VolumeTotal=mStrategyParams.GridVol;
									openThostOrder.VolumeTotalOriginal=openThostOrder.VolumeTotal;
									openThostOrder.VolumeTraded=0;

									openThostOrder.ProfitPrice=openThostOrder.LimitPrice+mStrategyParams.ProfitPoint*m_dOneTick;
									openThostOrder.mStoplossPoint=mStrategyParams.StoplossPoint;

									openThostOrder.OpenOrderCanbeCanceled=true;
									
									openThostOrder.maxProfit=0.0;			
									openThostOrder.MAStop=false;
									GridCloseOrderCount[nGrid]=openThostOrder.VolumeTotal;
									ReqOpenOrderInsert(&openThostOrder);	
									OnHandPositionCount+=openThostOrder.VolumeTotal;
									TotalOnHandPosition+=openThostOrder.VolumeTotal;

									char log[200];
									sprintf(log,"%s,����,��=%.5f,ʱ��=%s,����:%d",mStrategyAndInstance.c_str(),openThostOrder.LimitPrice,pOrderTradeMsg->InsertOrTradeTime,openThostOrder.MOrderType);
									AddtoTipMsgListBox(log);
									WriteMsgToLogList(log);

									// ������һ����ҵ�
									std::list<MyOpenOrderType>::iterator openorder_itother;
									for(openorder_itother = OpenOrderList.begin(); openorder_itother != OpenOrderList.end();openorder_itother++){
										if(nGrid-1>=0&&openorder_itother->Direction==MORDER_BUY&&GridCloseOrderCount[nGrid-1]>0){
											ReqOrderDelete(openorder_itother->OrderId,openorder_itother->OrderLocalRef,openorder_itother->FrontID,openorder_itother->SessionID);
											openorder_itother->OpenOrderCanbeCanceled=false;
											break;
										}
									}
							}

							CloseOrderList.erase(closeorder_it);
							break;
						}
					}

				} //End if closeorder_it->OrderRef==thisTradeRef
			}//End While for Close Order List
		}

		if(tradedirection!=-1){
			TradeLogType trade;
			strcpy(trade.InstanceName,mInstanceName);
			strcpy(trade.StrategyID,mStrategyID);
			//int nYear,nMonth,nDate,nHour,nMin,nSec;
			string strfulltradetime(pOrderTradeMsg->InsertOrTradeTime);
			string strtimepart=strfulltradetime.substr(strfulltradetime.find_first_of(" ")+1,strfulltradetime.length()-strfulltradetime.find_first_of(" ")-1); 
			//char ctimepart[10];
			//strcpy(ctimepart,strtimepart.c_str());	
			string strdatepart=strfulltradetime.substr(0,strfulltradetime.find_first_of(" "));
			//nYear=atoi(strdatepart.c_str())/10000;
			//nMonth=(atoi(strdatepart.c_str())%10000)/100;
			//nDate=(atoi(strdatepart.c_str())%10000)%100;
			//sscanf_s(ctimepart, "%d:%d:%d",&nYear, &nMonth, &nDate,&nHour,&nMin,&nSec);
			CTime tm=CTime::GetCurrentTime();
			sprintf(trade.tradingday,"%04d%02d%02d",tm.GetYear(),tm.GetMonth(),tm.GetDay());

			//strcpy(trade.tradingday,strdatepart.c_str());
			strcpy(trade.tradingtime,strtimepart.c_str());
			//strcpy(trade.CodeName,InstCodeName);
			strcpy(trade.CodeName,m_InstCodeName);
			trade.tradeprice=pOrderTradeMsg->Price;
			trade.submitprice=submitprice;
			if(tradedirection==MORDER_BUY){
				trade.qty=pOrderTradeMsg->Volume;
			}else trade.qty=-pOrderTradeMsg->Volume;	
			trade.fee=pOrderTradeMsg->MatchFee;
			trade.openorclose=openorclose;
			if(openorclose==MORDER_OPEN){
				trade.openid=thisTradeRef;
				trade.closeid=-1;
			}else{
				trade.openid=openid;
				trade.closeid=thisTradeRef;
			}
			strcpy(trade.MatchNo,pOrderTradeMsg->MatchNo);

			Message logMsg;
			logMsg.type=TRADE_LOG;
			logMsg.AddData(&trade,0,sizeof(TradeLogType));
			LogMessageList.AddTail(logMsg);
			ReleaseSemaphore(logSemaphore, 1, NULL);

			WaitForSingleObject(MatchNoMapSem, INFINITE);
			string matchno(pOrderTradeMsg->MatchNo);
			MatchNoToStrategyNameForDisplay.insert(std::pair<string,string>(matchno,mStrategyAndInstance));
			ReleaseSemaphore(MatchNoMapSem, 1, NULL);

			DisplayTradeOnScreen(pOrderTradeMsg,tradedirection,openorclose,closeprofitornot);

			FlushStrategyInfoToFile();
		}
	}
}

void StrategyBaseGridOpen_plus::OnRspOrderInsert(ShmRspOrderInsert *pRspOrderInsert)
{
	int iRetReqID=pRspOrderInsert->iRetReqID;
	int OrderId=pRspOrderInsert->OrderID;
	if(!OpenOrderList.empty()){
		std::list<MyOpenOrderType>::iterator openorder_it;
		for(openorder_it = OpenOrderList.begin(); openorder_it != OpenOrderList.end();openorder_it++){
			//std::list<MyOpenOrderType>::iterator iter_e=openorder_it++;
			if(openorder_it->OrderLocalRetReqID==iRetReqID){
				openorder_it->OrderId=OrderId;

				map<int,int>::iterator iter;
				AcquireSRWLockExclusive(&g_srwLockReqId);
				iter=ReqIdToShmIndex.find(iRetReqID);
				if(iter!=ReqIdToShmIndex.end()){
					ReqIdToShmIndex.erase(iter);

					//string strategyname(mStrategyName);
					string sfinal(mStrategyAndInstance);
					sfinal.append("_open");
					OrderIdToStrategyNameForDisplay.insert(std::pair<int,string>(OrderId,sfinal));
				}
				ReleaseSRWLockExclusive(&g_srwLockReqId);
				return ;
			}
		}
	}

	if(!CloseOrderList.empty()){
		std::list<MyCloseOrderType>::iterator closeorder_it;
		for(closeorder_it = CloseOrderList.begin(); closeorder_it != CloseOrderList.end();closeorder_it++){
			//std::list<MyCloseOrderType>::iterator iter_ec=closeorder_it++;
			if(closeorder_it->OrderLocalRetReqID==iRetReqID){
				closeorder_it->OrderId=OrderId;
				map<int,int>::iterator iter;
				AcquireSRWLockExclusive(&g_srwLockReqId);
				iter=ReqIdToShmIndex.find(iRetReqID);
				if(iter!=ReqIdToShmIndex.end()){
					ReqIdToShmIndex.erase(iter);
					//AcquireSRWLockExclusive(&g_srwLockOrderId);
					//OrderIdToShmIndex.insert(std::pair<int,int>(OrderId,shmindex));
					//ReleaseSRWLockExclusive(&g_srwLockOrderId);
					//string strategyname(mStrategyName);
					string sfinal(mStrategyAndInstance);
					sfinal.append("_close");
					OrderIdToStrategyNameForDisplay.insert(std::pair<int,string>(OrderId,sfinal));
				}
				ReleaseSRWLockExclusive(&g_srwLockReqId);
				return;
			}
		}
	}


}

void StrategyBaseGridOpen_plus::WideCharToMultiChar(CString str,char *dest_str)
{
	//��ȡ�������Ĵ�С��������ռ䣬��������С�ǰ��ֽڼ����
	int len=WideCharToMultiByte(CP_ACP,0,str,str.GetLength(),NULL,0,NULL,NULL);
	dest_str=new char[len+1];
	WideCharToMultiByte(CP_ACP,0,str,str.GetLength(),dest_str,len,NULL,NULL);
	dest_str[len]='\0';
}

//datadate format: yyyymmdd , datatime formate hh24:mi:ss
//true - can open new order , false - shouldnot open new order
bool StrategyBaseGridOpen_plus::timeRuleForOpen(char datadate[10],char datatime[10]){
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

	//int nHour,nMin,nSec;
	//sscanf_s(datatime,"%d:%d:%d",&nHour,&nMin,&nSec);
	//int nBegin=2*3600+25*60;
	//int nEnd=2*3600+30*60;
	//int nTime=nHour*3600+nMin*60+nSec;
	//if(nTime>=nBegin&&nTime<=nEnd) return false;
	//return true;
	int nHour,nMin,nSec;
	sscanf(datatime, "%d:%d:%d", &nHour, &nMin, &nSec);

	if (nHour==9) return true;
	if (nHour == 10 && (nMin<15||nMin>29)) return true;
	if (nHour == 11 && nMin <= 29)
		return true;
	if (nHour == 13 && nMin >=30)
		return true;
	if (nHour == 14&&nMin<=56)
		return true;
	if (nHour >=21)
		return true;
	if (nHour<2)
		return true;
	if (nHour==2&&nMin<=26)
		return true;
	return false;
}

bool StrategyBaseGridOpen_plus::timeRuleForOpenNew(char datadate[10],char datatime[10]){

	int nHour,nMin,nSec;
	sscanf(datatime, "%d:%d:%d", &nHour, &nMin, &nSec);

	if(nHour==9 && nMin>=30) return true;
	if(nHour==10) return true;
	if(nHour==11 && nMin>=0 && nMin<=30) return true;

	if(nHour==13) return true;
	if(nHour==14&&nMin<=56) return true;

	return false;
}

//datadate format: yyyymmdd , datatime formate hh24:mi:ss
//true - must close current order , false - no need to close current order
bool StrategyBaseGridOpen_plus::timeRuleForClose(char datadate[10],char datatime[10]){
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

bool StrategyBaseGridOpen_plus::timeRuleForCancel(char datadate[10],char datatime[10]){
	int nYear, nMonth, nDate;
	nYear=atoi(datadate)/10000;
	nMonth=(atoi(datadate)%10000)/100;
	nDate=(atoi(datadate)%10000)%100;

	int nCHour, nCMin, nCSec;
	sscanf_s(mStrategyParams.CloseTime, "%d:%d:%d",&nCHour, &nCMin, &nCSec);

	CTime tm(nYear, nMonth, nDate,nCHour,nCMin,nCSec);
	time_t closeTime=tm.GetTime();

	int nHour,nMin,nSec; 	
	sscanf_s(datatime, "%d:%d:%d",&nHour, &nMin, &nSec);

	CTime tm2(nYear, nMonth, nDate,nHour,nMin,nSec);
	time_t currTime=tm2.GetTime();

	int nNightHour=2;
	int nNightMin=30;

	CTime tm3(nYear,nMonth,nDate,nNightHour,nNightMin,0);
	time_t nightTime=tm3.GetTime();

	if(nightTime-currTime>=0&&nightTime-currTime<180) return true;

	if(closeTime-currTime>=0&&closeTime-currTime<180) return true;

	return false;
}

bool StrategyBaseGridOpen_plus::timeRuleForClosePrice(char datadate[10],char datatime[10]){
	int nHour,nMin,nSec;
	sscanf_s(datatime,"%d:%d:%d",&nHour,&nMin,&nSec);
	int nBegin=14*3600+30*60;
	int nEnd=15*3600;
	int nTime=nHour*3600+nMin*60+nSec;
	if(nTime>=nBegin&&nTime<=nEnd) return true;
	return false;
}

void StrategyBaseGridOpen_plus::ReqOpenOrderInsert(MyOpenOrderType *pOpenOrder){
	InsertOrderField cOrder;
	strcpy(cOrder.ClientNo,"");
	strcpy(cOrder.CommodityNo,m_CommodityNo);
	strcpy(cOrder.InstrumentID,m_InstID);
	strcpy(cOrder.ExchangeID, m_ExchangeID);
	cOrder.Direction=pOpenOrder->Direction;
	cOrder.Offset=pOpenOrder->Offset;
	cOrder.OrderPrice=floor((pOpenOrder->LimitPrice+0.00001)/m_dOneTick)*m_dOneTick;
	cOrder.OrderVol=pOpenOrder->VolumeTotal;
	//cOrder.OrderLocalRef=pOpenOrder->OrderLocalRef;
	if(ThostTraderAPI||SgitTraderAPI){
		pOpenOrder->OrderId=-1;
		if(ThostTraderAPI)pThostTraderSpi->ReqOrderInsert(&cOrder,m_bCrossTradingDay,false,shmindex);
		else pSgitTraderSpi->ReqOrderInsert(&cOrder,m_bCrossTradingDay,false,shmindex);

		pOpenOrder->OrderLocalRef=cOrder.OrderLocalRef;
		pOpenOrder->FrontID=cOrder.FrontID;
		pOpenOrder->SessionID=cOrder.SessionID;

		CTime mCurrTime=CTime::GetCurrentTime();	
		CString str_mCurrTime=mCurrTime.Format("%Y%m%d %X");			
		int len=WideCharToMultiByte(CP_ACP,0,str_mCurrTime,str_mCurrTime.GetLength(),NULL,0,NULL,NULL);
		char *c_str_mCurrTime=new char[len+1];
		WideCharToMultiByte(CP_ACP,0,str_mCurrTime,str_mCurrTime.GetLength(),c_str_mCurrTime,len,NULL,NULL);
		c_str_mCurrTime[len]='\0';
		strcpy(pOpenOrder->SubmitDateAndTime,c_str_mCurrTime);
		strcpy(pOpenOrder->OpenTime,c_str_mCurrTime);
		delete c_str_mCurrTime;

		OpenOrderList.push_back((*pOpenOrder));
	}else{
		int iRetReqID;
		pEsunTraderSpi->ReqOrderInsert(&cOrder,&iRetReqID,m_bCrossTradingDay);
		pOpenOrder->OrderLocalRef=-1;
		pOpenOrder->OrderLocalRetReqID=iRetReqID;
		pOpenOrder->FrontID=cOrder.FrontID;
		pOpenOrder->SessionID=cOrder.SessionID;

		CTime mCurrTime=CTime::GetCurrentTime();	
		CString str_mCurrTime=mCurrTime.Format("%Y%m%d %X");			
		int len=WideCharToMultiByte(CP_ACP,0,str_mCurrTime,str_mCurrTime.GetLength(),NULL,0,NULL,NULL);
		char *c_str_mCurrTime=new char[len+1];
		WideCharToMultiByte(CP_ACP,0,str_mCurrTime,str_mCurrTime.GetLength(),c_str_mCurrTime,len,NULL,NULL);
		c_str_mCurrTime[len]='\0';
		strcpy(pOpenOrder->SubmitDateAndTime,c_str_mCurrTime);
		strcpy(pOpenOrder->OpenTime,c_str_mCurrTime);
		delete c_str_mCurrTime;

		OpenOrderList.push_back((*pOpenOrder));

		AcquireSRWLockExclusive(&g_srwLockReqId);
		ReqIdToShmIndex.insert(std::pair<int,int>(iRetReqID,shmindex));
		ReleaseSRWLockExclusive(&g_srwLockReqId);

		ReleaseSemaphore(OrderInsertSem, 1, NULL);
	}
}


void StrategyBaseGridOpen_plus::ReqCloseOrderInsert(MyCloseOrderType *pCloseOrder,char OpenOrderTime[21]){

	InsertOrderField cOrder;
	strcpy(cOrder.ClientNo,"");
	strcpy(cOrder.CommodityNo,m_CommodityNo);
	strcpy(cOrder.InstrumentID,m_InstID);
	strcpy(cOrder.ExchangeID, m_ExchangeID);
	cOrder.Direction=pCloseOrder->Direction;
	cOrder.Offset=pCloseOrder->Offset;
	cOrder.OrderPrice=floor((pCloseOrder->LimitPrice+0.00001)/m_dOneTick)*m_dOneTick;
	cOrder.OrderVol=pCloseOrder->VolumeTotal;
	//cOrder.OrderLocalRef=pCloseOrder->OrderLocalRef;
	if(ThostTraderAPI||SgitTraderAPI){
		CTime mCurrTime=CTime::GetCurrentTime();
		CString str_mCurrDate=mCurrTime.Format("%Y%m%d");			
		int lenDate=WideCharToMultiByte(CP_ACP,0,str_mCurrDate,str_mCurrDate.GetLength(),NULL,0,NULL,NULL);
		char *c_str_mCurrDate=new char[lenDate+1];
		WideCharToMultiByte(CP_ACP,0,str_mCurrDate,str_mCurrDate.GetLength(),c_str_mCurrDate,lenDate,NULL,NULL);
		c_str_mCurrDate[lenDate]='\0';		

		string strOpenOrderTime(OpenOrderTime);
		string strOpenDataDate=strOpenOrderTime.substr(0,strOpenOrderTime.find_first_of(" "));
		bool CloseToday=false;
		// �Աȿ������ںͽ�����
		if(0==strcmp(strOpenDataDate.c_str(),CTPTradingDay)) CloseToday=true;
		//if(strcmp(strOpenDataDate.c_str(),c_str_mCurrDate)==0){
		//	CloseToday=true;
		//}
		free(c_str_mCurrDate);

		pCloseOrder->OrderId=-1;
		if(ThostTraderAPI)pThostTraderSpi->ReqOrderInsert(&cOrder,m_bCrossTradingDay,CloseToday,shmindex);
		else pSgitTraderSpi->ReqOrderInsert(&cOrder,m_bCrossTradingDay,CloseToday,shmindex);
		pCloseOrder->OrderLocalRef=cOrder.OrderLocalRef;
		pCloseOrder->FrontID=cOrder.FrontID;
		pCloseOrder->SessionID=cOrder.SessionID;

		CString str_mCurrTime=mCurrTime.Format("%Y%m%d %X");			
		int len=WideCharToMultiByte(CP_ACP,0,str_mCurrTime,str_mCurrTime.GetLength(),NULL,0,NULL,NULL);
		char *c_str_mCurrTime=new char[len+1];
		WideCharToMultiByte(CP_ACP,0,str_mCurrTime,str_mCurrTime.GetLength(),c_str_mCurrTime,len,NULL,NULL);
		c_str_mCurrTime[len]='\0';
		strcpy(pCloseOrder->SubmitDateAndTime,c_str_mCurrTime);
		delete c_str_mCurrTime;	

		CloseOrderList.push_back((*pCloseOrder));

	}else{
		int iRetReqID;
		pEsunTraderSpi->ReqOrderInsert(&cOrder,&iRetReqID,m_bCrossTradingDay);
		pCloseOrder->OrderLocalRef=-1;
		pCloseOrder->OrderLocalRetReqID=iRetReqID;
		pCloseOrder->FrontID=cOrder.FrontID;
		pCloseOrder->SessionID=cOrder.SessionID;

		CTime mCurrTime=CTime::GetCurrentTime();	
		CString str_mCurrTime=mCurrTime.Format("%Y%m%d %X");			
		int len=WideCharToMultiByte(CP_ACP,0,str_mCurrTime,str_mCurrTime.GetLength(),NULL,0,NULL,NULL);
		char *c_str_mCurrTime=new char[len+1];
		WideCharToMultiByte(CP_ACP,0,str_mCurrTime,str_mCurrTime.GetLength(),c_str_mCurrTime,len,NULL,NULL);
		c_str_mCurrTime[len]='\0';
		strcpy(pCloseOrder->OpenTime,c_str_mCurrTime);
		strcpy(pCloseOrder->SubmitDateAndTime,c_str_mCurrTime);
		delete c_str_mCurrTime;

		CloseOrderList.push_back((*pCloseOrder));

		AcquireSRWLockExclusive(&g_srwLockReqId);
		ReqIdToShmIndex.insert(std::pair<int,int>(iRetReqID,shmindex));
		ReleaseSRWLockExclusive(&g_srwLockReqId);

		ReleaseSemaphore(OrderInsertSem, 1, NULL);
	}
}

void StrategyBaseGridOpen_plus::ReqOrderDelete(int pOrderId,int pOrderLocalRef,int pFrontID,int pSessionID)
{
	if(ThostTraderAPI||SgitTraderAPI){
		if(ThostTraderAPI)pThostTraderSpi->ReqOrderDeletePerOrderLocalRef(pOrderLocalRef, m_InstID, m_ExchangeID, pFrontID, pSessionID);
		else pSgitTraderSpi->ReqOrderDeletePerOrderLocalRef(pOrderLocalRef, m_InstID, pFrontID, pSessionID);
	}else{
		pEsunTraderSpi->ReqOrderDelete(pOrderId);
	}

	char line[200];
	sprintf(line,"%s,ReqOrderDelete,orderId,%d,ref:%d",mStrategyAndInstance.c_str(),pOrderId,pOrderLocalRef);
	WriteMsgToLogList(line);
}

void StrategyBaseGridOpen_plus::WriteMsgToLogList(char logline[200])
{
	Message logMsg;
	logMsg.type=STRATEGY_LOG;
	logMsg.AddData(logline,0,sizeof(char)*200);
	LogMessageList.AddTail(logMsg);
	ReleaseSemaphore(logSemaphore, 1, NULL);
}	 

void StrategyBaseGridOpen_plus::OnDisplayLocalCloseOrderList()
{
	char log[200];
	sprintf(log,"%s,���Կ��̼�=%.5f,��׼��=%.4f,����ʱ��:%s",mStrategyAndInstance.c_str(),mOpenPrice,
		mStrategyParams.BasePrice,mStrategyParams.CloseTime);
	CString str(log);
	pPubMsg->AddString(str);

	std::list<MyOpenOrderType>::iterator openorder_it;
	for(openorder_it = OpenOrderList.begin(); openorder_it != OpenOrderList.end();openorder_it++){
		LocalCLForDisplayField lcposition;
		strcpy(lcposition.StrategyID,mStrategyID);
		strcpy(lcposition.InstanceName,mInstanceName);
		strcpy(lcposition.InstCodeName,InstCodeName);

		lcposition.OrderID=openorder_it->OrderId;
		lcposition.CloseOrderSeqNo=0;
		lcposition.Direction=openorder_it->Direction;

		strcpy(lcposition.OpenTime,openorder_it->OpenTime);
		lcposition.LimitPrice=openorder_it->LimitPrice;
		lcposition.OpenOrderTradePrice=openorder_it->LimitPrice;
		lcposition.VolumeTotal=openorder_it->VolumeTotal;
		lcposition.OffSet=MORDER_OPEN;
		lcposition.ManualStopPrice=0.0;
		lcposition.MOrderType=openorder_it->MOrderType;

		Message posiMsg;
		posiMsg.type=ON_RSP_LOCAL_CL;
		posiMsg.AddData(&lcposition,0,sizeof(LocalCLForDisplayField));
		ScreenDisplayMsgList.AddTail(posiMsg);

		ReleaseSemaphore(ScreenDisplaySem, 1, NULL);
	}

	std::list<MyCloseOrderType>::iterator closeorder_it;
	for(closeorder_it = CloseOrderList.begin(); closeorder_it != CloseOrderList.end();closeorder_it++){
		if(closeorder_it->IsClosePofitOrder){
			LocalCLForDisplayField lcposition;
			strcpy(lcposition.StrategyID,mStrategyID);
			strcpy(lcposition.InstanceName,mInstanceName);
			strcpy(lcposition.InstCodeName,InstCodeName);

			lcposition.OrderID=-1;
			lcposition.CloseOrderSeqNo=closeorder_it->CloseOrderSeqNo;
			lcposition.Direction=closeorder_it->Direction;

			strcpy(lcposition.OpenTime,closeorder_it->OpenTime);
			if(closeorder_it->Direction==MORDER_BUY){
				lcposition.LimitPrice=closeorder_it->OpenOrderTradePrice+closeorder_it->mStoplossPoint*m_dOneTick;
				lcposition.OpenOrderTradePrice=closeorder_it->OpenOrderTradePrice;
			}else if(closeorder_it->Direction==MORDER_SELL){
				lcposition.LimitPrice=closeorder_it->OpenOrderTradePrice-closeorder_it->mStoplossPoint*m_dOneTick;
				lcposition.OpenOrderTradePrice=closeorder_it->OpenOrderTradePrice;
			}

			lcposition.VolumeTotal=closeorder_it->VolumeTotal;
			lcposition.OffSet=MORDER_STOPLOSSCLOSE;
			lcposition.ManualStopPrice=closeorder_it->ManualStopPrice;
			lcposition.MOrderType=closeorder_it->MOrderType;

			Message posiMsg;
			posiMsg.type=ON_RSP_LOCAL_CL;
			posiMsg.AddData(&lcposition,0,sizeof(LocalCLForDisplayField));
			ScreenDisplayMsgList.AddTail(posiMsg);

			ReleaseSemaphore(ScreenDisplaySem, 1, NULL);
		}
		LocalCLForDisplayField lcposition;
		strcpy(lcposition.StrategyID,mStrategyID);
		strcpy(lcposition.InstanceName,mInstanceName);
		strcpy(lcposition.InstCodeName,InstCodeName);

		lcposition.OrderID=closeorder_it->OrderId;
		lcposition.CloseOrderSeqNo=closeorder_it->CloseOrderSeqNo;
		lcposition.Direction=closeorder_it->Direction;

		strcpy(lcposition.OpenTime,closeorder_it->OpenTime);
		lcposition.LimitPrice=closeorder_it->LimitPrice;
		lcposition.OpenOrderTradePrice=closeorder_it->OpenOrderTradePrice;
		lcposition.VolumeTotal=closeorder_it->VolumeTotal;
		lcposition.OffSet=MORDER_CLOSE;
		lcposition.ManualStopPrice=0.0;
		lcposition.MOrderType=closeorder_it->MOrderType;

		Message posiMsg;
		posiMsg.type=ON_RSP_LOCAL_CL;
		posiMsg.AddData(&lcposition,0,sizeof(LocalCLForDisplayField));
		ScreenDisplayMsgList.AddTail(posiMsg);

		ReleaseSemaphore(ScreenDisplaySem, 1, NULL);
	}
}

void StrategyBaseGridOpen_plus::AddtoTipMsgListBox(char msgline[200])
{
	WaitForSingleObject(MainScreenFreshSem, INFINITE); 
	CString str(msgline);
	pPubMsg->AddString(str);
	ReleaseSemaphore(MainScreenFreshSem, 1, NULL);
}

void StrategyBaseGridOpen_plus::DisplayTradeOnScreen(OrderTradeMsg *pOrderTradeMsg,int mDirection,int mOpenOrClose,int mCloseProfitOrNot)
{
	WaitForSingleObject(MainScreenFreshSem, INFINITE); 
	int orderIndex=pTradesDetailsList->GetItemCount();
	CString itemname("");					
	itemname.Format(_T("%d"),orderIndex);
	pTradesDetailsList->InsertItem(orderIndex,(LPCTSTR)itemname);

	//CString csStrategyName(mStrategyName);
	CString csStrategyID(mStrategyID);
	CString csInstanceName(mInstanceName);

	//pTradesDetailsList->SetItemText(orderIndex,0,(LPCTSTR)csStrategyName);
	pTradesDetailsList->SetItemText(orderIndex,0,(LPCTSTR)csStrategyID);
	pTradesDetailsList->SetItemText(orderIndex,1,(LPCTSTR)csInstanceName);

	CString csInstCodeName(mStrategyParams.InstCodeName);
	pTradesDetailsList->SetItemText(orderIndex,2,(LPCTSTR)csInstCodeName);

	CString csTradeTime(pOrderTradeMsg->InsertOrTradeTime);
	pTradesDetailsList->SetItemText(orderIndex,3,(LPCTSTR)csTradeTime);

	CString direction("");
	if(mDirection==MORDER_SELL){
		direction=_T("��");
	}else if(mDirection==MORDER_BUY){
		direction=_T("��");
	}
	pTradesDetailsList->SetItemText(orderIndex,4,(LPCTSTR)direction);

	CString csTradePrice;
	csTradePrice.Format(_T("%.5f"),pOrderTradeMsg->Price);
	pTradesDetailsList->SetItemText(orderIndex,5,(LPCTSTR)csTradePrice);		

	CString vol;
	vol.Format(_T("%d"),pOrderTradeMsg->Volume);
	pTradesDetailsList->SetItemText(orderIndex,6,(LPCTSTR)vol);

	CString csOpenOrClose("");
	if(mOpenOrClose==MORDER_OPEN){
		csOpenOrClose=_T("��");
	}else if(mOpenOrClose==MORDER_CLOSE){
		if(mCloseProfitOrNot==1){
			csOpenOrClose=_T("ֹӯƽ");
		}else csOpenOrClose=_T("ֹ��ƽ");			
	}
	pTradesDetailsList->SetItemText(orderIndex,7,(LPCTSTR)csOpenOrClose);

	pTradesDetailsList->EnsureVisible(pTradesDetailsList->GetItemCount()-1,FALSE);
	ReleaseSemaphore(MainScreenFreshSem, 1, NULL);
}

void StrategyBaseGridOpen_plus::RecoverInstance(char cfgFileName[500])
{
	FILE *fptr=fopen(cfgFileName,"rb");
	fseek(fptr, 0, SEEK_SET);
	fread(&header, sizeof(mSerializeHeader), 1, fptr);
	int OpenOrderCount=header.OpenOrderCount;								
	int CloseOrderCount=header.CloseOrderCount;
	memcpy(&mOpenTimes,header.SpecificArea,sizeof(int));
	memcpy(&m_bStoplossClose,header.SpecificArea+sizeof(int),sizeof(bool));
	memcpy(&mStrategyParams.BasePrice,header.SpecificArea+sizeof(int)+sizeof(bool),sizeof(double));
	memcpy(&GridCloseOrderCount,header.SpecificArea+sizeof(int)+sizeof(bool)+sizeof(double),sizeof(GridCloseOrderCount));
	//	strcpy(header.StartTime,mStrategyParams.StartTime);
	//	strcpy(header.EndTime,mStrategyParams.EndTime);

	MyOpenOrderType openOrder;
	for(int i=0;i<OpenOrderCount;i++){
		fread(&openOrder, sizeof(MyOpenOrderType), 1, fptr);
		OpenOrderList.push_back(openOrder);

		//OrderIdToShmIndex.insert(std::pair<int,int>(openOrder.OrderId,GetShmindex()));
		// ���ָ̻��õ�
		if(openOrder.OrderLocalRef>0)OrderLocalRefToShmIndex.insert(std::pair<int,int>(openOrder.OrderLocalRef,GetShmindex()));
		else OrderIdToShmIndex.insert(std::pair<int,int>(openOrder.OrderId,GetShmindex()));
		string sfinal(mStrategyAndInstance);
		sfinal.append("_open");
		OrderIdToStrategyNameForDisplay.insert(std::pair<int,string>(openOrder.OrderId,sfinal));
	}
	MyCloseOrderType closeOrder;
	for(int i=0;i<CloseOrderCount;i++){
		fread(&closeOrder, sizeof(MyCloseOrderType), 1, fptr);
		CloseOrderList.push_back(closeOrder);
		//OrderIdToShmIndex.insert(std::pair<int,int>(closeOrder.OrderId,GetShmindex()));
		if(closeOrder.OrderLocalRef>0)OrderLocalRefToShmIndex.insert(std::pair<int,int>(closeOrder.OrderLocalRef,GetShmindex()));
		else OrderIdToShmIndex.insert(std::pair<int,int>(closeOrder.OrderId,GetShmindex()));
		string sfinal(mStrategyAndInstance);
		sfinal.append("_close");
		OrderIdToStrategyNameForDisplay.insert(std::pair<int,string>(closeOrder.OrderId,sfinal));
	}

	fclose(fptr);

	CTime mCurrTime=CTime::GetCurrentTime();

	CString str_mCurrTime=mCurrTime.Format("%X");			
	int len=WideCharToMultiByte(CP_ACP,0,str_mCurrTime,str_mCurrTime.GetLength(),NULL,0,NULL,NULL);
	char *c_str_mCurrTime=new char[len+1];
	WideCharToMultiByte(CP_ACP,0,str_mCurrTime,str_mCurrTime.GetLength(),c_str_mCurrTime,len,NULL,NULL);
	c_str_mCurrTime[len]='\0';	

	char log[300];
	sprintf(log,"%s,StrategyGridOpen,�����ָ�,��ǰʱ��=%s,Shmindex:%d",mStrategyAndInstance.c_str(),c_str_mCurrTime,GetShmindex());
	CString str(log);
	pPubMsg->AddString(str);
	WriteMsgToLogList(log);
	free(c_str_mCurrTime);

	FlushStrategyInfoToFile();
}

void StrategyBaseGridOpen_plus::FlushStrategyInfoToFile(){
	header.OpenOrderCount=OpenOrderList.size();
	header.CloseOrderCount=CloseOrderList.size();
	CTime mCurrTime=CTime::GetCurrentTime();
	CString str_mCurrTime=mCurrTime.Format("%Y-%m-%d %X");			
	int len=WideCharToMultiByte(CP_ACP,0,str_mCurrTime,str_mCurrTime.GetLength(),NULL,0,NULL,NULL);
	char *c_str_mCurrTime=new char[len+1];
	WideCharToMultiByte(CP_ACP,0,str_mCurrTime,str_mCurrTime.GetLength(),c_str_mCurrTime,len,NULL,NULL);
	c_str_mCurrTime[len]='\0';
	strcpy(header.StopTime,c_str_mCurrTime);
	free(c_str_mCurrTime);
	memcpy(header.SpecificArea,&mOpenTimes,sizeof(int));
	memcpy(header.SpecificArea+sizeof(int),&m_bStoplossClose,sizeof(bool));
	memcpy(header.SpecificArea+sizeof(int)+sizeof(bool),&mStrategyParams.BasePrice,sizeof(double));
	memcpy(header.SpecificArea+sizeof(int)+sizeof(bool)+sizeof(double),&GridCloseOrderCount,sizeof(GridCloseOrderCount));
	

	memcpy(StrategyfilePoint,&header,sizeof(mSerializeHeader));

	memset(StrategyfilePoint+sizeof(mSerializeHeader),0,(sizeof(MyCloseOrderType)+sizeof(MyOpenOrderType))*100);

	int OpenOrderIndex=0;	
	if(!OpenOrderList.empty()){
		std::list<MyOpenOrderType>::iterator openorder_ittmp;
		for(openorder_ittmp = OpenOrderList.begin(); openorder_ittmp != OpenOrderList.end();openorder_ittmp++){
			memcpy(StrategyfilePoint+sizeof(mSerializeHeader)+OpenOrderIndex*sizeof(MyOpenOrderType),&(*openorder_ittmp),sizeof(MyOpenOrderType));
			OpenOrderIndex++;
		}
	}
	int CloseOrderIndex=0;
	if(!CloseOrderList.empty()){
		std::list<MyCloseOrderType>::iterator closeorder_ittmp;
		for(closeorder_ittmp = CloseOrderList.begin(); closeorder_ittmp != CloseOrderList.end();closeorder_ittmp++){
			memcpy(StrategyfilePoint+sizeof(mSerializeHeader)+OpenOrderIndex*sizeof(MyOpenOrderType)+CloseOrderIndex*sizeof(MyCloseOrderType),&(*closeorder_ittmp),sizeof(MyCloseOrderType));
			CloseOrderIndex++;
		}
	}
	FlushViewOfFile(StrategyfilePoint,sizeof(mSerializeHeader)+(sizeof(MyCloseOrderType)+sizeof(MyOpenOrderType))*100);
}

wstring StrategyBaseGridOpen_plus::s2ws(const string& s)
{
	setlocale(LC_ALL, "chs"); 
	const char* _Source = s.c_str();
	size_t _Dsize = s.size() + 1;
	wchar_t *_Dest = new wchar_t[_Dsize];
	wmemset(_Dest, 0, _Dsize);
	mbstowcs(_Dest,_Source,_Dsize);
	wstring result = _Dest;
	delete []_Dest;
	setlocale(LC_ALL, "C");
	return result;
}

int StrategyBaseGridOpen_plus::CreateStrategyMapOfView(){

	CString strPathFile;
	::GetModuleFileName( NULL, strPathFile.GetBuffer( MAX_PATH ), MAX_PATH );
	strPathFile.ReleaseBuffer();
	strPathFile = strPathFile.Left(strPathFile.ReverseFind(_T('\\')));
	strPathFile+="\\Strategies\\TTimeOpenModel\\";
	char filename[500];
	ConvertCStringToCharArray(strPathFile,filename);
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
		FILE_SHARE_READ|FILE_SHARE_WRITE,//0,    // ����Ϊ���ļ����κγ��Ծ���ʧ��
		NULL,
		CREATE_ALWAYS,//OPEN_EXISTING,OPEN_ALWAYS,TRUNCATE_EXISTING,CREATE_ALWAYS
		FILE_ATTRIBUTE_NORMAL|FILE_FLAG_WRITE_THROUGH, //|FILE_FLAG_WRITE_THROUGH,����1��
		NULL);
	if (StrahFile == INVALID_HANDLE_VALUE)// �ļ���ʧ�ܷ��ؾ��Ϊ-1
		// �ⲽ������ԣ���ϸ������2
	{
		return (-1);		
	}

	// ����2 �����ڴ�ӳ���ļ�
	//DWORD dwFileSize = GetFileSize(hFile, NULL);

	header.OpenOrderCount=0;header.CloseOrderCount=0;
	strcpy(header.CodeName,InstCodeName);
	strcpy(header.StartTime,mStrategyParams.StartTime);
	strcpy(header.EndTime,mStrategyParams.EndTime);

	CTime mCurrTime=CTime::GetCurrentTime();
	CString str_mCurrTime=mCurrTime.Format("%Y-%m-%d %X");			
	int len=WideCharToMultiByte(CP_ACP,0,str_mCurrTime,str_mCurrTime.GetLength(),NULL,0,NULL,NULL);
	char *c_str_mCurrTime=new char[len+1];
	WideCharToMultiByte(CP_ACP,0,str_mCurrTime,str_mCurrTime.GetLength(),c_str_mCurrTime,len,NULL,NULL);
	c_str_mCurrTime[len]='\0';
	strcpy(header.StopTime,c_str_mCurrTime);
	free(c_str_mCurrTime);

	int xmShmindex=GetShmindex();
	CString csxmShmindex("");
	csxmShmindex.Format(_T("%d"),xmShmindex);

	CString FileMappingName(mStrategyAndInstance.c_str());
	StrategyInfoFileMap = CreateFileMapping(
		StrahFile, // �����ֵΪINVALID_HANDLE_VALUE,�ǺϷ��ģ��ϲ�һ�����԰�
		NULL,   // Ĭ�ϰ�ȫ��
		PAGE_READWRITE,   // �ɶ�д
		0, // 2��32λ��ʾ1��64λ��������ļ��ֽ�����
		// ���ֽڣ��ļ���СС��4Gʱ�����ֽ���ԶΪ0
		sizeof(mSerializeHeader)+(sizeof(MyCloseOrderType)+sizeof(MyOpenOrderType))*100,//dwFileSize, // ��Ϊ���ֽڣ�Ҳ��������Ҫ�Ĳ��������Ϊ0��ȡ�ļ���ʵ��С
		_T("XX"+FileMappingName+"Map"+csxmShmindex));
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
	memcpy(StrategyfilePoint,&header,sizeof(mSerializeHeader));
	FlushViewOfFile(StrategyfilePoint,sizeof(mSerializeHeader));

	return 0;
}

int StrategyBaseGridOpen_plus::OpenStrategyMapOfView(){

	CString strPathFile;
	::GetModuleFileName( NULL, strPathFile.GetBuffer( MAX_PATH ), MAX_PATH );
	strPathFile.ReleaseBuffer();
	strPathFile = strPathFile.Left(strPathFile.ReverseFind(_T('\\')));
	strPathFile+="\\Strategies\\TTimeOpenModel\\";
	char filename[500];
	ConvertCStringToCharArray(strPathFile,filename);
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
		FILE_SHARE_READ|FILE_SHARE_WRITE,//0,    // ����Ϊ���ļ����κγ��Ծ���ʧ��
		NULL,
		OPEN_EXISTING,//OPEN_EXISTING,OPEN_ALWAYS,TRUNCATE_EXISTING,CREATE_ALWAYS
		FILE_ATTRIBUTE_NORMAL|FILE_FLAG_WRITE_THROUGH, //|FILE_FLAG_WRITE_THROUGH,����1��
		NULL);
	if (StrahFile == INVALID_HANDLE_VALUE)// �ļ���ʧ�ܷ��ؾ��Ϊ-1
		// �ⲽ������ԣ���ϸ������2
	{
		return (-1);		
	}

	// ����2 �����ڴ�ӳ���ļ�
	//DWORD dwFileSize = GetFileSize(hFile, NULL);
	int xmShmindex=GetShmindex();
	CString csxmShmindex("");
	csxmShmindex.Format(_T("%d"),xmShmindex);

	CString FileMappingName(mStrategyAndInstance.c_str());
	StrategyInfoFileMap = CreateFileMapping(
		StrahFile, // �����ֵΪINVALID_HANDLE_VALUE,�ǺϷ��ģ��ϲ�һ�����԰�
		NULL,   // Ĭ�ϰ�ȫ��
		PAGE_READWRITE,   // �ɶ�д
		0, // 2��32λ��ʾ1��64λ��������ļ��ֽ�����
		// ���ֽڣ��ļ���СС��4Gʱ�����ֽ���ԶΪ0
		sizeof(mSerializeHeader)+(sizeof(MyCloseOrderType)+sizeof(MyOpenOrderType))*100,//dwFileSize, // ��Ϊ���ֽڣ�Ҳ��������Ҫ�Ĳ��������Ϊ0��ȡ�ļ���ʵ��С
		_T("XX"+FileMappingName+"Map"+csxmShmindex));
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

void StrategyBaseGridOpen_plus::CloseMap()
{
	//ɾ����Ӧ��cfg�ļ�
	UnmapViewOfFile(StrategyfilePoint);
	CloseHandle(StrategyInfoFileMap); 

	if(ClearInstanceCfgFile){

		CString strategyName(mStrategyName);
		char cInstanceName[50];
		GetInstanceName(cInstanceName);
		CString instranceName(cInstanceName);
		CString strPathFile;
		::GetModuleFileName( NULL, strPathFile.GetBuffer( MAX_PATH ), MAX_PATH );
		strPathFile.ReleaseBuffer();
		strPathFile = strPathFile.Left(strPathFile.ReverseFind(_T('\\')));
		strPathFile+="\\Strategies";

		std::list<ModelNode>::iterator model_itr;	
		for(model_itr = ModelList.begin(); model_itr != ModelList.end(); ++model_itr){
			CString ModelName(model_itr->ModelName);
			CString csModel1Path;
			csModel1Path.Append(strPathFile);
			csModel1Path+="\\";
			csModel1Path.Append(ModelName);
			std::list<StrategyNode>::iterator strategy_itr;			
			for(strategy_itr = model_itr->StrategyList.begin(); strategy_itr != model_itr->StrategyList.end(); ++strategy_itr){
				CString csStrategyName(strategy_itr->StrategyName);
				if(csStrategyName.CompareNoCase(strategyName)==0){
					CString csStrategy1Path;
					csStrategy1Path.Append(csModel1Path);
					csStrategy1Path+="\\";
					csStrategy1Path.Append(csStrategyName);
					std::list<StrategyInstanceNode>::iterator instance_itr;
					for(instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr){
						CString csInstanceName(instance_itr->InstanceName);
						if(csInstanceName.CompareNoCase(instranceName)==0){
							CString csInstanceCfg;
							csInstanceCfg.Append(csStrategy1Path);
							csInstanceCfg+="\\";
							csInstanceCfg.Append(csInstanceName);
							csInstanceCfg+=".cfg";
							DeleteFile((LPCWSTR)csInstanceCfg);	
						}
					}
				}
			}
		}
	}
}


void StrategyBaseGridOpen_plus::SetCloseLocalOrder(LocalCLForDisplayField *pCLOrder)
{

}

void StrategyBaseGridOpen_plus::SetStrategyID(char strategyId[50])
{
	strcpy(mStrategyID,strategyId);
}

void StrategyBaseGridOpen_plus::InitAction(){
	mTickCount=0;
	m_bSampleReady=false;
	mOpenRet=false;
	InitVariables();
}