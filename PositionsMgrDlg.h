#pragma once
#include "resource.h"

// PositionsMgrDlg �Ի���

class PositionsMgrDlg : public CDialogEx
{
	DECLARE_DYNAMIC(PositionsMgrDlg)

public:
	PositionsMgrDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~PositionsMgrDlg();

	// �Ի�������
	enum { IDD = IDD_POSITIONS_DLG };

	void ReSize();
	POINT old;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedQryPositionsBtn();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLvnColumnclickList1(NMHDR* pNMHDR, LRESULT* pResult);
};
