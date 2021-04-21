// OrdersMgrDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "OrdersMgrDlg.h"
#include "afxdialogex.h"
#include "MyThread.h"
#include "MyStruct.h"

// OrdersMgrDlg �Ի���
extern CListCtrl* pQryOrdersList;
extern CMyThread* pQryActionThread;
extern CListBox* pPubMsg;
extern bool g_bQryOrderSentByRecoverDlg;
extern OrderDetailField gOrderDelete;
extern list<InsertOrderField> ambushOrderList;
extern HANDLE ambushordersem;

IMPLEMENT_DYNAMIC(OrdersMgrDlg, CDialogEx)

static int CALLBACK MyOrderMgrCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
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

OrdersMgrDlg::OrdersMgrDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(OrdersMgrDlg::IDD, pParent)
{
}

OrdersMgrDlg::~OrdersMgrDlg()
{
}

void OrdersMgrDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(OrdersMgrDlg, CDialogEx)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_QRY_ORDERS_BTN, &OrdersMgrDlg::OnBnClickedQryOrdersBtn)
	ON_NOTIFY(NM_CLICK, IDC_ORDERS_LIST, &OrdersMgrDlg::OnNMClickOrdersList)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_ORDERS_LIST, &OrdersMgrDlg::OnLvnColumnclickList1)
END_MESSAGE_MAP()

BOOL OrdersMgrDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	//ί�е��б�
	pQryOrdersList = (CListCtrl*)GetDlgItem(IDC_ORDERS_LIST);
	pQryOrdersList->DeleteAllItems();
	pQryOrdersList->ModifyStyle(0, LVS_REPORT);
	//pQryOrdersList->ModifyStyle(0,LVS_SINGLESEL);
	//	int  k=m_list->GetHeaderCtrl()->GetItemCount();
	//	for(int m=0;m <=k;m++)
	//	{
	//		m_list->DeleteColumn(0);
	//	}
	LV_COLUMN column;
	column.pszText = _T("ί��ID");
	column.mask = LVCF_TEXT;
	pQryOrdersList->InsertColumn(0, &column); //����һ�У�������Ϊ0
	//m_list->SetColumnWidth(0,100); //�п�����
	column.pszText = _T("Ʒ��ID");
	column.mask = LVCF_TEXT;
	pQryOrdersList->InsertColumn(1, &column);

	column.pszText = _T("��ԼID");
	column.mask = LVCF_TEXT;
	pQryOrdersList->InsertColumn(2, &column);

	column.pszText = _T("�ύʱ��");
	column.mask = LVCF_TEXT;
	pQryOrdersList->InsertColumn(3, &column);

	column.pszText = _T("����");
	column.mask = LVCF_TEXT;
	pQryOrdersList->InsertColumn(4, &column);

	column.pszText = _T("����");
	column.mask = LVCF_TEXT;
	pQryOrdersList->InsertColumn(5, &column);

	column.pszText = _T("�ύ��");
	column.mask = LVCF_TEXT;
	pQryOrdersList->InsertColumn(6, &column);

	column.pszText = _T("״̬");
	column.mask = LVCF_TEXT;
	pQryOrdersList->InsertColumn(7, &column);

	column.pszText = _T("�ɽ���");
	column.mask = LVCF_TEXT;
	pQryOrdersList->InsertColumn(8, &column);

	column.pszText = _T("�ɽ���");
	column.mask = LVCF_TEXT;
	pQryOrdersList->InsertColumn(9, &column);

	column.pszText = _T("��������");
	column.mask = LVCF_TEXT;
	pQryOrdersList->InsertColumn(10, &column);

	column.pszText = _T("���ر��");
	column.mask = LVCF_TEXT;
	pQryOrdersList->InsertColumn(11, &column);
	column.pszText = _T("ǰ�ñ��");
	column.mask = LVCF_TEXT;
	pQryOrdersList->InsertColumn(12, &column);
	column.pszText = _T("�Ự��");
	column.mask = LVCF_TEXT;
	pQryOrdersList->InsertColumn(13, &column);

	CRect rect4;
	pQryOrdersList->GetClientRect(rect4);                     //��õ�ǰ�ͻ�����Ϣ
	pQryOrdersList->SetColumnWidth(0, rect4.Width() / 13);         //�����еĿ�ȡ�
	pQryOrdersList->SetColumnWidth(1, rect4.Width() / 13);
	pQryOrdersList->SetColumnWidth(2, rect4.Width() / 13); //�ύʱ��
	pQryOrdersList->SetColumnWidth(3, 2.0 * rect4.Width() / 13);
	pQryOrdersList->SetColumnWidth(4, rect4.Width() / 13);
	pQryOrdersList->SetColumnWidth(5, rect4.Width() / 13);
	pQryOrdersList->SetColumnWidth(6, rect4.Width() / 13);  //״̬
	pQryOrdersList->SetColumnWidth(7, rect4.Width() / 13);
	pQryOrdersList->SetColumnWidth(8, rect4.Width() / 13);
	pQryOrdersList->SetColumnWidth(9, rect4.Width() / 13);
	pQryOrdersList->SetColumnWidth(10, 2.0 * rect4.Width() / 13);

	return TRUE;
}

