#include "StdAfx.h"
#include "StrategySpeedBreak.h"
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

int StrategySpeedBreak::MaxOnHandPositionCount = 0;
int StrategySpeedBreak::OnHandPositionCount = 0;

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
extern char CTPTradingDay[];
extern char SgitTradingDay[];
extern SRWLOCK g_srwLockOrderLocalRef;
extern map<int, int> OrderLocalRefToShmIndex;

StrategySpeedBreak::StrategySpeedBreak(char InstrumentID[30]) :m_Price(0)
{
	//	strcpy(m_InstID,InstrumentID);
	CTime mCurrTime = CTime::GetCurrentTime();
	//全局变量

	mTickCount = 0;
	m_dOneTick = 0.01;

	int m_dallocBarNum = 10000;
	mBarFile = (BarRateInfo*)malloc(sizeof(BarRateInfo) * m_dallocBarNum);
	mBarIndex = 0;
	mBarSize = 60;

	lastmBarIndex = 0;
	mTodayTickCount = 0;

	MovingBuffer = (double*)malloc(sizeof(double) * m_dallocBarNum);
	UpperBuffer = (double*)malloc(sizeof(double) * m_dallocBarNum);
	LowerBuffer = (double*)malloc(sizeof(double) * m_dallocBarNum);
	mCloseBuffer = (double*)malloc(sizeof(double) * m_dallocBarNum);

	m_bIsRunning = true;
	m_bCrossTradingDay = false;
	m_bType1StoplossClose = false;

	memset(&mStrategyParams, 0, sizeof(mParasType));

	strcpy(mStrategyName, "SpeedBreak");
	strcpy(mInstanceName, "");
	mStrategyAndInstance = mStrategyName;

	MarketOpenExecuted = false;
	MarketCloseExecuted = false;

	mOpenPrice = 0;
	InstanceOnHandPositionCount = 0;
	WriteMsgToLogList("Strategy Init..");
}

void StrategySpeedBreak::CloseMap()
{
	//删除对应的cfg文件
	UnmapViewOfFile(StrategyfilePoint);
	CloseHandle(StrategyInfoFileMap);
	/*
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
	*/
}

StrategySpeedBreak::~StrategySpeedBreak(void)
{
	free(mBarFile);

	free(MovingBuffer);
	free(UpperBuffer);
	free(LowerBuffer);
	free(mCloseBuffer);

	fclose(fmddata);
	fclose(findicator);
}

int StrategySpeedBreak::GetShmindex()
{
	return shmindex;
}

void StrategySpeedBreak::SetShmindex(int xshmindex)
{
	shmindex = xshmindex;
}

wstring StrategySpeedBreak::s2ws(const string& s)
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

