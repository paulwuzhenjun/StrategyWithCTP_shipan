// TestTraderApiDlg.cpp : implementation file
//
#include "stdafx.h"
#include "HitTraderApi.h"
#include "HitTraderApiDlg.h"
#include "TestSpi.h"
#include "MyThread.h"
#include "InsertOrderDlg.h"
#include "EnterCTPUserDlg.h"
#include "Strategy.h"
#include <io.h>
#include <string>
#include <map>
#include <algorithm>
#include "InstMgrDlg.h"
#include "OrderDataList.h"
#include "StrategyRecoverDlg.h"
#include "EsunTraderSpi.h"
#include "EsunMdSpi.h"
#include "ThostTraderSpi.h"
#include "GlobalFunc.h"
#include "MessageList.h"
#include "StrategyKDJ.h"
#include "MdDlg.h"
#include "StrategyBandMomnt.h"
#include "StrategyWaveOpen.h"
#include "StrategyWaveOpenAdd.h"
#include "ThostMdSpi.h"
#include "StrategyBar.h"
#include "StrategyUpDownR.h"
#include "StrategyAvgLine.h"
#include "StrategyAvgDown.h"
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
#include "StrategyDT.h"
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern char LoginUserTDEsun[];
extern CListBox* pPubMsg;
extern CListCtrl* pMDMsgDisplay;
extern CStatic* pDay;
extern CStatic* pMoney;
extern CStatic* pBalance;
extern CStatic* pUsername;
extern CStatic* pMoneyCTP;
extern CStatic* pBalanceCTP;
extern CStatic* pUsernameCTP;
extern CButton* pConnectTDBTN;
extern CListCtrl* pParamsList;

extern CMyThread* pLogThread;
extern CMyThread* pEsunTraderThread;
extern CMyThread* pThostTraderThread;
extern CMyThread* pSgitTraderThread;
extern CMyThread* pMdThread;
extern CMyThread* pTdDispatcher;
extern CMyThread* pStrategyThread;
extern CMyThread* pQryActionThread;
extern CMyThread* pCTPMdThread;
extern CMyThread* pRestartEsunMdThread;
extern CMyThread* pUpLoadThread;
extern CMyThread* pUpLoadValueThread;
extern CMyThread* pSendUdpMsThread;
extern CMyThread* pRSTcpLoadThread;
extern CStrategy* gStrategyImpl[MAX_RUNNING_STRATEGY];
extern int gStrategyImplIndex;

extern list<ParamNode> paramslist;
extern list<ModelNode> ModelList;
// CAboutDlg dialog used for App About
extern HANDLE logSemaphore;
extern HANDLE ScreenDisplaySem;
extern HANDLE MainScreenFreshSem;
extern HANDLE DispatchTdSem;
extern HANDLE MDMainScreenFreshSem;
extern HANDLE CreateThreadSem;
extern HANDLE TraderOrderInsertSem;
extern HANDLE MatchNoMapSem;
extern HANDLE OrderInsertSem;
extern HANDLE ambushordersem;
extern HANDLE RecoverStrategyDlgSem;
extern HANDLE localCLSem;
extern HANDLE tradeSemaphore;

extern CString ClassNameShowing;
extern CString StrategyIDShowing;
MapViewType* gMapView;
SRWLOCK  g_srwLockReqId;
SRWLOCK  g_srwLockOrderId;
SRWLOCK  g_srwLockOrderLocalRef;
SRWLOCK  g_srwLockReqIdStra;
SRWLOCK  g_srwLockOrderIdStra;
SRWLOCK  g_srwLockReqIdDirection;
extern int TotalOnHandPosition; //原子操作,故不用锁
extern int MaxTotalOnHandPosition;
extern int TotalCancelTimes; //原子操作,故不用锁
extern int MaxTotalCancelTimes;
int openShmRet;
extern map<string, InstrumentInfo> InstrumentsSubscribed;
extern list<string> InstrumentsSubscribedList;
extern OrderDataList OrderList;
extern bool ParamsChangedOrNot;
extern bool g_bQryOrderSentByRecoverDlg;
extern bool ClearInstanceCfgFile;

list<CMyThread*> mStraThreadStarted;
extern map<string, int> InstanceToShmindexMap;
extern EsunTraderSpi* pEsunTraderSpi;
extern ThostTraderSpi* pThostTraderSpi;
extern CThostMdSpi* pThostMdSpi;
extern CEsunMdSpi* pEsunMdSpi;

bool CTPTDReConnnectEveryDay = false;
bool ReleasedAction = false;
bool CTPReleasedAction = false;
bool EsunTDReConnnectEveryDay = false;
bool EsunTDReleasedAction = false;
bool EsunMDReleasedAction = false;
extern bool EsunAPIUsed;
extern bool CTPAPIUsed;
extern bool SgitAPIUsed;
extern GlobalFunc globalFuncUtil;
extern MessageList LogMessageList;
extern map<int, int> OrderLocalRefToShmIndex;
extern map<int, int> OrderIdToShmIndex;

extern GlobalFunc globalFuncUtil;
bool StrategyResetAllow = true;
bool StrategyInitAllow = false;

extern char LoginUserTDEsun[];
extern char RSServerIP[];
extern bool gEndSendUdp;
bool g_bQryMoney = false;
extern int gBuyPosition;
extern int gSellPosition;

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:

public:
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

// CHitTraderApiDlg dialog

CHitTraderApiDlg::CHitTraderApiDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHitTraderApiDlg::IDD, pParent), m_testString("hello12")
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	insertOrderDlg = new CInsertOrderDlg(this);
	mMdDlg = new MdDlg(this);
	//m_testString = new CString("hello1");
}

CHitTraderApiDlg::~CHitTraderApiDlg()
{
	//查看启动的策略线程的状态,释放共享区的锁
	while (!mStraThreadStarted.empty()) {
		if (mStraThreadStarted.front() != NULL) {
			CMyThread* xTmp = mStraThreadStarted.front();
			DWORD retCode;
			GetExitCodeThread(xTmp->m_hThread, &retCode);
			if (retCode == STILL_ACTIVE) {
				TerminateThread(xTmp->m_hThread, 0);
			}
		}
		mStraThreadStarted.pop_front();
	}

	if (insertOrderDlg != NULL)free(insertOrderDlg);
	if (mMdDlg != NULL)free(mMdDlg);
}

void CHitTraderApiDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PUBLIC_MSG_LIST, m_ctrlPublisMsg);
	DDX_Control(pDX, IDC_STRATEGY_TREE, m_webTree);
	DDX_Control(pDX, IDC_MAINTAB, m_tab);
}

BEGIN_MESSAGE_MAP(CHitTraderApiDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_PUBMSG, OnPubmsg)
	ON_BN_CLICKED(IDC_BTNConnect, &CHitTraderApiDlg::OnBnClickedConnect)
	ON_BN_CLICKED(IDC_START_STRATEGY_BTN, &CHitTraderApiDlg::OnBnClickedStartStrategy)
	ON_BN_CLICKED(IDC_STOP_STRATEGY_BTN, &CHitTraderApiDlg::OnBnClickedStopStrategy)
	ON_BN_CLICKED(IDC_REFRESH_ORDER_COUNT_BTN, &CHitTraderApiDlg::OnBnClickedRefreshAvailMoney)
	ON_BN_CLICKED(IDOK, &CHitTraderApiDlg::OnBnOK)
	ON_BN_CLICKED(IDCANCEL, &CHitTraderApiDlg::OnBnCancel)
	ON_COMMAND(ID_MENU_INSERT_ORDER, OnMenuInsertOrder)
	//	ON_BN_CLICKED(IDC_SCHEDULE_BTN, &CHitTraderApiDlg::OnBnClickedScheduleBtn)
	ON_COMMAND(ID_MENU_OPTIONS, &CHitTraderApiDlg::OnMenuOptions)
	ON_NOTIFY(TVN_SELCHANGED, IDC_STRATEGY_TREE, &CHitTraderApiDlg::OnTvnSelchangedStrategyTree)
	ON_NOTIFY(TCN_SELCHANGE, IDC_MAINTAB, &CHitTraderApiDlg::OnTcnSelchangeMaintab)
	ON_BN_CLICKED(IDC_SET_POSITION_LIMIT_BTN, &CHitTraderApiDlg::OnBnClickedSetPositionLimitBtn)
	ON_COMMAND(ID_MENU_PROFIT_ANALYZE, &CHitTraderApiDlg::OnMenuProfitAnalyze)
	ON_COMMAND(ID_RECONNECT_TD, &CHitTraderApiDlg::OnReconnectTd)
	ON_COMMAND(ID_MD_DLG, &CHitTraderApiDlg::OnMdDlg)
	ON_COMMAND(ID_ST_RECOVER, &CHitTraderApiDlg::OnStRecover)
END_MESSAGE_MAP()

// CHitTraderApiDlg message handlers

