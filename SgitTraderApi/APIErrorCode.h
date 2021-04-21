/********************************************************
 * �ļ����ƣ�APIErrorCode.h
 *********************************************************/

#ifndef _API_ERROR_CODE_H
#define _API_ERROR_CODE_H

//sgit:�����ɹ�
#define	NONE                                    0            
#define	NONE_msg                                "sgit:�����ɹ�"                    

//sgit:��Ϣ�����߳���δ����
#define	SE_REACTOR_NO_INITIALIZE                1            
#define	SE_REACTOR_NO_INITIALIZE_msg            "sgit:��Ϣ�����߳���δ����"                    

//sgit:CAģ����δ����
#define	SE_CA_NO_INITIALIZE                     2            
#define	SE_CA_NO_INITIALIZE_msg                 "sgit:CAģ����δ����"                    

//sgit:API Controlģ����δ����
#define	SE_APICONTROL_NO_INITIALIZE             3            
#define	SE_APICONTROL_NO_INITIALIZE_msg         "sgit:API Controlģ����δ����"                    

//sgit:����TCP����ʧ��
#define	SE_CHNL_CREATE_FAIL                     4            
#define	SE_CHNL_CREATE_FAIL_msg                 "sgit:����TCP����ʧ��"                    

//sgit:ͬ�����ó�ʱ����
#define	SE_SYNC_CALL_TIMEOUT                    5            
#define	SE_SYNC_CALL_TIMEOUT_msg                "sgit:ͬ�����ó�ʱ����"                    

//sgit:�������ӳ���
#define	SE_CHNL_ERR                             6            
#define	SE_CHNL_ERR_msg                         "sgit:�������ӳ���"                    

//sgit:CA��֤ģ��ǩ������
#define	SE_CA_SIGN_ERROR                        7            
#define	SE_CA_SIGN_ERROR_msg                    "sgit:CA��֤ģ��ǩ������"                    

//sgit:�ظ���֤
#define	SE_RE_CERT                              8            
#define	SE_RE_CERT_msg                          "sgit:�ظ���֤"                    

//sgit:��֤��������ǩ��ʧ��
#define	SE_CA_CHECKSIGN_ERROR                   9            
#define	SE_CA_CHECKSIGN_ERROR_msg               "sgit:��֤��������ǩ��ʧ��"                    

//sgit:�ظ���½
#define	SE_RE_LOGIN                             10           
#define	SE_RE_LOGIN_msg                         "sgit:�ظ���½"                   

//sgit:��δͨ��CA��֤
#define	SE_NO_CERT                              11           
#define	SE_NO_CERT_msg                          "sgit:��δͨ��CA��֤"                   

//sgit:��δ��¼ǰ��
#define	SE_NO_LOGIN                             12           
#define	SE_NO_LOGIN_msg                         "sgit:��δ��¼ǰ��"                   

//sgit:CAģ���ʼ��ʧ��
#define	SE_CA_INIT_ERR                          13           
#define	SE_CA_INIT_ERR_msg                      "sgit:CAģ���ʼ��ʧ��"                   

//sgit:û��ǰ�ÿ�������
#define	SE_NO_SERVICE                           14           
#define	SE_NO_SERVICE_msg                       "sgit:û��ǰ�ÿ�������"                   

//sgit:ͬ����ʱ����Ϊ��
#define	SE_NO_SYNCTIMEOUT                       45           
#define	SE_NO_SYNCTIMEOUT_msg                   "sgit:ͬ����ʱ����Ϊ��"                   

//sgit:�ظ��Ự����
#define	SE_RE_SESSION                           16           
#define	SE_RE_SESSION_msg                       "sgit:�ظ��Ự����"                   

//sgit:����֤֮ǰ��δͨ���Ự����
#define	SE_NO_SESSION                           17           
#define	SE_NO_SESSION_msg                       "sgit:����֤֮ǰ��δͨ���Ự����"                   

