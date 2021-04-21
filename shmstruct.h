#pragma once

#define NMAXTICK 256
#define NMAXSTRA 20
#define MMF_FILE_SIZE (2048*2048)
#define MD_MSG 0
#define TD_MSG 1

#pragma pack(push,1)
struct ShmMsg
{
	int msgtype;
	char msg[256];
};
#pragma pack(pop)

#pragma pack(push,1)
struct ShmRspOrderInsert
{
	int OrderID;
	int iRetReqID;
};
#pragma pack(pop)

#pragma pack(push,1)
struct ShmStrategyAction
{
	int shmindex;
	int actionType;
};
#pragma pack(pop)

#pragma pack(push,1)
struct ShmTickInfo
{
	char InstCodeName[30];
	char 	 updatetime[10];
	double    price;
	double    ask1;
	double    bid1;
	int    ask1vol;
	int    bid1vol;
	double    upperLimitPrice;
	double	 lowerLimitPrice;
	double    openPrice;				// add by wxj 2018-11-28
};
#pragma pack(pop)

#pragma pack(push,1)
struct StrategyQueueType
{
	char CodeName[30];
	bool StrategyStarted;
	int nput;
	int nget;
	char mutexname[10];
	char emptysemname[20];
	char storedsemname[20];
	int nempty;
	int nstored;
	ShmMsg MDArray[NMAXTICK];
};
#pragma pack(pop)

struct MapViewType
{
	bool mdstarted;
	bool tdstarted;
	int strategynum;
	char instarray[20][30];
	int instnum;
	StrategyQueueType strategyarray[NMAXSTRA];
};