BOOL CHitTraderApiDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	TRACE("tdmsgd=%d,%d\n", sizeof(OrderTradeMsg), sizeof(ShmMsg));
	//初始化各模型策略信息
	char beginrun_date[20];
	char beginrun_datetime[20];
	struct tm* ptTm;
	time_t nowtime;
	memset(&beginrun_date, 0, 20);
	time(&nowtime);
	nowtime += 60;
	ptTm = localtime(&nowtime);
	strftime(beginrun_date, 20, "%Y-%m-%d", ptTm);
	strftime(beginrun_datetime, 20, "%Y-%m-%d %X", ptTm);

	char beginrun_date_nextday[20];
	nowtime += 86400;
	ptTm = localtime(&nowtime);
	strftime(beginrun_date_nextday, 20, "%Y-%m-%d", ptTm);

	char lastModelName[50] = "";
	char lastStrategyID[50] = "";
	char lastInstanceName[50] = "";
	FILE* fp = fopen("Model.ini", "r");
	if (fp == NULL) { TRACE("Error in Open ini file"); TRACE("%s \n", strerror(errno)); }

	char* ptr;
	char achBuf[256] = { 0 };
	ptr = fgets(achBuf, 256, fp);
	bool DomesticInst = false;
	while (ptr != NULL) {
		char* column[8], * p = achBuf;
		int i;
		for (i = 0; i < 8; i++) {
			column[i] = p;
			if ((p = strchr(p, ',')) == NULL)break;
			else *p++ = '\0';
		}
		strtok(column[7], "\n");

		if (strcmp(column[5], "InstCodeName") == 0) {
			string strInstCodeTmp(column[7]);
			if (strInstCodeTmp.find("CFFEX ") != std::string::npos || strInstCodeTmp.find("SHFE ") != std::string::npos
				|| strInstCodeTmp.find("DCE ") != std::string::npos || strInstCodeTmp.find("CZCE ") != std::string::npos
				|| strInstCodeTmp.find("SGE ") != std::string::npos) {
				DomesticInst = true;
			}
			else DomesticInst = false;
		}

		if (strcmp(column[5], "StartTime") == 0) {
			//string s(column[6]);
			//string timepart=s.substr(s.find_first_of(" ")+1,s.length()-s.find_first_of(" "));
			string datetimepart(beginrun_datetime);
			//datetimepart.append(" ");
			//datetimepart.append(timepart);
			strcpy(column[7], datetimepart.c_str());
		}
		else if (strcmp(column[5], "EndTime") == 0) {
			string s(column[6]);
			string timepart = s.substr(s.find_first_of(" ") + 1, s.length() - s.find_first_of(" "));
			string datetimepart("");
			//datetimepart.append(timepart);
			if (DomesticInst) {
				datetimepart.append(beginrun_date);
				datetimepart.append(" ");
				datetimepart.append("23:00:00");
			}
			else {
				datetimepart.append(beginrun_date_nextday);
				datetimepart.append(" ");
				datetimepart.append("12:00:00");
			}

			strcpy(column[7], datetimepart.c_str());
		}

		if (ModelList.empty()) {
			ModelNode model;
			strcpy(model.ModelName, column[0]);
			StrategyNode strategy;
			strcpy(strategy.StrategyName, column[1]);
			strcpy(strategy.StrategyID, column[2]);
			strategy.MaxOnHandPositionCount = atoi(column[3]);
			StrategyInstanceNode instance;
			instance.StrategyStarted = false;
			instance.shmindex = -1;
			strcpy(instance.InstanceName, column[4]);
			ParamNode paramnode;
			strcpy(paramnode.ParamName, column[5]);
			strcpy(paramnode.ParamChineseName, column[6]);
			strcpy(paramnode.ParamValue, column[7]);
			//paramnode.ParamValue=atof(column[4]);
			instance.ParamList.push_back(paramnode);
			strategy.InstanceList.push_back(instance);
			strategy.ParamENGNameList.push_back(paramnode.ParamName);
			strategy.ParamCHNNameList.push_back(paramnode.ParamChineseName);
			model.StrategyList.push_back(strategy);
			ModelList.push_back(model);
		}
		else {
			if (strcmp(column[0], lastModelName) == 0) {
				if (strcmp(column[2], lastStrategyID) == 0) {
					if (strcmp(column[4], lastInstanceName) == 0) {
						ParamNode paramnode;
						strcpy(paramnode.ParamName, column[5]);
						strcpy(paramnode.ParamChineseName, column[6]);
						strcpy(paramnode.ParamValue, column[7]);

						string strparamName(paramnode.ParamName);
						list<string>::iterator it = find(ModelList.back().StrategyList.back().ParamENGNameList.begin(), ModelList.back().StrategyList.back().ParamENGNameList.end(), strparamName);
						if (it == ModelList.back().StrategyList.back().ParamENGNameList.end()) {
							ModelList.back().StrategyList.back().ParamENGNameList.push_back(paramnode.ParamName);
							ModelList.back().StrategyList.back().ParamCHNNameList.push_back(paramnode.ParamChineseName);
						}

						ModelList.back().StrategyList.back().InstanceList.back().ParamList.push_back(paramnode);
					}
					else {
						//新实例Node
						ParamNode paramnode;
						strcpy(paramnode.ParamName, column[5]);
						strcpy(paramnode.ParamChineseName, column[6]);
						strcpy(paramnode.ParamValue, column[7]);

						string strparamName(paramnode.ParamName);
						list<string>::iterator it = find(ModelList.back().StrategyList.back().ParamENGNameList.begin(), ModelList.back().StrategyList.back().ParamENGNameList.end(), strparamName);
						if (it == ModelList.back().StrategyList.back().ParamENGNameList.end()) {
							ModelList.back().StrategyList.back().ParamENGNameList.push_back(paramnode.ParamName);
							ModelList.back().StrategyList.back().ParamCHNNameList.push_back(paramnode.ParamChineseName);
						}

						StrategyInstanceNode instance;
						strcpy(instance.InstanceName, column[4]);
						instance.StrategyStarted = false;
						instance.shmindex = -1;
						instance.ParamList.push_back(paramnode);
						ModelList.back().StrategyList.back().InstanceList.push_back(instance);
					}
				}
				else {
					//新策略Node
					ParamNode paramnode;
					strcpy(paramnode.ParamName, column[5]);
					strcpy(paramnode.ParamChineseName, column[6]);
					strcpy(paramnode.ParamValue, column[7]);

					StrategyNode strategy;
					strcpy(strategy.StrategyName, column[1]);
					strcpy(strategy.StrategyID, column[2]);
					strategy.MaxOnHandPositionCount = atoi(column[3]);
					StrategyInstanceNode instance;
					instance.StrategyStarted = false;
					instance.shmindex = -1;
					strcpy(instance.InstanceName, column[4]);
					instance.ParamList.push_back(paramnode);
					strategy.InstanceList.push_back(instance);

					strategy.ParamENGNameList.push_back(paramnode.ParamName);
					strategy.ParamCHNNameList.push_back(paramnode.ParamChineseName);

					ModelList.back().StrategyList.push_back(strategy);
				}
			}
			else {
				//新模型Node
				ModelNode model;
				strcpy(model.ModelName, column[0]);
				StrategyNode strategy;
				strcpy(strategy.StrategyName, column[1]);
				strcpy(strategy.StrategyID, column[2]);
				strategy.MaxOnHandPositionCount = atoi(column[3]);
				StrategyInstanceNode instance;
				strcpy(instance.InstanceName, column[4]);
				instance.StrategyStarted = false;
				instance.shmindex = -1;
				ParamNode paramnode;
				strcpy(paramnode.ParamName, column[5]);
				strcpy(paramnode.ParamChineseName, column[6]);
				strcpy(paramnode.ParamValue, column[7]);
				instance.ParamList.push_back(paramnode);
				strategy.InstanceList.push_back(instance);
				strategy.ParamENGNameList.push_back(paramnode.ParamName);
				strategy.ParamCHNNameList.push_back(paramnode.ParamChineseName);

				model.StrategyList.push_back(strategy);
				ModelList.push_back(model);
			}
		}

		strcpy(lastModelName, column[0]);
		strcpy(lastStrategyID, column[2]);
		strcpy(lastInstanceName, column[4]);
		ptr = fgets(achBuf, 256, fp);
	}

	fclose(fp);

	//Save all Params to file
	FILE* fp2 = fopen("Model.ini", "w+");
	if (fp2 == NULL) { TRACE("Error in Open ini file"); TRACE("%s\n", strerror(errno)); }
	for (std::list<ModelNode>::iterator model_itr = ModelList.begin(); model_itr != ModelList.end(); ++model_itr) {
		std::list<StrategyNode>::iterator strategy_itr;
		for (strategy_itr = model_itr->StrategyList.begin(); strategy_itr != model_itr->StrategyList.end(); ++strategy_itr) {
			std::list<StrategyInstanceNode>::iterator instance_itr;
			for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
				std::list<ParamNode>::iterator param_itr;
				for (param_itr = instance_itr->ParamList.begin(); param_itr != instance_itr->ParamList.end(); ++param_itr) {
					char line[500];
					sprintf(line, "%s,%s,%s,%d,%s,%s,%s,%s\n", model_itr->ModelName, strategy_itr->StrategyName, strategy_itr->StrategyID, strategy_itr->MaxOnHandPositionCount, instance_itr->InstanceName, param_itr->ParamName, param_itr->ParamChineseName, param_itr->ParamValue);
					fwrite(line, strlen(line), 1, fp2);
				}
			}
		}
	}
	fclose(fp2);
	//_tsetlocale( LC_CTYPE, old_locale );
   // free( old_locale );
	//初始化各策略的参数列表

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	pPubMsg = (CListBox*)GetDlgItem(IDC_PUBLIC_MSG_LIST);
	pMDMsgDisplay = (CListCtrl*)GetDlgItem(IDC_MD_MSG_LIST);

	pDay = (CStatic*)GetDlgItem(IDC_DAY);
	pMoney = (CStatic*)GetDlgItem(IDC_MONEY);
	pBalance = (CStatic*)GetDlgItem(IDC_BALANCE);
	pUsername = (CStatic*)GetDlgItem(IDC_USERNAME);
	pMoneyCTP = (CStatic*)GetDlgItem(IDC_MONEY_CTP);
	pBalanceCTP = (CStatic*)GetDlgItem(IDC_BALANCE_CTP);
	pUsernameCTP = (CStatic*)GetDlgItem(IDC_USERNAME_CTP);
	pConnectTDBTN = (CButton*)GetDlgItem(IDC_BTNConnect);

	// TODO: Add extra initialization here
	//初始化树形图
	HICON hIcon[3];      // 图标句柄数组
	HTREEITEM hRoot;     // 树的根节点的句柄
	HTREEITEM hModelItem; // 可表示任一分类节点的句柄
	HTREEITEM hArtItem;  // 可表示任一文章节点的句柄

	// 加载三个图标，并将它们的句柄保存到数组
	hIcon[0] = theApp.LoadIcon(IDI_STRATEGY_ICON);
	hIcon[1] = theApp.LoadIcon(IDI_STRATEGY_ICON);
	hIcon[2] = theApp.LoadIcon(IDI_STRATEGY_ICON);

	// 创建图像序列CImageList对象
	m_imageList.Create(16, 16, ILC_COLOR32, 3, 3);
	// 将三个图标添加到图像序列
	for (int i = 0; i < 3; i++)
	{
		m_imageList.Add(hIcon[i]);
	}

	// 为树形控件设置图像序列
	m_webTree.SetImageList(&m_imageList, TVSIL_NORMAL);

	CString strPathFile, strPathFile2;
	::GetModuleFileName(NULL, strPathFile.GetBuffer(MAX_PATH), MAX_PATH);
	strPathFile.ReleaseBuffer();
	strPathFile = strPathFile.Left(strPathFile.ReverseFind(_T('\\')));
	strPathFile2 = strPathFile;

	// 插入根节点
	hRoot = m_webTree.InsertItem(_T("策略列表"), 0, 0);

	strPathFile += "\\Strategies";
	CheckAndCreateDirectory(strPathFile);

	strPathFile2 += "\\Data";
	CheckAndCreateDirectory(strPathFile2);

	int ItermNum = 0;
	std::list<ModelNode>::iterator model_itr;
	if (!ModelList.empty()) {
		for (model_itr = ModelList.begin(); model_itr != ModelList.end(); ++model_itr) {
			CString ModelName(model_itr->ModelName);
			hModelItem = m_webTree.InsertItem(ModelName, 1, 1, hRoot, TVI_LAST);
			ItermNum++;
			m_webTree.SetItemData(hModelItem, ItermNum);
			CString csModel1Path;
			csModel1Path.Append(strPathFile);
			csModel1Path += "\\";
			csModel1Path.Append(ModelName);
			CheckAndCreateDirectory(csModel1Path);
			std::list<StrategyNode>::iterator strategy_itr;
			if (!model_itr->StrategyList.empty()) {
				for (strategy_itr = model_itr->StrategyList.begin(); strategy_itr != model_itr->StrategyList.end(); ++strategy_itr) {
					CString StrategyID(strategy_itr->StrategyID);
					hArtItem = m_webTree.InsertItem(StrategyID, 2, 2, hModelItem, TVI_LAST);
					ItermNum++;
					m_webTree.SetItemData(hArtItem, ItermNum);
					CString csStrategy1Path;
					csStrategy1Path.Append(csModel1Path);
					csStrategy1Path += "\\";
					csStrategy1Path.Append(StrategyID);
					CheckAndCreateDirectory(csStrategy1Path);
				}
			}
		}
	}

	m_tab.InsertItem(0, _T("参数管理"));
	m_tab.InsertItem(1, _T("委托管理"));
	m_tab.InsertItem(2, _T("持仓管理"));
	m_tab.InsertItem(3, _T("策略在手开平信息"));
	m_tab.InsertItem(4, _T("成交信息"));

	m_ParamsMgrPage.Create(IDD_PARAMS_DLG, &m_tab);
	m_OrdersMgrPage.Create(IDD_ORDERS_DLG, &m_tab);
	m_PositionsMgrPage.Create(IDD_POSITIONS_DLG, &m_tab);
	m_LocalCLMgrDlgPage.Create(IDD_STRATEGY_LOCAL_CL, &m_tab);
	m_TradesDisplayDlgPage.Create(IDD_TRADES_DLG, &m_tab);
	//设定在Tab内显示的范围
	CRect rc;
	m_tab.GetClientRect(rc);
	rc.top += 20;
	rc.bottom -= 0;
	rc.left += 0;
	rc.right -= 0;
	m_ParamsMgrPage.MoveWindow(&rc);
	m_OrdersMgrPage.MoveWindow(&rc);
	m_PositionsMgrPage.MoveWindow(&rc);
	m_LocalCLMgrDlgPage.MoveWindow(&rc);
	m_TradesDisplayDlgPage.MoveWindow(&rc);

	//把对话框对象指针保存起来
	pDialog[0] = &m_ParamsMgrPage;
	pDialog[1] = &m_OrdersMgrPage;
	pDialog[2] = &m_PositionsMgrPage;
	pDialog[3] = &m_LocalCLMgrDlgPage;
	pDialog[4] = &m_TradesDisplayDlgPage;
	//显示初始页面
	pDialog[0]->ShowWindow(SW_SHOW);
	pDialog[1]->ShowWindow(SW_HIDE);
	pDialog[2]->ShowWindow(SW_HIDE);
	pDialog[3]->ShowWindow(SW_HIDE);
	pDialog[4]->ShowWindow(SW_HIDE);
	//保存当前选择
	m_CurSelTab = 0;

	logSemaphore = CreateSemaphore(NULL, 10, 200, NULL);
	ScreenDisplaySem = CreateSemaphore(NULL, 0, 500, NULL);
	DispatchTdSem = CreateSemaphore(NULL, 0, 500, NULL);
	MainScreenFreshSem = CreateSemaphore(NULL, 1, 1, NULL);
	MDMainScreenFreshSem = CreateSemaphore(NULL, 1, 1, NULL);
	OrderInsertSem = CreateSemaphore(NULL, 0, 100, NULL);
	tradeSemaphore = CreateSemaphore(NULL, 1, 100, NULL);

	CreateThreadSem = CreateSemaphore(NULL, 1, 1, NULL);
	MatchNoMapSem = CreateSemaphore(NULL, 1, 1, NULL);
	ambushordersem = CreateSemaphore(NULL, 1, 1, NULL);

	RecoverStrategyDlgSem = CreateSemaphore(NULL, 0, 10, NULL);
	localCLSem = CreateSemaphore(NULL, 1, 1, NULL);

	//初始化读写锁
	InitializeSRWLock(&g_srwLockReqId);
	InitializeSRWLock(&g_srwLockOrderId);
	InitializeSRWLock(&g_srwLockOrderLocalRef);
	InitializeSRWLock(&g_srwLockReqIdStra);
	InitializeSRWLock(&g_srwLockOrderIdStra);
	InitializeSRWLock(&g_srwLockReqIdDirection);

	pPubMsg->SetHorizontalExtent(650);

	CRect rect;
	GetClientRect(&rect);     //取客户区大小
	old.x = rect.right - rect.left;
	old.y = rect.bottom - rect.top;

	GetDlgItem(IDC_START_STRATEGY_BTN)->EnableWindow(TRUE);
	GetDlgItem(IDC_STOP_STRATEGY_BTN)->EnableWindow(TRUE);

	CString MaxPosition("");
	MaxPosition.AppendFormat(_T("%d"), MaxTotalOnHandPosition);
	SetDlgItemText(IDC_MAX_POSITION, MaxPosition);
	GetDlgItem(IDC_STRATEGY_MAX_POSITION)->EnableWindow(FALSE);

	CString MaxCancelTime("");
	MaxCancelTime.AppendFormat(_T("%d"), MaxTotalCancelTimes);
	SetDlgItemText(IDC_MAX_CANCELTIME, MaxCancelTime);
	GetDlgItem(IDC_STRATEGY_MAX_POSITION)->EnableWindow(FALSE);

	CString csRSServerIP(RSServerIP);
	SetDlgItemText(IDC_RS_SERVER_IP, csRSServerIP);

	m_OrdersMgrPage.GetDlgItem(IDC_QRY_ORDERS_BTN)->EnableWindow(FALSE);
	m_PositionsMgrPage.GetDlgItem(IDC_QRY_POSITIONS_BTN)->EnableWindow(FALSE);
	GetDlgItem(IDC_REFRESH_ORDER_COUNT_BTN)->EnableWindow(FALSE);
	GetDlgItem(IDC_START_STRATEGY_BTN)->EnableWindow(FALSE);
	GetDlgItem(IDC_STOP_STRATEGY_BTN)->EnableWindow(FALSE);
	m_LocalCLMgrDlgPage.GetDlgItem(IDC_QRY_STRA_LOCAL_CL_BTN)->EnableWindow(FALSE);

	pMDMsgDisplay->DeleteAllItems();
	pMDMsgDisplay->ModifyStyle(0, LVS_REPORT);
	LV_COLUMN column1;
	column1.pszText = _T("模型");
	column1.mask = LVCF_TEXT;
	pMDMsgDisplay->InsertColumn(0, &column1); //插入一列，列索引为0

	column1.pszText = _T("实例");
	column1.mask = LVCF_TEXT;
	pMDMsgDisplay->InsertColumn(1, &column1);

	column1.pszText = _T("合约");
	column1.mask = LVCF_TEXT;
	pMDMsgDisplay->InsertColumn(2, &column1);
	//m_list->SetColumnWidth(0,100); //列宽设置

	column1.pszText = _T("最新价");
	column1.mask = LVCF_TEXT;
	pMDMsgDisplay->InsertColumn(3, &column1);
	CRect rectx;
	pMDMsgDisplay->GetClientRect(rectx);                     //获得当前客户区信息
	pMDMsgDisplay->SetColumnWidth(0, rectx.Width() / 4);        //设置列的宽度。
	pMDMsgDisplay->SetColumnWidth(1, rectx.Width() / 4);
	pMDMsgDisplay->SetColumnWidth(2, rectx.Width() / 4);
	pMDMsgDisplay->SetColumnWidth(3, rectx.Width() / 4);

	pConnectTDBTN->EnableWindow(FALSE);
	//   pRSTcpLoadThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
	   //Sleep(500);
	   //pRSTcpLoadThread->PostThreadMessage(WM_CREATE_RM_TCP,NULL,NULL);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CHitTraderApiDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	// TODO: Add your message handler code here
	if (nType == SIZE_RESTORED || nType == SIZE_MAXIMIZED)
	{
		ReSize();
	}
}

