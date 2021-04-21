#include "StdAfx.h"
#include "SgitMdSpi.h"
//#include <iostream>
#include "MyStruct.h"
#include "TickDataList.h"
#include "MessageList.h"

using namespace std;

#pragma warning(disable : 4996)

// USER_API参数
extern CListBox* pMdPubMsg;
extern TickDataList TickList;

char* SgitppInstrumentID[100];
int SgitiInstrumentID;

// 请求编号
int iSgitRequestID = 0;

extern HANDLE MdTickSem;
extern MessageList LogMessageList;
//extern HANDLE logSemaphore;
extern HANDLE logSemaphore;

CSgitMdSpi::CSgitMdSpi(fstech::CThostFtdcMdApi* xMdApi, char xBROKER_ID[20], char xINVESTOR_ID[20], char xPASSWORD[20])
{
	pMdApi = xMdApi;
	strcpy(mBROKER_ID, xBROKER_ID);
	strcpy(mINVESTOR_ID, xINVESTOR_ID);
	strcpy(mPASSWORD, xPASSWORD);
	//SgitppInstrumentID[0]=new char[20];
	//strcpy(SgitppInstrumentID[0],"Au(T+D)");
	//SgitiInstrumentID=1;
}

void CSgitMdSpi::OnRspError(fstech::CThostFtdcRspInfoField* pRspInfo,
	int nRequestID, bool bIsLast)
{
	//cerr << "--->>> "<< "OnRspError" << endl;
	CString str(pRspInfo->ErrorMsg);
	//pRspInfo->ErrorMsg
	char* s = new char[200];
	itoa(pRspInfo->ErrorID, s, 10);
	CString str1(s);
	pMdPubMsg->AddString(str);
	pMdPubMsg->AddString(str1);
	IsErrorRspInfo(pRspInfo);

	delete s;
}

void CSgitMdSpi::OnFrontDisconnected(int nReason)
{
	//cerr << "--->>> " << "OnFrontDisconnected" << endl;
	//cerr << "--->>> Reason = " << nReason << endl;
	CString str("md front disconnected");
	//
	pMdPubMsg->AddString(str);
}

void CSgitMdSpi::OnHeartBeatWarning(int nTimeLapse)
{
}

void CSgitMdSpi::OnFrontConnected()
{
	//cerr << "--->>> " << "OnFrontConnected" << endl;
	///用户登录请求

	CString str("与Sgit行情服务器建立连接成功");
	pMdPubMsg->AddString(str);

	ReqUserLogin();
}

void CSgitMdSpi::Release()
{
	pMdApi->Release();
	//cerr << "--->>> 发送用户登录请求: " << ((iResult == 0) ? "成功" : "失败") << endl;
	CString str("Sgit Release 行情服务器");
	pMdPubMsg->AddString(str);
}

void CSgitMdSpi::ReqUserLogin()
{
	fstech::CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy_s(req.BrokerID, mBROKER_ID);
	strcpy_s(req.UserID, mINVESTOR_ID);
	strcpy_s(req.Password, mPASSWORD);
	int iResult = pMdApi->ReqUserLogin(&req, ++iSgitRequestID);
	//cerr << "--->>> 发送用户登录请求: " << ((iResult == 0) ? "成功" : "失败") << endl;
	CString str("Sgit 登录行情服务器");
	pMdPubMsg->AddString(str);

	struct tm* ptTm;
	time_t nowtime;
	memset(&beginrun_date, 0, 10);
	time(&nowtime);
	ptTm = localtime(&nowtime);
	strftime(beginrun_date, 10, "%Y%m%d", ptTm);
}

void CSgitMdSpi::OnRspUserLogin(fstech::CThostFtdcRspUserLoginField* pRspUserLogin,
	fstech::CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		CString str("Sgit 登录行情服务器成功");
		pMdPubMsg->AddString(str);

		///获取当前交易日
		//cerr << "--->>> 获取当前交易日 = " << pUserApi->GetTradingDay() << endl;
		strcpy(tradingday, pMdApi->GetTradingDay());
		// 请求订阅行情
		SubscribeMarketData();

		char logline[200];
		sprintf(logline, "Sgit MD RspLogin Success.");
		Message logMsg;
		logMsg.type = MD_LOG;
		logMsg.AddData(logline, 0, sizeof(char) * 200);
		LogMessageList.AddTail(logMsg);
		ReleaseSemaphore(logSemaphore, 1, NULL);
	}
	else {
		if (IsErrorRspInfo(pRspInfo)) {
			char logline[200];
			sprintf(logline, "Sgit MD RspLogin Failed.");
			Message logMsg;
			logMsg.type = MD_LOG;
			logMsg.AddData(logline, 0, sizeof(char) * 200);
			LogMessageList.AddTail(logMsg);
			ReleaseSemaphore(logSemaphore, 1, NULL);
		}
	}
}

