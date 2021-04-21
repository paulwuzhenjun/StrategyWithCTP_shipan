// StrategyRecoverDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "StrategyRecoverDlg.h"
#include "afxdialogex.h"
#include "MyStruct.h"
#include <algorithm>
#include <io.h>
#include <map>
#include "MyThread.h"
#include <list>
#include "LockVariable.h"
using namespace std;

// StrategyRecoverDlg 对话框
CListCtrl* pStrategyToBeRecover;
extern CMyThread* pRecoverStrategyThread;
extern list<ModelNode> ModelList;
extern map<int, OrderDetailField> RecoverRspOrderMap;
extern RecoverStrategy gRecoverStrategy;
extern list<CMyThread*> mStraThreadStarted;
extern CMyThread* pQryActionThread;
extern bool g_bQryOrderSentByRecoverDlg;
HANDLE RecoverScreenDisplaySem;
extern CLockVariable gLockVariable;

IMPLEMENT_DYNAMIC(StrategyRecoverDlg, CDialog)

StrategyRecoverDlg::StrategyRecoverDlg(CWnd* pParent /*=NULL*/)
	: CDialog(StrategyRecoverDlg::IDD, pParent)
{
}

StrategyRecoverDlg::~StrategyRecoverDlg()
{
}

void StrategyRecoverDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_STRATEGY_TOBE_RECOVER_LST, pStrategyToBeRecover);
	//DDX_Control(pDX, IDC_RECOVER_PARAM_LIST, mRecoverParamList);
	//DDX_Control(pDX, IDC_RECOVER_ORDER_LIST, mRecoverOrderList);
}

BEGIN_MESSAGE_MAP(StrategyRecoverDlg, CDialog)
	ON_WM_SIZE()
	ON_NOTIFY(NM_CLICK, IDC_STRATEGY_TOBE_RECOVER_LST, &StrategyRecoverDlg::OnNMClickStrategyTobeRecoverLst)
	ON_BN_CLICKED(IDC_RESTART_INSTANCE_BTN, &StrategyRecoverDlg::OnBnClickedRestartInstanceBtn)
	ON_BN_CLICKED(IDC_CLEAR_INSTANCE_BTN, &StrategyRecoverDlg::OnBnClickedClearInstanceBtn)
	ON_NOTIFY(NM_DBLCLK, IDC_RECOVER_ORDER_LIST, &StrategyRecoverDlg::OnNMDblclkRecoverOrderList)
	ON_BN_CLICKED(IDC_SAVE_RECOV_ORDERS_BTN, &StrategyRecoverDlg::OnBnClickedSaveRecovOrdersBtn)
	ON_EN_KILLFOCUS(IDC_EDIT1, &StrategyRecoverDlg::OnEnKillfocusEdit1)
END_MESSAGE_MAP()