int StrategySpeedBreak::CreateStrategyMapOfView() {
	CString strPathFile;
	::GetModuleFileName(NULL, strPathFile.GetBuffer(MAX_PATH), MAX_PATH);
	strPathFile.ReleaseBuffer();
	strPathFile = strPathFile.Left(strPathFile.ReverseFind(_T('\\')));
	strPathFile += "\\Strategies\\IndicatorOpenModel\\";
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

int StrategySpeedBreak::OpenStrategyMapOfView() {
	CString strPathFile;
	::GetModuleFileName(NULL, strPathFile.GetBuffer(MAX_PATH), MAX_PATH);
	strPathFile.ReleaseBuffer();
	strPathFile = strPathFile.Left(strPathFile.ReverseFind(_T('\\')));
	strPathFile += "\\Strategies\\IndicatorOpenModel\\";
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

void StrategySpeedBreak::SetInstanceName(char xInstanceName[50])
{
	strcpy(mInstanceName, xInstanceName);
	mStrategyAndInstance = mStrategyID;
	mStrategyAndInstance.append("_");
	mStrategyAndInstance.append(mInstanceName);
}

void StrategySpeedBreak::GetInstanceName(char* xInstanceName) {
	strcpy(xInstanceName, mInstanceName);
}

void StrategySpeedBreak::SetParamValue(ParamNode node) {
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
	else if (strcmp(node.ParamName, "BreakHour") == 0) {
		mStrategyParams.BreakHour = atoi(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "BreakMinute") == 0) {
		mStrategyParams.BreakMinute = atoi(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "RangeSecond") == 0) {
		mStrategyParams.RangeSecond = atoi(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "RangeTick") == 0) {
		mStrategyParams.RangeTick = atoi(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "BackPercent") == 0) {
		mStrategyParams.BackPercent = atof(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "ProfitPoint") == 0) {
		mStrategyParams.ProfitPoint = atof(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "LossPoint") == 0) {
		mStrategyParams.LossPoint = atof(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "ProfitCount") == 0) {
		mStrategyParams.ProfitCount = atoi(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "LossCount") == 0) {
		mStrategyParams.LossCount = atoi(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "OpenBuyAllow") == 0) {
		if (atoi(node.ParamValue) >= 1)mStrategyParams.OpenBuyAllow = true;
		else mStrategyParams.OpenBuyAllow = false;
	}
	else if (strcmp(node.ParamName, "OpenSellAllow") == 0) {
		if (atoi(node.ParamValue) >= 1)mStrategyParams.OpenSellAllow = true;
		else mStrategyParams.OpenSellAllow = false;
	}
	else if (strcmp(node.ParamName, "HoldAllow") == 0) {
		if (atoi(node.ParamValue) >= 1)mStrategyParams.HoldAllow = true;
		else mStrategyParams.HoldAllow = false;
	}
	else if (strcmp(node.ParamName, "OpenVol") == 0) {
		mStrategyParams.OpenVol = atoi(node.ParamValue);
	}
	else if (strcmp(node.ParamName, "StartTime") == 0) {
		strcpy(mStrategyParams.StartTime, node.ParamValue);
	}
	else if (strcmp(node.ParamName, "EndTime") == 0) {
		strcpy(mStrategyParams.EndTime, node.ParamValue);
	}
	else if (strcmp(node.ParamName, "StartTimeNight") == 0) {
		strcpy(mStrategyParams.StartTimeNight, node.ParamValue);
	}
	else if (strcmp(node.ParamName, "EndTimeNight") == 0) {
		strcpy(mStrategyParams.EndTimeNight, node.ParamValue);

		//}else if(strcmp(node.ParamName,"LoopTimes")==0){
		//	mStrategyParams.LoopTimes=atoi(node.ParamValue);
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

void StrategySpeedBreak::InitVariables() {
	int nYearS, nMonthS, nDateS, nHourS, nMinS, nSecS;
	int nYearE, nMonthE, nDateE, nHourE, nMinE, nSecE;

	sscanf(mStrategyParams.StartTime, "%d-%d-%d %d:%d:%d", &nYearS, &nMonthS, &nDateS, &nHourS, &nMinS, &nSecS);
	sscanf(mStrategyParams.EndTime, "%d-%d-%d %d:%d:%d", &nYearE, &nMonthE, &nDateE, &nHourE, &nMinE, &nSecE);

	CTime tms(nYearS, nMonthS, nDateS, nHourS, nMinS, nSecS);
	tmt_StartTime = tms.GetTime();

	CTime tme(nYearE, nMonthE, nDateE, nHourE, nMinE, nSecE);
	tmt_EndTime = tme.GetTime();

	mTickCount = 0;
	openBarIndex = 0;

	findicator = fopen("band.txt", "w+");

	initHstDataFromCSV();

	string strmdfilename(mStrategyName);
	char xBarSize[20];
	sprintf(xBarSize, "%d", mBarSize);
	strmdfilename.append("_");
	strmdfilename.append(InstCodeName);
	strmdfilename.append("_");
	strmdfilename.append(xBarSize);
	strmdfilename.append(".txt");
	fmddata = fopen(strmdfilename.c_str(), "a+");
}
void StrategySpeedBreak::ResetStrategy() {
}

void StrategySpeedBreak::ResetAction() {
	char line[200];
	sprintf(line, "%s,ResetAction", mStrategyAndInstance.c_str());
	WriteMsgToLogList(line);
	matchnomap.clear();
}
//检查消息队列，并将得到的消息转给相应的处理函数
/*
void StrategySpeedBreak::MessageProcess()
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
void StrategySpeedBreak::OnRtnDepthMarketData(TickInfo* pDepthMarketData)
{
	if (!timeRuleOK(pDepthMarketData->updatetime)) return;
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

	//CTime tm_m(nYear_m, nMonth_m, nDate_m,nHour_m,nMin_m,nSec_m);
	//curTickTime=tm_m.GetTime();
	strcpy(TickRealTimeDataDate, pDepthMarketData->datadate);

	//if(curTickTime>tmt_StartTime) {
	AddToRatesX(mTickCount, pDepthMarketData, mBarSize);
	mCloseBuffer[mBarIndex] = mBarFile[mBarIndex].close;
	mTickCount++;
	mTodayTickCount++;

	//		mIndicator.iBolingerBand(mCloseBuffer,mBarIndex,mBarIndex+1,mStrategyParams.BandsPeriod,mStrategyParams.BandsDeviations,MovingBuffer,UpperBuffer,LowerBuffer);

	if (lastmBarIndex != mBarIndex && mBarIndex > 0 && mTodayTickCount > 2
		&& strcmp(mBarFile[mBarIndex - 1].datatime, "15:00:00") != 0
		&& strcmp(mBarFile[mBarIndex - 1].datatime, "02:30:00") != 0
		&& strcmp(mBarFile[mBarIndex - 1].datatime, "08:59:00") != 0
		&& strcmp(mBarFile[mBarIndex - 1].datatime, "10:15:00") != 0
		&& strcmp(mBarFile[mBarIndex - 1].datatime, "20:59:00") != 0
		&& strcmp(mBarFile[mBarIndex - 1].datatime, "11:30:00") != 0) {
		//记录行情数据
		char dataline[200];
		sprintf(dataline, "%s,%s,%.4f,%.4f,%.4f,%.4f\n", mBarFile[mBarIndex - 1].datadate, mBarFile[mBarIndex - 1].datatime, mBarFile[mBarIndex - 1].open, mBarFile[mBarIndex - 1].high, mBarFile[mBarIndex - 1].low, mBarFile[mBarIndex - 1].close);
		fwrite(dataline, strlen(dataline), 1, fmddata);
		fflush(fmddata);

		sprintf(dataline, "%s,%s,%.4f,%.4f,%.4f\n", mBarFile[mBarIndex - 1].datadate, mBarFile[mBarIndex - 1].datatime, MovingBuffer[mBarIndex - 1], UpperBuffer[mBarIndex - 1], LowerBuffer[mBarIndex - 1]);
		fwrite(dataline, strlen(dataline), 1, findicator);
		fflush(findicator);
	}
	//}

	//if(mCloseBuffer[mBarIndex-1]>UpperBuffer[mBarIndex-1]&&(mBarFile[mBarIndex-1].high-mBarFile[mBarIndex-1].low+0.000001)>mStrategyParams.HLDist){
	//	EnterSellTrade=1;
	//}else if(mCloseBuffer[mBarIndex-1]<LowerBuffer[mBarIndex-1]&&(mBarFile[mBarIndex-1].high-mBarFile[mBarIndex-1].low+0.000001)>mStrategyParams.HLDist){
	//	EnterBuyTrade=1;
	//}

	//Processing Opened Order
	std::list<MyOpenOrderType>::iterator openorder_it;
	if (!OpenOrderList.empty()) {
		for (openorder_it = OpenOrderList.begin(); openorder_it != OpenOrderList.end(); ++openorder_it) {
			if (openorder_it->VolumeTotal != 0 && openorder_it->OpenOrderCanbeCanceled) {
				if ((mBarIndex - openorder_it->dwOpenSubmitStart) > 0
					) {
					char line[200];
					sprintf(line, "%s,Type 1 Cancel Open Order,Ref=%d,OrderId=%d", mStrategyAndInstance.c_str(), openorder_it->OrderLocalRef, openorder_it->OrderId);
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
		int CloseOrderCount = 0;
		for (closeorder_it = CloseOrderList.begin(); closeorder_it != CloseOrderList.end();) {
			std::list<MyCloseOrderType>::iterator iter_e = closeorder_it++;

			if (iter_e->Direction == MORDER_SELL &&
				(iter_e->OrderStatus == MORDER_PART_TRADED || iter_e->OrderStatus == MORDER_ACCEPTED || iter_e->OrderStatus == MORDER_QUEUED)) {
				//Closing the buy order
				iter_e->maxProfit = max(iter_e->maxProfit, m_Price - iter_e->OpenOrderTradePrice);
				/*
				if((iter_e->maxProfit+0.00001)>mStrategyParams.MAProfitPoint*m_dOneTick){
					iter_e->mStoplossPoint=floor(min(iter_e->mStoplossPoint*m_dOneTick,-iter_e->maxProfit*mStrategyParams.BackPerc)/m_dOneTick+0.00001);
				}else if((iter_e->maxProfit-0.00001)<mStrategyParams.MAProfitPoint*m_dOneTick){
					iter_e->mStoplossPoint=floor(min(iter_e->mStoplossPoint*m_dOneTick,mStrategyParams.StoplossPoint*m_dOneTick-iter_e->maxProfit)/m_dOneTick+0.00001);
				}
				*/
				if ((m_Sell1 + 0.00001) > (iter_e->OpenOrderTradePrice + mStrategyParams.ProfitPoint * m_dOneTick) && iter_e->IsClosePofitOrder
					&& timeRuleForClose(pDepthMarketData->datadate, pDepthMarketData->updatetime)) {
					char line[200];
					sprintf(line, "%s,Buy Order Profit Close,OrderId=%d", mStrategyAndInstance.c_str(), iter_e->OrderId);
					WriteMsgToLogList(line);

					MyCloseOrderType newThostOrder;
					iNextOrderRef++;
					newThostOrder.OrderLocalRetReqID = 0;
					newThostOrder.OpenOrderID = iter_e->OpenOrderID;
					strcpy(newThostOrder.OpenTime, iter_e->OpenTime);
					newThostOrder.IsStoplessOrder = true;
					newThostOrder.CanbeCanceled = true;
					newThostOrder.IsClosePofitOrder = false;
					newThostOrder.OpenOrderTradePrice = iter_e->OpenOrderTradePrice;
					newThostOrder.MOrderType = 1;
					newThostOrder.Direction = iter_e->Direction;
					newThostOrder.LimitPrice = iter_e->OpenOrderTradePrice + mStrategyParams.ProfitPoint * m_dOneTick;//以原先设定的止盈价进行报单
					newThostOrder.OrigSubmitPrice = newThostOrder.LimitPrice;
					newThostOrder.Offset = iter_e->Offset;
					newThostOrder.VolumeTotalOriginal = iter_e->VolumeTotal;
					newThostOrder.VolumeTotal = newThostOrder.VolumeTotalOriginal;
					newThostOrder.VolumeTraded = 0;

					iter_e->NextCloseOrderPrice = m_Buy1;
					ReqCloseOrderInsert(&newThostOrder, newThostOrder.OpenTime);
					CloseOrderList.erase(iter_e);
					CloseOrderCount++;
					if (CloseOrderCount >= 6) break;
				}
				else if (iter_e->IsClosePofitOrder && (iter_e->OpenOrderTradePrice - m_Price) >= (iter_e->mStoplossPoint * m_dOneTick - 0.00001)
					) {
					char line[200];
					sprintf(line, "%s,Buy Order Stoploss Close,%.5f,OrderId=%d", mStrategyAndInstance.c_str(), m_Price, iter_e->OrderId);
					WriteMsgToLogList(line);

					MyCloseOrderType newThostOrder;
					iNextOrderRef++;
					newThostOrder.OrderLocalRetReqID = 0;
					newThostOrder.OpenOrderID = iter_e->OpenOrderID;
					strcpy(newThostOrder.OpenTime, iter_e->OpenTime);
					newThostOrder.IsStoplessOrder = true;
					newThostOrder.CanbeCanceled = true;
					newThostOrder.IsClosePofitOrder = false;
					newThostOrder.OpenOrderTradePrice = iter_e->OpenOrderTradePrice;
					newThostOrder.MOrderType = iter_e->MOrderType;
					newThostOrder.Direction = iter_e->Direction;
					newThostOrder.LimitPrice = iter_e->OpenOrderTradePrice - iter_e->mStoplossPoint * m_dOneTick;//以止损点进行报止损单
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
					sprintf(line, "%s,Buy Order Close ReSubmit,OrderId=%d", mStrategyAndInstance.c_str(), iter_e->OrderId);
					WriteMsgToLogList(line);

					iter_e->NextCloseOrderPrice = m_Buy1;
					ReqOrderDelete(iter_e->OrderId, iter_e->OrderLocalRef, iter_e->FrontID, iter_e->SessionID);
					iter_e->CanbeCanceled = false;
					CloseOrderCount++;
					if (CloseOrderCount >= 6) break;
				}
			}//End IF Closing buy order

			else if (iter_e->Direction == MORDER_BUY &&
				(iter_e->OrderStatus == MORDER_PART_TRADED || iter_e->OrderStatus == MORDER_ACCEPTED || iter_e->OrderStatus == MORDER_QUEUED)) {
				//onhandSellOrder++;
				//Closing sell order
				iter_e->maxProfit = max(iter_e->maxProfit, iter_e->OpenOrderTradePrice - m_Price);
				/*
				if((iter_e->maxProfit+0.00001)>mStrategyParams.MAProfitPoint*m_dOneTick){
					iter_e->mStoplossPoint=floor(min(iter_e->mStoplossPoint*m_dOneTick,-iter_e->maxProfit*mStrategyParams.BackPerc)/m_dOneTick+0.00001);
				}else if((iter_e->maxProfit-0.00001)<mStrategyParams.MAProfitPoint*m_dOneTick){
					iter_e->mStoplossPoint=floor(min(iter_e->mStoplossPoint*m_dOneTick,mStrategyParams.StoplossPoint*m_dOneTick-iter_e->maxProfit)/m_dOneTick+0.00001);
				}
				*/
				if ((m_Buy1 - 0.00001) < (iter_e->OpenOrderTradePrice - mStrategyParams.ProfitPoint * m_dOneTick) && iter_e->IsClosePofitOrder
					&& timeRuleForClose(pDepthMarketData->datadate, pDepthMarketData->updatetime)) {
					char line[200];
					sprintf(line, "%s,Sell Order Profit Close,OrderId=%d", mStrategyAndInstance.c_str(), iter_e->OrderId);
					WriteMsgToLogList(line);

					MyCloseOrderType newThostOrder;
					iNextOrderRef++;
					newThostOrder.OrderLocalRetReqID = 0;
					newThostOrder.OpenOrderID = iter_e->OpenOrderID;
					strcpy(newThostOrder.OpenTime, iter_e->OpenTime);
					newThostOrder.IsStoplessOrder = true;
					newThostOrder.CanbeCanceled = true;
					newThostOrder.IsClosePofitOrder = false;
					newThostOrder.OpenOrderTradePrice = iter_e->OpenOrderTradePrice;
					newThostOrder.MOrderType = 1;
					newThostOrder.Direction = iter_e->Direction;
					newThostOrder.LimitPrice = iter_e->OpenOrderTradePrice - mStrategyParams.ProfitPoint * m_dOneTick;//以原先设定的止盈价进行报单
					newThostOrder.OrigSubmitPrice = newThostOrder.LimitPrice;
					newThostOrder.Offset = iter_e->Offset;
					newThostOrder.VolumeTotalOriginal = iter_e->VolumeTotal;
					newThostOrder.VolumeTotal = newThostOrder.VolumeTotalOriginal;
					newThostOrder.VolumeTraded = 0;

					iter_e->NextCloseOrderPrice = m_Sell1;
					ReqCloseOrderInsert(&newThostOrder, newThostOrder.OpenTime);
					CloseOrderList.erase(iter_e);
					CloseOrderCount++;
					if (CloseOrderCount >= 6) break;
				}
				else if (iter_e->IsClosePofitOrder && (m_Price - iter_e->OpenOrderTradePrice) >= (iter_e->mStoplossPoint * m_dOneTick - 0.00001)
					) {
					char line[200];
					sprintf(line, "%s,Sell Order Stoploss Close,%.5f,OrderId=%d", mStrategyAndInstance.c_str(), m_Price, iter_e->OrderId);
					WriteMsgToLogList(line);

					MyCloseOrderType newThostOrder;
					iNextOrderRef++;
					newThostOrder.OrderLocalRetReqID = 0;
					newThostOrder.OpenOrderID = iter_e->OpenOrderID;
					strcpy(newThostOrder.OpenTime, iter_e->OpenTime);
					newThostOrder.IsStoplessOrder = true;
					newThostOrder.CanbeCanceled = true;
					newThostOrder.IsClosePofitOrder = false;
					newThostOrder.OpenOrderTradePrice = iter_e->OpenOrderTradePrice;
					newThostOrder.MOrderType = iter_e->MOrderType;
					newThostOrder.Direction = iter_e->Direction;
					newThostOrder.LimitPrice = iter_e->OpenOrderTradePrice + iter_e->mStoplossPoint * m_dOneTick;//以止损点进行报止损单
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
					sprintf(line, "%s,Sell Order Close ReSubmit,OrderId=%d", mStrategyAndInstance.c_str(), iter_e->OrderId);
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
			//			&&InstanceOnHandPositionCount<mStrategyParams.LoopTimes
			//			&&mStrategyParams.OpenBuyAllow
			&& openBarIndex != mBarIndex
			&& (OnHandPositionCount + mStrategyParams.OpenVol) <= MaxOnHandPositionCount && (TotalOnHandPosition + mStrategyParams.OpenVol) <= MaxTotalOnHandPosition) {
			//Open Buy Order
			MyOpenOrderType openThostOrder;
			iNextOrderRef++;
			openThostOrder.OrderLocalRetReqID = 0;
			openThostOrder.OrderId = -1;
			openThostOrder.LimitPrice = mCloseBuffer[mBarIndex - 1];
			openThostOrder.OrigSubmitPrice = openThostOrder.LimitPrice;
			openThostOrder.Direction = MORDER_BUY;
			openThostOrder.Offset = MORDER_OPEN;
			openThostOrder.VolumeTotal = mStrategyParams.OpenVol;
			openThostOrder.VolumeTotalOriginal = openThostOrder.VolumeTotal;
			openThostOrder.VolumeTraded = 0;
			openThostOrder.ProfitPrice = openThostOrder.LimitPrice + mStrategyParams.ProfitPoint * m_dOneTick;
			openThostOrder.OpenOrderCanbeCanceled = true;
			openThostOrder.MOrderType = 1;
			openThostOrder.dwOpenSubmitStart = mBarIndex;

			openThostOrder.maxProfit = 0.0;

			//				openThostOrder.mStoplossPoint=mStrategyParams.StoplossPoint;
			ReqOpenOrderInsert(&openThostOrder);

			OnHandPositionCount += openThostOrder.VolumeTotal;
			InstanceOnHandPositionCount += openThostOrder.VolumeTotal;
			TotalOnHandPosition += openThostOrder.VolumeTotal;

			BuyOrderSubmitted = true;
			m_bType1StoplossClose = false;
			openBarIndex = mBarIndex;
			char log[200];
			sprintf(log, "%s,开仓,买=%.5f,时间=%s,手数=%d", mStrategyAndInstance.c_str(), openThostOrder.LimitPrice, pDepthMarketData->updatetime, openThostOrder.VolumeTotal);
			AddtoTipMsgListBox(log);
			WriteMsgToLogList(log);
		}

		if (EnterSellTrade == 1
			//			&&InstanceOnHandPositionCount<mStrategyParams.LoopTimes
			//			&&mStrategyParams.OpenSellAllow
			&& openBarIndex != mBarIndex
			&& (OnHandPositionCount + mStrategyParams.OpenVol) <= MaxOnHandPositionCount && (TotalOnHandPosition + mStrategyParams.OpenVol) <= MaxTotalOnHandPosition) {
			//Open Sell Order
			MyOpenOrderType openThostOrder;
			iNextOrderRef++;
			openThostOrder.OrderLocalRetReqID = 0;
			openThostOrder.OrderId = -1;
			openThostOrder.LimitPrice = mCloseBuffer[mBarIndex - 1];
			openThostOrder.OrigSubmitPrice = openThostOrder.LimitPrice;
			openThostOrder.Direction = MORDER_SELL;
			openThostOrder.Offset = MORDER_OPEN;
			openThostOrder.VolumeTotal = mStrategyParams.OpenVol;
			openThostOrder.VolumeTotalOriginal = openThostOrder.VolumeTotal;
			openThostOrder.VolumeTraded = 0;
			openThostOrder.ProfitPrice = openThostOrder.LimitPrice - mStrategyParams.ProfitPoint * m_dOneTick;
			openThostOrder.OpenOrderCanbeCanceled = true;
			openThostOrder.MOrderType = 1;
			openThostOrder.maxProfit = 0.0;
			openThostOrder.dwOpenSubmitStart = mBarIndex;

			//				openThostOrder.mStoplossPoint=mStrategyParams.StoplossPoint;
			ReqOpenOrderInsert(&openThostOrder);

			OnHandPositionCount += openThostOrder.VolumeTotal;
			InstanceOnHandPositionCount += openThostOrder.VolumeTotal;
			TotalOnHandPosition += openThostOrder.VolumeTotal;

			SellOrderSubmitted = true;
			m_bType1StoplossClose = false;

			openBarIndex = mBarIndex;

			char log[200];
			sprintf(log, "%s,开仓,卖=%.5f,时间=%s,手数=%d", mStrategyAndInstance.c_str(), openThostOrder.LimitPrice, pDepthMarketData->updatetime, openThostOrder.VolumeTotal);
			AddtoTipMsgListBox(log);
			WriteMsgToLogList(log);
		}
	}

	lastmBarIndex = mBarIndex;
	FlushStrategyInfoToFile();
}

void StrategySpeedBreak::OnRtnOrder(OrderTradeMsg* pOrderTradeMsg)
{
	int thisOrderId = (pOrderTradeMsg->OrderSysId);
	//UpdateOrderIdToOrderList(thisOrderId);
	char line[200];
	sprintf(line, "%s,OnRtnOrder,Ref=%d,OrderId=%d,Status=%c,VolRemain=%d", mStrategyAndInstance.c_str(), pOrderTradeMsg->OrderLocalRef, thisOrderId, pOrderTradeMsg->OrderStatus, pOrderTradeMsg->VolumeTotal);
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
					OpenOrderList.erase(openorder_it);
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
				if (iter_ec->OrderLocalRef == pOrderTradeMsg->OrderLocalRef && iter_ec->OrderLocalRef > 0)iter_ec->OrderId = thisOrderId;
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

				iter_ec->OrderStatus = pOrderTradeMsg->OrderStatus;
				if (pOrderTradeMsg->OrderStatus == MORDER_PART_CANCELLED || pOrderTradeMsg->OrderStatus == MORDER_CANCELLED)
				{
					if (iter_ec->IsStoplessOrder) {
						char linex[200];
						sprintf(linex, "%s,OnRtnOrder IsStoplessOrder,Ref=%d", mStrategyAndInstance.c_str(), pOrderTradeMsg->OrderLocalRef);
						WriteMsgToLogList(linex);

						//止损单被撤
						if (iter_ec->Direction == MORDER_SELL)
						{
							MyCloseOrderType newThostOrder;
							iNextOrderRef++;
							newThostOrder.OrderLocalRetReqID = 0;
							newThostOrder.OpenOrderID = iter_ec->OpenOrderID;
							strcpy(newThostOrder.OpenTime, iter_ec->OpenTime);
							newThostOrder.IsStoplessOrder = true;
							newThostOrder.MAStop = iter_ec->MAStop;
							newThostOrder.CanbeCanceled = true;
							newThostOrder.IsClosePofitOrder = false;
							newThostOrder.ManualStopPrice = iter_ec->ManualStopPrice;
							newThostOrder.OpenOrderTradePrice = iter_ec->OpenOrderTradePrice;
							newThostOrder.MOrderType = iter_ec->MOrderType;
							newThostOrder.Direction = iter_ec->Direction;
							newThostOrder.LimitPrice = m_Buy1;//iter_ec->NextCloseOrderPrice;//以原先设定的价格进行报单
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
							iNextOrderRef++;
							newThostOrder.OrderLocalRetReqID = 0;
							newThostOrder.OpenOrderID = iter_ec->OpenOrderID;
							strcpy(newThostOrder.OpenTime, iter_ec->OpenTime);
							newThostOrder.IsStoplessOrder = true;
							newThostOrder.MAStop = iter_ec->MAStop;
							newThostOrder.CanbeCanceled = true;
							newThostOrder.IsClosePofitOrder = false;
							newThostOrder.ManualStopPrice = iter_ec->ManualStopPrice;
							newThostOrder.OpenOrderTradePrice = iter_ec->OpenOrderTradePrice;
							newThostOrder.MOrderType = iter_ec->MOrderType;
							newThostOrder.Direction = iter_ec->Direction;
							newThostOrder.LimitPrice = m_Sell1;//iter_ec->NextCloseOrderPrice;//以原先设定的价格进行报单
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

	FlushStrategyInfoToFile();
}

void StrategySpeedBreak::FlushStrategyInfoToFile() {
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

void StrategySpeedBreak::OnRtnTrade(OrderTradeMsg* pOrderTradeMsg)
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
						MyCloseOrderType thostOrder;//虚拟的平仓止盈单,未实际提交到交易所,当触碰止盈价是提交止盈单
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
						thostOrder.OpenOrderID = openorder_it->OrderId;
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

						CloseOrderList.push_back(thostOrder);

						submitprice = openorder_it->OrigSubmitPrice;

						if (openorder_it->VolumeTotal == 0) {
							OpenOrderList.erase(openorder_it);
							break;
						}
					}
					else if (openorder_it->Direction == MORDER_SELL) {
						tradedirection = MORDER_SELL;
						MyCloseOrderType thostOrder;//虚拟的平仓止盈单,未实际提交到交易所,当触碰止盈价是提交止盈单
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
						thostOrder.OpenOrderID = openorder_it->OrderId;
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
						//strcpy(thostOrder.OpenTime,gEqualVolRatesX[gEqualVolRatesXIndex].datatime);
						CloseOrderList.push_back(thostOrder);

						submitprice = openorder_it->OrigSubmitPrice;
						if (openorder_it->VolumeTotal == 0) {
							OpenOrderList.erase(openorder_it);
							break;
						}
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
					openid = closeorder_it->OpenOrderID;
					openorclose = MORDER_CLOSE;
					submitprice = closeorder_it->OrigSubmitPrice;

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
						break;
					}
					else {
						closeorder_it->VolumeTraded = closeorder_it->VolumeTraded + pOrderTradeMsg->Volume;
						closeorder_it->VolumeTotal = closeorder_it->VolumeTotal - pOrderTradeMsg->Volume;
						if (closeorder_it->VolumeTotal == 0) {
							CloseOrderList.erase(closeorder_it);
							break;
						}
					}
				} //End if closeorder_it->OrderSysId==thisTradeRef
			}//End While for Close Order List
		}
		//循环平仓队列
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

			DisplayTradeOnScreen(pOrderTradeMsg, tradedirection, openorclose, closeprofitornot);

			FlushStrategyInfoToFile();
		}
	}
}

