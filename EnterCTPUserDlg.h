#pragma once
#include "resource.h"
// EnterCTPUserDlg �Ի���

class EnterCTPUserDlg : public CDialog
{
	DECLARE_DYNAMIC(EnterCTPUserDlg)

public:
	EnterCTPUserDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~EnterCTPUserDlg();

	// �Ի�������
	enum { IDD = IDD_CTP_USER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
