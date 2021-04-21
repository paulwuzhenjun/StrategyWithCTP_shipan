#pragma once
#include "EsForeignApiStruct.h"
#include "EsunnyForeignApi.h"
#include "MyStruct.h"
#include <vector>
#include <hash_map>
using namespace std;

#pragma   comment(lib,	"ForeignTradeApi.lib")
enum ConnectState
{
	STATE_DISCONNECTED = 1,    //未连接
	STATE_CONNECTED = 2,    //已连接
	STATE_LOGIN = 3     //已登录
};

enum OrderState
{
	ORDER_INVALID = 0,    //无效单
	ORDER_NEW = 1,    //新单
	ORDER_IN_QUEUE = 2,    //已排队
	ORDER_ALL_MATCH = 3,    //完全成交
	ORDER_IN_CANCEL = 4,    //撤单请求
	ORDER_CANCELED = 5,    //已撤单
	ORDER_CHECK = 6,	//埋单
	ORDER_CHECK_DELETE = 7,    //埋单删除
	ORDER_ALREADY_SENT = 8,    //已发送
	ORDER_TO_CANCEL = 9,    //待撤
	ORDER_PART_MATCH = 10,   //部分成交
	ORDER_NON_TRADING_HOURS = 11,  //非交易时间
	ORDER_INSUFFICIENT_FUNDS = 12,  //资金不足
	ORDER_TRADE_CLOSED = 13,  //交易关闭
	ORDER_INSTRUCT_FAILED = 14,  //指令失败
	ORDER_WITHDRAWALS_FAILED = 15,  //撤单失败
	ORDER_SYSTEM_WITHDRAWALS = 16,  //系统撤单
	ORDER_TRIGGER_ORDER = 17,  //触发单
	ORDER_TRIGGER_ORDER_DELETE = 18,  //触发单删除
	ORDER_SUSPEND = 19,  //挂起
	ORDER_ACTIVATE = 20,  //激活
	ORDER_ACCEPTED = 21,  //已受理
	ORDER_AUTO = 22,  //自动单
	ORDER_AUTO_DELTE = 23   //自动单删除
};

