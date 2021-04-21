// LocalCLMgrDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "LocalCLMgrDlg.h"
#include "afxdialogex.h"
#include "MyStruct.h"
#include "OrderDataList.h"
#include "Strategy.h"
#include "StrategyKDJ.h"
#include "StrategyGridOpen.h"
#include "StrategyWaveOpen.h"
#include "StrategyWaveOpenAdd.h"
#include "StrategyBar.h"
#include "StrategyOpenPriceOpening.h"
#include "StrategyUpDownR.h"
#include "StrategyAvgLine.h"
#include "StrategyAvgDown.h"
#include "StrategyDT.h"
#include "StrategyBaseGridOpen.h"
#include "StrategyBaseGridOpen_plus.h"
#include "StrategyBaoChe.h"
#include "StrategyOpenPriceOpeningNew.h"
#include "StrategyOpenPriceOpeningAsia.h"
#include "StrategyBaseGridMAStopOpen.h"
#include "StrategyBaseGridOpenCffex.h"
#include "StrategyThreeK.h"
#include "StrategyOpenPriceOpeningNight.h"
#include "StrategyBaseGridMAStopOpenCffex.h"
#include "StrategyGridMAStopGTCChop.h"
// LocalCLMgrDlg 对话框
extern CListCtrl* pStraLocalCLList;
extern CString StrategyIDShowing;
extern CString ClassNameShowing;
extern list<ModelNode> ModelList;
extern OrderDataList OrderList;
extern HANDLE DispatchTdSem;
extern CStrategy* gStrategyImpl[];
extern int gStrategyImplIndex;

IMPLEMENT_DYNAMIC(LocalCLMgrDlg, CDialogEx)

static int CALLBACK MyLocalCLMgrCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
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

LocalCLMgrDlg::LocalCLMgrDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(LocalCLMgrDlg::IDD, pParent)
{
}

LocalCLMgrDlg::~LocalCLMgrDlg()
{
}

void LocalCLMgrDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(LocalCLMgrDlg, CDialogEx)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_QRY_STRA_LOCAL_CL_BTN, &LocalCLMgrDlg::OnBnClickedQryStraLocalClBtn)
	ON_EN_KILLFOCUS(IDC_EDIT1, &LocalCLMgrDlg::OnEnKillfocusEdit1)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_QRY_STRA_LOCAL_CL_BTN, &LocalCLMgrDlg::OnLvnColumnclickList1)
	ON_NOTIFY(NM_DBLCLK, IDC_STRATEGY_LOCAL_CL_LIST, &LocalCLMgrDlg::OnNMDblclkStrategyLocalClList)
	ON_BN_CLICKED(IDC_SAVE_CL_BTN, &LocalCLMgrDlg::OnBnClickedSaveClBtn)
END_MESSAGE_MAP()

BOOL LocalCLMgrDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	//持仓明细列表
	pStraLocalCLList = (CListCtrl*)GetDlgItem(IDC_STRATEGY_LOCAL_CL_LIST);
	pStraLocalCLList->DeleteAllItems();
	pStraLocalCLList->ModifyStyle(0, LVS_REPORT);
	pStraLocalCLList->SetExtendedStyle(pStraLocalCLList->GetExtendedStyle() | LVS_EX_FULLROWSELECT);
	//	int  k=m_list->GetHeaderCtrl()->GetItemCount();
	//	for(int m=0;m <=k;m++)
	//	{
	//		m_list->DeleteColumn(0);
	//	}
	LV_COLUMN column1;
	column1.pszText = _T("策略");
	column1.mask = LVCF_TEXT;
	pStraLocalCLList->InsertColumn(0, &column1); //插入一列，列索引为0
	//m_list->SetColumnWidth(0,100); //列宽设置

	column1.pszText = _T("实例");
	column1.mask = LVCF_TEXT;
	pStraLocalCLList->InsertColumn(1, &column1);

	column1.pszText = _T("品种合约");
	column1.mask = LVCF_TEXT;
	pStraLocalCLList->InsertColumn(2, &column1);

	column1.pszText = _T("序号");
	column1.mask = LVCF_TEXT;
	pStraLocalCLList->InsertColumn(3, &column1);

	column1.pszText = _T("OrderID");
	column1.mask = LVCF_TEXT;
	pStraLocalCLList->InsertColumn(4, &column1);

	column1.pszText = _T("开平");
	column1.mask = LVCF_TEXT;
	pStraLocalCLList->InsertColumn(5, &column1);

	column1.pszText = _T("方向");
	column1.mask = LVCF_TEXT;
	pStraLocalCLList->InsertColumn(6, &column1);

	column1.pszText = _T("开仓时间");
	column1.mask = LVCF_TEXT;
	pStraLocalCLList->InsertColumn(7, &column1);

	column1.pszText = _T("未成交手数");
	column1.mask = LVCF_TEXT;
	pStraLocalCLList->InsertColumn(8, &column1);

	column1.pszText = _T("委托价格");
	column1.mask = LVCF_TEXT;
	pStraLocalCLList->InsertColumn(9, &column1);

	column1.pszText = _T("开仓价");
	column1.mask = LVCF_TEXT;
	pStraLocalCLList->InsertColumn(10, &column1);

	column1.pszText = _T("手动止损价(非零)");
	column1.mask = LVCF_TEXT;
	pStraLocalCLList->InsertColumn(11, &column1);

	column1.pszText = _T("类型");
	column1.mask = LVCF_TEXT;
	pStraLocalCLList->InsertColumn(12, &column1);

	if (pStraLocalCLList != NULL) {
		CRect rect4;
		pStraLocalCLList->GetClientRect(rect4);                     //获得当前客户区信息
		int columnCount = pStraLocalCLList->GetHeaderCtrl()->GetItemCount();
		for (int i = 0; i < columnCount; i++) {
			//pParamsList->SetColumnWidth(i,rect4.Width()/columnCount);         //设置列的宽度。
			pStraLocalCLList->SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
		}
	}

	return TRUE;
}
// PositionsMgrDlg 消息处理程序

void LocalCLMgrDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	// TODO: Add your message handler code here
	if (nType == SIZE_RESTORED || nType == SIZE_MAXIMIZED)
	{
		ReSize();
	}
}

void LocalCLMgrDlg::ReSize()
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

	if (pStraLocalCLList != NULL) {
		CRect rect4;
		pStraLocalCLList->GetClientRect(rect4);                     //获得当前客户区信息
		int columnCount = pStraLocalCLList->GetHeaderCtrl()->GetItemCount();
		for (int i = 0; i < columnCount; i++) {
			//pParamsList->SetColumnWidth(i,rect4.Width()/columnCount);         //设置列的宽度。
			pStraLocalCLList->SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
		}
	}
}

// LocalCLMgrDlg 消息处理程序

void LocalCLMgrDlg::OnBnClickedQryStraLocalClBtn()
{
	// TODO: 在此添加控件通知处理程序代码
	if (IDOK == AfxMessageBox(_T("是否查询本地持仓?"), MB_OKCANCEL, 0)) {
		pStraLocalCLList->DeleteAllItems();//清空窗口列表
		//添加查询策略平仓单的动作信息到策略消息队列
		std::list<ModelNode>::iterator model_itr;
		for (model_itr = ModelList.begin(); model_itr != ModelList.end(); ++model_itr) {
			CString ModelName(model_itr->ModelName);
			std::list<StrategyNode>::iterator strategy_itr;
			for (strategy_itr = model_itr->StrategyList.begin(); strategy_itr != model_itr->StrategyList.end(); ++strategy_itr) {
				CString mClassName(strategy_itr->StrategyName);
				CString mStrategyID(strategy_itr->StrategyID);
				if (mClassName.CompareNoCase(ClassNameShowing) == 0 && mStrategyID.CompareNoCase(StrategyIDShowing) == 0) {
					std::list<StrategyInstanceNode>::iterator instance_itr;
					for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
						CString mInstanceName(instance_itr->InstanceName);
						if (instance_itr->StrategyStarted && instance_itr->shmindex > -1) {
							//添加查询策略平仓单的动作信息到策略消息队列
							OrderTradeMsg order;
							order.shmindex = instance_itr->shmindex;
							order.OrderType = ON_TD_DISPLAY_LOCAL_CL;
							OrderList.AddTail(order);
							ReleaseSemaphore(DispatchTdSem, 2, NULL);
						}
					}
				}
			}
		}
	}
}

