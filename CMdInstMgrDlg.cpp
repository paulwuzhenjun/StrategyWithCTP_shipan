// CMdInstMgrDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "CMdInstMgrDlg.h"
#include "afxdialogex.h"
#include "MyStruct.h"
#include "MyThread.h"

extern list<InstrumentsName> OverSeaInstSubscribed;
extern list<InstrumentsName> DomesticInstSubscribed;
extern list<InstrumentsName> SGEInstSubscribed;

extern list<InstrumentsName> OverSeaInstSubscribedNew;
extern list<InstrumentsName> DomesticInstSubscribedNew;
extern list<InstrumentsName> SGEInstSubscribedNew;

extern CListBox* pMdInstList;
extern bool MDServerConnected;

extern CMyThread* pAddNewSubInst;

// CMdInstMgrDlg 对话框

IMPLEMENT_DYNAMIC(CMdInstMgrDlg, CDialogEx)

CMdInstMgrDlg::CMdInstMgrDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMdInstMgrDlg::IDD, pParent)
	, xxtest(0)
{
}

CMdInstMgrDlg::~CMdInstMgrDlg()
{
}

void CMdInstMgrDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SUB_LIST, m_sublist_listbox);
}

BEGIN_MESSAGE_MAP(CMdInstMgrDlg, CDialogEx)
	ON_NOTIFY(NM_CLICK, IDC_ALL_INST_LIST, &CMdInstMgrDlg::OnNMClickParamList)
	ON_EN_KILLFOCUS(IDC_EDIT1, &CMdInstMgrDlg::OnEnKillfocusEdit1)
	ON_BN_CLICKED(IDC_ADD_BTN, &CMdInstMgrDlg::OnBnClickedAddBtn)
	ON_BN_CLICKED(IDC_DELETE_BTN, &CMdInstMgrDlg::OnBnClickedDeleteBtn)
	ON_BN_CLICKED(IDC_SAVE_BTN, &CMdInstMgrDlg::OnBnClickedSaveBtn)
	ON_BN_CLICKED(IDCANCEL, &CMdInstMgrDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_ADDINTO_SUBLIST, &CMdInstMgrDlg::OnBnClickedAddintoSublist)
	ON_BN_CLICKED(IDC_REMOVEFROM_SUBLIST, &CMdInstMgrDlg::OnBnClickedRemovefromSublist)
	ON_BN_CLICKED(IDC_SUB_BTN, &CMdInstMgrDlg::OnBnClickedSubBtn)
END_MESSAGE_MAP()