BOOL StrategyRecoverDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	pStrategyToBeRecover = (CListCtrl*)GetDlgItem(IDC_STRATEGY_TOBE_RECOVER_LST);
	mRecoverParamList = (CListCtrl*)GetDlgItem(IDC_RECOVER_PARAM_LIST);
	mRecoverOrderList = (CListCtrl*)GetDlgItem(IDC_RECOVER_ORDER_LIST);

	pStrategyToBeRecover->DeleteAllItems();
	pStrategyToBeRecover->ModifyStyle(0, LVS_REPORT);
	pStrategyToBeRecover->SetExtendedStyle(pStrategyToBeRecover->GetExtendedStyle() | LVS_EX_FULLROWSELECT);
	pStrategyToBeRecover->SetExtendedStyle(pStrategyToBeRecover->GetExtendedStyle() | LVS_EX_CHECKBOXES);

	LV_COLUMN column;
	column.pszText = _T("类名");
	column.mask = LVCF_TEXT;
	pStrategyToBeRecover->InsertColumn(0, &column); //插入一列，列索引为0
	column.pszText = _T("模型ID");
	column.mask = LVCF_TEXT;
	pStrategyToBeRecover->InsertColumn(1, &column);
	//m_list->SetColumnWidth(0,100); //列宽设置
	column.pszText = _T("实例名");
	column.mask = LVCF_TEXT;
	pStrategyToBeRecover->InsertColumn(2, &column);
	column.pszText = _T("合约名");
	column.mask = LVCF_TEXT;
	pStrategyToBeRecover->InsertColumn(3, &column);
	column.pszText = _T("开始时间");
	column.mask = LVCF_TEXT;
	pStrategyToBeRecover->InsertColumn(4, &column);
	column.pszText = _T("结束时间");
	column.mask = LVCF_TEXT;
	pStrategyToBeRecover->InsertColumn(5, &column);
	column.pszText = _T("停止时间");
	column.mask = LVCF_TEXT;
	pStrategyToBeRecover->InsertColumn(6, &column);
	column.pszText = _T("当前状态");
	column.mask = LVCF_TEXT;
	pStrategyToBeRecover->InsertColumn(7, &column);
	column.pszText = _T("实例状态文件");
	column.mask = LVCF_TEXT;
	pStrategyToBeRecover->InsertColumn(8, &column);
	column.pszText = _T("在手开仓");
	column.mask = LVCF_TEXT;
	pStrategyToBeRecover->InsertColumn(9, &column);
	column.pszText = _T("在手平仓");
	column.mask = LVCF_TEXT;
	pStrategyToBeRecover->InsertColumn(10, &column);

	int columnCount = pStrategyToBeRecover->GetHeaderCtrl()->GetItemCount();
	for (int i = 0; i < columnCount; i++) {
		//pParamsList->SetColumnWidth(i,rect4.Width()/columnCount);         //设置列的宽度。
		pStrategyToBeRecover->SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
	}

	//检查各实例是否正常退出,异常退出的实例将在磁盘上保留 实例名.cfg 文件
	CString strPathFile;
	::GetModuleFileName(NULL, strPathFile.GetBuffer(MAX_PATH), MAX_PATH);
	strPathFile.ReleaseBuffer();
	strPathFile = strPathFile.Left(strPathFile.ReverseFind(_T('\\')));
	strPathFile += "\\Strategies";

	int ItermNum = 0;
	std::list<ModelNode>::iterator model_itr;
	if (!ModelList.empty()) {
		for (model_itr = ModelList.begin(); model_itr != ModelList.end(); ++model_itr) {
			CString ModelName(model_itr->ModelName);
			CString csModel1Path;
			csModel1Path.Append(strPathFile);
			csModel1Path += "\\";
			csModel1Path.Append(ModelName);
			std::list<StrategyNode>::iterator strategy_itr;
			if (!model_itr->StrategyList.empty()) {
				for (strategy_itr = model_itr->StrategyList.begin(); strategy_itr != model_itr->StrategyList.end(); ++strategy_itr) {
					CString csStrategyID(strategy_itr->StrategyID);
					CString csStrategyName(strategy_itr->StrategyName);
					CString csStrategy1Path;
					csStrategy1Path.Append(csModel1Path);
					csStrategy1Path += "\\";
					csStrategy1Path.Append(csStrategyID);
					std::list<StrategyInstanceNode>::iterator instance_itr;
					for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
						CString csInstanceName(instance_itr->InstanceName);

						CString csInstanceCfg;
						csInstanceCfg.Append(csStrategy1Path);
						csInstanceCfg += "\\";
						csInstanceCfg.Append(csInstanceName);
						csInstanceCfg += ".cfg";

						int len = WideCharToMultiByte(CP_ACP, 0, csInstanceCfg, csInstanceCfg.GetLength(), NULL, 0, NULL, NULL);
						char* c_str_filename = new char[len + 1];
						WideCharToMultiByte(CP_ACP, 0, csInstanceCfg, csInstanceCfg.GetLength(), c_str_filename, len, NULL, NULL);
						c_str_filename[len] = '\0';
						if (_access(c_str_filename, 0) != -1) {
							//文件存在,实例属于异常退出
							int orderIndex = pStrategyToBeRecover->GetItemCount();
							CString itemname("");
							itemname.Format(_T("%d"), orderIndex);
							pStrategyToBeRecover->InsertItem(orderIndex, (LPCTSTR)itemname);

							CString csInstanceNotStarted(InstanceNotStarted);

							pStrategyToBeRecover->SetItemText(orderIndex, 0, (LPCTSTR)csStrategyName);
							pStrategyToBeRecover->SetItemText(orderIndex, 1, (LPCTSTR)csStrategyID);
							pStrategyToBeRecover->SetItemText(orderIndex, 2, (LPCTSTR)csInstanceName);

							pStrategyToBeRecover->SetItemText(orderIndex, 7, (LPCTSTR)csInstanceNotStarted);//状态
							pStrategyToBeRecover->SetItemText(orderIndex, 8, (LPCTSTR)csInstanceCfg);

							//if(csStrategyName.CompareNoCase(_T("OpenPriceOpening"))==0){
							FILE* fptr = fopen(c_str_filename, "rb");
							fseek(fptr, 0, SEEK_SET);
							mSerializeHeader header;
							fread(&header, sizeof(mSerializeHeader), 1, fptr);
							int OpenOrderCount = header.OpenOrderCount;
							int CloseOrderCount = header.CloseOrderCount;

							CString csInstCode(header.CodeName);
							CString csStartTime(header.StartTime);
							CString csEndTime(header.EndTime);

							CString csStopTime(header.StopTime);
							pStrategyToBeRecover->SetItemText(orderIndex, 3, (LPCTSTR)csInstCode);
							pStrategyToBeRecover->SetItemText(orderIndex, 4, (LPCTSTR)csStartTime);
							pStrategyToBeRecover->SetItemText(orderIndex, 5, (LPCTSTR)csEndTime);
							pStrategyToBeRecover->SetItemText(orderIndex, 6, (LPCTSTR)csStopTime);

							CString csOpenCount("");
							csOpenCount.Format(_T("%d"), OpenOrderCount);
							pStrategyToBeRecover->SetItemText(orderIndex, 9, (LPCTSTR)csOpenCount);

							CString csCloseCount("");
							csCloseCount.Format(_T("%d"), CloseOrderCount);
							pStrategyToBeRecover->SetItemText(orderIndex, 10, (LPCTSTR)csCloseCount);

							fclose(fptr);
							//}
						}
						free(c_str_filename);
					}
				}
			}
		}
	}

	RecoverScreenDisplaySem = CreateSemaphore(NULL, 0, 1, NULL);

	mRecoverOrderList->DeleteAllItems();
	mRecoverOrderList->ModifyStyle(0, LVS_REPORT);
	mRecoverOrderList->SetExtendedStyle(mRecoverOrderList->GetExtendedStyle() | LVS_EX_FULLROWSELECT);
	LV_COLUMN columno;
	columno.pszText = _T("委托ID");
	columno.mask = LVCF_TEXT;
	mRecoverOrderList->InsertColumn(0, &columno); //插入一列，列索引为0
	columno.pszText = _T("开平");
	columno.mask = LVCF_TEXT;
	mRecoverOrderList->InsertColumn(1, &columno);
	columno.pszText = _T("方向");
	columno.mask = LVCF_TEXT;
	mRecoverOrderList->InsertColumn(2, &columno);
	columno.pszText = _T("提交价");
	columno.mask = LVCF_TEXT;
	mRecoverOrderList->InsertColumn(3, &columno);
	columno.pszText = _T("手数");
	columno.mask = LVCF_TEXT;
	mRecoverOrderList->InsertColumn(4, &columno);
	columno.pszText = _T("原成交手数");
	columno.mask = LVCF_TEXT;
	mRecoverOrderList->InsertColumn(5, &columno);
	columno.pszText = _T("原状态");
	columno.mask = LVCF_TEXT;
	mRecoverOrderList->InsertColumn(6, &columno);
	columno.pszText = _T("现成交手数");
	columno.mask = LVCF_TEXT;
	mRecoverOrderList->InsertColumn(7, &columno);
	columno.pszText = _T("现成交价");
	columno.mask = LVCF_TEXT;
	mRecoverOrderList->InsertColumn(8, &columno);
	columno.pszText = _T("现状态");
	columno.mask = LVCF_TEXT;
	mRecoverOrderList->InsertColumn(9, &columno);
	columno.pszText = _T("OpenTradePrice");
	columno.mask = LVCF_TEXT;
	mRecoverOrderList->InsertColumn(10, &columno);
	columno.pszText = _T("MaxProfit");
	columno.mask = LVCF_TEXT;
	mRecoverOrderList->InsertColumn(11, &columno);
	columno.pszText = _T("Stoploss");
	columno.mask = LVCF_TEXT;
	mRecoverOrderList->InsertColumn(12, &columno);
	columno.pszText = _T("开仓ID");
	columno.mask = LVCF_TEXT;
	mRecoverOrderList->InsertColumn(13, &columno);
	//---add 网格序号
	columno.pszText = _T("网格ID");
	columno.mask = LVCF_TEXT;
	mRecoverOrderList->InsertColumn(14, &columno);
	//---end

	int columnCount2 = mRecoverOrderList->GetHeaderCtrl()->GetItemCount();
	for (int i = 0; i < columnCount2; i++) {
		//pParamsList->SetColumnWidth(i,rect4.Width()/columnCount);         //设置列的宽度。
		mRecoverOrderList->SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
	}

	RecoverStrategySelectedRow = -1;

	return TRUE;
}

// StrategyRecoverDlg 消息处理程序
void StrategyRecoverDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	// TODO: Add your message handler code here
	if (nType == SIZE_RESTORED || nType == SIZE_MAXIMIZED)
	{
		ReSize();
	}
}

void StrategyRecoverDlg::ReSize()
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

	/*
	//
	if(pStrategyToBeRecover!=NULL){
		CRect rect4;
		pStrategyToBeRecover->GetClientRect(rect4);                     //获得当前客户区信息
		int columnCount=pStrategyToBeRecover->GetHeaderCtrl()->GetItemCount();
		for(int i=0;i<columnCount;i++){
			pStrategyToBeRecover->SetColumnWidth(i,rect4.Width()/columnCount);         //设置列的宽度。
		}
	}

	if(mRecoverParamList!=NULL){
		CRect rect4;
		mRecoverParamList->GetClientRect(rect4);                     //获得当前客户区信息
		int columnCount=mRecoverParamList->GetHeaderCtrl()->GetItemCount();
		for(int i=0;i<columnCount;i++){
			mRecoverParamList->SetColumnWidth(i,rect4.Width()/columnCount);         //设置列的宽度。
		}
	}

	if(mRecoverOrderList!=NULL){
		CRect rect4;
		mRecoverOrderList->GetClientRect(rect4);                     //获得当前客户区信息
		int columnCount=mRecoverOrderList->GetHeaderCtrl()->GetItemCount();
		for(int i=0;i<columnCount;i++){
			mRecoverOrderList->SetColumnWidth(i,rect4.Width()/columnCount);         //设置列的宽度。
		}
	}
	*/
}

