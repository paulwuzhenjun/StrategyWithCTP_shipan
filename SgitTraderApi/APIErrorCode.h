/********************************************************
 * 文件名称：APIErrorCode.h
 *********************************************************/

#ifndef _API_ERROR_CODE_H
#define _API_ERROR_CODE_H

//sgit:操作成功
#define	NONE                                    0            
#define	NONE_msg                                "sgit:操作成功"                    

//sgit:消息驱动线程尚未创建
#define	SE_REACTOR_NO_INITIALIZE                1            
#define	SE_REACTOR_NO_INITIALIZE_msg            "sgit:消息驱动线程尚未创建"                    

//sgit:CA模块尚未创建
#define	SE_CA_NO_INITIALIZE                     2            
#define	SE_CA_NO_INITIALIZE_msg                 "sgit:CA模块尚未创建"                    

//sgit:API Control模块尚未创建
#define	SE_APICONTROL_NO_INITIALIZE             3            
#define	SE_APICONTROL_NO_INITIALIZE_msg         "sgit:API Control模块尚未创建"                    

//sgit:创建TCP连接失败
#define	SE_CHNL_CREATE_FAIL                     4            
#define	SE_CHNL_CREATE_FAIL_msg                 "sgit:创建TCP连接失败"                    

//sgit:同步调用超时错误
#define	SE_SYNC_CALL_TIMEOUT                    5            
#define	SE_SYNC_CALL_TIMEOUT_msg                "sgit:同步调用超时错误"                    

//sgit:物理连接出错
#define	SE_CHNL_ERR                             6            
#define	SE_CHNL_ERR_msg                         "sgit:物理连接出错"                    

//sgit:CA认证模块签名错误
#define	SE_CA_SIGN_ERROR                        7            
#define	SE_CA_SIGN_ERROR_msg                    "sgit:CA认证模块签名错误"                    

//sgit:重复认证
#define	SE_RE_CERT                              8            
#define	SE_RE_CERT_msg                          "sgit:重复认证"                    

//sgit:验证服务器端签名失败
#define	SE_CA_CHECKSIGN_ERROR                   9            
#define	SE_CA_CHECKSIGN_ERROR_msg               "sgit:验证服务器端签名失败"                    

//sgit:重复登陆
#define	SE_RE_LOGIN                             10           
#define	SE_RE_LOGIN_msg                         "sgit:重复登陆"                   

//sgit:尚未通过CA认证
#define	SE_NO_CERT                              11           
#define	SE_NO_CERT_msg                          "sgit:尚未通过CA认证"                   

//sgit:尚未登录前置
#define	SE_NO_LOGIN                             12           
#define	SE_NO_LOGIN_msg                         "sgit:尚未登录前置"                   

//sgit:CA模块初始化失败
#define	SE_CA_INIT_ERR                          13           
#define	SE_CA_INIT_ERR_msg                      "sgit:CA模块初始化失败"                   

//sgit:没有前置可以连接
#define	SE_NO_SERVICE                           14           
#define	SE_NO_SERVICE_msg                       "sgit:没有前置可以连接"                   

//sgit:同步超时不能为零
#define	SE_NO_SYNCTIMEOUT                       45           
#define	SE_NO_SYNCTIMEOUT_msg                   "sgit:同步超时不能为零"                   

//sgit:重复会话请求
#define	SE_RE_SESSION                           16           
#define	SE_RE_SESSION_msg                       "sgit:重复会话请求"                   

//sgit:在认证之前尚未通过会话请求
#define	SE_NO_SESSION                           17           
#define	SE_NO_SESSION_msg                       "sgit:在认证之前尚未通过会话请求"                   

//sgit:会话和认证过程中席位号不同
#define	SE_DIFF_TRADERNO                        18           
#define	SE_DIFF_TRADERNO_msg                    "sgit:会话和认证过程中席位号不同"                   

//sgit:CA证书初始化错误
#define	SE_CA_CERT_ERR                          19           
#define	SE_CA_CERT_ERR_msg                      "sgit:CA证书初始化错误"                   

//sgit:认证应答CA证书解密失败
#define	SE_CA_DECODE_ERR                        20           
#define	SE_CA_DECODE_ERR_msg                    "sgit:认证应答CA证书解密失败"                   

//sgit:装载动态库出错
#define	SE_CA_ERROR_LOAD_LIBRARY                21           
#define	SE_CA_ERROR_LOAD_LIBRARY_msg            "sgit:装载动态库出错"                   

//sgit:读写私钥设备失败
#define	SE_CA_ERROR_PRIVATE_KEY_DEV             22           
#define	SE_CA_ERROR_PRIVATE_KEY_DEV_msg         "sgit:读写私钥设备失败"                   

//sgit:登录成功
#define	SE_LOGIN_OK                             0            
#define	SE_LOGIN_OK_msg                         "sgit:登录成功"                    

