#pragma once
#include "resource.h"
//#include "afxcmn.h"

// StrategyRecoverDlg 对话框

class StrategyRecoverDlg : public CDialog
{
	DECLARE_DYNAMIC(StrategyRecoverDlg)

public:
	StrategyRecoverDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~StrategyRecoverDlg();

	// 对话框数据
	enum { IDD = IDD_RECOVER_DLG };

	int nItem, nSubItem;
	int nItemOrder, nSubItemOrder;

	wstring s2ws(const string& s);
	string ws2s(const wstring& ws);
	void ConvertCStringToCharArray(CString csSource, char* rtnCharArray);

	void ReSize();
	POINT old;

	int RecoverStrategySelectedRow;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()
public:
	void SetCell(HWND hWnd1, CString value, int nRow, int nCol);
	afx_msg void OnEnKillfocusEdit1();

	CString GetItemText(HWND hWnd, int nItem, int nSubItem) const;
	void ShowRecoverInstance(CString xStrategyName, CString xStrategyID, CString  xInstanceName, CString xStartTime, CString xEndTime);
	void ShowRecoverInstanceOrder(CString xStrategyName, CString xStrategyID, CString  xInstanceName);

	afx_msg void OnNMClickStrategyTobeRecoverLst(NMHDR* pNMHDR, LRESULT* pResult);
	CListCtrl* mRecoverParamList;
	CListCtrl* mRecoverOrderList;
	afx_msg void OnBnClickedRestartInstanceBtn();
	afx_msg void OnBnClickedClearInstanceBtn();
	afx_msg void OnNMDblclkRecoverOrderList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedSaveRecovOrdersBtn();
};
