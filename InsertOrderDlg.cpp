// InsertOrderDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "HitTraderApi.h"
#include "InsertOrderDlg.h"
#include "MyStruct.h"
#include "EsunTraderSpi.h"
#include "ThostTraderSpi.h"
#include <map>

using namespace std;

//extern char InstrumentID[];
extern char LoginUserTDEsun[21];
extern CListBox* pPubMsg;

extern EsunTraderSpi* pEsunTraderSpi;
extern ThostTraderSpi* pThostTraderSpi;
extern int iNextOrderRef;    //下一单引用
extern list<ModelNode> ModelList;
extern map<int, ManualOrder> ReqIdToStrategyManualOrder;
extern SRWLOCK  g_srwLockReqIdStra;
extern SRWLOCK  g_srwLockReqIdDirection;
extern map<string, InstrumentInfo> InstrumentsSubscribed;
extern bool EsunAPIUsed;
extern bool CTPAPIUsed;
extern bool SgitAPIUsed;
extern list<InsertOrderField> ambushOrderList;
extern HANDLE ambushordersem;
// CInsertOrderDlg 对话框

IMPLEMENT_DYNAMIC(CInsertOrderDlg, CDialog)

CInsertOrderDlg::CInsertOrderDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CInsertOrderDlg::IDD, pParent)
{
}

CInsertOrderDlg::~CInsertOrderDlg()
{
}

BOOL CInsertOrderDlg::Create()
{
	CDialog::Create(CInsertOrderDlg::IDD);
	return TRUE;
}

void CInsertOrderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_COMB_InstCode, m_cbInstCode);
	int mapindex = 0;
	std::map<string, InstrumentInfo>::iterator map_itr;
	for (map_itr = InstrumentsSubscribed.begin(); map_itr != InstrumentsSubscribed.end(); ++map_itr) {
		CString InstrumentFullName(map_itr->first.c_str());
		m_cbInstCode.InsertString(mapindex, InstrumentFullName);
		mapindex++;
	}

	CString csInst("");
	SetDlgItemText(IDC_InstrumentID, csInst);

	DDX_Control(pDX, IDC_COMBDirection, m_cbDirection);
	m_cbDirection.InsertString(0, _T("Buy"));
	m_cbDirection.InsertString(1, _T("Sell"));

	DDX_Control(pDX, IDC_COMBOPENCLOSE, m_cbOpenOrClose);
	m_cbOpenOrClose.InsertString(0, _T("Open"));
	m_cbOpenOrClose.InsertString(1, _T("Close"));
	m_cbOpenOrClose.InsertString(2, _T("CloseToday"));

	DDX_Control(pDX, IDC_MAMUAL_ORDER_STRA, m_cbManualOrderStra);
	int ItermNum = 0;
	std::list<ModelNode>::iterator model_itr;
	int index = 0;
	m_cbManualOrderStra.InsertString(index, _T("Manual"));
	index++;
	m_cbManualOrderStra.InsertString(index, _T("Ambush"));
	index++;

	if (!ModelList.empty()) {
		for (model_itr = ModelList.begin(); model_itr != ModelList.end(); ++model_itr) {
			if (!model_itr->StrategyList.empty()) {
				std::list<StrategyNode>::iterator strategy_itr;
				for (strategy_itr = model_itr->StrategyList.begin(); strategy_itr != model_itr->StrategyList.end(); ++strategy_itr) {
					CString StrategyName(strategy_itr->StrategyName);
					std::list<StrategyInstanceNode>::iterator instance_itr;
					for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
						CString InstanceName(instance_itr->InstanceName);
						CString xName = StrategyName;
						xName.Append(_T("_"));
						xName.Append(InstanceName);
						m_cbManualOrderStra.InsertString(index, xName);
						index++;
					}
				}
			}
		}
	}
}

BEGIN_MESSAGE_MAP(CInsertOrderDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CInsertOrderDlg::OnBnClickedOk)
END_MESSAGE_MAP()

// CInsertOrderDlg 消息处理程序