//sgit:席位不符
#define	SE_TRADERID_ERROR                       10001        
#define	SE_TRADERID_ERROR_msg                   "sgit:席位不符"                

//sgit:不能为该客户操作
#define	SE_ACCOUNT_ERROR                        10002        
#define	SE_ACCOUNT_ERROR_msg                    "sgit:不能为该客户操作"                

//sgit:登录未成功,拒绝发送交易
#define	SE_TRADE_NOLOGIN                        10003        
#define	SE_TRADE_NOLOGIN_msg                    "sgit:登录未成功,拒绝发送交易"                

//sgit:未登录
#define	SE_NOLOGIN                              10004        
#define	SE_NOLOGIN_msg                          "sgit:未登录"                

//sgit:登录包错误
#define	SE_LOGONPACKAGE_ERROR                   10005        
#define	SE_LOGONPACKAGE_ERROR_msg               "sgit:登录包错误"                

//sgit:发送失败
#define	SE_SEND_PACKAGE_ERROR                   10006        
#define	SE_SEND_PACKAGE_ERROR_msg               "sgit:发送失败"                

//sgit:操作员不存在
#define	SE_NO_USERID                            10007        
#define	SE_NO_USERID_msg                        "sgit:操作员不存在"                

//sgit:密码错误
#define	SE_PWD_ERROR                            10008        
#define	SE_PWD_ERROR_msg                        "sgit:密码错误"                

//sgit:原密码不匹配
#define	SE_OLDPWD_ERROR                         10009        
#define	SE_OLDPWD_ERROR_msg                     "sgit:原密码不匹配"                

//sgit:发送交易所失败
#define	SE_SENDTOEXC_ERROR                      10010        
#define	SE_SENDTOEXC_ERROR_msg                  "sgit:发送交易所失败"                

//sgit:委托成功
#define	SE_ORDER_INSERT_OK                      0            
#define	SE_ORDER_INSERT_OK_msg                  "sgit:委托成功"                    

//sgit:交易所接口不能使用
#define	SE_EXCAPI_INVALID                       11001        
#define	SE_EXCAPI_INVALID_msg                   "sgit:交易所接口不能使用"                

//sgit:报单错误：本地报单号重复
#define	SE_DUPLICATE_ORDER_REF                  11002        
#define	SE_DUPLICATE_ORDER_REF_msg              "sgit:报单错误：本地报单号重复"                

//sgit:当前状态不允许报单
#define	SE_EXC_NOT_TRADING                      11003        
#define	SE_EXC_NOT_TRADING_msg                  "sgit:当前状态不允许报单"                

//sgit:客户无资金帐号
#define	SE_NO_INVESTORID                        11004        
#define	SE_NO_INVESTORID_msg                    "sgit:客户无资金帐号"                

//sgit:资金帐号无保证金率
#define	SE_NO_MARGIN_RATE                       11005        
#define	SE_NO_MARGIN_RATE_msg                   "sgit:资金帐号无保证金率"                

//sgit:资金不足
#define	SE_CAPITAL_LOW                          11006        
#define	SE_CAPITAL_LOW_msg                      "sgit:资金不足"                

//sgit:无交易编码
#define	SE_NO_CLIENT                            11007        
#define	SE_NO_CLIENT_msg                        "sgit:无交易编码"                

//sgit:价格不再长跌停板
#define	SE_PRICE_H_OR_L                         11008        
#define	SE_PRICE_H_OR_L_msg                     "sgit:价格不再长跌停板"                

//sgit:价格不是tick整数倍
#define	SE_PRICE_TICK_ERROR                     11009        
#define	SE_PRICE_TICK_ERROR_msg                 "sgit:价格不是tick整数倍"                

//sgit:申报数量大于最大手数或小于最小手数
#define	SE_VOLUME_H_OR_L                        11010        
#define	SE_VOLUME_H_OR_L_msg                    "sgit:申报数量大于最大手数或小于最小手数"                

//sgit:超过最大持仓限额
#define	SE_POSITION_H                           11011        
#define	SE_POSITION_H_msg                       "sgit:超过最大持仓限额"                

//sgit:子账户不允许交收操作
#define	SE_CHILD_DELIVER_MID                    11012        
#define	SE_CHILD_DELIVER_MID_msg                "sgit:子账户不允许交收操作"                

//sgit:不足持仓可平
#define	SE_POSITON_LESS                         11013        
#define	SE_POSITON_LESS_msg                     "sgit:不足持仓可平"                

//sgit:存在自成交可能
#define	SE_MATCH_SELF                           11014        
#define	SE_MATCH_SELF_msg                       "sgit:存在自成交可能"                

//sgit:递延交割持仓不足
#define	SE_DELIVERY_POSI_LESS                   11015        
#define	SE_DELIVERY_POSI_LESS_msg               "sgit:递延交割持仓不足"                

//sgit:第二腿合约行情不存在
#define	SE_NO_MARKET_2                          11016        
#define	SE_NO_MARKET_2_msg                      "sgit:第二腿合约行情不存在"                

