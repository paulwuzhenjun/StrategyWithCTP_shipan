#pragma once
#include "resource.h"

// PositionsMgrDlg 对话框

class PositionsMgrDlg : public CDialogEx
{
	DECLARE_DYNAMIC(PositionsMgrDlg)

public:
	PositionsMgrDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~PositionsMgrDlg();

	// 对话框数据
	enum { IDD = IDD_POSITIONS_DLG };

	void ReSize();
	POINT old;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedQryPositionsBtn();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLvnColumnclickList1(NMHDR* pNMHDR, LRESULT* pResult);
};