void CSgitMdSpi::SubscribeMarketData()
{
	int iResult = pMdApi->SubscribeMarketData(SgitppInstrumentID, SgitiInstrumentID);
	CString str("Sgit请求订阅行情数据");
	pMdPubMsg->AddString(str);
	//cerr << "--->>> 发送行情订阅请求: " << ((iResult == 0) ? "成功" : "失败") << endl;
}

void CSgitMdSpi::OnRspSubMarketData(fstech::CThostFtdcSpecificInstrumentField* pSpecificInstrument, fstech::CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	CString str("SgitOnRspSubMarketData");
	CString strInst(pSpecificInstrument->InstrumentID);

	if (IsErrorRspInfo(pRspInfo) == false) {
		str.Append(_T("-->>"));
		str.Append(strInst);
		str.Append(_T(" 成功"));
		pMdPubMsg->AddString(str);
	}
	//cerr << "OnRspSubMarketData" << endl;
}

void CSgitMdSpi::OnRspUnSubMarketData(fstech::CThostFtdcSpecificInstrumentField* pSpecificInstrument, fstech::CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	//cerr << "OnRspUnSubMarketData" << endl;
}

void CSgitMdSpi::OnRtnDepthMarketData(fstech::CThostFtdcDepthMarketDataField* pDepthMarketData)
{
	int nHour, nMin, nSec;
	scanf(pDepthMarketData->UpdateTime, "%d:%d:%d", &nHour, &nMin, &nSec);

	if ((nHour > 2 && nHour <= 8) || nHour > 15 && nHour < 20)return;

	TickInfo tickData;
	strcpy(tickData.ordername, pDepthMarketData->InstrumentID);
	//strcpy(tickData.TradingDay,beginrun_date);
	strcpy(tickData.updatetime, pDepthMarketData->UpdateTime);
	//tickData.UpdateMillisec=pDepthMarketData->UpdateMillisec/500*500;
	tickData.bid1 = pDepthMarketData->BidPrice1;
	//tickData.BidVolume1=pDepthMarketData->BidVolume1;
	tickData.ask1 = pDepthMarketData->AskPrice1;
	//tickData.AskVolume1=pDepthMarketData->AskVolume1;
	tickData.price = pDepthMarketData->LastPrice;
	//tickData.Volume=pDepthMarketData->Volume;
	TickList.AddTail(tickData);

	//	TRACE("%s,%.5f,%s\n",tickData.CodeName,tickData.price,tickData.updatetime);

	ReleaseSemaphore(MdTickSem, 1, NULL);
}

bool CSgitMdSpi::IsErrorRspInfo(fstech::CThostFtdcRspInfoField* pRspInfo)
{
	// 如果ErrorID != 0, 收到了错误的响应
	bool bResult = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	//if (bResult)
		//cerr << "--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << endl;
	if (bResult) {
		char logline[200];
		sprintf(logline, "ErrorID=%d,ErrorMsg=%s\n", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
		Message logMsg;
		logMsg.type = MD_LOG;
		logMsg.AddData(logline, 0, sizeof(char) * 200);
		LogMessageList.AddTail(logMsg);
		ReleaseSemaphore(logSemaphore, 1, NULL);
	}
	return bResult;
}

bool CSgitMdSpi::timeRuleForOpen(fstech::TThostFtdcTimeType datatime) {
	int nHour, nMin, nSec;

	sscanf_s(datatime, "%d:%d:%d", &nHour, &nMin, &nSec);

	if (nHour == 9 && nMin <= 14) return false;
	if (nHour == 11 && nMin == 29) return false;
	if (nHour == 15 && nMin >= 14) return false;

	return true;
}