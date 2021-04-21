// TradesDisplayDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "TradesDisplayDlg.h"
#include "afxdialogex.h"

extern CListCtrl* pTradesDetailsList;
// TradesDisplayDlg �Ի���

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

// TradesDisplayDlg ��Ϣ�������
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
	//�ֲ���ϸ�б�
	pTradesDetailsList = (CListCtrl*)GetDlgItem(IDC_TRADE_LST);
	pTradesDetailsList->DeleteAllItems();
	pTradesDetailsList->ModifyStyle(0, LVS_REPORT);
	//	int  k=m_list->GetHeaderCtrl()->GetItemCount();
	//	for(int m=0;m <=k;m++)
	//	{
	//		m_list->DeleteColumn(0);
	//	}
	LV_COLUMN column1;
	column1.pszText = _T("ģ��");
	column1.mask = LVCF_TEXT;
	pTradesDetailsList->InsertColumn(0, &column1); //����һ�У�������Ϊ0
	//m_list->SetColumnWidth(0,100); //�п�����

	column1.pszText = _T("ʵ��");
	column1.mask = LVCF_TEXT;
	pTradesDetailsList->InsertColumn(1, &column1);

	column1.pszText = _T("Ʒ�ֺ�Լ");
	column1.mask = LVCF_TEXT;
	pTradesDetailsList->InsertColumn(2, &column1);

	column1.pszText = _T("�ɽ�ʱ��");
	column1.mask = LVCF_TEXT;
	pTradesDetailsList->InsertColumn(3, &column1);

	column1.pszText = _T("����");
	column1.mask = LVCF_TEXT;
	pTradesDetailsList->InsertColumn(4, &column1);

	column1.pszText = _T("�ɽ��۸�");
	column1.mask = LVCF_TEXT;
	pTradesDetailsList->InsertColumn(5, &column1);

	column1.pszText = _T("�ɽ�����");
	column1.mask = LVCF_TEXT;
	pTradesDetailsList->InsertColumn(6, &column1);

	column1.pszText = _T("��ƽ");
	column1.mask = LVCF_TEXT;
	pTradesDetailsList->InsertColumn(7, &column1);

	CRect rect;
	pTradesDetailsList->GetClientRect(rect);                     //��õ�ǰ�ͻ�����Ϣ
	pTradesDetailsList->SetColumnWidth(0, 2 * rect.Width() / 9);        //�����еĿ�ȡ�
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
	POINT Newp; //��ȡ���ڶԻ���Ĵ�С
	CRect recta;
	GetClientRect(&recta);     //ȡ�ͻ�����С
	Newp.x = recta.right - recta.left;
	Newp.y = recta.bottom - recta.top;
	fsp[0] = (float)Newp.x / old.x;
	fsp[1] = (float)Newp.y / old.y;
	CRect Rect;
	int woc;
	CPoint OldTLPoint, TLPoint; //���Ͻ�
	CPoint OldBRPoint, BRPoint; //���½�
	HWND  hwndChild = ::GetWindow(m_hWnd, GW_CHILD);  //�г����пؼ�
	while (hwndChild)
	{
		woc = ::GetDlgCtrlID(hwndChild);//ȡ��ID
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
		pTradesDetailsList->GetClientRect(rect);                     //��õ�ǰ�ͻ�����Ϣ
		pTradesDetailsList->SetColumnWidth(0, 2 * rect.Width() / 9);        //�����еĿ�ȡ�
		pTradesDetailsList->SetColumnWidth(1, rect.Width() / 9);
		pTradesDetailsList->SetColumnWidth(2, rect.Width() / 9);
		pTradesDetailsList->SetColumnWidth(3, rect.Width() / 9);
		pTradesDetailsList->SetColumnWidth(4, rect.Width() / 9);
		pTradesDetailsList->SetColumnWidth(5, rect.Width() / 9);
		pTradesDetailsList->SetColumnWidth(6, rect.Width() / 9);
		pTradesDetailsList->SetColumnWidth(7, rect.Width() / 9);
	}
}