CString StrategyRecoverDlg::GetItemText(
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

string StrategyRecoverDlg::ws2s(const wstring& ws)
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

wstring StrategyRecoverDlg::s2ws(const string& s)
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

void StrategyRecoverDlg::ConvertCStringToCharArray(CString csSource, char* rtnCharArray)
{
	int cslen = WideCharToMultiByte(CP_ACP, 0, csSource, csSource.GetLength(), NULL, 0, NULL, NULL);
	char* carray = new char[cslen + 1];
	WideCharToMultiByte(CP_ACP, 0, csSource, csSource.GetLength(), carray, cslen, NULL, NULL);
	carray[cslen] = '\0';
	strcpy(rtnCharArray, carray);
	delete carray;
}

void StrategyRecoverDlg::OnNMClickStrategyTobeRecoverLst(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	DWORD dwPos = GetMessagePos();
	CPoint point(LOWORD(dwPos), HIWORD(dwPos));

	pStrategyToBeRecover->ScreenToClient(&point);

	LVHITTESTINFO lvinfo;
	lvinfo.pt = point;
	lvinfo.flags = LVHT_ABOVE;

	UINT nFlag;
	int nItemRow = pStrategyToBeRecover->HitTest(point, &nFlag);
	//判断是否点在checkbox上
	if (nFlag == LVHT_ONITEMSTATEICON)
	{
		CString strInstanceName = pStrategyToBeRecover->GetItemText(pNMLV->iItem, 2);//GetItemText(hWnd1,nItem ,1);
		AfxMessageBox(_T("点在checkbox上:") + strInstanceName, MB_OK, 0);
	}
	else {
		// *pResult = 0;
		Invalidate();
		//get the row number
		nItem = pNMLV->iItem;

		if (nItem == -1)
			return;
		//Retrieve the text of the selected subItem
		//from the list
		RecoverStrategySelectedRow = nItem;

		CString csStrategyName = pStrategyToBeRecover->GetItemText(pNMLV->iItem, 0);
		CString csStrategyID = pStrategyToBeRecover->GetItemText(pNMLV->iItem, 1);//GetItemText(hWnd1,nItem ,
		CString csInstanceName = pStrategyToBeRecover->GetItemText(pNMLV->iItem, 2);
		//1);
		CString csStartTime = pStrategyToBeRecover->GetItemText(pNMLV->iItem, 3);
		CString csEndTime = pStrategyToBeRecover->GetItemText(pNMLV->iItem, 4);

		ShowRecoverInstance(csStrategyName, csStrategyID, csInstanceName, csStartTime, csEndTime);
		ShowRecoverInstanceOrder(csStrategyName, csStrategyID, csInstanceName);

		AfxMessageBox(_T("点在:") + csInstanceName, MB_OK, 0);
	}
	*pResult = 0;
}

void StrategyRecoverDlg::ShowRecoverInstance(CString xStrategyName, CString xStrategyID, CString xInstanceName, CString xStartTime, CString xEndTime)
{
	mRecoverParamList->DeleteAllItems();
	mRecoverParamList->ModifyStyle(0, LVS_REPORT);
	mRecoverParamList->SetExtendedStyle(mRecoverParamList->GetExtendedStyle() | LVS_EX_FULLROWSELECT);

	int  k = mRecoverParamList->GetHeaderCtrl()->GetItemCount();
	for (int m = 0; m <= k; m++)
	{
		mRecoverParamList->DeleteColumn(0);
	}

	//产生表头
	std::list<string>::iterator paramname_itr;
	int columnid = 0;

	LV_COLUMN column;
	std::list<ModelNode>::iterator model_itr;
	for (model_itr = ModelList.begin(); model_itr != ModelList.end(); ++model_itr) {
		CString ModelName(model_itr->ModelName);
		std::list<StrategyNode>::iterator strategy_itr;
		for (strategy_itr = model_itr->StrategyList.begin(); strategy_itr != model_itr->StrategyList.end(); ++strategy_itr) {
			CString csStrategyName(strategy_itr->StrategyName);
			if (csStrategyName.CompareNoCase(xStrategyName) == 0) {
				for (paramname_itr = strategy_itr->ParamCHNNameList.begin(); paramname_itr != strategy_itr->ParamCHNNameList.end(); ++paramname_itr) {
					wstring widstr;
					widstr = s2ws((*paramname_itr));//std::wstring((*paramname_itr).begin(), (*paramname_itr).end());
					//LV_COLUMN column;
					column.pszText = (LPWSTR)widstr.c_str();//_T("ID");
					column.mask = LVCF_TEXT;
					mRecoverParamList->InsertColumn(columnid, &column); //插入一列，列索引为0
					columnid++;
				}

				std::list<StrategyInstanceNode>::iterator instance_itr;
				for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
					CString csInstanceName(instance_itr->InstanceName);
					int orderIndex = 0;
					int paramindex = 0;
					if (csInstanceName.CompareNoCase(xInstanceName) == 0) {
						CString itemname("");
						itemname.Format(_T("%d"), orderIndex);
						mRecoverParamList->InsertItem(orderIndex, (LPCTSTR)itemname);
						std::list<ParamNode>::iterator param_it;
						for (param_it = instance_itr->ParamList.begin(); param_it != instance_itr->ParamList.end(); ++param_it) {
							CString paramvalue(param_it->ParamValue);

							mRecoverParamList->SetItemText(orderIndex, paramindex, (LPCTSTR)paramvalue);
							if (strcmp(param_it->ParamName, "StartTime") == 0) {
								mRecoverParamList->SetItemText(orderIndex, paramindex, (LPCTSTR)xStartTime);
							}
							else if (strcmp(param_it->ParamName, "EndTime") == 0) {
								mRecoverParamList->SetItemText(orderIndex, paramindex, (LPCTSTR)xEndTime);
							}

							paramindex++;
						}
					}
				}
				break;
			}
		}
	}

	if (columnid > 0) {
		CRect rect4;
		mRecoverParamList->GetClientRect(rect4);                     //获得当前客户区信息
		for (int i = 0; i < columnid; i++) {
			//mRecoverParamList->SetColumnWidth(i,rect4.Width()/columnid);         //设置列的宽度。
			mRecoverParamList->SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
		}
	}
}

