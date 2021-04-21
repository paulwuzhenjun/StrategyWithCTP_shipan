#pragma once
#include "resource.h"

// InstMgrDlg 对话框

class InstMgrDlg : public CDialogEx
{
	DECLARE_DYNAMIC(InstMgrDlg)

public:
	InstMgrDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~InstMgrDlg();

	// 对话框数据
	enum { IDD = IDD_INST_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
};
