// InstMgrDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "InstMgrDlg.h"
#include "afxdialogex.h"
#include <map>
#include "MyStruct.h"

using namespace std;

// InstMgrDlg 对话框
extern map<string, InstrumentInfo> InstrumentsSubscribed;

IMPLEMENT_DYNAMIC(InstMgrDlg, CDialogEx)

InstMgrDlg::InstMgrDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(InstMgrDlg::IDD, pParent)
{
}

InstMgrDlg::~InstMgrDlg()
{
}

void InstMgrDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(InstMgrDlg, CDialogEx)
END_MESSAGE_MAP()

// InstMgrDlg 消息处理程序
BOOL InstMgrDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	//委托单列表
	CListCtrl* pInstsSubscribedList = (CListCtrl*)GetDlgItem(IDC_INST_LIST);
	pInstsSubscribedList->DeleteAllItems();
	pInstsSubscribedList->ModifyStyle(0, LVS_REPORT);
	//pQryOrdersList->ModifyStyle(0,LVS_SINGLESEL);
	//	int  k=m_list->GetHeaderCtrl()->GetItemCount();
	//	for(int m=0;m <=k;m++)
	//	{
	//		m_list->DeleteColumn(0);
	//	}
	LV_COLUMN column;
	column.pszText = _T("交易所代码");
	column.mask = LVCF_TEXT;
	pInstsSubscribedList->InsertColumn(0, &column); //插入一列，列索引为0
	//m_list->SetColumnWidth(0,100); //列宽设置
	column.pszText = _T("品种代码");
	column.mask = LVCF_TEXT;
	pInstsSubscribedList->InsertColumn(1, &column);

	column.pszText = _T("合约代码");
	column.mask = LVCF_TEXT;
	pInstsSubscribedList->InsertColumn(2, &column);

	column.pszText = _T("最小变动价位");
	column.mask = LVCF_TEXT;
	pInstsSubscribedList->InsertColumn(3, &column);

	CRect rect4;
	pInstsSubscribedList->GetClientRect(rect4);                     //获得当前客户区信息
	pInstsSubscribedList->SetColumnWidth(0, rect4.Width() / 4);        //设置列的宽度。
	pInstsSubscribedList->SetColumnWidth(1, rect4.Width() / 4);
	pInstsSubscribedList->SetColumnWidth(2, rect4.Width() / 4);
	pInstsSubscribedList->SetColumnWidth(3, rect4.Width() / 4);

	map<string, InstrumentInfo>::iterator iter;//定义一个迭代指针iter
	for (iter = InstrumentsSubscribed.begin(); iter != InstrumentsSubscribed.end(); iter++) {
		int orderIndex = pInstsSubscribedList->GetItemCount();
		CString itemname("");
		itemname.Format(_T("%d"), orderIndex);
		pInstsSubscribedList->InsertItem(orderIndex, (LPCTSTR)itemname);

		CString csExchangeID(iter->second.ExchangeID);
		pInstsSubscribedList->SetItemText(orderIndex, 0, (LPCTSTR)csExchangeID);

		CString csCommodityNo(iter->second.CommodityNo);
		pInstsSubscribedList->SetItemText(orderIndex, 1, (LPCTSTR)csCommodityNo);

		CString csInstrumentID(iter->second.InstrumentID);
		pInstsSubscribedList->SetItemText(orderIndex, 2, (LPCTSTR)csInstrumentID);

		CString csOneTick("");
		csOneTick.Format(_T("%.5f"), iter->second.OneTick);
		pInstsSubscribedList->SetItemText(orderIndex, 3, (LPCTSTR)csOneTick);

		pInstsSubscribedList->EnsureVisible(pInstsSubscribedList->GetItemCount() - 1, FALSE);
	}
	return TRUE;
}