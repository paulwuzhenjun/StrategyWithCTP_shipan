#pragma once
#include "resource.h"
#include "afxwin.h"

// CMdInstMgrDlg 对话框
class CMdInstMgrDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMdInstMgrDlg)

public:
	CMdInstMgrDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CMdInstMgrDlg();

	// 对话框数据
	enum { IDD = IDD_INST_MGR_DLG };

	int nItem, nSubItem;
	CListCtrl* pInstsSubscribedList;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	void ConvertCStringToCharArray(CString csSource, char* rtnCharArray);
	CString GetItemText(HWND hWnd, int nItem, int nSubItem) const;
	afx_msg void OnNMClickParamList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEnKillfocusEdit1();
	afx_msg void OnBnClickedAddBtn();
	afx_msg void OnBnClickedDeleteBtn();
	afx_msg void OnBnClickedSaveBtn();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedAddintoSublist();
	afx_msg void OnBnClickedRemovefromSublist();
	afx_msg void OnBnClickedSubBtn();
	CListBox m_sublist_listbox;
	int xxtest;
};
