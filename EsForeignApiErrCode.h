#ifndef _ESUNNY_FOREIGN_API_ErrCODE_H_
#define _ESUNNY_FOREIGN_API_ErrCODE_H_

#include "EsForeignApiType.h"

namespace ESForeign
{
    ///////////////////////////////////////////////////////////////////////////
    //δ֪����
    const TErrorCodeType Err_Unknown								= -1;
    //�ɹ�
    const TErrorCodeType Err_Succeed								= 0;

    ///////////////////////////////////////////////////////////////////////////
    //��¼�汾������
    const TErrorCodeType Err_Login_Version						= 1;
    //��¼���Ͳ�����
    const TErrorCodeType Err_Login_Type							= 2;
    //CA��¼��ʽ����
    const TErrorCodeType Err_Login_Ca							= 3;
    //���޸������¼
    const TErrorCodeType Err_Login_ForcePwd						= 4;
    //��¼��ݲ�����
    const TErrorCodeType Err_Login_Identity						= 5;
    //��¼��������
    const TErrorCodeType Err_Login_LoginNo						= 6;
    //��¼�����
    const TErrorCodeType Err_Login_Password						= 7;
    //��ֹ��¼
    const TErrorCodeType Err_Login_Forbidden						= 8;
    //��½������
    const TErrorCodeType Err_Login_Count							= 9;
    //��������֤ʧ��
    const TErrorCodeType Err_Otp									= 10;
    //δ���ӻ����������
    const TErrorCodeType Err_Network_Disconnected					= 11;
    //Ӧ���в�������Ч����
    const TErrorCodeType Err_No_Valid_Data   					    = 12;
    //δ��½�ɹ���û�в���Ȩ��
    const TErrorCodeType Err_Not_Login   					        = 13;
    //û��Ȩ�޲������û�
    const TErrorCodeType Err_No_Such_Client_Rights		        = 14;
    //û�б�������Ȩ��
    const TErrorCodeType Err_No_Oper_Rights		                = 15;
    //��ʼ�����ݳ���1
    const TErrorCodeType Err_Init_FAIL1		                = 16;
    //��ʼ�����ݳ���2
    const TErrorCodeType Err_Init_FAIL2		                = 17;
    //��ʼ�����ݳ���3
    const TErrorCodeType Err_Init_FAIL3		                = 18;
    //δ�ɹ���ʼ��
    const TErrorCodeType Err_Init_Unsuccess	                = 19;
    //��֤���ѹ���
    const TErrorCodeType Err_Cert_Expired	                = 20;
    //�û��Ѿ���¼����Ͽ����ٵ�½
    const TErrorCodeType Err_User_Has_Logged                = 21;
    //����Ƶ�ʹ��ߣ����Ժ�����
    const TErrorCodeType Err_Req_Too_Much                   = 22;
    //���ݲ�ѯ�����������Ժ�����
    const TErrorCodeType Err_Qry_Incomplete                 = 23;
	//Ӧ�ó���ID����֤�벻��
	const TErrorCodeType Err_Cert_APPID						= 24;

    ///////////////////////////////////////////////////////////////////////////
    //��ѯ��Լʧ�� �޴�Ʒ��
    const TErrorCodeType Err_Contract_Qry							= 101;    

    ///////////////////////////////////////////////////////////////////////////
    //�µ��޴˺�Լ
    const TErrorCodeType Err_Order_Contract						= 201;
    //���ֲ�����
    const TErrorCodeType Err_Order_Upper							= 202;
    //��״̬���ܳ���
    const TErrorCodeType Err_Order_State							= 203;
    //��Լ��ֹ����
    const TErrorCodeType Err_Contract_NoTrade						= 204;
    //��Լ��ֹ����
    const TErrorCodeType Err_Contract_NoOpen						= 205;
    //ί�г�������µ���
    const TErrorCodeType Err_MaxOrderVol							= 206;
    //�ֲֳ����������
    const TErrorCodeType Err_MaxHoldVol							= 207;
    //�ͻ���ֹ����
    const TErrorCodeType Err_Client_NoTrade						= 208;
    //�ͻ���ֹ����
    const TErrorCodeType Err_Client_NoOpen						= 209;
    //�µ��ʽ���
    const TErrorCodeType Err_Money_NotEnough						= 210;
    //LMEδ׼������
    const TErrorCodeType Err_LME_Check							= 211;
    //��ɾ����������ת��
    const TErrorCodeType Err_CanNotMove_DeletedOrder				= 212;
    //ǿƽ�����ܳ���
    const TErrorCodeType Err_CanNotDelete_RiskOrder				= 213;
    //�ͻ��Ų�����
    const TErrorCodeType Err_Order_ClientNoExist					= 214;
    //�����Ų���ȷ
    const TErrorCodeType Err_OrderID_InCorrect                  = 215;
	//¼��ɽ��������Ϸ�
	const TErrorCodeType Error_MatchInput_Vol						= 216;
	//¼��ɽ��Ҳ���ί��
	const TErrorCodeType Error_Order_NoExist						= 217;
	//ǿƽ�������޸�
	const TErrorCodeType Error_CanNotModify_RiskOrder				= 218;
	//ƽ�ַ�ʽ����
	const TErrorCodeType Error_CoverMode							= 219;

    ///////////////////////////////////////////////////////////////////////////
    //���ݿ��������
    const TErrorCodeType Err_Database								= 301;


