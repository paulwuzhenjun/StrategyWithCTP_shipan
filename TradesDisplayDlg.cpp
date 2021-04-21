// TradesDisplayDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "TradesDisplayDlg.h"
#include "afxdialogex.h"

extern CListCtrl* pTradesDetailsList;
// TradesDisplayDlg 对话框

IMPLEMENT_DYNAMIC(TradesDisplayDlg, CDialogEx)

TradesDisplayDlg::TradesDisplayDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(TradesDisplayDlg::IDD, pParent)
{
}

TradesDisplayDlg::~TradesDisplayDlg()
{
}

void TradesDisplayDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(TradesDisplayDlg, CDialogEx)
	ON_WM_SIZE()
END_MESSAGE_MAP()

// TradesDisplayDlg 消息处理程序
void TradesDisplayDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	// TODO: Add your message handler code here
	if (nType == SIZE_RESTORED || nType == SIZE_MAXIMIZED)
	{
		ReSize();
	}
}

BOOL TradesDisplayDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	//持仓明细列表
	pTradesDetailsList = (CListCtrl*)GetDlgItem(IDC_TRADE_LST);
	pTradesDetailsList->DeleteAllItems();
	pTradesDetailsList->ModifyStyle(0, LVS_REPORT);
	//	int  k=m_list->GetHeaderCtrl()->GetItemCount();
	//	for(int m=0;m <=k;m++)
	//	{
	//		m_list->DeleteColumn(0);
	//	}
	LV_COLUMN column1;
	column1.pszText = _T("模型");
	column1.mask = LVCF_TEXT;
	pTradesDetailsList->InsertColumn(0, &column1); //插入一列，列索引为0
	//m_list->SetColumnWidth(0,100); //列宽设置

	column1.pszText = _T("实例");
	column1.mask = LVCF_TEXT;
	pTradesDetailsList->InsertColumn(1, &column1);

	column1.pszText = _T("品种合约");
	column1.mask = LVCF_TEXT;
	pTradesDetailsList->InsertColumn(2, &column1);

	column1.pszText = _T("成交时间");
	column1.mask = LVCF_TEXT;
	pTradesDetailsList->InsertColumn(3, &column1);

	column1.pszText = _T("方向");
	column1.mask = LVCF_TEXT;
	pTradesDetailsList->InsertColumn(4, &column1);

	column1.pszText = _T("成交价格");
	column1.mask = LVCF_TEXT;
	pTradesDetailsList->InsertColumn(5, &column1);

	column1.pszText = _T("成交手数");
	column1.mask = LVCF_TEXT;
	pTradesDetailsList->InsertColumn(6, &column1);

	column1.pszText = _T("开平");
	column1.mask = LVCF_TEXT;
	pTradesDetailsList->InsertColumn(7, &column1);

	CRect rect;
	pTradesDetailsList->GetClientRect(rect);                     //获得当前客户区信息
	pTradesDetailsList->SetColumnWidth(0, 2 * rect.Width() / 9);        //设置列的宽度。
	pTradesDetailsList->SetColumnWidth(1, rect.Width() / 9);
	pTradesDetailsList->SetColumnWidth(2, rect.Width() / 9);
	pTradesDetailsList->SetColumnWidth(3, rect.Width() / 9);
	pTradesDetailsList->SetColumnWidth(4, rect.Width() / 9);
	pTradesDetailsList->SetColumnWidth(5, rect.Width() / 9);
	pTradesDetailsList->SetColumnWidth(6, rect.Width() / 9);
	pTradesDetailsList->SetColumnWidth(7, rect.Width() / 9);

	return TRUE;
}

void TradesDisplayDlg::ReSize()
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

	if (pTradesDetailsList != NULL) {
		CRect rect;
		pTradesDetailsList->GetClientRect(rect);                     //获得当前客户区信息
		pTradesDetailsList->SetColumnWidth(0, 2 * rect.Width() / 9);        //设置列的宽度。
		pTradesDetailsList->SetColumnWidth(1, rect.Width() / 9);
		pTradesDetailsList->SetColumnWidth(2, rect.Width() / 9);
		pTradesDetailsList->SetColumnWidth(3, rect.Width() / 9);
		pTradesDetailsList->SetColumnWidth(4, rect.Width() / 9);
		pTradesDetailsList->SetColumnWidth(5, rect.Width() / 9);
		pTradesDetailsList->SetColumnWidth(6, rect.Width() / 9);
		pTradesDetailsList->SetColumnWidth(7, rect.Width() / 9);
	}
}