void StrategyRecoverDlg::ShowRecoverInstanceOrder(CString xStrategyName, CString xStrategyID, CString  xInstanceName)
{
	mRecoverOrderList->DeleteAllItems();
	if (mRecoverOrderList != NULL) {
		CRect rect4;
		mRecoverOrderList->GetClientRect(rect4);                     //获得当前客户区信息
		int columnCount = mRecoverOrderList->GetHeaderCtrl()->GetItemCount();
		for (int i = 0; i < columnCount; i++) {
			//mRecoverOrderList->SetColumnWidth(i,rect4.Width()/columnCount);         //设置列的宽度。
			mRecoverOrderList->SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
		}
	}

	CString strPathFile;
	::GetModuleFileName(NULL, strPathFile.GetBuffer(MAX_PATH), MAX_PATH);
	strPathFile.ReleaseBuffer();
	strPathFile = strPathFile.Left(strPathFile.ReverseFind(_T('\\')));
	strPathFile += "\\Strategies";

	int ItermNum = 0;
	std::list<ModelNode>::iterator model_itr;

	for (model_itr = ModelList.begin(); model_itr != ModelList.end(); ++model_itr) {
		CString ModelName(model_itr->ModelName);
		CString csModel1Path;
		csModel1Path.Append(strPathFile);
		csModel1Path += "\\";
		csModel1Path.Append(ModelName);
		std::list<StrategyNode>::iterator strategy_itr;
		for (strategy_itr = model_itr->StrategyList.begin(); strategy_itr != model_itr->StrategyList.end(); ++strategy_itr) {
			CString csStrategyName(strategy_itr->StrategyName);
			CString csStrategyID(strategy_itr->StrategyID);
			if (csStrategyID.CompareNoCase(xStrategyID) == 0) {
				CString csStrategy1Path;
				csStrategy1Path.Append(csModel1Path);
				csStrategy1Path += "\\";
				csStrategy1Path.Append(csStrategyID);
				std::list<StrategyInstanceNode>::iterator instance_itr;
				for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
					CString csInstanceName(instance_itr->InstanceName);

					if (csInstanceName.CompareNoCase(xInstanceName) == 0) {
						CString csInstanceCfg;
						csInstanceCfg.Append(csStrategy1Path);
						csInstanceCfg += "\\";
						csInstanceCfg.Append(csInstanceName);
						csInstanceCfg += ".cfg";

						int len = WideCharToMultiByte(CP_ACP, 0, csInstanceCfg, csInstanceCfg.GetLength(), NULL, 0, NULL, NULL);
						char* c_str_filename = new char[len + 1];
						WideCharToMultiByte(CP_ACP, 0, csInstanceCfg, csInstanceCfg.GetLength(), c_str_filename, len, NULL, NULL);
						c_str_filename[len] = '\0';

						FILE* fptr = fopen(c_str_filename, "rb");
						fseek(fptr, 0, SEEK_SET);
						mSerializeHeader header;
						fread(&header, sizeof(mSerializeHeader), 1, fptr);
						int OpenOrderCount = header.OpenOrderCount;
						int CloseOrderCount = header.CloseOrderCount;

						//list<MyCloseOrderType> CloseOrderList;
						//list<MyOpenOrderType> OpenOrderList;

						MyOpenOrderType openOrder;
						for (int i = 0; i < OpenOrderCount; i++) {
							fread(&openOrder, sizeof(MyOpenOrderType), 1, fptr);
							//OpenOrderList.push_back(openOrder);

							int orderIndex = mRecoverOrderList->GetItemCount();
							CString itemname("");
							itemname.Format(_T("%d"), orderIndex);
							mRecoverOrderList->InsertItem(orderIndex, (LPCTSTR)itemname);

							CString csValue("");
							csValue.Format(_T("%d"), openOrder.OrderId);
							mRecoverOrderList->SetItemText(orderIndex, 0, (LPCTSTR)csValue);

							CString csOpen(_T("开仓"));
							mRecoverOrderList->SetItemText(orderIndex, 1, (LPCTSTR)csOpen);

							CString direction("");
							if (openOrder.Direction == MORDER_BUY) {
								direction = _T("买");
							}
							else if (openOrder.Direction == MORDER_SELL) {
								direction = _T("卖");
							}
							mRecoverOrderList->SetItemText(orderIndex, 2, (LPCTSTR)direction);

							CString csSubmitPrice;
							csSubmitPrice.Format(_T("%.5f"), openOrder.LimitPrice);
							mRecoverOrderList->SetItemText(orderIndex, 3, (LPCTSTR)csSubmitPrice);

							CString csSubmitVol;
							csSubmitVol.Format(_T("%d"), openOrder.VolumeTotalOriginal);
							mRecoverOrderList->SetItemText(orderIndex, 4, (LPCTSTR)csSubmitVol);

							CString csTradedVol;
							csTradedVol.Format(_T("%d"), openOrder.VolumeTraded);
							mRecoverOrderList->SetItemText(orderIndex, 5, (LPCTSTR)csTradedVol);

							CString csOrderStatus("");
							if (openOrder.OrderStatus == MORDER_FAIL) {
								csOrderStatus = _T("指令失败");
							}
							else if (openOrder.OrderStatus == MORDER_ACCEPTED) {
								csOrderStatus = _T("已受理");
							}
							else if (openOrder.OrderStatus == MORDER_QUEUED) {
								csOrderStatus = _T("已排队");
							}
							else if (openOrder.OrderStatus == MORDER_PART_TRADED) {
								csOrderStatus = _T("部分成交");
							}
							else if (openOrder.OrderStatus == MORDER_FULL_TRADED) {
								csOrderStatus = _T("完全成交");
							}
							else if (openOrder.OrderStatus == MORDER_PART_CANCELLED) {
								csOrderStatus = _T("部分撤单");
							}
							else if (openOrder.OrderStatus == MORDER_CANCELLED) {
								csOrderStatus = _T("完全撤单");
							}
							else if (openOrder.OrderStatus == MORDER_WAITFORSUBMIT) {
								csOrderStatus = _T("待报单");
							}
							else {
								csOrderStatus = _T("其它");
							}
							mRecoverOrderList->SetItemText(orderIndex, 6, (LPCTSTR)csOrderStatus);

							map<int, OrderDetailField>::iterator iter;
							iter = RecoverRspOrderMap.find(openOrder.OrderId);
							if (iter != RecoverRspOrderMap.end()) {
								CString csNowTradedVol;
								csNowTradedVol.Format(_T("%d"), iter->second.VolumeTraded);
								mRecoverOrderList->SetItemText(orderIndex, 7, (LPCTSTR)csNowTradedVol);

								CString csNowTradedPrice;
								csNowTradedPrice.Format(_T("%.5f"), iter->second.TradePrice);
								mRecoverOrderList->SetItemText(orderIndex, 8, (LPCTSTR)csNowTradedPrice);

								CString csNowOrderStatus("");
								if (iter->second.OrderStatus == MORDER_FAIL) {
									csNowOrderStatus = _T("指令失败");
								}
								else if (iter->second.OrderStatus == MORDER_ACCEPTED) {
									csNowOrderStatus = _T("已受理");
								}
								else if (iter->second.OrderStatus == MORDER_QUEUED) {
									csNowOrderStatus = _T("已排队");
								}
								else if (iter->second.OrderStatus == MORDER_PART_TRADED) {
									csNowOrderStatus = _T("部分成交");
								}
								else if (iter->second.OrderStatus == MORDER_FULL_TRADED) {
									csNowOrderStatus = _T("完全成交");
								}
								else if (iter->second.OrderStatus == MORDER_PART_CANCELLED) {
									csNowOrderStatus = _T("部分撤单");
								}
								else if (iter->second.OrderStatus == MORDER_CANCELLED) {
									csNowOrderStatus = _T("完全撤单");
								}
								else {
									csNowOrderStatus = _T("其它");
								}
								mRecoverOrderList->SetItemText(orderIndex, 9, (LPCTSTR)csNowOrderStatus);
							}
							else {
								CString csNowTradedVol(_T("0"));
								mRecoverOrderList->SetItemText(orderIndex, 7, (LPCTSTR)csNowTradedVol);

								CString csNowTradedPrice(_T("0"));
								mRecoverOrderList->SetItemText(orderIndex, 8, (LPCTSTR)csNowTradedPrice);

								CString csNowOrderStatus("未报出");
								mRecoverOrderList->SetItemText(orderIndex, 9, (LPCTSTR)csNowOrderStatus);
							}

							CString csOpenTradePrice;
							csOpenTradePrice.Format(_T("%.2f"), openOrder.LimitPrice);
							mRecoverOrderList->SetItemText(orderIndex, 10, (LPCTSTR)csOpenTradePrice);

							CString csMaxProfit;
							csMaxProfit.Format(_T("%.2f"), openOrder.maxProfit);
							mRecoverOrderList->SetItemText(orderIndex, 11, (LPCTSTR)csMaxProfit);

							CString csStoplossPoint;
							csStoplossPoint.Format(_T("%.2f"), openOrder.mStoplossPoint);
							mRecoverOrderList->SetItemText(orderIndex, 12, (LPCTSTR)csStoplossPoint);
							mRecoverOrderList->EnsureVisible(mRecoverOrderList->GetItemCount() - 1, FALSE);

							//----set OrderRef
							if (openOrder.OrderLocalRef > 0) {
								gLockVariable.setThostNextOrderRef(max(openOrder.OrderLocalRef, gLockVariable.getThostNextOrderRef()));
							}
							//End
						}

						MyCloseOrderType closeOrder;
						for (int i = 0; i < CloseOrderCount; i++) {
							fread(&closeOrder, sizeof(MyCloseOrderType), 1, fptr);
							//CloseOrderList.push_back(closeOrder);

							int orderIndex = mRecoverOrderList->GetItemCount();
							CString itemname("");
							itemname.Format(_T("%d"), orderIndex);
							mRecoverOrderList->InsertItem(orderIndex, (LPCTSTR)itemname);

							CString csValue("");
							csValue.Format(_T("%d"), closeOrder.OrderId);
							mRecoverOrderList->SetItemText(orderIndex, 0, (LPCTSTR)csValue);

							CString csClose(_T("平仓"));
							mRecoverOrderList->SetItemText(orderIndex, 1, (LPCTSTR)csClose);

							CString direction("");
							if (closeOrder.Direction == MORDER_BUY) {
								direction = _T("买");
							}
							else if (closeOrder.Direction == MORDER_SELL) {
								direction = _T("卖");
							}
							mRecoverOrderList->SetItemText(orderIndex, 2, (LPCTSTR)direction);

							CString csSubmitPrice;
							csSubmitPrice.Format(_T("%.5f"), closeOrder.LimitPrice);
							mRecoverOrderList->SetItemText(orderIndex, 3, (LPCTSTR)csSubmitPrice);

							CString csSubmitVol;
							csSubmitVol.Format(_T("%d"), closeOrder.VolumeTotalOriginal);
							mRecoverOrderList->SetItemText(orderIndex, 4, (LPCTSTR)csSubmitVol);

							CString csTradedVol;
							csTradedVol.Format(_T("%d"), closeOrder.VolumeTraded);
							mRecoverOrderList->SetItemText(orderIndex, 5, (LPCTSTR)csTradedVol);

							CString csOrderStatus("");
							if (closeOrder.OrderStatus == MORDER_FAIL) {
								csOrderStatus = _T("指令失败");
							}
							else if (closeOrder.OrderStatus == MORDER_ACCEPTED) {
								csOrderStatus = _T("已受理");
							}
							else if (closeOrder.OrderStatus == MORDER_QUEUED) {
								csOrderStatus = _T("已排队");
							}
							else if (closeOrder.OrderStatus == MORDER_PART_TRADED) {
								csOrderStatus = _T("部分成交");
							}
							else if (closeOrder.OrderStatus == MORDER_FULL_TRADED) {
								csOrderStatus = _T("完全成交");
							}
							else if (closeOrder.OrderStatus == MORDER_PART_CANCELLED) {
								csOrderStatus = _T("部分撤单");
							}
							else if (closeOrder.OrderStatus == MORDER_WAITFORSUBMIT) {
								csOrderStatus = _T("待报单");
							}
							else if (closeOrder.OrderStatus == MORDER_CANCELLED) {
								csOrderStatus = _T("完全撤单");
							}
							else {
								csOrderStatus = _T("其它");
							}
							mRecoverOrderList->SetItemText(orderIndex, 6, (LPCTSTR)csOrderStatus);

							map<int, OrderDetailField>::iterator iter;
							iter = RecoverRspOrderMap.find(closeOrder.OrderId);
							if (iter != RecoverRspOrderMap.end()) {
								CString csNowTradedVol;
								csNowTradedVol.Format(_T("%d"), iter->second.VolumeTraded);
								mRecoverOrderList->SetItemText(orderIndex, 7, (LPCTSTR)csNowTradedVol);

								CString csNowTradedPrice;
								csNowTradedPrice.Format(_T("%.5f"), iter->second.TradePrice);
								mRecoverOrderList->SetItemText(orderIndex, 8, (LPCTSTR)csNowTradedPrice);

								CString csNowOrderStatus("");
								if (iter->second.OrderStatus == MORDER_FAIL) {
									csNowOrderStatus = _T("指令失败");
								}
								else if (iter->second.OrderStatus == MORDER_ACCEPTED) {
									csNowOrderStatus = _T("已受理");
								}
								else if (iter->second.OrderStatus == MORDER_QUEUED) {
									csNowOrderStatus = _T("已排队");
								}
								else if (iter->second.OrderStatus == MORDER_PART_TRADED) {
									csNowOrderStatus = _T("部分成交");
								}
								else if (iter->second.OrderStatus == MORDER_FULL_TRADED) {
									csNowOrderStatus = _T("完全成交");
								}
								else if (iter->second.OrderStatus == MORDER_PART_CANCELLED) {
									csNowOrderStatus = _T("部分撤单");
								}
								else if (iter->second.OrderStatus == MORDER_CANCELLED) {
									csNowOrderStatus = _T("完全撤单");
								}
								else {
									csNowOrderStatus = _T("其它");
								}
								mRecoverOrderList->SetItemText(orderIndex, 9, (LPCTSTR)csNowOrderStatus);
							}
							else {
								CString csNowTradedVol(_T("0"));
								mRecoverOrderList->SetItemText(orderIndex, 7, (LPCTSTR)csNowTradedVol);

								CString csNowTradedPrice(_T("0"));
								mRecoverOrderList->SetItemText(orderIndex, 8, (LPCTSTR)csNowTradedPrice);

								CString csNowOrderStatus("未报出");
								mRecoverOrderList->SetItemText(orderIndex, 9, (LPCTSTR)csNowOrderStatus);
							}

							CString csOpenTradePrice;
							csOpenTradePrice.Format(_T("%.2f"), closeOrder.OpenOrderTradePrice);
							mRecoverOrderList->SetItemText(orderIndex, 10, (LPCTSTR)csOpenTradePrice);

							CString csMaxProfit;
							csMaxProfit.Format(_T("%.2f"), closeOrder.maxProfit);
							mRecoverOrderList->SetItemText(orderIndex, 11, (LPCTSTR)csMaxProfit);

							CString csStoplossPoint;
							csStoplossPoint.Format(_T("%.2f"), closeOrder.mStoplossPoint);
							mRecoverOrderList->SetItemText(orderIndex, 12, (LPCTSTR)csStoplossPoint);

							CString csOpenId;
							csOpenId.Format(_T("%d"), closeOrder.OpenOrderID);
							mRecoverOrderList->SetItemText(orderIndex, 13, (LPCTSTR)csOpenId);

							CString csGridId;
							csGridId.Format(_T("%d"), closeOrder.MOrderType);
							mRecoverOrderList->SetItemText(orderIndex, 14, (LPCTSTR)csGridId);

							mRecoverOrderList->EnsureVisible(mRecoverOrderList->GetItemCount() - 1, FALSE);

							//----set OrderRef
							if (closeOrder.OrderLocalRef > 0) {
								gLockVariable.setThostNextOrderRef(max(closeOrder.OrderLocalRef, gLockVariable.getThostNextOrderRef()));
							}
							//End
						}

						fclose(fptr);

						free(c_str_filename);
					}
				}
			}
		}
	}
}

