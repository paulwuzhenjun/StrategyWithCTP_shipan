#include "StdAfx.h"
#include "DispatchTdMsgThread.h"
#include "MyStruct.h"
#include "OrderDataList.h"
#include <map>
#include "Message.h"
#include "MessageList.h"
#include "GlobalFunc.h"

#define MMF_FILE_SIZE (2048*2048)

using namespace std;

extern MapViewType* gMapView;
extern OrderDataList OrderList;
extern MessageList LogMessageList;
extern map<int, int> ReqIdToShmIndex;
extern map<int, int> OrderIdToShmIndex;
extern map<int, int> OrderLocalRefToShmIndex;
extern SRWLOCK  g_srwLockReqId;
extern SRWLOCK  g_srwLockOrderId;
extern SRWLOCK g_srwLockOrderLocalRef;

extern SRWLOCK  g_srwLockReqIdStra;
extern map<int, ManualOrder> ReqIdToStrategyManualOrder;
extern SRWLOCK  g_srwLockOrderIdStra;
extern map<int, ManualOrder> OrderIdToStrategyManualOrder;
extern HANDLE logSemaphore;
extern HANDLE DispatchTdSem;
extern HANDLE OrderInsertSem;
extern HANDLE CreateThreadSem;
extern GlobalFunc globalFuncUtil;

extern HANDLE allmutexarray[NMAXSTRA];
extern HANDLE allemptysemarray[NMAXSTRA];
extern HANDLE allstoredsemarray[NMAXSTRA];

DispatchTdMsgThread::DispatchTdMsgThread(void)
{
	m_bIsRunning = true;

	struct tm* ptTm;
	time_t nowtime;
	memset(&beginrun_date, 0, 10);
	time(&nowtime);
	ptTm = localtime(&nowtime);
	strftime(beginrun_date, 10, "%Y%m%d", ptTm);
}

DispatchTdMsgThread::~DispatchTdMsgThread(void)
{
}

