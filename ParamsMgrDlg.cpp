// ParamsMgrDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ParamsMgrDlg.h"
#include "afxdialogex.h"
#include "MyStruct.h"
#include "Strategy.h"
#include "StrategyKDJ.h"
#include "StrategyBandMomnt.h"
#include "StrategyGridOpen.h"
#include "StrategyWaveOpen.h"
#include "StrategyWaveOpenAdd.h"
#include <map>
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
#include "StrategyBaseGridMAStopOpenCffex.h"
#include "StrategyThreeK.h"
#include "StrategyOpenPriceOpeningNight.h"
#include "StrategyGridMAStopGTCChop.h"
using namespace std;

extern list<ParamNode> paramslist;
extern list<ModelNode> ModelList;
extern CListCtrl* pParamsList;
extern CStrategy* gStrategyImpl[MAX_RUNNING_STRATEGY];
extern int gStrategyImplIndex;
extern CString StrategyIDShowing;
extern CString ClassNameShowing;
extern bool ParamsChangedOrNot;
// ParamsMgrDlg 对话框

IMPLEMENT_DYNAMIC(ParamsMgrDlg, CDialogEx)

ParamsMgrDlg::ParamsMgrDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(ParamsMgrDlg::IDD, pParent)
{
}

ParamsMgrDlg::~ParamsMgrDlg()
{
}

void ParamsMgrDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_PARAM_LIST, pParamsList);
}

BEGIN_MESSAGE_MAP(ParamsMgrDlg, CDialogEx)
	ON_WM_SIZE()
	//ON_NOTIFY(LVN_ITEMCHANGED, IDC_PARAM_LIST, &ParamsMgrDlg::OnItemchangedLinksList)
	ON_NOTIFY(NM_CLICK, IDC_PARAM_LIST, &ParamsMgrDlg::OnNMClickParamList)
	ON_EN_KILLFOCUS(IDC_EDIT1, &ParamsMgrDlg::OnEnKillfocusEdit1)
	ON_BN_CLICKED(IDC_SAVE_BTN, &ParamsMgrDlg::OnBnClickedSaveBtn)
	ON_BN_CLICKED(IDC_ADD_INSTANCE_BTN, &ParamsMgrDlg::OnBnClickedAddInstanceBtn)
	ON_BN_CLICKED(IDC_DELETE_INSTANCE_BTN, &ParamsMgrDlg::OnBnClickedDeleteInstanceBtn)
END_MESSAGE_MAP()

BOOL ParamsMgrDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	pParamsList = (CListCtrl*)GetDlgItem(IDC_PARAM_LIST);

	pParamsList->DeleteAllItems();
	pParamsList->ModifyStyle(0, LVS_REPORT);
	pParamsList->SetExtendedStyle(pParamsList->GetExtendedStyle() | LVS_EX_FULLROWSELECT);
	pParamsList->SetExtendedStyle(pParamsList->GetExtendedStyle() | LVS_EX_CHECKBOXES);

	return TRUE;
}
// ParamsMgrDlg 消息处理程序

void ParamsMgrDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	// TODO: Add your message handler code here
	if (nType == SIZE_RESTORED || nType == SIZE_MAXIMIZED)
	{
		ReSize();
	}
}

void ParamsMgrDlg::ReSize()
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

	if (pParamsList != NULL) {
		CRect rect4;
		pParamsList->GetClientRect(rect4);                     //获得当前客户区信息
		int columnCount = pParamsList->GetHeaderCtrl()->GetItemCount();
		for (int i = 0; i < columnCount; i++) {
			//pParamsList->SetColumnWidth(i,rect4.Width()/columnCount);         //设置列的宽度。
			pParamsList->SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
		}
	}
}

void ParamsMgrDlg::AddParamList(CString xStrategyID, StrategyNode mStrategyNode)
{
	pParamsList->DeleteAllItems();
	int  k = pParamsList->GetHeaderCtrl()->GetItemCount();
	for (int m = 0; m <= k; m++)
	{
		pParamsList->DeleteColumn(0);
	}
	/*
	if(StrategyStarted){
	char ParamName[30];
	char ParamChineseName[50];
	char ParamValue[30];

	int orderIndex=0;
	std::list<ParamNode>::iterator param_it;
	if(!gStrategyImpl[shmindex]->mParamslist.empty()){
	for(param_it = gStrategyImpl[shmindex]->mParamslist.begin(); param_it != gStrategyImpl[shmindex]->mParamslist.end(); ++param_it){
	strcpy(ParamName,param_it->ParamName);
	strcpy(ParamChineseName,param_it->ParamChineseName);
	strcpy(ParamValue,param_it->ParamValue);
	//ParamValue=param_it->ParamValue;

	CString itemname("");
	itemname.Format(_T("%d"),orderIndex);
	pParamsList->InsertItem(orderIndex,(LPCTSTR)itemname);

	CString orderId("");
	orderId.Format(_T("%d"),orderIndex+1);
	pParamsList->SetItemText(orderIndex,0,(LPCTSTR)orderId);

	CString paramname(ParamName);
	pParamsList->SetItemText(orderIndex,1,(LPCTSTR)paramname);

	CString paramchnname(ParamChineseName);
	pParamsList->SetItemText(orderIndex,2,(LPCTSTR)paramchnname);

	CString paramvalue(ParamValue);
	//paramvalue.Format(_T("%.5f"),ParamValue);
	pParamsList->SetItemText(orderIndex,3,(LPCTSTR)paramvalue);

	orderIndex++;
	}
	}

	pParamsList->EnableWindow(FALSE);
	}else{
	*/
	/*
	pParamsList->EnableWindow(TRUE);
	std::list<ModelNode>::iterator model_itr;
	if(!ModelList.empty()){
	for(model_itr = ModelList.begin(); model_itr != ModelList.end(); ++model_itr){
	CString ModelName(model_itr->ModelName);
	std::list<StrategyNode>::iterator strategy_itr;
	if(!model_itr->StrategyList.empty()){
	for(strategy_itr = model_itr->StrategyList.begin(); strategy_itr != model_itr->StrategyList.end(); ++strategy_itr){
	CString mStrategyName(strategy_itr->StrategyName);
	if(mStrategyName.CompareNoCase(StrategyName)==0){
	char ParamName[30];
	char ParamChineseName[50];
	char ParamValue[30];

	int orderIndex=0;
	std::list<ParamNode>::iterator param_it;
	if(!strategy_itr->ParamList.empty()){
	for(param_it = strategy_itr->ParamList.begin(); param_it != strategy_itr->ParamList.end(); ++param_it){
	strcpy(ParamName,param_it->ParamName);
	strcpy(ParamChineseName,param_it->ParamChineseName);
	strcpy(ParamValue,param_it->ParamValue);
	//ParamValue=param_it->ParamValue;

	CString itemname("");
	itemname.Format(_T("%d"),orderIndex);
	pParamsList->InsertItem(orderIndex,(LPCTSTR)itemname);

	CString orderId("");
	orderId.Format(_T("%d"),orderIndex+1);
	pParamsList->SetItemText(orderIndex,0,(LPCTSTR)orderId);

	CString paramname(ParamName);
	pParamsList->SetItemText(orderIndex,1,(LPCTSTR)paramname);

	CString paramchnname(ParamChineseName);
	pParamsList->SetItemText(orderIndex,2,(LPCTSTR)paramchnname);

	CString paramvalue(ParamValue);
	//paramvalue.Format(_T("%.5f"),ParamValue);
	pParamsList->SetItemText(orderIndex,3,(LPCTSTR)paramvalue);

	orderIndex++;
	}
	}
	}
	}
	}
	}
	//}
	}
	*/
	//产生表头
	std::list<string>::iterator paramname_itr;
	int columnid = 0;

	LV_COLUMN column;

	column.pszText = _T("状态");
	column.mask = LVCF_TEXT;
	pParamsList->InsertColumn(columnid, &column); //插入一列，列索引为0
	columnid++;

	column.pszText = _T("实例名");
	column.mask = LVCF_TEXT;
	pParamsList->InsertColumn(columnid, &column); //插入一列，列索引为0
	columnid++;

	for (paramname_itr = mStrategyNode.ParamCHNNameList.begin(); paramname_itr != mStrategyNode.ParamCHNNameList.end(); ++paramname_itr) {
		wstring widstr;
		widstr = s2ws((*paramname_itr));//std::wstring((*paramname_itr).begin(), (*paramname_itr).end());
		//LV_COLUMN column;
		column.pszText = (LPWSTR)widstr.c_str();//_T("ID");
		column.mask = LVCF_TEXT;
		pParamsList->InsertColumn(columnid, &column); //插入一列，列索引为0
		columnid++;
		/*
		CRect rect4;
		pParamsList->GetClientRect(rect4);                     //获得当前客户区信息
		pParamsList->SetColumnWidth(0,rect4.Width()/7);         //设置列的宽度。
		pParamsList->SetColumnWidth(1,2*rect4.Width()/7);
		pParamsList->SetColumnWidth(2,2*rect4.Width()/7);
		pParamsList->SetColumnWidth(3,2*rect4.Width()/7);
		*/
	}
	if (columnid > 0) {
		CRect rect4;
		pParamsList->GetClientRect(rect4);                     //获得当前客户区信息
		for (int i = 0; i < columnid; i++) {
			//pParamsList->SetColumnWidth(i,rect4.Width()/columnid);         //设置列的宽度。
			pParamsList->SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
		}
	}
	//插入各实例具体值
	int orderIndex = 0;
	std::list<StrategyInstanceNode>::iterator instance_itr;
	for (instance_itr = mStrategyNode.InstanceList.begin(); instance_itr != mStrategyNode.InstanceList.end(); ++instance_itr) {
		CString itemname("");
		itemname.Format(_T("%d"), orderIndex);
		pParamsList->InsertItem(orderIndex, (LPCTSTR)itemname);

		char ParamName[30];
		char ParamChineseName[50];
		char ParamValue[30];

		int paramindex = 0;

		string strInstanceStarted(InstanceStarted);
		wstring widstr;
		widstr = s2ws(strInstanceStarted);

		string strInstanceNotStarted(InstanceNotStarted);
		wstring widstr2;
		widstr2 = s2ws(InstanceNotStarted);

		if (instance_itr->StrategyStarted) {
			pParamsList->SetItemText(orderIndex, paramindex, (LPCTSTR)widstr.c_str());
			//pParamsList->SetCheck(orderIndex,TRUE);
			//pParamsList->SetItemState(orderIndex,LVIS_SELECTED,LVIS_SELECTED);
		}
		else {
			pParamsList->SetItemText(orderIndex, paramindex, (LPCTSTR)widstr2.c_str());
			//pParamsList->SetCheck(orderIndex,FALSE);
		}
		paramindex++;

		CString csInstanceName(instance_itr->InstanceName);
		pParamsList->SetItemText(orderIndex, paramindex, (LPCTSTR)csInstanceName);
		paramindex++;

		std::list<ParamNode>::iterator param_it;
		if (!instance_itr->ParamList.empty()) {
			for (param_it = instance_itr->ParamList.begin(); param_it != instance_itr->ParamList.end(); ++param_it) {
				strcpy(ParamName, param_it->ParamName);
				strcpy(ParamChineseName, param_it->ParamChineseName);
				strcpy(ParamValue, param_it->ParamValue);
				//ParamValue=param_it->ParamValue;

				CString paramvalue(ParamValue);
				pParamsList->SetItemText(orderIndex, paramindex, (LPCTSTR)paramvalue);
				paramindex++;
			}
		}
		orderIndex++;
	}
}