void StrategyRecoverDlg::OnBnClickedRestartInstanceBtn()
{
	// TODO: 在此添加控件通知处理程序代码
	CString csInstanceStarted(InstanceStarted);
	CString csInstanceNotStarted(InstanceNotStarted);
	bool actionOKorNot = true;
	int lineCount = pStrategyToBeRecover->GetItemCount();
	for (int i = 0; i < lineCount; i++) {
		if (pStrategyToBeRecover->GetCheck(i)) {
			CString instanceStatus = pStrategyToBeRecover->GetItemText(i, 7);//第七列是实例状态
			CString instanceName = pStrategyToBeRecover->GetItemText(i, 2);//第二列是实例名称
			if (csInstanceStarted.CompareNoCase(instanceStatus) == 0) {
				AfxMessageBox(_T("有实例处于启动状态,无法再启动"), MB_OK, 0);
				actionOKorNot = false;
				break;
			}
		}
	}

	if (actionOKorNot) {
		for (int i = 0; i < lineCount; i++) {
			if (pStrategyToBeRecover->GetCheck(i)) {
				//CString csStrategyName=pStrategyToBeRecover->GetItemText(i,0);
				//CString csInstanceName=pStrategyToBeRecover->GetItemText(i,1);
				//ConvertCStringToCharArray(csStrategyName,gRecoverStrategy.StrategyName);
				//ConvertCStringToCharArray(csInstanceName,gRecoverStrategy.InstanceName);

				pRecoverStrategyThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
				Sleep(500);
				pRecoverStrategyThread->PostThreadMessage(WM_RESTART_STRATEGY_RECOVERDLG, (WPARAM)i, NULL);
				mStraThreadStarted.push_back(pRecoverStrategyThread);
			}
		}
	}
}

