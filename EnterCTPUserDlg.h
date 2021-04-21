#pragma once
#include "resource.h"
// EnterCTPUserDlg 对话框

class EnterCTPUserDlg : public CDialog
{
	DECLARE_DYNAMIC(EnterCTPUserDlg)

public:
	EnterCTPUserDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~EnterCTPUserDlg();

	// 对话框数据
	enum { IDD = IDD_CTP_USER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
