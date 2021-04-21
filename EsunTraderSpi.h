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
	STATE_DISCONNECTED = 1,    //δ����
	STATE_CONNECTED = 2,    //������
	STATE_LOGIN = 3     //�ѵ�¼
};

enum OrderState
{
	ORDER_INVALID = 0,    //��Ч��
	ORDER_NEW = 1,    //�µ�
	ORDER_IN_QUEUE = 2,    //���Ŷ�
	ORDER_ALL_MATCH = 3,    //��ȫ�ɽ�
	ORDER_IN_CANCEL = 4,    //��������
	ORDER_CANCELED = 5,    //�ѳ���
	ORDER_CHECK = 6,	//��
	ORDER_CHECK_DELETE = 7,    //��ɾ��
	ORDER_ALREADY_SENT = 8,    //�ѷ���
	ORDER_TO_CANCEL = 9,    //����
	ORDER_PART_MATCH = 10,   //���ֳɽ�
	ORDER_NON_TRADING_HOURS = 11,  //�ǽ���ʱ��
	ORDER_INSUFFICIENT_FUNDS = 12,  //�ʽ���
	ORDER_TRADE_CLOSED = 13,  //���׹ر�
	ORDER_INSTRUCT_FAILED = 14,  //ָ��ʧ��
	ORDER_WITHDRAWALS_FAILED = 15,  //����ʧ��
	ORDER_SYSTEM_WITHDRAWALS = 16,  //ϵͳ����
	ORDER_TRIGGER_ORDER = 17,  //������
	ORDER_TRIGGER_ORDER_DELETE = 18,  //������ɾ��
	ORDER_SUSPEND = 19,  //����
	ORDER_ACTIVATE = 20,  //����
	ORDER_ACCEPTED = 21,  //������
	ORDER_AUTO = 22,  //�Զ���
	ORDER_AUTO_DELTE = 23   //�Զ���ɾ��
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
	/// \brief ��������Ͽ�����ʱ����
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnClose();

	////////////////////////////////////////
	/// \fn    OnLogin
	/// \brief ����Login��½�ɹ�ʱ�յ���������½��Ӧ����
	/// \param const TEsLoginRspField & rsp
	/// \param int iReqID
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnLogin(const ESForeign::TEsLoginRspField* rsp, int errCode, const int iReqID);

	////////////////////////////////////////
	/// \fn    OnInitFinished
	/// \brief ����Login�ɹ����յ�OnLoginӦ��ɹ����յ���ʼ���������
	///        ���е�ҵ�������Ҫ�ڱ���ӦerrCodeΪ0���ɹ�����ɽ���
	/// \param int errCode �����룬0-�ɹ�������ֵ-�����ԭ�����
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnInitFinished(int errCode);

	////////////////////////////////////////
	/// \fn    OnLogOut
	/// \brief �յ��ǳ�Ӧ�����
	/// \param int errCode
	/// \param const int iReqID
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnLogOut(int errCode, const int iReqID) {};

	////////////////////////////////////////
	/// \fn    OnRspSetPassword
	/// \brief  �޸Ŀͻ�����ʱӦ�𣬷�����Ϣ�����޸�������ϸ����
	/// \param const TEsClientPasswordModifyRspField & rsp
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnRspSetPassword(const ESForeign::TEsClientPasswordModifyRspField* rsp, int errCode, const int iReqID) {};

	////////////////////////////////////////
	/// \fn    OnRspSetOperPassword
	/// \brief �޸Ĳ���Ա����ʱӦ�𣬷�����Ϣ�����޸�������ϸ����
	/// \param const TEsOperatorPasswordModifyRspField & rsp
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnRspSetOperPassword(const ESForeign::TEsOperatorPasswordModifyRspField* rsp, int errCode, const int iReqID) {};

	////////////////////////////////////////
	/// \fn    OnQryMoney
	/// \brief �ʽ��ѯӦ��
	/// \param TEsMoneyQryRspField * rsp ��ѯ���ʱ��ָ��Ϊ�գ�δ���ʱ������ѯ���
	/// \param TEsIsLastType islast ָʾ�Ƿ��ѯ��ɣ����ʱrspָ��Ϊ��
	/// \param const int iReqID ��Ӧ���������ID
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnQryMoney(const ESForeign::TEsMoneyQryRspField* rsp, ESForeign::TEsIsLastType islast, int errCode, const int iReqID);

