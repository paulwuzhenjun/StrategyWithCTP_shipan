#pragma once
#include "EsunnyQuot.h"

#pragma  comment(lib,"EsunnyQuot.lib")
class CEsunMdSpi : public IEsunnyQuotNotify
{
public:
	CEsunMdSpi();
	~CEsunMdSpi(void);
	bool timeRuleForOpen(char datatime[10]);
	int m_count;
	char beginrun_date[10];

	IEsunnyQuotClient* m_pQuoteAPI;
	bool m_bInitOK;
	bool m_bLoginSuccess;
	bool m_bLoginDone;
	//vector<MarketInfo> m_MarKetInfoVector;

public:

	char m_buffer[10 * 1024];

	//连接行情服务器
	int		Connect(const char* ip, int port);
	//断开当前连接
	void	DisConnect();
	//登录行情服务器
	int		Login(const char* user, const char* password);
	//请求/取消订阅品种即时行情
	int		RequestQuot(const char* market, const char* stk, int need);
	//请求品种历史行情
	int		RequestHistory(const char* market, const char* stk, int period);
	//请求品种明细数据
	int		RequestTrace(const char* market, const char* stk, const char* date);
	//针对订阅品种比较多时，先循环调用AddReqStk，最后调用一次SendReqStk
	int		AddReqStk(const char* market, const char* stk, int need);
	//给服务器发送品种订阅请求
	int		SendReqStk();
	//初始化API密钥
	int		InitSecretKey(const char* secretkey, int option);

public:
	virtual int __cdecl OnRspLogin(int err, const char* errtext);
	virtual int __cdecl OnChannelLost(int err, const char* errtext);
	virtual int __cdecl OnStkQuot(struct STKDATA* pData);
	virtual int __cdecl OnRspHistoryQuot(struct STKHISDATA* pHisData);
	virtual int __cdecl OnRspTraceData(struct STKTRACEDATA* pTraceData);
	virtual int __cdecl OnRspMarketInfo(struct MarketInfo* pMarketInfo, int bLast);
};