void CHitTraderApiDlg::ReSize()
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

	LONG xy;
	xy = MAKELONG(Newp.x, Newp.y);
	HWND hwnd1 = m_ParamsMgrPage.GetSafeHwnd();
	::PostMessage(hwnd1, WM_SIZE, WPARAM(SIZE_MAXSHOW), (LPARAM)xy);

	HWND hwnd2 = m_OrdersMgrPage.GetSafeHwnd();
	::PostMessage(hwnd2, WM_SIZE, WPARAM(SIZE_MAXSHOW), (LPARAM)xy);

	HWND hwnd3 = m_PositionsMgrPage.GetSafeHwnd();
	::PostMessage(hwnd3, WM_SIZE, WPARAM(SIZE_MAXSHOW), (LPARAM)xy);

	HWND hwnd4 = m_LocalCLMgrDlgPage.GetSafeHwnd();
	::PostMessage(hwnd4, WM_SIZE, WPARAM(SIZE_MAXSHOW), (LPARAM)xy);

	HWND hwnd5 = m_TradesDisplayDlgPage.GetSafeHwnd();
	::PostMessage(hwnd5, WM_SIZE, WPARAM(SIZE_MAXSHOW), (LPARAM)xy);

	CRect rc;
	m_tab.GetClientRect(rc);
	TRACE("%ld,%ld\n", rc.bottom - rc.top, rc.right - rc.left);
	rc.top += 20;
	rc.bottom -= 0;
	rc.left += 0;
	rc.right -= 0;
	m_ParamsMgrPage.MoveWindow(&rc);
	m_OrdersMgrPage.MoveWindow(&rc);
	m_PositionsMgrPage.MoveWindow(&rc);
	m_LocalCLMgrDlgPage.MoveWindow(&rc);
	m_TradesDisplayDlgPage.MoveWindow(&rc);
	//
	if (pMDMsgDisplay != NULL) {
		CRect rectx;
		pMDMsgDisplay->GetClientRect(rectx);                     //获得当前客户区信息
		pMDMsgDisplay->SetColumnWidth(0, rectx.Width() / 4);        //设置列的宽度。
		pMDMsgDisplay->SetColumnWidth(1, rectx.Width() / 4);
		pMDMsgDisplay->SetColumnWidth(2, rectx.Width() / 4);
		pMDMsgDisplay->SetColumnWidth(3, rectx.Width() / 4);
	}
}

