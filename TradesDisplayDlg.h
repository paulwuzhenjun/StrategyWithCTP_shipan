#pragma once
#include "resource.h"

// TradesDisplayDlg �Ի���

class TradesDisplayDlg : public CDialogEx
{
	DECLARE_DYNAMIC(TradesDisplayDlg)

public:
	TradesDisplayDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~TradesDisplayDlg();

	// �Ի�������
	enum { IDD = IDD_TRADES_DLG };

	void ReSize();
	POINT old;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
