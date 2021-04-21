// MyStruct.h: interface for the MyStruct class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYSTRUCT_H__7460E514_C16C_4241_BF7B_0DF4D7E3C653__INCLUDED_)
#define AFX_MYSTRUCT_H__7460E514_C16C_4241_BF7B_0DF4D7E3C653__INCLUDED_

#include<Afxtempl.h>
#include<time.h>
#include "shmstruct.h"
#include <stdint.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define MORDER_FAIL '0'
#define MORDER_SUBMITTED '1'
#define MORDER_ACCEPTED '2'
#define MORDER_QUEUED '3'
#define MORDER_PART_CANCELLED '6'
#define MORDER_CANCELLED '7'
#define MORDER_PART_TRADED '8'
#define MORDER_FULL_TRADED '9'
#define MORDER_STATE_OTHER 'x'
#define MORDER_AMBUSH 'a'
#define MORDER_WAITFORSUBMIT 's'

#define ON_RTN_DEPTH_MARKET_DATA 1
#define ON_RTN_ORDER 2
#define ON_RTN_TRADE 3
#define ON_RSP_QRY_TRADE 4

#define ON_TD_STRATEGY_EXIT 5
#define ON_TD_STRATEGY_START 6
#define ON_TD_DISPLAY_LOCAL_CL 7
#define ON_TD_RSP_ORDER_INSERT 8
#define ON_TD_STRATEGY_RESET 9

#define ON_STRA_STRATEGY_EXIT 10
#define ON_STRA_DISPLAY_LOCAL_CL 11
#define ON_STRA_RSP_ORDER_INSERT 12
#define ON_STRA_STRATEGY_RESET 13

#define ON_RSP_ORDER_ACTION 22

#define ON_THOST_RTN_ORDER 32
#define ON_THOST_RTN_TRADE 33

#define ON_RSP_QRY_ORDER 'o'
#define ON_RECOVER_QRY_ORDER 'r'
#define ON_RSP_QRY_POSITION 'p'
#define ON_RSP_LOCAL_CL 'c'

#define MORDER_BUY 0
#define MORDER_SELL 1
#define MORDER_OPEN 0
#define MORDER_CLOSE 1
#define MORDER_CLOSETODAY 2
#define MORDER_STOPLOSSCLOSE 3
#define MORDER_OPENCLOSENONE 4

#define STRATEGY_LOG '1'
#define TRADE_LOG '2'
#define MD_LOG '3'

#define MAX_RUNNING_STRATEGY 30

const char InstanceStarted[50] = "������";
const char InstanceNotStarted[50] = "δ����";

#define U_SIGNAL 0
#define D_SIGNAL 1

#pragma pack(push,1)
struct RateInfo
{
	char           time[20];//"YYYY-MM-DD H24:MI:SS\0"
	int            open;
	int            low;
	int            high;
	int            close;
	int            vol;
};
#pragma pack(pop)

#pragma pack(push,1)
struct TickInfo
{
	char      ordername[20];
	char      datadate[10];
	char 	 updatetime[10];
	int		 millsec;
	double    price;
	double    ask1;
	double    bid1;
	int	     askvol1;
	int	     bidvol1;
	double    position;
	double    vol;
	double    upperLimitPrice;
	double	 lowerLimitPrice;
	double    openprice;				// add by wxj 2018-11-28
};
#pragma pack(pop)

struct ManualOrder
{
	char StrategyName[50];
	int Direction;
	double subprice;
};
struct ParamNode
{
	char ParamName[30];
	char ParamChineseName[50];
	char ParamValue[30];
};

struct StrategyInstanceNode
{
	char InstanceName[50];
	bool StrategyStarted;
	int shmindex;
	list<ParamNode> ParamList;
};

struct StrategyNode
{
	char StrategyName[50];
	char StrategyID[50];
	int MaxOnHandPositionCount;
	list<string> ParamCHNNameList;
	list<string> ParamENGNameList;
	list<StrategyInstanceNode> InstanceList;
};

struct ModelNode
{
	char ModelName[50];
	list<StrategyNode> StrategyList;
};

struct LogData
{
	char            time[19];//"YYYY-MM-DD H24:MI:SS\0"
	char            lastUpdate[6];
	char            vol[20];
	char	           xline[2];
};

struct LogDataHeader
{
	// WORD wFlag; //UNICODE�ļ���־0xFEFF
	char           type[10];//"RU1201"
	char			  barsize[10];
	char			  xline[2];
};

struct BarRateInfo
{
	char      datadate[10];
	char		 datatime[10];
	double    open;
	double    low;
	double    high;
	double    close;
	double    vol;
	double	position;
};