//sgit:第二腿合约保证金不存在
#define	SE_NO_MARGIN_2                          11017        
#define	SE_NO_MARGIN_2_msg                      "sgit:第二腿合约保证金不存在"                

//sgit:超过风险度范围
#define	SE_RISK_DEGREE                          11018        
#define	SE_RISK_DEGREE_msg                      "sgit:超过风险度范围"                

//sgit:无效报单
#define	SE_INVALID_ORDER                        11019        
#define	SE_INVALID_ORDER_msg                    "sgit:无效报单"                

//sgit:超过持仓限额
#define	SE_OVER_ORDERNUMBER_LIMIT               11020        
#define	SE_OVER_ORDERNUMBER_LIMIT_msg           "sgit:超过持仓限额"                

//sgit:超过反向禁开范围
#define	SE_OVER_BANOPEN                         11021        
#define	SE_OVER_BANOPEN_msg                     "sgit:超过反向禁开范围"                

//sgit:资管账户超过风险度
#define	SE_ASSET_RISK                           11022        
#define	SE_ASSET_RISK_msg                       "sgit:资管账户超过风险度"                

//sgit:剔除用户成功
#define	SE_USER_ELIMINATE                       0            
#define	SE_USER_ELIMINATE_msg                   "sgit:剔除用户成功"                    

//sgit:激活用户成功
#define	SE_USER_ACTIVATE                        0            
#define	SE_USER_ACTIVATE_msg                    "sgit:激活用户成功"                    

//sgit:账号无相关性
#define	SE_NO_CORRELATION                       11101        
#define	SE_NO_CORRELATION_msg                   "sgit:账号无相关性"                

//sgit:密码不正确
#define	SE_CANTOPER_ADMIN                       0            
#define	SE_CANTOPER_ADMIN_msg                   "sgit:密码不正确"                    

//sgit:交易员号不存在
#define	SE_NO_COMMAND_USER                      0            
#define	SE_NO_COMMAND_USER_msg                  "sgit:交易员号不存在"                    

//sgit:重复操作
#define	SE_REPEAT_OPERAT                        0            
#define	SE_REPEAT_OPERAT_msg                    "sgit:重复操作"                    

//sgit:撤单成功
#define	SE_ORDER_CANCLE_OK                      0            
#define	SE_ORDER_CANCLE_OK_msg                  "sgit:撤单成功"                    

//sgit:错误的报单操作字段
#define	SE_ORDER_ACTION_FIELD                   12001        
#define	SE_ORDER_ACTION_FIELD_msg               "sgit:错误的报单操作字段"                

//sgit:重复撤单
#define	SE_ACTION_REPEAT                        12002        
#define	SE_ACTION_REPEAT_msg                    "sgit:重复撤单"                

//sgit:撤单找不到相应报单
#define	SE_ORDER_NOT_FOUND                      12003        
#define	SE_ORDER_NOT_FOUND_msg                  "sgit:撤单找不到相应报单"                

//sgit:报单已全成交
#define	SE_ORDER_ALL_TRADE                      12004        
#define	SE_ORDER_ALL_TRADE_msg                  "sgit:报单已全成交"                

//sgit:无撤单权限
#define	SE_NO_RIGHT                             12005        
#define	SE_NO_RIGHT_msg                         "sgit:无撤单权限"                

//sgit:查询成功
#define	SE_QUERY_OK                             0            
#define	SE_QUERY_OK_msg                         "sgit:查询成功"                    

//sgit:查询成功无记录
#define	SE_QUERY_NONE                           0            
#define	SE_QUERY_NONE_msg                       "sgit:查询成功无记录"                    

//sgit:查询失败
#define	SE_QUERY_FAILE                          13001        
#define	SE_QUERY_FAILE_msg                      "sgit:查询失败"                

//sgit:只有风控员可操作
#define	SE_NOT_CENSOR                           14001        
#define	SE_NOT_CENSOR_msg                       "sgit:只有风控员可操作"                

//sgit:无权操作该账户
#define	SE_CENSOR_NO_RIGHT                      14002        
#define	SE_CENSOR_NO_RIGHT_msg                  "sgit:无权操作该账户"                

//sgit:无此配置模板
#define	SE_NO_TEMP                              14003        
#define	SE_NO_TEMP_msg                          "sgit:无此配置模板"                

//sgit:无权操作该账户/模板
#define	SE_NO_RIGHT_TEMP                        14004        
#define	SE_NO_RIGHT_TEMP_msg                    "sgit:无权操作该账户/模板"                

//sgit:禁止交易
#define	SE_FORBID_TRADE                         14005        
#define	SE_FORBID_TRADE_msg                     "sgit:禁止交易"                

//sgit:非飞鼠错误码
#define	SE_ERROR_NOT                            20000        
#define	SE_ERROR_NOT_msg                        "sgit:非飞鼠错误码"                


//总共78个
#endif
