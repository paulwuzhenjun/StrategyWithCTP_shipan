#include "StdAfx.h"
#include "StrategyOpenPriceOpening.h"
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

int StrategyOpenPriceOpening::MaxOnHandPositionCount = 0;
int StrategyOpenPriceOpening::OnHandPositionCount = 0;

extern CListBox* pPubMsg;

extern EsunTraderSpi* pEsunTraderSpi;
extern ThostTraderSpi* pThostTraderSpi;
extern SgitTraderSpi* pSgitTraderSpi;

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
extern map<string, double> instlastpricemap;
extern HANDLE ScreenDisplaySem;
extern HANDLE MainScreenFreshSem;
extern HANDLE OrderInsertSem;
extern CListCtrl* pTradesDetailsList;
extern list<ModelNode> ModelList;
extern bool ClearInstanceCfgFile;
extern CLockVariable gLockVariable;
extern SRWLOCK g_srwLockOrderLocalRef;
extern map<int, int> OrderLocalRefToShmIndex;

StrategyOpenPriceOpening::StrategyOpenPriceOpening(char InstrumentID[30]) :m_Price(0)
{
	//	strcpy(m_InstID,InstrumentID);
	CTime mCurrTime = CTime::GetCurrentTime();
	/*
	CString str_mCurrTime=mCurrTime.Format("%Y-%m-%d");
	int len=WideCharToMultiByte(CP_ACP,0,str_mCurrTime,str_mCurrTime.GetLength(),NULL,0,NULL,NULL);
	char *c_str_mCurrTime=new char[len+1];
	WideCharToMultiByte(CP_ACP,0,str_mCurrTime,str_mCurrTime.GetLength(),c_str_mCurrTime,len,NULL,NULL);
	c_str_mCurrTime[len]='\0';
	sscanf_s(c_str_mCurrTime, "%d-%d-%d",&nYearG, &nMonthG, &nDateG);
	free(c_str_mCurrTime);
	*/
	/*
	CString str_mCurrDate=mCurrTime.Format("%Y%m%d");
	int lenDate=WideCharToMultiByte(CP_ACP,0,str_mCurrDate,str_mCurrDate.GetLength(),NULL,0,NULL,NULL);
	char *c_str_mCurrDate=new char[lenDate+1];
	WideCharToMultiByte(CP_ACP,0,str_mCurrDate,str_mCurrDate.GetLength(),c_str_mCurrDate,lenDate,NULL,NULL);
	c_str_mCurrDate[lenDate]='\0';
	free(c_str_mCurrDate);
	*/
	//全局变量
	iNextOrderRef = 1;
	mTickCount = 0;
	m_dOneTick = 0.01;
	mOpenTimes = 0;
	Type1OrderExist = false;
	Type2OrderExist = false;
	Type2StoplossClose = false;

	m_bIsRunning = true;
	m_bCrossTradingDay = false;
	m_bType1StoplossClose = false;

	memset(&mStrategyParams, 0, sizeof(mParasType));

	strcpy(mStrategyName, "OpenPriceOpening");
	strcpy(mInstanceName, "");
	mStrategyAndInstance = mStrategyID;

	MarketOpenExecuted = false;
	MarketCloseExecuted = false;
	mCloseOrderSeqNo = 0;
	mOpenRet = false;
	WriteMsgToLogList("Strategy Init..");
}

void StrategyOpenPriceOpening::CloseMap()
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

StrategyOpenPriceOpening::~StrategyOpenPriceOpening(void)
{
}

int StrategyOpenPriceOpening::GetShmindex()
{
	return shmindex;
}

void StrategyOpenPriceOpening::SetShmindex(int xshmindex)
{
	shmindex = xshmindex;
}

wstring StrategyOpenPriceOpening::s2ws(const string& s)
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