void StrategySpeedBreak::OnRspOrderInsert(ShmRspOrderInsert* pRspOrderInsert)
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
bool StrategySpeedBreak::timeRuleForOpen(char datadate[10], char datatime[10]) {
	//int nYear, nMonth, nDate;
	//nYear=atoi(datadate)/10000;
	//nMonth=(atoi(datadate)%10000)/100;
	//nDate=(atoi(datadate)%10000)%100;
	int nHour, nMin, nSec;
	sscanf(datatime, "%d:%d:%d", &nHour, &nMin, &nSec);
	//CTime tm(nYear, nMonth, nDate,nHour,nMin,nSec);
	//time_t curTime=tm.GetTime();

	if (nHour == 9) return true;
	if (nHour == 10 && (nMin < 15 || nMin>29)) return true;

	if (nHour == 11 && nMin < 29) return true;
	if (nHour == 13 && nMin > 29) return true;
	if (nHour == 14 && nMin < 59) return true;

	return false;
}

//datadate format: yyyymmdd , datatime formate hh24:mi:ss
//true - must close current order , false - no need to close current order
bool StrategySpeedBreak::timeRuleForClose(char datadate[10], char datatime[10]) {
	//int nYear, nMonth, nDate;
	//nYear=atoi(datadate)/10000;
	//nMonth=(atoi(datadate)%10000)/100;
	//nDate=(atoi(datadate)%10000)%100;
	int nHour, nMin, nSec;
	sscanf(datatime, "%d:%d:%d", &nHour, &nMin, &nSec);
	//CTime tm(nYear, nMonth, nDate,nHour,nMin,nSec);
	//time_t curTime=tm.GetTime();

	if (nHour == 9) return true;
	if (nHour == 10 && (nMin < 15 || nMin>29)) return true;
	if (nHour == 11 && nMin < 29) return true;
	if (nHour == 13 && nMin > 29) return true;
	if (nHour == 14 && nMin < 59) return true;

	return false;
}

