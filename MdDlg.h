#pragma once
#include "stdafx.h"
#include "resource.h"
// MdDlg 对话框

class MdDlg : public CDialog
{
	DECLARE_DYNAMIC(MdDlg)

public:
	MdDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~MdDlg();
	BOOL Create();
	// 对话框数据
	enum { IDD = IDD_MD_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedConnectMdServer();
	afx_msg void OnMenuInstMgr();
	afx_msg void OnReconnectMenu();
};