    //-----���������������
    //��¼ʱ�������汾����
    const TErrorCodeType	Err_Login_VersionErr				    = 401;		
    //��¼ʱ���ֺŴ���                                            
    const TErrorCodeType	Err_Login_UpNOErr					    = 402;		
    //��¼ʱ���ֿͻ��Ŵ���			                              
    const TErrorCodeType	Err_Login_UpUserErr					    = 403;			
    //�������ݰ�ת������                                          
    const TErrorCodeType	Err_Trade_Trans						    = 404;		
    //�������ݰ����ʹ���                                          
    const TErrorCodeType	Err_Trade_Send						    = 405;	
    //��������δ����                                              
    const TErrorCodeType	Err_Trade_APINoReady				    = 406;		
    //�������س���ʧ��                                            
    const TErrorCodeType	Err_Trade_APIFail					    = 407;		
    //ϵͳ���ڽ���ǿ�Ƴ�ʼ��                                      
    const TErrorCodeType	Err_Trade_Initing					    = 408;		
    //�����־ܾ�                                                  
    const TErrorCodeType	Err_Trade_RejectByUpper				    = 409;	
    //�г���Ʒ�ֲ�����                                          
    const TErrorCodeType	Err_Trade_TransWNOFind				    = 410;		
    //�������Ͳ���֧��                                            
    const TErrorCodeType	Err_Trade_InvalidOrderType			    = 411;		
    //�۸����                                                    
    const TErrorCodeType	Err_Trade_PriceErr					    = 412;		
    //�������ʹ���                                                
    const TErrorCodeType	Err_Trade_UnknownOrderType			    = 413;		
    //��Լ����                                                    
    const TErrorCodeType	Err_Trade_UnknownContract			    = 414;		
    //������δ����                                                
    const TErrorCodeType	Err_Trade_ServerUnConnect			    = 415;		
    //����������
    const TErrorCodeType	Err_Trade_UnknownOrder				    = 416;		
    //����״̬������                                              
    const TErrorCodeType	Err_Trade_InvalidState				    = 417;		
    //��������֧�ָĵ�                                            
    const TErrorCodeType	Err_Trade_AmendDisabled				    = 418;		
    //�������Ϸ�                                                  
    const TErrorCodeType	Err_Trade_InvalidVolume				    = 419;		
    //�����Ͳ����޸�                                              
    const TErrorCodeType	Err_Trade_InvalidAmendOrderType		    = 420;		
    //�˻����û���ƥ��                                            
    const TErrorCodeType	Err_Trade_UnknownAccount			    = 421;		
    //����ķ�����������                                        
    const TErrorCodeType	Err_Trade_ErrDirect					    = 422;		
    //��֧�ֵ���Ʒ����                                            
    const TErrorCodeType	Err_Trade_InvalidCommodityType		    = 423;		
    //�Ƿ��ı���ģʽ                                              
    const TErrorCodeType	Err_Trade_InvalidMode				    = 424;

    //ǰ�û����صĴ�����
    //�汾�Ų�����ǰ�û�Ҫ��
    const TErrorCodeType    Err_Frnt_Version                = 501;
    //��ݴ���, ǰ�ò�����ǰ�õ�¼
    const TErrorCodeType    Err_Frnt_Identity                = 502;
    //���͸�������ʧ��
    const TErrorCodeType    Err_Frnt_SendServer                = 503;
    //����֧�ֵ�����
    const TErrorCodeType    Err_Frnt_CMDNotAllow                = 504;
    //�ͻ�û��Ȩ��
    const TErrorCodeType  Err_Frnt_NORight                = 505;
    //ϵͳ�Ŵ���
    const TErrorCodeType    Err_Frnt_SysNOErr                = 506;
    //δ��¼��Ȩ��
    const TErrorCodeType  Err_Frnt_NOLogin                = 508;
    //ǰ�õ�¼�ͻ�������
    const TErrorCodeType  Err_Frnt_ClientFull                    = 509;
    //ǰ�ò���������Ϳͻ���½
    const TErrorCodeType  Err_Frnt_IsNotClient                = 510;
    //ǰ��û��׼����
    const TErrorCodeType  Err_Frnt_NoReady                = 511;
	//ǰ��û��������
	const TErrorCodeType  Err_Frnt_NoData				  = 512;
	//�ͻ�Ӧ��û����Ȩ
	const TErrorCodeType  Err_Frnt_APPNotAllow			  = 513;

    //��֤������Ϣ
    //��֤�����
    const TErrorCodeType  Err_Cert_Incorrect              = 600;
    //��־�ļ���ʼ��ʧ��
    const TErrorCodeType  Err_Open_Log_Init_Failed        = 601;
    //��־�ļ���ʧ��
    const TErrorCodeType  Err_Open_Log_Failed             = 602;
	//�û��޴�Ȩ��
	const TErrorCodeType  Error_No_SuchRights			  = 603;
	//ί�з�ʽ����
	const TErrorCodeType  Error_No_SuchOrderWay			  = 604;
	//��ƽ��־����
	const TErrorCodeType  Error_No_SuchOffset			  = 605;


	///////////////////////////////////////////////////////////////////////////
	//��̬����ʱ��ͬ��ʧ��
	const TErrorCodeType Error_Otp_SyncTime				  = 700;


	//////////////////////////////////////////////////////////////////////////
	//����������������
	const TErrorCodeType Error_CashOper_Value			= 800;
	//�������������кŴ���
	const TErrorCodeType Error_CashCheck_SerialID		= 810;
	//�������������ʽ���Ϣ
	const TErrorCodeType Error_CashCheck_NoMoneyInfo	= 811;
	//�����������ʽ���
	const TErrorCodeType Error_CashCheck_MoneyNotEnough	= 812;
}

#endif