void CInsertOrderDlg::OnBnClickedOk()
{
	CString str;
	GetDlgItemText(IDC_InstrumentID, str);
	//获取缓冲区的大小，并申请空间，缓冲区大小是按字节计算的
	int len = WideCharToMultiByte(CP_ACP, 0, str, str.GetLength(), NULL, 0, NULL, NULL);
	char* mInstrumentID = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, str, str.GetLength(), mInstrumentID, len, NULL, NULL);
	mInstrumentID[len] = '\0';
	/*
	char m_InstID[30];
	char m_CommodityNo[30];
		string strInstCodeName(mInstrumentID);
		string strCommodityNo=strInstCodeName.substr(strInstCodeName.find_first_of(" ")+1,strInstCodeName.find_last_of(" ")-strInstCodeName.find_first_of(" ")-1);
		string strInstID=strInstCodeName.substr(strInstCodeName.find_last_of(" ")+1,strInstCodeName.length()-strInstCodeName.find_last_of(" "));
		strcpy(m_InstID,strInstID.c_str());
		strcpy(m_CommodityNo,strCommodityNo.c_str());
		*/
	GetDlgItemText(IDC_OpenPrice, str);
	double LimitPrice = _tcstod(str, 0);

	GetDlgItemText(IDC_OpenVol, str);
	int openVol = _ttoi(str);

	//int nIndex = m_cbOpenOrClose.GetCurSel();
	//CString strCBText;
	//m_cbOpenOrClose.GetLBText( nIndex, strCBText);
	//int openOrClose=nIndex;//_ttoi(strCBText);

	int nIndex2 = m_cbDirection.GetCurSel();
	//CString strCBText2;
	//m_cbDirection.GetLBText( nIndex2, strCBText2);
	int buyOrSell = nIndex2;//_ttoi(str);

	int openOrClose = m_cbOpenOrClose.GetCurSel();

	InsertOrderField cOrder;
	strcpy(cOrder.ClientNo, LoginUserTDEsun);
	strcpy(cOrder.CommodityNo, "");
	strcpy(cOrder.InstrumentID, "");
	cOrder.Direction = buyOrSell;
	//cOrder.Offset=openOrClose;
	int nIndexInstCode = m_cbInstCode.GetCurSel();
	CString strCBTextInstCode;
	m_cbInstCode.GetLBText(nIndexInstCode, strCBTextInstCode);

	int instcodealen = WideCharToMultiByte(CP_ACP, 0, strCBTextInstCode, strCBTextInstCode.GetLength(), NULL, 0, NULL, NULL);
	char* cinstcodea = new char[instcodealen + 1];
	WideCharToMultiByte(CP_ACP, 0, strCBTextInstCode, strCBTextInstCode.GetLength(), cinstcodea, instcodealen, NULL, NULL);
	cinstcodea[instcodealen] = '\0';
	string strInstCodeName(cinstcodea);

	double m_dOneTickx = 0.01;

	std::map<string, InstrumentInfo>::iterator itrmap;
	itrmap = InstrumentsSubscribed.find(strInstCodeName);
	char xExchangeID[30];
	if (itrmap != InstrumentsSubscribed.end()) {
		m_dOneTickx = itrmap->second.OneTick;
		strcpy(cOrder.CommodityNo, itrmap->second.CommodityNo);
		strcpy(cOrder.InstrumentID, itrmap->second.InstrumentID);
		strcpy(xExchangeID, itrmap->second.ExchangeID);

		if (openOrClose == 0) {
			cOrder.Offset = MORDER_OPEN;
		}
		else if (openOrClose == 2) {
			cOrder.Offset = MORDER_CLOSETODAY;
		}
		else {
			cOrder.Offset = MORDER_CLOSE;
		}
	}
	else {
		CString strtemp;
		strtemp.Format(_T("行情服务器未订阅此合约,无法报单!"));
		AfxMessageBox(strtemp, MB_OKCANCEL, 0);
		return;
	}

	cOrder.OrderPrice = floor((LimitPrice + 0.00001) / m_dOneTickx) * m_dOneTickx;
	cOrder.OrderVol = openVol;

	//报单失败处理  ？？
	//cerr << "--->>> 报单录入请求: " << iResult << ((iResult == 0) ? ", 成功" : ", 失败") << endl;
	bool confirmOrder = false;
	if (openVol >= 50) {
		CString strtemp;
		strtemp.Format(_T("报单手数为%d,超过50手,确定报单?"), openVol); // pNMListView->iSubItem
		if (IDOK == AfxMessageBox(strtemp, MB_OKCANCEL, 0)) {
			confirmOrder = true;
		}
	}
	else confirmOrder = true;

	int nIndexStra = m_cbManualOrderStra.GetCurSel();
	CString strCBTextStra;
	m_cbManualOrderStra.GetLBText(nIndexStra, strCBTextStra);

	int stralen = WideCharToMultiByte(CP_ACP, 0, strCBTextStra, strCBTextStra.GetLength(), NULL, 0, NULL, NULL);
	char* cstra = new char[stralen + 1];
	WideCharToMultiByte(CP_ACP, 0, strCBTextStra, strCBTextStra.GetLength(), cstra, stralen, NULL, NULL);
	cstra[stralen] = '\0';
	string stStrategyManualOrder(cstra);

	ManualOrder manualOrder;
	strcpy(manualOrder.StrategyName, cstra);
	delete cstra;
	manualOrder.Direction = buyOrSell;
	manualOrder.subprice = LimitPrice;

	int iRetReqID;
	if (confirmOrder) {
		if (nIndexStra != 1) {
			if (strcmp(xExchangeID, "CFFEX") == 0 || strcmp(xExchangeID, "SHFE") == 0
				|| strcmp(xExchangeID, "DCE") == 0 || strcmp(xExchangeID, "CZCE") == 0
				|| strcmp(xExchangeID, "INE") == 0) {
				bool CloseToday = false;
				if (openOrClose == 2) {
					CloseToday = true;
				}
				if (CTPAPIUsed)pThostTraderSpi->ReqOrderInsert(&cOrder, false, CloseToday, -1);
			}
			else {
				if (EsunAPIUsed)pEsunTraderSpi->ReqOrderInsert(&cOrder, &iRetReqID, false);
			}
		}
		else {
			WaitForSingleObject(ambushordersem, INFINITE);
			ambushOrderList.push_back(cOrder);
			ReleaseSemaphore(ambushordersem, 1, NULL);
		}
	}
	AcquireSRWLockExclusive(&g_srwLockReqIdStra);
	ReqIdToStrategyManualOrder.insert(std::pair<int, ManualOrder>(iRetReqID, manualOrder));
	ReleaseSRWLockExclusive(&g_srwLockReqIdStra);

	m_cbManualOrderStra.Clear();
	m_cbManualOrderStra.ResetContent();

	m_cbDirection.Clear();
	m_cbDirection.ResetContent();

	m_cbOpenOrClose.Clear();
	m_cbOpenOrClose.ResetContent();

	m_cbInstCode.Clear();
	m_cbInstCode.ResetContent();

	OnOK();
}