#pragma pack(push,1)
struct OrderTradeMsg {
	///����ϵͳ���
	int OrderSysId;
	///�������ر��
	int OrderLocalRef;
	///��������
	char Direction;
	///�۸�
	double LimitPrice;
	///����
	int VolumeTotalOriginal;
	///����״̬
	char OrderStatus;
	///��ɽ�����
	int VolumeTraded;
	///ʣ������
	int VolumeTotal;
	///ί��ʱ��
	char InsertOrTradeTime[21];
	///OrderType
	int OrderType; // 0 - Order , 1 - Trade
	///�۸�
	double Price;
	///���� - For Trade
	int Volume;
	double MatchFee;
	int ActionLocalNo;
	int shmindex;
	char OffFlag;
	char MatchNo[71];//For Position Displaying,����̫��,ShmMsg.msg=128
};
#pragma pack(pop)

struct InsertOrderField {
	int OrderLocalRef;
	char ClientNo[20];
	char CommodityNo[11];
	char ExchangeID[9];
	char InstrumentID[30];
	int  Offset;
	int Direction;
	double OrderPrice;
	int OrderVol;
	int FrontID;
	int SessionID;
};

struct OrderDetailField {
	int OrderId;
	char CommodityNo[31];
	char InstrumentID[31];
	char InsertDateTime[21];
	int Direction;
	int Offset;
	char OrderStatus;
	double SubmitPrice;
	double TradePrice;
	int VolumeTotalOriginal;
	int VolumeTraded;
	int OrderLocalRef;
	int FrontID;
	int SessionID;
};

struct PositionDetailField {
	char MatchNo[71];
	char CommodityNo[31];
	char InstrumentID[31];
	int Direction;
	char TradeDateTime[21];
	double TradePrice;
	int   TradeVol;
};
///Open Order
#pragma pack(push,1)
struct MyOpenOrderType {
	int OrderId;
	int OrderLocalRef;
	int OrderLocalRetReqID;
	char SubmitDateAndTime[21];//YYYYMMDD HH24:MI:SS ����ʱ��,����û�г�����Ч�������ֶ����ڵڶ��յ�δ�ɽ������������ύ
	char OpenTime[21];//���ֵ���Ӧ�Ŀ��ֳɽ�ʱ�䣬���������ж��Ƿ�Ҫ��ƽ�����
	int Direction;
	char Offset;
	int VolumeTotalOriginal;
	int VolumeTraded;
	int VolumeTotal;
	double LimitPrice;
	double OrigSubmitPrice;
	unsigned long dwCloseOrderStart;
	double ProfitPrice;
	char OrderStatus;

	double maxProfit;
	double OpenOrderATR;

	unsigned long dwOpenSubmitStart;
	double DealPrice;
	bool OpenOrderCanbeCanceled;
	int MOrderType;
	bool MAStop;
	double mStoplossPoint;
	int FrontID;
	int SessionID;
	int PairID;
	int CancelType;
};
#pragma pack(pop)

///Close Order
#pragma pack(push,1)
struct MyCloseOrderType {
	int CloseOrderSeqNo; //ƽ�ֵ����,�ڲ����,���ڳ���ά��
	int OrderId;
	int OrderLocalRef;
	int OrderLocalRetReqID;
	int OpenOrderID;
	char SubmitDateAndTime[21];//��������,����û�г�����Ч�������ֶ����ڵڶ��յ�δ�ɽ������������ύ
	char OpenTime[21];//���ֵ���Ӧ�Ŀ��ֳɽ�ʱ�䣬���������ж��Ƿ�Ҫ��ƽ�����
	int Direction;
	char Offset;
	int VolumeTotalOriginal;
	int VolumeTraded;
	int VolumeTotal;
	double LimitPrice;
	double OrigSubmitPrice;
	unsigned long dwCloseOrderStart;
	double OpenOrderSubmitPrice;
	double OpenOrderTradePrice;
	double NextCloseOrderPrice;
	double ProfitPrice;
	char OrderStatus;

	double ManualStopPrice;
	double maxProfit;
	double OpenOrderATR;
	bool IsClosePofitOrder;
	bool IsStoplessOrder;
	bool CanbeCanceled;
	int MOrderType;
	bool MAStop;
	double mStoplossPoint;
	int FrontID;
	int SessionID;
	int PairID;
	int CancelType;
};
#pragma pack(pop)

