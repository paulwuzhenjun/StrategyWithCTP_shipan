#pragma once
#include "afxwin.h"

// CInsertOrderDlg 对话框

class CInsertOrderDlg : public CDialog
{
	DECLARE_DYNAMIC(CInsertOrderDlg)

public:
	CInsertOrderDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CInsertOrderDlg();

	BOOL Create();
	// 对话框数据
	enum { IDD = IDD_INSERT_ORDERS_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	//CComboBox m_cbOpenOrClose;
	CComboBox m_cbDirection;
	CComboBox m_cbManualOrderStra;
	CComboBox m_cbInstCode;
	CComboBox m_cbOpenOrClose;
};