//sgit:�Ự����֤������ϯλ�Ų�ͬ
#define	SE_DIFF_TRADERNO                        18           
#define	SE_DIFF_TRADERNO_msg                    "sgit:�Ự����֤������ϯλ�Ų�ͬ"                   

//sgit:CA֤���ʼ������
#define	SE_CA_CERT_ERR                          19           
#define	SE_CA_CERT_ERR_msg                      "sgit:CA֤���ʼ������"                   

//sgit:��֤Ӧ��CA֤�����ʧ��
#define	SE_CA_DECODE_ERR                        20           
#define	SE_CA_DECODE_ERR_msg                    "sgit:��֤Ӧ��CA֤�����ʧ��"                   

//sgit:װ�ض�̬�����
#define	SE_CA_ERROR_LOAD_LIBRARY                21           
#define	SE_CA_ERROR_LOAD_LIBRARY_msg            "sgit:װ�ض�̬�����"                   

//sgit:��д˽Կ�豸ʧ��
#define	SE_CA_ERROR_PRIVATE_KEY_DEV             22           
#define	SE_CA_ERROR_PRIVATE_KEY_DEV_msg         "sgit:��д˽Կ�豸ʧ��"                   

//sgit:��¼�ɹ�
#define	SE_LOGIN_OK                             0            
#define	SE_LOGIN_OK_msg                         "sgit:��¼�ɹ�"                    

//sgit:ϯλ����
#define	SE_TRADERID_ERROR                       10001        
#define	SE_TRADERID_ERROR_msg                   "sgit:ϯλ����"                

//sgit:����Ϊ�ÿͻ�����
#define	SE_ACCOUNT_ERROR                        10002        
#define	SE_ACCOUNT_ERROR_msg                    "sgit:����Ϊ�ÿͻ�����"                

//sgit:��¼δ�ɹ�,�ܾ����ͽ���
#define	SE_TRADE_NOLOGIN                        10003        
#define	SE_TRADE_NOLOGIN_msg                    "sgit:��¼δ�ɹ�,�ܾ����ͽ���"                

//sgit:δ��¼
#define	SE_NOLOGIN                              10004        
#define	SE_NOLOGIN_msg                          "sgit:δ��¼"                

//sgit:��¼������
#define	SE_LOGONPACKAGE_ERROR                   10005        
#define	SE_LOGONPACKAGE_ERROR_msg               "sgit:��¼������"                

//sgit:����ʧ��
#define	SE_SEND_PACKAGE_ERROR                   10006        
#define	SE_SEND_PACKAGE_ERROR_msg               "sgit:����ʧ��"                

//sgit:����Ա������
#define	SE_NO_USERID                            10007        
#define	SE_NO_USERID_msg                        "sgit:����Ա������"                

//sgit:�������
#define	SE_PWD_ERROR                            10008        
#define	SE_PWD_ERROR_msg                        "sgit:�������"                

//sgit:ԭ���벻ƥ��
#define	SE_OLDPWD_ERROR                         10009        
#define	SE_OLDPWD_ERROR_msg                     "sgit:ԭ���벻ƥ��"                

//sgit:���ͽ�����ʧ��
#define	SE_SENDTOEXC_ERROR                      10010        
#define	SE_SENDTOEXC_ERROR_msg                  "sgit:���ͽ�����ʧ��"                

//sgit:ί�гɹ�
#define	SE_ORDER_INSERT_OK                      0            
#define	SE_ORDER_INSERT_OK_msg                  "sgit:ί�гɹ�"                    

//sgit:�������ӿڲ���ʹ��
#define	SE_EXCAPI_INVALID                       11001        
#define	SE_EXCAPI_INVALID_msg                   "sgit:�������ӿڲ���ʹ��"                

//sgit:�������󣺱��ر������ظ�
#define	SE_DUPLICATE_ORDER_REF                  11002        
#define	SE_DUPLICATE_ORDER_REF_msg              "sgit:�������󣺱��ر������ظ�"                

//sgit:��ǰ״̬��������
#define	SE_EXC_NOT_TRADING                      11003        
#define	SE_EXC_NOT_TRADING_msg                  "sgit:��ǰ״̬��������"                

