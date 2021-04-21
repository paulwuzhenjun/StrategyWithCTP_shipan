#pragma once
#include "resource.h"

// TradesDisplayDlg 对话框

class TradesDisplayDlg : public CDialogEx
{
	DECLARE_DYNAMIC(TradesDisplayDlg)

public:
	TradesDisplayDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~TradesDisplayDlg();

	// 对话框数据
	enum { IDD = IDD_TRADES_DLG };

	void ReSize();
	POINT old;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
