#include "StdAfx.h"
#include "ThostMdSpi.h"
//#include <iostream>
#include ".\ThostTraderApi\ThostFtdcMdApi.h"
#include "MyStruct.h"
#include "TickDataList.h"
#include "MessageList.h"

using namespace std;

#pragma warning(disable : 4996)

// USER_API����
extern CListBox* pMdPubMsg;
extern TickDataList TickList;

char* ppInstrumentID[100];
int iInstrumentID;

// ������
int iThostRequestID = 0;

extern HANDLE MdTickSem;
extern MessageList LogMessageList;
extern HANDLE logSemaphore;

CThostMdSpi::CThostMdSpi(CThostFtdcMdApi* xMdApi, char xBROKER_ID[20], char xINVESTOR_ID[20], char xPASSWORD[20])
{
	pMdApi = xMdApi;
	strcpy(mBROKER_ID, xBROKER_ID);
	strcpy(mINVESTOR_ID, xINVESTOR_ID);
	strcpy(mPASSWORD, xPASSWORD);
}

void CThostMdSpi::OnRspError(CThostFtdcRspInfoField* pRspInfo,
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

void CThostMdSpi::OnFrontDisconnected(int nReason)
{
	//cerr << "--->>> " << "OnFrontDisconnected" << endl;
	//cerr << "--->>> Reason = " << nReason << endl;
	struct tm* ptTm;
	time_t nowtime;
	time(&nowtime);
	ptTm = localtime(&nowtime);

	CString str("md disconnected ");
	char curtime[20];
	strftime(curtime, 20, "%X", ptTm);
	CString cstime(curtime);
	str.Append(cstime);
	pMdPubMsg->AddString(str);
}

void CThostMdSpi::OnHeartBeatWarning(int nTimeLapse)
{
}

void CThostMdSpi::OnFrontConnected()
{
	//cerr << "--->>> " << "OnFrontConnected" << endl;
	///�û���¼����
	CString str("��CTP����������������ӳɹ�");
	pMdPubMsg->AddString(str);

	ReqUserLogin();
}

void CThostMdSpi::ReqUserLogin()
{
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy_s(req.BrokerID, mBROKER_ID);
	strcpy_s(req.UserID, mINVESTOR_ID);
	strcpy_s(req.Password, mPASSWORD);
	int iResult = pMdApi->ReqUserLogin(&req, ++iThostRequestID);
	//cerr << "--->>> �����û���¼����: " << ((iResult == 0) ? "�ɹ�" : "ʧ��") << endl;
	struct tm* ptTm;
	time_t nowtime;
	memset(&beginrun_date, 0, 10);
	time(&nowtime);
	ptTm = localtime(&nowtime);
	strftime(beginrun_date, 10, "%Y%m%d", ptTm);

	CString str("CTP ��¼��������� ");
	char curtime[20];
	strftime(curtime, 20, "%X", ptTm);
	CString cstime(curtime);
	str.Append(cstime);
	pMdPubMsg->AddString(str);
}

void CThostMdSpi::Release()
{
	pMdApi->Release();
	//cerr << "--->>> �����û���¼����: " << ((iResult == 0) ? "�ɹ�" : "ʧ��") << endl;
	struct tm* ptTm;
	time_t nowtime;
	time(&nowtime);
	ptTm = localtime(&nowtime);

	CString str("CTP Release ���������");
	char curtime[20];
	strftime(curtime, 20, "%X", ptTm);
	CString cstime(curtime);
	str.Append(cstime);
	pMdPubMsg->AddString(str);
}

void CThostMdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
	CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	if (bIsLast && !IsErrorRspInfo(pRspInfo))
	{
		CString str("CTP ��¼����������ɹ�");
		pMdPubMsg->AddString(str);

		///��ȡ��ǰ������
		//cerr << "--->>> ��ȡ��ǰ������ = " << pUserApi->GetTradingDay() << endl;
		strcpy(tradingday, pMdApi->GetTradingDay());
		// ����������
		SubscribeMarketData();

		char logline[200];
		sprintf(logline, "CTP MD RspLogin Success.");
		Message logMsg;
		logMsg.type = MD_LOG;
		logMsg.AddData(logline, 0, sizeof(char) * 200);
		LogMessageList.AddTail(logMsg);
		ReleaseSemaphore(logSemaphore, 1, NULL);
	}
	else {
		if (IsErrorRspInfo(pRspInfo)) {
			char logline[200];
			sprintf(logline, "CTP MD RspLogin Failed.");
			Message logMsg;
			logMsg.type = MD_LOG;
			logMsg.AddData(logline, 0, sizeof(char) * 200);
			LogMessageList.AddTail(logMsg);
			ReleaseSemaphore(logSemaphore, 1, NULL);
		}
	}
}