#pragma pack(push,1)
struct MyOldCloseOrderType {
	int CloseOrderSeqNo; //ƽ�ֵ����,�ڲ����,���ڳ���ά��
	int OrderId;
	int OrderLocalRef;
	int OrderLocalRetReqID;
	char SubmitDateAndTime[21];//��������,����û�г�����Ч�������ֶ����ڵڶ��յ�δ�ɽ������������ύ
	char OpenTime[21];//���ֵ���Ӧ�Ŀ��ֳɽ�ʱ�䣬���������ж��Ƿ�Ҫ��ƽ�����
	int Direction;
	char Offset;
	int VolumeTotalOriginal;
	int VolumeTraded;
	int VolumeTotal;
	double LimitPrice;
	double OrigSubmitPrice;
	unsigned long dwCloseOrderStart;
	double OpenOrderSubmitPrice;
	double OpenOrderTradePrice;
	double NextCloseOrderPrice;
	double ProfitPrice;
	char OrderStatus;

	double ManualStopPrice;
	double maxProfit;
	bool IsClosePofitOrder;
	bool IsStoplessOrder;
	bool CanbeCanceled;
	int MOrderType;
	bool MAStop;
	double mStoplossPoint;
	int FrontID;
	int SessionID;
};
#pragma pack(pop)

struct LocalCLForDisplayField {
	char StrategyID[50];
	char InstanceName[50];
	char InstCodeName[50];
	int CloseOrderSeqNo;
	int OrderID;
	int Direction;
	char OpenTime[21];
	double OpenOrderTradePrice;
	double LimitPrice;
	double ManualStopPrice;
	int   VolumeTotal;
	int OffSet;
	int MOrderType;
};

struct TradeLogType
{
	char AccountID[50];
	char InstanceName[50];
	char StrategyID[50];
	char CodeName[50];
	char CommodityNo[50];
	char tradingday[12];
	char tradingtime[12];
	char MatchNo[80];
	double tradeprice;
	double submitprice;
	int qty;
	int openorclose;
	int openid;
	int closeid;
	double fee;
};

#pragma pack(push,1)
struct SendLogType
{
	char AccountID[50];
	char InstanceName[50];
	char StrategyID[50];
	char CodeName[50];
	char CommodityNo[50];
	char tradingday[12];
	char tradingtime[12];
	char MatchNo[80];
	double tradeprice;
	double submitprice;
	int qty;
	int openorclose;
	char openid[21];
	char closeid[21];
	double fee;
};
#pragma pack(pop)

struct InstrumentInfo
{
	char ExchangeID[30];
	char CommodityNo[30];
	char InstrumentID[30];
	double OneTick;
	int Multiplier;//ÿ���ֵı���
};

struct RecoverStrategy
{
	char StrategyName[100];
	char InstanceName[100];
};

#pragma pack(push,1)
struct mSerializeHeader
{
	int OpenOrderCount;
	int CloseOrderCount;
	char StartTime[21];
	char EndTime[21];
	char CodeName[50];//��ʽ NYMEX CL 1707
	char StopTime[21];//��ʽ yyyy-MM-dd hh:nn:ss
	char SpecificArea[500];			// modify by Allen
};
#pragma pack(pop)

struct InstrumentsName
{
	char ExchangeID[30];
	char CommodityNo[30];
	char InstrumentID[30];
	double OneTick;
};

struct WavePoint
{
	char WaveSerialID[20];
	char HighWaveSerialID[20];
	char datadate[10];
	char datatime[10];
	char firstdatadate[10];
	char firstdatatime[10];
	int firstindex;
	double firstclose;
	int segnum;
	int samelowsegnum;
	double open;
	double high;
	double low;
	double close;
	int index;
	int type;//top - 1, bottom - -1
};

#define BUFFER_SIZE 1024*1024
struct ring_buffer
{
	void* buffer;     //������
	uint32_t     size;       //��С
	uint32_t     in;         //���λ��
	uint32_t     out;        //����λ��
	uint32_t     nodecount;
	CCriticalSection* f_lock;    //������
};

#define MAXMDADDRNO 2
#define MAXTDADDRNO 2
struct addrtype
{
	char serverip[50];
	int port;
};

struct Posi {
	int buyposition;
	int sellposition;
	bool bBuy;			// true today
	bool bSell;
};

struct BasePriceData {
	char StrategyID[50];
	char InstanceName[50];
	char DataDate[20];
	char ItemData[20];
	//char ItemValue[20];
	double Price;
	int Vol;
	int Dir;
};

#endif // !defined(AFX_MYSTRUCT_H__7460E514_C16C_4241_BF7B_0DF4D7E3C653__INCLUDED_)