void StrategyRecoverDlg::OnBnClickedClearInstanceBtn()
{
	// TODO: 在此添加控件通知处理程序代码
	// TODO: 在此添加控件通知处理程序代码
	int lineCount = pStrategyToBeRecover->GetItemCount();
	CString csInstanceStarted(InstanceStarted);
	CString csInstanceNotStarted(InstanceNotStarted);

	for (int i = 0; i < pStrategyToBeRecover->GetItemCount(); i++) {
		if (pStrategyToBeRecover->GetCheck(i) || pStrategyToBeRecover->GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED) {
			//Check处于选中状态
			CString InstanceStatus = pStrategyToBeRecover->GetItemText(i, 7);
			if (csInstanceStarted.CompareNoCase(InstanceStatus) == 0) {
				AfxMessageBox(_T("实例已启动,无法清除."), MB_OK, 0);
			}
			else if (csInstanceNotStarted.CompareNoCase(InstanceStatus) == 0) {
				CString InstanceCfgFile = pStrategyToBeRecover->GetItemText(i, 8);
				InstanceCfgFile.Replace(_T("\\"), _T("\\\\"));
				DeleteFile((LPCWSTR)InstanceCfgFile);
				pStrategyToBeRecover->DeleteItem(i);
				//DeleteFile("c:\\abc\\test.exe ");
				i--;
			}
		}
	}
}

