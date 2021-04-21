#include "StdAfx.h"
#include "Strategy.h"
#include "MyStruct.h"
#include "TickDataList.h"
#include "OrderDataList.h"
#include "Message.h"
#include "MessageList.h"
#include <map>

using namespace std;

extern TickDataList TickList;
extern OrderDataList OrderList;
extern MapViewType* gMapView;
extern CListCtrl* pMDMsgDisplay;
extern HANDLE MDMainScreenFreshSem;
extern list<ModelNode> ModelList;
extern map<string, int> InstanceToShmindexMap;
extern MessageList LogMessageList;
extern HANDLE logSemaphore;
extern HANDLE localCLSem;
extern CListBox* pPubMsg;
extern HANDLE allmutexarray[NMAXSTRA];
extern HANDLE allemptysemarray[NMAXSTRA];
extern HANDLE allstoredsemarray[NMAXSTRA];

CStrategy::CStrategy(void)
{
	struct tm* ptTm;
	time_t nowtime;
	memset(&beginrun_date, 0, 10);
	time(&nowtime);
	ptTm = localtime(&nowtime);
	strftime(beginrun_date, 10, "%Y%m%d", ptTm);

	m_bIsRunning = false;
	ThostTraderAPI = false;
	SgitTraderAPI = false;

	strcpy(yesterdaydate, beginrun_date);
	DateChange = false;
}

CStrategy::~CStrategy(void)
{
}