int CHitTraderApiDlg::Split(CString source, CString ch, CStringArray& strarr)
{
	/*---------------------------------------------------------

	* 函数介绍： 从原字符串里按照指定的分隔字符串进行分割,将分隔的结果存放到字符串数组里

	* 输入参数：
	source -- 原字符串
	ch -- 指定的分隔字符串
	strarr -- 外部引用的一个字符串数组

	* 输出参数：

	* 返回值 ：总共分隔了多少段字符串.

	-----------------------------------------------------------*/
	CString TmpStr;
	strarr.RemoveAll();
	if (source.IsEmpty() || ch.IsEmpty())
		return 0;
	int len = ch.GetLength();
	int findi = 0;
	int findn = 0;
	int sum = 0;

	findn = source.Find(ch, findi);
	if (findn != -1)
	{
		TmpStr = source.Mid(0, findn);
		//TmpStr.Trim();
		strarr.Add(TmpStr);
		findi = findn + len;
		sum++;
	}
	else
	{
		//source.Trim();
		strarr.Add(source);
		sum++;
		return sum;
	}
	while (findn != -1)//有发现
	{
		findn = source.Find(ch, findi);
		if (findn != -1)
		{
			TmpStr = source.Mid(findi, findn - findi);
			//TmpStr.Trim();//去除头尾空格
			strarr.Add(TmpStr);

			findi = findn + len;
			sum++;
		}
		else
		{
			TmpStr = source.Mid(findi, source.GetLength() - findi);
			//TmpStr.Trim();
			strarr.Add(TmpStr);
			sum++;
		}
	}

	return sum;
}