void StrategyRecoverDlg::OnNMDblclkRecoverOrderList(NMHDR* pNMHDR, LRESULT* pResult)
{
	//LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	HWND hWnd1 = ::GetDlgItem(m_hWnd, IDC_RECOVER_ORDER_LIST);
	NM_LISTVIEW* temp = (NM_LISTVIEW*)pNMHDR;
	CRect rc;
	if (temp->iItem != -1)
	{
		RECT rect;
		//get the row number
		nItemOrder = temp->iItem;
		//get the column number
		nSubItemOrder = temp->iSubItem;
		if (nSubItemOrder < 1 || nItemOrder == -1)
			return;

		//Retrieve the text of the selected subItem
		//from the list
		CString str = GetItemText(hWnd1, nItemOrder,
			nSubItemOrder);

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

		if (nItemOrder != -1)
			::SetWindowPos(::GetDlgItem(m_hWnd, IDC_EDIT1),
				HWND_TOP, rect.left + x + 2, rect.top + y - 22,
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

void StrategyRecoverDlg::OnEnKillfocusEdit1()
{
	// TODO: 在此添加控件通知处理程序代码
	CString tem;
	GetDlgItem(IDC_EDIT1)->GetWindowText(tem);    //得到用户输入的新的内容
	mRecoverOrderList->SetItemText(nItemOrder, nSubItemOrder, tem);  //设置编辑框的新内容
	GetDlgItem(IDC_EDIT1)->ShowWindow(SW_HIDE);   //隐藏编辑框
}

void StrategyRecoverDlg::SetCell(HWND hWnd1,
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

void StrategyRecoverDlg::OnBnClickedSaveRecovOrdersBtn()
{
	// TODO: 在此添加控件通知处理程序代码
	CString csInstanceCfgFileFullName = pStrategyToBeRecover->GetItemText(RecoverStrategySelectedRow, 8);
	if (IDOK == AfxMessageBox(_T("保存配置文件,是否确定保存?") + csInstanceCfgFileFullName, MB_OKCANCEL, 0)) {
		char cCfgFileName[500];
		ConvertCStringToCharArray(csInstanceCfgFileFullName, cCfgFileName);

		FILE* fptr = fopen(cCfgFileName, "rb");
		fseek(fptr, 0, SEEK_SET);
		mSerializeHeader header;
		fread(&header, sizeof(mSerializeHeader), 1, fptr);
		int OpenOrderCount = header.OpenOrderCount;
		int CloseOrderCount = header.CloseOrderCount;

		// modify---------------------------
		//strcpy(header.CodeName,"sc1906");

		int mOpenTimes;
		bool m_bStoplossClose;
		int GridCloseOrderCount[60];
		double mPrice;
		memcpy(&mOpenTimes, header.SpecificArea, sizeof(int));
		memcpy(&m_bStoplossClose, header.SpecificArea + sizeof(int), sizeof(bool));
		memcpy(&mPrice, header.SpecificArea + sizeof(int) + sizeof(bool), sizeof(double));
		memcpy(&GridCloseOrderCount, header.SpecificArea + sizeof(int) + sizeof(bool) + sizeof(double), sizeof(GridCloseOrderCount));
		//GridCloseOrderCount[28]=0;
		mPrice = 463.5;

		memcpy(header.SpecificArea, &mOpenTimes, sizeof(int));
		memcpy(header.SpecificArea + sizeof(int), &m_bStoplossClose, sizeof(bool));
		memcpy(header.SpecificArea + sizeof(int) + sizeof(bool), &mPrice, sizeof(double));
		memcpy(header.SpecificArea + sizeof(int) + sizeof(bool) + sizeof(double), &GridCloseOrderCount, sizeof(GridCloseOrderCount));

		// End-------------------------------

		list<MyCloseOrderType> CloseOrderList;
		list<MyOpenOrderType> OpenOrderList;

		MyOpenOrderType openOrder;
		int newOpenOrderCount = 0;
		int i = 0;

		for (int openOrderIndex = 0; openOrderIndex < OpenOrderCount; openOrderIndex++, i++) {
			fread(&openOrder, sizeof(MyOpenOrderType), 1, fptr);
			CString csOpenOrderDir = mRecoverOrderList->GetItemText(i, 2);
			CString csOpenOrderPrice = mRecoverOrderList->GetItemText(i, 3);
			CString csOpenOrderVol = mRecoverOrderList->GetItemText(i, 4);
			CString csOpenOrderTradeVol = mRecoverOrderList->GetItemText(i, 5);
			CString csOpenOrderStatus = mRecoverOrderList->GetItemText(i, 6);

			char cOpenOrderDir[10];
			ConvertCStringToCharArray(csOpenOrderDir, cOpenOrderDir);
			char cOpenOrderPrice[15];
			ConvertCStringToCharArray(csOpenOrderPrice, cOpenOrderPrice);
			char cOpenOrderVol[15];
			ConvertCStringToCharArray(csOpenOrderVol, cOpenOrderVol);
			char cOpenOrderTradeVol[15];
			ConvertCStringToCharArray(csOpenOrderTradeVol, cOpenOrderTradeVol);

			if (strcmp(cOpenOrderDir, "买") == MORDER_BUY) {
				openOrder.Direction = MORDER_BUY;
			}
			else {
				openOrder.Direction = MORDER_SELL;
			}
			openOrder.LimitPrice = atof(cOpenOrderPrice);
			openOrder.VolumeTotalOriginal = atoi(cOpenOrderVol);
			openOrder.VolumeTraded = atoi(cOpenOrderTradeVol);
			openOrder.VolumeTotal = openOrder.VolumeTotalOriginal - openOrder.VolumeTraded;

			if (csOpenOrderStatus.CompareNoCase(_T("完全成交")) == 0) {
				openOrder.OrderStatus = MORDER_FULL_TRADED;
			}
			else if (csOpenOrderStatus.CompareNoCase(_T("完全撤单")) == 0) {
				openOrder.OrderStatus = MORDER_CANCELLED;
			}
			else if (csOpenOrderStatus.CompareNoCase(_T("待报单")) == 0) {
				openOrder.OrderStatus = MORDER_WAITFORSUBMIT;
			}

			if (csOpenOrderStatus.CompareNoCase(_T("未报单")) != 0) {
				OpenOrderList.push_back(openOrder);
				newOpenOrderCount++;
			}
		}

		header.OpenOrderCount = newOpenOrderCount;

		//MyCloseOrderType oldcloseOrder;
		MyCloseOrderType closeOrder;
		int newCloseOrderCount = 0;
		for (int closeOrderIndex = 0; closeOrderIndex < CloseOrderCount; closeOrderIndex++, i++) {
			fread(&closeOrder, sizeof(MyCloseOrderType), 1, fptr);
			///------------------------
			/*
			oldcloseOrder.OpenOrderID=(i+100);
			oldcloseOrder.OrderLocalRef=-1;
			strcpy(oldcloseOrder.SubmitDateAndTime,"20180124 22:00:00");
			strcpy(oldcloseOrder.OpenTime,"20180124 22:00:00");
			closeOrder.OrderId=oldcloseOrder.OrderId;
			closeOrder.OpenOrderID=oldcloseOrder.OpenOrderID;
			closeOrder.OrderLocalRef=oldcloseOrder.OrderLocalRef;
			closeOrder.OrderLocalRetReqID=oldcloseOrder.OrderLocalRetReqID;
			strcpy(closeOrder.SubmitDateAndTime,oldcloseOrder.SubmitDateAndTime);//报单日期,内盘没有长期有效单，此字段用于第二日的未成交订单的重新提交
			strcpy(closeOrder.OpenTime,oldcloseOrder.OpenTime);//开仓单对应的开仓成交时间，内盘用于判断是否要用平今代码
			closeOrder.Direction=oldcloseOrder.Direction;
			closeOrder.Offset=oldcloseOrder.Offset;
			closeOrder.VolumeTotalOriginal=oldcloseOrder.VolumeTotalOriginal;
			closeOrder.VolumeTraded=oldcloseOrder.VolumeTraded;
			closeOrder.VolumeTotal=oldcloseOrder.VolumeTotal;
			closeOrder.LimitPrice=oldcloseOrder.LimitPrice;
			closeOrder.OrigSubmitPrice=oldcloseOrder.OrigSubmitPrice;
			closeOrder.dwCloseOrderStart=oldcloseOrder.dwCloseOrderStart;
			closeOrder.OpenOrderSubmitPrice=oldcloseOrder.OpenOrderSubmitPrice;
			closeOrder.OpenOrderTradePrice=oldcloseOrder.OpenOrderTradePrice;
			closeOrder.NextCloseOrderPrice=oldcloseOrder.NextCloseOrderPrice;
			closeOrder.ProfitPrice=oldcloseOrder.ProfitPrice;
			closeOrder.OrderStatus=oldcloseOrder.OrderStatus;

			closeOrder.ManualStopPrice=oldcloseOrder.ManualStopPrice;
			closeOrder.maxProfit=oldcloseOrder.maxProfit;
			closeOrder.IsClosePofitOrder=oldcloseOrder.IsClosePofitOrder;
			closeOrder.IsStoplessOrder=oldcloseOrder.IsStoplessOrder;
			closeOrder.CanbeCanceled=oldcloseOrder.CanbeCanceled;
			closeOrder.MOrderType=oldcloseOrder.MOrderType;
			closeOrder.MAStop=oldcloseOrder.MAStop;
			closeOrder.mStoplossPoint=oldcloseOrder.mStoplossPoint;
			closeOrder.FrontID=oldcloseOrder.FrontID;
			closeOrder.SessionID=oldcloseOrder.SessionID;
			*/
			///------------------------
			CString csCloseOrderDir = mRecoverOrderList->GetItemText(i, 2);
			CString csCloseOrderPrice = mRecoverOrderList->GetItemText(i, 3);
			CString csCloseOrderVol = mRecoverOrderList->GetItemText(i, 4);
			CString csCloseOrderTradeVol = mRecoverOrderList->GetItemText(i, 5);
			CString csCloseOrderStatus = mRecoverOrderList->GetItemText(i, 6);
			CString csOpenTradePrice = mRecoverOrderList->GetItemText(i, 10);
			CString csMaxProfit = mRecoverOrderList->GetItemText(i, 11);
			CString csStoplossPoint = mRecoverOrderList->GetItemText(i, 12);

			char cCloseOrderDir[10];
			ConvertCStringToCharArray(csCloseOrderDir, cCloseOrderDir);
			char cCloseOrderPrice[15];
			ConvertCStringToCharArray(csCloseOrderPrice, cCloseOrderPrice);
			char cCloseOrderVol[15];
			ConvertCStringToCharArray(csCloseOrderVol, cCloseOrderVol);
			char cCloseOrderTradeVol[15];
			ConvertCStringToCharArray(csCloseOrderTradeVol, cCloseOrderTradeVol);
			char cOpenTradePrice[10];
			ConvertCStringToCharArray(csOpenTradePrice, cOpenTradePrice);
			// 暂时注释
			char cMaxProfit[10];
			ConvertCStringToCharArray(csMaxProfit, cMaxProfit);
			char cStoplossPoint[10];
			ConvertCStringToCharArray(csStoplossPoint, cStoplossPoint);

			if (strcmp(cCloseOrderDir, "买") == MORDER_BUY) {
				closeOrder.Direction = MORDER_BUY;
			}
			else {
				closeOrder.Direction = MORDER_SELL;
			}
			closeOrder.LimitPrice = atof(cCloseOrderPrice);
			closeOrder.VolumeTotalOriginal = atoi(cCloseOrderVol);
			closeOrder.VolumeTraded = atoi(cCloseOrderTradeVol);
			closeOrder.VolumeTotal = closeOrder.VolumeTotalOriginal - closeOrder.VolumeTraded;
			closeOrder.OpenOrderTradePrice = atof(cOpenTradePrice);
			closeOrder.maxProfit = atof(cMaxProfit);
			closeOrder.mStoplossPoint = atof(cStoplossPoint);
			//closeOrder.MOrderType+=20;

			// -- modify order
			// End
			//
			//if(closeOrder.IsStoplessOrder){
			//	closeOrder.IsClosePofitOrder=true;
			//	closeOrder.IsStoplessOrder=false;
			//	closeOrder.OrderStatus=MORDER_ACCEPTED;
			//	closeOrder.VolumeTotalOriginal=1;
			//	closeOrder.VolumeTotal=1;
			//	closeOrder.VolumeTraded=0;
			//	closeOrder.ProfitPrice=closeOrder.OpenOrderTradePrice+1000;
			//	closeOrder.LimitPrice=closeOrder.ProfitPrice;
			//	closeOrder.OrigSubmitPrice=closeOrder.LimitPrice;
			//}

			//------------
			/*
			strcpy(closeOrder.SubmitDateAndTime,"20180306 14:05:00");
			strcpy(closeOrder.OpenTime,"20180306 14:05:00");
			closeOrder.LimitPrice=3669;
			closeOrder.OrigSubmitPrice=3669;
			closeOrder.ProfitPrice=closeOrder.LimitPrice;
			closeOrder.ManualStopPrice=0;
			closeOrder.OpenOrderSubmitPrice=2669;
			closeOrder.OpenOrderTradePrice=2669;
			closeOrder.OpenOrderID=4876;
			closeOrder.OrderLocalRef=-1;
			*/
			///----------------
			if (csCloseOrderStatus.CompareNoCase(_T("完全成交")) == 0) {
				closeOrder.OrderStatus = MORDER_FULL_TRADED;
			}
			else if (csCloseOrderStatus.CompareNoCase(_T("完全撤单")) == 0) {
				closeOrder.OrderStatus = MORDER_CANCELLED;
			}
			else if (csCloseOrderStatus.CompareNoCase(_T("待报单")) == 0) {
				closeOrder.OrderStatus = MORDER_WAITFORSUBMIT;
			}
			///*平仓报单信息修改-----paul*/
			//if(closeOrder.MOrderType==1){
			//	closeOrder.CloseOrderSeqNo=-842150450;
			//	closeOrder.OrderId=-1;
			//	closeOrder.OrderLocalRef=-1;
			//	closeOrder.Direction=1;
			//	closeOrder.VolumeTotalOriginal=1;
			//	closeOrder.VolumeTraded=0;
			//	closeOrder.VolumeTotal=1;

			//	closeOrder.LimitPrice=3231.8;
			//	closeOrder.OrigSubmitPrice=closeOrder.LimitPrice;
			//	closeOrder.OpenOrderTradePrice=431.5;
			//	closeOrder.OpenOrderSubmitPrice=2881.6;
			//	closeOrder.maxProfit=10;
			//	closeOrder.ProfitPrice=closeOrder.LimitPrice;
			//	closeOrder.OrderStatus=MORDER_ACCEPTED;
			//	closeOrder.ManualStopPrice=0;
			//	closeOrder.MAStop=false;
			//	closeOrder.mStoplossPoint=500;
			//
			//	/*
			//	//closeOrder.LimitPrice=442.2;
			//	//closeOrder.OrigSubmitPrice=closeOrder.LimitPrice;
			//	//closeOrder.ProfitPrice=closeOrder.LimitPrice;
			//	//closeOrder.OpenOrderSubmitPrice=440;
			//	//closeOrder.OpenOrderTradePrice=440;
			//	//closeOrder.OrderStatus=MORDER_ACCEPTED;
			//	*/
			//	closeOrder.IsClosePofitOrder=true;
			//	closeOrder.IsStoplessOrder=false;
			//	closeOrder.CanbeCanceled=true;
			//}
			if (csCloseOrderStatus.CompareNoCase(_T("未报单")) != 0) {
				CloseOrderList.push_back(closeOrder);
				newCloseOrderCount++;
			}

			//if(newCloseOrderCount==CloseOrderCount){
			//	MyCloseOrderType xnew;
			//	memset(&xnew,0,sizeof(MyCloseOrderType));
			//	memcpy(&xnew,&closeOrder,sizeof(MyCloseOrderType));
			//	strcpy(xnew.SubmitDateAndTime,"20180822 00:55:37");
			//	strcpy(xnew.OpenTime,"20180822 00:55:37");
			//	xnew.OpenOrderID=10835;
			//	xnew.OpenOrderSubmitPrice=267.6;
			//	xnew.OpenOrderTradePrice=267.6;
			//	xnew.LimitPrice=265.4;
			//	xnew.OrigSubmitPrice=xnew.LimitPrice;
			//	xnew.ProfitPrice=xnew.LimitPrice;
			//	xnew.maxProfit=0;
			//	xnew.mStoplossPoint=10;
			//	xnew.VolumeTotalOriginal=1;
			//	xnew.VolumeTotal=1;
			//	xnew.VolumeTraded=0;
			//	xnew.Direction=MORDER_BUY;
			//	xnew.Offset=MORDER_CLOSE;
			//	xnew.OrderStatus=MORDER_WAITFORSUBMIT;
			//	xnew.MOrderType=37;
			//	CloseOrderList.push_back(xnew);
			//	newCloseOrderCount++;
			//}
		}

		fclose(fptr);
		header.CloseOrderCount = newCloseOrderCount;
		///---------write the cfg file
		fptr = fopen(cCfgFileName, "wb");
		fwrite(&header, sizeof(mSerializeHeader), 1, fptr);

		std::list<MyOpenOrderType>::iterator open_itr;
		for (open_itr = OpenOrderList.begin(); open_itr != OpenOrderList.end(); ++open_itr) {
			fwrite(&(*open_itr), sizeof(MyOpenOrderType), 1, fptr);
		}

		std::list<MyCloseOrderType>::iterator close_itr;
		for (close_itr = CloseOrderList.begin(); close_itr != CloseOrderList.end(); ++close_itr) {
			fwrite(&(*close_itr), sizeof(MyCloseOrderType), 1, fptr);
		}
		fclose(fptr);
	}
}