void LocalCLMgrDlg::OnLvnColumnclickList1(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	int Length = pStraLocalCLList->GetItemCount();
	CArray<CString, CString> ItemData;
	ItemData.SetSize(Length);
	for (int i = 0; i < Length; i++)
	{
		ItemData[i] = pStraLocalCLList->GetItemText(i, pNMLV->iSubItem);
		pStraLocalCLList->SetItemData(i, (DWORD_PTR)&ItemData[i]);//设置排序关键字
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
	pStraLocalCLList->SortItems(MyLocalCLMgrCompareProc, (DWORD_PTR)&sort);//排序
	*pResult = 0;
}

CString LocalCLMgrDlg::GetItemText(
	HWND hWnd, int nItem, int nSubItem) const
{
	LVITEM lvi;
	memset(&lvi, 0, sizeof(LVITEM));
	lvi.iSubItem = nSubItem;
	CString str;
	int nLen = 128;
	int nRes;
	do
	{
		nLen *= 2;
		lvi.cchTextMax = nLen;
		lvi.pszText = str.GetBufferSetLength(nLen);
		nRes = (int)::SendMessage(hWnd,
			LVM_GETITEMTEXT, (WPARAM)nItem,
			(LPARAM)&lvi);
	} while (nRes == nLen - 1);
	str.ReleaseBuffer();
	return str;
}

// LocalCLMgrDlg 消息处理程序
void LocalCLMgrDlg::SetCell(HWND hWnd1,
	CString value, int nRow, int nCol)
{
	TCHAR     szString[256];
	wsprintf(szString, value, 0);

	//Fill the LVITEM structure with the
	//values given as parameters.
	LVITEM lvItem;
	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = nRow;
	lvItem.pszText = szString;
	lvItem.iSubItem = nCol;
	if (nCol > 0)
		//set the value of listItem
		::SendMessage(hWnd1, LVM_SETITEM,
		(WPARAM)0, (WPARAM)&lvItem);
	else
		//Insert the value into List
		ListView_InsertItem(hWnd1, &lvItem);
}

void LocalCLMgrDlg::OnEnKillfocusEdit1()
{
	// TODO: 在此添加控件通知处理程序代码
	CString tem;
	GetDlgItem(IDC_EDIT1)->GetWindowText(tem);    //得到用户输入的新的内容
	pStraLocalCLList->SetItemText(nItem, nSubItem, tem);  //设置编辑框的新内容
	GetDlgItem(IDC_EDIT1)->ShowWindow(SW_HIDE);   //隐藏编辑框
}

void LocalCLMgrDlg::OnNMDblclkStrategyLocalClList(NMHDR* pNMHDR, LRESULT* pResult)
{
	//LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	HWND hWnd1 = ::GetDlgItem(m_hWnd, IDC_STRATEGY_LOCAL_CL_LIST);
	NM_LISTVIEW* temp = (NM_LISTVIEW*)pNMHDR;
	CRect rc;
	if (temp->iItem != -1)
	{
		RECT rect;
		//get the row number
		nItem = temp->iItem;
		//get the column number
		nSubItem = temp->iSubItem;
		if (nSubItem < 5 || nItem == -1)
			return;

		//Retrieve the text of the selected subItem
		//from the list
		CString str = GetItemText(hWnd1, nItem,
			nSubItem);

		RECT rect1, rect2;
		// this macro is used to retrieve the Rectanle
		// of the selected SubItem
		ListView_GetSubItemRect(hWnd1, temp->iItem,
			temp->iSubItem, LVIR_BOUNDS, &rect);
		//Get the Rectange of the listControl
		::GetWindowRect(temp->hdr.hwndFrom, &rect1);
		//Get the Rectange of the Dialog
		::GetWindowRect(m_hWnd, &rect2);

		int x = rect1.left - rect2.left;
		int y = rect1.top - rect2.top;

		if (nItem != -1)
			::SetWindowPos(::GetDlgItem(m_hWnd, IDC_EDIT1),
				HWND_TOP, rect.left + x + 2, rect.top + y + 2,
				(rect.right - rect.left) / 5 * 2 + 60,//宽度
				rect.bottom - rect.top, NULL);
		::ShowWindow(::GetDlgItem(m_hWnd, IDC_EDIT1), SW_SHOW);
		::SetFocus(::GetDlgItem(m_hWnd, IDC_EDIT1));
		//Draw a Rectangle around the SubItem
		::Rectangle(::GetDC(temp->hdr.hwndFrom),
			rect.left, rect.top - 1, rect.right, rect.bottom);
		//Set the listItem text in the EditBox
		::SetWindowText(::GetDlgItem(m_hWnd, IDC_EDIT1), str);
	}
	*pResult = 0;
}

void LocalCLMgrDlg::OnBnClickedSaveClBtn()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strtemp;
	strtemp.Format(_T("确定更新在手报单信息?"));
	if (IDOK != AfxMessageBox(strtemp, MB_OKCANCEL, 0)) {
		return;
	}

	int lineCount = pStraLocalCLList->GetItemCount();
	CString strategyName("");
	for (int i = 0; i < lineCount; i++) {
		strategyName = pStraLocalCLList->GetItemText(i, 0);
		break;
	}

	for (int i = 0; i < gStrategyImplIndex; i++) {
		CString gStrategyName(gStrategyImpl[i]->mStrategyName);
		if (gStrategyImpl[i] != NULL && gStrategyImpl[i]->GetShmindex() >= 0 && gStrategyName.CompareNoCase(strategyName) == 0 && gStrategyImpl[i]->m_bIsRunning) {
			if (gStrategyName.CompareNoCase(_T("KDJ")) == 0) {
				StrategyKDJ* xCurrentStrategy = (StrategyKDJ*)gStrategyImpl[i];
				CString csInstanceName(xCurrentStrategy->mInstanceName);
				for (int i = 0; i < lineCount; i++) {
					CString csInstanceNameLC = pStraLocalCLList->GetItemText(i, 1);
					if (csInstanceNameLC.CompareNoCase(csInstanceName) == 0) {
						CString csSeqNo = pStraLocalCLList->GetItemText(i, 3);
						CString csManualStopPrice = pStraLocalCLList->GetItemText(i, 11);
						LocalCLForDisplayField mCLOrder;
						mCLOrder.CloseOrderSeqNo = _ttoi(csSeqNo);
						mCLOrder.ManualStopPrice = _tcstod(csManualStopPrice, 0);

						xCurrentStrategy->SetCloseLocalOrder(&mCLOrder);
					}
				}//end for all the CL Order Lines
			}
			else if (gStrategyName.CompareNoCase(_T("ThreeK")) == 0) {
				StrategyThreeK* xCurrentStrategy = (StrategyThreeK*)gStrategyImpl[i];
				CString csInstanceName(xCurrentStrategy->mInstanceName);
				for (int i = 0; i < lineCount; i++) {
					CString csInstanceNameLC = pStraLocalCLList->GetItemText(i, 1);
					if (csInstanceNameLC.CompareNoCase(csInstanceName) == 0) {
						CString csSeqNo = pStraLocalCLList->GetItemText(i, 3);
						CString csManualStopPrice = pStraLocalCLList->GetItemText(i, 11);
						LocalCLForDisplayField mCLOrder;
						mCLOrder.CloseOrderSeqNo = _ttoi(csSeqNo);
						mCLOrder.ManualStopPrice = _tcstod(csManualStopPrice, 0);

						xCurrentStrategy->SetCloseLocalOrder(&mCLOrder);
					}
				}//end for all the CL Order Lines
			}
			else if (gStrategyName.CompareNoCase(_T("OpenPriceOpening")) == 0) {
				StrategyOpenPriceOpening* xCurrentStrategy = (StrategyOpenPriceOpening*)gStrategyImpl[i];
				CString csInstanceName(xCurrentStrategy->mInstanceName);
				for (int i = 0; i < lineCount; i++) {
					CString csInstanceNameLC = pStraLocalCLList->GetItemText(i, 1);
					if (csInstanceNameLC.CompareNoCase(csInstanceName) == 0) {
						CString csSeqNo = pStraLocalCLList->GetItemText(i, 3);
						CString csManualStopPrice = pStraLocalCLList->GetItemText(i, 11);
						LocalCLForDisplayField mCLOrder;
						mCLOrder.CloseOrderSeqNo = _ttoi(csSeqNo);
						mCLOrder.ManualStopPrice = _tcstod(csManualStopPrice, 0);

						xCurrentStrategy->SetCloseLocalOrder(&mCLOrder);
					}
				}//end for all the CL Order Lines
			}
			else if (gStrategyName.CompareNoCase(_T("Bar")) == 0) {
				StrategyBar* xCurrentStrategy = (StrategyBar*)gStrategyImpl[i];
				CString csInstanceName(xCurrentStrategy->mInstanceName);
				for (int i = 0; i < lineCount; i++) {
					CString csInstanceNameLC = pStraLocalCLList->GetItemText(i, 1);
					if (csInstanceNameLC.CompareNoCase(csInstanceName) == 0) {
						CString csSeqNo = pStraLocalCLList->GetItemText(i, 3);
						CString csManualStopPrice = pStraLocalCLList->GetItemText(i, 11);
						LocalCLForDisplayField mCLOrder;
						mCLOrder.CloseOrderSeqNo = _ttoi(csSeqNo);
						mCLOrder.ManualStopPrice = _tcstod(csManualStopPrice, 0);

						xCurrentStrategy->SetCloseLocalOrder(&mCLOrder);
					}
				}//end for all the CL Order Lines
			}
			else if (gStrategyName.CompareNoCase(_T("GridOpen")) == 0) {
				StrategyGridOpen* xCurrentStrategy = (StrategyGridOpen*)gStrategyImpl[i];
				CString csInstanceName(xCurrentStrategy->mInstanceName);
				for (int i = 0; i < lineCount; i++) {
					CString csInstanceNameLC = pStraLocalCLList->GetItemText(i, 1);
					if (csInstanceNameLC.CompareNoCase(csInstanceName) == 0) {
						CString csSeqNo = pStraLocalCLList->GetItemText(i, 3);
						CString csManualStopPrice = pStraLocalCLList->GetItemText(i, 11);
						LocalCLForDisplayField mCLOrder;
						mCLOrder.CloseOrderSeqNo = _ttoi(csSeqNo);
						mCLOrder.ManualStopPrice = _tcstod(csManualStopPrice, 0);

						xCurrentStrategy->SetCloseLocalOrder(&mCLOrder);
					}
				}//end for all the CL Order Lines
			}
			else if (gStrategyName.CompareNoCase(_T("WaveOpen")) == 0) {
				StrategyWaveOpen* xCurrentStrategy = (StrategyWaveOpen*)gStrategyImpl[i];
				CString csInstanceName(xCurrentStrategy->mInstanceName);
				for (int i = 0; i < lineCount; i++) {
					CString csInstanceNameLC = pStraLocalCLList->GetItemText(i, 1);
					if (csInstanceNameLC.CompareNoCase(csInstanceName) == 0) {
						CString csSeqNo = pStraLocalCLList->GetItemText(i, 3);
						CString csManualStopPrice = pStraLocalCLList->GetItemText(i, 11);
						LocalCLForDisplayField mCLOrder;
						mCLOrder.CloseOrderSeqNo = _ttoi(csSeqNo);
						mCLOrder.ManualStopPrice = _tcstod(csManualStopPrice, 0);

						xCurrentStrategy->SetCloseLocalOrder(&mCLOrder);
					}
				}//end for all the CL Order Lines
			}
			else if (gStrategyName.CompareNoCase(_T("WaveOpenAdd")) == 0) {
				StrategyWaveOpenAdd* xCurrentStrategy = (StrategyWaveOpenAdd*)gStrategyImpl[i];
				CString csInstanceName(xCurrentStrategy->mInstanceName);
				for (int i = 0; i < lineCount; i++) {
					CString csInstanceNameLC = pStraLocalCLList->GetItemText(i, 1);
					if (csInstanceNameLC.CompareNoCase(csInstanceName) == 0) {
						CString csSeqNo = pStraLocalCLList->GetItemText(i, 3);
						CString csManualStopPrice = pStraLocalCLList->GetItemText(i, 11);
						LocalCLForDisplayField mCLOrder;
						mCLOrder.CloseOrderSeqNo = _ttoi(csSeqNo);
						mCLOrder.ManualStopPrice = _tcstod(csManualStopPrice, 0);

						xCurrentStrategy->SetCloseLocalOrder(&mCLOrder);
					}
				}//end for all the CL Order Lines
			}
			else if (gStrategyName.CompareNoCase(_T("UpDownR")) == 0) {
				StrategyUpDownR* xCurrentStrategy = (StrategyUpDownR*)gStrategyImpl[i];
				CString csInstanceName(xCurrentStrategy->mInstanceName);
				for (int i = 0; i < lineCount; i++) {
					CString csInstanceNameLC = pStraLocalCLList->GetItemText(i, 1);
					if (csInstanceNameLC.CompareNoCase(csInstanceName) == 0) {
						CString csSeqNo = pStraLocalCLList->GetItemText(i, 3);
						CString csManualStopPrice = pStraLocalCLList->GetItemText(i, 11);
						LocalCLForDisplayField mCLOrder;
						mCLOrder.CloseOrderSeqNo = _ttoi(csSeqNo);
						mCLOrder.ManualStopPrice = _tcstod(csManualStopPrice, 0);

						xCurrentStrategy->SetCloseLocalOrder(&mCLOrder);
					}
				}//end for all the CL Order Lines
			}
			else if (gStrategyName.CompareNoCase(_T("LastTTimeOpen")) == 0) {
				StrategyUpDownR* xCurrentStrategy = (StrategyUpDownR*)gStrategyImpl[i];
				CString csInstanceName(xCurrentStrategy->mInstanceName);
				for (int i = 0; i < lineCount; i++) {
					CString csInstanceNameLC = pStraLocalCLList->GetItemText(i, 1);
					if (csInstanceNameLC.CompareNoCase(csInstanceName) == 0) {
						CString csSeqNo = pStraLocalCLList->GetItemText(i, 3);
						CString csManualStopPrice = pStraLocalCLList->GetItemText(i, 11);
						LocalCLForDisplayField mCLOrder;
						mCLOrder.CloseOrderSeqNo = _ttoi(csSeqNo);
						mCLOrder.ManualStopPrice = _tcstod(csManualStopPrice, 0);

						xCurrentStrategy->SetCloseLocalOrder(&mCLOrder);
					}
				}//end for all the CL Order Lines
			}
			else if (gStrategyName.CompareNoCase(_T("AvgLine")) == 0) {
				StrategyAvgLine* xCurrentStrategy = (StrategyAvgLine*)gStrategyImpl[i];
				CString csInstanceName(xCurrentStrategy->mInstanceName);
				for (int i = 0; i < lineCount; i++) {
					CString csInstanceNameLC = pStraLocalCLList->GetItemText(i, 1);
					if (csInstanceNameLC.CompareNoCase(csInstanceName) == 0) {
						CString csSeqNo = pStraLocalCLList->GetItemText(i, 3);
						CString csManualStopPrice = pStraLocalCLList->GetItemText(i, 11);
						LocalCLForDisplayField mCLOrder;
						mCLOrder.CloseOrderSeqNo = _ttoi(csSeqNo);
						mCLOrder.ManualStopPrice = _tcstod(csManualStopPrice, 0);

						xCurrentStrategy->SetCloseLocalOrder(&mCLOrder);
					}
				}//end for all the CL Order Lines
			}
			else if (gStrategyName.CompareNoCase(_T("AvgDown")) == 0) {
				StrategyAvgDown* xCurrentStrategy = (StrategyAvgDown*)gStrategyImpl[i];
				CString csInstanceName(xCurrentStrategy->mInstanceName);
				for (int i = 0; i < lineCount; i++) {
					CString csInstanceNameLC = pStraLocalCLList->GetItemText(i, 1);
					if (csInstanceNameLC.CompareNoCase(csInstanceName) == 0) {
						CString csSeqNo = pStraLocalCLList->GetItemText(i, 3);
						CString csManualStopPrice = pStraLocalCLList->GetItemText(i, 11);
						LocalCLForDisplayField mCLOrder;
						mCLOrder.CloseOrderSeqNo = _ttoi(csSeqNo);
						mCLOrder.ManualStopPrice = _tcstod(csManualStopPrice, 0);

						xCurrentStrategy->SetCloseLocalOrder(&mCLOrder);
					}
				}
			}
			else if (gStrategyName.CompareNoCase(_T("DT")) == 0) {
				StrategyDT* xCurrentStrategy = (StrategyDT*)gStrategyImpl[i];
				CString csInstanceName(xCurrentStrategy->mInstanceName);
				for (int i = 0; i < lineCount; i++) {
					CString csInstanceNameLC = pStraLocalCLList->GetItemText(i, 1);
					if (csInstanceNameLC.CompareNoCase(csInstanceName) == 0) {
						CString csSeqNo = pStraLocalCLList->GetItemText(i, 3);
						CString csManualStopPrice = pStraLocalCLList->GetItemText(i, 11);
						LocalCLForDisplayField mCLOrder;
						mCLOrder.CloseOrderSeqNo = _ttoi(csSeqNo);
						mCLOrder.ManualStopPrice = _tcstod(csManualStopPrice, 0);

						xCurrentStrategy->SetCloseLocalOrder(&mCLOrder);
					}
				}
			}
			else if (gStrategyName.CompareNoCase(_T("BaseGridOpen")) == 0) {
				StrategyBaseGridOpen* xCurrentStrategy = (StrategyBaseGridOpen*)gStrategyImpl[i];
				CString csInstanceName(xCurrentStrategy->mInstanceName);
				for (int i = 0; i < lineCount; i++) {
					CString csInstanceNameLC = pStraLocalCLList->GetItemText(i, 1);
					if (csInstanceNameLC.CompareNoCase(csInstanceName) == 0) {
						CString csSeqNo = pStraLocalCLList->GetItemText(i, 3);
						CString csManualStopPrice = pStraLocalCLList->GetItemText(i, 11);
						LocalCLForDisplayField mCLOrder;
						mCLOrder.CloseOrderSeqNo = _ttoi(csSeqNo);
						mCLOrder.ManualStopPrice = _tcstod(csManualStopPrice, 0);

						xCurrentStrategy->SetCloseLocalOrder(&mCLOrder);
					}
				}
			}
			else if (gStrategyName.CompareNoCase(_T("BaseGridOpen_plus")) == 0) {
			StrategyBaseGridOpen_plus* xCurrentStrategy = (StrategyBaseGridOpen_plus*)gStrategyImpl[i];
			CString csInstanceName(xCurrentStrategy->mInstanceName);
			for (int i = 0; i < lineCount; i++) {
				CString csInstanceNameLC = pStraLocalCLList->GetItemText(i, 1);
				if (csInstanceNameLC.CompareNoCase(csInstanceName) == 0) {
					CString csSeqNo = pStraLocalCLList->GetItemText(i, 3);
					CString csManualStopPrice = pStraLocalCLList->GetItemText(i, 11);
					LocalCLForDisplayField mCLOrder;
					mCLOrder.CloseOrderSeqNo = _ttoi(csSeqNo);
					mCLOrder.ManualStopPrice = _tcstod(csManualStopPrice, 0);

					xCurrentStrategy->SetCloseLocalOrder(&mCLOrder);
				}
			}
			}
			else if (gStrategyName.CompareNoCase(_T("BaoChe")) == 0) {
				StrategyBaoChe* xCurrentStrategy = (StrategyBaoChe*)gStrategyImpl[i];
				CString csInstanceName(xCurrentStrategy->mInstanceName);
				for (int i = 0; i < lineCount; i++) {
					CString csInstanceNameLC = pStraLocalCLList->GetItemText(i, 1);
					if (csInstanceNameLC.CompareNoCase(csInstanceName) == 0) {
						CString csSeqNo = pStraLocalCLList->GetItemText(i, 3);
						CString csManualStopPrice = pStraLocalCLList->GetItemText(i, 11);
						LocalCLForDisplayField mCLOrder;
						mCLOrder.CloseOrderSeqNo = _ttoi(csSeqNo);
						mCLOrder.ManualStopPrice = _tcstod(csManualStopPrice, 0);

						xCurrentStrategy->SetCloseLocalOrder(&mCLOrder);
					}
				}
			}
			else if (gStrategyName.CompareNoCase(_T("OpenPriceOpeningNew")) == 0) {
				StrategyOpenPriceOpeningNew* xCurrentStrategy = (StrategyOpenPriceOpeningNew*)gStrategyImpl[i];
				CString csInstanceName(xCurrentStrategy->mInstanceName);
				for (int i = 0; i < lineCount; i++) {
					CString csInstanceNameLC = pStraLocalCLList->GetItemText(i, 1);
					if (csInstanceNameLC.CompareNoCase(csInstanceName) == 0) {
						CString csSeqNo = pStraLocalCLList->GetItemText(i, 3);
						CString csManualStopPrice = pStraLocalCLList->GetItemText(i, 11);
						LocalCLForDisplayField mCLOrder;
						mCLOrder.CloseOrderSeqNo = _ttoi(csSeqNo);
						mCLOrder.ManualStopPrice = _tcstod(csManualStopPrice, 0);

						xCurrentStrategy->SetCloseLocalOrder(&mCLOrder);
					}
				}
			}
			else if (gStrategyName.CompareNoCase(_T("OpenPriceOpeningAsia")) == 0) {
				StrategyOpenPriceOpeningAsia* xCurrentStrategy = (StrategyOpenPriceOpeningAsia*)gStrategyImpl[i];
				CString csInstanceName(xCurrentStrategy->mInstanceName);
				for (int i = 0; i < lineCount; i++) {
					CString csInstanceNameLC = pStraLocalCLList->GetItemText(i, 1);
					if (csInstanceNameLC.CompareNoCase(csInstanceName) == 0) {
						CString csSeqNo = pStraLocalCLList->GetItemText(i, 3);
						CString csManualStopPrice = pStraLocalCLList->GetItemText(i, 11);
						LocalCLForDisplayField mCLOrder;
						mCLOrder.CloseOrderSeqNo = _ttoi(csSeqNo);
						mCLOrder.ManualStopPrice = _tcstod(csManualStopPrice, 0);

						xCurrentStrategy->SetCloseLocalOrder(&mCLOrder);
					}
				}
			}
			else if (gStrategyName.CompareNoCase(_T("BaseGridMAStopOpen")) == 0) {
				StrategyBaseGridMAStopOpen* xCurrentStrategy = (StrategyBaseGridMAStopOpen*)gStrategyImpl[i];
				CString csInstanceName(xCurrentStrategy->mInstanceName);
				for (int i = 0; i < lineCount; i++) {
					CString csInstanceNameLC = pStraLocalCLList->GetItemText(i, 1);
					if (csInstanceNameLC.CompareNoCase(csInstanceName) == 0) {
						CString csSeqNo = pStraLocalCLList->GetItemText(i, 3);
						CString csManualStopPrice = pStraLocalCLList->GetItemText(i, 11);
						LocalCLForDisplayField mCLOrder;
						mCLOrder.CloseOrderSeqNo = _ttoi(csSeqNo);
						mCLOrder.ManualStopPrice = _tcstod(csManualStopPrice, 0);

						xCurrentStrategy->SetCloseLocalOrder(&mCLOrder);
					}
				}
			}
			else if (gStrategyName.CompareNoCase(_T("BaseGridOpenCffex")) == 0) {
				StrategyBaseGridOpenCffex* xCurrentStrategy = (StrategyBaseGridOpenCffex*)gStrategyImpl[i];
				CString csInstanceName(xCurrentStrategy->mInstanceName);
				for (int i = 0; i < lineCount; i++) {
					CString csInstanceNameLC = pStraLocalCLList->GetItemText(i, 1);
					if (csInstanceNameLC.CompareNoCase(csInstanceName) == 0) {
						CString csSeqNo = pStraLocalCLList->GetItemText(i, 3);
						CString csManualStopPrice = pStraLocalCLList->GetItemText(i, 11);
						LocalCLForDisplayField mCLOrder;
						mCLOrder.CloseOrderSeqNo = _ttoi(csSeqNo);
						mCLOrder.ManualStopPrice = _tcstod(csManualStopPrice, 0);

						xCurrentStrategy->SetCloseLocalOrder(&mCLOrder);
					}
				}
			}
			else if (gStrategyName.CompareNoCase(_T("BaseGridMAStopOpenCffex")) == 0) {
				StrategyBaseGridMAStopOpenCffex* xCurrentStrategy = (StrategyBaseGridMAStopOpenCffex*)gStrategyImpl[i];
				CString csInstanceName(xCurrentStrategy->mInstanceName);
				for (int i = 0; i < lineCount; i++) {
					CString csInstanceNameLC = pStraLocalCLList->GetItemText(i, 1);
					if (csInstanceNameLC.CompareNoCase(csInstanceName) == 0) {
						CString csSeqNo = pStraLocalCLList->GetItemText(i, 3);
						CString csManualStopPrice = pStraLocalCLList->GetItemText(i, 11);
						LocalCLForDisplayField mCLOrder;
						mCLOrder.CloseOrderSeqNo = _ttoi(csSeqNo);
						mCLOrder.ManualStopPrice = _tcstod(csManualStopPrice, 0);

						xCurrentStrategy->SetCloseLocalOrder(&mCLOrder);
					}
				}
			}
			else if (gStrategyName.CompareNoCase(_T("OpenPriceOpeningNight")) == 0) {
				StrategyOpenPriceOpeningNight* xCurrentStrategy = (StrategyOpenPriceOpeningNight*)gStrategyImpl[i];
				CString csInstanceName(xCurrentStrategy->mInstanceName);
				for (int i = 0; i < lineCount; i++) {
					CString csInstanceNameLC = pStraLocalCLList->GetItemText(i, 1);
					if (csInstanceNameLC.CompareNoCase(csInstanceName) == 0) {
						CString csSeqNo = pStraLocalCLList->GetItemText(i, 3);
						CString csManualStopPrice = pStraLocalCLList->GetItemText(i, 11);
						LocalCLForDisplayField mCLOrder;
						mCLOrder.CloseOrderSeqNo = _ttoi(csSeqNo);
						mCLOrder.ManualStopPrice = _tcstod(csManualStopPrice, 0);

						xCurrentStrategy->SetCloseLocalOrder(&mCLOrder);
					}
				}
			}
			else if (gStrategyName.CompareNoCase(_T("GridMAStopGTCChop")) == 0) {
				StrategyGridMAStopGTCChop* xCurrentStrategy = (StrategyGridMAStopGTCChop*)gStrategyImpl[i];
				CString csInstanceName(xCurrentStrategy->mInstanceName);
				for (int i = 0; i < lineCount; i++) {
					CString csInstanceNameLC = pStraLocalCLList->GetItemText(i, 1);
					if (csInstanceNameLC.CompareNoCase(csInstanceName) == 0) {
						CString csSeqNo = pStraLocalCLList->GetItemText(i, 3);
						CString csManualStopPrice = pStraLocalCLList->GetItemText(i, 11);
						LocalCLForDisplayField mCLOrder;
						mCLOrder.CloseOrderSeqNo = _ttoi(csSeqNo);
						mCLOrder.ManualStopPrice = _tcstod(csManualStopPrice, 0);

						xCurrentStrategy->SetCloseLocalOrder(&mCLOrder);
					}
				}
			}
		}//end if it is running
	}//end for strategy array
}