// MdDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "MdDlg.h"
#include "afxdialogex.h"
#include "MyStruct.h"
#include "MyThread.h"
#include "CMdInstMgrDlg.h"
#include "ThostMdSpi.h"
#include "EsunMdSpi.h"

extern CListBox* pMdPubMsg;
extern CListBox* pMdInstList;
extern list<InstrumentsName> OverSeaInstSubscribed;
extern list<InstrumentsName> DomesticInstSubscribed;
extern list<InstrumentsName> SGEInstSubscribed;
extern char LoginMDUser[];
extern char LoginMDPwd[];
extern char LoginMDUserCTP[];
extern char LoginMDPwdCTP[];

extern HANDLE MdTickSem;
extern HANDLE logSemaphore;
extern CMyThread* pLogThread;
extern CMyThread* pEsunMdThread;
extern CMyThread* pCTPMdThread;
extern CMyThread* pSgitMdThread;
extern CMyThread* pMDDispatcherThread;
extern CMyThread* pRestartEsunMdThread;
extern CMyThread* pRestartCTPMdThread;
extern CEsunMdSpi* pEsunMdSpi;
extern CThostMdSpi* pThostMdSpi;
extern bool MDServerConnected;
bool EsunMDReConnnectEveryDay = false;
bool CTPMDReConnnectEveryDay = false;
extern CButton* pConnectTDBTN;
// MdDlg 对话框

IMPLEMENT_DYNAMIC(MdDlg, CDialog)

MdDlg::MdDlg(CWnd* pParent /*=NULL*/)
	: CDialog(MdDlg::IDD, pParent)
{
}

MdDlg::~MdDlg()
{
}

BOOL MdDlg::Create()
{
	CDialog::Create(MdDlg::IDD);
	return TRUE;
}

void MdDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(MdDlg, CDialog)
	ON_BN_CLICKED(IDC_CONNECT_MD_SERVER, &MdDlg::OnBnClickedConnectMdServer)
	ON_COMMAND(ID_MENU_INST_MGR, &MdDlg::OnMenuInstMgr)
	ON_COMMAND(ID_RECONNECT_MENU, &MdDlg::OnReconnectMenu)
END_MESSAGE_MAP()