string ParamsMgrDlg::ws2s(const wstring& ws)
{
	string curLocale = setlocale(LC_ALL, NULL); // curLocale = "C";
	setlocale(LC_ALL, "chs");
	const wchar_t* _Source = ws.c_str();
	size_t _Dsize = 2 * ws.size() + 1;
	char* _Dest = new char[_Dsize];
	memset(_Dest, 0, _Dsize);
	wcstombs(_Dest, _Source, _Dsize);
	string result = _Dest;
	delete[]_Dest;
	setlocale(LC_ALL, curLocale.c_str());
	return result;
}

wstring ParamsMgrDlg::s2ws(const string& s)
{
	setlocale(LC_ALL, "chs");
	const char* _Source = s.c_str();
	size_t _Dsize = s.size() + 1;
	wchar_t* _Dest = new wchar_t[_Dsize];
	wmemset(_Dest, 0, _Dsize);
	mbstowcs(_Dest, _Source, _Dsize);
	wstring result = _Dest;
	delete[]_Dest;
	setlocale(LC_ALL, "C");
	return result;
}

void ParamsMgrDlg::SetSaveButtonDisable() {
	GetDlgItem(IDC_SAVE_BTN)->EnableWindow(FALSE);
}

void ParamsMgrDlg::SetSaveButtonEnable() {
	GetDlgItem(IDC_SAVE_BTN)->EnableWindow(TRUE);
}

