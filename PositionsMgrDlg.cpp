// PositionsMgrDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "PositionsMgrDlg.h"
#include "afxdialogex.h"
#include "MyThread.h"

// PositionsMgrDlg �Ի���
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
	//�ֲ���ϸ�б�
	pPositionDetailsList = (CListCtrl*)GetDlgItem(IDC_POSITIONS_LIST);
	pPositionDetailsList->DeleteAllItems();
	pPositionDetailsList->ModifyStyle(0, LVS_REPORT);
	//	int  k=m_list->GetHeaderCtrl()->GetItemCount();
	//	for(int m=0;m <=k;m++)
	//	{
	//		m_list->DeleteColumn(0);
	//	}
	LV_COLUMN column1;
	column1.pszText = _T("Ʒ��ID");
	column1.mask = LVCF_TEXT;
	pPositionDetailsList->InsertColumn(0, &column1);

	column1.pszText = _T("��ԼID");
	column1.mask = LVCF_TEXT;
	pPositionDetailsList->InsertColumn(1, &column1); //����һ�У�������Ϊ0
	//m_list->SetColumnWidth(0,100); //�п�����

	column1.pszText = _T("����");
	column1.mask = LVCF_TEXT;
	pPositionDetailsList->InsertColumn(2, &column1);

	column1.pszText = _T("����ʱ��");
	column1.mask = LVCF_TEXT;
	pPositionDetailsList->InsertColumn(3, &column1);

	column1.pszText = _T("����");
	column1.mask = LVCF_TEXT;
	pPositionDetailsList->InsertColumn(4, &column1);

	column1.pszText = _T("���ּ۸�");
	column1.mask = LVCF_TEXT;
	pPositionDetailsList->InsertColumn(5, &column1);

	column1.pszText = _T("��������");
	column1.mask = LVCF_TEXT;
	pPositionDetailsList->InsertColumn(6, &column1);

	CRect rect;
	pPositionDetailsList->GetClientRect(rect);                     //��õ�ǰ�ͻ�����Ϣ
	pPositionDetailsList->SetColumnWidth(0, rect.Width() / 8);        //�����еĿ�ȡ�
	pPositionDetailsList->SetColumnWidth(1, rect.Width() / 8);
	pPositionDetailsList->SetColumnWidth(2, rect.Width() / 8);
	pPositionDetailsList->SetColumnWidth(3, rect.Width() / 8);
	pPositionDetailsList->SetColumnWidth(4, rect.Width() / 8);
	pPositionDetailsList->SetColumnWidth(5, rect.Width() / 8);
	pPositionDetailsList->SetColumnWidth(6, 2 * rect.Width() / 8);

	return TRUE;
}
// PositionsMgrDlg ��Ϣ�������

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

	if (pPositionDetailsList != NULL) {
		CRect rect;
		pPositionDetailsList->GetClientRect(rect);                     //��õ�ǰ�ͻ�����Ϣ
		pPositionDetailsList->SetColumnWidth(0, rect.Width() / 8);        //�����еĿ�ȡ�
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (IDOK == AfxMessageBox(_T("�Ƿ��ѯ�ֲ�?"), MB_OKCANCEL, 0)) {
		pPositionDetailsList->DeleteAllItems();//��մ����б�

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
		pPositionDetailsList->SetItemData(i, (DWORD_PTR)&ItemData[i]);//��������ؼ���
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
	pPositionDetailsList->SortItems(MyPosiMgrCompareProc, (DWORD_PTR)&sort);//����
	*pResult = 0;
}