// CMdInstMgrDlg 消息处理程序
BOOL CMdInstMgrDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	pInstsSubscribedList = (CListCtrl*)GetDlgItem(IDC_ALL_INST_LIST);
	pInstsSubscribedList->DeleteAllItems();
	pInstsSubscribedList->ModifyStyle(0, LVS_REPORT);
	pInstsSubscribedList->SetExtendedStyle(pInstsSubscribedList->GetExtendedStyle() | LVS_EX_FULLROWSELECT);
	pInstsSubscribedList->SetExtendedStyle(pInstsSubscribedList->GetExtendedStyle() | LVS_EX_CHECKBOXES);
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

	FILE* fp = fopen("InstrumentsID.ini", "r");
	if (fp == NULL) { TRACE("Error in Open ini file"); TRACE("%s \n", strerror(errno)); }
	InstrumentsName instRec;
	char* ptr;
	char achBuf[256] = { 0 };
	ptr = fgets(achBuf, 256, fp);
	while (ptr != NULL) {
		char* column[4], * p = achBuf;
		int i;
		for (i = 0; i < 4; i++) {
			column[i] = p;
			if ((p = strchr(p, ',')) == NULL)break;
			else *p++ = '\0';
		}
		strtok(column[3], "\n");
		strcpy(instRec.ExchangeID, column[0]);
		strcpy(instRec.CommodityNo, column[1]);
		strcpy(instRec.InstrumentID, column[2]);
		instRec.OneTick = atof(column[3]);
		//string strInst(instRec.ExchangeID);
		int orderIndex = pInstsSubscribedList->GetItemCount();
		CString itemname("");
		itemname.Format(_T("%d"), orderIndex);
		pInstsSubscribedList->InsertItem(orderIndex, (LPCTSTR)itemname);

		CString csExchangeID(instRec.ExchangeID);
		pInstsSubscribedList->SetItemText(orderIndex, 0, (LPCTSTR)csExchangeID);

		CString csCommodityNo(instRec.CommodityNo);
		pInstsSubscribedList->SetItemText(orderIndex, 1, (LPCTSTR)csCommodityNo);

		CString csInstrumentID(instRec.InstrumentID);
		pInstsSubscribedList->SetItemText(orderIndex, 2, (LPCTSTR)csInstrumentID);

		CString csOneTick("");
		csOneTick.Format(_T("%.5f"), instRec.OneTick);
		pInstsSubscribedList->SetItemText(orderIndex, 3, (LPCTSTR)csOneTick);
		pInstsSubscribedList->EnsureVisible(pInstsSubscribedList->GetItemCount() - 1, FALSE);

		ptr = fgets(achBuf, 256, fp);
	}
	fclose(fp);

	std::list<InstrumentsName>::iterator inst_itr;
	for (inst_itr = DomesticInstSubscribed.begin(); inst_itr != DomesticInstSubscribed.end(); ++inst_itr) {
		char cInstFullName[100];
		sprintf(cInstFullName, "%s,%s,%s,%.5f", inst_itr->ExchangeID, inst_itr->CommodityNo, inst_itr->InstrumentID, inst_itr->OneTick);
		CString csInstFullName(cInstFullName);
		m_sublist_listbox.AddString(csInstFullName);
	}

	for (inst_itr = SGEInstSubscribed.begin(); inst_itr != SGEInstSubscribed.end(); ++inst_itr) {
		char cInstFullName[100];
		sprintf(cInstFullName, "%s,%s,%s,%.5f", inst_itr->ExchangeID, inst_itr->CommodityNo, inst_itr->InstrumentID, inst_itr->OneTick);
		CString csInstFullName(cInstFullName);
		m_sublist_listbox.AddString(csInstFullName);
	}

	for (inst_itr = OverSeaInstSubscribed.begin(); inst_itr != OverSeaInstSubscribed.end(); ++inst_itr) {
		char cInstFullName[100];
		sprintf(cInstFullName, "%s,%s,%s,%.5f", inst_itr->ExchangeID, inst_itr->CommodityNo, inst_itr->InstrumentID, inst_itr->OneTick);
		CString csInstFullName(cInstFullName);
		m_sublist_listbox.AddString(csInstFullName);
	}

	//if(MDServerConnected){
		//程序处于连接服务器状态
	//	GetDlgItem(IDC_ADD_BTN)->EnableWindow(FALSE);
	//	GetDlgItem(IDC_DELETE_BTN)->EnableWindow(FALSE);
	//	GetDlgItem(IDC_SAVE_BTN)->EnableWindow(FALSE);
	//}
	return TRUE;
}