// MdDlg 消息处理程序
BOOL MdDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作

	// TODO: 在此添加额外的初始化代码
	pMdPubMsg = (CListBox*)GetDlgItem(IDC_MSG_LIST);
	pMdInstList = (CListBox*)GetDlgItem(IDC_INST_LIST);

	//	SetDlgItemText(IDC_USER_NAME,(LPCTSTR)_T("9657010"));
	//	SetDlgItemText(IDC_USER_PWD,(LPCTSTR)_T("Ca842759"));

	SetDlgItemText(IDC_USER_NAME_CTP, (LPCTSTR)_T("123456"));
	SetDlgItemText(IDC_USER_PWD_CTP, (LPCTSTR)_T("123456"));

	pMdPubMsg->SetHorizontalExtent(500);

	TRACE("size=%d,%d\n", sizeof(MapViewType), 2048 * 2048);

	FILE* fp = fopen("InstrumentsIDSub.ini", "r");
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
		if (strcmp(instRec.ExchangeID, "CFFEX") == 0 || strcmp(instRec.ExchangeID, "SHFE") == 0
			|| strcmp(instRec.ExchangeID, "DCE") == 0 || strcmp(instRec.ExchangeID, "CZCE") == 0
			|| strcmp(instRec.ExchangeID, "INE") == 0) {
			DomesticInstSubscribed.push_back(instRec);
		}
		else if (strcmp(instRec.ExchangeID, "SGE") == 0) {
			SGEInstSubscribed.push_back(instRec);
		}
		else OverSeaInstSubscribed.push_back(instRec);

		//		char InstFullName[80];
		//		sprintf(InstFullName, "%s %s %s", instRec.ExchangeID,instRec.CommodityNo,instRec.InstrumentID);
		//		CString csInstFullName(InstFullName);
		//		pInstList->AddString(csInstFullName);

		ptr = fgets(achBuf, 256, fp);
	}
	fclose(fp);

	std::list<InstrumentsName>::iterator inst_itr;
	char InstFullName[80];
	for (inst_itr = DomesticInstSubscribed.begin(); inst_itr != DomesticInstSubscribed.end(); ++inst_itr) {
		sprintf(InstFullName, "%s %s %s", inst_itr->ExchangeID, inst_itr->CommodityNo, inst_itr->InstrumentID);
		CString csInstFullName(InstFullName);
		pMdInstList->AddString(csInstFullName);
	}
	for (inst_itr = SGEInstSubscribed.begin(); inst_itr != SGEInstSubscribed.end(); ++inst_itr) {
		sprintf(InstFullName, "%s %s %s", inst_itr->ExchangeID, inst_itr->CommodityNo, inst_itr->InstrumentID);
		CString csInstFullName(InstFullName);
		pMdInstList->AddString(csInstFullName);
	}
	for (inst_itr = OverSeaInstSubscribed.begin(); inst_itr != OverSeaInstSubscribed.end(); ++inst_itr) {
		sprintf(InstFullName, "%s %s %s", inst_itr->ExchangeID, inst_itr->CommodityNo, inst_itr->InstrumentID);
		CString csInstFullName(InstFullName);
		pMdInstList->AddString(csInstFullName);
	}
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void MdDlg::OnBnClickedConnectMdServer()
{
	// TODO: 在此添加控件通知处理程序代码
	CString csUserName(_T(""));
	//	GetDlgItem(IDC_USER_NAME)->GetWindowText(csUserName);
	CString csUserPwd(_T(""));
	//	GetDlgItem(IDC_USER_PWD)->GetWindowText(csUserPwd);

	CString csUserNameCTP;
	GetDlgItem(IDC_USER_NAME_CTP)->GetWindowText(csUserNameCTP);
	CString csUserPwdCTP;
	GetDlgItem(IDC_USER_PWD_CTP)->GetWindowText(csUserPwdCTP);

	MdTickSem = CreateSemaphore(NULL, 1, 200, NULL);

	CString pubMsg("Connecting...");
	pMdPubMsg->AddString(pubMsg);

	//	GetDlgItem(IDC_USER_NAME)->EnableWindow(FALSE);
	//	GetDlgItem(IDC_USER_PWD)->EnableWindow(FALSE);
	GetDlgItem(IDC_USER_NAME_CTP)->EnableWindow(FALSE);
	GetDlgItem(IDC_USER_PWD_CTP)->EnableWindow(FALSE);

	int len = WideCharToMultiByte(CP_ACP, 0, csUserName, csUserName.GetLength(), NULL, 0, NULL, NULL);
	char* gInvestor_ID = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, csUserName, csUserName.GetLength(), gInvestor_ID, len, NULL, NULL);
	gInvestor_ID[len] = '\0';
#ifdef _TEST
	strcpy(LoginMDUser, "C1171");
#else
	//strcpy(LoginMDUser,"89501009");
	//strcpy(LoginMDUser,"89501008");
	strcpy(LoginMDUser, "89501007");
	//strcpy(LoginMDUser,"2208002103");
#endif

	int pwdlen = WideCharToMultiByte(CP_ACP, 0, csUserPwd, csUserPwd.GetLength(), NULL, 0, NULL, NULL);
	char* gPASSWORD = new char[pwdlen + 1];
	WideCharToMultiByte(CP_ACP, 0, csUserPwd, csUserPwd.GetLength(), gPASSWORD, pwdlen, NULL, NULL);
	gPASSWORD[pwdlen] = '\0';
#ifdef _TEST
	strcpy(LoginMDPwd, "Lh851171");
#else
	//strcpy(LoginMDPwd,"Dk285697");
	strcpy(LoginMDPwd, "Li007007");
	//strcpy(LoginMDPwd,"171028");

