#pragma once
#include "MyTimer.h"

#define WM_CREATE_ESUN_MD WM_USER+1
#define WM_CREATE_CTP_MD WM_USER+2
#define WM_CREATE_SGIT_MD WM_USER+3
#define WM_CREATE_LOG_THREAD WM_USER+4

#define WM_CREATE_MD_DISPATHER WM_USER+5
#define WM_RESTART_ESUN_MD WM_USER+6
#define WM_RESTART_CTP_MD WM_USER+7
#define WM_ADD_NEW_SUBINST WM_USER+8

#define WM_START_STRATEGY WM_USER+9
#define WM_STOP_STRATEGY WM_USER+10
#define WM_STOP WM_USER+11  //程序动作停止，账号登出
#define WM_QRY_POSITION_DETAILS WM_USER+12
#define WM_QRY_ORDERS WM_USER+13
#define WM_WITHDRAW_ORDER WM_USER+14
#define WM_START_ACTION_THREAD WM_USER+15
#define WM_QRY_MONEY WM_USER+16

#define WM_CREATE_TDSHM_THREAD WM_USER+17
#define WM_RESTART_ESUN_TRADER WM_USER+18
#define WM_RESTART_STRATEGY_RECOVERDLG WM_USER+19
#define WM_QRY_TRADES WM_USER+20
#define WM_RECOVER_QRY_ORDERS WM_USER+21
#define WM_TDLOST_QRY_TRADES WM_USER+22
#define WM_CREATE_ESUN_TRADER WM_USER+23
#define WM_CREATE_CTP_TRADER WM_USER+24
#define WM_RESTART_CTP_TRADER WM_USER+25
#define WM_CREATE_SGIT_TRADER WM_USER+26
#define WM_CHECK_INSTANCE_CFG WM_USER+27
#define WM_UPLOAD_TRADE_LOG WM_USER+28
#define WM_UPLOAD_VALUE WM_USER+29
#define WM_UPLOAD_CTP_ACCVALUE WM_USER+30
#define WM_SEND_UDP_MESSAGE WM_USER+31
#define WM_CREATE_RM_TCP WM_USER+32

// CMyThread
class CMyThread : public CWinThread
{
	DECLARE_DYNCREATE(CMyThread)

protected:
	CMyThread();           // protected constructor used by dynamic creation
	virtual ~CMyThread();

	afx_msg void OnCreateEsunTrader(UINT wParam, LONG lParam);
	afx_msg void OnRestartEsunTrader(UINT wParam, LONG lParam);
	afx_msg void OnCreateMd(UINT wParam, LONG lParam);
	afx_msg void OnStartStrategy(UINT wParam, LONG lParam);
	afx_msg void OnReStartStrategyFromRecoverDlg(UINT wParam, LONG lParam);
	afx_msg void OnStopStrategy(UINT wParam, LONG lParam);
	afx_msg void OnStartProcessActionThread(UINT wParam, LONG lParam);
	afx_msg void OnStop(UINT wParam, LONG lParam);
	afx_msg void OnSendQryPositionDetailsMsg(UINT wParam, LONG lParam);
	afx_msg void OnSendQryOrdersMsg(UINT wParam, LONG lParam);
	afx_msg void OnSendQryMoneyMsg(UINT wParam, LONG lParam);
	afx_msg void OnWithdrawOrder(UINT wParam, LONG lParam);
	afx_msg void OnCreateLogThread(UINT wParam, LONG lParam);
	afx_msg void OnCreateTdDispatchThread(UINT wParam, LONG lParam);
	afx_msg void OnSendQryTradesMsg(UINT wParam, LONG lParam);
	afx_msg void OnTDLostSendQryTradesMsg(UINT wParam, LONG lParam);
	afx_msg void OnSendRecoverQryOrdersMsg(UINT wParam, LONG lParam);
	afx_msg void OnCreateCTPTrader(UINT wParam, LONG lParam);
	afx_msg void OnRestartCTPTrader(UINT wParam, LONG lParam);
	afx_msg void OnCreateSgitTrader(UINT wParam, LONG lParam);
	afx_msg void CheckAllInstanceStatus(WPARAM wParam, LPARAM lParam);

	afx_msg void OnCreateEsunMd(UINT wParam, LONG lParam);
	afx_msg void OnCreateCTPMd(UINT wParam, LONG lParam);
	afx_msg void OnCreateSgitMd(UINT wParam, LONG lParam);
	afx_msg void OnCreateMDDispatchThread(UINT wParam, LONG lParam);
	afx_msg void OnRestartEsunMd(UINT wParam, LONG lParam);
	afx_msg void OnRestartCTPMd(WPARAM wParam, LPARAM lParam);
	afx_msg void OnAddNewSubInst(WPARAM wParam, LPARAM lParam);

	afx_msg void OnUpLoadTradeLog(WPARAM wParam, LPARAM lParam);
	afx_msg void OnUpLoadValue(WPARAM wParam, LPARAM lParam);
	afx_msg void OnUpLoadCTPAccValue(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSendUdpMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void OnCreateRMTcpThread(UINT wParam, LONG lParam);
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

	CMyTimer simulateTimer;
	//void CreateMdSpi();
	//void CreateTraderApi();
	void Stop();

	wstring s2ws(const string& s);
	void ConvertCStringToCharArray(CString csSource, char* rtnCharArray);
protected:
	DECLARE_MESSAGE_MAP()
};
