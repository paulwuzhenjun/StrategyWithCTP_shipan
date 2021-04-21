// InstMgrDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "InstMgrDlg.h"
#include "afxdialogex.h"
#include <map>
#include "MyStruct.h"

using namespace std;

// InstMgrDlg �Ի���
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

// InstMgrDlg ��Ϣ�������
BOOL InstMgrDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	//ί�е��б�
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
	column.pszText = _T("����������");
	column.mask = LVCF_TEXT;
	pInstsSubscribedList->InsertColumn(0, &column); //����һ�У�������Ϊ0
	//m_list->SetColumnWidth(0,100); //�п�����
	column.pszText = _T("Ʒ�ִ���");
	column.mask = LVCF_TEXT;
	pInstsSubscribedList->InsertColumn(1, &column);

	column.pszText = _T("��Լ����");
	column.mask = LVCF_TEXT;
	pInstsSubscribedList->InsertColumn(2, &column);

	column.pszText = _T("��С�䶯��λ");
	column.mask = LVCF_TEXT;
	pInstsSubscribedList->InsertColumn(3, &column);

	CRect rect4;
	pInstsSubscribedList->GetClientRect(rect4);                     //��õ�ǰ�ͻ�����Ϣ
	pInstsSubscribedList->SetColumnWidth(0, rect4.Width() / 4);        //�����еĿ�ȡ�
	pInstsSubscribedList->SetColumnWidth(1, rect4.Width() / 4);
	pInstsSubscribedList->SetColumnWidth(2, rect4.Width() / 4);
	pInstsSubscribedList->SetColumnWidth(3, rect4.Width() / 4);

	map<string, InstrumentInfo>::iterator iter;//����һ������ָ��iter
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