void ParamsMgrDlg::SetCell(HWND hWnd1,
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

//this function will returns the item
//text depending on the item and SubItem Index
CString ParamsMgrDlg::GetItemText(
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

void ParamsMgrDlg::OnNMClickParamList(NMHDR* pNMHDR, LRESULT* pResult)
{
	DWORD dwPos = GetMessagePos();
	CPoint point(LOWORD(dwPos), HIWORD(dwPos));

	pParamsList->ScreenToClient(&point);

	LVHITTESTINFO lvinfo;
	lvinfo.pt = point;
	lvinfo.flags = LVHT_ABOVE;

	UINT nFlag;
	int nItemRow = pParamsList->HitTest(point, &nFlag);
	//判断是否点在checkbox上
	if (nFlag == LVHT_ONITEMSTATEICON)
	{
		HWND hWnd1 = ::GetDlgItem(m_hWnd, IDC_PARAM_LIST);
		NM_LISTVIEW* temp = (NM_LISTVIEW*)pNMHDR;
		nItem = temp->iItem;
		CString strInstanceName = GetItemText(hWnd1, nItem, 1);
		//AfxMessageBox(_T("点在listctrl的checkbox上"),MB_OK,0);

		/*
		std::list<ModelNode>::iterator model_itr;
		for(model_itr = ModelList.begin(); model_itr != ModelList.end(); ++model_itr){
		CString ModelName(model_itr->ModelName);
		std::list<StrategyNode>::iterator strategy_itr;
		for(strategy_itr = model_itr->StrategyList.begin(); strategy_itr != model_itr->StrategyList.end(); ++strategy_itr){
		CString mStrategyName(strategy_itr->StrategyName);
		if(mStrategyName.CompareNoCase(StrategyShowing)==0){
		std::list<StrategyInstanceNode>::iterator instance_itr;
		for(instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr){
		CString mInstanceName(instance_itr->InstanceName);
		if(mInstanceName.CompareNoCase(strInstanceName)==0){
		if(instance_itr->StrategyStarted){
		AfxMessageBox(_T("Instance Started."),MB_OK,0);
		pParamsList->SetItemState(nItem,LVIS_SELECTED,LVIS_SELECTED);
		}else{
		AfxMessageBox(_T("Instance not started."),MB_OK,0);
		}
		}
		}
		}
		}
		}
		*/
		//CString csInstanceStarted(InstanceStarted);
		//CString csInstanceNotStarted(InstanceNotStarted);

		//CString InstanceStatus=pParamsList->GetItemText(nItem,0);
		//if(csInstanceStarted.CompareNoCase(InstanceStatus)==0){
		//	AfxMessageBox(_T("实例处于启动状态"),MB_OK,0);
			//pParamsList->SetItemState(nItem,LVIS_SELECTED,LVIS_SELECTED);
			//pParamsList->SetCheck(nItem,TRUE);
		//}
	}
	else {
		// *pResult = 0;
		Invalidate();
		HWND hWnd1 = ::GetDlgItem(m_hWnd, IDC_PARAM_LIST);
		NM_LISTVIEW* temp = (NM_LISTVIEW*)pNMHDR;
		RECT rect;
		//get the row number
		nItem = temp->iItem;
		//get the column number
		nSubItem = temp->iSubItem;

		if (nSubItem == 0 || nSubItem == -1 || nItem == -1)
			return;
		/*
		CInstListDlg mInstListDlg;
		int columnCount=pParamsList->GetHeaderCtrl()->GetItemCount();
		for(int i=0;i<columnCount;i++){
			LVCOLUMN lvcol;
			int   nColNum;
			char  str[256];
			lvcol.mask = LVCF_TEXT;
			USES_CONVERSION;
			lvcol.pszText = A2T(str);
			lvcol.cchTextMax = 256;
			pParamsList->GetColumn(i, &lvcol);//从第三列开始，第一列是实例运行状态,第二列是实例名称

			CString header=lvcol.pszText;
			char cheader[50];
			ConvertCStringToCharArray(header,cheader);
			string strheader(cheader);
			mInstListDlg.paramnamelist.push_back(strheader);

			CString paraValue=pParamsList->GetItemText(nItem,i);
			char cParaValue[50];
			ConvertCStringToCharArray(paraValue,cParaValue);
			string strvalue(cParaValue);
			mInstListDlg.paramvaluelist.push_back(strvalue);
		}

		if (mInstListDlg.DoModal()!=IDOK)
		{
			ExitProcess(0);
		}
		*/
		//CString csInstFullName(mInstListDlg.InstFullName);
		//pParamsList->SetItemText(nItem,nSubItem,csInstFullName);

		LV_COLUMN lvColumn;
		const int MAX_HEADER_LEN = 256;
		TCHAR  lpBuffer[MAX_HEADER_LEN];
		lvColumn.pszText = lpBuffer;
		lvColumn.cchTextMax = 20;
		lvColumn.mask = LVCF_TEXT;
		pParamsList->GetColumn(nSubItem, &lvColumn);
		CString df = lvColumn.pszText;

		CString csInstanceStarted(InstanceStarted);
		CString InstanceStatus = pParamsList->GetItemText(nItem, 0);
		/*if(StrategyShowing.CompareNoCase(_T("KDJ"))==0||StrategyShowing.CompareNoCase(_T("Bar"))==0||StrategyShowing.CompareNoCase(_T("OpenPriceOpening"))==0||
			StrategyShowing.CompareNoCase(_T("BandMomnt"))==0||StrategyShowing.CompareNoCase(_T("GridOpen"))==0
			||StrategyShowing.CompareNoCase(_T("WaveOpen"))==0||StrategyShowing.CompareNoCase(_T("WaveOpenAdd"))==0
			||StrategyShowing.CompareNoCase(_T("UpDownR"))==0||StrategyShowing.CompareNoCase(_T("LastTTimeOpen"))==0
			||StrategyShowing.CompareNoCase(_T("AvgLine"))==0||StrategyShowing.CompareNoCase(_T("AvgDown"))==0
			||StrategyShowing.CompareNoCase(_T("BaseGridOpen"))==0||StrategyShowing.CompareNoCase(_T("OpenPriceOpeningNew"))==0
			||StrategyShowing.CompareNoCase(_T("OpenPriceOpeningAsia"))==0){
			if(csInstanceStarted.CompareNoCase(InstanceStatus)==0&&
				df.CompareNoCase(_T("允许开买单"))!=0&&df.CompareNoCase(_T("允许开卖单"))!=0&&df.CompareNoCase(_T("允许平仓"))!=0
				&&df.CompareNoCase(_T("交易时段内开仓次数"))!=0&&df.CompareNoCase(_T("开盘价"))!=0){
				CString strInstanceName = GetItemText(hWnd1,nItem,0);
				AfxMessageBox(_T("实例处于启动状态,仅可修改是否允许买卖平仓及开仓次数"),MB_OK,0);
				return ;
			}
		}else{
			if(csInstanceStarted.CompareNoCase(InstanceStatus)==0){
				CString strInstanceName = GetItemText(hWnd1,nItem,0);
				AfxMessageBox(_T("实例处于启动状态,不可修改"),MB_OK,0);
				return ;
			}
		}*/

		ParamsChangedOrNot = true;

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

void ParamsMgrDlg::OnEnKillfocusEdit1()
{
	// TODO: 在此添加控件通知处理程序代码
	CString tem;
	GetDlgItem(IDC_EDIT1)->GetWindowText(tem);    //得到用户输入的新的内容
	pParamsList->SetItemText(nItem, nSubItem, tem);  //设置编辑框的新内容
	GetDlgItem(IDC_EDIT1)->ShowWindow(SW_HIDE);   //隐藏编辑框
}

int ParamsMgrDlg::ValidateInstance(StrategyInstanceNode mInstance)
{
	//Start to validate the input Start&End Time
	char csstart[30];
	char csend[30];
	std::list<ParamNode>::iterator param_itr;
	for (param_itr = mInstance.ParamList.begin(); param_itr != mInstance.ParamList.end(); ++param_itr) {
		CString paraName(param_itr->ParamName);
		CString newParamValue(param_itr->ParamValue);
		if (paraName.CompareNoCase(_T("StartTime")) == 0)
		{
			/*
			int paramValuelen=WideCharToMultiByte(CP_ACP,0,newParamValue,newParamValue.GetLength(),NULL,0,NULL,NULL);
			char *cparavalue=new char[paramValuelen+1];
			WideCharToMultiByte(CP_ACP,0,newParamValue,newParamValue.GetLength(),cparavalue,paramValuelen,NULL,NULL);
			cparavalue[paramValuelen]='\0';
			strcpy(csstart,cparavalue);
			delete cparavalue;
			*/
			ConvertCStringToCharArray(newParamValue, csstart);
			//param_it->ParamValue=wcstod(newParamValue,NULL);
		}
		else if (paraName.CompareNoCase(_T("EndTime")) == 0)
		{
			/*
			int paramValuelen=WideCharToMultiByte(CP_ACP,0,newParamValue,newParamValue.GetLength(),NULL,0,NULL,NULL);
			char *cparavalue=new char[paramValuelen+1];
			WideCharToMultiByte(CP_ACP,0,newParamValue,newParamValue.GetLength(),cparavalue,paramValuelen,NULL,NULL);
			cparavalue[paramValuelen]='\0';
			strcpy(csend,cparavalue);
			delete cparavalue;
			*/
			ConvertCStringToCharArray(newParamValue, csend);
			//param_it->ParamValue=wcstod(newParamValue,NULL);
		}/*else if(paraName.CompareNoCase(_T("UpGridCount"))==0)		// 注释网格数量限制 modify by Allen
		{
			char cUpGridCount[10];
			ConvertCStringToCharArray(newParamValue,cUpGridCount);
			if(atoi(cUpGridCount)>5){
				CString strtemp;
				strtemp.Format(_T("网格数必须小于5,请重新设置向上的网格数!")); // pNMListView->iSubItem
				AfxMessageBox(strtemp,MB_OK,0);
				return -1;
			}
		}else if(paraName.CompareNoCase(_T("DnGridCount"))==0)
		{
			char cDnGridCount[10];
			ConvertCStringToCharArray(newParamValue,cDnGridCount);
			if(atoi(cDnGridCount)>5){
				CString strtemp;
				strtemp.Format(_T("网格数必须小于5,请重新设置向下的网格数!")); // pNMListView->iSubItem
				AfxMessageBox(strtemp,MB_OK,0);
				return -1;
			}
		}*/
	}
	CString csStartTime(csstart);
	CString csEndTime(csend);
	if (csStartTime.GetLength() > 0 && csEndTime.GetLength() > 0) {
		bool TimeValidateOK = true;
		COleDateTime tms;
		if (tms.ParseDateTime(csStartTime) && strlen(csstart) == 19)
		{
			COleDateTime tme;
			if (tme.ParseDateTime(csEndTime) && strlen(csend) == 19)
			{
				if (tms < tme) {
					TimeValidateOK = true;
				}
				else {
					CString strtemp;
					strtemp.Format(_T("结束时间必须大于开始时间,请重新输入!")); // pNMListView->iSubItem
					AfxMessageBox(strtemp, MB_OK, 0);
					TimeValidateOK = false;
				}
			}
			else {
				CString strtemp;
				strtemp.Format(_T("结束时间格式有误,请重新输入!")); // pNMListView->iSubItem
				AfxMessageBox(strtemp, MB_OK, 0);
				TimeValidateOK = false;
			}
		}
		else {
			CString strtemp;
			strtemp.Format(_T("开始时间格式有误,请重新输入!")); // pNMListView->iSubItem
			AfxMessageBox(strtemp, MB_OK, 0);
			TimeValidateOK = false;
		}
		if (!TimeValidateOK) {
			return -1;
		}
	}

	//End to validate the input StartTime/EndTime
	return 1;
}

void ParamsMgrDlg::ConvertCStringToCharArray(CString csSource, char* rtnCharArray)
{
	int cslen = WideCharToMultiByte(CP_ACP, 0, csSource, csSource.GetLength(), NULL, 0, NULL, NULL);
	char* carray = new char[cslen + 1];
	WideCharToMultiByte(CP_ACP, 0, csSource, csSource.GetLength(), carray, cslen, NULL, NULL);
	carray[cslen] = '\0';
	strcpy(rtnCharArray, carray);
	delete carray;
}
void ParamsMgrDlg::OnBnClickedSaveBtn()
{
	// TODO: 在此添加控件通知处理程序代码
	std::list<ModelNode>::iterator model_itr;
	if (!ModelList.empty()) {
		for (model_itr = ModelList.begin(); model_itr != ModelList.end(); ++model_itr) {
			CString ModelName(model_itr->ModelName);
			std::list<StrategyNode>::iterator strategy_itr;
			if (!model_itr->StrategyList.empty()) {
				for (strategy_itr = model_itr->StrategyList.begin(); strategy_itr != model_itr->StrategyList.end(); ++strategy_itr) {
					CString ClassName(strategy_itr->StrategyName);
					CString StrategyID(strategy_itr->StrategyID);
					if (ClassName.CompareNoCase(ClassNameShowing) == 0 && StrategyID.CompareNoCase(StrategyIDShowing) == 0) {
						//save the instances for current startegy
						int lineCount = pParamsList->GetItemCount();
						int columnCount = strategy_itr->ParamENGNameList.size();

						map<string, int> instancetoshmindexmap;
						std::list<StrategyInstanceNode>::iterator instance_itr;
						for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
							string strinstancename(instance_itr->InstanceName);
							instancetoshmindexmap.insert(std::pair<string, int>(strinstancename, instance_itr->shmindex));
						}

						StrategyNode newStrategyInfo;
						map<string, int> instancenamemap;
						for (int i = 0; i < lineCount; i++) {
							StrategyInstanceNode instance;
							CString instanceName = pParamsList->GetItemText(i, 1);
							ConvertCStringToCharArray(instanceName, instance.InstanceName);

							string strInstanceName(instance.InstanceName);

							map<string, int>::iterator instancenamemap_iter;
							instancenamemap_iter = instancenamemap.find(strInstanceName);
							if (instancenamemap_iter != instancenamemap.end()) {
								AfxMessageBox(_T("实例名称重复,未通过验证,无法保存."), MB_OK, 0);
								return;
							}
							else {
								instancenamemap.insert(std::pair<string, int>(strInstanceName, i));
							}

							map<string, int>::iterator iter;
							iter = instancetoshmindexmap.find(strInstanceName);
							if (iter != instancetoshmindexmap.end()) {
								instance.shmindex = iter->second;
							}
							else instance.shmindex = -1;

							if (instance.shmindex >= 0)instance.StrategyStarted = true;
							else instance.StrategyStarted = false;

							ParamNode paramNode;
							int j = 0;
							std::list<string>::iterator paramname_itr;
							for (j = 0, paramname_itr = strategy_itr->ParamENGNameList.begin(); j < columnCount && paramname_itr != strategy_itr->ParamENGNameList.end(); ++paramname_itr, j++) {
								CString newParamValue = pParamsList->GetItemText(i, j + 2);//从第三列开始，第一列是实例运行状态,第二列是实例名称

								ConvertCStringToCharArray(newParamValue, paramNode.ParamValue);
								strcpy(paramNode.ParamName, (*paramname_itr).c_str());

								LVCOLUMN lvcol;
								//int   nColNum;
								char  str[256];
								lvcol.mask = LVCF_TEXT;
								USES_CONVERSION;
								lvcol.pszText = A2T(str);
								lvcol.cchTextMax = 256;
								pParamsList->GetColumn(j + 2, &lvcol);//从第三列开始，第一列是实例运行状态,第二列是实例名称

								CString header = lvcol.pszText;

								ConvertCStringToCharArray(header, paramNode.ParamChineseName);
								instance.ParamList.push_back(paramNode);
							}
							int validateRtn = ValidateInstance(instance);
							if (validateRtn == -1) {
								AfxMessageBox(_T("实例信息未通过验证,无法保存."), MB_OK, 0);
								return;
							}
							newStrategyInfo.InstanceList.push_back(instance);
						}//End for all the instances for current startegy
						//所有实例通过验证,保存到原有的策略信息中
						strategy_itr->InstanceList.clear();
						for (instance_itr = newStrategyInfo.InstanceList.begin(); instance_itr != newStrategyInfo.InstanceList.end(); ++instance_itr) {
							strategy_itr->InstanceList.push_back((*instance_itr));
						}

						break;
					}
				}
			}
		}
	}

	//Save all Params to file
	FILE* fp = fopen("Model.ini", "w+");
	if (fp == NULL) { TRACE("Error in Open ini file"); TRACE("%s\n", strerror(errno)); }
	for (model_itr = ModelList.begin(); model_itr != ModelList.end(); ++model_itr) {
		std::list<StrategyNode>::iterator strategy_itr;
		for (strategy_itr = model_itr->StrategyList.begin(); strategy_itr != model_itr->StrategyList.end(); ++strategy_itr) {
			std::list<StrategyInstanceNode>::iterator instance_itr;
			for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
				std::list<ParamNode>::iterator param_itr;
				for (param_itr = instance_itr->ParamList.begin(); param_itr != instance_itr->ParamList.end(); ++param_itr) {
					char line[500];
					sprintf(line, "%s,%s,%s,%d,%s,%s,%s,%s\n", model_itr->ModelName, strategy_itr->StrategyName, strategy_itr->StrategyID, strategy_itr->MaxOnHandPositionCount, instance_itr->InstanceName, param_itr->ParamName, param_itr->ParamChineseName, param_itr->ParamValue);
					fwrite(line, strlen(line), 1, fp);
				}
			}
		}
	}
	fclose(fp);

	//更新修改到已启动的策略程序中
	for (int i = 0; i < gStrategyImplIndex; i++) {
		CString gClassName(gStrategyImpl[i]->mStrategyName);
		CString gStrategyID(gStrategyImpl[i]->mStrategyID);
		if (gStrategyImpl[i] != NULL && gStrategyImpl[i]->GetShmindex() >= 0
			&& gClassName.CompareNoCase(ClassNameShowing) == 0 && gStrategyID.CompareNoCase(StrategyIDShowing) == 0 && gStrategyImpl[i]->m_bIsRunning) {
			//Find the same instance
			for (model_itr = ModelList.begin(); model_itr != ModelList.end(); ++model_itr) {
				CString ModelName(model_itr->ModelName);
				std::list<StrategyNode>::iterator strategy_itr;
				for (strategy_itr = model_itr->StrategyList.begin(); strategy_itr != model_itr->StrategyList.end(); ++strategy_itr) {
					CString ClassName(strategy_itr->StrategyName);
					CString StrategyID(strategy_itr->StrategyID);
					if (ClassName.CompareNoCase(ClassNameShowing) == 0 && StrategyID.CompareNoCase(StrategyIDShowing) == 0) {
						//save the instances for current startegy
						if (ClassName.CompareNoCase(_T("KDJ")) == 0) {
							StrategyKDJ* xCurrentStrategy = (StrategyKDJ*)gStrategyImpl[i];
							CString csInstanceName(xCurrentStrategy->mInstanceName);
							std::list<StrategyInstanceNode>::iterator instance_itr;
							for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
								CString instanceNameT(instance_itr->InstanceName);
								if (instanceNameT.CompareNoCase(csInstanceName) == 0) {
									std::list<ParamNode>::iterator param_itr;
									for (param_itr = instance_itr->ParamList.begin(); param_itr != instance_itr->ParamList.end(); ++param_itr) {
										if (strcmp(param_itr->ParamName, "OpenBuyAllow") == 0
											|| strcmp(param_itr->ParamName, "OpenSellAllow") == 0
											|| strcmp(param_itr->ParamName, "LoopTimes") == 0) {
											xCurrentStrategy->SetParamValue(*param_itr);
										}
									}
								}//找到对应的实例
							}
						}
						else if (ClassName.CompareNoCase(_T("ThreeK")) == 0) {
							StrategyThreeK* xCurrentStrategy = (StrategyThreeK*)gStrategyImpl[i];
							CString csInstanceName(xCurrentStrategy->mInstanceName);
							std::list<StrategyInstanceNode>::iterator instance_itr;
							for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
								CString instanceNameT(instance_itr->InstanceName);
								if (instanceNameT.CompareNoCase(csInstanceName) == 0) {
									std::list<ParamNode>::iterator param_itr;
									for (param_itr = instance_itr->ParamList.begin(); param_itr != instance_itr->ParamList.end(); ++param_itr) {
										if (strcmp(param_itr->ParamName, "OpenBuyAllow") == 0
											|| strcmp(param_itr->ParamName, "OpenSellAllow") == 0
											|| strcmp(param_itr->ParamName, "LoopTimes") == 0) {
											xCurrentStrategy->SetParamValue(*param_itr);
										}
									}
								}//找到对应的实例
							}
						}
						else if (ClassName.CompareNoCase(_T("OpenPriceOpening")) == 0) {
							StrategyOpenPriceOpening* xCurrentStrategy = (StrategyOpenPriceOpening*)gStrategyImpl[i];
							CString csInstanceName(xCurrentStrategy->mInstanceName);
							std::list<StrategyInstanceNode>::iterator instance_itr;
							for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
								CString instanceNameT(instance_itr->InstanceName);
								if (instanceNameT.CompareNoCase(csInstanceName) == 0) {
									std::list<ParamNode>::iterator param_itr;
									for (param_itr = instance_itr->ParamList.begin(); param_itr != instance_itr->ParamList.end(); ++param_itr) {
										if (strcmp(param_itr->ParamName, "OpenBuyAllow") == 0
											|| strcmp(param_itr->ParamName, "OpenSellAllow") == 0
											|| strcmp(param_itr->ParamName, "LoopTimes") == 0
											|| strcmp(param_itr->ParamName, "OpenPrice") == 0) {
											xCurrentStrategy->SetParamValue(*param_itr);
										}
									}
								}//找到对应的实例
							}
						}
						else if (ClassName.CompareNoCase(_T("Bar")) == 0) {
							StrategyBar* xCurrentStrategy = (StrategyBar*)gStrategyImpl[i];
							CString csInstanceName(xCurrentStrategy->mInstanceName);
							std::list<StrategyInstanceNode>::iterator instance_itr;
							for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
								CString instanceNameT(instance_itr->InstanceName);
								if (instanceNameT.CompareNoCase(csInstanceName) == 0) {
									std::list<ParamNode>::iterator param_itr;
									for (param_itr = instance_itr->ParamList.begin(); param_itr != instance_itr->ParamList.end(); ++param_itr) {
										if (strcmp(param_itr->ParamName, "OpenBuyAllow") == 0
											|| strcmp(param_itr->ParamName, "OpenSellAllow") == 0
											|| strcmp(param_itr->ParamName, "LoopTimes") == 0) {
											xCurrentStrategy->SetParamValue(*param_itr);
										}
									}
								}//找到对应的实例
							}
						}
						else if (ClassName.CompareNoCase(_T("BandMomnt")) == 0) {
							StrategyBandMomnt* xCurrentStrategy = (StrategyBandMomnt*)gStrategyImpl[i];
							CString csInstanceName(xCurrentStrategy->mInstanceName);
							std::list<StrategyInstanceNode>::iterator instance_itr;
							for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
								CString instanceNameT(instance_itr->InstanceName);
								if (instanceNameT.CompareNoCase(csInstanceName) == 0) {
									std::list<ParamNode>::iterator param_itr;
									for (param_itr = instance_itr->ParamList.begin(); param_itr != instance_itr->ParamList.end(); ++param_itr) {
										if (strcmp(param_itr->ParamName, "OpenBuyAllow") == 0
											|| strcmp(param_itr->ParamName, "OpenSellAllow") == 0
											|| strcmp(param_itr->ParamName, "LoopTimes") == 0) {
											xCurrentStrategy->SetParamValue(*param_itr);
										}
									}
								}//找到对应的实例
							}
						}
						else if (ClassName.CompareNoCase(_T("GridOpen")) == 0) {
							StrategyGridOpen* xCurrentStrategy = (StrategyGridOpen*)gStrategyImpl[i];
							CString csInstanceName(xCurrentStrategy->mInstanceName);
							std::list<StrategyInstanceNode>::iterator instance_itr;
							for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
								CString instanceNameT(instance_itr->InstanceName);
								if (instanceNameT.CompareNoCase(csInstanceName) == 0) {
									std::list<ParamNode>::iterator param_itr;
									for (param_itr = instance_itr->ParamList.begin(); param_itr != instance_itr->ParamList.end(); ++param_itr) {
										if (strcmp(param_itr->ParamName, "OpenBuyAllow") == 0
											|| strcmp(param_itr->ParamName, "OpenSellAllow") == 0
											|| strcmp(param_itr->ParamName, "LoopTimes") == 0) {
											xCurrentStrategy->SetParamValue(*param_itr);
										}
									}
								}//找到对应的实例
							}
						}
						else if (ClassName.CompareNoCase(_T("WaveOpen")) == 0) {
							StrategyWaveOpen* xCurrentStrategy = (StrategyWaveOpen*)gStrategyImpl[i];
							CString csInstanceName(xCurrentStrategy->mInstanceName);
							std::list<StrategyInstanceNode>::iterator instance_itr;
							for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
								CString instanceNameT(instance_itr->InstanceName);
								if (instanceNameT.CompareNoCase(csInstanceName) == 0) {
									std::list<ParamNode>::iterator param_itr;
									for (param_itr = instance_itr->ParamList.begin(); param_itr != instance_itr->ParamList.end(); ++param_itr) {
										if (strcmp(param_itr->ParamName, "OpenBuyAllow") == 0
											|| strcmp(param_itr->ParamName, "OpenSellAllow") == 0
											|| strcmp(param_itr->ParamName, "LoopTimes") == 0) {
											xCurrentStrategy->SetParamValue(*param_itr);
										}
									}
								}//找到对应的实例
							}
						}
						else if (ClassName.CompareNoCase(_T("WaveOpenAdd")) == 0) {
							StrategyWaveOpenAdd* xCurrentStrategy = (StrategyWaveOpenAdd*)gStrategyImpl[i];
							CString csInstanceName(xCurrentStrategy->mInstanceName);
							std::list<StrategyInstanceNode>::iterator instance_itr;
							for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
								CString instanceNameT(instance_itr->InstanceName);
								if (instanceNameT.CompareNoCase(csInstanceName) == 0) {
									std::list<ParamNode>::iterator param_itr;
									for (param_itr = instance_itr->ParamList.begin(); param_itr != instance_itr->ParamList.end(); ++param_itr) {
										if (strcmp(param_itr->ParamName, "OpenBuyAllow") == 0
											|| strcmp(param_itr->ParamName, "OpenSellAllow") == 0
											|| strcmp(param_itr->ParamName, "LoopTimes") == 0) {
											xCurrentStrategy->SetParamValue(*param_itr);
										}
									}
								}//找到对应的实例
							}
						}
						else if (ClassName.CompareNoCase(_T("UpDownR")) == 0) {
							StrategyUpDownR* xCurrentStrategy = (StrategyUpDownR*)gStrategyImpl[i];
							CString csInstanceName(xCurrentStrategy->mInstanceName);
							std::list<StrategyInstanceNode>::iterator instance_itr;
							for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
								CString instanceNameT(instance_itr->InstanceName);
								if (instanceNameT.CompareNoCase(csInstanceName) == 0) {
									std::list<ParamNode>::iterator param_itr;
									for (param_itr = instance_itr->ParamList.begin(); param_itr != instance_itr->ParamList.end(); ++param_itr) {
										if (strcmp(param_itr->ParamName, "OpenBuyAllow") == 0
											|| strcmp(param_itr->ParamName, "OpenSellAllow") == 0
											|| strcmp(param_itr->ParamName, "LoopTimes") == 0) {
											xCurrentStrategy->SetParamValue(*param_itr);
										}
									}
								}//找到对应的实例
							}
						}
						else if (ClassName.CompareNoCase(_T("LastTTimeOpen")) == 0) {
							StrategyUpDownR* xCurrentStrategy = (StrategyUpDownR*)gStrategyImpl[i];
							CString csInstanceName(xCurrentStrategy->mInstanceName);
							std::list<StrategyInstanceNode>::iterator instance_itr;
							for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
								CString instanceNameT(instance_itr->InstanceName);
								if (instanceNameT.CompareNoCase(csInstanceName) == 0) {
									std::list<ParamNode>::iterator param_itr;
									for (param_itr = instance_itr->ParamList.begin(); param_itr != instance_itr->ParamList.end(); ++param_itr) {
										if (strcmp(param_itr->ParamName, "OpenBuyAllow") == 0
											|| strcmp(param_itr->ParamName, "OpenSellAllow") == 0
											|| strcmp(param_itr->ParamName, "LoopTimes") == 0) {
											xCurrentStrategy->SetParamValue(*param_itr);
										}
									}
								}//找到对应的实例
							}
						}
						else if (ClassName.CompareNoCase(_T("AvgLine")) == 0) {
							StrategyAvgLine* xCurrentStrategy = (StrategyAvgLine*)gStrategyImpl[i];
							CString csInstanceName(xCurrentStrategy->mInstanceName);
							std::list<StrategyInstanceNode>::iterator instance_itr;
							for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
								CString instanceNameT(instance_itr->InstanceName);
								if (instanceNameT.CompareNoCase(csInstanceName) == 0) {
									std::list<ParamNode>::iterator param_itr;
									for (param_itr = instance_itr->ParamList.begin(); param_itr != instance_itr->ParamList.end(); ++param_itr) {
										if (strcmp(param_itr->ParamName, "OpenBuyAllow") == 0
											|| strcmp(param_itr->ParamName, "OpenSellAllow") == 0
											|| strcmp(param_itr->ParamName, "LoopTimes") == 0) {
											xCurrentStrategy->SetParamValue(*param_itr);
										}
									}
								}//找到对应的实例
							}
						}
						else if (ClassName.CompareNoCase(_T("AvgDown")) == 0) {
							StrategyAvgDown* xCurrentStrategy = (StrategyAvgDown*)gStrategyImpl[i];
							CString csInstanceName(xCurrentStrategy->mInstanceName);
							std::list<StrategyInstanceNode>::iterator instance_itr;
							for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
								CString instanceNameT(instance_itr->InstanceName);
								if (instanceNameT.CompareNoCase(csInstanceName) == 0) {
									std::list<ParamNode>::iterator param_itr;
									for (param_itr = instance_itr->ParamList.begin(); param_itr != instance_itr->ParamList.end(); ++param_itr) {
										if (strcmp(param_itr->ParamName, "OpenBuyAllow") == 0
											|| strcmp(param_itr->ParamName, "OpenSellAllow") == 0
											|| strcmp(param_itr->ParamName, "LoopTimes") == 0) {
											xCurrentStrategy->SetParamValue(*param_itr);
										}
									}
								}//找到对应的实例
							}
						}
						else if (ClassName.CompareNoCase(_T("DT")) == 0) {
							StrategyDT* xCurrentStrategy = (StrategyDT*)gStrategyImpl[i];
							CString csInstanceName(xCurrentStrategy->mInstanceName);
							std::list<StrategyInstanceNode>::iterator instance_itr;
							for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
								CString instanceNameT(instance_itr->InstanceName);
								if (instanceNameT.CompareNoCase(csInstanceName) == 0) {
									std::list<ParamNode>::iterator param_itr;
									for (param_itr = instance_itr->ParamList.begin(); param_itr != instance_itr->ParamList.end(); ++param_itr) {
										if (strcmp(param_itr->ParamName, "OpenBuyAllow") == 0
											|| strcmp(param_itr->ParamName, "OpenSellAllow") == 0
											|| strcmp(param_itr->ParamName, "LoopTimes") == 0) {
											xCurrentStrategy->SetParamValue(*param_itr);
										}
									}
								}//找到对应的实例
							}
						}
						else if (ClassName.CompareNoCase(_T("BaseGridOpen")) == 0) {
							StrategyBaseGridOpen* xCurrentStrategy = (StrategyBaseGridOpen*)gStrategyImpl[i];
							CString csInstanceName(xCurrentStrategy->mInstanceName);
							std::list<StrategyInstanceNode>::iterator instance_itr;
							for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
								CString instanceNameT(instance_itr->InstanceName);
								if (instanceNameT.CompareNoCase(csInstanceName) == 0) {
									std::list<ParamNode>::iterator param_itr;
									for (param_itr = instance_itr->ParamList.begin(); param_itr != instance_itr->ParamList.end(); ++param_itr) {
										if (strcmp(param_itr->ParamName, "OpenBuyAllow") == 0
											|| strcmp(param_itr->ParamName, "OpenSellAllow") == 0
											|| strcmp(param_itr->ParamName, "UpGridCount") == 0
											|| strcmp(param_itr->ParamName, "UpGridDistPoint") == 0
											|| strcmp(param_itr->ParamName, "DnGridCount") == 0
											|| strcmp(param_itr->ParamName, "DnGridDistPoint") == 0
											|| strcmp(param_itr->ParamName, "ProfitPoint") == 0
											|| strcmp(param_itr->ParamName, "StoplossPoint") == 0
											|| strcmp(param_itr->ParamName, "GridVol") == 0
											|| strcmp(param_itr->ParamName, "OpenTime") == 0
											|| strcmp(param_itr->ParamName, "CloseTime") == 0
											|| strcmp(param_itr->ParamName, "IsBasePrice") == 0
											|| strcmp(param_itr->ParamName, "LoopTimes") == 0
											|| strcmp(param_itr->ParamName, "BasePrice") == 0) {
											xCurrentStrategy->SetParamValue(*param_itr);
										}
									}
								}//找到对应的实例
							}
						}
						else if (ClassName.CompareNoCase(_T("BaseGridOpen_plus")) == 0) {
						StrategyBaseGridOpen_plus* xCurrentStrategy = (StrategyBaseGridOpen_plus*)gStrategyImpl[i];
						CString csInstanceName(xCurrentStrategy->mInstanceName);
						std::list<StrategyInstanceNode>::iterator instance_itr;
						for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
							CString instanceNameT(instance_itr->InstanceName);
							if (instanceNameT.CompareNoCase(csInstanceName) == 0) {
								std::list<ParamNode>::iterator param_itr;
								for (param_itr = instance_itr->ParamList.begin(); param_itr != instance_itr->ParamList.end(); ++param_itr) {
									if (strcmp(param_itr->ParamName, "OpenBuyAllow") == 0
										|| strcmp(param_itr->ParamName, "OpenSellAllow") == 0
										|| strcmp(param_itr->ParamName, "UpGridCount") == 0
										|| strcmp(param_itr->ParamName, "UpGridDistPoint") == 0
										|| strcmp(param_itr->ParamName, "DnGridCount") == 0
										|| strcmp(param_itr->ParamName, "DnGridDistPoint") == 0
										|| strcmp(param_itr->ParamName, "ProfitPoint") == 0
										|| strcmp(param_itr->ParamName, "StoplossPoint") == 0
										|| strcmp(param_itr->ParamName, "GridVol") == 0
										|| strcmp(param_itr->ParamName, "OpenTime") == 0
										|| strcmp(param_itr->ParamName, "CloseTime") == 0
										|| strcmp(param_itr->ParamName, "IsBasePrice") == 0
										|| strcmp(param_itr->ParamName, "LoopTimes") == 0
										|| strcmp(param_itr->ParamName, "BasePrice") == 0) {
										xCurrentStrategy->SetParamValue(*param_itr);
									}
								}
							}//找到对应的实例
						}
						}
						else if (ClassName.CompareNoCase(_T("BaoChe")) == 0) {
							StrategyBaoChe* xCurrentStrategy = (StrategyBaoChe*)gStrategyImpl[i];
							CString csInstanceName(xCurrentStrategy->mInstanceName);
							std::list<StrategyInstanceNode>::iterator instance_itr;
							for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
								CString instanceNameT(instance_itr->InstanceName);
								if (instanceNameT.CompareNoCase(csInstanceName) == 0) {
									std::list<ParamNode>::iterator param_itr;
									for (param_itr = instance_itr->ParamList.begin(); param_itr != instance_itr->ParamList.end(); ++param_itr) {
										if (strcmp(param_itr->ParamName, "OpenBuyAllow") == 0
											|| strcmp(param_itr->ParamName, "OpenSellAllow") == 0
											|| strcmp(param_itr->ParamName, "ProfitPoint") == 0
											|| strcmp(param_itr->ParamName, "GridVol") == 0
											|| strcmp(param_itr->ParamName, "BackPerc") == 0
											|| strcmp(param_itr->ParamName, "Percent") == 0
											|| strcmp(param_itr->ParamName, "StartTime") == 0
											|| strcmp(param_itr->ParamName, "EndTime") == 0
											|| strcmp(param_itr->ParamName, "OpenTime") == 0
											|| strcmp(param_itr->ParamName, "CloseTime") == 0
											|| strcmp(param_itr->ParamName, "Bandsecond") == 0
											|| strcmp(param_itr->ParamName, "Bandtick") == 0
											|| strcmp(param_itr->ParamName, "Cancelsecond") == 0
											|| strcmp(param_itr->ParamName, "Cancelsecondclose") == 0
											|| strcmp(param_itr->ParamName, "OpenSellAllow") == 0) {
											xCurrentStrategy->SetParamValue(*param_itr);
										}
									}
								}//找到对应的实例
							}
						}
						else if (ClassName.CompareNoCase(_T("OpenPriceOpeningNew")) == 0) {
							StrategyOpenPriceOpeningNew* xCurrentStrategy = (StrategyOpenPriceOpeningNew*)gStrategyImpl[i];
							CString csInstanceName(xCurrentStrategy->mInstanceName);
							std::list<StrategyInstanceNode>::iterator instance_itr;
							for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
								CString instanceNameT(instance_itr->InstanceName);
								if (instanceNameT.CompareNoCase(csInstanceName) == 0) {
									std::list<ParamNode>::iterator param_itr;
									for (param_itr = instance_itr->ParamList.begin(); param_itr != instance_itr->ParamList.end(); ++param_itr) {
										if (strcmp(param_itr->ParamName, "OpenBuyAllow") == 0
											|| strcmp(param_itr->ParamName, "OpenSellAllow") == 0
											|| strcmp(param_itr->ParamName, "LoopTimes") == 0) {
											xCurrentStrategy->SetParamValue(*param_itr);
										}
									}
								}//找到对应的实例
							}
						}
						else if (ClassName.CompareNoCase(_T("OpenPriceOpeningAsia")) == 0) {
							StrategyOpenPriceOpeningAsia* xCurrentStrategy = (StrategyOpenPriceOpeningAsia*)gStrategyImpl[i];
							CString csInstanceName(xCurrentStrategy->mInstanceName);
							std::list<StrategyInstanceNode>::iterator instance_itr;
							for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
								CString instanceNameT(instance_itr->InstanceName);
								if (instanceNameT.CompareNoCase(csInstanceName) == 0) {
									std::list<ParamNode>::iterator param_itr;
									for (param_itr = instance_itr->ParamList.begin(); param_itr != instance_itr->ParamList.end(); ++param_itr) {
										if (strcmp(param_itr->ParamName, "OpenBuyAllow") == 0
											|| strcmp(param_itr->ParamName, "OpenSellAllow") == 0
											|| strcmp(param_itr->ParamName, "LoopTimes") == 0) {
											xCurrentStrategy->SetParamValue(*param_itr);
										}
									}
								}//找到对应的实例
							}
						}
						else if (ClassName.CompareNoCase(_T("BaseGridMAStopOpen")) == 0) {
							StrategyBaseGridMAStopOpen* xCurrentStrategy = (StrategyBaseGridMAStopOpen*)gStrategyImpl[i];
							CString csInstanceName(xCurrentStrategy->mInstanceName);
							std::list<StrategyInstanceNode>::iterator instance_itr;
							for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
								CString instanceNameT(instance_itr->InstanceName);
								if (instanceNameT.CompareNoCase(csInstanceName) == 0) {
									std::list<ParamNode>::iterator param_itr;
									for (param_itr = instance_itr->ParamList.begin(); param_itr != instance_itr->ParamList.end(); ++param_itr) {
										if (strcmp(param_itr->ParamName, "OpenBuyAllow") == 0
											|| strcmp(param_itr->ParamName, "OpenSellAllow") == 0
											|| strcmp(param_itr->ParamName, "LoopTimes") == 0
											|| strcmp(param_itr->ParamName, "BasePrice") == 0) {
											xCurrentStrategy->SetParamValue(*param_itr);
										}
									}
								}//找到对应的实例
							}
						}
						else if (ClassName.CompareNoCase(_T("BaseGridOpenCffex")) == 0) {
							if (::MessageBox(NULL, CString("确定要更改?") + StrategyID, CString("保存设置"), MB_YESNO) == IDYES) {
								StrategyBaseGridOpenCffex* xCurrentStrategy = (StrategyBaseGridOpenCffex*)gStrategyImpl[i];
								CString csInstanceName(xCurrentStrategy->mInstanceName);
								std::list<StrategyInstanceNode>::iterator instance_itr;
								for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
									CString instanceNameT(instance_itr->InstanceName);
									if (instanceNameT.CompareNoCase(csInstanceName) == 0) {
										std::list<ParamNode>::iterator param_itr;
										for (param_itr = instance_itr->ParamList.begin(); param_itr != instance_itr->ParamList.end(); ++param_itr) {
											if (strcmp(param_itr->ParamName, "OpenBuyAllow") == 0
												|| strcmp(param_itr->ParamName, "OpenSellAllow") == 0
												|| strcmp(param_itr->ParamName, "LoopTimes") == 0
												|| strcmp(param_itr->ParamName, "BasePrice") == 0
												|| strcmp(param_itr->ParamName, "yBuyPosition") == 0
												|| strcmp(param_itr->ParamName, "ySellPosition") == 0) {
												xCurrentStrategy->SetParamValue(*param_itr);
											}
										}
									}//找到对应的实例
								}
							}
						}
						else if (ClassName.CompareNoCase(_T("BaseGridMAStopOpenCffex")) == 0) {
							if (::MessageBox(NULL, CString("确定要更改?") + StrategyID, CString("保存设置"), MB_YESNO) == IDYES) {
								StrategyBaseGridMAStopOpenCffex* xCurrentStrategy = (StrategyBaseGridMAStopOpenCffex*)gStrategyImpl[i];
								CString csInstanceName(xCurrentStrategy->mInstanceName);
								std::list<StrategyInstanceNode>::iterator instance_itr;
								for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
									CString instanceNameT(instance_itr->InstanceName);
									if (instanceNameT.CompareNoCase(csInstanceName) == 0) {
										std::list<ParamNode>::iterator param_itr;
										for (param_itr = instance_itr->ParamList.begin(); param_itr != instance_itr->ParamList.end(); ++param_itr) {
											if (strcmp(param_itr->ParamName, "OpenBuyAllow") == 0
												|| strcmp(param_itr->ParamName, "OpenSellAllow") == 0
												|| strcmp(param_itr->ParamName, "LoopTimes") == 0
												|| strcmp(param_itr->ParamName, "BasePrice") == 0
												|| strcmp(param_itr->ParamName, "yBuyPosition") == 0
												|| strcmp(param_itr->ParamName, "ySellPosition") == 0) {
												xCurrentStrategy->SetParamValue(*param_itr);
											}
										}
									}//找到对应的实例
								}
							}
						}
						else if (ClassName.CompareNoCase(_T("OpenPriceOpeningNight")) == 0) {
							if (::MessageBox(NULL, CString("确定要更改?") + StrategyID, CString("保存设置"), MB_YESNO) == IDYES) {
								StrategyOpenPriceOpeningNight* xCurrentStrategy = (StrategyOpenPriceOpeningNight*)gStrategyImpl[i];
								CString csInstanceName(xCurrentStrategy->mInstanceName);
								std::list<StrategyInstanceNode>::iterator instance_itr;
								for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
									CString instanceNameT(instance_itr->InstanceName);
									if (instanceNameT.CompareNoCase(csInstanceName) == 0) {
										std::list<ParamNode>::iterator param_itr;
										for (param_itr = instance_itr->ParamList.begin(); param_itr != instance_itr->ParamList.end(); ++param_itr) {
											if (strcmp(param_itr->ParamName, "OpenBuyAllow") == 0
												|| strcmp(param_itr->ParamName, "OpenSellAllow") == 0
												|| strcmp(param_itr->ParamName, "LoopTimes") == 0
												|| strcmp(param_itr->ParamName, "BasePrice") == 0
												|| strcmp(param_itr->ParamName, "yBuyPosition") == 0
												|| strcmp(param_itr->ParamName, "ySellPosition") == 0) {
												xCurrentStrategy->SetParamValue(*param_itr);
											}
										}
									}//找到对应的实例
								}
							}
						}
						else if (ClassName.CompareNoCase(_T("GridMAStopGTCChop")) == 0) {
							if (::MessageBox(NULL, CString("确定要更改?") + StrategyID, CString("保存设置"), MB_YESNO) == IDYES) {
								StrategyGridMAStopGTCChop* xCurrentStrategy = (StrategyGridMAStopGTCChop*)gStrategyImpl[i];
								CString csInstanceName(xCurrentStrategy->mInstanceName);
								std::list<StrategyInstanceNode>::iterator instance_itr;
								for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
									CString instanceNameT(instance_itr->InstanceName);
									if (instanceNameT.CompareNoCase(csInstanceName) == 0) {
										std::list<ParamNode>::iterator param_itr;
										for (param_itr = instance_itr->ParamList.begin(); param_itr != instance_itr->ParamList.end(); ++param_itr) {
											if (strcmp(param_itr->ParamName, "OpenBuyAllow") == 0
												|| strcmp(param_itr->ParamName, "OpenSellAllow") == 0
												|| strcmp(param_itr->ParamName, "LoopTimes") == 0
												|| strcmp(param_itr->ParamName, "BasePrice") == 0
												|| strcmp(param_itr->ParamName, "yBuyPosition") == 0
												|| strcmp(param_itr->ParamName, "ySellPosition") == 0) {
												xCurrentStrategy->SetParamValue(*param_itr);
											}
										}
									}//找到对应的实例
								}
							}
						}
						break;
					}//找到当前策略
				}
			}
			//End Find the same instance
		}
	}

	ParamsChangedOrNot = false;
}
/*
void ParamsMgrDlg::OnItemchangedLinksList(NMHDR* pNMHDR, LRESULT* pResult)
{
NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
*pResult = 0;
if (pNMListView->uOldState == 0 && pNMListView->uNewState == 0)
return;    // No change

// Old check box state
BOOL bPrevState = (BOOL)(((pNMListView->uOldState &
LVIS_STATEIMAGEMASK)>>12)-1);
if (bPrevState < 0)    // On startup there's no previous state
bPrevState = 0; // so assign as false (unchecked)
// New check box state
BOOL bChecked =
(BOOL)(((pNMListView->uNewState & LVIS_STATEIMAGEMASK)>>12)-1);
if (bChecked < 0) // On non-checkbox notifications assume false
bChecked = 0;
if (bPrevState == bChecked) // No change in check box
return;

// Now bChecked holds the new check box state
// ....
}
*/