class EsunTraderSpi :
	public ESForeign::IEsunnyTradeSpi
{
public:
	EsunTraderSpi()
	{
		m_iConnState = 0;
		m_AccountValue = 0;
	}
	~EsunTraderSpi() {}
public:

	virtual void __cdecl OnOpen();

	////////////////////////////////////////
	/// \fn    OnClose
	/// \brief 与服务器断开连接时调用
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnClose();

	////////////////////////////////////////
	/// \fn    OnLogin
	/// \brief 发送Login登陆成功时收到服务器登陆响应调用
	/// \param const TEsLoginRspField & rsp
	/// \param int iReqID
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnLogin(const ESForeign::TEsLoginRspField* rsp, int errCode, const int iReqID);

	////////////////////////////////////////
	/// \fn    OnInitFinished
	/// \brief 发送Login成功后，收到OnLogin应答成功后收到初始化操作完成
	///        所有的业务操作需要在本响应errCode为0（成功）后可进行
	/// \param int errCode 错误码，0-成功，其他值-错误的原因代码
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnInitFinished(int errCode);

	////////////////////////////////////////
	/// \fn    OnLogOut
	/// \brief 收到登出应答调用
	/// \param int errCode
	/// \param const int iReqID
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnLogOut(int errCode, const int iReqID) {};

	////////////////////////////////////////
	/// \fn    OnRspSetPassword
	/// \brief  修改客户密码时应答，返回信息包括修改密码详细内容
	/// \param const TEsClientPasswordModifyRspField & rsp
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnRspSetPassword(const ESForeign::TEsClientPasswordModifyRspField* rsp, int errCode, const int iReqID) {};

	////////////////////////////////////////
	/// \fn    OnRspSetOperPassword
	/// \brief 修改操作员密码时应答，返回信息包括修改密码详细内容
	/// \param const TEsOperatorPasswordModifyRspField & rsp
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnRspSetOperPassword(const ESForeign::TEsOperatorPasswordModifyRspField* rsp, int errCode, const int iReqID) {};

	////////////////////////////////////////
	/// \fn    OnQryMoney
	/// \brief 资金查询应答
	/// \param TEsMoneyQryRspField * rsp 查询完成时，指针为空，未完成时包含查询结果
	/// \param TEsIsLastType islast 指示是否查询完成，完成时rsp指针为空
	/// \param const int iReqID 对应发送请求的ID
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnQryMoney(const ESForeign::TEsMoneyQryRspField* rsp, ESForeign::TEsIsLastType islast, int errCode, const int iReqID);

	////////////////////////////////////////
	/// \fn    OnRtnMoney
	/// \brief 资金变化推送通知
	/// \param TEsMoneyChgNoticeField & rsp 资金变化详细信息
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnRtnMoney(const ESForeign::TEsMoneyChgNoticeField& rsp);

	////////////////////////////////////////
	/// \fn    OnRspCashOperQry
	/// \brief 出金入金查询应答
	/// \param TEsCashOperRspField * rsp查询完成时，指针为空，未完成时包含查询结果
	/// \param TEsIsLastType islast 指示是否查询完成，完成时rsp指针为空
	/// \param const int iReqID 对应发送请求的ID
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnRspCashOperQry(const ESForeign::TEsCashOperQryRspField* rsp, ESForeign::TEsIsLastType islast, int errCode, const int iReqID) {};

	////////////////////////////////////////
	/// \fn    OnRspCashAdjustQry
	/// \brief 资金调整查询应答
	/// \param TEsAdjustOperRspField * rsp 查询结果，查询完成时，指针为空
	/// \param TEsIsLastType islast 指示是否查询完成，完成时rsp指针为空
	/// \param const int iReqID 对应发送请求的ID
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnRspCashAdjustQry(const ESForeign::TEsAdjustQryRspField* rsp, ESForeign::TEsIsLastType islast, int errCode, const int iReqID) {};

	////////////////////////////////////////
	/// \fn    OnRspOrderInsert
	/// \brief 报单请求的应答
	/// \param TEsOrderInsertRspField & rsp 报单请求的详细结果，会返回委托号及本地号
	/// \param const int iReqID 对应发送请求的ID
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnRspOrderInsert(const ESForeign::TEsOrderInsertRspField* rsp, int errCode, const int iReqID);

	////////////////////////////////////////
	/// \fn    OnRspOrderModify
	/// \brief 改单请求的应答，出错时会收到本应答
	/// \param TEsOrderModifyRspField & rsp
	/// \param const int iReqID 对应发送请求的ID
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnRspOrderModify(const ESForeign::TEsOrderModifyRspField* rsp, int errCode, const int iReqID) {};

	////////////////////////////////////////
	/// \fn    OnRspOrderDelete
	/// \brief 撤单请求的应答，出错时会收到本应答
	/// \param TEsOrderDeleteRspField & rsp
	/// \param const int iReqID 对应发送请求的ID
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnRspOrderDelete(const ESForeign::TEsOrderDeleteRspField* rsp, int errCode, const int iReqID);

	////////////////////////////////////////
	/// \fn    OnRspQryOrder
	/// \brief 委托查询应答
	/// \param TEsOrderDataQryRspField * rsp 委托查询结果，查询完成时，指针为空
	/// \param TEsIsLastType islast 指示是否查询完成，完成时rsp指针为空
	/// \param const int iReqID 对应发送请求的ID
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnRspQryOrder(const ESForeign::TEsOrderDataQryRspField* rsp, ESForeign::TEsIsLastType islast, int errCode, const int iReqID);

	////////////////////////////////////////
	/// \fn    OnRspHistOrderQry
	/// \brief 历史委托查询应答
	/// \param TEsHisOrderQryRspField * rsp 历史委托查询结果，查询完成时，指针为空
	/// \param TEsIsLastType islast 指示是否查询完成，完成时rsp指针为空
	/// \param const int iReqID 对应发送请求的ID
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnRspHistOrderQry(const ESForeign::TEsHisOrderQryRspField* rsp, ESForeign::TEsIsLastType islast, int errCode, const int iReqID) {};

	////////////////////////////////////////
	/// \fn    OnRtnOrderState
	/// \brief 委托变化通知，委托状态变化时回调
	/// \param TEsOrderStateNoticeField & rsp 委托详细数据
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnRtnOrderState(const ESForeign::TEsOrderStateNoticeField& rsp);

	////////////////////////////////////////
	/// \fn    OnRtnOrderInfo
	/// \brief 委托信息变化通知，委托信息变化时回调
	/// \param TEsOrderInfoNoticeField & rsp 委托信息详细数据
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnRtnOrderInfo(const ESForeign::TEsOrderInfoNoticeField& rsp) {};

	////////////////////////////////////////
	/// \fn    OnRspMatchQry
	/// \brief  成交查询应答
	/// \param TEsMatchDataQryRspField * rsp 成交查询结果，查询完成时，指针为空
	/// \param TEsIsLastType islast 指示是否查询完成，完成时rsp指针为空
	/// \param const int iReqID 对应发送请求的ID
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnRspMatchQry(const ESForeign::TEsMatchDataQryRspField* rsp, ESForeign::TEsIsLastType islast, int errCode, const int iReqID);

	////////////////////////////////////////
	/// \fn    OnRtnMatchState
	/// \brief 成交变化推送通知
	/// \param TEsMatchStateNoticeField & rsp 成交详细数据
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnRtnMatchState(const ESForeign::TEsMatchStateNoticeField& rsp);

	////////////////////////////////////////
	/// \fn    OnRtnMatchInfo
	/// \brief 成交信息变化推送通知
	/// \param TEsMatchInfoNoticeField & rsp 成交信息详细数据
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnRtnMatchInfo(const ESForeign::TEsMatchInfoNoticeField& rsp) {};

	////////////////////////////////////////
	/// \fn    OnRspHistMatchQry
	/// \brief 历史成交查询应答
	/// \param TEsHisMatchQryRspField * rsp 历史成交查询结果，查询完成时，指针为空
	/// \param TEsIsLastType islast 指示是否查询完成，完成时rsp指针为空
	/// \param const int iReqID 对应发送请求的ID
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnRspHistMatchQry(const ESForeign::TEsHisMatchQryRspField* rsp, ESForeign::TEsIsLastType islast, int errCode, const int iReqID) {}

	////////////////////////////////////////
	/// \fn    OnQryHold
	/// \brief 持仓查询应答
	/// \param TEsHoldQryRspField * rsp 持仓查询结果，查询完成时，指针为空
	/// \param TEsIsLastType islast 指示是否查询完成，完成时rsp指针为空
	/// \param const int iReqID 对应发送请求的ID
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnQryHold(const ESForeign::TEsHoldQryRspField* rsp, ESForeign::TEsIsLastType islast, int errCode, const int iReqID);

	////////////////////////////////////////
	/// \fn    OnRtnHold
	/// \brief 持仓变化推送通知，暂时未用
	/// \param TEsHoldQryRspField & rsp 持仓详细信息
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnRtnHold(const ESForeign::TEsHoldQryRspField& rsp) {};

	////////////////////////////////////////
	/// \fn    OnRtnExchangeState
	/// \brief 市场状态变更推送通知
	/// \param TEsExchangeQryRspField & rsp 市场状态详细信息
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnRtnExchangeState(const ESForeign::TEsExchangeStateModifyNoticeField& rsp) {};

	////////////////////////////////////////
	/// \fn    OnQryClient
	/// \brief 交易员下属客户信息查询应答
	/// \param TEsOperatorClientQryRspField * rsp 下属客户详细信息，查询完成时，指针为空
	/// \param TEsIsLastType islast 指示是否查询完成，完成时rsp指针为空
	/// \param const int iReqID 对应发送请求的ID
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnQryClient(const ESForeign::TEsOperatorClientQryRspField* rsp, ESForeign::TEsIsLastType islast, int errCode, const int iReqID) {};

	////////////////////////////////////////
	/// \fn    OnRspQryCurrency
	/// \brief 查询货币币种信息应答
	/// \param const TEsCurrencyQryRspField * rsp
	/// \param int errCode
	/// \param const int iReqID
	/// \return void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnRspQryCurrency(const ESForeign::TEsCurrencyQryRspField* rsp, ESForeign::TEsIsLastType islast, int errCode, const int iReqID);

	void Initialize();
public:
	int m_iConnState;
	int m_nLoginState;
	bool m_bLoginSuccess;
	bool m_bLoginDone;
	double m_dAvailable;
	double m_Balance;
	double m_RMBExchangeRate;
	double m_AccountValue;

	ESForeign::IEsunnyTradeApi* m_Api;

	void ReqQryPosition(char ExchangeNo[30], char CommodityNo[30], char InstrumentID[30]);
	void ReqQryOrder(char ExchangeNo[30], char CommodityNo[30], char InstrumentID[30]);
	void ReqQryTrade(char ExchangeNo[30], char CommodityNo[30], char InstrumentID[30], char BeginMatchDateTime[21]);

	int ReqOrderInsert(InsertOrderField* pReq, int* iRetReqID, bool x_bCrossTradingDay);
	void ReqOrderDelete(int OrderId);
	void ReqQryCurrency();
	void ReqQryMoney();

	void Release();
	void Close();
};