int StrategyOpenPriceOpening::CreateStrategyMapOfView() {
	CString strPathFile;
	::GetModuleFileName(NULL, strPathFile.GetBuffer(MAX_PATH), MAX_PATH);
	strPathFile.ReleaseBuffer();
	strPathFile = strPathFile.Left(strPathFile.ReverseFind(_T('\\')));
	strPathFile += "\\Strategies\\TTimeOpenModel\\";
	char filename[500];
	ConvertCStringToCharArray(strPathFile, filename);
	string strFileName(filename);
	//strFileName.append(mStrategyName);
	strFileName.append(mStrategyID);		// 路径id
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

int StrategyOpenPriceOpening::OpenStrategyMapOfView() {
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

void StrategyOpenPriceOpening::SetInstanceName(char xInstanceName[50])
{
	strcpy(mInstanceName, xInstanceName);
	mStrategyAndInstance = mStrategyID;
	mStrategyAndInstance.append("_");
	mStrategyAndInstance.append(mInstanceName);
}

void StrategyOpenPriceOpening::GetInstanceName(char* xInstanceName) {
	strcpy(xInstanceName, mInstanceName);
}

void StrategyOpenPriceOpening::SetParamValue(ParamNode node) {
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
	else if (strcmp(node.ParamName, "OpenDistPoint") == 0) {
		mStrategyParams.OpenDistPoint = atof(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "OpenPrice") == 0) {
		mStrategyParams.OpenPrice = atof(node.ParamValue);
		mOpenPrice = mStrategyParams.OpenPrice;
	}
	else if (strcmp(node.ParamName, "OpenTime") == 0) {
		strcpy(mStrategyParams.OpenTime, node.ParamValue);
	}
	else if (strcmp(node.ParamName, "ChopTime") == 0) {
		strcpy(mStrategyParams.ChopTime, node.ParamValue);
	}
	else if (strcmp(node.ParamName, "CloseTime") == 0) {
		strcpy(mStrategyParams.CloseTime, node.ParamValue);
	}
	else if (strcmp(node.ParamName, "OpenVol1") == 0) {
		mStrategyParams.OpenVol1 = atoi(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "SecondOpenDistPoint") == 0) {
		mStrategyParams.SecondOpenDistPoint = atof(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "OpenVol2") == 0) {
		mStrategyParams.OpenVol2 = atoi(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "FirstProfitPoint") == 0) {
		mStrategyParams.FirstProfitPoint = atof(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "FirstStoplossPoint") == 0) {
		mStrategyParams.FirstStoplossPoint = atof(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "SecondProfitPoint") == 0) {
		mStrategyParams.SecondProfitPoint = atof(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "SecondStoplossPoint") == 0) {
		mStrategyParams.SecondStoplossPoint = atof(node.ParamValue);
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

void StrategyOpenPriceOpening::InitVariables() {
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

	mOpenTimes = 0;
	Type1OrderExist = false;
	Type2OrderExist = false;
	Type2StoplossClose = false;
	m_bType1StoplossClose = false;
	mOpenRet = false;
}

void StrategyOpenPriceOpening::InitAction() {
	mOpenTimes = 0;
	Type1OrderExist = false;
	Type2OrderExist = false;
	Type2StoplossClose = false;
	m_bType1StoplossClose = false;
	mOpenRet = false;
}

void StrategyOpenPriceOpening::ResetStrategy() {
}
//检查消息队列，并将得到的消息转给相应的处理函数
/*
void StrategyOpenPriceOpening::MessageProcess()
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
void StrategyOpenPriceOpening::OnRtnDepthMarketData(TickInfo* pDepthMarketData)
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
	strcpy(TickRealTimeDataDate, pDepthMarketData->datadate);

	int nOpenHour, nOpenMin, nOpenSec, nCloseHour, nCloseMin, nCloseSec;
	sscanf_s(mStrategyParams.OpenTime, "%d:%d:%d", &nOpenHour, &nOpenMin, &nOpenSec);
	sscanf_s(mStrategyParams.CloseTime, "%d:%d:%d", &nCloseHour, &nCloseMin, &nCloseSec);

	if (nHour_m >= nCloseHour && nHour_m < nOpenHour) return;

	// 获取开盘价格
	if (nHour_m == nOpenHour && nMin_m >= nOpenMin && !mOpenRet) {
		mOpenPrice = pDepthMarketData->price;
		mOpenRet = true;
		char log[200];
		sprintf(log, "%s,OpenPriceOpening,%s,开盘价=%.5f", mStrategyAndInstance.c_str(), pDepthMarketData->updatetime, mOpenPrice);
		CString str(log);
		pPubMsg->AddString(str);
		WriteMsgToLogList(log);

		//sprintf(log,"%s,%d,%d,%d,v=%d,PC=%d,MPC=%d,TPC=%d,MTPC=%d",mStrategyAndInstance.c_str(),mStrategyParams.LoopTimes,mStrategyParams.OpenBuyAllow,mStrategyParams.OpenSellAllow,mStrategyParams.OpenVol1,OnHandPositionCount,MaxOnHandPositionCount,TotalOnHandPosition,MaxTotalOnHandPosition);
		//WriteMsgToLogList(log);
	}

	//if(mOpenPrice<0.00001){
	//	if(curTickTime>tmt_StartTime) {
	//		mOpenPrice=m_Buy1;
	//		char log[200];
	//		sprintf(log,"%s,OpenPriceOpening,%s,开盘价=%.5f",mStrategyAndInstance.c_str(),pDepthMarketData->updatetime,mOpenPrice);
	//		CString str(log);
	//		pPubMsg->AddString(str);
	//		WriteMsgToLogList(log);

	//		sprintf(log,"%s,%d,%d,%d,v=%d,PC=%d,MPC=%d,TPC=%d,MTPC=%d",mStrategyAndInstance.c_str(),mStrategyParams.LoopTimes,mStrategyParams.OpenBuyAllow,mStrategyParams.OpenSellAllow,mStrategyParams.OpenVol1,OnHandPositionCount,MaxOnHandPositionCount,TotalOnHandPosition,MaxTotalOnHandPosition);
	//		WriteMsgToLogList(log);
	//	}
	//}

	if (curTickTime > tmt_StartTime&& mOpenPrice > 0.00001 && !timeRuleForStop(nHour_m, nMin_m, nSec_m)) {
		EnterBuyTrade = 1;
	}

	if (curTickTime > tmt_StartTime&& mOpenPrice > 0.00001 && !timeRuleForStop(nHour_m, nMin_m, nSec_m)) {
		EnterSellTrade = 1;
	}

	//Processing Opened Order
	std::list<MyOpenOrderType>::iterator openorder_it;
	if (!OpenOrderList.empty()) {
		for (openorder_it = OpenOrderList.begin(); openorder_it != OpenOrderList.end(); ++openorder_it) {
			if (openorder_it->VolumeTotal != 0 && openorder_it->OpenOrderCanbeCanceled && openorder_it->VolumeTraded > 0) {
				if (openorder_it->MOrderType == 1) {
					if (((openorder_it->LimitPrice - 0.00001 - m_dOneTick * mStrategyParams.OpenDistPoint) > m_Sell1&& openorder_it->Direction == MORDER_SELL)
						|| ((m_Buy1 - 0.00001 - m_dOneTick * mStrategyParams.OpenDistPoint) > openorder_it->LimitPrice&& openorder_it->Direction == MORDER_BUY)
						) {
						char line[200];
						sprintf(line, "%s,Type 1 Cancel Open Order,OrderId=%d", mStrategyAndInstance.c_str(), openorder_it->OrderId);
						WriteMsgToLogList(line);

						ReqOrderDelete(openorder_it->OrderId, openorder_it->OrderLocalRef, openorder_it->FrontID, openorder_it->SessionID);
						openorder_it->OpenOrderCanbeCanceled = false;
					}
				}
				else if (timeRuleForCancelOpen(pDepthMarketData->datadate, pDepthMarketData->updatetime)) {
					//结束时间已到,未成交的开仓单撤掉
					char line[200];
					sprintf(line, "%s,%s,OpenOrderId=%d", pDepthMarketData->datadate, pDepthMarketData->updatetime, openorder_it->OrderId);
					WriteMsgToLogList(line);

					ReqOrderDelete(openorder_it->OrderId, openorder_it->OrderLocalRef, openorder_it->FrontID, openorder_it->SessionID);
					openorder_it->OpenOrderCanbeCanceled = false;
				}
			}
			else if (openorder_it->VolumeTotal != 0 && openorder_it->OpenOrderCanbeCanceled && openorder_it->MOrderType == 2) {
				//价格回到开盘价附近则类型二订单可以被撤掉,撤掉后以便可以重报另一边类型1订单
				if (((openorder_it->LimitPrice - 0.00001 - m_dOneTick * mStrategyParams.SecondOpenDistPoint) > m_Sell1&& openorder_it->Direction == MORDER_SELL)
					|| ((m_Buy1 - 0.00001 - m_dOneTick * mStrategyParams.SecondOpenDistPoint) > openorder_it->LimitPrice&& openorder_it->Direction == MORDER_BUY)
					) {
					char line[200];
					sprintf(line, "%s,Type 2 Cancel Open Order,OrderId=%d", mStrategyAndInstance.c_str(), openorder_it->OrderId);
					WriteMsgToLogList(line);

					ReqOrderDelete(openorder_it->OrderId, openorder_it->OrderLocalRef, openorder_it->FrontID, openorder_it->SessionID);
					openorder_it->OpenOrderCanbeCanceled = false;
				}
			}
			else if (timeRuleForCancelOpen(pDepthMarketData->datadate, pDepthMarketData->updatetime)) {
				//结束时间已到,未成交的开仓单撤掉
				char line[200];
				sprintf(line, "%s,%s,OpenOrderId=%d", pDepthMarketData->datadate, pDepthMarketData->updatetime, openorder_it->OrderId);
				WriteMsgToLogList(line);

				ReqOrderDelete(openorder_it->OrderId, openorder_it->OrderLocalRef, openorder_it->FrontID, openorder_it->SessionID);
				openorder_it->OpenOrderCanbeCanceled = false;
			}
		}//end for loop
	}

	//End for Opened Order processing

	mTickCount++;

	std::list<MyCloseOrderType>::iterator closeorder_it;
	if (!CloseOrderList.empty()) {
		//Loop Close Order List
		for (closeorder_it = CloseOrderList.begin(); closeorder_it != CloseOrderList.end(); closeorder_it++) {
			//std::list<MyCloseOrderType>::iterator iter_e=closeorder_it++;

			if (closeorder_it->Direction == MORDER_SELL &&
				(closeorder_it->OrderStatus == MORDER_PART_TRADED || closeorder_it->OrderStatus == MORDER_ACCEPTED || closeorder_it->OrderStatus == MORDER_QUEUED)) {
				//Closing the buy order
				closeorder_it->maxProfit = max(closeorder_it->maxProfit, m_Price - closeorder_it->OpenOrderTradePrice);

				//if((closeorder_it->maxProfit+0.00001)>10){
				//	closeorder_it->mStoplossPoint=min(0,10-closeorder_it->maxProfit);
				//}

				if (closeorder_it->IsClosePofitOrder && (((closeorder_it->OpenOrderTradePrice - m_Price) >= (mStrategyParams.FirstStoplossPoint * m_dOneTick - 0.00001) && closeorder_it->MOrderType == 1)
					|| ((closeorder_it->OpenOrderTradePrice - m_Price) >= (mStrategyParams.SecondStoplossPoint * m_dOneTick - 0.00001) && closeorder_it->MOrderType == 2))
					) {
					char line[200];
					sprintf(line, "%s,Buy Order Stoploss Close,%.5f,OrderId=%d", mStrategyAndInstance.c_str(), m_Price, closeorder_it->OrderId);
					WriteMsgToLogList(line);

					closeorder_it->NextCloseOrderPrice = m_Sell1 - m_dOneTick;
					ReqOrderDelete(closeorder_it->OrderId, closeorder_it->OrderLocalRef, closeorder_it->FrontID, closeorder_it->SessionID);
					if (closeorder_it->MOrderType == 1)m_bType1StoplossClose = true;
					if (closeorder_it->MOrderType == 2)Type2StoplossClose = true;
				}
				else if (timeRuleForCloseNew(closeorder_it->OpenTime, pDepthMarketData->datadate, pDepthMarketData->updatetime)
					&& closeorder_it->IsClosePofitOrder) {
					char line[200];
					sprintf(line, "%s,Buy Order timeRuleForCloseNew Close,OrderId=%d", mStrategyAndInstance.c_str(), closeorder_it->OrderId);
					WriteMsgToLogList(line);

					closeorder_it->NextCloseOrderPrice = m_Sell1 - m_dOneTick;
					ReqOrderDelete(closeorder_it->OrderId, closeorder_it->OrderLocalRef, closeorder_it->FrontID, closeorder_it->SessionID);
				}
				else if ((closeorder_it->LimitPrice - 0.00001) > m_Sell1&& closeorder_it->IsStoplessOrder&& closeorder_it->CanbeCanceled) {
					char line[200];
					sprintf(line, "%s,Buy Order Close ReSubmit,OrderId=%d", mStrategyAndInstance.c_str(), closeorder_it->OrderId);
					WriteMsgToLogList(line);

					//TRACE("Cancel the stopless order for buy order,stopless price=%.1f,sell1=%.1f\n",closeorder_it->LimitPrice,m_Sell1);
					closeorder_it->NextCloseOrderPrice = m_Buy1;
					ReqOrderDelete(closeorder_it->OrderId, closeorder_it->OrderLocalRef, closeorder_it->FrontID, closeorder_it->SessionID);
					closeorder_it->CanbeCanceled = false;
				}
			}//End IF Closing buy order

			if (closeorder_it->Direction == MORDER_BUY &&
				(closeorder_it->OrderStatus == MORDER_PART_TRADED || closeorder_it->OrderStatus == MORDER_ACCEPTED || closeorder_it->OrderStatus == MORDER_QUEUED)) {
				//onhandSellOrder++;
				//Closing sell order
				closeorder_it->maxProfit = max(closeorder_it->maxProfit, closeorder_it->OpenOrderTradePrice - m_Price);
				//if((closeorder_it->maxProfit+0.00001)>10){
				//	closeorder_it->mStoplossPoint=min(0,10-closeorder_it->maxProfit);
				//}
				if (closeorder_it->IsClosePofitOrder && (((m_Price - closeorder_it->OpenOrderTradePrice) >= (mStrategyParams.FirstStoplossPoint * m_dOneTick - 0.00001) && closeorder_it->MOrderType == 1)
					|| ((m_Price - closeorder_it->OpenOrderTradePrice) >= (mStrategyParams.SecondStoplossPoint * m_dOneTick - 0.00001) && closeorder_it->MOrderType == 2))
					) {
					char line[200];
					sprintf(line, "%s,Sell Order Stoploss Close,%.5f,OrderId=%d", mStrategyAndInstance.c_str(), m_Price, closeorder_it->OrderId);
					WriteMsgToLogList(line);
					closeorder_it->NextCloseOrderPrice = m_Buy1 + m_dOneTick;
					ReqOrderDelete(closeorder_it->OrderId, closeorder_it->OrderLocalRef, closeorder_it->FrontID, closeorder_it->SessionID);
					if (closeorder_it->MOrderType == 1)m_bType1StoplossClose = true;
					if (closeorder_it->MOrderType == 2)Type2StoplossClose = true;
				}
				else if (timeRuleForCloseNew(closeorder_it->OpenTime, pDepthMarketData->datadate, pDepthMarketData->updatetime)
					&& closeorder_it->IsClosePofitOrder) {
					char line[200];
					sprintf(line, "%s,Sell Order timeRuleForCloseNew Close,OrderId=%d", mStrategyAndInstance.c_str(), closeorder_it->OrderId);
					WriteMsgToLogList(line);

					closeorder_it->NextCloseOrderPrice = m_Buy1 + m_dOneTick;
					ReqOrderDelete(closeorder_it->OrderId, closeorder_it->OrderLocalRef, closeorder_it->FrontID, closeorder_it->SessionID);
				}
				else if (closeorder_it->LimitPrice < (m_Buy1 - 0.00001) && closeorder_it->IsStoplessOrder && closeorder_it->CanbeCanceled) {
					char line[200];
					sprintf(line, "%s,Sell Order Close ReSubmit,OrderId=%d", mStrategyAndInstance.c_str(), closeorder_it->OrderId);
					WriteMsgToLogList(line);
					//TRACE("Cancel the stopless order for sell order,stopless price=%.1f,buy1=%.1f\n",closeorder_it->LimitPrice,m_Buy1);
					closeorder_it->NextCloseOrderPrice = m_Sell1;
					ReqOrderDelete(closeorder_it->OrderId, closeorder_it->OrderLocalRef, closeorder_it->FrontID, closeorder_it->SessionID);
					closeorder_it->CanbeCanceled = false;
				}
			}//End IF Closing sell order
			//}//End for Strategy 1
		}//End Looping close order list
	}//End if the close order list is not null

	Type1OrderExist = false;
	Type2OrderExist = false;
	bool x_bType2BuyOrderExist = false;
	bool x_bType1BuyOrderExist = false;
	bool x_bType2SellOrderExist = false;
	bool x_bType1SellOrderExist = false;
	//Looping Opened Order to Check Type1Order
	std::list<MyOpenOrderType>::iterator order_itr;
	if (!OpenOrderList.empty()) {
		for (order_itr = OpenOrderList.begin(); order_itr != OpenOrderList.end(); ++order_itr) {
			if (order_itr->MOrderType == 1) {
				Type1OrderExist = true;
				if (order_itr->Direction == MORDER_BUY) {
					x_bType1BuyOrderExist = true;
				}
				else if (order_itr->Direction == MORDER_SELL) {
					x_bType1SellOrderExist = true;
				}
			}
			else if (order_itr->MOrderType == 2) {
				Type2OrderExist = true;
				if (order_itr->Direction == MORDER_BUY) {
					x_bType2BuyOrderExist = true;
				}
				else if (order_itr->Direction == MORDER_SELL) {
					x_bType2SellOrderExist = true;
				}
			}
		}//end for loop
	}
	std::list<MyCloseOrderType>::iterator corder_itr;
	if (!CloseOrderList.empty()) {
		for (corder_itr = CloseOrderList.begin(); corder_itr != CloseOrderList.end(); ++corder_itr) {
			if (corder_itr->MOrderType == 1) {
				Type1OrderExist = true;
				if (corder_itr->Direction == MORDER_SELL) {
					x_bType1BuyOrderExist = true;
				}
				else if (corder_itr->Direction == MORDER_BUY) {
					x_bType1SellOrderExist = true;
				}
			}
			else if (corder_itr->MOrderType == 2) {
				Type2OrderExist = true;
				if (corder_itr->Direction == MORDER_SELL) {
					x_bType2BuyOrderExist = true;
				}
				else if (corder_itr->Direction == MORDER_BUY) {
					x_bType2SellOrderExist = true;
				}
			}
		}//end for loop
	}
	// 订单1类型m_bType1StoplossClose 订单2类型Type2StoplossClose 第二天置位
	if (timeRuleForOpen(pDepthMarketData->datadate, pDepthMarketData->updatetime) && !m_bType1StoplossClose && !Type2StoplossClose) {
		bool BuyOrderSubmitted = false; bool SellOrderSubmitted = false;
		if (EnterBuyTrade == 1
			&& mOpenTimes < mStrategyParams.LoopTimes
			&& mStrategyParams.OpenBuyAllow
			&& !x_bType1BuyOrderExist && !x_bType2SellOrderExist
			&& (OnHandPositionCount + mStrategyParams.OpenVol1) <= MaxOnHandPositionCount && (TotalOnHandPosition + mStrategyParams.OpenVol1) <= MaxTotalOnHandPosition) {
			//Open Buy Order
			MyOpenOrderType openThostOrder;
			iNextOrderRef++;
			openThostOrder.OrderLocalRetReqID = 0;
			openThostOrder.OrderId = -1;
			openThostOrder.LimitPrice = mOpenPrice - mStrategyParams.OpenDistPoint * m_dOneTick;
			openThostOrder.OrigSubmitPrice = openThostOrder.LimitPrice;
			openThostOrder.Direction = MORDER_BUY;
			openThostOrder.Offset = MORDER_OPEN;
			openThostOrder.VolumeTotal = mStrategyParams.OpenVol1;
			openThostOrder.VolumeTotalOriginal = openThostOrder.VolumeTotal;
			openThostOrder.VolumeTraded = 0;
			openThostOrder.ProfitPrice = openThostOrder.LimitPrice + mStrategyParams.FirstProfitPoint * m_dOneTick;
			openThostOrder.OpenOrderCanbeCanceled = true;
			openThostOrder.MOrderType = 1;

			openThostOrder.maxProfit = 0.0;

			Type1OrderExist = true;
			openThostOrder.mStoplossPoint = mStrategyParams.FirstStoplossPoint;
			ReqOpenOrderInsert(&openThostOrder);

			OnHandPositionCount += openThostOrder.VolumeTotal;
			TotalOnHandPosition += openThostOrder.VolumeTotal;

			BuyOrderSubmitted = true;
			m_bType1StoplossClose = false;

			char log[200];
			sprintf(log, "%s,开仓,买=%.5f,时间=%s,手数=%d", mStrategyAndInstance.c_str(), openThostOrder.LimitPrice, pDepthMarketData->updatetime, openThostOrder.VolumeTotal);
			AddtoTipMsgListBox(log);
			WriteMsgToLogList(log);
		}

		if (EnterSellTrade == 1
			&& mOpenTimes < mStrategyParams.LoopTimes
			&& mStrategyParams.OpenSellAllow
			&& !x_bType1SellOrderExist && !x_bType2BuyOrderExist
			&& (OnHandPositionCount + mStrategyParams.OpenVol1) <= MaxOnHandPositionCount && (TotalOnHandPosition + mStrategyParams.OpenVol1) <= MaxTotalOnHandPosition) {
			//Open Sell Order
			MyOpenOrderType openThostOrder;
			iNextOrderRef++;
			openThostOrder.OrderLocalRetReqID = 0;
			openThostOrder.OrderId = -1;
			openThostOrder.LimitPrice = mOpenPrice + mStrategyParams.OpenDistPoint * m_dOneTick;
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

			Type1OrderExist = true;
			openThostOrder.mStoplossPoint = mStrategyParams.FirstStoplossPoint;
			ReqOpenOrderInsert(&openThostOrder);

			OnHandPositionCount += openThostOrder.VolumeTotal;
			TotalOnHandPosition += openThostOrder.VolumeTotal;

			SellOrderSubmitted = true;
			m_bType1StoplossClose = false;

			char log[200];
			sprintf(log, "%s,开仓,卖=%.5f,时间=%s,手数=%d", mStrategyAndInstance.c_str(), openThostOrder.LimitPrice, pDepthMarketData->updatetime, openThostOrder.VolumeTotal);
			AddtoTipMsgListBox(log);
			WriteMsgToLogList(log);
		}
		if (BuyOrderSubmitted || SellOrderSubmitted)mOpenTimes++;
	}

	FlushStrategyInfoToFile();
}

void StrategyOpenPriceOpening::CheckAndResubmitOutOfDateOrder(char cTickDataDate[11], char cTickUpdateTime[11]) {
	int tickHour, tickMin, tickSec;
	sscanf(cTickUpdateTime, "%d:%d:%d", &tickHour, &tickMin, &tickSec);
	int tickDate = (atoi(cTickDataDate) % 10000) % 100;

	std::list<MyOpenOrderType>::iterator openorder_it;
	for (openorder_it = OpenOrderList.begin(); openorder_it != OpenOrderList.end();) {
		std::list<MyOpenOrderType>::iterator iter_e = openorder_it++;
		string strOpenDateTime(iter_e->SubmitDateAndTime);
		string strDate = strOpenDateTime.substr(0, strOpenDateTime.find_first_of(" "));
		int nDate_m = (atoi(strDate.c_str()) % 10000) % 100;
		int nHour, nMin, nSec;
		string strSubUpdateTime = strOpenDateTime.substr(strOpenDateTime.find_last_of(" ") + 1, strOpenDateTime.length() - strOpenDateTime.find_last_of(" "));
		sscanf(strSubUpdateTime.c_str(), "%d:%d:%d", &nHour, &nMin, &nSec);

		if (((tickDate != nDate_m && strDate.length() == 8 && nDate_m > 0 && (tickHour == 0 || (tickHour >= 9 && tickHour < 15) || (tickHour >= 21)))//不是同一自然日的订单都重报一遍
			|| ((tickDate == nDate_m) && strDate.length() == 8 && nDate_m > 0 && nHour < 20 && tickHour >= 20)//相同自然日，tick时间和订单时间跨晚上20:00的也重新报一遍
			) && iter_e->OrderId > 0) {
			MyOpenOrderType openThostOrder;
			openThostOrder.OrderLocalRetReqID = 0;
			openThostOrder.OrderId = -1;
			openThostOrder.LimitPrice = iter_e->LimitPrice;
			openThostOrder.OrigSubmitPrice = openThostOrder.LimitPrice;
			openThostOrder.Direction = iter_e->Direction;
			openThostOrder.Offset = iter_e->Offset;
			openThostOrder.VolumeTotal = iter_e->VolumeTotal;
			openThostOrder.VolumeTotalOriginal = iter_e->VolumeTotalOriginal;
			openThostOrder.VolumeTraded = iter_e->VolumeTraded;
			openThostOrder.ProfitPrice = iter_e->ProfitPrice;
			openThostOrder.OpenOrderCanbeCanceled = true;
			openThostOrder.MOrderType = iter_e->MOrderType;
			openThostOrder.maxProfit = iter_e->maxProfit;
			openThostOrder.mStoplossPoint = iter_e->mStoplossPoint;
			string tmp(cTickDataDate);
			tmp.append(" ");
			tmp.append(cTickUpdateTime);
			strcpy(openThostOrder.SubmitDateAndTime, tmp.c_str());
			strcpy(openThostOrder.OpenTime, iter_e->OpenTime);

			ReqOrderDelete(iter_e->OrderId, iter_e->OrderLocalRef, iter_e->FrontID, iter_e->SessionID);

			ReqOpenOrderInsert(&openThostOrder);

			OpenOrderList.erase(iter_e);
		}
	}

	std::list<MyCloseOrderType>::iterator closeorder_it;
	for (closeorder_it = CloseOrderList.begin(); closeorder_it != CloseOrderList.end();) {
		std::list<MyCloseOrderType>::iterator iter_e = closeorder_it++;

		string strOpenDateTime(iter_e->SubmitDateAndTime);
		string strDate = strOpenDateTime.substr(0, strOpenDateTime.find_first_of(" "));
		int nDate_m = (atoi(strDate.c_str()) % 10000) % 100;
		int nHour, nMin, nSec;
		string strSubUpdateTime = strOpenDateTime.substr(strOpenDateTime.find_last_of(" ") + 1, strOpenDateTime.length() - strOpenDateTime.find_last_of(" "));
		sscanf(strSubUpdateTime.c_str(), "%d:%d:%d", &nHour, &nMin, &nSec);

		if (((tickDate != nDate_m && strDate.length() == 8 && nDate_m > 0 && (tickHour == 0 || (tickHour >= 9 && tickHour < 15) || (tickHour >= 21)))//不是同一自然日的订单都重报一遍
			|| ((tickDate == nDate_m) && strDate.length() == 8 && nDate_m > 0 && nHour < 20 && tickHour >= 20)//相同自然日，tick时间和订单时间跨晚上20:00的也重新报一遍
			) && iter_e->OrderId > 0) {
			//报平仓单到交易所
			MyCloseOrderType thostOrder;
			thostOrder.OpenOrderSubmitPrice = iter_e->OpenOrderSubmitPrice;
			thostOrder.OpenOrderTradePrice = iter_e->OpenOrderTradePrice;
			thostOrder.IsClosePofitOrder = iter_e->IsClosePofitOrder;
			thostOrder.IsStoplessOrder = iter_e->IsStoplessOrder;
			thostOrder.CanbeCanceled = true;
			thostOrder.dwCloseOrderStart = GetTickCount();
			thostOrder.MOrderType = iter_e->MOrderType;
			thostOrder.MAStop = iter_e->MAStop;

			string tmp(cTickDataDate);
			tmp.append(" ");
			tmp.append(cTickUpdateTime);
			strcpy(thostOrder.SubmitDateAndTime, tmp.c_str());
			strcpy(thostOrder.OpenTime, iter_e->OpenTime);

			iNextOrderRef++;
			thostOrder.OrderLocalRetReqID = 0;
			thostOrder.Offset = iter_e->Offset;
			thostOrder.OrderStatus = iter_e->OrderStatus;
			thostOrder.Direction = iter_e->Direction;
			thostOrder.LimitPrice = iter_e->LimitPrice;
			thostOrder.OrigSubmitPrice = iter_e->OrigSubmitPrice;
			thostOrder.ProfitPrice = iter_e->ProfitPrice;
			thostOrder.VolumeTotalOriginal = iter_e->VolumeTotalOriginal;
			thostOrder.VolumeTraded = iter_e->VolumeTraded;
			thostOrder.VolumeTotal = iter_e->VolumeTotal;//
			thostOrder.maxProfit = iter_e->maxProfit;
			thostOrder.mStoplossPoint = iter_e->mStoplossPoint;
			ReqCloseOrderInsert(&thostOrder, thostOrder.OpenTime);

			ReqOrderDelete(iter_e->OrderId, iter_e->OrderLocalRef, iter_e->FrontID, iter_e->SessionID);

			CloseOrderList.erase(iter_e);
		}
	}
}

void StrategyOpenPriceOpening::OnRtnOrder(OrderTradeMsg* pOrderTradeMsg)
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
			if (openorder_it->OrderId == thisOrderId || (openorder_it->OrderLocalRef == pOrderTradeMsg->OrderLocalRef && openorder_it->OrderLocalRef > 0 && (ThostTraderAPI || SgitTraderAPI))) {
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
					if (openorder_it->MOrderType == 2)mOpenTimes--;

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
			if (closeorder_it->OrderId == thisOrderId || (closeorder_it->OrderLocalRef == pOrderTradeMsg->OrderLocalRef && closeorder_it->OrderLocalRef > 0 && (ThostTraderAPI || SgitTraderAPI))) {
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
				if (pOrderTradeMsg->OrderStatus == MORDER_PART_CANCELLED || pOrderTradeMsg->OrderStatus == MORDER_CANCELLED)
				{
					if (closeorder_it->IsClosePofitOrder) {
						//报到交易所的止盈单被撤
						if (closeorder_it->Direction == MORDER_SELL)
						{
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
							newThostOrder.LimitPrice = m_Buy1;//以最新价进行报止损单
							newThostOrder.OrigSubmitPrice = newThostOrder.LimitPrice;
							newThostOrder.Offset = closeorder_it->Offset;
							newThostOrder.VolumeTotalOriginal = closeorder_it->VolumeTotal;
							newThostOrder.VolumeTotal = newThostOrder.VolumeTotalOriginal;
							newThostOrder.VolumeTraded = 0;
							newThostOrder.OpenOrderID = closeorder_it->OpenOrderID;

							//if(closeorder_it->MOrderType==2)Type2StoplossClose=true;//类型2订单触碰止损线,程序不再允许开仓

							ReqCloseOrderInsert(&newThostOrder, newThostOrder.OpenTime);
						}
						else if (closeorder_it->Direction == MORDER_BUY)
						{
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
							newThostOrder.LimitPrice = m_Sell1;//以最新价进行报止损单
							newThostOrder.OrigSubmitPrice = newThostOrder.LimitPrice;
							newThostOrder.Offset = closeorder_it->Offset;
							newThostOrder.VolumeTotalOriginal = closeorder_it->VolumeTotal;
							newThostOrder.VolumeTotal = newThostOrder.VolumeTotalOriginal;
							newThostOrder.VolumeTraded = 0;
							newThostOrder.OpenOrderID = closeorder_it->OpenOrderID;

							//if(closeorder_it->MOrderType==2)Type2StoplossClose=true;//类型2订单触碰止损线,程序不再允许开仓

							ReqCloseOrderInsert(&newThostOrder, newThostOrder.OpenTime);
						}
						CloseOrderList.erase(closeorder_it);
						break;
					}
					else if (closeorder_it->IsStoplessOrder) {
						//止损单被撤
						if (closeorder_it->Direction == MORDER_SELL)
						{
							MyCloseOrderType newThostOrder;
							iNextOrderRef++;
							newThostOrder.OrderLocalRetReqID = 0;
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
							newThostOrder.OpenOrderID = closeorder_it->OpenOrderID;

							ReqCloseOrderInsert(&newThostOrder, newThostOrder.OpenTime);
						}
						else if (closeorder_it->Direction == MORDER_BUY)
						{
							MyCloseOrderType newThostOrder;
							iNextOrderRef++;
							newThostOrder.OrderLocalRetReqID = 0;
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
							newThostOrder.OpenOrderID = closeorder_it->OpenOrderID;

							ReqCloseOrderInsert(&newThostOrder, newThostOrder.OpenTime);
						}
					}
					CloseOrderList.erase(closeorder_it);
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

void StrategyOpenPriceOpening::FlushStrategyInfoToFile() {
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
	memcpy(header.SpecificArea + sizeof(double), &mOpenTimes, sizeof(int));

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

void StrategyOpenPriceOpening::OnRtnTrade(OrderTradeMsg* pOrderTradeMsg)
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
		// add by Allen
		char tmpTime[32] = { 0 };
		int nHour, nMin, nSec;
		strcpy(tmpTime, pOrderTradeMsg->InsertOrTradeTime);
		if (strlen(tmpTime) >= 17) {
			char* datetime = strstr(tmpTime, " ");
			*(datetime++) = '\0';
			sscanf_s(datetime, "%d:%d:%d", &nHour, &nMin, &nSec);
		}

		double submitprice = 0;
		int tradedirection = -1;
		int openorclose = -1;
		std::list<MyOpenOrderType>::iterator openorder_it;
		if (!OpenOrderList.empty()) {
			for (openorder_it = OpenOrderList.begin(); openorder_it != OpenOrderList.end(); openorder_it++) {
				//std::list<MyOpenOrderType>::iterator iter_e=openorder_it++;
				if (openorder_it->OrderId == thisTradeRef || (openorder_it->OrderLocalRef == pOrderTradeMsg->OrderLocalRef && openorder_it->OrderLocalRef > 0 && (ThostTraderAPI || SgitTraderAPI))) {
					//printf("OpenOrderList exist order \n");
					if (openorder_it->OrderLocalRef == pOrderTradeMsg->OrderLocalRef && openorder_it->OrderLocalRef > 0)openorder_it->OrderId = thisTradeRef;

					openorclose = MORDER_OPEN;
					openorder_it->VolumeTraded = openorder_it->VolumeTraded + pOrderTradeMsg->Volume;
					openorder_it->VolumeTotal = openorder_it->VolumeTotal - pOrderTradeMsg->Volume;
					strcpy(openorder_it->OpenTime, pOrderTradeMsg->InsertOrTradeTime);

					if (openorder_it->Direction == MORDER_BUY)
					{
						tradedirection = MORDER_BUY;
						//报平仓单到交易所
						MyCloseOrderType thostOrder;
						thostOrder.OpenOrderSubmitPrice = openorder_it->OrigSubmitPrice;
						thostOrder.OpenOrderTradePrice = pOrderTradeMsg->Price;
						thostOrder.IsClosePofitOrder = true;
						thostOrder.IsStoplessOrder = false;
						thostOrder.CanbeCanceled = true;
						thostOrder.dwCloseOrderStart = GetTickCount();
						thostOrder.MOrderType = openorder_it->MOrderType;
						thostOrder.MAStop = openorder_it->MAStop;
						strcpy(thostOrder.OpenTime, pOrderTradeMsg->InsertOrTradeTime);
						iNextOrderRef++;
						thostOrder.OrderLocalRetReqID = 0;
						thostOrder.Offset = MORDER_CLOSE;
						thostOrder.OrderStatus = MORDER_ACCEPTED;
						thostOrder.Direction = MORDER_SELL;
						thostOrder.LimitPrice = openorder_it->ProfitPrice;
						thostOrder.OrigSubmitPrice = thostOrder.LimitPrice;
						thostOrder.ProfitPrice = openorder_it->ProfitPrice;
						thostOrder.VolumeTotalOriginal = pOrderTradeMsg->Volume;
						thostOrder.VolumeTraded = 0;
						thostOrder.VolumeTotal = pOrderTradeMsg->Volume;//
						thostOrder.maxProfit = openorder_it->maxProfit;
						thostOrder.mStoplossPoint = openorder_it->mStoplossPoint;
						thostOrder.OpenOrderID = thisTradeRef;
						ReqCloseOrderInsert(&thostOrder, openorder_it->OpenTime);

						//检查是否存在第二单
						bool mType2OrderExist = false;
						//Looping Opened Order to Check Type1Order
						std::list<MyOpenOrderType>::iterator order_itr;
						for (order_itr = OpenOrderList.begin(); order_itr != OpenOrderList.end(); ++order_itr) {
							if (order_itr->MOrderType == 2 && order_itr->VolumeTotal > 0 && order_itr->Direction == MORDER_BUY) {
								mType2OrderExist = true;
							}
						}//end for loop

						std::list<MyCloseOrderType>::iterator corder_itr;
						for (corder_itr = CloseOrderList.begin(); corder_itr != CloseOrderList.end(); ++corder_itr) {
							if (corder_itr->MOrderType == 2 && corder_itr->VolumeTotal > 0 && corder_itr->Direction == MORDER_SELL) {
								mType2OrderExist = true;
							}
						}//end for loop
						//报第二个开仓单到交易所
						if (!timeRuleForStop(nHour, nMin, nSec) && openorder_it->MOrderType == 1 && mOpenTimes < mStrategyParams.LoopTimes
							&& mOpenPrice>0.00001
							&& !mType2OrderExist
							&& (OnHandPositionCount + mStrategyParams.OpenVol2) <= MaxOnHandPositionCount
							&& (TotalOnHandPosition + mStrategyParams.OpenVol2) <= MaxTotalOnHandPosition
							&& openorder_it->VolumeTotal == 0
							) {
							char line[200];
							sprintf(line, "%s,Open Type 2 Order,Buy,OrderId=%d", mStrategyAndInstance.c_str(), openorder_it->OrderId);
							WriteMsgToLogList(line);

							MyOpenOrderType openThostOrder;
							iNextOrderRef++;
							openThostOrder.OrderLocalRetReqID = 0;
							openThostOrder.OrderId = -1;
							openThostOrder.LimitPrice = mOpenPrice - mStrategyParams.SecondOpenDistPoint * m_dOneTick;
							openThostOrder.OrigSubmitPrice = openThostOrder.LimitPrice;
							openThostOrder.Direction = MORDER_BUY;
							openThostOrder.Offset = MORDER_OPEN;
							openThostOrder.VolumeTotal = mStrategyParams.OpenVol2;
							openThostOrder.VolumeTotalOriginal = openThostOrder.VolumeTotal;
							openThostOrder.VolumeTraded = 0;
							openThostOrder.ProfitPrice = openThostOrder.LimitPrice + mStrategyParams.SecondProfitPoint * m_dOneTick;
							openThostOrder.OpenOrderCanbeCanceled = true;
							openThostOrder.MOrderType = 2;
							openThostOrder.maxProfit = 0.0;

							OnHandPositionCount += openThostOrder.VolumeTotal;
							TotalOnHandPosition += openThostOrder.VolumeTotal;
							mOpenTimes++;

							Type2OrderExist = true;
							openThostOrder.mStoplossPoint = mStrategyParams.SecondStoplossPoint;
							ReqOpenOrderInsert(&openThostOrder);
						}
						//strcpy(thostOrder.OpenTime,gEqualVolRatesX[gEqualVolRatesXIndex].datatime);
						//CloseOrderList.push_back(thostOrder);

						submitprice = openorder_it->OrigSubmitPrice;

						if (openorder_it->VolumeTotal == 0) {
							map<int, int>::iterator iter;
							AcquireSRWLockExclusive(&g_srwLockOrderId);
							iter = OrderIdToShmIndex.find(pOrderTradeMsg->OrderSysId);
							if (iter != OrderIdToShmIndex.end()) {
								OrderIdToShmIndex.erase(iter);
							}
							ReleaseSRWLockExclusive(&g_srwLockOrderId);

							OpenOrderList.erase(openorder_it);
						}

						//撤销另一侧的类型1的卖开仓报单,
						std::list<MyOpenOrderType>::iterator openorder_itother;
						for (openorder_itother = OpenOrderList.begin(); openorder_itother != OpenOrderList.end(); openorder_itother++) {
							if (openorder_itother->Direction == MORDER_SELL && openorder_itother->MOrderType == 1) {
								ReqOrderDelete(openorder_itother->OrderId, openorder_itother->OrderLocalRef, openorder_itother->FrontID, openorder_itother->SessionID);
								openorder_itother->OpenOrderCanbeCanceled = false;
							}
						}
						break;
					}
					else if (openorder_it->Direction == MORDER_SELL) {
						tradedirection = MORDER_SELL;
						//报平仓单到交易所
						MyCloseOrderType thostOrder;
						thostOrder.OpenOrderSubmitPrice = openorder_it->OrigSubmitPrice;
						thostOrder.OpenOrderTradePrice = pOrderTradeMsg->Price;
						thostOrder.IsClosePofitOrder = true;
						thostOrder.IsStoplessOrder = false;
						thostOrder.CanbeCanceled = true;
						thostOrder.dwCloseOrderStart = GetTickCount();
						thostOrder.MOrderType = openorder_it->MOrderType;
						thostOrder.MAStop = openorder_it->MAStop;
						strcpy(thostOrder.OpenTime, pOrderTradeMsg->InsertOrTradeTime);
						iNextOrderRef++;
						thostOrder.OrderLocalRetReqID = 0;
						thostOrder.Offset = MORDER_CLOSE;
						thostOrder.OrderStatus = MORDER_ACCEPTED;
						thostOrder.Direction = MORDER_BUY;
						thostOrder.LimitPrice = openorder_it->ProfitPrice;
						thostOrder.OrigSubmitPrice = thostOrder.LimitPrice;
						thostOrder.ProfitPrice = openorder_it->ProfitPrice;
						thostOrder.VolumeTotalOriginal = pOrderTradeMsg->Volume;
						thostOrder.VolumeTraded = 0;
						thostOrder.VolumeTotal = pOrderTradeMsg->Volume;//脢拢脫脿脢媒脕驴
						thostOrder.maxProfit = openorder_it->maxProfit;
						thostOrder.mStoplossPoint = openorder_it->mStoplossPoint;
						thostOrder.OpenOrderID = thisTradeRef;

						//strcpy(thostOrder.OpenTime,gEqualVolRatesX[gEqualVolRatesXIndex].datatime);
						//CloseOrderList.push_back(thostOrder);
						ReqCloseOrderInsert(&thostOrder, openorder_it->OpenTime);

						//检查是否存在第二单
						bool mType2OrderExist = false;
						//Looping Opened Order to Check Type1Order
						std::list<MyOpenOrderType>::iterator order_itr;
						for (order_itr = OpenOrderList.begin(); order_itr != OpenOrderList.end(); ++order_itr) {
							if (order_itr->MOrderType == 2 && order_itr->VolumeTotal > 0 && order_itr->Direction == MORDER_SELL) {
								mType2OrderExist = true;
							}
						}//end for loop

						std::list<MyCloseOrderType>::iterator corder_itr;
						for (corder_itr = CloseOrderList.begin(); corder_itr != CloseOrderList.end(); ++corder_itr) {
							if (corder_itr->MOrderType == 2 && corder_itr->VolumeTotal > 0 && corder_itr->Direction == MORDER_BUY) {
								mType2OrderExist = true;
							}
						}//end for loop
						//报第二个开仓单到交易所
						if (!timeRuleForStop(nHour, nMin, nSec) && openorder_it->MOrderType == 1 && mOpenTimes < mStrategyParams.LoopTimes
							&& mOpenPrice>0.00001
							&& !mType2OrderExist
							&& (OnHandPositionCount + mStrategyParams.OpenVol2) <= MaxOnHandPositionCount
							&& (TotalOnHandPosition + mStrategyParams.OpenVol2) <= MaxTotalOnHandPosition
							&& openorder_it->VolumeTotal == 0
							) {
							char line[200];
							sprintf(line, "%s,Open Type 2 Order,Sell,OrderId=%d", mStrategyAndInstance.c_str(), openorder_it->OrderId);
							WriteMsgToLogList(line);

							//存在类型1的订单且价格继续下降突破入场的第二档,加开买单
							//Open Buy Order
							MyOpenOrderType openThostOrder;
							iNextOrderRef++;
							openThostOrder.OrderLocalRetReqID = 0;
							openThostOrder.OrderId = -1;
							openThostOrder.LimitPrice = mOpenPrice + mStrategyParams.SecondOpenDistPoint * m_dOneTick;
							openThostOrder.OrigSubmitPrice = openThostOrder.LimitPrice;
							openThostOrder.Direction = MORDER_SELL;
							openThostOrder.Offset = MORDER_OPEN;
							openThostOrder.VolumeTotal = mStrategyParams.OpenVol2;
							openThostOrder.VolumeTotalOriginal = openThostOrder.VolumeTotal;
							openThostOrder.VolumeTraded = 0;
							openThostOrder.ProfitPrice = openThostOrder.LimitPrice - mStrategyParams.SecondProfitPoint * m_dOneTick;
							openThostOrder.OpenOrderCanbeCanceled = true;
							openThostOrder.MOrderType = 2;
							openThostOrder.maxProfit = 0.0;

							OnHandPositionCount += openThostOrder.VolumeTotal;
							TotalOnHandPosition += openThostOrder.VolumeTotal;
							mOpenTimes++;

							Type2OrderExist = true;
							openThostOrder.mStoplossPoint = mStrategyParams.SecondStoplossPoint;
							ReqOpenOrderInsert(&openThostOrder);
						}
						submitprice = openorder_it->OrigSubmitPrice;
						if (openorder_it->VolumeTotal == 0) {
							map<int, int>::iterator iter;
							AcquireSRWLockExclusive(&g_srwLockOrderId);
							iter = OrderIdToShmIndex.find(pOrderTradeMsg->OrderSysId);
							if (iter != OrderIdToShmIndex.end()) {
								OrderIdToShmIndex.erase(iter);
							}
							ReleaseSRWLockExclusive(&g_srwLockOrderId);

							OpenOrderList.erase(openorder_it);
						}
						//撤销另一侧的类型1的买开仓报单
						std::list<MyOpenOrderType>::iterator openorder_itother;
						for (openorder_itother = OpenOrderList.begin(); openorder_itother != OpenOrderList.end(); openorder_itother++) {
							if (openorder_itother->Direction == MORDER_BUY && openorder_itother->MOrderType == 1) {
								ReqOrderDelete(openorder_itother->OrderId, openorder_itother->OrderLocalRef, openorder_itother->FrontID, openorder_itother->SessionID);
								openorder_itother->OpenOrderCanbeCanceled = false;
							}
						}
						break;
					}
					else {
						printf("openDirection Excetion!!! =%i\n", pOrderTradeMsg->Direction);
					}
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

					openid = closeorder_it->OpenOrderID;

					if (pOrderTradeMsg->Volume == closeorder_it->VolumeTotalOriginal) {
						//第二单止盈成交,继续报第二个开仓单到交易所
						bool mType2OrderExist = false;
						//Looping Opened Order to Check Type1Order
						std::list<MyOpenOrderType>::iterator order_itr;
						for (order_itr = OpenOrderList.begin(); order_itr != OpenOrderList.end(); ++order_itr) {
							if (order_itr->MOrderType == 2 && order_itr->VolumeTotal > 0) {
								mType2OrderExist = true;
							}
						}//end for loop

						std::list<MyCloseOrderType>::iterator corder_itr;
						for (corder_itr = CloseOrderList.begin(); corder_itr != CloseOrderList.end(); ++corder_itr) {
							if (corder_itr->MOrderType == 2 && corder_itr->VolumeTotal > 0) {
								mType2OrderExist = true;
							}
						}//end for loop

						if (!timeRuleForStop(nHour, nMin, nSec) && closeorder_it->MOrderType == 2 && closeorder_it->IsClosePofitOrder && mOpenTimes < mStrategyParams.LoopTimes
							&& mOpenPrice>0.00001
							&& (OnHandPositionCount + mStrategyParams.OpenVol2) <= MaxOnHandPositionCount
							&& (TotalOnHandPosition + mStrategyParams.OpenVol2) <= MaxTotalOnHandPosition
							&& !mType2OrderExist
							) {
							if (closeorder_it->Direction == MORDER_SELL) {
								char line[200];
								sprintf(line, "%s,Open Type 2 Order,Buy,COrderId=%d", mStrategyAndInstance.c_str(), closeorder_it->OrderId);
								WriteMsgToLogList(line);

								MyOpenOrderType openThostOrder;
								iNextOrderRef++;
								openThostOrder.OrderLocalRetReqID = 0;
								openThostOrder.OrderId = -1;
								openThostOrder.LimitPrice = mOpenPrice - mStrategyParams.SecondOpenDistPoint * m_dOneTick;
								openThostOrder.OrigSubmitPrice = openThostOrder.LimitPrice;
								openThostOrder.Direction = MORDER_BUY;
								openThostOrder.Offset = MORDER_OPEN;
								openThostOrder.VolumeTotal = mStrategyParams.OpenVol2;
								openThostOrder.VolumeTotalOriginal = openThostOrder.VolumeTotal;
								openThostOrder.VolumeTraded = 0;
								openThostOrder.ProfitPrice = openThostOrder.LimitPrice + mStrategyParams.SecondProfitPoint * m_dOneTick;
								openThostOrder.OpenOrderCanbeCanceled = true;
								openThostOrder.MOrderType = 2;
								openThostOrder.maxProfit = 0.0;

								OnHandPositionCount += openThostOrder.VolumeTotal;
								TotalOnHandPosition += openThostOrder.VolumeTotal;
								mOpenTimes++;

								Type2OrderExist = true;
								openThostOrder.mStoplossPoint = mStrategyParams.SecondStoplossPoint;
								ReqOpenOrderInsert(&openThostOrder);
							}
							else if (closeorder_it->Direction == MORDER_BUY) {
								char line[200];
								sprintf(line, "%s,Open Type 2 Order,Sell,COrderId=%d", mStrategyAndInstance.c_str(), closeorder_it->OrderId);
								WriteMsgToLogList(line);

								MyOpenOrderType openThostOrder;
								iNextOrderRef++;
								openThostOrder.OrderLocalRetReqID = 0;
								openThostOrder.OrderId = -1;
								openThostOrder.LimitPrice = mOpenPrice + mStrategyParams.SecondOpenDistPoint * m_dOneTick;
								openThostOrder.OrigSubmitPrice = openThostOrder.LimitPrice;
								openThostOrder.Direction = MORDER_SELL;
								openThostOrder.Offset = MORDER_OPEN;
								openThostOrder.VolumeTotal = mStrategyParams.OpenVol2;
								openThostOrder.VolumeTotalOriginal = openThostOrder.VolumeTotal;
								openThostOrder.VolumeTraded = 0;
								openThostOrder.ProfitPrice = openThostOrder.LimitPrice - mStrategyParams.SecondProfitPoint * m_dOneTick;
								openThostOrder.OpenOrderCanbeCanceled = true;
								openThostOrder.MOrderType = 2;
								openThostOrder.maxProfit = 0.0;

								OnHandPositionCount += openThostOrder.VolumeTotal;
								TotalOnHandPosition += openThostOrder.VolumeTotal;
								mOpenTimes++;

								Type2OrderExist = true;
								openThostOrder.mStoplossPoint = mStrategyParams.SecondStoplossPoint;
								ReqOpenOrderInsert(&openThostOrder);
							}
						}

						map<int, int>::iterator iter;
						AcquireSRWLockExclusive(&g_srwLockOrderId);
						iter = OrderIdToShmIndex.find(pOrderTradeMsg->OrderSysId);
						if (iter != OrderIdToShmIndex.end()) {
							OrderIdToShmIndex.erase(iter);
						}
						ReleaseSRWLockExclusive(&g_srwLockOrderId);

						CloseOrderList.erase(closeorder_it);
						break;
					}
					else {
						if (closeorder_it->VolumeTotal == 0) {
							//第二单止盈成交,继续报第二个开仓单到交易所
							bool mType2OrderExist = false;
							//Looping Opened Order to Check Type1Order
							std::list<MyOpenOrderType>::iterator order_itr;
							for (order_itr = OpenOrderList.begin(); order_itr != OpenOrderList.end(); ++order_itr) {
								if (order_itr->MOrderType == 2 && order_itr->VolumeTotal > 0) {
									mType2OrderExist = true;
								}
							}//end for loop

							std::list<MyCloseOrderType>::iterator corder_itr;
							for (corder_itr = CloseOrderList.begin(); corder_itr != CloseOrderList.end(); ++corder_itr) {
								if (corder_itr->MOrderType == 2 && corder_itr->VolumeTotal > 0) {
									mType2OrderExist = true;
								}
							}//end for loop

							if (!timeRuleForStop(nHour, nMin, nSec) && closeorder_it->MOrderType == 2 && closeorder_it->IsClosePofitOrder && mOpenTimes < mStrategyParams.LoopTimes
								&& mOpenPrice>0.00001
								&& (OnHandPositionCount + mStrategyParams.OpenVol2) <= MaxOnHandPositionCount
								&& (TotalOnHandPosition + mStrategyParams.OpenVol2) <= MaxTotalOnHandPosition
								&& !mType2OrderExist
								) {
								if (closeorder_it->Direction == MORDER_SELL) {
									char line[200];
									sprintf(line, "%s,Open Type 2 Order,Buy,COrderId=%d", mStrategyAndInstance.c_str(), closeorder_it->OrderId);
									WriteMsgToLogList(line);

									MyOpenOrderType openThostOrder;
									iNextOrderRef++;
									openThostOrder.OrderLocalRetReqID = 0;
									openThostOrder.OrderId = -1;
									openThostOrder.LimitPrice = mOpenPrice - mStrategyParams.SecondOpenDistPoint * m_dOneTick;
									openThostOrder.OrigSubmitPrice = openThostOrder.LimitPrice;
									openThostOrder.Direction = MORDER_BUY;
									openThostOrder.Offset = MORDER_OPEN;
									openThostOrder.VolumeTotal = mStrategyParams.OpenVol2;
									openThostOrder.VolumeTotalOriginal = openThostOrder.VolumeTotal;
									openThostOrder.VolumeTraded = 0;
									openThostOrder.ProfitPrice = openThostOrder.LimitPrice + mStrategyParams.SecondProfitPoint * m_dOneTick;
									openThostOrder.OpenOrderCanbeCanceled = true;
									openThostOrder.MOrderType = 2;
									openThostOrder.maxProfit = 0.0;

									OnHandPositionCount += openThostOrder.VolumeTotal;
									TotalOnHandPosition += openThostOrder.VolumeTotal;
									mOpenTimes++;

									Type2OrderExist = true;
									openThostOrder.mStoplossPoint = mStrategyParams.SecondStoplossPoint;
									ReqOpenOrderInsert(&openThostOrder);
								}
								else if (closeorder_it->Direction == MORDER_BUY) {
									char line[200];
									sprintf(line, "%s,Open Type 2 Order,Sell,COrderId=%d", mStrategyAndInstance.c_str(), closeorder_it->OrderId);
									WriteMsgToLogList(line);

									MyOpenOrderType openThostOrder;
									iNextOrderRef++;
									openThostOrder.OrderLocalRetReqID = 0;
									openThostOrder.OrderId = -1;
									openThostOrder.LimitPrice = mOpenPrice + mStrategyParams.SecondOpenDistPoint * m_dOneTick;
									openThostOrder.OrigSubmitPrice = openThostOrder.LimitPrice;
									openThostOrder.Direction = MORDER_SELL;
									openThostOrder.Offset = MORDER_OPEN;
									openThostOrder.VolumeTotal = mStrategyParams.OpenVol2;
									openThostOrder.VolumeTotalOriginal = openThostOrder.VolumeTotal;
									openThostOrder.VolumeTraded = 0;
									openThostOrder.ProfitPrice = openThostOrder.LimitPrice - mStrategyParams.SecondProfitPoint * m_dOneTick;
									openThostOrder.OpenOrderCanbeCanceled = true;
									openThostOrder.MOrderType = 2;
									openThostOrder.maxProfit = 0.0;

									OnHandPositionCount += openThostOrder.VolumeTotal;
									TotalOnHandPosition += openThostOrder.VolumeTotal;
									mOpenTimes++;

									Type2OrderExist = true;
									openThostOrder.mStoplossPoint = mStrategyParams.SecondStoplossPoint;
									ReqOpenOrderInsert(&openThostOrder);
								}
							}

							map<int, int>::iterator iter;
							AcquireSRWLockExclusive(&g_srwLockOrderId);
							iter = OrderIdToShmIndex.find(pOrderTradeMsg->OrderSysId);
							if (iter != OrderIdToShmIndex.end()) {
								OrderIdToShmIndex.erase(iter);
							}
							ReleaseSRWLockExclusive(&g_srwLockOrderId);

							CloseOrderList.erase(closeorder_it);
							break;
						}
					}
				} //End if closeorder_it->OrderSysId==thisTradeRef
			}//End While for Close Order List
		}
		//循环平仓队列,若队列中没有第一单的止盈单且第一单未止损平仓,则撤掉第二单的开仓单

		std::list<MyCloseOrderType>::iterator closeorder_itr1;
		bool find1 = false;
		for (closeorder_itr1 = CloseOrderList.begin(); closeorder_itr1 != CloseOrderList.end(); closeorder_itr1++) {
			if (closeorder_itr1->MOrderType == 1 && closeorder_itr1->IsClosePofitOrder) {
				find1 = true; break;
			}
		}
		if (!find1 && !m_bType1StoplossClose) {
			std::list<MyOpenOrderType>::iterator openorder_it1;
			for (openorder_it1 = OpenOrderList.begin(); openorder_it1 != OpenOrderList.end(); openorder_it1++) {
				if (openorder_it1->OpenOrderCanbeCanceled && openorder_it1->MOrderType == 2) {
					ReqOrderDelete(openorder_it1->OrderId, openorder_it1->OrderLocalRef, openorder_it1->FrontID, openorder_it1->SessionID);
					openorder_it1->OpenOrderCanbeCanceled = false;
				}
			}
		}

		//End 撤掉第二单的开仓单

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
			string strtimepart = strfulltradetime.substr(strfulltradetime.find_first_of(" ") + 1, strfulltradetime.length() - strfulltradetime.find_first_of(" ") - 1);
			//nYear=atoi(strdatepart.c_str())/10000;
			//nMonth=(atoi(strdatepart.c_str())%10000)/100;
			//nDate=(atoi(strdatepart.c_str())%10000)%100;
			//sscanf_s(ctimepart, "%d:%d:%d",&nYear, &nMonth, &nDate,&nHour,&nMin,&nSec);

			//strcpy(trade.tradingday,strdatepart.c_str());
			//strcpy(trade.CodeName,InstCodeName);
			//trade.tradeprice=pOrderTradeMsg->Price;
			//trade.submitprice=submitprice;
			//if(tradedirection==MORDER_BUY){
			//	trade.qty=pOrderTradeMsg->Volume;
			//}else trade.qty=-pOrderTradeMsg->Volume;
			//trade.fee=pOrderTradeMsg->MatchFee;

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

			DisplayTradeOnScreen(pOrderTradeMsg, tradedirection, openorclose, closeprofitornot);

			FlushStrategyInfoToFile();
		}
	}
}

void StrategyOpenPriceOpening::OnRspOrderInsert(ShmRspOrderInsert* pRspOrderInsert)
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

//datadate format: yyyymmdd , datatime formate hh24:mi:ss
//true - can open new order , false - shouldnot open new order
bool StrategyOpenPriceOpening::timeRuleForOpen(char datadate[10], char datatime[10]) {
	//int nYear, nMonth, nDate;
	//nYear=atoi(datadate)/10000;
	//nMonth=(atoi(datadate)%10000)/100;
	//nDate=(atoi(datadate)%10000)%100;
	//int nHour,nMin,nSec;
	//sscanf(datatime, "%d:%d:%d",&nHour, &nMin, &nSec);
	//CTime tm(nYear, nMonth, nDate,nHour,nMin,nSec);
	//time_t curTime=tm.GetTime();

	//int nTime=mHour*3600+mMin*60+mSec;
	//if(nTime>=2*3600+30*60&&nTime<=5*3600) return true;
	//return false;
	//
	//if(curTime>tmt_StartTime/*&&curTime<tmt_EndTime*/) return true;
	//return false;
	// 00:00:00~02:30:00 || 6:00:00

	int mHour, mMin, mSec;
	sscanf_s(datatime, "%d:%d:%d", &mHour, &mMin, &mSec);

	int mOpenHour, mOpenMin, mOpenSec;
	sscanf_s(mStrategyParams.OpenTime, "%d:%d:%d", &mOpenHour, &mOpenMin, &mOpenSec);

	int nTime = mHour * 3600 + mMin * 60 + mSec;
	if (nTime >= 0 && nTime < 2 * 3600 + 30 * 60 || nTime >= mOpenHour * 3600) return true;
	return false;
}

//datadate format: yyyymmdd , datatime formate hh24:mi:ss
//true - must close current order , false - no need to close current order
bool StrategyOpenPriceOpening::timeRuleForClose(char datadate[10], char datatime[10]) {
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

void StrategyOpenPriceOpening::ReqOpenOrderInsert(MyOpenOrderType* pOpenOrder) {
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

void StrategyOpenPriceOpening::ReqCloseOrderInsert(MyCloseOrderType* pCloseOrder, char OpenOrderTime[21]) {
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

void StrategyOpenPriceOpening::ReqOrderDelete(int pOrderId, int pOrderLocalRef, int pFrontID, int pSessionID)
{
	if (ThostTraderAPI || SgitTraderAPI) {
		if (ThostTraderAPI)pThostTraderSpi->ReqOrderDeletePerOrderLocalRef(pOrderLocalRef, m_InstID, m_ExchangeID, pFrontID, pSessionID);
		else pSgitTraderSpi->ReqOrderDeletePerOrderLocalRef(pOrderLocalRef, m_InstID, pFrontID, pSessionID);
	}
	else {
		pEsunTraderSpi->ReqOrderDelete(pOrderId);
	}

	char line[200];
	sprintf(line, "ReqOrderDelete,%d", pOrderId);
	WriteMsgToLogList(line);
}

void StrategyOpenPriceOpening::WriteMsgToLogList(char logline[200])
{
	Message logMsg;
	logMsg.type = STRATEGY_LOG;
	logMsg.AddData(logline, 0, sizeof(char) * 200);
	LogMessageList.AddTail(logMsg);
	ReleaseSemaphore(logSemaphore, 1, NULL);
}

void StrategyOpenPriceOpening::OnDisplayLocalCloseOrderList()
{
	char log[200];
	sprintf(log, "%s,策略开盘价=%.5f,开盘时间:%s", mStrategyAndInstance.c_str(), mOpenPrice, mStrategyParams.OpenTime);
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
				lcposition.OpenOrderTradePrice = closeorder_it->OpenOrderTradePrice;
				if (closeorder_it->MOrderType == 1) {
					lcposition.LimitPrice = closeorder_it->OpenOrderTradePrice + mStrategyParams.FirstStoplossPoint * m_dOneTick;
				}
				else lcposition.LimitPrice = closeorder_it->OpenOrderTradePrice + mStrategyParams.SecondStoplossPoint * m_dOneTick;
			}
			else if (closeorder_it->Direction == MORDER_SELL) {
				lcposition.OpenOrderTradePrice = closeorder_it->OpenOrderTradePrice;
				if (closeorder_it->MOrderType == 1) {
					lcposition.LimitPrice = closeorder_it->OpenOrderTradePrice - mStrategyParams.FirstStoplossPoint * m_dOneTick;
				}
				else lcposition.LimitPrice = closeorder_it->OpenOrderTradePrice - mStrategyParams.SecondStoplossPoint * m_dOneTick;
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

void StrategyOpenPriceOpening::AddtoTipMsgListBox(char msgline[200])
{
	WaitForSingleObject(MainScreenFreshSem, INFINITE);
	CString str(msgline);
	pPubMsg->AddString(str);
	ReleaseSemaphore(MainScreenFreshSem, 1, NULL);
}

void StrategyOpenPriceOpening::DisplayTradeOnScreen(OrderTradeMsg* pOrderTradeMsg, int mDirection, int mOpenOrClose, int mCloseProfitOrNot)
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

void StrategyOpenPriceOpening::RecoverInstance(char cfgFileName[500])
{
	FILE* fptr = fopen(cfgFileName, "rb");
	fseek(fptr, 0, SEEK_SET);
	fread(&header, sizeof(mSerializeHeader), 1, fptr);
	int OpenOrderCount = header.OpenOrderCount;
	int CloseOrderCount = header.CloseOrderCount;
	memcpy(&mOpenPrice, header.SpecificArea, sizeof(double));
	memcpy(&mOpenTimes, header.SpecificArea + sizeof(double), sizeof(int));
	//	strcpy(header.StartTime,mStrategyParams.StartTime);
	//	strcpy(header.EndTime,mStrategyParams.EndTime);

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

	char log[200];
	sprintf(log, "%s,OpenPriceOpening,重启恢复,当前时间=%s,原始开盘价=%.5f", mStrategyAndInstance.c_str(), c_str_mCurrTime, mOpenPrice);
	CString str(log);
	pPubMsg->AddString(str);
	WriteMsgToLogList(log);
	free(c_str_mCurrTime);

	FlushStrategyInfoToFile();
}

void StrategyOpenPriceOpening::SetStrategyID(char strategyId[50])
{
	strcpy(mStrategyID, strategyId);
}

//extern map<string,string> MatchNoToStrategyNameForDisplay;

bool StrategyOpenPriceOpening::timeRuleForStop(int nHour, int nMin, int nSec) {
	int mCloseHour, mCloseMin, mCloseSec;
	sscanf_s(mStrategyParams.CloseTime, "%d:%d:%d", &mCloseHour, &mCloseMin, &mCloseSec);
	int nTime = nHour * 3600 + nMin * 60 + nSec;
	if (nTime >= 2 * 3600 + 30 * 60 && nTime <= mCloseHour * 3600) return true;
	return false;
}

bool StrategyOpenPriceOpening::timeRuleForCancelOpen(char datadate[10], char datatime[10]) {
	int mHour, mMin, mSec;
	sscanf_s(datatime, "%d:%d:%d", &mHour, &mMin, &mSec);

	int nTime = mHour * 3600 + mMin * 60 + mSec;
	if (nTime >= 2 * 3600 + 30 * 60 && nTime <= 5 * 3600) return true;
	return false;
}

bool StrategyOpenPriceOpening::timeRuleForCloseNew(char openTime[30], char datadate[10], char datatime[10]) {
	char tmpOpenTime[32] = { 0 };
	strcpy(tmpOpenTime, openTime);
	if (strlen(tmpOpenTime) >= 17) {
		char* date = tmpOpenTime;
		char* time = strstr(tmpOpenTime, " ");
		*(time++) = '\0';
		int nOrderDate = atoi(date);
		int nOrderHour, nOrderMin, nOrderSec;
		sscanf_s(time, "%d:%d:%d", &nOrderHour, &nOrderMin, &nOrderSec);
		int nOpenHour, nOpenMin, nOpenSec;
		sscanf_s(mStrategyParams.OpenTime, "%d:%d:%d", &nOpenHour, &nOpenMin, &nOpenSec);
		int nChopHour, nChopMin, nChopSec;
		sscanf_s(mStrategyParams.ChopTime, "%d:%d:%d", &nChopHour, &nChopMin, &nChopSec);
		int nHour, nMin, nSec;
		sscanf_s(datatime, "%d:%d:%d", &nHour, &nMin, &nSec);			// 12点砍仓
		if (nHour == nChopHour && nMin >= nChopMin) {
			// 先比较订单日期,再比较订单时间确定是否是砍仓订单
			if (nOrderDate < atoi(datadate))
				return true;
			else {
				if (nOrderHour < nOpenHour) return true;
			}
		}
	}
	return false;
}