void StrategySpeedBreak::ReqOpenOrderInsert(MyOpenOrderType* pOpenOrder) {
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

void StrategySpeedBreak::ReqCloseOrderInsert(MyCloseOrderType* pCloseOrder, char OpenOrderTime[21]) {
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

void StrategySpeedBreak::ReqOrderDelete(int pOrderId, int pOrderLocalRef, int pFrontID, int pSessionID)
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

void StrategySpeedBreak::WriteMsgToLogList(char logline[200])
{
	Message logMsg;
	logMsg.type = STRATEGY_LOG;
	logMsg.AddData(logline, 0, sizeof(char) * 200);
	LogMessageList.AddTail(logMsg);
	ReleaseSemaphore(logSemaphore, 1, NULL);
}

void StrategySpeedBreak::OnDisplayLocalCloseOrderList()
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
		lcposition.OpenOrderTradePrice = closeorder_it->LimitPrice;
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

	if (mBarIndex > 0) {
		char log[200];
		sprintf(log, "%s,%d,BandMA=%.2f,Upper=%.2f,Lower=%.2f", mStrategyAndInstance.c_str(), CloseOrderList.size(), MovingBuffer[mBarIndex], UpperBuffer[mBarIndex], LowerBuffer[mBarIndex]);
		CString str(log);
		pPubMsg->AddString(str);
	}
}

void StrategySpeedBreak::AddtoTipMsgListBox(char msgline[200])
{
	WaitForSingleObject(MainScreenFreshSem, INFINITE);
	CString str(msgline);
	pPubMsg->AddString(str);
	ReleaseSemaphore(MainScreenFreshSem, 1, NULL);
}

void StrategySpeedBreak::DisplayTradeOnScreen(OrderTradeMsg* pOrderTradeMsg, int mDirection, int mOpenOrClose, int mCloseProfitOrNot)
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

void StrategySpeedBreak::RecoverInstance(char cfgFileName[500])
{
	FILE* fptr = fopen(cfgFileName, "rb");
	fseek(fptr, 0, SEEK_SET);
	fread(&header, sizeof(mSerializeHeader), 1, fptr);
	int OpenOrderCount = header.OpenOrderCount;
	int CloseOrderCount = header.CloseOrderCount;
	memcpy(&mOpenPrice, header.SpecificArea, sizeof(double));
	memcpy(&InstanceOnHandPositionCount, header.SpecificArea + sizeof(double), sizeof(int));
	InstanceOnHandPositionCount = 0;
	//	strcpy(header.StartTime,mStrategyParams.StartTime);
	//	strcpy(header.EndTime,mStrategyParams.EndTime);

	MyOpenOrderType openOrder;
	for (int i = 0; i < OpenOrderCount; i++) {
		fread(&openOrder, sizeof(MyOpenOrderType), 1, fptr);
		OpenOrderList.push_back(openOrder);
		//OrderIdToShmIndex.insert(std::pair<int,int>(openOrder.OrderId,GetShmindex()));
		string sfinal(mStrategyAndInstance);
		sfinal.append("_open");
		OrderIdToStrategyNameForDisplay.insert(std::pair<int, string>(openOrder.OrderId, sfinal));
	}
	MyCloseOrderType closeOrder;
	for (int i = 0; i < CloseOrderCount; i++) {
		fread(&closeOrder, sizeof(MyCloseOrderType), 1, fptr);
		OnHandPositionCount += closeOrder.VolumeTotal;
		InstanceOnHandPositionCount += closeOrder.VolumeTotal;
		TotalOnHandPosition += closeOrder.VolumeTotal;
		CloseOrderList.push_back(closeOrder);
		//OrderIdToShmIndex.insert(std::pair<int,int>(closeOrder.OrderId,GetShmindex()));
		string sfinal(mStrategyAndInstance);
		sfinal.append("_close");
		OrderIdToStrategyNameForDisplay.insert(std::pair<int, string>(closeOrder.OrderId, sfinal));
	}
	fclose(fptr);
	mOpenPrice = 0.0;

	CTime mCurrTime = CTime::GetCurrentTime();

	CString str_mCurrTime = mCurrTime.Format("%X");
	int len = WideCharToMultiByte(CP_ACP, 0, str_mCurrTime, str_mCurrTime.GetLength(), NULL, 0, NULL, NULL);
	char* c_str_mCurrTime = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, str_mCurrTime, str_mCurrTime.GetLength(), c_str_mCurrTime, len, NULL, NULL);
	c_str_mCurrTime[len] = '\0';

	char log[200];
	sprintf(log, "%s,重启恢复,当前时间=%s,OnHand=%d", mStrategyAndInstance.c_str(), c_str_mCurrTime, InstanceOnHandPositionCount);
	CString str(log);
	pPubMsg->AddString(str);
	WriteMsgToLogList(log);
	free(c_str_mCurrTime);

	FlushStrategyInfoToFile();
}