void CStrategy::MessageProcess()
{
	//	char cInstanceName1[50];
	//	GetInstanceName(cInstanceName1);

	TickInfo* pDepthMarketData = 0;

	OrderTradeMsg pOrderTrade;

	ShmTickInfo curtick;
	TickInfo tickData;
	long offset;
	int mshmindex = GetShmindex();
	//CString csMutexName(gMapView->strategyarray[mshmindex].mutexname);
	HANDLE mymutex = allmutexarray[mshmindex];//OpenMutex(MUTEX_ALL_ACCESS,FALSE,csMutexName);
	strcpy(gMapView->strategyarray[mshmindex].CodeName, InstCodeName);
	//CString csEmptySem(gMapView->strategyarray[mshmindex].emptysemname);
	HANDLE emptysem = allemptysemarray[mshmindex];//OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE,csEmptySem); //指定想要的访问权限，SEMAPHORE_ALL_ACCESS 请求对事件对象的完全访问，SEMAPHORE_MODIFY_STATE 修改状态权限，使用 ReleaseSemaphore函数需要该权限；
	//CString csStoredSem(gMapView->strategyarray[mshmindex].storedsemname);
	HANDLE storedsem = allstoredsemarray[mshmindex];//OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE,csStoredSem);

	ShmMsg md_msg;
	ShmTickInfo shm_tick;
	bool shmmsg = false;
	TickInfo tick;
	int lasttickHour = -1;

	string xmStrategyAndInstance(mStrategyID);
	xmStrategyAndInstance.append("_");
	char cInstanceNametmp[50];
	GetInstanceName(cInstanceNametmp);
	xmStrategyAndInstance.append(cInstanceNametmp);
	InstanceToShmindexMap.insert(std::pair<string, int>(xmStrategyAndInstance, mshmindex));

	char log[200];
	sprintf(log, "%s Started,OneTick=%.2f,Mul=%d", xmStrategyAndInstance.c_str(), m_dOneTick, Multiplier);
	CString str(log);
	pPubMsg->AddString(str);

	while (m_bIsRunning)
	{
		shmmsg = false;
		WaitForSingleObject(storedsem, INFINITE);
		WaitForSingleObject(mymutex, INFINITE);
		if (gMapView->strategyarray[mshmindex].nstored > 0) {
			offset = gMapView->strategyarray[mshmindex].nget;
			memset(&md_msg, 0, sizeof(ShmMsg));
			memcpy(&md_msg, &gMapView->strategyarray[mshmindex].MDArray[offset], sizeof(ShmMsg));
			shmmsg = true;
			if (++(gMapView->strategyarray[mshmindex].nget) >= NMAXTICK)
				gMapView->strategyarray[mshmindex].nget = 0;
			gMapView->strategyarray[mshmindex].nempty++;
			gMapView->strategyarray[mshmindex].nstored--;
		}
		ReleaseMutex(mymutex);
		ReleaseSemaphore(emptysem, 1, NULL);
		if (shmmsg) {
			if (md_msg.msgtype == MD_MSG) {
				ShmTickInfo shmtick;
				memset(&shmtick, 0, sizeof(ShmTickInfo));
				memcpy(&shmtick, &md_msg.msg, sizeof(ShmTickInfo));
				strcpy(tick.ordername, gMapView->strategyarray[mshmindex].CodeName);

				if (strcmp(tick.ordername, shmtick.InstCodeName) == 0) {
					strcpy(tick.updatetime, shmtick.updatetime);
					tick.ask1 = shmtick.ask1;
					tick.bid1 = shmtick.bid1;
					tick.askvol1 = shmtick.ask1vol;
					tick.bidvol1 = shmtick.bid1vol;
					tick.price = shmtick.price;
					tick.upperLimitPrice = shmtick.upperLimitPrice;
					tick.lowerLimitPrice = shmtick.lowerLimitPrice;
					tick.openprice = shmtick.openPrice;

					int nHour, nMin, nSec;
					sscanf_s(tick.updatetime, "%d:%d:%d", &nHour, &nMin, &nSec);
					char newdate[10];
					struct tm* ptTm;
					time_t nowtime;
					time(&nowtime);
					ptTm = localtime(&nowtime);
					strftime(newdate, 10, "%Y%m%d", ptTm);

					if (abs(nHour - ptTm->tm_hour) > 3 && abs(nHour - ptTm->tm_hour) != 23)continue;

					if (ptTm->tm_hour >= 1 && ptTm->tm_hour < 23 && DateChange) {
						DateChange = false;
						strcpy(yesterdaydate, beginrun_date);
					}

					if ((lasttickHour - nHour) > 10) {
						//new Day begin
						if (DateChange == false && strcmp(newdate, beginrun_date) >= 0 && lasttickHour == 23 && nHour == 0) {
							//只处理跨零点的情况,在此基础上加上一天
							int nYearT = atoi(beginrun_date) / 10000;
							int nMonthT = (atoi(beginrun_date) % 10000) / 100;
							int nDateT = (atoi(beginrun_date) % 10000) % 100;
							CTime x(nYearT, nMonthT, nDateT, 0, 0, 0);
							time_t newt = x.GetTime() + 86400;
							ptTm = localtime(&newt);

							strcpy(yesterdaydate, beginrun_date);
							memset(&beginrun_date, 0, 10);
							strftime(beginrun_date, 10, "%Y%m%d", ptTm);
							DateChange = true;
						}
					}
					else if (((lasttickHour != 0 && lasttickHour != 23) || (nHour != 0 && nHour != 23) || strcmp(beginrun_date, "") == 0) && !DateChange) {
						//程序初次启动或处于正常时段,则用系统日期
						strcpy(beginrun_date, newdate);
					}
					if (nHour == 23 && DateChange) {
						strcpy(tick.datadate, yesterdaydate);
					}
					else {
						strcpy(tick.datadate, beginrun_date);
					}
					lasttickHour = nHour;

					if (tick.price > 0 && tick.price < 999999)OnRtnDepthMarketData(&tick);

					//Display the MD Msg to Main Screen
					CString codeName(tick.ordername);
					CString strategyID(mStrategyID);
					char cInstanceName[50];
					GetInstanceName(cInstanceName);
					CString instranceName(cInstanceName);
					char pr[30];
					double price = tick.price;
					if (price > 0 && price < 999999)sprintf_s(pr, "%.4f", price);
					else sprintf_s(pr, "%.4f", 0.0);
					CString priceString(pr);

					WaitForSingleObject(MDMainScreenFreshSem, INFINITE);
					int lineCount = pMDMsgDisplay->GetItemCount();
					bool find = false;
					for (int i = 0; i < pMDMsgDisplay->GetItemCount(); i++) {
						CString StrategyID = pMDMsgDisplay->GetItemText(i, 0);
						CString InstanceName = pMDMsgDisplay->GetItemText(i, 1);
						CString InstName = pMDMsgDisplay->GetItemText(i, 2);
						CString PriceValue = pMDMsgDisplay->GetItemText(i, 3);

						if (codeName.CompareNoCase(InstName) == 0 && strategyID.CompareNoCase(StrategyID) == 0 && instranceName.CompareNoCase(InstanceName) == 0) {
							if (PriceValue.CompareNoCase(priceString) != 0) {
								pMDMsgDisplay->SetItemText(i, 3, (LPCTSTR)priceString);
							}
							find = true;
						}
						else if (strategyID.CompareNoCase(StrategyID) == 0 && instranceName.CompareNoCase(InstanceName) == 0) {
							pMDMsgDisplay->DeleteItem(i);
							i--;
						}
					}
					if (!find) {
						int orderIndex = pMDMsgDisplay->GetItemCount();
						CString itemname("");
						itemname.Format(_T("%d"), orderIndex);
						pMDMsgDisplay->InsertItem(orderIndex, (LPCTSTR)itemname);

						pMDMsgDisplay->SetItemText(orderIndex, 0, (LPCTSTR)strategyID);
						pMDMsgDisplay->SetItemText(orderIndex, 1, (LPCTSTR)instranceName);
						pMDMsgDisplay->SetItemText(orderIndex, 2, (LPCTSTR)codeName);
						pMDMsgDisplay->SetItemText(orderIndex, 3, (LPCTSTR)priceString);
					}
					ReleaseSemaphore(MDMainScreenFreshSem, 1, NULL);
				}//End for Code Name is same
				//
			}
			else if (md_msg.msgtype == TD_MSG) {
				memcpy(&pOrderTrade, &md_msg.msg, sizeof(OrderTradeMsg));
				if (pOrderTrade.OrderType == ON_RTN_ORDER)OnRtnOrder(&pOrderTrade);
				else if (pOrderTrade.OrderType == ON_RTN_TRADE)OnRtnTrade(&pOrderTrade);
			}
			else if (md_msg.msgtype == ON_STRA_RSP_ORDER_INSERT) {
				ShmRspOrderInsert pRspOrderInsert;
				memcpy(&pRspOrderInsert, &md_msg.msg, sizeof(ShmRspOrderInsert));
				OnRspOrderInsert(&pRspOrderInsert);
			}
			else if (md_msg.msgtype == ON_STRA_DISPLAY_LOCAL_CL) {
				WaitForSingleObject(localCLSem, INFINITE);
				OnDisplayLocalCloseOrderList();
				ReleaseSemaphore(localCLSem, 1, NULL);
			}
			else if (md_msg.msgtype == ON_STRA_STRATEGY_RESET) {
				ResetAction();
			}
			else if (md_msg.msgtype == ON_STRA_STRATEGY_EXIT) {
				m_bIsRunning = false;
				//重置缓冲区信息,重置empty&stored信号量

				gMapView->strategyarray[mshmindex].StrategyStarted = false;
				SetShmindex(-1);

				WaitForSingleObject(mymutex, INFINITE);
				gMapView->strategyarray[mshmindex].StrategyStarted = false;
				SetShmindex(-1);
				gMapView->strategyarray[mshmindex].nget = 0;
				gMapView->strategyarray[mshmindex].nput = 0;
				gMapView->strategyarray[mshmindex].nempty = NMAXTICK;
				gMapView->strategyarray[mshmindex].nstored = 0;
				//放空stored信号量
				int retStoredSemValue;
				bool x = ReleaseSemaphore(storedsem, 1, (LPLONG)&retStoredSemValue);
				if (x) {
					retStoredSemValue++;
				}
				else {
					retStoredSemValue = NMAXTICK;
				}
				for (int i = 0; i < retStoredSemValue; i++) WaitForSingleObject(storedsem, 100);

				//放空empty信号量
				int retEmpty = 0;
				while (ReleaseSemaphore(emptysem, 1, NULL))retEmpty++;

				ReleaseMutex(mymutex);
				CloseMap();

				map<string, int>::iterator iter;
				iter = InstanceToShmindexMap.find(xmStrategyAndInstance);
				if (iter != InstanceToShmindexMap.end()) {
					InstanceToShmindexMap.erase(iter);
				}

				exitAction();

				//Added on 2017-07
				char log[200];
				sprintf(log, "%s,Stop", xmStrategyAndInstance.c_str());
				Message logMsg;
				logMsg.type = STRATEGY_LOG;
				logMsg.AddData(log, 0, sizeof(char) * 200);
				LogMessageList.AddTail(logMsg);
				ReleaseSemaphore(logSemaphore, 1, NULL);
				//End 重置empty&stored信号量
				return;
			}
		}
	}//end while
}

void CStrategy::ConvertCStringToCharArray(CString csSource, char* rtnCharArray)
{
	int cslen = WideCharToMultiByte(CP_ACP, 0, csSource, csSource.GetLength(), NULL, 0, NULL, NULL);
	char* carray = new char[cslen + 1];
	WideCharToMultiByte(CP_ACP, 0, csSource, csSource.GetLength(), carray, cslen, NULL, NULL);
	carray[cslen] = '\0';
	strcpy(rtnCharArray, carray);
	delete carray;
}