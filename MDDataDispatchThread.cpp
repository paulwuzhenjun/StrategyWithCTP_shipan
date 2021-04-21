#include "StdAfx.h"
#include "MDDataDispatchThread.h"
#include "TickDataList.h"

#define MMF_FILE_SIZE (2048*2048)

extern TickDataList TickList;
extern MapViewType* gMapView;
extern HANDLE MdTickSem;

void* mview_ptr;
HANDLE allmutexarray[NMAXSTRA];
HANDLE allemptysemarray[NMAXSTRA];
HANDLE allstoredsemarray[NMAXSTRA];

extern list<InstrumentsName> OverSeaInstSubscribed;
extern list<InstrumentsName> DomesticInstSubscribed;
extern list<InstrumentsName> SGEInstSubscribed;

MDDataDispatchThread::MDDataDispatchThread(void)
{
	m_bIsRunning = true;
}

MDDataDispatchThread::~MDDataDispatchThread(void)
{
	if (gMapView != NULL) {
		free(gMapView);
	}
}

int MDDataDispatchThread::openThreadShm() {
	gMapView = (MapViewType*)malloc(sizeof(MapViewType));
	if (gMapView == NULL) return -1;

	gMapView->tdstarted = true;
	InitMapArea();
	return 0;
}

int MDDataDispatchThread::InitMapArea()
{
	gMapView->strategynum = 0;
	for (int i = 0; i < NMAXSTRA; i++) {
		strcpy(gMapView->strategyarray[i].CodeName, "");
		gMapView->strategyarray[i].StrategyStarted = false;
		gMapView->strategyarray[i].nput = 0;
		gMapView->strategyarray[i].nget = 0;
		//gMapView->strategyarray[i].mutex=new CMutex();
		/*
		CString tmp("tmutex");
		tmp.AppendFormat(_T("%d"),i);
		int tmplen=WideCharToMultiByte(CP_ACP,0,tmp,tmp.GetLength(),NULL,0,NULL,NULL);
		char *ctmp=new char[tmplen+1];
		WideCharToMultiByte(CP_ACP,0,tmp,tmp.GetLength(),ctmp,tmplen,NULL,NULL);
		ctmp[tmplen]='\0';
		strcpy(gMapView->strategyarray[i].mutexname,ctmp);
		delete ctmp;

		CString tmp2("tempsem");
		tmp2.AppendFormat(_T("%d"),i);
		int tmplen2=WideCharToMultiByte(CP_ACP,0,tmp2,tmp2.GetLength(),NULL,0,NULL,NULL);
		char *ctmp2=new char[tmplen2+1];
		WideCharToMultiByte(CP_ACP,0,tmp2,tmp2.GetLength(),ctmp2,tmplen2,NULL,NULL);
		ctmp2[tmplen2]='\0';
		strcpy(gMapView->strategyarray[i].emptysemname,ctmp2);
		delete ctmp2;

		CString tmp3("tssem");
		tmp3.AppendFormat(_T("%d"),i);
		int tmplen3=WideCharToMultiByte(CP_ACP,0,tmp3,tmp3.GetLength(),NULL,0,NULL,NULL);
		char *ctmp3=new char[tmplen3+1];
		WideCharToMultiByte(CP_ACP,0,tmp3,tmp3.GetLength(),ctmp3,tmplen3,NULL,NULL);
		ctmp3[tmplen3]='\0';
		strcpy(gMapView->strategyarray[i].storedsemname,ctmp3);
		delete ctmp3;
		*/
		gMapView->strategyarray[i].nempty = NMAXTICK;
		gMapView->strategyarray[i].nstored = 0;

		//CString csMutexName(gMapView->strategyarray[i].mutexname);
		allmutexarray[i] = CreateMutex(NULL, FALSE, NULL);//如果CreateMutex第2个参数是false，则任何进程都可以waitfor，并立即成为该mutex的owner

		//CString csEmptySem(gMapView->strategyarray[i].emptysemname);
		allemptysemarray[i] = CreateSemaphore(NULL, NMAXTICK, NMAXTICK, NULL);

		//CString csStoredSem(gMapView->strategyarray[i].storedsemname);
		allstoredsemarray[i] = CreateSemaphore(NULL, 0, NMAXTICK, NULL);
	}

	//初始化合约列表,传递此列表给交易程序查看
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
	//for(int i=0;i<gMapView->strategynum;i++){
	//	CString csMutexName(gMapView->strategyarray[i].mutexname);
	//	allmutexarray[i]=CreateMutex(NULL, FALSE, csMutexName);//如果CreateMutex第2个参数是false，则任何进程都可以waitfor，并立即成为该mutex的owner
	//}
	return 0;
}

void MDDataDispatchThread::DispatchMdMsg()
{
	TickInfo pDepthMarketData;
	while (m_bIsRunning) {
		WaitForSingleObject(MdTickSem, INFINITE);

		if (!TickList.DataListCore.IsEmpty())
		{
			TickInfo tick = TickList.GetHead();
			memcpy(&pDepthMarketData, &tick, sizeof(TickInfo));
			DispatchMdTicks(&pDepthMarketData);
		}
	}
}

void MDDataDispatchThread::DispatchMdTicks(TickInfo* pData)
{
	strcpy(shm_tick.updatetime, pData->updatetime);
	strcpy(shm_tick.InstCodeName, pData->ordername);
	shm_tick.ask1 = pData->ask1;
	shm_tick.bid1 = pData->bid1;
	shm_tick.ask1vol = pData->askvol1;
	shm_tick.bid1vol = pData->bidvol1;
	shm_tick.price = pData->price;
	shm_tick.upperLimitPrice = pData->upperLimitPrice;
	shm_tick.lowerLimitPrice = pData->lowerLimitPrice;
	shm_tick.openPrice = pData->openprice;
	memset(&md_msg, 0, sizeof(ShmMsg));
	md_msg.msgtype = MD_MSG;
	memcpy(&md_msg.msg, &shm_tick, sizeof(ShmTickInfo));
	int StrategyRunning = gMapView->strategynum;
	for (int i = 0; i < StrategyRunning; i++) {
		if (gMapView->strategyarray[i].StrategyStarted && strcmp(gMapView->strategyarray[i].CodeName, pData->ordername) == 0) {
			long offset;
			StrategyRunning = gMapView->strategynum;
			//int ret =
			WaitForSingleObject(allemptysemarray[i], INFINITE);
			//if(ret==WAIT_OBJECT_0){
			//	int ret2=
			WaitForSingleObject(allmutexarray[i], INFINITE);
			//	if(ret2==WAIT_OBJECT_0){
			if (gMapView->strategyarray[i].nempty > 0) {
				offset = gMapView->strategyarray[i].nput;
				if (++(gMapView->strategyarray[i].nput) >= NMAXTICK)
					gMapView->strategyarray[i].nput = 0;
				memcpy(&gMapView->strategyarray[i].MDArray[offset], &md_msg, sizeof(ShmMsg));
				gMapView->strategyarray[i].nstored++;
				gMapView->strategyarray[i].nempty--;
			}
			ReleaseMutex(allmutexarray[i]);
			ReleaseSemaphore(allstoredsemarray[i], 1, NULL);
			//}
		//}
		}
	}
	return;
}