void StrategySpeedBreak::AddToRatesX(int tickCount, TickInfo* curTick, int gRatesXSize) {
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
		//初始化柱体数据
		d_open = curTick->price;
		d_high = curTick->price;
		d_low = curTick->price;
		d_close = curTick->price;
		d_time = curTime / gRatesXSize;
		d_time = d_time * gRatesXSize;
		//update to MX Bar Array
		mBarIndex = 0;
	}
	else {
		nXYear = atoi(mBarFile[mBarIndex].datadate) / 10000;
		nXMonth = (atoi(mBarFile[mBarIndex].datadate) % 10000) / 100;
		nXDate = (atoi(mBarFile[mBarIndex].datadate) % 10000) % 100;
		sscanf_s(mBarFile[mBarIndex].datatime, "%d:%d:%d", &nXHour, &nXMin, &nXSec);
		CTime tmX(nXYear, nXMonth, nXDate, nXHour, nXMin, nXSec);
		d_time = tmX.GetTime();

		if (curTime >= (d_time + gRatesXSize)) {
			if (strcmp(curTick->datadate, mBarFile[mBarIndex].datadate) != 0) {
				//New Date
				d_time = curTime / gRatesXSize;
				d_time = d_time * gRatesXSize;
			}
			else {
				d_time = curTime / gRatesXSize;
				d_time = d_time * gRatesXSize;
			}

			//New Bar
			d_open = curTick->price;
			d_high = curTick->price;
			d_low = curTick->price;
			d_close = curTick->price;

			mBarIndex++;
		}
		else {
			if (curTick->price < mBarFile[mBarIndex].low) mBarFile[mBarIndex].low = curTick->price;
			if (curTick->price > mBarFile[mBarIndex].high)mBarFile[mBarIndex].high = curTick->price;
			mBarFile[mBarIndex].close = curTick->price;
			return;
		}
	}
	char timet[10];
	strftime(timet, sizeof(timet), "%Y%m%d", localtime(&d_time));
	strcpy_s(mBarFile[mBarIndex].datadate, 10, timet);
	strftime(timet, sizeof(timet), "%X", localtime(&d_time));
	strcpy_s(mBarFile[mBarIndex].datatime, 10, timet);
	mBarFile[mBarIndex].open = d_open;
	mBarFile[mBarIndex].high = d_high;
	mBarFile[mBarIndex].low = d_low;
	mBarFile[mBarIndex].close = d_close;
}