#endif

	int lenctp = WideCharToMultiByte(CP_ACP, 0, csUserNameCTP, csUserNameCTP.GetLength(), NULL, 0, NULL, NULL);
	char* gInvestor_IDCTP = new char[lenctp + 1];
	WideCharToMultiByte(CP_ACP, 0, csUserNameCTP, csUserNameCTP.GetLength(), gInvestor_IDCTP, lenctp, NULL, NULL);
	gInvestor_IDCTP[lenctp] = '\0';
	strcpy(LoginMDUserCTP, gInvestor_IDCTP);

	int pwdlenctp = WideCharToMultiByte(CP_ACP, 0, csUserPwdCTP, csUserPwdCTP.GetLength(), NULL, 0, NULL, NULL);
	char* gPASSWORDCTP = new char[pwdlenctp + 1];
	WideCharToMultiByte(CP_ACP, 0, csUserPwdCTP, csUserPwdCTP.GetLength(), gPASSWORDCTP, pwdlenctp, NULL, NULL);
	gPASSWORDCTP[pwdlenctp] = '\0';
	strcpy(LoginMDPwdCTP, gPASSWORDCTP);

	// TODO: 在此添加控件通知处理程序代码
	pLogThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
	Sleep(500);
	pLogThread->PostThreadMessage(WM_CREATE_LOG_THREAD, NULL, NULL);

	pMDDispatcherThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
	Sleep(500);
	pMDDispatcherThread->PostThreadMessage(WM_CREATE_MD_DISPATHER, NULL, NULL);

	if (!OverSeaInstSubscribed.empty()) {
		pEsunMdThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
		Sleep(500);
		pEsunMdThread->PostThreadMessage(WM_CREATE_ESUN_MD, NULL, NULL);
	}

	if (!DomesticInstSubscribed.empty()) {
		pCTPMdThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
		Sleep(500);
		pCTPMdThread->PostThreadMessage(WM_CREATE_CTP_MD, NULL, NULL);
	}

	if (!SGEInstSubscribed.empty()) {
		pSgitMdThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
		Sleep(500);
		pSgitMdThread->PostThreadMessage(WM_CREATE_SGIT_MD, NULL, NULL);
	}

	GetDlgItem(IDC_CONNECT_MD_SERVER)->EnableWindow(FALSE);

	MDServerConnected = true;

	EsunMDReConnnectEveryDay = false;
	CTPMDReConnnectEveryDay = false;

	//写入文件
	FILE* fp = fopen("InstrumentsIDSub.ini", "w+");
	if (fp == NULL) { TRACE("Error in Open ini file"); TRACE("%s\n", strerror(errno)); }
	std::list<InstrumentsName>::iterator inst_itr;
	for (inst_itr = DomesticInstSubscribed.begin(); inst_itr != DomesticInstSubscribed.end(); ++inst_itr) {
		char line[500];
		sprintf(line, "%s,%s,%s,%.5f\n", inst_itr->ExchangeID, inst_itr->CommodityNo, inst_itr->InstrumentID, inst_itr->OneTick);
		fwrite(line, strlen(line), 1, fp);
	}
	for (inst_itr = SGEInstSubscribed.begin(); inst_itr != SGEInstSubscribed.end(); ++inst_itr) {
		char line[500];
		sprintf(line, "%s,%s,%s,%.5f\n", inst_itr->ExchangeID, inst_itr->CommodityNo, inst_itr->InstrumentID, inst_itr->OneTick);
		fwrite(line, strlen(line), 1, fp);
	}
	for (inst_itr = OverSeaInstSubscribed.begin(); inst_itr != OverSeaInstSubscribed.end(); ++inst_itr) {
		char line[500];
		sprintf(line, "%s,%s,%s,%.5f\n", inst_itr->ExchangeID, inst_itr->CommodityNo, inst_itr->InstrumentID, inst_itr->OneTick);
		fwrite(line, strlen(line), 1, fp);
	}
	fclose(fp);

	pConnectTDBTN->EnableWindow(TRUE);
}

void MdDlg::OnMenuInstMgr()
{
	// TODO: 在此添加命令处理程序代码
	CMdInstMgrDlg mCMdInstMgrDlg;

	if (mCMdInstMgrDlg.DoModal() != IDOK)
	{
		return;
	}
}

void MdDlg::OnReconnectMenu()
{
	// TODO: 在此添加命令处理程序代码
	// TODO: 在此添加命令处理程序代码
	CString str("手动重新连接行情...");
	pMdPubMsg->AddString(str);

	if (pEsunMdSpi != NULL) {
		pEsunMdSpi->DisConnect();
	}

	if (pThostMdSpi != NULL) {
		pThostMdSpi->Release();
	}
	if (!OverSeaInstSubscribed.empty()) {
		pRestartEsunMdThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
		Sleep(500);
		pRestartEsunMdThread->PostThreadMessage(WM_RESTART_ESUN_MD, NULL, NULL);
	}

	if (!DomesticInstSubscribed.empty()) {
		pRestartCTPMdThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
		Sleep(500);
		pRestartCTPMdThread->PostThreadMessage(WM_RESTART_CTP_MD, NULL, NULL);
	}
}