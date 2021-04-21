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

	//�������������
	int		Connect(const char* ip, int port);
	//�Ͽ���ǰ����
	void	DisConnect();
	//��¼���������
	int		Login(const char* user, const char* password);
	//����/ȡ������Ʒ�ּ�ʱ����
	int		RequestQuot(const char* market, const char* stk, int need);
	//����Ʒ����ʷ����
	int		RequestHistory(const char* market, const char* stk, int period);
	//����Ʒ����ϸ����
	int		RequestTrace(const char* market, const char* stk, const char* date);
	//��Զ���Ʒ�ֱȽ϶�ʱ����ѭ������AddReqStk��������һ��SendReqStk
	int		AddReqStk(const char* market, const char* stk, int need);
	//������������Ʒ�ֶ�������
	int		SendReqStk();
	//��ʼ��API��Կ
	int		InitSecretKey(const char* secretkey, int option);

public:
	virtual int __cdecl OnRspLogin(int err, const char* errtext);
	virtual int __cdecl OnChannelLost(int err, const char* errtext);
	virtual int __cdecl OnStkQuot(struct STKDATA* pData);
	virtual int __cdecl OnRspHistoryQuot(struct STKHISDATA* pHisData);
	virtual int __cdecl OnRspTraceData(struct STKTRACEDATA* pTraceData);
	virtual int __cdecl OnRspMarketInfo(struct MarketInfo* pMarketInfo, int bLast);
};