int StrategySpeedBreak::initHstDataFromCSV() {
	string strHstFileName(mStrategyName);
	char xBarSize[20];
	sprintf(xBarSize, "%d", mBarSize);
	strHstFileName.append("_");
	strHstFileName.append(InstCodeName);
	strHstFileName.append("_");
	strHstFileName.append(xBarSize);
	strHstFileName.append(".txt");

	FILE* askfp = fopen(strHstFileName.c_str(), "r");
	if (askfp == NULL) {
		char line[200];
		sprintf(line, "cannot open the csv hst file");
		WriteMsgToLogList(line);
		return -1;
	}
	char* askptr;
	char askachBuf[256] = { 0 };
	askptr = fgets(askachBuf, 256, askfp);

	BarRateInfo curBar;
	list<BarRateInfo> barlist;
	while (askptr != NULL) {
		char* column[6], * p = askachBuf;
		int i;
		for (i = 0; i < 6; i++) {
			column[i] = p;
			if ((p = strchr(p, ',')) == NULL)break;
			else *p++ = '\0';
		}
		//strcpy(mBarFile[mBarIndex].datadate,column[0]);
		//strcpy(mBarFile[mBarIndex].datatime,column[1]);
		//mBarFile[mBarIndex].open=atof(column[2]);
		//mBarFile[mBarIndex].high=atof(column[3]);
		//mBarFile[mBarIndex].low=atof(column[4]);
		//mBarFile[mBarIndex].close=atof(column[5]);
		//mBarIndex++;
		strcpy(curBar.datadate, column[0]);
		strcpy(curBar.datatime, column[1]);
		curBar.open = atof(column[2]);
		curBar.high = atof(column[3]);
		curBar.low = atof(column[4]);
		curBar.close = atof(column[5]);
		curBar.vol = 1; curBar.position = 0;

		barlist.push_back(curBar);
		if (barlist.size() > 5000)barlist.pop_front();

		askptr = fgets(askachBuf, 256, askfp);
	}
	fclose(askfp);

	mBarIndex = 0;
	int csvBarCount = 0;
	std::list<BarRateInfo>::iterator bar_itr;
	for (bar_itr = barlist.begin(); bar_itr != barlist.end(); bar_itr++) {
		memset(&curBar, 0, sizeof(BarRateInfo));
		strcpy(curBar.datadate, bar_itr->datadate);
		strcpy(curBar.datatime, bar_itr->datatime);
		curBar.open = bar_itr->open;
		curBar.high = bar_itr->high;
		curBar.low = bar_itr->low;
		curBar.close = bar_itr->close;
		curBar.vol = 1; curBar.position = 0;

		AddInToDesRatesX(&curBar, csvBarCount, mBarFile, &mBarIndex, mBarSize);
		mCloseBuffer[mBarIndex] = mBarFile[mBarIndex].close;

		csvBarCount++;
	}

	if (mBarSize == 0) {
		char line[200];
		sprintf(line, "mBarSize is zero, default to 1s.");
		WriteMsgToLogList(line);
		mBarSize = 1;
	}

	if (mBarIndex > 0) {
		mBarIndex = mBarIndex + 1;
		mTickCount = mBarSize * mBarIndex;
		mBarIndex = mBarIndex - 1;
	}

	if (mBarIndex <= 0) {
		char line[200];
		sprintf(line, "no Bar in csv file or mBarIndex=0.");
		WriteMsgToLogList(line);
		mBarIndex = 0;
		//return -1;
	}

	//	if(mBarIndex>mStrategyParams.BandsPeriod){
	//		mIndicator.iBolingerBand(mCloseBuffer,0,mBarIndex+1,mStrategyParams.BandsPeriod,mStrategyParams.BandsDeviations,MovingBuffer,UpperBuffer,LowerBuffer);
	//	}

	char dataline[200];
	for (int i = 0; i <= mBarIndex; i++) {
		sprintf(dataline, "%s,%s,%.4f,%.4f,%.4f\n", mBarFile[i].datadate, mBarFile[i].datatime, MovingBuffer[i], UpperBuffer[i], LowerBuffer[i]);
		fwrite(dataline, strlen(dataline), 1, findicator);
		fflush(findicator);
	}

	lastmBarIndex = mBarIndex;

	return 1;
}

