#pragma once
#include "resource.h"
#include "afxcmn.h"
#include "MyStruct.h"
#include "InstListDlg.h"

// ParamsMgrDlg 对话框

class ParamsMgrDlg : public CDialogEx
{
	DECLARE_DYNAMIC(ParamsMgrDlg)

public:
	ParamsMgrDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~ParamsMgrDlg();

	// 对话框数据
	enum { IDD = IDD_PARAMS_DLG };

	//CString StrategyShowing;
	int nItem, nSubItem;

	void ReSize();
	POINT old;
	wstring s2ws(const string& s);
	string ws2s(const wstring& ws);
	int ValidateInstance(StrategyInstanceNode mInstance);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	void AddParamList(CString xStrategyID, StrategyNode mStrategyNode);
	void SetCell(HWND hWnd1, CString value, int nRow, int nCol);
	void SetSaveButtonDisable();
	void SetSaveButtonEnable();
	void ConvertCStringToCharArray(CString csSource, char* rtnCharArray);

	CString GetItemText(HWND hWnd, int nItem, int nSubItem) const;
	afx_msg void OnNMClickParamList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEnKillfocusEdit1();
	afx_msg void OnBnClickedSaveBtn();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//afx_msg void OnItemchangedLinksList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedAddInstanceBtn();
	afx_msg void OnBnClickedDeleteInstanceBtn();
};