//sgit:�ͻ����ʽ��ʺ�
#define	SE_NO_INVESTORID                        11004        
#define	SE_NO_INVESTORID_msg                    "sgit:�ͻ����ʽ��ʺ�"                

//sgit:�ʽ��ʺ��ޱ�֤����
#define	SE_NO_MARGIN_RATE                       11005        
#define	SE_NO_MARGIN_RATE_msg                   "sgit:�ʽ��ʺ��ޱ�֤����"                

//sgit:�ʽ���
#define	SE_CAPITAL_LOW                          11006        
#define	SE_CAPITAL_LOW_msg                      "sgit:�ʽ���"                

//sgit:�޽��ױ���
#define	SE_NO_CLIENT                            11007        
#define	SE_NO_CLIENT_msg                        "sgit:�޽��ױ���"                

//sgit:�۸��ٳ���ͣ��
#define	SE_PRICE_H_OR_L                         11008        
#define	SE_PRICE_H_OR_L_msg                     "sgit:�۸��ٳ���ͣ��"                

//sgit:�۸���tick������
#define	SE_PRICE_TICK_ERROR                     11009        
#define	SE_PRICE_TICK_ERROR_msg                 "sgit:�۸���tick������"                

//sgit:�걨�����������������С����С����
#define	SE_VOLUME_H_OR_L                        11010        
#define	SE_VOLUME_H_OR_L_msg                    "sgit:�걨�����������������С����С����"                

//sgit:�������ֲ��޶�
#define	SE_POSITION_H                           11011        
#define	SE_POSITION_H_msg                       "sgit:�������ֲ��޶�"                

//sgit:���˻��������ղ���
#define	SE_CHILD_DELIVER_MID                    11012        
#define	SE_CHILD_DELIVER_MID_msg                "sgit:���˻��������ղ���"                

//sgit:����ֲֿ�ƽ
#define	SE_POSITON_LESS                         11013        
#define	SE_POSITON_LESS_msg                     "sgit:����ֲֿ�ƽ"                

//sgit:�����Գɽ�����
#define	SE_MATCH_SELF                           11014        
#define	SE_MATCH_SELF_msg                       "sgit:�����Գɽ�����"                

//sgit:���ӽ���ֲֲ���
#define	SE_DELIVERY_POSI_LESS                   11015        
#define	SE_DELIVERY_POSI_LESS_msg               "sgit:���ӽ���ֲֲ���"                

//sgit:�ڶ��Ⱥ�Լ���鲻����
#define	SE_NO_MARKET_2                          11016        
#define	SE_NO_MARKET_2_msg                      "sgit:�ڶ��Ⱥ�Լ���鲻����"                

//sgit:�ڶ��Ⱥ�Լ��֤�𲻴���
#define	SE_NO_MARGIN_2                          11017        
#define	SE_NO_MARGIN_2_msg                      "sgit:�ڶ��Ⱥ�Լ��֤�𲻴���"                

//sgit:�������նȷ�Χ
#define	SE_RISK_DEGREE                          11018        
#define	SE_RISK_DEGREE_msg                      "sgit:�������նȷ�Χ"                

//sgit:��Ч����
#define	SE_INVALID_ORDER                        11019        
#define	SE_INVALID_ORDER_msg                    "sgit:��Ч����"                

//sgit:�����ֲ��޶�
#define	SE_OVER_ORDERNUMBER_LIMIT               11020        
#define	SE_OVER_ORDERNUMBER_LIMIT_msg           "sgit:�����ֲ��޶�"                

//sgit:�������������Χ
#define	SE_OVER_BANOPEN                         11021        
#define	SE_OVER_BANOPEN_msg                     "sgit:�������������Χ"                

//sgit:�ʹ��˻��������ն�
#define	SE_ASSET_RISK                           11022        
#define	SE_ASSET_RISK_msg                       "sgit:�ʹ��˻��������ն�"                

//sgit:�޳��û��ɹ�
#define	SE_USER_ELIMINATE                       0            
#define	SE_USER_ELIMINATE_msg                   "sgit:�޳��û��ɹ�"                    