void CHitTraderApiDlg::CheckAndCreateDirectory(CString csDirectionName)
{
	int len = WideCharToMultiByte(CP_ACP, 0, csDirectionName, csDirectionName.GetLength(), NULL, 0, NULL, NULL);
	char* c_str_filename = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, csDirectionName, csDirectionName.GetLength(), c_str_filename, len, NULL, NULL);
	c_str_filename[len] = '\0';
	if (_access(c_str_filename, 0) != 0) {
		CreateDirectory(csDirectionName, NULL);
	}
	free(c_str_filename);
}
int g_nCount = 0;
int g_nDate = 0;
void CHitTraderApiDlg::OnTimer(UINT nIDEvent) {
	CTime mCurrTime = CTime::GetCurrentTime();
	int nHour = mCurrTime.GetHour();
	int nMin = mCurrTime.GetMinute();

	bool CTPExist = false;
	bool EsunExist = false;
	std::map<string, InstrumentInfo>::iterator map_itr;
	CTime currenTime;
	char cDate[32] = { 0 };

	switch (nIDEvent)
	{
	case 1:
		for (map_itr = InstrumentsSubscribed.begin(); map_itr != InstrumentsSubscribed.end(); ++map_itr) {
			if (strcmp(map_itr->second.ExchangeID, "CFFEX") == 0 || strcmp(map_itr->second.ExchangeID, "SHFE") == 0
				|| strcmp(map_itr->second.ExchangeID, "DCE") == 0 || strcmp(map_itr->second.ExchangeID, "CZCE") == 0
				|| strcmp(map_itr->second.ExchangeID, "INE") == 0) {
				CTPExist = true;
			}
			else {
				EsunExist = true;
			}
		}

		//释放Esun
		sprintf(cDate, "%04d%02d%02d", mCurrTime.GetYear(), mCurrTime.GetMonth(), mCurrTime.GetDay());
		if (((nHour == 5 && nMin >= 5 && nMin < 40)) && !EsunTDReleasedAction && EsunExist && 0 == g_nCount && g_nDate != atoi(cDate)) {
			EsunTDReleasedAction = true;
			EsunMDReleasedAction = true;
			g_nCount++;
			g_nDate = atoi(cDate);
			if (pEsunTraderSpi != NULL && EsunAPIUsed) {
				pEsunTraderSpi->Release();
				globalFuncUtil.WriteMsgToLogList("pEsunTraderSpi->Release()...");
			}
			if (pEsunMdSpi != NULL && EsunAPIUsed) {
				pEsunMdSpi->DisConnect();
			}
			pEsunTraderSpi = NULL;

			if (StrategyResetAllow) {
				StrategyResetAction();
				StrategyResetAllow = false;
				StrategyInitAllow = true;
			}
			globalFuncUtil.WriteMsgToLogList("Esun Release() Done...");
		}

		//if(((nHour==3)||(nHour==16))&&StrategyResetAllow){
		//	StrategyResetAction();
		//	StrategyResetAllow=false;
		//	StrategyInitAllow=true;
		//}

		if (1 == g_nCount && ((nHour == 5 && nMin >= 45 && nMin < 50)) && !EsunTDReConnnectEveryDay && EsunExist && EsunTDReleasedAction && (mCurrTime.GetDayOfWeek() != 1 && mCurrTime.GetDayOfWeek() != 7)) {
			CString str("每天5:40自动重新连接Esun交易服务器...");
			pPubMsg->AddString(str);
			globalFuncUtil.WriteMsgToLogList("每天5:40自动重新连接Esun交易服务器...");

			if (StrategyInitAllow) {
				StrategyInitAction();
				StrategyInitAllow = false;
				StrategyResetAllow = true;
			}

			pRestartEsunMdThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
			Sleep(500);
			pRestartEsunMdThread->PostThreadMessage(WM_RESTART_ESUN_MD, NULL, NULL);

			pEsunTraderThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
			Sleep(500);
			pEsunTraderThread->PostThreadMessage(WM_CREATE_ESUN_TRADER, NULL, NULL);
			Sleep(2000);

			//StrategyResetAllow=true;
			EsunTDReleasedAction = false;
			EsunMDReleasedAction = false;
			EsunTDReConnnectEveryDay = true;
			g_nCount = 0;
		}
		else if ((nHour > 8 && nHour != 19) || nHour > 19) {
			EsunTDReConnnectEveryDay = false;
		}

		if (((nHour == 2 && nMin >= 35 && nMin <= 55) || (nHour == 15 && nMin >= 30 && nMin <= 50)) && !CTPReleasedAction && CTPExist) {
			if (pThostTraderSpi != NULL && CTPAPIUsed) {
				pThostTraderSpi->Release();
			}
			if (pThostMdSpi != NULL && CTPAPIUsed) {
				pThostMdSpi->Release();
			}

			pThostTraderSpi = NULL;
			pThostMdSpi = NULL;
			CTPReleasedAction = true;
			CTPTDReConnnectEveryDay = false;
		}

		if (((nHour == 8 && nMin >= 45 && nMin <= 50) || (nHour == 20 && nMin >= 50 && nMin <= 55)) && !CTPTDReConnnectEveryDay && CTPExist && CTPReleasedAction
			&& (mCurrTime.GetDayOfWeek() != 1 && mCurrTime.GetDayOfWeek() != 7)) {
			CString str("每天8:45/20:45自动重新连接CTP交易服务器...");
			pPubMsg->AddString(str);

			if (nHour == 20 && nMin >= 45 && nMin <= 50) {
				//StrategyResetAction();
				OrderLocalRefToShmIndex.clear();
			}
			//StrategyResetAllow=true;
			StrategyInitAction();
			CTPReleasedAction = false;
			CTPTDReConnnectEveryDay = true;
			//if(StrategyInitAllow){
			//	StrategyInitAction();
			//	StrategyInitAllow=false;
			//}

			pThostTraderThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
			Sleep(500);
			pThostTraderThread->PostThreadMessage(WM_CREATE_CTP_TRADER, NULL, NULL);
			Sleep(2000);

			pCTPMdThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
			Sleep(500);
			pCTPMdThread->PostThreadMessage(WM_CREATE_CTP_MD, NULL, NULL);
		}/*else if((nHour>8&&nHour!=20)||nHour>20){
			CTPTDReConnnectEveryDay=false;
		}*/
		break;
	case 2:
		for (map_itr = InstrumentsSubscribed.begin(); map_itr != InstrumentsSubscribed.end(); ++map_itr) {
			if (strcmp(map_itr->second.ExchangeID, "CFFEX") == 0 || strcmp(map_itr->second.ExchangeID, "SHFE") == 0
				|| strcmp(map_itr->second.ExchangeID, "DCE") == 0 || strcmp(map_itr->second.ExchangeID, "CZCE") == 0
				|| strcmp(map_itr->second.ExchangeID, "INE") == 0) {
				CTPExist = true;
			}
			else {
				EsunExist = true;
			}
		}
		// 周一到周六自动上传前一交易日交易记录
		//if (((nHour==5&&nMin==6))&&(EsunExist||CTPExist)&&(mCurrTime.GetDayOfWeek()!=1)){
		//	pUpLoadThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
		//	Sleep(200);
		//	pUpLoadThread->PostThreadMessage(WM_UPLOAD_TRADE_LOG,NULL,NULL);
		//}
		// 内盘查权益
		if (((nHour == 15 && nMin == 5)) && CTPExist && (mCurrTime.GetDayOfWeek() != 1) && (mCurrTime.GetDayOfWeek() != 7)) {
			pQryActionThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
			Sleep(200);
			pQryActionThread->PostThreadMessage(WM_QRY_MONEY, NULL, NULL);
		}
		//// 上传内盘权益
		//if ((nHour==15&&nMin==6)&&CTPExist&&(mCurrTime.GetDayOfWeek()!=1)&&(mCurrTime.GetDayOfWeek()!=7))
		//{
		//	pUpLoadValueThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
		//	Sleep(200);
		//	pUpLoadValueThread->PostThreadMessage(WM_UPLOAD_CTP_ACCVALUE,NULL,NULL);
		//}
//#ifdef _POSITION
//		if((nHour=5&&nMin==2)&&EsunExist&&(mCurrTime.GetDayOfWeek()!=1)){
//			pQryActionThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
//			Sleep(200);
//			pQryActionThread->PostThreadMessage(WM_QRY_POSITION_DETAILS,NULL,NULL);
//		}
//#endif

//#ifdef _MONEY
//		// 05:03:00查询权益周一到周六
//		if ((nHour==5&&nMin==3)&&EsunExist&&(mCurrTime.GetDayOfWeek()!=1))
//		{
//			pQryActionThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
//			Sleep(200);
//			pQryActionThread->PostThreadMessage(WM_QRY_MONEY,NULL,NULL);
//		}
//		// 上传权益
//		//if (((nHour==0&&nMin==1))&&EsunExist&&(mCurrTime.GetDayOfWeek()!=1))
//		//{
//		//	pUpLoadValueThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
//		//	Sleep(200);
//		//	pUpLoadValueThread->PostThreadMessage(WM_UPLOAD_VALUE,NULL,NULL);
//		//}
//#endif
		break;
	case 3:
		//#ifdef _MONEY
		//		// 12点,每隔10s查询一次权益
		//		if ((nHour==15&&nMin==40)&&EsunExist&&(mCurrTime.GetDayOfWeek()!=1))
		//		{
		//			pQryActionThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
		//			Sleep(200);
		//			pQryActionThread->PostThreadMessage(WM_QRY_MONEY,NULL,NULL);
		//		}
		//#endif
		break;
	}
}