// OrdersMgrDlg ��Ϣ�������

void OrdersMgrDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	// TODO: Add your message handler code here
	if (nType == SIZE_RESTORED || nType == SIZE_MAXIMIZED)
	{
		ReSize();
	}
}

void OrdersMgrDlg::ReSize()
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

	if (pQryOrdersList != NULL) {
		CRect rect4;
		pQryOrdersList->GetClientRect(rect4);                     //��õ�ǰ�ͻ�����Ϣ
		pQryOrdersList->SetColumnWidth(0, rect4.Width() / 13);         //�����еĿ�ȡ�
		pQryOrdersList->SetColumnWidth(1, rect4.Width() / 13);
		pQryOrdersList->SetColumnWidth(2, rect4.Width() / 13);
		pQryOrdersList->SetColumnWidth(3, 2.0 * rect4.Width() / 13); //�ύʱ��
		pQryOrdersList->SetColumnWidth(4, rect4.Width() / 13);
		pQryOrdersList->SetColumnWidth(5, rect4.Width() / 13);
		pQryOrdersList->SetColumnWidth(6, rect4.Width() / 13);
		pQryOrdersList->SetColumnWidth(7, rect4.Width() / 13);  //״̬
		pQryOrdersList->SetColumnWidth(8, rect4.Width() / 13);
		pQryOrdersList->SetColumnWidth(9, rect4.Width() / 13);
		pQryOrdersList->SetColumnWidth(10, 2.0 * rect4.Width() / 13);
	}
}

void OrdersMgrDlg::OnBnClickedQryOrdersBtn()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//TRACE("Qry Orders Info");
	if (IDOK == AfxMessageBox(_T("�Ƿ��ѯί��?"), MB_OKCANCEL, 0)) {
		pQryOrdersList->DeleteAllItems();//��մ����б�
		g_bQryOrderSentByRecoverDlg = false;
		pQryActionThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
		Sleep(500);
		pQryActionThread->PostThreadMessage(WM_QRY_ORDERS, NULL, NULL);
	}

	//TRACE("Qry Orders Info");
}