void DispatchTdMsgThread::DispatchTdMsg()
{
	OrderTradeMsg pOrderTrade;
	ShmMsg td_msg;
	while (m_bIsRunning) {
		WaitForSingleObject(DispatchTdSem, INFINITE);

		if (!OrderList.DataListCore.IsEmpty())
		{
			OrderTradeMsg order = OrderList.GetHead();
			memset(&pOrderTrade, 0, sizeof(OrderTradeMsg));
			memcpy(&pOrderTrade, &order, sizeof(OrderTradeMsg));
			int shmindex = -1;
			if (pOrderTrade.OrderType == ON_TD_RSP_ORDER_INSERT) {
				int iRetReqID = pOrderTrade.ActionLocalNo;
				int OrderID = pOrderTrade.OrderSysId;
				Sleep(1000);
				WaitForSingleObject(OrderInsertSem, 500);

				AcquireSRWLockShared(&g_srwLockReqId);
				if (!ReqIdToShmIndex.empty()) {
					map<int, int>::iterator iter;
					iter = ReqIdToShmIndex.find(iRetReqID);
					if (iter != ReqIdToShmIndex.end()) {
						shmindex = iter->second;
						TRACE("TD_RSP_ORDER_INSERT=%d\n", shmindex);
						ShmRspOrderInsert pRspOrderInsertMsg;
						//CString csMutexName(gMapView->strategyarray[shmindex].mutexname);
						HANDLE mymutex = allmutexarray[shmindex];//OpenMutex(MUTEX_ALL_ACCESS,FALSE,csMutexName);
						//CString csEmptySem(gMapView->strategyarray[shmindex].emptysemname);
						HANDLE emptysem = allemptysemarray[shmindex];//OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE,csEmptySem); //指定想要的访问权限，SEMAPHORE_ALL_ACCESS 请求对事件对象的完全访问，SEMAPHORE_MODIFY_STATE 修改状态权限，使用 ReleaseSemaphore函数需要该权限；
						//CString csStoredSem(gMapView->strategyarray[shmindex].storedsemname);
						HANDLE storedsem = allstoredsemarray[shmindex];//OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE,csStoredSem);

						long offset;
						WaitForSingleObject(emptysem, INFINITE);
						WaitForSingleObject(mymutex, INFINITE);
						if (gMapView->strategyarray[shmindex].nempty > 0) {
							offset = gMapView->strategyarray[shmindex].nput;
							if (++(gMapView->strategyarray[shmindex].nput) >= NMAXTICK)
								gMapView->strategyarray[shmindex].nput = 0;
							td_msg.msgtype = ON_STRA_RSP_ORDER_INSERT;
							pRspOrderInsertMsg.OrderID = OrderID;
							pRspOrderInsertMsg.iRetReqID = iRetReqID;
							memcpy(&td_msg.msg, &pRspOrderInsertMsg, sizeof(ShmRspOrderInsert));
							memcpy(&gMapView->strategyarray[shmindex].MDArray[offset], &td_msg, sizeof(ShmMsg));
							gMapView->strategyarray[shmindex].nstored++;
							gMapView->strategyarray[shmindex].nempty--;
						}
						ReleaseMutex(mymutex);
						ReleaseSemaphore(storedsem, 1, NULL);
						//更新OrderID 和 ShmIndex的对应关系
						AcquireSRWLockExclusive(&g_srwLockOrderId);
						OrderIdToShmIndex.insert(std::pair<int, int>(OrderID, shmindex));
						ReleaseSRWLockExclusive(&g_srwLockOrderId);
					}
				}
				ReleaseSRWLockShared(&g_srwLockReqId);

				if (shmindex == -1) {
					//未找到对应的策略缓存,可能属于手工提交的订单,查找对应的策略名称
					AcquireSRWLockShared(&g_srwLockReqIdStra);
					map<int, ManualOrder>::iterator iter_stra;
					iter_stra = ReqIdToStrategyManualOrder.find(iRetReqID);
					if (iter_stra != ReqIdToStrategyManualOrder.end()) {
						ManualOrder manualOrder;
						strcpy(manualOrder.StrategyName, iter_stra->second.StrategyName);
						manualOrder.Direction = iter_stra->second.Direction;
						manualOrder.subprice = iter_stra->second.subprice;

						AcquireSRWLockExclusive(&g_srwLockOrderIdStra);
						OrderIdToStrategyManualOrder.insert(std::pair<int, ManualOrder>(OrderID, manualOrder));
						ReleaseSRWLockExclusive(&g_srwLockOrderIdStra);
						//ReqIdToStrategyManualOrder.erase(iter_stra);
					}
					ReleaseSRWLockShared(&g_srwLockReqIdStra);
				}
			}
			else if (pOrderTrade.OrderType == ON_RTN_ORDER || pOrderTrade.OrderType == ON_RTN_TRADE) {
				//if it is OnRtnOrder Or OnRtnTrade
				int OrderID = pOrderTrade.OrderSysId;
				AcquireSRWLockShared(&g_srwLockOrderId);
				if (!OrderIdToShmIndex.empty()) {
					map<int, int>::iterator iter;
					iter = OrderIdToShmIndex.find(OrderID);
					if (iter != OrderIdToShmIndex.end()) {
						shmindex = iter->second;

						//CString csMutexName(gMapView->strategyarray[shmindex].mutexname);
						HANDLE mymutex = allmutexarray[shmindex];//OpenMutex(MUTEX_ALL_ACCESS,FALSE,csMutexName);
						//CString csEmptySem(gMapView->strategyarray[shmindex].emptysemname);
						HANDLE emptysem = allemptysemarray[shmindex];//OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE,csEmptySem); //指定想要的访问权限，SEMAPHORE_ALL_ACCESS 请求对事件对象的完全访问，SEMAPHORE_MODIFY_STATE 修改状态权限，使用 ReleaseSemaphore函数需要该权限；
						//CString csStoredSem(gMapView->strategyarray[shmindex].storedsemname);
						HANDLE storedsem = allstoredsemarray[shmindex];//OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE,csStoredSem);

						long offset;
						WaitForSingleObject(emptysem, INFINITE);
						WaitForSingleObject(mymutex, INFINITE);
						if (gMapView->strategyarray[shmindex].nempty > 0) {
							offset = gMapView->strategyarray[shmindex].nput;
							if (++(gMapView->strategyarray[shmindex].nput) >= NMAXTICK)
								gMapView->strategyarray[shmindex].nput = 0;
							td_msg.msgtype = TD_MSG;
							memcpy(&td_msg.msg, &pOrderTrade, sizeof(OrderTradeMsg));
							memcpy(&gMapView->strategyarray[shmindex].MDArray[offset], &td_msg, sizeof(ShmMsg));
							gMapView->strategyarray[shmindex].nstored++;
							gMapView->strategyarray[shmindex].nempty--;
						}
						ReleaseMutex(mymutex);
						ReleaseSemaphore(storedsem, 1, NULL);
					}
				}
				ReleaseSRWLockShared(&g_srwLockOrderId);

				if (shmindex == -1) {
					//未找到对应的策略缓存,可能属于手工提交的订单,查找对应的策略名称
					AcquireSRWLockExclusive(&g_srwLockOrderIdStra);
					map<int, ManualOrder>::iterator iter_stra;
					iter_stra = OrderIdToStrategyManualOrder.find(OrderID);
					if (iter_stra != OrderIdToStrategyManualOrder.end()) {
						if (pOrderTrade.OrderType == ON_RTN_TRADE) {
							TradeLogType trade;
							strcpy(trade.StrategyID, iter_stra->second.StrategyName);
							int nYear, nMonth, nDate, nHour, nMin, nSec;
							sscanf_s(pOrderTrade.InsertOrTradeTime, "%d-%d-%d %d:%d:%d", &nYear, &nMonth, &nDate, &nHour, &nMin, &nSec);
							sprintf(trade.tradingday, "%2d%02d%02d", nYear, nMonth, nDate);
							trade.tradeprice = pOrderTrade.Price;
							trade.submitprice = iter_stra->second.subprice;

							if (pOrderTrade.Direction == iter_stra->second.Direction) {
								trade.qty = pOrderTrade.VolumeTraded;
							}
							else trade.qty = -pOrderTrade.VolumeTraded;
							trade.fee = pOrderTrade.MatchFee;

							Message logMsg;
							logMsg.type = TRADE_LOG;
							logMsg.AddData(&trade, 0, sizeof(TradeLogType));
							LogMessageList.AddTail(logMsg);
							ReleaseSemaphore(logSemaphore, 1, NULL);
						}
					}
					ReleaseSRWLockExclusive(&g_srwLockOrderIdStra);
				}
			}
			else if (pOrderTrade.OrderType == ON_THOST_RTN_ORDER || pOrderTrade.OrderType == ON_THOST_RTN_TRADE) {
				//if it is OnRtnOrder Or OnRtnTrade
				int OrderID = pOrderTrade.OrderSysId;
				int OrderLocalRef = pOrderTrade.OrderLocalRef;
				char line[200];
				sprintf(line, "DispatchTdMsg,orderSysId:%d,ref:%d", OrderID, OrderLocalRef);
				globalFuncUtil.WriteMsgToLogList(line);
				AcquireSRWLockShared(&g_srwLockOrderId);
				if (!OrderIdToShmIndex.empty()) {
					map<int, int>::iterator iter;
					iter = OrderIdToShmIndex.find(OrderID);
					if (iter != OrderIdToShmIndex.end()) {
						shmindex = iter->second;

						//CString csMutexName(gMapView->strategyarray[shmindex].mutexname);
						HANDLE mymutex = allmutexarray[shmindex];//OpenMutex(MUTEX_ALL_ACCESS,FALSE,csMutexName);
						//CString csEmptySem(gMapView->strategyarray[shmindex].emptysemname);
						HANDLE emptysem = allemptysemarray[shmindex];//OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE,csEmptySem); //指定想要的访问权限，SEMAPHORE_ALL_ACCESS 请求对事件对象的完全访问，SEMAPHORE_MODIFY_STATE 修改状态权限，使用 ReleaseSemaphore函数需要该权限；
						//CString csStoredSem(gMapView->strategyarray[shmindex].storedsemname);
						HANDLE storedsem = allstoredsemarray[shmindex];//OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE,csStoredSem);

						long offset;
						WaitForSingleObject(emptysem, INFINITE);
						WaitForSingleObject(mymutex, INFINITE);
						if (gMapView->strategyarray[shmindex].nempty > 0) {
							offset = gMapView->strategyarray[shmindex].nput;
							if (++(gMapView->strategyarray[shmindex].nput) >= NMAXTICK)
								gMapView->strategyarray[shmindex].nput = 0;
							td_msg.msgtype = TD_MSG;

							if (pOrderTrade.OrderType == ON_THOST_RTN_ORDER)pOrderTrade.OrderType = ON_RTN_ORDER;
							else pOrderTrade.OrderType = ON_RTN_TRADE;

							memcpy(&td_msg.msg, &pOrderTrade, sizeof(OrderTradeMsg));
							memcpy(&gMapView->strategyarray[shmindex].MDArray[offset], &td_msg, sizeof(ShmMsg));
							gMapView->strategyarray[shmindex].nstored++;
							gMapView->strategyarray[shmindex].nempty--;
						}
						ReleaseMutex(mymutex);
						ReleaseSemaphore(storedsem, 1, NULL);
					}
				}
				ReleaseSRWLockShared(&g_srwLockOrderId);

				if (shmindex == -1) {
					//未找到对应的策略缓存,可能是还未更新OrderSysID,查找OrderRef和ShmIndex对应map
					map<int, int>::iterator iter_orderlocalref;
					AcquireSRWLockShared(&g_srwLockOrderLocalRef);
					iter_orderlocalref = OrderLocalRefToShmIndex.find(OrderLocalRef);
					if (iter_orderlocalref != OrderLocalRefToShmIndex.end()) {
						shmindex = iter_orderlocalref->second;
						//更新OrderID 和 ShmIndex的对应关系,内盘使用orderlocalref对应
						//AcquireSRWLockExclusive(&g_srwLockOrderId);
						//OrderIdToShmIndex.insert(std::pair<int,int>(OrderID,shmindex));
						//ReleaseSRWLockExclusive(&g_srwLockOrderId);
						//将回报存入策略对应的缓存
						//CString csMutexName(gMapView->strategyarray[shmindex].mutexname);
						HANDLE mymutex = allmutexarray[shmindex];//OpenMutex(MUTEX_ALL_ACCESS,FALSE,csMutexName);
						//CString csEmptySem(gMapView->strategyarray[shmindex].emptysemname);
						HANDLE emptysem = allemptysemarray[shmindex];//OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE,csEmptySem); //指定想要的访问权限，SEMAPHORE_ALL_ACCESS 请求对事件对象的完全访问，SEMAPHORE_MODIFY_STATE 修改状态权限，使用 ReleaseSemaphore函数需要该权限；
						//CString csStoredSem(gMapView->strategyarray[shmindex].storedsemname);
						HANDLE storedsem = allstoredsemarray[shmindex];//OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE,csStoredSem);

						long offset;
						WaitForSingleObject(emptysem, INFINITE);
						WaitForSingleObject(mymutex, INFINITE);
						if (gMapView->strategyarray[shmindex].nempty > 0) {
							offset = gMapView->strategyarray[shmindex].nput;
							if (++(gMapView->strategyarray[shmindex].nput) >= NMAXTICK)
								gMapView->strategyarray[shmindex].nput = 0;
							td_msg.msgtype = TD_MSG;

							if (pOrderTrade.OrderType == ON_THOST_RTN_ORDER)pOrderTrade.OrderType = ON_RTN_ORDER;
							else pOrderTrade.OrderType = ON_RTN_TRADE;

							memcpy(&td_msg.msg, &pOrderTrade, sizeof(OrderTradeMsg));
							memcpy(&gMapView->strategyarray[shmindex].MDArray[offset], &td_msg, sizeof(ShmMsg));
							gMapView->strategyarray[shmindex].nstored++;
							gMapView->strategyarray[shmindex].nempty--;
						}
						ReleaseMutex(mymutex);
						ReleaseSemaphore(storedsem, 1, NULL);

						ReleaseSRWLockShared(&g_srwLockOrderLocalRef);
					}
					else {
						ReleaseSRWLockShared(&g_srwLockOrderLocalRef);
						//仍未找到对应的策略缓存,属于手工提交的订单,查找对应的策略名称
						AcquireSRWLockExclusive(&g_srwLockOrderIdStra);
						map<int, ManualOrder>::iterator iter_stra;
						iter_stra = OrderIdToStrategyManualOrder.find(OrderID);
						if (iter_stra != OrderIdToStrategyManualOrder.end()) {
							if (pOrderTrade.OrderType == ON_THOST_RTN_TRADE) {
								TradeLogType trade;
								strcpy(trade.StrategyID, iter_stra->second.StrategyName);
								int nYear, nMonth, nDate, nHour, nMin, nSec;
								sscanf_s(pOrderTrade.InsertOrTradeTime, "%d-%d-%d %d:%d:%d", &nYear, &nMonth, &nDate, &nHour, &nMin, &nSec);
								sprintf(trade.tradingday, "%2d%02d%02d", nYear, nMonth, nDate);
								trade.tradeprice = pOrderTrade.Price;
								trade.submitprice = iter_stra->second.subprice;

								if (pOrderTrade.Direction == iter_stra->second.Direction) {
									trade.qty = pOrderTrade.VolumeTraded;
								}
								else trade.qty = -pOrderTrade.VolumeTraded;
								trade.fee = pOrderTrade.MatchFee;

								Message logMsg;
								logMsg.type = TRADE_LOG;
								logMsg.AddData(&trade, 0, sizeof(TradeLogType));
								LogMessageList.AddTail(logMsg);
								ReleaseSemaphore(logSemaphore, 1, NULL);
							}
						}
						ReleaseSRWLockExclusive(&g_srwLockOrderIdStra);
					}
				}
			}
			else if (pOrderTrade.OrderType == ON_RSP_QRY_TRADE) {
				//if it is OnRtnOrder Or OnRtnTrade
				int OrderID = pOrderTrade.OrderSysId;
				AcquireSRWLockShared(&g_srwLockOrderId);
				if (!OrderIdToShmIndex.empty()) {
					map<int, int>::iterator iter;
					iter = OrderIdToShmIndex.find(OrderID);
					if (iter != OrderIdToShmIndex.end()) {
						shmindex = iter->second;

						//shmindex=pOrderTrade.shmindex;
						//CString csMutexName(gMapView->strategyarray[shmindex].mutexname);
						HANDLE mymutex = allmutexarray[shmindex];//OpenMutex(MUTEX_ALL_ACCESS,FALSE,csMutexName);
						//CString csEmptySem(gMapView->strategyarray[shmindex].emptysemname);
						HANDLE emptysem = allemptysemarray[shmindex];//OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE,csEmptySem); //指定想要的访问权限，SEMAPHORE_ALL_ACCESS 请求对事件对象的完全访问，SEMAPHORE_MODIFY_STATE 修改状态权限，使用 ReleaseSemaphore函数需要该权限；
						//CString csStoredSem(gMapView->strategyarray[shmindex].storedsemname);
						HANDLE storedsem = allstoredsemarray[shmindex];//OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE,csStoredSem);

						long offset;
						WaitForSingleObject(emptysem, INFINITE);
						WaitForSingleObject(mymutex, INFINITE);
						if (gMapView->strategyarray[shmindex].nempty > 0) {
							offset = gMapView->strategyarray[shmindex].nput;
							if (++(gMapView->strategyarray[shmindex].nput) >= NMAXTICK)
								gMapView->strategyarray[shmindex].nput = 0;
							td_msg.msgtype = TD_MSG;
							pOrderTrade.OrderType = ON_RTN_TRADE;
							memcpy(&td_msg.msg, &pOrderTrade, sizeof(OrderTradeMsg));
							memcpy(&gMapView->strategyarray[shmindex].MDArray[offset], &td_msg, sizeof(ShmMsg));
							gMapView->strategyarray[shmindex].nstored++;
							gMapView->strategyarray[shmindex].nempty--;
						}
						ReleaseMutex(mymutex);
						ReleaseSemaphore(storedsem, 1, NULL);
					}
				}
				ReleaseSRWLockShared(&g_srwLockOrderId);
				//若shmindex==-1,则未找到对应的策略缓存,不做处理
			}
			else if (pOrderTrade.OrderType == ON_TD_STRATEGY_EXIT) {
				ShmStrategyAction pShmStrategyAction;
				shmindex = pOrderTrade.shmindex;
				pShmStrategyAction.actionType = pOrderTrade.OrderType;
				pShmStrategyAction.shmindex = shmindex;

				//CString csMutexName(gMapView->strategyarray[shmindex].mutexname);
				HANDLE mymutex = allmutexarray[shmindex];//OpenMutex(MUTEX_ALL_ACCESS,FALSE,csMutexName);
				//CString csEmptySem(gMapView->strategyarray[shmindex].emptysemname);
				HANDLE emptysem = allemptysemarray[shmindex];//OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE,csEmptySem); //指定想要的访问权限，SEMAPHORE_ALL_ACCESS 请求对事件对象的完全访问，SEMAPHORE_MODIFY_STATE 修改状态权限，使用 ReleaseSemaphore函数需要该权限；
				//CString csStoredSem(gMapView->strategyarray[shmindex].storedsemname);
				HANDLE storedsem = allstoredsemarray[shmindex];//OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE,csStoredSem);

				long offset;
				WaitForSingleObject(emptysem, INFINITE);
				WaitForSingleObject(mymutex, INFINITE);
				if (gMapView->strategyarray[shmindex].nempty > 0) {
					offset = gMapView->strategyarray[shmindex].nput;
					if (++(gMapView->strategyarray[shmindex].nput) >= NMAXTICK)
						gMapView->strategyarray[shmindex].nput = 0;
					td_msg.msgtype = ON_STRA_STRATEGY_EXIT;

					memcpy(&td_msg.msg, &pShmStrategyAction, sizeof(ShmStrategyAction));
					memcpy(&gMapView->strategyarray[shmindex].MDArray[offset], &td_msg, sizeof(ShmMsg));
					gMapView->strategyarray[shmindex].nstored++;
					gMapView->strategyarray[shmindex].nempty--;
				}
				ReleaseMutex(mymutex);
				ReleaseSemaphore(storedsem, 1, NULL);
			}
			else if (pOrderTrade.OrderType == ON_TD_DISPLAY_LOCAL_CL) {
				ShmStrategyAction pShmStrategyAction;
				shmindex = pOrderTrade.shmindex;
				pShmStrategyAction.actionType = ON_STRA_DISPLAY_LOCAL_CL;
				pShmStrategyAction.shmindex = shmindex;

				//CString csMutexName(gMapView->strategyarray[shmindex].mutexname);
				HANDLE mymutex = allmutexarray[shmindex];//OpenMutex(MUTEX_ALL_ACCESS,FALSE,csMutexName);
				//CString csEmptySem(gMapView->strategyarray[shmindex].emptysemname);
				HANDLE emptysem = allemptysemarray[shmindex];//OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE,csEmptySem); //指定想要的访问权限，SEMAPHORE_ALL_ACCESS 请求对事件对象的完全访问，SEMAPHORE_MODIFY_STATE 修改状态权限，使用 ReleaseSemaphore函数需要该权限；
				//CString csStoredSem(gMapView->strategyarray[shmindex].storedsemname);
				HANDLE storedsem = allstoredsemarray[shmindex];//OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE,csStoredSem);

				long offset;
				WaitForSingleObject(emptysem, INFINITE);
				WaitForSingleObject(mymutex, INFINITE);
				if (gMapView->strategyarray[shmindex].nempty > 0) {
					offset = gMapView->strategyarray[shmindex].nput;
					if (++(gMapView->strategyarray[shmindex].nput) >= NMAXTICK)
						gMapView->strategyarray[shmindex].nput = 0;
					td_msg.msgtype = ON_STRA_DISPLAY_LOCAL_CL;

					memcpy(&td_msg.msg, &pShmStrategyAction, sizeof(ShmStrategyAction));
					memcpy(&gMapView->strategyarray[shmindex].MDArray[offset], &td_msg, sizeof(ShmMsg));
					gMapView->strategyarray[shmindex].nstored++;
					gMapView->strategyarray[shmindex].nempty--;
				}
				ReleaseMutex(mymutex);
				ReleaseSemaphore(storedsem, 1, NULL);
			}
			else if (pOrderTrade.OrderType == ON_TD_STRATEGY_RESET) {
				ShmStrategyAction pShmStrategyAction;
				shmindex = pOrderTrade.shmindex;
				pShmStrategyAction.actionType = ON_STRA_STRATEGY_RESET;
				pShmStrategyAction.shmindex = shmindex;

				//CString csMutexName(gMapView->strategyarray[shmindex].mutexname);
				HANDLE mymutex = allmutexarray[shmindex];//OpenMutex(MUTEX_ALL_ACCESS,FALSE,csMutexName);
				//CString csEmptySem(gMapView->strategyarray[shmindex].emptysemname);
				HANDLE emptysem = allemptysemarray[shmindex];//OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE,csEmptySem); //指定想要的访问权限，SEMAPHORE_ALL_ACCESS 请求对事件对象的完全访问，SEMAPHORE_MODIFY_STATE 修改状态权限，使用 ReleaseSemaphore函数需要该权限；
				//CString csStoredSem(gMapView->strategyarray[shmindex].storedsemname);
				HANDLE storedsem = allstoredsemarray[shmindex];//OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE,csStoredSem);

				long offset;
				WaitForSingleObject(emptysem, INFINITE);
				WaitForSingleObject(mymutex, INFINITE);
				if (gMapView->strategyarray[shmindex].nempty > 0) {
					offset = gMapView->strategyarray[shmindex].nput;
					if (++(gMapView->strategyarray[shmindex].nput) >= NMAXTICK)
						gMapView->strategyarray[shmindex].nput = 0;
					td_msg.msgtype = ON_STRA_STRATEGY_RESET;

					memcpy(&td_msg.msg, &pShmStrategyAction, sizeof(ShmStrategyAction));
					memcpy(&gMapView->strategyarray[shmindex].MDArray[offset], &td_msg, sizeof(ShmMsg));
					gMapView->strategyarray[shmindex].nstored++;
					gMapView->strategyarray[shmindex].nempty--;
				}
				ReleaseMutex(mymutex);
				ReleaseSemaphore(storedsem, 1, NULL);
			}
			else if (pOrderTrade.OrderType == ON_TD_STRATEGY_START) {
				shmindex = pOrderTrade.shmindex;

				//CString csMutexName(gMapView->strategyarray[shmindex].mutexname);
				HANDLE mymutex = allmutexarray[shmindex];//OpenMutex(MUTEX_ALL_ACCESS,FALSE,csMutexName);
				//CString csEmptySem(gMapView->strategyarray[shmindex].emptysemname);
				HANDLE emptysem = allemptysemarray[shmindex];//OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE,csEmptySem); //指定想要的访问权限，SEMAPHORE_ALL_ACCESS 请求对事件对象的完全访问，SEMAPHORE_MODIFY_STATE 修改状态权限，使用 ReleaseSemaphore函数需要该权限；
				//CString csStoredSem(gMapView->strategyarray[shmindex].storedsemname);
				HANDLE storedsem = allstoredsemarray[shmindex];//OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE,csStoredSem);

				WaitForSingleObject(mymutex, INFINITE);

				//Begin 初始化当前缓冲区数据 Added 2017-06-28
				//重置缓冲区信息,重置empty&stored信号量
				gMapView->strategyarray[shmindex].StrategyStarted = true;
				gMapView->strategyarray[shmindex].nget = 0;
				gMapView->strategyarray[shmindex].nput = 0;
				gMapView->strategyarray[shmindex].nempty = NMAXTICK;
				gMapView->strategyarray[shmindex].nstored = 0;
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

				//End 重置empty&stored信号量
				//End 初始化当前缓冲区数据 Added 2017-06-28

				gMapView->strategyarray[shmindex].StrategyStarted = true;
				if (shmindex >= gMapView->strategynum && shmindex < NMAXSTRA)gMapView->strategynum = shmindex + 1;
				ReleaseMutex(mymutex);

				ReleaseSemaphore(CreateThreadSem, 1, NULL);
			}
			//delete pOrderTrade;
			//pOrderTrade=0;
		}//End if OrderList is not Empty
	}
}