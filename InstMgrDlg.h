#pragma once
#include "resource.h"

// InstMgrDlg �Ի���

class InstMgrDlg : public CDialogEx
{
	DECLARE_DYNAMIC(InstMgrDlg)

public:
	InstMgrDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~InstMgrDlg();

	// �Ի�������
	enum { IDD = IDD_INST_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
};