//sgit:�����û��ɹ�
#define	SE_USER_ACTIVATE                        0            
#define	SE_USER_ACTIVATE_msg                    "sgit:�����û��ɹ�"                    

//sgit:�˺��������
#define	SE_NO_CORRELATION                       11101        
#define	SE_NO_CORRELATION_msg                   "sgit:�˺��������"                

//sgit:���벻��ȷ
#define	SE_CANTOPER_ADMIN                       0            
#define	SE_CANTOPER_ADMIN_msg                   "sgit:���벻��ȷ"                    

//sgit:����Ա�Ų�����
#define	SE_NO_COMMAND_USER                      0            
#define	SE_NO_COMMAND_USER_msg                  "sgit:����Ա�Ų�����"                    

//sgit:�ظ�����
#define	SE_REPEAT_OPERAT                        0            
#define	SE_REPEAT_OPERAT_msg                    "sgit:�ظ�����"                    

//sgit:�����ɹ�
#define	SE_ORDER_CANCLE_OK                      0            
#define	SE_ORDER_CANCLE_OK_msg                  "sgit:�����ɹ�"                    

//sgit:����ı��������ֶ�
#define	SE_ORDER_ACTION_FIELD                   12001        
#define	SE_ORDER_ACTION_FIELD_msg               "sgit:����ı��������ֶ�"                

//sgit:�ظ�����
#define	SE_ACTION_REPEAT                        12002        
#define	SE_ACTION_REPEAT_msg                    "sgit:�ظ�����"                

//sgit:�����Ҳ�����Ӧ����
#define	SE_ORDER_NOT_FOUND                      12003        
#define	SE_ORDER_NOT_FOUND_msg                  "sgit:�����Ҳ�����Ӧ����"                

//sgit:������ȫ�ɽ�
#define	SE_ORDER_ALL_TRADE                      12004        
#define	SE_ORDER_ALL_TRADE_msg                  "sgit:������ȫ�ɽ�"                

//sgit:�޳���Ȩ��
#define	SE_NO_RIGHT                             12005        
#define	SE_NO_RIGHT_msg                         "sgit:�޳���Ȩ��"                

//sgit:��ѯ�ɹ�
#define	SE_QUERY_OK                             0            
#define	SE_QUERY_OK_msg                         "sgit:��ѯ�ɹ�"                    

//sgit:��ѯ�ɹ��޼�¼
#define	SE_QUERY_NONE                           0            
#define	SE_QUERY_NONE_msg                       "sgit:��ѯ�ɹ��޼�¼"                    

//sgit:��ѯʧ��
#define	SE_QUERY_FAILE                          13001        
#define	SE_QUERY_FAILE_msg                      "sgit:��ѯʧ��"                

//sgit:ֻ�з��Ա�ɲ���
#define	SE_NOT_CENSOR                           14001        
#define	SE_NOT_CENSOR_msg                       "sgit:ֻ�з��Ա�ɲ���"                

//sgit:��Ȩ�������˻�
#define	SE_CENSOR_NO_RIGHT                      14002        
#define	SE_CENSOR_NO_RIGHT_msg                  "sgit:��Ȩ�������˻�"                

//sgit:�޴�����ģ��
#define	SE_NO_TEMP                              14003        
#define	SE_NO_TEMP_msg                          "sgit:�޴�����ģ��"                

//sgit:��Ȩ�������˻�/ģ��
#define	SE_NO_RIGHT_TEMP                        14004        
#define	SE_NO_RIGHT_TEMP_msg                    "sgit:��Ȩ�������˻�/ģ��"                

//sgit:��ֹ����
#define	SE_FORBID_TRADE                         14005        
#define	SE_FORBID_TRADE_msg                     "sgit:��ֹ����"                

//sgit:�Ƿ��������
#define	SE_ERROR_NOT                            20000        
#define	SE_ERROR_NOT_msg                        "sgit:�Ƿ��������"                


//�ܹ�78��
#endif
