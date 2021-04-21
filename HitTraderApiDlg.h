// TestTraderApiDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "MyThread.h"
#include "InsertOrderDlg.h"
#include "OrdersMgrDlg.h"
#include "ParamsMgrDlg.h"
#include "PositionsMgrDlg.h"
#include "LocalCLMgrDlg.h"
#include "TradesDisplayDlg.h"
#include "MdDlg.h"
//#include "StrategyRecoverDlg.h"

#define WM_PUBMSG WM_USER+22

// CHitTraderApiDlg dialog
class CHitTraderApiDlg : public CDialog
{
	// Construction
public:
	CHitTraderApiDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CHitTraderApiDlg();

	CInsertOrderDlg* insertOrderDlg;
	MdDlg* mMdDlg;

	CString m_testString;

	// Dialog Data
	enum { IDD = 102 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	void ReSize();
	POINT old;
	// Implementation
protected:
	HICON m_hIcon;

	LRESULT OnPubmsg(WPARAM wParam, LPARAM lParam);

	// Generated message map functions
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLbnSelchangeList1();
	CListBox m_ctrlPublisMsg;
	CImageList m_imageList;
	CTreeCtrl m_webTree;

	CTabCtrl m_tab;
	int m_CurSelTab;
	ParamsMgrDlg m_ParamsMgrPage;
	OrdersMgrDlg m_OrdersMgrPage;
	PositionsMgrDlg m_PositionsMgrPage;
	LocalCLMgrDlg m_LocalCLMgrDlgPage;
	TradesDisplayDlg m_TradesDisplayDlgPage;

	//StrategyRecoverDlg m_StrategyRecoverDlg;

	void StrategyResetAction();
	void StrategyInitAction();
	CDialog* pDialog[5];

	HANDLE mfile_handle;
	bool MdStarted;
	bool TraderStarted;
	bool StrategyStarted;
	void CheckAndCreateDirectory(CString csDirectionName);
	int Split(CString source, CString ch, CStringArray& strarr);
	//void UpLoadAcctValue(char* time);

	afx_msg void OnBnOK();
	afx_msg void OnBnCancel();
	afx_msg void OnBnClickedConnect();
	afx_msg void OnBnClickedStartStrategy();
	afx_msg void OnBnClickedStopStrategy();
	afx_msg void OnMenuInsertOrder();
	//afx_msg void OnBnClickedExportOrders();
	afx_msg void OnBnClickedRefreshAvailMoney();
	//afx_msg void OnBnClickedScheduleBtn();
	afx_msg void OnMenuOptions();
	afx_msg void OnTvnSelchangedStrategyTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTcnSelchangeMaintab(NMHDR* pNMHDR, LRESULT* pResult);

	afx_msg void OnBnClickedSetPositionLimitBtn();
	afx_msg void OnMenuProfitAnalyze();
	afx_msg void OnReconnectTd();
	afx_msg void OnMdDlg();
	afx_msg void OnStRecover();
};