	////////////////////////////////////////
	/// \fn    OnRtnMoney
	/// \brief �ʽ�仯����֪ͨ
	/// \param TEsMoneyChgNoticeField & rsp �ʽ�仯��ϸ��Ϣ
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnRtnMoney(const ESForeign::TEsMoneyChgNoticeField& rsp);

	////////////////////////////////////////
	/// \fn    OnRspCashOperQry
	/// \brief ��������ѯӦ��
	/// \param TEsCashOperRspField * rsp��ѯ���ʱ��ָ��Ϊ�գ�δ���ʱ������ѯ���
	/// \param TEsIsLastType islast ָʾ�Ƿ��ѯ��ɣ����ʱrspָ��Ϊ��
	/// \param const int iReqID ��Ӧ���������ID
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnRspCashOperQry(const ESForeign::TEsCashOperQryRspField* rsp, ESForeign::TEsIsLastType islast, int errCode, const int iReqID) {};

	////////////////////////////////////////
	/// \fn    OnRspCashAdjustQry
	/// \brief �ʽ������ѯӦ��
	/// \param TEsAdjustOperRspField * rsp ��ѯ�������ѯ���ʱ��ָ��Ϊ��
	/// \param TEsIsLastType islast ָʾ�Ƿ��ѯ��ɣ����ʱrspָ��Ϊ��
	/// \param const int iReqID ��Ӧ���������ID
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnRspCashAdjustQry(const ESForeign::TEsAdjustQryRspField* rsp, ESForeign::TEsIsLastType islast, int errCode, const int iReqID) {};

	////////////////////////////////////////
	/// \fn    OnRspOrderInsert
	/// \brief ���������Ӧ��
	/// \param TEsOrderInsertRspField & rsp �����������ϸ������᷵��ί�кż����غ�
	/// \param const int iReqID ��Ӧ���������ID
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnRspOrderInsert(const ESForeign::TEsOrderInsertRspField* rsp, int errCode, const int iReqID);

	////////////////////////////////////////
	/// \fn    OnRspOrderModify
	/// \brief �ĵ������Ӧ�𣬳���ʱ���յ���Ӧ��
	/// \param TEsOrderModifyRspField & rsp
	/// \param const int iReqID ��Ӧ���������ID
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnRspOrderModify(const ESForeign::TEsOrderModifyRspField* rsp, int errCode, const int iReqID) {};

	////////////////////////////////////////
	/// \fn    OnRspOrderDelete
	/// \brief ���������Ӧ�𣬳���ʱ���յ���Ӧ��
	/// \param TEsOrderDeleteRspField & rsp
	/// \param const int iReqID ��Ӧ���������ID
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnRspOrderDelete(const ESForeign::TEsOrderDeleteRspField* rsp, int errCode, const int iReqID);

	////////////////////////////////////////
	/// \fn    OnRspQryOrder
	/// \brief ί�в�ѯӦ��
	/// \param TEsOrderDataQryRspField * rsp ί�в�ѯ�������ѯ���ʱ��ָ��Ϊ��
	/// \param TEsIsLastType islast ָʾ�Ƿ��ѯ��ɣ����ʱrspָ��Ϊ��
	/// \param const int iReqID ��Ӧ���������ID
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnRspQryOrder(const ESForeign::TEsOrderDataQryRspField* rsp, ESForeign::TEsIsLastType islast, int errCode, const int iReqID);

	////////////////////////////////////////
	/// \fn    OnRspHistOrderQry
	/// \brief ��ʷί�в�ѯӦ��
	/// \param TEsHisOrderQryRspField * rsp ��ʷί�в�ѯ�������ѯ���ʱ��ָ��Ϊ��
	/// \param TEsIsLastType islast ָʾ�Ƿ��ѯ��ɣ����ʱrspָ��Ϊ��
	/// \param const int iReqID ��Ӧ���������ID
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnRspHistOrderQry(const ESForeign::TEsHisOrderQryRspField* rsp, ESForeign::TEsIsLastType islast, int errCode, const int iReqID) {};

	////////////////////////////////////////
	/// \fn    OnRtnOrderState
	/// \brief ί�б仯֪ͨ��ί��״̬�仯ʱ�ص�
	/// \param TEsOrderStateNoticeField & rsp ί����ϸ����
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnRtnOrderState(const ESForeign::TEsOrderStateNoticeField& rsp);

