#pragma once
#include "stdafx.h"
#include "resource.h"
// MdDlg �Ի���

class MdDlg : public CDialog
{
	DECLARE_DYNAMIC(MdDlg)

public:
	MdDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~MdDlg();
	BOOL Create();
	// �Ի�������
	enum { IDD = IDD_MD_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedConnectMdServer();
	afx_msg void OnMenuInstMgr();
	afx_msg void OnReconnectMenu();
};
