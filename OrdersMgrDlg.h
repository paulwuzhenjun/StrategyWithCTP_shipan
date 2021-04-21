#pragma once
#include "resource.h"

// OrdersMgrDlg �Ի���

class OrdersMgrDlg : public CDialogEx
{
	DECLARE_DYNAMIC(OrdersMgrDlg)

public:
	OrdersMgrDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~OrdersMgrDlg();

	// �Ի�������
	enum { IDD = IDD_ORDERS_DLG };

	void ReSize();
	POINT old;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedQryOrdersBtn();
	afx_msg void OnNMClickOrdersList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLvnColumnclickList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