BOOL CHitTraderApiDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (pMsg->message == WM_KEYDOWN)
	{
		switch (pMsg->wParam)
		{
		case VK_RETURN:    // 屏蔽回车
			OnBnClickedConnect();
			return TRUE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CHitTraderApiDlg::OnBnOK()
{
	if (IDOK != AfxMessageBox(_T("确定退出程序?"), MB_OKCANCEL | MB_ICONQUESTION, 0)) return;
	//关闭启动中的策略线程
	bool RunningStrategyExist = false;
	std::list<ModelNode>::iterator model_itr;
	for (model_itr = ModelList.begin(); model_itr != ModelList.end(); ++model_itr) {
		std::list<StrategyNode>::iterator strategy_itr;
		for (strategy_itr = model_itr->StrategyList.begin(); strategy_itr != model_itr->StrategyList.end(); ++strategy_itr) {
			std::list<StrategyInstanceNode>::iterator instance_itr;
			for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
				if (instance_itr->StrategyStarted && instance_itr->shmindex >= 0) {
					for (int i = 0; i < gStrategyImplIndex; i++) {
						if (gStrategyImpl[i] != NULL && gStrategyImpl[i]->GetShmindex() == instance_itr->shmindex && gStrategyImpl[i]->m_bIsRunning) {
							//设置实例为未启动状态
							//添加退出策略的动作信息到策略消息队列
							OrderTradeMsg order;
							order.shmindex = instance_itr->shmindex;
							order.OrderType = ON_TD_STRATEGY_EXIT;
							OrderList.AddTail(order);
							instance_itr->StrategyStarted = false;
							ReleaseSemaphore(DispatchTdSem, 1, NULL);
							RunningStrategyExist = true;
						}
					}
				}
			}
		}
	}
	/*
	if(RunningStrategyExist){
		if(IDOK==AfxMessageBox(_T("是否清除状态文件(清除后将策略将无法恢复)?"),MB_OKCANCEL|MB_ICONQUESTION,0)){
			ClearInstanceCfgFile=true;
		}else{
			ClearInstanceCfgFile=false;
		}
	}
	Sleep(2000);
	*/
	if (RunningStrategyExist) {
		CString strtemp;
		strtemp.Format(_T("系统正在关闭未停止策略!"));
		AfxMessageBox(strtemp, MB_OK, 0);
	}
	Sleep(2000);

	CDialog::OnOK();
}

void CHitTraderApiDlg::OnBnCancel()
{
	if (IDOK != AfxMessageBox(_T("确定退出程序?"), MB_OKCANCEL | MB_ICONQUESTION, 0)) return;
	gEndSendUdp = true;
	//关闭启动中的策略线程
	bool RunningStrategyExist = false;
	std::list<ModelNode>::iterator model_itr;
	for (model_itr = ModelList.begin(); model_itr != ModelList.end(); ++model_itr) {
		std::list<StrategyNode>::iterator strategy_itr;
		for (strategy_itr = model_itr->StrategyList.begin(); strategy_itr != model_itr->StrategyList.end(); ++strategy_itr) {
			std::list<StrategyInstanceNode>::iterator instance_itr;
			for (instance_itr = strategy_itr->InstanceList.begin(); instance_itr != strategy_itr->InstanceList.end(); ++instance_itr) {
				if (instance_itr->StrategyStarted && instance_itr->shmindex >= 0) {
					for (int i = 0; i < gStrategyImplIndex; i++) {
						if (gStrategyImpl[i] != NULL && gStrategyImpl[i]->GetShmindex() == instance_itr->shmindex && gStrategyImpl[i]->m_bIsRunning) {
							//gStrategyImpl[instance_itr->shmindex]->m_bIsRunning=false;
							//设置实例为未启动状态
							//添加退出策略的动作信息到策略消息队列
							OrderTradeMsg order;
							order.shmindex = instance_itr->shmindex;
							order.OrderType = ON_TD_STRATEGY_EXIT;
							OrderList.AddTail(order);
							instance_itr->StrategyStarted = false;
							ReleaseSemaphore(DispatchTdSem, 1, NULL);
							RunningStrategyExist = true;
						}
					}
				}
			}
		}
	}
	if (RunningStrategyExist) {
		if (IDOK == AfxMessageBox(_T("是否清除状态文件(清除后将策略将无法恢复)?"), MB_OKCANCEL | MB_ICONQUESTION, 0)) {
			ClearInstanceCfgFile = true;
		}
		else {
			ClearInstanceCfgFile = false;
		}
	}
	Sleep(2000);
	if (RunningStrategyExist) {
		CString strtemp;
		strtemp.Format(_T("系统正在关闭未停止策略!"));
		AfxMessageBox(strtemp, MB_OK, 0);
	}
	Sleep(2000);

	CDialog::OnCancel();
}

void CHitTraderApiDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CHitTraderApiDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CHitTraderApiDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CHitTraderApiDlg::OnPubmsg(WPARAM wParam, LPARAM lParam)
{
	AfxMessageBox(_T("message received"), MB_OK);

	return 0;
}

void CHitTraderApiDlg::OnBnClickedConnect() //Connnect Server Button
{
	// TODO: Add your control notification handler code here
	EnterCTPUserDlg ctpUserPwd;

	if (ctpUserPwd.DoModal() != IDOK)
	{
		ExitProcess(0);
	}

	bool CTPExist = false;
	bool SgitExist = false;
	bool EsunExist = false;
	std::map<string, InstrumentInfo>::iterator map_itr;
	for (map_itr = InstrumentsSubscribed.begin(); map_itr != InstrumentsSubscribed.end(); ++map_itr) {
		if (strcmp(map_itr->second.ExchangeID, "CFFEX") == 0 || strcmp(map_itr->second.ExchangeID, "SHFE") == 0
			|| strcmp(map_itr->second.ExchangeID, "DCE") == 0 || strcmp(map_itr->second.ExchangeID, "CZCE") == 0
			|| strcmp(map_itr->second.ExchangeID, "SGE") == 0
			|| strcmp(map_itr->second.ExchangeID, "INE") == 0) {
			if (strcmp(map_itr->second.ExchangeID, "SGE") == 0) {
				SgitExist = true;
			}
			else {
				CTPExist = true;
			}
		}
		else {
			EsunExist = true;
		}
	}

	if (CTPAPIUsed) {
		pThostTraderThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
		Sleep(500);
		pThostTraderThread->PostThreadMessage(WM_CREATE_CTP_TRADER, NULL, NULL);
		Sleep(2000);
	}

	if (EsunAPIUsed) {
		pEsunTraderThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
		Sleep(500);
		pEsunTraderThread->PostThreadMessage(WM_CREATE_ESUN_TRADER, NULL, NULL);
	}

	if (SgitExist) {
		SgitAPIUsed = true;
		pSgitTraderThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
		Sleep(500);
		pSgitTraderThread->PostThreadMessage(WM_CREATE_SGIT_TRADER, NULL, NULL);
		Sleep(2000);
	}

	pTdDispatcher = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
	Sleep(500);
	pTdDispatcher->PostThreadMessage(WM_CREATE_TDSHM_THREAD, NULL, NULL);

	pQryActionThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
	Sleep(500);
	pQryActionThread->PostThreadMessage(WM_START_ACTION_THREAD, NULL, NULL);

	// send udp thread
	pSendUdpMsThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
	Sleep(500);
	pSendUdpMsThread->PostThreadMessage(WM_SEND_UDP_MESSAGE, NULL, NULL);

	GetDlgItem(IDC_BTNConnect)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE);

	m_OrdersMgrPage.GetDlgItem(IDC_QRY_ORDERS_BTN)->EnableWindow(TRUE);
	m_PositionsMgrPage.GetDlgItem(IDC_QRY_POSITIONS_BTN)->EnableWindow(TRUE);
	GetDlgItem(IDC_REFRESH_ORDER_COUNT_BTN)->EnableWindow(TRUE);
	GetDlgItem(IDC_START_STRATEGY_BTN)->EnableWindow(TRUE);
	GetDlgItem(IDC_STOP_STRATEGY_BTN)->EnableWindow(TRUE);
	m_LocalCLMgrDlgPage.GetDlgItem(IDC_QRY_STRA_LOCAL_CL_BTN)->EnableWindow(TRUE);

	SetTimer(1, 60000, NULL);
	// 上传交易记录
	SetTimer(2, 1000 * 60, NULL);
	SetTimer(3, 1000 * 10, NULL);
	CTPTDReConnnectEveryDay = false;

	//Sleep(1000);
	//CMyThread *pCheckCfgThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
	//Sleep(500);
	//pCheckCfgThread->PostThreadMessage(WM_CHECK_INSTANCE_CFG,NULL,NULL);

	//------------------------------------------------------------------------------------
	//刷新缓存中的已订阅合约列表
	//初始化订阅的合约信息
	//InstrumentInfo
	globalFuncUtil.InitInstrumentSubList();
	//------------------------------------------------------------------------------------
	TRACE("Connnect to Server");
}