void OrdersMgrDlg::OnNMClickOrdersList(NMHDR* pNMHDR, LRESULT* pResult)
{
	//LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if (pNMListView->iItem != -1)
	{
		//ֵ��0��ʼ����,������������Ӧ���
		//��Ӧ����
		CString str;
		str = pQryOrdersList->GetItemText(pNMListView->iItem, 0);

		CString csCommodityNo = pQryOrdersList->GetItemText(pNMListView->iItem, 1);
		CString csInstrumentID = pQryOrdersList->GetItemText(pNMListView->iItem, 2);
		CString csOrderLocalRef = pQryOrdersList->GetItemText(pNMListView->iItem, 11);
		CString csFrontID = pQryOrdersList->GetItemText(pNMListView->iItem, 12);
		CString csSessionID = pQryOrdersList->GetItemText(pNMListView->iItem, 13);

		int EsunApiOrNot = 1;//1 - EsunApi, 0 - ThostApi , 3 - SgitApi
		if (csCommodityNo.CompareNoCase(csInstrumentID) == 0) {
			if (csCommodityNo.CompareNoCase(_T("Au(T+D)")) == 0 || csCommodityNo.CompareNoCase(_T("Ag(T+D)")) == 0) {
				EsunApiOrNot = 3;
			}
			else {
				EsunApiOrNot = 0;
			}
		}

		CString strtemp;
		strtemp.Format(_T("������%d��"), pNMListView->iItem); // pNMListView->iSubItem
		strtemp.Append(_T(",OrderID="));
		strtemp.Append(str);
		if (EsunApiOrNot == 1) {
			strtemp.Append(_T(",����"));
		}
		else {
			strtemp.Append(_T(",����"));
		}
		CString strstatus;
		USES_CONVERSION;
		strstatus = pQryOrdersList->GetItemText(pNMListView->iItem, 7);
		csInstrumentID = pQryOrdersList->GetItemText(pNMListView->iItem, 2);
		char* p = T2A(strstatus.GetBuffer(0));
		char* p2 = T2A(csInstrumentID.GetBuffer(0));
		strstatus.ReleaseBuffer();
		csInstrumentID.ReleaseBuffer();

		if (strcmp(p, "���Ŷ�") == 0 || strcmp(p, "���ֳɽ�") == 0) {
			if (IDOK == AfxMessageBox(strtemp, MB_OKCANCEL, 0)) {
				memset(&gOrderDelete, 0, sizeof(sizeof(gOrderDelete)));

				pQryActionThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
				Sleep(500);

				USES_CONVERSION;
				//int orderRef=_wtoi(str.GetBuffer(0));
				//int sessionid=_wtoi(str2.GetBuffer(0));
				//int pQryOrderListIndex=pNMListView->iItem;

				int OrderID = _wtoi(str.GetBuffer(0));
				gOrderDelete.OrderId = OrderID;
				gOrderDelete.OrderLocalRef = _wtoi(csOrderLocalRef.GetBuffer(0));
				gOrderDelete.FrontID = _wtoi(csFrontID.GetBuffer(0));
				gOrderDelete.SessionID = _wtoi(csSessionID.GetBuffer(0));
				strcpy(gOrderDelete.InstrumentID, p2);

				pQryActionThread->PostThreadMessage(WM_WITHDRAW_ORDER, (WPARAM)EsunApiOrNot, (LPARAM)&gOrderDelete);

				char data[500];
				sprintf_s(data, "��������,ί�к�=%d", OrderID);
				CString str(data);
				pPubMsg->AddString(str);
			}
		}
		else if (strcmp(p, "Ԥ��") == 0) {
			if (IDOK == AfxMessageBox(strtemp, MB_OKCANCEL, 0)) {
				int wantdeleteid = pNMListView->iItem;
				int ambushindex = -1;
				std::list<InsertOrderField>::iterator insertorder_it;
				WaitForSingleObject(ambushordersem, INFINITE);
				if (!ambushOrderList.empty()) {
					for (insertorder_it = ambushOrderList.begin(); insertorder_it != ambushOrderList.end();) {
						std::list<InsertOrderField>::iterator iter_insert = insertorder_it++;
						ambushindex++;
						if (wantdeleteid == ambushindex) {
							ambushOrderList.erase(iter_insert);
							break;
						}
					}
				}
				ReleaseSemaphore(ambushordersem, 1, NULL);
			}
		}
		else {
			CString strtemp;
			strtemp.Format(_T("�����޷�����"));
			AfxMessageBox(strtemp, MB_OK, 0);
		}
	}
	else {
		CString strtemp;
		strtemp.Format(_T("��ѡ�ж�Ӧ�Ŀɳ�������ί��ID"));
		AfxMessageBox(strtemp, MB_OK, 0);
	}
}

void OrdersMgrDlg::OnLvnColumnclickList1(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	int Length = pQryOrdersList->GetItemCount();
	CArray<CString, CString> ItemData;
	ItemData.SetSize(Length);
	for (int i = 0; i < Length; i++)
	{
		ItemData[i] = pQryOrdersList->GetItemText(i, pNMLV->iSubItem);
		pQryOrdersList->SetItemData(i, (DWORD_PTR)&ItemData[i]);//��������ؼ���
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
	pQryOrdersList->SortItems(MyOrderMgrCompareProc, (DWORD_PTR)&sort);//����
	*pResult = 0;
}