	////////////////////////////////////////
	/// \fn    OnRtnOrderInfo
	/// \brief ί����Ϣ�仯֪ͨ��ί����Ϣ�仯ʱ�ص�
	/// \param TEsOrderInfoNoticeField & rsp ί����Ϣ��ϸ����
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnRtnOrderInfo(const ESForeign::TEsOrderInfoNoticeField& rsp) {};

	////////////////////////////////////////
	/// \fn    OnRspMatchQry
	/// \brief  �ɽ���ѯӦ��
	/// \param TEsMatchDataQryRspField * rsp �ɽ���ѯ�������ѯ���ʱ��ָ��Ϊ��
	/// \param TEsIsLastType islast ָʾ�Ƿ��ѯ��ɣ����ʱrspָ��Ϊ��
	/// \param const int iReqID ��Ӧ���������ID
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnRspMatchQry(const ESForeign::TEsMatchDataQryRspField* rsp, ESForeign::TEsIsLastType islast, int errCode, const int iReqID);

	////////////////////////////////////////
	/// \fn    OnRtnMatchState
	/// \brief �ɽ��仯����֪ͨ
	/// \param TEsMatchStateNoticeField & rsp �ɽ���ϸ����
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnRtnMatchState(const ESForeign::TEsMatchStateNoticeField& rsp);

	////////////////////////////////////////
	/// \fn    OnRtnMatchInfo
	/// \brief �ɽ���Ϣ�仯����֪ͨ
	/// \param TEsMatchInfoNoticeField & rsp �ɽ���Ϣ��ϸ����
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnRtnMatchInfo(const ESForeign::TEsMatchInfoNoticeField& rsp) {};

	////////////////////////////////////////
	/// \fn    OnRspHistMatchQry
	/// \brief ��ʷ�ɽ���ѯӦ��
	/// \param TEsHisMatchQryRspField * rsp ��ʷ�ɽ���ѯ�������ѯ���ʱ��ָ��Ϊ��
	/// \param TEsIsLastType islast ָʾ�Ƿ��ѯ��ɣ����ʱrspָ��Ϊ��
	/// \param const int iReqID ��Ӧ���������ID
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnRspHistMatchQry(const ESForeign::TEsHisMatchQryRspField* rsp, ESForeign::TEsIsLastType islast, int errCode, const int iReqID) {}

	////////////////////////////////////////
	/// \fn    OnQryHold
	/// \brief �ֲֲ�ѯӦ��
	/// \param TEsHoldQryRspField * rsp �ֲֲ�ѯ�������ѯ���ʱ��ָ��Ϊ��
	/// \param TEsIsLastType islast ָʾ�Ƿ��ѯ��ɣ����ʱrspָ��Ϊ��
	/// \param const int iReqID ��Ӧ���������ID
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnQryHold(const ESForeign::TEsHoldQryRspField* rsp, ESForeign::TEsIsLastType islast, int errCode, const int iReqID);

	////////////////////////////////////////
	/// \fn    OnRtnHold
	/// \brief �ֱֲ仯����֪ͨ����ʱδ��
	/// \param TEsHoldQryRspField & rsp �ֲ���ϸ��Ϣ
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnRtnHold(const ESForeign::TEsHoldQryRspField& rsp) {};

	////////////////////////////////////////
	/// \fn    OnRtnExchangeState
	/// \brief �г�״̬�������֪ͨ
	/// \param TEsExchangeQryRspField & rsp �г�״̬��ϸ��Ϣ
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnRtnExchangeState(const ESForeign::TEsExchangeStateModifyNoticeField& rsp) {};

	////////////////////////////////////////
	/// \fn    OnQryClient
	/// \brief ����Ա�����ͻ���Ϣ��ѯӦ��
	/// \param TEsOperatorClientQryRspField * rsp �����ͻ���ϸ��Ϣ����ѯ���ʱ��ָ��Ϊ��
	/// \param TEsIsLastType islast ָʾ�Ƿ��ѯ��ɣ����ʱrspָ��Ϊ��
	/// \param const int iReqID ��Ӧ���������ID
	/// \return   void __cdecl
	////////////////////////////////////////
	virtual void __cdecl OnQryClient(const ESForeign::TEsOperatorClientQryRspField* rsp, ESForeign::TEsIsLastType islast, int errCode, const int iReqID) {};

	////////////////////////////////////////
	/// \fn    OnRspQryCurrency
	/// \brief ��ѯ���ұ�����ϢӦ��
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
