#pragma once
#include "resource.h"

// LocalCLMgrDlg �Ի���

class LocalCLMgrDlg : public CDialogEx
{
	DECLARE_DYNAMIC(LocalCLMgrDlg)

public:
	LocalCLMgrDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~LocalCLMgrDlg();

	// �Ի�������
	enum { IDD = IDD_STRATEGY_LOCAL_CL };

	void ReSize();
	POINT old;

	int nItem, nSubItem;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	void SetCell(HWND hWnd1, CString value, int nRow, int nCol);
	CString GetItemText(HWND hWnd, int nItem, int nSubItem) const;
	afx_msg void OnEnKillfocusEdit1();

	afx_msg void OnBnClickedQryStraLocalClBtn();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLvnColumnclickList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMDblclkStrategyLocalClList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedSaveClBtn();
};
