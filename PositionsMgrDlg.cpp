// PositionsMgrDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "PositionsMgrDlg.h"
#include "afxdialogex.h"
#include "MyThread.h"

// PositionsMgrDlg 对话框
extern CListCtrl* pPositionDetailsList;
extern CMyThread* pQryActionThread;

IMPLEMENT_DYNAMIC(PositionsMgrDlg, CDialogEx)

static int CALLBACK MyPosiMgrCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CString& lp1 = *((CString*)lParam1);
	CString& lp2 = *((CString*)lParam2);
	int& sort = *(int*)lParamSort;
	if (sort == 0)
	{
		return lp1.CompareNoCase(lp2);
	}
	else
	{
		return lp2.CompareNoCase(lp1);
	}
}

PositionsMgrDlg::PositionsMgrDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(PositionsMgrDlg::IDD, pParent)
{
}

PositionsMgrDlg::~PositionsMgrDlg()
{
}

void PositionsMgrDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(PositionsMgrDlg, CDialogEx)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_QRY_POSITIONS_BTN, &PositionsMgrDlg::OnBnClickedQryPositionsBtn)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_QRY_POSITIONS_BTN, &PositionsMgrDlg::OnLvnColumnclickList1)
END_MESSAGE_MAP()

BOOL PositionsMgrDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	//持仓明细列表
	pPositionDetailsList = (CListCtrl*)GetDlgItem(IDC_POSITIONS_LIST);
	pPositionDetailsList->DeleteAllItems();
	pPositionDetailsList->ModifyStyle(0, LVS_REPORT);
	//	int  k=m_list->GetHeaderCtrl()->GetItemCount();
	//	for(int m=0;m <=k;m++)
	//	{
	//		m_list->DeleteColumn(0);
	//	}
	LV_COLUMN column1;
	column1.pszText = _T("品种ID");
	column1.mask = LVCF_TEXT;
	pPositionDetailsList->InsertColumn(0, &column1);

	column1.pszText = _T("合约ID");
	column1.mask = LVCF_TEXT;
	pPositionDetailsList->InsertColumn(1, &column1); //插入一列，列索引为0
	//m_list->SetColumnWidth(0,100); //列宽设置

	column1.pszText = _T("方向");
	column1.mask = LVCF_TEXT;
	pPositionDetailsList->InsertColumn(2, &column1);

	column1.pszText = _T("开仓时间");
	column1.mask = LVCF_TEXT;
	pPositionDetailsList->InsertColumn(3, &column1);

	column1.pszText = _T("手数");
	column1.mask = LVCF_TEXT;
	pPositionDetailsList->InsertColumn(4, &column1);

	column1.pszText = _T("开仓价格");
	column1.mask = LVCF_TEXT;
	pPositionDetailsList->InsertColumn(5, &column1);

	column1.pszText = _T("所属策略");
	column1.mask = LVCF_TEXT;
	pPositionDetailsList->InsertColumn(6, &column1);

	CRect rect;
	pPositionDetailsList->GetClientRect(rect);                     //获得当前客户区信息
	pPositionDetailsList->SetColumnWidth(0, rect.Width() / 8);        //设置列的宽度。
	pPositionDetailsList->SetColumnWidth(1, rect.Width() / 8);
	pPositionDetailsList->SetColumnWidth(2, rect.Width() / 8);
	pPositionDetailsList->SetColumnWidth(3, rect.Width() / 8);
	pPositionDetailsList->SetColumnWidth(4, rect.Width() / 8);
	pPositionDetailsList->SetColumnWidth(5, rect.Width() / 8);
	pPositionDetailsList->SetColumnWidth(6, 2 * rect.Width() / 8);

	return TRUE;
}
// PositionsMgrDlg 消息处理程序

void PositionsMgrDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	// TODO: Add your message handler code here
	if (nType == SIZE_RESTORED || nType == SIZE_MAXIMIZED)
	{
		ReSize();
	}
}

void PositionsMgrDlg::ReSize()
{
	float fsp[2];
	POINT Newp; //获取现在对话框的大小
	CRect recta;
	GetClientRect(&recta);     //取客户区大小
	Newp.x = recta.right - recta.left;
	Newp.y = recta.bottom - recta.top;
	fsp[0] = (float)Newp.x / old.x;
	fsp[1] = (float)Newp.y / old.y;
	CRect Rect;
	int woc;
	CPoint OldTLPoint, TLPoint; //左上角
	CPoint OldBRPoint, BRPoint; //右下角
	HWND  hwndChild = ::GetWindow(m_hWnd, GW_CHILD);  //列出所有控件
	while (hwndChild)
	{
		woc = ::GetDlgCtrlID(hwndChild);//取得ID
		GetDlgItem(woc)->GetWindowRect(Rect);
		ScreenToClient(Rect);
		OldTLPoint = Rect.TopLeft();
		TLPoint.x = long(OldTLPoint.x * fsp[0]);
		TLPoint.y = long(OldTLPoint.y * fsp[1]);
		OldBRPoint = Rect.BottomRight();
		BRPoint.x = long(OldBRPoint.x * fsp[0]);
		BRPoint.y = long(OldBRPoint.y * fsp[1]);
		Rect.SetRect(TLPoint, BRPoint);
		GetDlgItem(woc)->MoveWindow(Rect, TRUE);
		hwndChild = ::GetWindow(hwndChild, GW_HWNDNEXT);
	}
	old = Newp;

	if (pPositionDetailsList != NULL) {
		CRect rect;
		pPositionDetailsList->GetClientRect(rect);                     //获得当前客户区信息
		pPositionDetailsList->SetColumnWidth(0, rect.Width() / 8);        //设置列的宽度。
		pPositionDetailsList->SetColumnWidth(1, rect.Width() / 8);
		pPositionDetailsList->SetColumnWidth(2, rect.Width() / 8);
		pPositionDetailsList->SetColumnWidth(3, rect.Width() / 8);
		pPositionDetailsList->SetColumnWidth(4, rect.Width() / 8);
		pPositionDetailsList->SetColumnWidth(5, 2 * rect.Width() / 8);
		pPositionDetailsList->SetColumnWidth(6, 2 * rect.Width() / 8);
	}
}

void PositionsMgrDlg::OnBnClickedQryPositionsBtn()
{
	// TODO: 在此添加控件通知处理程序代码
	if (IDOK == AfxMessageBox(_T("是否查询持仓?"), MB_OKCANCEL, 0)) {
		pPositionDetailsList->DeleteAllItems();//清空窗口列表

		pQryActionThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
		Sleep(500);
		pQryActionThread->PostThreadMessage(WM_QRY_POSITION_DETAILS, NULL, NULL);
		TRACE("Qry Postions Info");
	}
}

void PositionsMgrDlg::OnLvnColumnclickList1(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	int Length = pPositionDetailsList->GetItemCount();
	CArray<CString, CString> ItemData;
	ItemData.SetSize(Length);
	for (int i = 0; i < Length; i++)
	{
		ItemData[i] = pPositionDetailsList->GetItemText(i, pNMLV->iSubItem);
		pPositionDetailsList->SetItemData(i, (DWORD_PTR)&ItemData[i]);//设置排序关键字
	}
	static int sort = 0;
	static int SubItem = 0;
	if (SubItem != pNMLV->iSubItem)
	{
		sort = 0;
		SubItem = pNMLV->iSubItem;
	}
	else
	{
		if (sort == 0)
		{
			sort = 1;
		}
		else
		{
			sort = 0;
		}
	}
	pPositionDetailsList->SortItems(MyPosiMgrCompareProc, (DWORD_PTR)&sort);//排序
	*pResult = 0;
}