void CThostMdSpi::SubscribeMarketData()
{
	int iResult = pMdApi->SubscribeMarketData(ppInstrumentID, iInstrumentID);
	CString str("��������������");
	pMdPubMsg->AddString(str);
	//cerr << "--->>> �������鶩������: " << ((iResult == 0) ? "�ɹ�" : "ʧ��") << endl;
}

int CThostMdSpi::AddSubscribeMarketData(char* ppNewInstrumentID[100], int iNewInstrumentID)
{
	int iResult = pMdApi->SubscribeMarketData(ppNewInstrumentID, iNewInstrumentID);
	CString str("��������������������");
	pMdPubMsg->AddString(str);
	return iResult;
	//cerr << "--->>> �������鶩������: " << ((iResult == 0) ? "�ɹ�" : "ʧ��") << endl;
}

int CThostMdSpi::UnSubscribeMarketData(char* ppNewInstrumentID[100], int iNewInstrumentID)
{
	int iResult = pMdApi->UnSubscribeMarketData(ppNewInstrumentID, iNewInstrumentID);
	CString str("�����˶���������");
	pMdPubMsg->AddString(str);
	return iResult;
	//cerr << "--->>> �������鶩������: " << ((iResult == 0) ? "�ɹ�" : "ʧ��") << endl;
}

void CThostMdSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	CString str("OnRspSubMarketData");
	CString strInst(pSpecificInstrument->InstrumentID);

	if (IsErrorRspInfo(pRspInfo) == false) {
		str.Append(_T("-->>"));
		str.Append(strInst);
		str.Append(_T(" �ɹ�"));
		pMdPubMsg->AddString(str);
	}
	//cerr << "OnRspSubMarketData" << endl;
}

void CThostMdSpi::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	CString str("OnRspUnSubMarketData");
	CString strInst(pSpecificInstrument->InstrumentID);

	if (IsErrorRspInfo(pRspInfo) == false) {
		str.Append(_T("-->>"));
		str.Append(strInst);
		str.Append(_T(" �ɹ�"));
		pMdPubMsg->AddString(str);
	}
}

void CThostMdSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData)
{
	int nHour, nMin, nSec;
	scanf(pDepthMarketData->UpdateTime, "%d:%d:%d", &nHour, &nMin, &nSec);

	if ((nHour > 2 && nHour <= 8) || (nHour == 15 && nMin >= 15) || nHour > 15 && nHour < 21)return;

	TickInfo tickData;
	strcpy(tickData.ordername, pDepthMarketData->InstrumentID);
	//strcpy(tickData.TradingDay,beginrun_date);
	strcpy(tickData.updatetime, pDepthMarketData->UpdateTime);
	//tickData.UpdateMillisec=pDepthMarketData->UpdateMillisec/500*500;
	tickData.bid1 = pDepthMarketData->BidPrice1;
	tickData.bidvol1 = pDepthMarketData->BidVolume1;
	tickData.ask1 = pDepthMarketData->AskPrice1;
	tickData.askvol1 = pDepthMarketData->AskVolume1;
	tickData.price = pDepthMarketData->LastPrice;
	tickData.vol = pDepthMarketData->Volume;
	tickData.upperLimitPrice = pDepthMarketData->UpperLimitPrice;
	tickData.lowerLimitPrice = pDepthMarketData->LowerLimitPrice;
	tickData.openprice = pDepthMarketData->OpenPrice;
	TickList.AddTail(tickData);
	//	TRACE("%s,%.5f,%s\n",tickData.CodeName,tickData.price,tickData.updatetime);

	ReleaseSemaphore(MdTickSem, 2, NULL);
}

bool CThostMdSpi::IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo)
{
	// ���ErrorID != 0, �յ��˴������Ӧ
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

bool CThostMdSpi::timeRuleForOpen(TThostFtdcTimeType datatime) {
	int nHour, nMin, nSec;

	sscanf_s(datatime, "%d:%d:%d", &nHour, &nMin, &nSec);

	if (nHour == 9 && nMin <= 14) return false;
	if (nHour == 11 && nMin == 29) return false;
	if (nHour == 15 && nMin >= 14) return false;

	return true;
}