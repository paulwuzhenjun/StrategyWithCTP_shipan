// EnterCTPUserDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "EnterCTPUserDlg.h"
#include "GlobalFunc.h"

extern char LoginUserTDEsun[];
extern char LoginPwdTDEsun[];
extern char LoginMDUser[];
extern char LoginMDPwd[];
extern char LoginUserTDCTP[];
extern char LoginPwdCTP[];
extern bool EsunAPIUsed;
extern bool CTPAPIUsed;
//extern char CodeName[];
// EnterCTPUserDlg �Ի���
extern GlobalFunc globalFuncUtil;

IMPLEMENT_DYNAMIC(EnterCTPUserDlg, CDialog)

EnterCTPUserDlg::EnterCTPUserDlg(CWnd* pParent /*=NULL*/)
	: CDialog(EnterCTPUserDlg::IDD, pParent)
{
}

EnterCTPUserDlg::~EnterCTPUserDlg()
{
}

void EnterCTPUserDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(EnterCTPUserDlg, CDialog)
	ON_BN_CLICKED(IDOK, &EnterCTPUserDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &EnterCTPUserDlg::OnBnClickedCancel)
END_MESSAGE_MAP()

BOOL EnterCTPUserDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//SetDlgItemText(IDC_CTP_USER_NAME_ESUN,(LPCTSTR)_T("89501009"));
	//SetDlgItemText(IDC_CTP_USER_PWD_ESUN,(LPCTSTR)_T("Dk285697"));
#ifdef _TEST
	// 043714 cx161354
	// 094440 980010
	SetDlgItemText(IDC_CTP_USER_NAME_CTP, (LPCTSTR)_T("932535"));
	SetDlgItemText(IDC_CTP_USER_PWD_CTP, (LPCTSTR)_T("wn980785"));
	//SetDlgItemText(IDC_CTP_USER_NAME_CTP,(LPCTSTR)_T("094440"));
	//SetDlgItemText(IDC_CTP_USER_PWD_CTP,(LPCTSTR)_T("980010"));
#else
	// �𹤾��ʺ�
	//SetDlgItemText(IDC_CTP_USER_NAME_CTP,(LPCTSTR)_T("8808967002"));
	//SetDlgItemText(IDC_CTP_USER_PWD_CTP,(LPCTSTR)_T("965321"));
	// �����ʺ�
	//SetDlgItemText(IDC_CTP_USER_NAME_CTP,(LPCTSTR)_T("2208002102"));
	//SetDlgItemText(IDC_CTP_USER_PWD_CTP,(LPCTSTR)_T("181105"));

	// ���ܾ��ʺ�
	//SetDlgItemText(IDC_CTP_USER_NAME_CTP,(LPCTSTR)_T("8808967001"));
	//SetDlgItemText(IDC_CTP_USER_PWD_CTP,(LPCTSTR)_T("280528"));
	// �������ʺ�
	//SetDlgItemText(IDC_CTP_USER_NAME_CTP,(LPCTSTR)_T("2208002007"));
	//SetDlgItemText(IDC_CTP_USER_PWD_CTP,(LPCTSTR)_T("158763"));

	// �����˻�
	SetDlgItemText(IDC_CTP_USER_NAME_CTP, (LPCTSTR)_T("2208002103"));
	SetDlgItemText(IDC_CTP_USER_PWD_CTP, (LPCTSTR)_T("171028"));

	// �����˻�
	//SetDlgItemText(IDC_CTP_USER_NAME_CTP,(LPCTSTR)_T("2208002006"));
	//SetDlgItemText(IDC_CTP_USER_PWD_CTP,(LPCTSTR)_T("181228"));
	// ���ܹ�ָ�˻�
	//SetDlgItemText(IDC_CTP_USER_NAME_CTP,(LPCTSTR)_T("2208002005"));
	//SetDlgItemText(IDC_CTP_USER_PWD_CTP,(LPCTSTR)_T("158763"));
#endif

	//((CButton*)GetDlgItem(IDC_CHECK_ESUN))->SetCheck(1);
	//((CButton*)GetDlgItem(IDC_CHECK_CTP))->SetCheck(1);
	return TRUE;
}
// EnterCTPUserDlg ��Ϣ�������

