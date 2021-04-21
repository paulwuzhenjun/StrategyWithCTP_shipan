// InstListDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "InstListDlg.h"
#include "afxdialogex.h"
#include "GlobalFunc.h"

extern GlobalFunc globalFuncUtil;
// CInstListDlg 对话框

IMPLEMENT_DYNAMIC(CInstListDlg, CDialog)

CInstListDlg::CInstListDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CInstListDlg::IDD, pParent)
{
}

CInstListDlg::~CInstListDlg()
{
	int n = p_MyStatics.GetSize();
	for (int i = 0; i < n; i++)
	{
		CStatic* pStatic = (CStatic*)p_MyStatics.GetAt(i);
		//  pStatic->DestroyWindow();
		if (pStatic != NULL)
			delete pStatic;
		p_MyStatics[i] = NULL;
	}
	p_MyStatics.RemoveAll();
	n = p_MyEdits.GetSize();
	for (int i = 0; i < n; i++)
	{
		CEdit* pEdit = (CEdit*)p_MyEdits.GetAt(i);
		//pEdit->DestroyWindow();
		if (pEdit != NULL)
			delete pEdit;
		p_MyEdits[i] = NULL;
	}
	p_MyEdits.RemoveAll();
}

BOOL CInstListDlg::Create()
{
	CDialog::Create(CInstListDlg::IDD);
	return TRUE;
}

void CInstListDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CInstListDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CInstListDlg::OnBnClickedOk)
END_MESSAGE_MAP()

// CInstListDlg 消息处理程序
BOOL CInstListDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	CString m_staticCaption;
	CRect rect, rect2;
	GetClientRect(&rect);
	GetDlgItem(IDOK)->GetWindowRect(&rect2);
	ScreenToClient(&rect2);

	int m_counts = paramnamelist.size();

	//	MoveWindow(rect.left,rect.top,rect.Width(),m_counts/2*60);

	int perWidth = rect.Width() / 5;
	int perHeight = rect2.top / m_counts;
	CStatic* p_MyStatic;
	CEdit* p_MyEdit;

	std::list<string>::iterator strname_itr;
	std::list<string>::iterator strvalue_itr;
	int i = 0;
	for (strname_itr = paramnamelist.begin(), strvalue_itr = paramvaluelist.begin(); strname_itr != paramnamelist.end() && strvalue_itr != paramvaluelist.end(); ++strname_itr, ++strvalue_itr, i++) {
		TRACE("%s,%s\n", (*strname_itr).c_str(), (*strvalue_itr).c_str());

		p_MyStatic = new CStatic();
		p_MyEdit = new CEdit();
		CString csName((*strname_itr).c_str());
		m_staticCaption = csName;
		//m_staticCaption.Format(_T("第%d台IP地址:"),i+1);
		if (i % 2 == 0)
		{
			p_MyStatic->Create(m_staticCaption, WS_CHILD | WS_VISIBLE | SS_CENTER, CRect(0, i * perHeight + 5, perWidth, (i + 1) * perHeight + 5), this);

			CString csValue((*strvalue_itr).c_str());
			m_staticCaption = csValue;

			p_MyEdit->Create(WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, CRect(perWidth, i * perHeight + 5, perWidth * 2, (i + 1) * perHeight + 5), this, EDIT_ID + i);
			p_MyEdit->SetWindowText(m_staticCaption);
			if (p_MyStatic != NULL)
			{
				p_MyStatics.Add((void*)p_MyStatic);
				//  delete p_MyStatic;
			}
			if (p_MyEdit != NULL)
			{
				p_MyEdits.Add((void*)p_MyEdit);
				//  delete p_MyEdit;
			}
		}
		else
		{
			p_MyStatic->Create(m_staticCaption, WS_CHILD | WS_VISIBLE | SS_CENTER, CRect(perWidth * 2, (i - 1) * perHeight + 5, perWidth * 3, i * perHeight + 5), this);

			CString csValue((*strvalue_itr).c_str());
			m_staticCaption = csValue;

			p_MyEdit->Create(WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTER, CRect(perWidth * 3, (i - 1) * perHeight + 5, perWidth * 4, i * perHeight + 5), this, EDIT_ID + i);
			p_MyEdit->SetWindowText(m_staticCaption);
			if (p_MyStatic != NULL)
			{
				p_MyStatics.Add((void*)p_MyStatic);
				//delete p_MyStatic;
			}
			if (p_MyEdit != NULL)
			{
				p_MyEdits.Add((void*)p_MyEdit);
				//  delete p_MyEdit;
			}
		}
	}

	return TRUE;
}

void CInstListDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	strcpy(InstFullName, "COMEX GC 1806");
	std::list<string>::iterator strname_itr;
	std::list<string>::iterator strvalue_itr;
	for (strname_itr = paramnamelist.begin(), strvalue_itr = paramvaluelist.begin(); strname_itr != paramnamelist.end() && strvalue_itr != paramvaluelist.end(); ++strname_itr, ++strvalue_itr) {
		TRACE("%s,%s\n", (*strname_itr).c_str(), (*strvalue_itr).c_str());
	}

	int	n = p_MyEdits.GetSize();
	for (int i = 0; i < n; i++)
	{
		CEdit* pEdit = (CEdit*)p_MyEdits.GetAt(i);
		CString csParamValue;
		pEdit->GetWindowTextW(csParamValue);
		char cParamValue[100];
		globalFuncUtil.ConvertCStringToCharArray(csParamValue, cParamValue);
		TRACE("%s\n", cParamValue);
		//pEdit->DestroyWindow();
		if (pEdit != NULL)
			delete pEdit;
		p_MyEdits[i] = NULL;
	}

	CDialog::OnOK();
}