void CHitTraderApiDlg::OnBnClickedStartStrategy() //Start Strategy
{
	if (ParamsChangedOrNot) {
		CString strtemp;
		strtemp.Format(_T("存在未保存的参数信息,请先保存参数信息!")); // pNMListView->iSubItem
		AfxMessageBox(strtemp, MB_OK, 0);
		return;
	}

	CString csInstanceStarted(InstanceStarted);
	CString csInstanceNotStarted(InstanceNotStarted);
	bool actionOKorNot = true;
	int lineCount = pParamsList->GetItemCount();
	for (int i = 0; i < lineCount; i++) {
		if (pParamsList->GetCheck(i)) {
			CString instanceStatus = pParamsList->GetItemText(i, 0);//第二列是实例名称
			CString instanceName = pParamsList->GetItemText(i, 1);//第二列是实例名称
			if (csInstanceStarted.CompareNoCase(instanceStatus) == 0) {
				AfxMessageBox(_T("有实例处于启动状态,无法再启动"), MB_OK, 0);
				actionOKorNot = false;
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------
	//刷新缓存中的已订阅合约列表
	//初始化订阅的合约信息
	//InstrumentInfo
	globalFuncUtil.InitInstrumentSubList();
	//------------------------------------------------------------------------------------
	bool startFlag = false;
	if (actionOKorNot) {
		for (int i = 0; i < lineCount; i++) {
			if (pParamsList->GetCheck(i)) {
				CString instanceName = pParamsList->GetItemText(i, 1);
				if (IDOK == AfxMessageBox(_T("启动实例:") + instanceName + "?", MB_OKCANCEL, 0)) {
					pStrategyThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
					Sleep(500);
					pStrategyThread->PostThreadMessage(WM_START_STRATEGY, (WPARAM)i, NULL);
					mStraThreadStarted.push_back(pStrategyThread);
					startFlag = true;
				}
			}
		}
	}

	if (startFlag) {
		AfxMessageBox(_T("实例启动操作结束..."), MB_OK, 0);
		Sleep(1000);
		UpdateWindow();
	}
	// TODO: Add your control notification handler code here
}

void CHitTraderApiDlg::OnBnClickedStopStrategy()
{
	// TODO: Add your control notification handler code here
	if (StrategyIDShowing.CompareNoCase(_T("None")) == 0) {
		AfxMessageBox(_T("请在左侧策略列表中选中要关闭的策略!"), MB_OK, 0);
		return;
	}

	if (IDOK == AfxMessageBox(_T("是否清除状态文件(清除后将策略将无法恢复)?"), MB_OKCANCEL | MB_ICONQUESTION, 0)) {
		ClearInstanceCfgFile = true;
	}
	else {
		ClearInstanceCfgFile = false;
	}

	CString csInstanceStarted(InstanceStarted);
	CString csInstanceNotStarted(InstanceNotStarted);
	bool actionOKorNot = true;
	int lineCount = pParamsList->GetItemCount();
	for (int i = 0; i < lineCount; i++) {
		if (pParamsList->GetCheck(i)) {
			CString instanceStatus = pParamsList->GetItemText(i, 0);//第一列是实例状态
			if (instanceStatus.CompareNoCase(csInstanceNotStarted) == 0) {
				CString instanceName = pParamsList->GetItemText(i, 1);//第二列是实例名称
				AfxMessageBox(_T("实例未启动,无法停止"), MB_OK, 0);
				actionOKorNot = false;
				break;
			}
		}
	}
	bool closeFlag = false;
	if (actionOKorNot) {
		for (int i = 0; i < lineCount; i++) {
			if (pParamsList->GetCheck(i)) {
				CString instanceName = pParamsList->GetItemText(i, 1);
				if (IDOK == AfxMessageBox(_T("关闭实例:") + instanceName + "?", MB_OKCANCEL, 0)) {
					pQryActionThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
					pQryActionThread->PostThreadMessage(WM_STOP_STRATEGY, (WPARAM)i, NULL);
					closeFlag = true;
				}
			}
		}
	}

	if (closeFlag) {
		AfxMessageBox(_T("实例关闭操作结束..."), MB_OK, 0);
		Sleep(1000);
		UpdateWindow();
	}
}

void CHitTraderApiDlg::OnMenuInsertOrder()
{
	// TODO: Add your control notification handler code here
	//TRACE("In OnMenuInsertOrder");
	///CInsertOrderDlg insertOrderDlg;
	//insertOrderDlg.DoModal();

	if (insertOrderDlg->GetSafeHwnd() == NULL) {
		insertOrderDlg->Create();
		insertOrderDlg->ShowWindow(SW_SHOWNORMAL);
	}
	else {
		insertOrderDlg->ShowWindow(SW_SHOWNORMAL);
		insertOrderDlg->SetWindowPos(&wndTopMost, 300, 400, NULL, NULL, SWP_NOSIZE);
	}
}

void CHitTraderApiDlg::OnBnClickedRefreshAvailMoney()
{
	if (gMapView != NULL) {
		char log[200];
		sprintf(log, "gStrategyImplIndex=%d,gMapView->strategynum=%d", gStrategyImplIndex, gMapView->strategynum);

		Message logMsg;
		logMsg.type = STRATEGY_LOG;
		logMsg.AddData(log, 0, sizeof(char) * 200);
		LogMessageList.AddTail(logMsg);
		ReleaseSemaphore(logSemaphore, 1, NULL);
	}

	pQryActionThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
	Sleep(500);
	pQryActionThread->PostThreadMessage(WM_QRY_MONEY, NULL, NULL);

	CString csLoginUser(LoginUserTDEsun);
	pUsername->SetWindowTextW(csLoginUser);

	char beginrun_date[30];
	struct tm* ptTm;
	time_t nowtime;
	memset(&beginrun_date, 0, 10);
	time(&nowtime);
	ptTm = localtime(&nowtime);
	strftime(beginrun_date, 10, "%Y%m%d", ptTm);
	CString csCurDate(beginrun_date);
	pDay->SetWindowTextW(csCurDate);
}

void CHitTraderApiDlg::OnMenuOptions()
{
	// TODO: 在此添加命令处理程序代码
	InstMgrDlg mInstMgrDlg;

	if (mInstMgrDlg.DoModal() != IDOK)
	{
		return;
	}
}

void CHitTraderApiDlg::OnTvnSelchangedStrategyTree(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	// TODO: Add your control notification handler code here
	*pResult = 0;
	bool m_bStrategyFound = false;
	CString strText; // 树节点的标签文本字符串

	// 获取当前选中节点的句柄
	HTREEITEM hItem = m_webTree.GetSelectedItem();
	// 获取选中节点的标签文本字符串
	strText = m_webTree.GetItemText(hItem);
	CString csDisplay("");
	std::list<ModelNode>::iterator model_itr;
	if (!ModelList.empty()) {
		for (model_itr = ModelList.begin(); model_itr != ModelList.end(); ++model_itr) {
			CString ModelName(model_itr->ModelName);
			if (strText.CompareNoCase(ModelName) == 0) {
				csDisplay = _T("模型类别:");
				csDisplay.Append(ModelName);
			}
			else {
				std::list<StrategyNode>::iterator strategy_itr;
				if (!model_itr->StrategyList.empty()) {
					for (strategy_itr = model_itr->StrategyList.begin(); strategy_itr != model_itr->StrategyList.end(); ++strategy_itr) {
						CString ClassName(strategy_itr->StrategyName);
						CString StrategyID(strategy_itr->StrategyID);

						std::list<StrategyInstanceNode>::iterator instance_itr;
						if (strText.CompareNoCase(StrategyID) == 0) {
							csDisplay = _T("当前模型:");
							csDisplay.Append(StrategyID);

							ClassNameShowing = ClassName;
							StrategyIDShowing = StrategyID;
							//把当前的页面隐藏起来
							pDialog[m_CurSelTab]->ShowWindow(SW_HIDE);
							//得到新的页面索引
							//m_CurSelTab = 0;
							StrategyNode strategyNode = *strategy_itr;
							//m_ParamsMgrPage.StrategyShowing=StrategyName;
							m_ParamsMgrPage.AddParamList(StrategyIDShowing, strategyNode);
							//if(strategy_itr->StrategyStarted){
							//	m_ParamsMgrPage.SetSaveButtonDisable();
							//	GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE);
							//	GetDlgItem(IDC_BUTTON4)->EnableWindow(TRUE);
							//}else{
							//	m_ParamsMgrPage.SetSaveButtonEnable();
							//}
							//把新的页面显示出来
							pDialog[m_CurSelTab]->ShowWindow(SW_SHOW);

							CString StrategyMaxPosition("");
							StrategyMaxPosition.AppendFormat(_T("%d"), strategy_itr->MaxOnHandPositionCount);
							SetDlgItemText(IDC_STRATEGY_MAX_POSITION, StrategyMaxPosition);
							GetDlgItem(IDC_STRATEGY_MAX_POSITION)->EnableWindow(TRUE);

							m_bStrategyFound = true;
						}
					}
				}
			}
		}
	}
	// 将字符串显示到编辑框中
	if (!m_bStrategyFound) {
		StrategyIDShowing = "None";
		ClassNameShowing = "None";
		SetDlgItemText(IDC_STRATEGY_MAX_POSITION, _T(""));
		GetDlgItem(IDC_STRATEGY_MAX_POSITION)->EnableWindow(FALSE);
	}
	SetDlgItemText(IDC_STRATEGY_NAME_STATIC, csDisplay);
}

void CHitTraderApiDlg::OnTcnSelchangeMaintab(NMHDR* pNMHDR, LRESULT* pResult)
{
	//把当前的页面隐藏起来
	pDialog[m_CurSelTab]->ShowWindow(SW_HIDE);
	//得到新的页面索引
	m_CurSelTab = m_tab.GetCurSel();
	//把新的页面显示出来
	pDialog[m_CurSelTab]->ShowWindow(SW_SHOW);
	*pResult = 0;
}

void CHitTraderApiDlg::OnBnClickedSetPositionLimitBtn()
{
	// TODO: 在此添加控件通知处理程序代码
	CString MaxPosition("");
	GetDlgItemText(IDC_MAX_POSITION, MaxPosition);
	CString MaxCancelTime("");
	GetDlgItemText(IDC_MAX_CANCELTIME, MaxCancelTime);
	CString StrategyMaxPosition("");
	GetDlgItemText(IDC_STRATEGY_MAX_POSITION, StrategyMaxPosition);
	MaxTotalOnHandPosition = _wtoi(MaxPosition.GetBuffer(0));
	MaxTotalCancelTimes = _wtoi(MaxCancelTime.GetBuffer(0));
	CString csRSServerIP("");
	GetDlgItemText(IDC_RS_SERVER_IP, csRSServerIP);
	globalFuncUtil.ConvertCStringToCharArray(csRSServerIP, RSServerIP);
	std::list<ModelNode>::iterator model_itr;
	for (model_itr = ModelList.begin(); model_itr != ModelList.end(); ++model_itr) {
		std::list<StrategyNode>::iterator strategy_itr;
		for (strategy_itr = model_itr->StrategyList.begin(); strategy_itr != model_itr->StrategyList.end(); ++strategy_itr) {
			CString mClassName(strategy_itr->StrategyName);
			CString mStrategyID(strategy_itr->StrategyID);
			if (mClassName.CompareNoCase(ClassNameShowing) == 0 && mStrategyID.CompareNoCase(StrategyIDShowing) == 0) {
				strategy_itr->MaxOnHandPositionCount = _wtoi(StrategyMaxPosition.GetBuffer(0));
				//更新修改到已启动的策略程序中
				for (int i = 0; i < gStrategyImplIndex; i++) {
					CString gClassName(gStrategyImpl[i]->mStrategyName);
					CString gStrategyID(gStrategyImpl[i]->mStrategyID);
					if (gStrategyImpl[i] != NULL && gStrategyImpl[i]->GetShmindex() >= 0
						&& gStrategyID.CompareNoCase(mStrategyID) == 0 && gClassName.CompareNoCase(mClassName) == 0 && gStrategyImpl[i]->m_bIsRunning) {
						if (gClassName.CompareNoCase(_T("KDJ")) == 0) {
							StrategyKDJ::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (gClassName.CompareNoCase(_T("ThreeK")) == 0) {
							StrategyThreeK::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (gClassName.CompareNoCase(_T("Bar")) == 0) {
							StrategyBar::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (gClassName.CompareNoCase(_T("WaveOpen")) == 0) {
							StrategyWaveOpen::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (gClassName.CompareNoCase(_T("WaveOpenAdd")) == 0) {
							StrategyWaveOpenAdd::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (gClassName.CompareNoCase(_T("BandMomnt")) == 0) {
							StrategyBandMomnt::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (gClassName.CompareNoCase(_T("UpDownR")) == 0) {
							StrategyUpDownR::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (gClassName.CompareNoCase(_T("LastTTimeOpen")) == 0) {
							StrategyUpDownR::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (gClassName.CompareNoCase(_T("OpenPriceOpening")) == 0) {
							StrategyUpDownR::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (gClassName.CompareNoCase(_T("AvgLine")) == 0) {
							StrategyAvgLine::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (gClassName.CompareNoCase(_T("AvgDown")) == 0) {
							StrategyAvgDown::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (gClassName.CompareNoCase(_T("DT")) == 0) {
							StrategyDT::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (gClassName.CompareNoCase(_T("BaseGridOpen")) == 0) {
							StrategyBaseGridOpen::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (gClassName.CompareNoCase(_T("BaseGridOpen_plus")) == 0) {
							StrategyBaseGridOpen_plus::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (gClassName.CompareNoCase(_T("BaoChe")) == 0) {
							StrategyBaoChe::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (gClassName.CompareNoCase(_T("OpenPriceOpeningNew")) == 0) {
							StrategyOpenPriceOpeningNew::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (gClassName.CompareNoCase(_T("OpenPriceOpeningAsia")) == 0) {
							StrategyOpenPriceOpeningAsia::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (gClassName.CompareNoCase(_T("BaseGridMAStopOpen")) == 0) {
							StrategyBaseGridMAStopOpen::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (gClassName.CompareNoCase(_T("BaseGridOpenCffex")) == 0) {
							StrategyBaseGridOpenCffex::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (gClassName.CompareNoCase(_T("BaseGridMAStopOpenCffex")) == 0) {
							StrategyBaseGridMAStopOpenCffex::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (gClassName.CompareNoCase(_T("OpenPriceOpeningNight")) == 0) {
							StrategyOpenPriceOpeningNight::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						else if (gClassName.CompareNoCase(_T("GridMAStopGTCChop")) == 0) {
							StrategyGridMAStopGTCChop::MaxOnHandPositionCount = strategy_itr->MaxOnHandPositionCount;
						}
						break;
					}
				}
			} //End if StrategyShowing StrategyBaseGridMAStopOpenCffex
		}
	}
	// print position canceltimes
	char line[200];
	sprintf(line, "MaxTotalOnHandPosition:%d,TotalOnHandPosition:%d,MaxTotalCancelTimes:%d,TotalCancelTimes:%d", MaxTotalOnHandPosition,
		TotalOnHandPosition, MaxTotalCancelTimes, TotalCancelTimes);
	globalFuncUtil.WriteMsgToLogList(line);
	// End
}

void CHitTraderApiDlg::OnMenuProfitAnalyze()
{
	// TODO: 在此添加命令处理程序代码
	ShellExecute(NULL, _T("open"), _T("E:\\Visual2008Projects\\ProfitAnalyze\\Debug\\ProfitAnalyze.exe"), NULL, NULL, SW_SHOWNORMAL);
}

void CHitTraderApiDlg::OnReconnectTd()
{
	// TODO: 在此添加命令处理程序代码
	bool CTPExist = false;
	bool EsunExist = false;
	std::map<string, InstrumentInfo>::iterator map_itr;
	for (map_itr = InstrumentsSubscribed.begin(); map_itr != InstrumentsSubscribed.end(); ++map_itr) {
		if (strcmp(map_itr->second.ExchangeID, "CFFEX") == 0 || strcmp(map_itr->second.ExchangeID, "SHFE") == 0
			|| strcmp(map_itr->second.ExchangeID, "DCE") == 0 || strcmp(map_itr->second.ExchangeID, "CZCE") == 0
			|| strcmp(map_itr->second.ExchangeID, "INE") == 0) {
			CTPExist = true;
		}
		else {
			EsunExist = true;
		}
	}

	if (CTPExist && CTPAPIUsed) {
		if (pThostTraderSpi != NULL) {
			pThostTraderSpi->Release();
		}

		pThostTraderThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
		Sleep(500);
		pThostTraderThread->PostThreadMessage(WM_CREATE_CTP_TRADER, NULL, NULL);
		Sleep(2000);
	}

	if (EsunExist && EsunAPIUsed) {
		if (pEsunTraderSpi != NULL) {
			pEsunTraderSpi->Close();
		}
	}
}

void CHitTraderApiDlg::OnMdDlg()
{
	// TODO: 在此添加命令处理程序代码
	if (mMdDlg->GetSafeHwnd() == NULL) {
		mMdDlg->Create();
		mMdDlg->ShowWindow(SW_SHOWNORMAL);
	}
	else {
		mMdDlg->ShowWindow(SW_SHOWNORMAL);
		mMdDlg->SetWindowPos(&wndTopMost, 300, 400, NULL, NULL, SWP_NOSIZE);
	}
}

void CHitTraderApiDlg::StrategyResetAction()
{
	for (int i = 0; i < gStrategyImplIndex; i++) {
		if (gStrategyImpl[i] != NULL && gStrategyImpl[i]->GetShmindex() >= 0 && gStrategyImpl[i]->m_bIsRunning) {
			gStrategyImpl[i]->ResetAction();
			//OrderTradeMsg order;
			//order.shmindex=gStrategyImpl[i]->GetShmindex();
			//order.OrderType=ON_TD_STRATEGY_RESET;
			//OrderList.AddTail(order);
			//ReleaseSemaphore(DispatchTdSem, 1, NULL);
		}
	}
}

void CHitTraderApiDlg::StrategyInitAction()
{
	for (int i = 0; i < gStrategyImplIndex; i++) {
		if (gStrategyImpl[i] != NULL && gStrategyImpl[i]->GetShmindex() >= 0 && gStrategyImpl[i]->m_bIsRunning) {
			gStrategyImpl[i]->InitAction();
			//OrderTradeMsg order;
			//order.shmindex=gStrategyImpl[i]->GetShmindex();
			//order.OrderType=ON_TD_STRATEGY_RESET;
			//OrderList.AddTail(order);
			//ReleaseSemaphore(DispatchTdSem, 1, NULL);
		}
	}
}

void CHitTraderApiDlg::OnStRecover()
{
	// TODO: 在此添加命令处理程序代码
	CMyThread* pCheckCfgThread = (CMyThread*)AfxBeginThread(RUNTIME_CLASS(CMyThread));
	Sleep(500);
	pCheckCfgThread->PostThreadMessage(WM_CHECK_INSTANCE_CFG, NULL, NULL);
}