void StrategySpeedBreak::AddInToDesRatesX(BarRateInfo* gCurrtRatesX, int gSourceRatesXIndex, BarRateInfo* gDesRatesX, int* gDesRatesXIndex, int gDesRatesXSize) {
	int nYearp, nMonthp, nDatep, nHourp, nMinp, nSecp;
	int nXYearp, nXMonthp, nXDatep, nXHourp, nXMinp, nXSecp;
	double dp_open, dp_high, dp_low, dp_close, dp_vol;
	double AskVolMA = 0, BidVolMA = 0;
	time_t dp_time;

	nYearp = atoi(gCurrtRatesX->datadate) / 10000;
	nMonthp = (atoi(gCurrtRatesX->datadate) % 10000) / 100;
	nDatep = (atoi(gCurrtRatesX->datadate) % 10000) % 100;
	sscanf_s(gCurrtRatesX->datatime, "%d:%d:%d", &nHourp, &nMinp, &nSecp);

	CTime tmp(nYearp, nMonthp, nDatep, nHourp, nMinp, nSecp);
	time_t curTimep = tmp.GetTime();
	if (gSourceRatesXIndex == 0) {
		//初始化柱体数据
		dp_open = gCurrtRatesX->open;
		dp_high = gCurrtRatesX->high;
		dp_low = gCurrtRatesX->low;
		dp_close = gCurrtRatesX->close;
		dp_vol = gCurrtRatesX->vol;
		dp_time = curTimep / gDesRatesXSize;
		dp_time = dp_time * gDesRatesXSize;
		*gDesRatesXIndex = 0;
	}
	else {
		nXYearp = atoi(gDesRatesX[*gDesRatesXIndex].datadate) / 10000;
		nXMonthp = (atoi(gDesRatesX[*gDesRatesXIndex].datadate) % 10000) / 100;
		nXDatep = (atoi(gDesRatesX[*gDesRatesXIndex].datadate) % 10000) % 100;
		sscanf_s(gDesRatesX[*gDesRatesXIndex].datatime, "%d:%d:%d", &nXHourp, &nXMinp, &nXSecp);
		CTime tmXp(nXYearp, nXMonthp, nXDatep, nXHourp, nXMinp, nXSecp);

		dp_time = tmXp.GetTime();

		if (curTimep >= (dp_time + gDesRatesXSize)) {
			//New Bar
			dp_open = gCurrtRatesX->open;
			dp_high = gCurrtRatesX->high;
			dp_low = gCurrtRatesX->low;
			dp_close = gCurrtRatesX->close;
			dp_vol = gCurrtRatesX->vol;
			dp_time = curTimep / gDesRatesXSize;
			dp_time = dp_time * gDesRatesXSize;

			*gDesRatesXIndex = (*gDesRatesXIndex) + 1;
		}
		else {
			if (gCurrtRatesX->low < gDesRatesX[*gDesRatesXIndex].low) gDesRatesX[*gDesRatesXIndex].low = gCurrtRatesX->low;
			if (gCurrtRatesX->high > gDesRatesX[*gDesRatesXIndex].high)gDesRatesX[*gDesRatesXIndex].high = gCurrtRatesX->high;
			gDesRatesX[*gDesRatesXIndex].close = gCurrtRatesX->close;
			gDesRatesX[*gDesRatesXIndex].vol += gCurrtRatesX->vol;
			return;
		}
	}
	char timetp[10];
	strftime(timetp, sizeof(timetp), "%Y%m%d", localtime(&dp_time));
	strcpy_s(gDesRatesX[*gDesRatesXIndex].datadate, 10, timetp);
	strftime(timetp, sizeof(timetp), "%X", localtime(&dp_time));
	strcpy_s(gDesRatesX[*gDesRatesXIndex].datatime, 10, timetp);
	gDesRatesX[*gDesRatesXIndex].open = dp_open;
	gDesRatesX[*gDesRatesXIndex].high = dp_high;
	gDesRatesX[*gDesRatesXIndex].low = dp_low;
	gDesRatesX[*gDesRatesXIndex].close = dp_close;
	gDesRatesX[*gDesRatesXIndex].vol = dp_vol;
}

bool StrategySpeedBreak::timeRuleOK(char datatime[10]) {
	//int nYear, nMonth, nDate;
	//nYear=atoi(datadate)/10000;
	//nMonth=(atoi(datadate)%10000)/100;
	//nDate=(atoi(datadate)%10000)%100;
	int nHour, nMin, nSec;
	sscanf(datatime, "%d:%d:%d", &nHour, &nMin, &nSec);
	//CTime tm(nYear, nMonth, nDate,nHour,nMin,nSec);
	//time_t curTime=tm.GetTime();

	if (nHour > 8 && nHour < 11) return true;
	if (nHour == 11 && nMin < 30) return true;
	if (nHour == 13 && nMin > 29) return true;
	if (nHour == 14) return true;
	if (nHour > 20) return true;
	if (nHour < 2) return true;

	return false;
}

void StrategySpeedBreak::SetStrategyID(char strategyId[50])
{
	strcpy(mStrategyID, strategyId);
}

//extern map<string,string> MatchNoToStrategyNameForDisplay;