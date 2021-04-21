#pragma once
#include "resource.h"
#include <string>
#include <list>

#define EDIT_ID 10000
// CInstListDlg 对话框

class CInstListDlg : public CDialog
{
	DECLARE_DYNAMIC(CInstListDlg)

public:
	CInstListDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CInstListDlg();

	BOOL Create();
	// 对话框数据
	enum { IDD = IDD_INST_LIST_DLG };

	std::list<string> paramnamelist;
	std::list<string> paramvaluelist;
	CPtrArray p_MyStatics;
	CPtrArray p_MyEdits;

	char InstFullName[50];
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};