CString CMdInstMgrDlg::GetItemText(
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

void CMdInstMgrDlg::OnNMClickParamList(NMHDR* pNMHDR, LRESULT* pResult)
{
	DWORD dwPos = GetMessagePos();
	CPoint point(LOWORD(dwPos), HIWORD(dwPos));

	pInstsSubscribedList->ScreenToClient(&point);

	LVHITTESTINFO lvinfo;
	lvinfo.pt = point;
	lvinfo.flags = LVHT_ABOVE;

	UINT nFlag;
	int nItemRow = pInstsSubscribedList->HitTest(point, &nFlag);
	//判断是否点在checkbox上
	if (nFlag == LVHT_ONITEMSTATEICON)
	{
		HWND hWnd1 = ::GetDlgItem(m_hWnd, IDC_ALL_INST_LIST);
		NM_LISTVIEW* temp = (NM_LISTVIEW*)pNMHDR;
		nItem = temp->iItem;
		CString strInstanceName = GetItemText(hWnd1, nItem, 1);
		//AfxMessageBox(_T("点在listctrl的checkbox上"),MB_OK,0);
	}
	else {
		// *pResult = 0;
		Invalidate();
		HWND hWnd1 = ::GetDlgItem(m_hWnd, IDC_ALL_INST_LIST);
		NM_LISTVIEW* temp = (NM_LISTVIEW*)pNMHDR;
		RECT rect;
		//get the row number
		nItem = temp->iItem;
		//get the column number
		nSubItem = temp->iSubItem;

		if (nSubItem == -1 || nItem == -1)
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
				HWND_TOP, rect.left + x + 2, rect.top + y - 25,
				(rect.right - rect.left),//宽度
				rect.bottom - rect.top + 2, NULL);
		TRACE("%d\n", rect.top + y - 21);
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

void CMdInstMgrDlg::OnEnKillfocusEdit1()
{
	// TODO: 在此添加控件通知处理程序代码
	CString tem;
	GetDlgItem(IDC_EDIT1)->GetWindowText(tem);    //得到用户输入的新的内容
	pInstsSubscribedList->SetItemText(nItem, nSubItem, tem);  //设置编辑框的新内容
	GetDlgItem(IDC_EDIT1)->ShowWindow(SW_HIDE);   //隐藏编辑框
}

void CMdInstMgrDlg::OnBnClickedAddBtn()
{
	// TODO: 在此添加控件通知处理程序代码
	int orderIndex = pInstsSubscribedList->GetItemCount();
	CString itemname("");
	itemname.Format(_T("%d"), orderIndex);
	pInstsSubscribedList->InsertItem(orderIndex, (LPCTSTR)itemname);
}

void CMdInstMgrDlg::OnBnClickedDeleteBtn()
{
	// TODO: 在此添加控件通知处理程序代码
	int lineCount = pInstsSubscribedList->GetItemCount();
	if (lineCount == 1) {
		AfxMessageBox(_T("仅有一个合约,不能删除."), MB_OK, 0);
	}
	else {
		for (int i = 0; i < pInstsSubscribedList->GetItemCount(); i++) {
			if (pInstsSubscribedList->GetCheck(i) || pInstsSubscribedList->GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED) {
				//Check处于选中状态
				pInstsSubscribedList->DeleteItem(i);
				i--;
			}
		}
	}
}

void CMdInstMgrDlg::ConvertCStringToCharArray(CString csSource, char* rtnCharArray)
{
	int cslen = WideCharToMultiByte(CP_ACP, 0, csSource, csSource.GetLength(), NULL, 0, NULL, NULL);
	char* carray = new char[cslen + 1];
	WideCharToMultiByte(CP_ACP, 0, csSource, csSource.GetLength(), carray, cslen, NULL, NULL);
	carray[cslen] = '\0';
	strcpy(rtnCharArray, carray);
	delete carray;
}

void CMdInstMgrDlg::OnBnClickedSaveBtn()
{
	// TODO: 在此添加控件通知处理程序代码
	//OverSeaInstSubscribed.clear();
	//DomesticInstSubscribed.clear();
	//写入文件
	FILE* fp = fopen("InstrumentsID.ini", "w+");
	for (int i = 0; i < pInstsSubscribedList->GetItemCount(); i++) {
		CString csExchangeID = pInstsSubscribedList->GetItemText(i, 0);
		CString csCommodityNo = pInstsSubscribedList->GetItemText(i, 1);
		CString csInstrumentID = pInstsSubscribedList->GetItemText(i, 2);
		CString csOneTick = pInstsSubscribedList->GetItemText(i, 3);

		InstrumentsName InstRec;
		ConvertCStringToCharArray(csExchangeID, InstRec.ExchangeID);
		ConvertCStringToCharArray(csCommodityNo, InstRec.CommodityNo);
		ConvertCStringToCharArray(csInstrumentID, InstRec.InstrumentID);
		InstRec.OneTick = _tcstod(csOneTick, 0);

		/*
		if(strcmp(InstRec.ExchangeID,"CFFEX")==0||strcmp(InstRec.ExchangeID,"SHFE")==0
			||strcmp(InstRec.ExchangeID,"DCE")==0||strcmp(InstRec.ExchangeID,"CZCE")==0){
				DomesticInstSubscribed.push_back(InstRec);
		}else if(strcmp(InstRec.ExchangeID,"SGE")==0){
			SGEInstSubscribed.push_back(InstRec);
		}else OverSeaInstSubscribed.push_back(InstRec);
		*/

		char line[500];
		sprintf(line, "%s,%s,%s,%.5f\n", InstRec.ExchangeID, InstRec.CommodityNo, InstRec.InstrumentID, InstRec.OneTick);
		fwrite(line, strlen(line), 1, fp);
	}
	fclose(fp);
	/*
	pMdInstList->ResetContent();
	std::list<InstrumentsName>::iterator inst_itr;
	char InstFullName[80];
	for(inst_itr = DomesticInstSubscribed.begin(); inst_itr != DomesticInstSubscribed.end(); ++inst_itr){
		sprintf(InstFullName, "%s %s %s", inst_itr->ExchangeID,inst_itr->CommodityNo,inst_itr->InstrumentID);
		CString csInstFullName(InstFullName);
		pMdInstList->AddString(csInstFullName);
	}
	for(inst_itr = SGEInstSubscribed.begin(); inst_itr != SGEInstSubscribed.end(); ++inst_itr){
		sprintf(InstFullName, "%s %s %s", inst_itr->ExchangeID,inst_itr->CommodityNo,inst_itr->InstrumentID);
		CString csInstFullName(InstFullName);
		pMdInstList->AddString(csInstFullName);
	}

	for(inst_itr = OverSeaInstSubscribed.begin(); inst_itr != OverSeaInstSubscribed.end(); ++inst_itr){
		sprintf(InstFullName, "%s %s %s", inst_itr->ExchangeID,inst_itr->CommodityNo,inst_itr->InstrumentID);
		CString csInstFullName(InstFullName);
		pMdInstList->AddString(csInstFullName);
	}
	//写入文件
	FILE *fp=fopen("InstrumentsID.ini","w+");
	if(fp==NULL) {TRACE("Error in Open ini file");TRACE("%s\n", strerror(errno));}
	for(inst_itr = DomesticInstSubscribed.begin(); inst_itr != DomesticInstSubscribed.end(); ++inst_itr){
		char line[500];
		sprintf(line,"%s,%s,%s,%.5f\n",inst_itr->ExchangeID,inst_itr->CommodityNo,inst_itr->InstrumentID,inst_itr->OneTick);
		fwrite(line,strlen(line),1,fp);
	}
	for(inst_itr = SGEInstSubscribed.begin(); inst_itr != SGEInstSubscribed.end(); ++inst_itr){
		char line[500];
		sprintf(line,"%s,%s,%s,%.5f\n",inst_itr->ExchangeID,inst_itr->CommodityNo,inst_itr->InstrumentID,inst_itr->OneTick);
		fwrite(line,strlen(line),1,fp);
	}
	for(inst_itr = OverSeaInstSubscribed.begin(); inst_itr != OverSeaInstSubscribed.end(); ++inst_itr){
		char line[500];
		sprintf(line,"%s,%s,%s,%.5f\n",inst_itr->ExchangeID,inst_itr->CommodityNo,inst_itr->InstrumentID,inst_itr->OneTick);
		fwrite(line,strlen(line),1,fp);
	}
	fclose(fp);
	*/
}

void CMdInstMgrDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
}

void CMdInstMgrDlg::OnBnClickedAddintoSublist()
{
	// TODO: 在此添加控件通知处理程序代码
	for (int i = 0; i < pInstsSubscribedList->GetItemCount(); i++) {
		if (pInstsSubscribedList->GetCheck(i) || pInstsSubscribedList->GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED) {
			//选项处于选中状态
			CString csExchangeID = pInstsSubscribedList->GetItemText(i, 0);
			CString csCommodityNo = pInstsSubscribedList->GetItemText(i, 1);
			CString csInstrumentID = pInstsSubscribedList->GetItemText(i, 2);
			CString csOneTick = pInstsSubscribedList->GetItemText(i, 3);

			CString csInstFullName(csExchangeID);
			csInstFullName.Append(_T(","));
			csInstFullName.Append(csCommodityNo);
			csInstFullName.Append(_T(","));
			csInstFullName.Append(csInstrumentID);
			csInstFullName.Append(_T(","));
			csInstFullName.Append(csOneTick);

			m_sublist_listbox.AddString(csInstFullName);
			//从左侧删除
			pInstsSubscribedList->DeleteItem(i);
			i--;
		}
	}
}

void CMdInstMgrDlg::OnBnClickedRemovefromSublist()
{
	// TODO: 在此添加控件通知处理程序代码
	int nItemCount = m_sublist_listbox.GetSelCount();
	if (0 != nItemCount)
	{
		int* indexBuf = new int[nItemCount];
		memset(indexBuf, 0, nItemCount * sizeof(int));
		// 存储选中的条目索引
		m_sublist_listbox.GetSelItems(nItemCount, indexBuf);
		for (int loop = nItemCount - 1; loop >= 0; loop--)
		{
			CString csSelText;
			m_sublist_listbox.GetText(indexBuf[loop], csSelText);

			char cItemValue[100];
			ConvertCStringToCharArray(csSelText, cItemValue);

			char a[30] = { 0 }, b[30] = { 0 }, c[30] = { 0 }, d[30] = { 0 };       //这里要注意多留点空间以存放各子串的长度
			sscanf(cItemValue, "%[^,],%[^,],%[^,],%[^,]", a, b, c, d);

			InstrumentsName InstRec;
			strcpy(InstRec.ExchangeID, a);
			strcpy(InstRec.CommodityNo, b);
			strcpy(InstRec.InstrumentID, c);
			CString csTick(d);
			InstRec.OneTick = _tcstod(csTick, 0);

			int orderIndex = pInstsSubscribedList->GetItemCount();
			CString itemname("");
			itemname.Format(_T("%d"), orderIndex);
			pInstsSubscribedList->InsertItem(orderIndex, (LPCTSTR)itemname);

			CString csExchangeID(InstRec.ExchangeID);
			pInstsSubscribedList->SetItemText(orderIndex, 0, (LPCTSTR)csExchangeID);

			CString csCommodityNo(InstRec.CommodityNo);
			pInstsSubscribedList->SetItemText(orderIndex, 1, (LPCTSTR)csCommodityNo);

			CString csInstrumentID(InstRec.InstrumentID);
			pInstsSubscribedList->SetItemText(orderIndex, 2, (LPCTSTR)csInstrumentID);

			CString csOneTick("");
			csOneTick.Format(_T("%.5f"), InstRec.OneTick);
			pInstsSubscribedList->SetItemText(orderIndex, 3, (LPCTSTR)csOneTick);
			pInstsSubscribedList->EnsureVisible(pInstsSubscribedList->GetItemCount() - 1, FALSE);

			m_sublist_listbox.DeleteString(indexBuf[loop]);
		}
		delete[] indexBuf;
	}
}

void CMdInstMgrDlg::OnBnClickedSubBtn()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_sublist_listbox.GetCount() > 15) {
		AfxMessageBox(_T("最多只能订阅15个合约!"), MB_OK, 0);
		return;
	}

	pMdInstList->ResetContent();
	DomesticInstSubscribedNew.clear();
	SGEInstSubscribedNew.clear();
	OverSeaInstSubscribedNew.clear();

	if (!MDServerConnected) {
		DomesticInstSubscribed.clear();
		SGEInstSubscribed.clear();
		OverSeaInstSubscribed.clear();
	}

	int ItemCount = m_sublist_listbox.GetCount();
	for (int i = 0; i < ItemCount; i++) {
		CString csItem;
		m_sublist_listbox.GetText(i, csItem);

		char cItemValue[100];
		ConvertCStringToCharArray(csItem, cItemValue);

		char a[30] = { 0 }, b[30] = { 0 }, c[30] = { 0 }, d[30] = { 0 };       //这里要注意多留点空间以存放各子串的长度
		sscanf(cItemValue, "%[^,],%[^,],%[^,],%[^,]", a, b, c, d);

		if (MDServerConnected) {
			//处于连接服务器状态，处理新增情况
			InstrumentsName InstRec;
			strcpy(InstRec.ExchangeID, a);
			strcpy(InstRec.CommodityNo, b);
			strcpy(InstRec.InstrumentID, c);
			CString csTick(d);
			InstRec.OneTick = _tcstod(csTick, 0);

			if (strcmp(InstRec.ExchangeID, "CFFEX") == 0 || strcmp(InstRec.ExchangeID, "SHFE") == 0
				|| strcmp(InstRec.ExchangeID, "DCE") == 0 || strcmp(InstRec.ExchangeID, "CZCE") == 0
				|| strcmp(InstRec.ExchangeID, "INE") == 0) {
				DomesticInstSubscribedNew.push_back(InstRec);
			}
			else if (strcmp(InstRec.ExchangeID, "SGE") == 0) {
				SGEInstSubscribedNew.push_back(InstRec);
			}
			else OverSeaInstSubscribedNew.push_back(InstRec);
		}
		else {
			InstrumentsName InstRec;
			strcpy(InstRec.ExchangeID, a);
			strcpy(InstRec.CommodityNo, b);
			strcpy(InstRec.InstrumentID, c);
			CString csTick(d);
			InstRec.OneTick = _tcstod(csTick, 0);

			if (strcmp(InstRec.ExchangeID, "CFFEX") == 0 || strcmp(InstRec.ExchangeID, "SHFE") == 0
				|| strcmp(InstRec.ExchangeID, "DCE") == 0 || strcmp(InstRec.ExchangeID, "CZCE") == 0
				|| strcmp(InstRec.ExchangeID, "INE") == 0) {
				DomesticInstSubscribed.push_back(InstRec);
			}
			else if (strcmp(InstRec.ExchangeID, "SGE") == 0) {
				SGEInstSubscribed.push_back(InstRec);
			}
			else OverSeaInstSubscribed.push_back(InstRec);
		}

		char InstFullName[80];
		sprintf(InstFullName, "%s %s %s", a, b, c);
		CString csInstFullName(InstFullName);
		pMdInstList->AddString(csInstFullName);
	}

	if (MDServerConnected) {
		if (IDOK == AfxMessageBox(_T("确定订阅这些合约?"), MB_OKCANCEL, 0)) {
			pAddNewSubInst = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
			Sleep(500);
			pAddNewSubInst->PostThreadMessage(WM_ADD_NEW_SUBINST, NULL, NULL);

			FILE* fp = fopen("InstrumentsID.ini", "w+");
			for (int i = 0; i < pInstsSubscribedList->GetItemCount(); i++) {
				CString csExchangeID = pInstsSubscribedList->GetItemText(i, 0);
				CString csCommodityNo = pInstsSubscribedList->GetItemText(i, 1);
				CString csInstrumentID = pInstsSubscribedList->GetItemText(i, 2);
				CString csOneTick = pInstsSubscribedList->GetItemText(i, 3);

				InstrumentsName InstRec;
				ConvertCStringToCharArray(csExchangeID, InstRec.ExchangeID);
				ConvertCStringToCharArray(csCommodityNo, InstRec.CommodityNo);
				ConvertCStringToCharArray(csInstrumentID, InstRec.InstrumentID);
				InstRec.OneTick = _tcstod(csOneTick, 0);

				char line[500];
				sprintf(line, "%s,%s,%s,%.5f\n", InstRec.ExchangeID, InstRec.CommodityNo, InstRec.InstrumentID, InstRec.OneTick);
				fwrite(line, strlen(line), 1, fp);
			}
			fclose(fp);
		}
	}

	CDialogEx::OnOK();
}