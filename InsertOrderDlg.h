#pragma once
#include "afxwin.h"

// CInsertOrderDlg �Ի���

class CInsertOrderDlg : public CDialog
{
	DECLARE_DYNAMIC(CInsertOrderDlg)

public:
	CInsertOrderDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CInsertOrderDlg();

	BOOL Create();
	// �Ի�������
	enum { IDD = IDD_INSERT_ORDERS_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	//CComboBox m_cbOpenOrClose;
	CComboBox m_cbDirection;
	CComboBox m_cbManualOrderStra;
	CComboBox m_cbInstCode;
	CComboBox m_cbOpenOrClose;
};