void ParamsMgrDlg::OnBnClickedAddInstanceBtn()
{
	// TODO: 在此添加控件通知处理程序代码
	if (IDOK == AfxMessageBox(_T("是否新增实例?"), MB_OKCANCEL, 0)) {
		int orderIndex = pParamsList->GetItemCount();
		CString itemname("");
		itemname.Format(_T("%d"), orderIndex);
		pParamsList->InsertItem(orderIndex, (LPCTSTR)itemname);

		string strInstanceNotStarted(InstanceNotStarted);
		wstring widstr2;
		widstr2 = s2ws(InstanceNotStarted);
		pParamsList->SetItemText(orderIndex, 0, (LPCTSTR)widstr2.c_str());
	}
}

void ParamsMgrDlg::OnBnClickedDeleteInstanceBtn()
{
	// TODO: 在此添加控件通知处理程序代码
	if (IDOK == AfxMessageBox(_T("是否删除实例?"), MB_OKCANCEL, 0)) {
		int lineCount = pParamsList->GetItemCount();
		CString csInstanceStarted(InstanceStarted);
		CString csInstanceNotStarted(InstanceNotStarted);
		if (lineCount == 1) {
			AfxMessageBox(_T("仅有一个实例,不能删除."), MB_OK, 0);
		}
		else {
			for (int i = 0; i < pParamsList->GetItemCount(); i++) {
				if (pParamsList->GetCheck(i) || pParamsList->GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED) {
					//Check处于选中状态
					CString InstanceStatus = pParamsList->GetItemText(i, 0);
					if (csInstanceStarted.CompareNoCase(InstanceStatus) == 0) {
						AfxMessageBox(_T("实例已启动,无法删除."), MB_OK, 0);
					}
					else if (csInstanceNotStarted.CompareNoCase(InstanceStatus) == 0) {
						pParamsList->DeleteItem(i);
						i--;
					}
				}
			}
		}
	}
}