void EnterCTPUserDlg::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString ctpUserName(_T(""));
	//GetDlgItem(IDC_CTP_USER_NAME_ESUN)->GetWindowText(ctpUserName);
	CString ctpUserPwd(_T(""));
	//GetDlgItem(IDC_CTP_USER_PWD_ESUN)->GetWindowText(ctpUserPwd);
	CString csUserNameCTP;
	GetDlgItem(IDC_CTP_USER_NAME_CTP)->GetWindowText(csUserNameCTP);
	CString csUserPwdCTP;
	GetDlgItem(IDC_CTP_USER_PWD_CTP)->GetWindowText(csUserPwdCTP);

	// modify by Allen
	//EsunAPIUsed=true;
	EsunAPIUsed = false;
	/*
	int EsunAPIState=((CButton*)GetDlgItem(IDC_CHECK_ESUN))->GetCheck();
	if(EsunAPIState==1){
		EsunAPIUsed=true;
	}else{
		EsunAPIUsed=false;
	}
	*/
	CTPAPIUsed = true;
	/*
	int CTPAPIState=((CButton*)GetDlgItem(IDC_CHECK_CTP))->GetCheck();
	if(CTPAPIState==1){
		CTPAPIUsed=true;
	}else{
		CTPAPIUsed=false;
	}
	*/
	if (!EsunAPIUsed && !CTPAPIUsed) {
		AfxMessageBox(_T("������������һ����Ҫ����"), MB_OK, 0);
		return;
	}

	/*if(!(csUserNameCTP.CompareNoCase(_T("094440"))==0)){
		ExitProcess(0);
	}*/

	globalFuncUtil.ConvertCStringToCharArray(ctpUserName, LoginUserTDEsun);
	/*
	int len=WideCharToMultiByte(CP_ACP,0,ctpUserName,ctpUserName.GetLength(),NULL,0,NULL,NULL);
	char *gInvestor_ID=new char[len+1];
	WideCharToMultiByte(CP_ACP,0,ctpUserName,ctpUserName.GetLength(),gInvestor_ID,len,NULL,NULL);
	gInvestor_ID[len]='\0';
	strcpy(LoginUserTDEsun,gInvestor_ID);
*/
	strcpy(LoginUserTDEsun, LoginMDUser);
	//��ȡ�������Ĵ�С��������ռ䣬��������С�ǰ��ֽڼ����

	globalFuncUtil.ConvertCStringToCharArray(ctpUserPwd, LoginPwdTDEsun);
	/*
	int pwdlen=WideCharToMultiByte(CP_ACP,0,ctpUserPwd,ctpUserPwd.GetLength(),NULL,0,NULL,NULL);
	char *gPASSWORD=new char[pwdlen+1];
	WideCharToMultiByte(CP_ACP,0,ctpUserPwd,ctpUserPwd.GetLength(),gPASSWORD,pwdlen,NULL,NULL);
	gPASSWORD[pwdlen]='\0';
	strcpy(LoginPwdTDEsun,gPASSWORD);
	*/
	strcpy(LoginPwdTDEsun, LoginMDPwd);
	globalFuncUtil.ConvertCStringToCharArray(csUserNameCTP, LoginUserTDCTP);
	/*
	int lenctp=WideCharToMultiByte(CP_ACP,0,csUserNameCTP,csUserNameCTP.GetLength(),NULL,0,NULL,NULL);
	char *gInvestor_IDCTP=new char[lenctp+1];
	WideCharToMultiByte(CP_ACP,0,csUserNameCTP,csUserNameCTP.GetLength(),gInvestor_IDCTP,lenctp,NULL,NULL);
	gInvestor_IDCTP[lenctp]='\0';
	strcpy(LoginUserTDCTP,gInvestor_IDCTP);
	*/
	globalFuncUtil.ConvertCStringToCharArray(csUserPwdCTP, LoginPwdCTP);
	/*
	int pwdlenctp=WideCharToMultiByte(CP_ACP,0,csUserPwdCTP,csUserPwdCTP.GetLength(),NULL,0,NULL,NULL);
	char *gPASSWORDCTP=new char[pwdlenctp+1];
	WideCharToMultiByte(CP_ACP,0,csUserPwdCTP,csUserPwdCTP.GetLength(),gPASSWORDCTP,pwdlenctp,NULL,NULL);
	gPASSWORDCTP[pwdlenctp]='\0';
	strcpy(LoginPwdCTP,gPASSWORDCTP);
	*/
	OnOK();
}

void EnterCTPUserDlg::OnBnClickedCancel()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	OnCancel();
}