#ifndef _ESUNNY_FOREIGN_TRADE_API_TYPE_H
#define _ESUNNY_FOREIGN_TRADE_API_TYPE_H

namespace ESForeign
{
    //��֤��Ϣ
    typedef char TEsCertInfoType[401];

    //��־·��
    typedef char TEsLogPathType[1024];

    //�汾��Ϣ
    typedef char TEsVersionType[51];

	//Ӧ�ó���ID
	typedef char TEsAppIDType[101];

    //��½������Ϣ����Ҫ����mac��ַ
    typedef char					TMachineInfoType[201];

    typedef unsigned char  u_char;
    typedef	unsigned short u_short;

    //��������
    typedef unsigned int TEsSizeType;

    ///////////////////////////////////////////////////////////////////////////
    //���������(0-��ʾ�ɹ�)������Ŷ���μ�Protocol_Error.h
    typedef int						TErrorCodeType;
    //�ɹ�
    const TErrorCodeType			ERROR_SUCCEED					= 0;

    //�Ƿ��ѯ����
    typedef bool TEsIsLastType;

    ///////////////////////////////////////////////////////////////////////////
    //�Ƿ�CA��֤����
    typedef char					TIsCaLoginType;
    //CA��֤��ʽ
    const TIsCaLoginType			CA_LOGIN						= 'Y';
    //��CA��֤��ʽ
    const TIsCaLoginType			NOT_CA_LOGIN					= 'N';

    ///////////////////////////////////////////////////////////////////////////
    //�Ƿ�ǿ���޸���������
    typedef char					TIsForcePasswordType;
    //ǿ���޸�����
    const TIsForcePasswordType		FORCE_PWD						= 'Y';
    //��ǿ���޸�����
    const TIsForcePasswordType		NOT_FORCE_PWD					= 'N';

    ///////////////////////////////////////////////////////////////////////////
    //��¼�������
    typedef char					TEsLoginIdentityType;
    //�µ��ͻ���
    const TEsLoginIdentityType		IDENTITY_CLIENT					= 'c';
    //�µ��˶�
    const TEsLoginIdentityType		IDENTITY_TCLIENT				= 'd';
    //�����˶�
    const TEsLoginIdentityType		IDENTITY_BROKER					= 'b';
    //����Ա��
    const TEsLoginIdentityType		IDENTITY_TRADER					= 't';
    //��̨�����
    const TEsLoginIdentityType		IDENTITY_MANAGER				= 'm';

    ///////////////////////////////////////////////////////////////////////////
    //��¼������
    typedef char					TLoginNoType[21];
    //��¼��������(�ͻ����,����Ա����)
    typedef char					TLoginNameType[21];
    //��¼��������
    typedef	char					TLoginPasswordType[21];
    //otp����
    typedef char					TOtpPassType[21];

    ///////////////////////////////////////////////////////////////////////////
    //Ip��ַ����(ipv4���15λ,ipv6��󳤶�39λ)
    typedef char					TIpType[41];

    ///////////////////////////////////////////////////////////////////////////
    //port�˿�����
    typedef u_short					TPortType;

    ///////////////////////////////////////////////////////////////////////////
    //CA��֤����������(�������ݰ���β,ͨ�����ݰ����Ȼ�ȡ���ݳ���)
    typedef char					TCaInfoType[1];

    ///////////////////////////////////////////////////////////////////////////
    //�ͻ�Ԥ����Ϣ����
    typedef char					TReservedInfoType[51];

    ///////////////////////////////////////////////////////////////////////////
    //�ͻ��������
    typedef char					TClientNoType[21];

    ///////////////////////////////////////////////////////////////////////////
    //����Ա�������
    typedef char					TOperatorNoType[11];
    //����Ա��������
    typedef char					TOperatorNameType[21];

    ///////////////////////////////////////////////////////////////////////////
    //���ں�ʱ������(��ʽ yyyy-MM-dd hh:nn:ss)
    typedef char					TDateTimeType[21];
    //��������(��ʽ yyyy-MM-dd)
    typedef char					TDateType[11];
    //ʱ������(��ʽ hh:nn:ss)
    typedef char					TTimeType[11];

    ///////////////////////////////////////////////////////////////////////////
    //���ұ������
    typedef char					TCurrencyNoType[11];
    //������������
    typedef char					TCurrencyNameType[21];

    ///////////////////////////////////////////////////////////////////////////
    //�Ƿ��������
    typedef char					TIsPrimaryCurrencyType;
    //�ǻ���
    const TIsPrimaryCurrencyType	CURRENCY_PRIMARY_YES			= 'Y';
    //���ǻ���
    const TIsPrimaryCurrencyType	CURRENCY_PRIMARY_NO				= 'N';

    ///////////////////////////////////////////////////////////////////////////
	//�������־(ͬһ�����飬�ʽ���)
	typedef char					TCurrencyGroupFlagType;
	//������A
	const TCurrencyGroupFlagType			CURRENCY_GROUP_A				= 'A';
	//������B
	const TCurrencyGroupFlagType			CURRENCY_GROUP_B				= 'B';
	//������C
	const TCurrencyGroupFlagType			CURRENCY_GROUP_C				= 'C';
	//������D
	const TCurrencyGroupFlagType			CURRENCY_GROUP_D				= 'D';
	//������E
	const TCurrencyGroupFlagType			CURRENCY_GROUP_E				= 'E';
	
    ///////////////////////////////////////////////////////////////////////////
    //����(1��ǰ���һ�����ٻ���)
    typedef double					TExchangeRateType;

    ///////////////////////////////////////////////////////////////////////////
    //�г��������
    typedef char					TExchangeNoType[11];
    //�г���������
    typedef char					TExchangeNameType[21];

    ///////////////////////////////////////////////////////////////////////////
    //�г�״̬����
    typedef char					TExchangeStateType;
    //�г�������
    const TExchangeStateType		EXCHANGE_STATE_YES				= 'Y';
    //�г���ֹ����
    const TExchangeStateType		EXCHANGE_STATE_NO				= 'N';
    //�г�ֻ��ƽ��
    const TExchangeStateType		EXCHANGE_STATE_COVER			= 'C';

    ///////////////////////////////////////////////////////////////////////////
    //ƽ�ַ�ʽ����
    typedef char					TCoverModeType;
    //�����ֿ�ƽ
    const TCoverModeType			COVER_MODE_NONE					= 'N';
    //ƽ��δ�˽�
    const TCoverModeType			COVER_MODE_UNFINISHED			= 'U';
    //���ֺ�ƽ��
    const TCoverModeType			COVER_MODE_OPENCOVER			= 'C';
    //���֡�ƽ�ֺ�ƽ��
    const TCoverModeType			COVER_MODE_COVERTODAY			= 'T';

    ///////////////////////////////////////////////////////////////////////////
    //��Ʒ�������(ͬһ�г��ڲ�Ψһ  �г�+��Ʒ����+��Ʒ Ψһ)
    typedef char					TCommodityNoType[11];
    //��Ʒ��������
    typedef char					TCommodityNameType[21];

    ///////////////////////////////////////////////////////////////////////////
    //��Ʒ��������
    typedef char					TCommodityTypeType;
    //�ֻ�
    const TCommodityTypeType		COMMODITY_TYPE_GOODS			= 'G';
    //�ڻ�
    const TCommodityTypeType		COMMODITY_TYPE_FUTURE			= 'F';
    //��Ȩ
    const TCommodityTypeType		COMMODITY_TYPE_OPTION			= 'O';
    //��������
    const TCommodityTypeType		COMMODITY_TYPE_SPREAD_MONTH		= 'M';
    //��Ʒ������
    const TCommodityTypeType		COMMODITY_TYPE_SPREAD_COMMODITY	= 'C';

    ///////////////////////////////////////////////////////////////////////////
    //��Ʒ��������
    typedef char					TCommodityAttributeType[21];

    ///////////////////////////////////////////////////////////////////////////
    //��Ʒ״̬����
    typedef char					TCommodityStateType;
    //��Ʒ������
    const TCommodityStateType		COMMODITY_STATE_YES				= 'Y';
    //��Ʒ��ֹ����
    const TCommodityStateType		COMMODITY_STATE_NO				= 'N';
    //��Ʒֻ��ƽ��
    const TCommodityStateType		COMMODITY_STATE_COVER			= 'C';

    ///////////////////////////////////////////////////////////////////////////
    //ÿ�ֳ�������(������ծ��С��,LIFFEС�ƽ�С��)
    typedef double					TProductDotType;

    ///////////////////////////////////////////////////////////////////////////
    //��С�䶯�۷�������
    typedef double					TUpperTickType;

    ///////////////////////////////////////////////////////////////////////////
    //��С�䶯�۷�ĸ����(�Ƿ�������Ϊ1)
    typedef int						TLowerTickType;

    ///////////////////////////////////////////////////////////////////////////
    //���ʽ����
    typedef char					TDeliveryModeType;
    //ʵ�ｻ��
    const TDeliveryModeType			DELIVERY_MODE_GOODS				= 'G';
    //�ֽ𽻸�
    const TDeliveryModeType			DELIVERY_MODE_CASH				= 'C';
    //��Ȩ��Ȩ
    const TDeliveryModeType			DELIVERY_MODE_EXECUTE			= 'E';

    ///////////////////////////////////////////////////////////////////////////
    //������ƫ������(0����ͬ���������)
    typedef int						TDeliveryDaysType;

    ///////////////////////////////////////////////////////////////////////////
    //�ֲּ��㷽ʽ
    typedef char					TDepositCalculateModeType;
    //����
    const TDepositCalculateModeType	DEPOSIT_CALCULATE_MODE_NORMAL	= 'N';
    //��Լ���ֲ�
    const TDepositCalculateModeType	DEPOSIT_CALCULATE_MODE_CLEAN	= 'C';
    //Ʒ������
    const TDepositCalculateModeType DEPOSIT_CALCULATE_MODE_LOCK		= 'L';

    ///////////////////////////////////////////////////////////////////////////
    //��Լ�������
    typedef char					TContractNoType[71];
    //��Լ��������
    typedef char					TContractNameType[21];
    //��Լ����������(��Լ���м��쵽��)
    typedef int						TContractLastDays;

    ///////////////////////////////////////////////////////////////////////////
    //��Լ��������
    typedef char					TContractTypeType;
    //���Ⱥ�Լ
    const TContractTypeType			CONTRACT_TYPE_SINGLE			= '0';
    //��������
    const TContractTypeType			CONTRACT_TYPE_SPREAD			= '1';
    //��������
    const TContractTypeType			CONTRACT_TYPE_SWAP				= '2';
    //��Ʒ������
    const TContractTypeType			CONTRACT_TYPE_COMMODITY			= '3';

    ///////////////////////////////////////////////////////////////////////////
    //��Լ״̬����
    typedef char					TContractStateType;
    //��Լ������
    const TContractStateType		CONTRACT_STATE_YES				= 'Y';
    //��Լ��ֹ����
    const TContractStateType		CONTRACT_STATE_NO				= 'N';
    //��Լֻ��ƽ��
    const TContractStateType		CONTRACT_STATE_COVER			= 'C';

    ///////////////////////////////////////////////////////////////////////////
    //����۸�����
    typedef double					TQuotePriceType;

    ///////////////////////////////////////////////////////////////////////////
    //������������
    typedef int						TQuoteVolType;

    ///////////////////////////////////////////////////////////////////////////
    //�ͻ��������
    typedef char					TClientShortNameType[21];

    ///////////////////////////////////////////////////////////////////////////
    //�ͻ�״̬
    typedef char					TClientStateType;
    //����
    const TClientStateType			CLIENT_STATE_NORMAL				= 'N';
    //����
    const TClientStateType			CLIENT_STATE_CANCEL				= 'C';

    ///////////////////////////////////////////////////////////////////////////
    //�ͻ���������
    typedef char					TClientTypeType;
    //���˿ͻ�
    const TClientTypeType			CLIENT_TYPE_PERSON				= 'P';
    //�����ͻ�
    const TClientTypeType			CLIENT_TYPE_ORGANIZATION		= 'O';

    ///////////////////////////////////////////////////////////////////////////
    //�ƶ��绰����
    typedef char					TMobilePhoneType[21];

    ///////////////////////////////////////////////////////////////////////////
    //�ͻ�����״̬����
    typedef char					TClientTradeStateType;
    //��������
    const TClientTradeStateType		CLIENT_TRADE_YES				= 'Y';
    //��ֹ��¼
    const TClientTradeStateType		CLIENT_TRADE_NO					= 'N';
    //ֻ�ɲ�ѯ
    const TClientTradeStateType		CLIENT_TRADE_QUERY				= 'Q';
    //ֻ��ƽ��
    const TClientTradeStateType		CLIENT_TRADE_COVER				= 'C';
    //����
    const TClientTradeStateType		CLIENT_TRADE_FROZEN				= 'F';

    ///////////////////////////////////////////////////////////////////////////
    //��½������(0-255,0�൱�ڽ�ֹ��¼)
    typedef unsigned char			TLoginCountType;

    ///////////////////////////////////////////////////////////////////////////
    //�µ�������
    typedef char					TDealManType[21];
    //�µ���֤������
    typedef char					TDealManIdType[21];

    ///////////////////////////////////////////////////////////////////////////
    //�ͻ�����״̬����
    typedef char					TClientQuoteStateType;
    //������¼
    const TClientQuoteStateType		CLIENT_QUOTE_YES				= 'Y';
    //��ֹ��¼
    const TClientQuoteStateType		CLIENT_QUOTE_NO					= 'N';

    ///////////////////////////////////////////////////////////////////////////
    //�ͻ�ȫ������
    typedef char					TClientFullNameType[101];

    ///////////////////////////////////////////////////////////////////////////
    //֤����������
    typedef char					TIdentityNoType[21];

    ///////////////////////////////////////////////////////////////////////////
    //��ַ����
    typedef char					TAddressType[101];

    ///////////////////////////////////////////////////////////////////////////
    //�ʱ��������
    typedef char					TPostNoType[21];

    ///////////////////////////////////////////////////////////////////////////
    //�ͻ�������Ϣ��ע����
    typedef char					TClientInfoRemark[201];

    ///////////////////////////////////////////////////////////////////////////
    //����Ա״̬����
    typedef char					TOperatorStateType;
    //��������
    const TOperatorStateType		OPERATOR_TRADE_YES				= 'Y';
    //��ֹ��¼
    const TOperatorStateType		OPERATOR_TRADE_NO				= 'N';
    //ֻ�ɵ�¼��ѯ
    const TOperatorStateType		OPERATOR_TRADE_QUERY			= 'Q';
    //ֻ��ƽ��
    const TOperatorStateType		OPERATOR_TRADE_COVER			= 'C';

    ///////////////////////////////////////////////////////////////////////////
    //����Ա��¼��ʽ
    typedef char					TOperatorLoginType;
    //�ܹ���¼�ͻ�ǰ��
    const TOperatorLoginType		OPERATOR_LOGIN_EVERY_FRONT		= 'Y';
    //���ܵ�¼�ͻ�ǰ��
    const TOperatorLoginType		OPERATOR_LOGIN_NOT_EVERY_FRONT	= 'N';

    ///////////////////////////////////////////////////////////////////////////
    //�ͻ����ʺ�����
    typedef char					TGroupNoType[11];
    //�ͻ�����������
    typedef char					TGroupNameType[21];

    ///////////////////////////////////////////////////////////////////////////
    //�ͻ�����������
    typedef char					TGroupTypeType;
    //���׷���(һ���ͻ����Է��䵽�������)
    const TGroupTypeType			GROUP_TYPE_TRADE				= 'T';
    //�������(һ���ͻ�ֻ���Է��䵽һ���������)
    const TGroupTypeType			GROUP_TYPE_SETTLE				= 'S';
    //�����Ѳ�����(һ���ͻ�ֻ���Է��䵽һ����������)
    const TGroupTypeType			GROUP_TYPE_FEERENT				= 'F';
    //��֤�������(һ���ͻ�ֻ���Է��䵽һ����֤����)
    const TGroupTypeType			GROUP_TYPE_DEPOSITRENT			= 'D';
    //�ʽ�����(һ���ͻ�ֻ���Է��䵽һ���ʽ�����,ͬһ�ʽ�����Ŀͻ������ܹ�����ռ���ʽ�)
    const TGroupTypeType			GROUP_TYPE_MONEY				= 'M';

    ///////////////////////////////////////////////////////////////////////////
    //��ɫ�������
    typedef char					TRoleNoType[11];
    //��ɫ��������
    typedef char					TRoleNameType[21];

    ///////////////////////////////////////////////////////////////////////////
    //��ɫ�������
    typedef int						TRightIdType;
    //��ɫ��������
    typedef char					TRightNameType[21];

    ///////////////////////////////////////////////////////////////////////////
    //��������
    typedef char					TPasswordTypeType;
    //��������
    const TPasswordTypeType			PASSWORD_TYPE_TRADE				= 'T';
    //��������
    const TPasswordTypeType			PASSWORD_TYPE_QUOTE				= 'Q';
    //��֤����
    const TPasswordTypeType			PASSWORD_TYPE_AUTH				= 'A';

    ///////////////////////////////////////////////////////////////////////////
    //ί�б�������(����)����
    typedef int						TOrderSaveIntType;
    //ί�б�������(������)����
    typedef double					TOrderSaveDoubleType;
    //ί�б�������(�ַ���)����
    typedef char					TOrderSaveStringType[51];

    ///////////////////////////////////////////////////////////////////////////
    //ί������
    typedef char					TOrderTypeType;
    //�޼�
    const TOrderTypeType			ORDER_TYPE_LIMIT				= '1';
    //�м�
    const TOrderTypeType			ORDER_TYPE_MARKET				= '2';
    //�޼�ֹ��
    const TOrderTypeType			ORDER_TYPE_STOP_LIMIT			= '3';
    //�м�ֹ��
    const TOrderTypeType			ORDER_TYPE_STOP_MARKET			= '4';

    ///////////////////////////////////////////////////////////////////////////
    //ί�з�ʽ
    typedef char					TOrderWayType;
    //�������ӵ�
    const char						ORDER_WAY_SELF_ETRADER			= 'E';
    //������ӵ�
    const char						ORDER_WAY_PROXY_ETRADER			= 'P';
    //�ⲿ���ӵ�(�ⲿ����ϵͳ�µ�����ϵͳ¼��)
    const char						ORDER_WAY_JTRADER				= 'J';
    //�˹�¼�뵥(�ⲿ������ʽ�µ�����ϵͳ¼��)
    const char						ORDER_WAY_MANUAL				= 'M';
    //carry��
    const char						ORDER_WAY_CARRY					= 'C';
    //��ʽ������
    const char						ORDER_WAY_PROGRAM				= 'S';

    ///////////////////////////////////////////////////////////////////////////
    //ί��ģʽ����
    typedef char					TOrderModeType;
    //FOK
    const TOrderModeType			ORDER_MODE_FOK					= '1';
    //FAK��IOC
    const TOrderModeType			ORDER_MODE_FAK					= '2';
    //������Ч
    const TOrderModeType			ORDER_MODE_GFD					= '3';
    //ȡ��ǰ��Ч
    const TOrderModeType			ORDER_MODE_GTC					= '4';
    //ָ������ǰ��Ч
    const TOrderModeType			ORDER_MODE_GTD					= '5';

    ///////////////////////////////////////////////////////////////////////////
    //�ɽ���ʽ
    typedef char					TMatchWayType;
    //����
    const char						MATCH_WAY_ALL					= 'A';
    //�������ӵ�
    const char						MATCH_WAY_SELF_ETRADER			= 'E';
    //������ӵ�
    const char						MATCH_WAY_PROXY_ETRADER			= 'P';
    //�ⲿ���ӵ�(�ⲿ����ϵͳ�µ�����ϵͳ¼��)
    const char						MATCH_WAY_JTRADER				= 'J';
    //�˹�¼�뵥(�ⲿ������ʽ�µ�����ϵͳ¼��)
    const char						MATCH_WAY_MANUAL				= 'M';
    //carry��
    const char						MATCH_WAY_CARRY					= 'C';
    //���
    const char						MATCH_WAY_DELIVERY				= 'D';
    //��ʽ����
    const char						MATCH_WAY_PROGRAM				= 'S';


    ///////////////////////////////////////////////////////////////////////////
    //�Ƿ���ձ���
    typedef char					TIsRiskOrderType;
    //�Ƿ��ձ���
    const TIsRiskOrderType			RISK_ORDER_YES					= 'Y';
    //���Ƿ��ձ���
    const TIsRiskOrderType			RISK_ORDER_NO					= 'N';

    ///////////////////////////////////////////////////////////////////////////
    //��������
    typedef char					TDirectType;
    //����
    const TDirectType				DIRECT_BUY						= 'B';
    //����
    const TDirectType				DIRECT_SELL						= 'S';

    ///////////////////////////////////////////////////////////////////////////
    //��ƽ����
    typedef char					TOffsetType;
    //��
    const TOffsetType				OFFSET_NONE						= 'N';
    //����
    const TOffsetType				OFFSET_OPEN						= 'O';
    //ƽ��
    const TOffsetType				OFFSET_COVER					= 'C';
    //ƽ��
    const TOffsetType				OFFSET_COVER_TODAY				= 'T';

    ///////////////////////////////////////////////////////////////////////////
    //Ͷ����ֵ����
    typedef char					THedgeType;
    //��
    const THedgeType				HEDGE_NONE						= 'N';
    //Ͷ��
    const THedgeType				HEDGE_T							= 'T';
    //��ֵ
    const THedgeType				HEDGE_B							= 'B';

    ///////////////////////////////////////////////////////////////////////////
    //���׼۸�����
    typedef double					TTradePriceType;
    //������������
    typedef int						TTradeVolType;

    ///////////////////////////////////////////////////////////////////////////
    //ί�б������(ÿ�����׷�������Ψһ��־,�ɷ���������)
    typedef int						TOrderIdType;
    //���ر������(ÿ�����׷�������Ψһ��־,�ɷ���������)
    typedef char					TLocalNoType[21];
    //ϵͳ�������(�ϼ�����������Ψһ��־,��ͬ���������ܻ��ظ�)
    typedef char					TSystemNoType[64];
    //�ɽ����
    typedef char					TMatchNoType[71];
    //���ر������(ÿ�����׷�������Ψһ��־,�ɷ���������)
    typedef char					TFeLocalNoType[sizeof(TLocalNoType) + 1];

    ///////////////////////////////////////////////////////////////////////////
    //ί��¼������
    typedef char					TOrderInputType;
    //��
    const TOrderInputType			ORDER_INPUT_YES					= 'Y';
    //��
    const TOrderInputType			ORDER_INPUT_NO					= 'N';

    ///////////////////////////////////////////////////////////////////////////
    //T+1�ɽ�
    typedef char					TAddOneType;
    //T+1�ɽ�
    const TAddOneType				ADD_ONE_YES						= 'Y';
    //��T+1�ɽ�
    const TAddOneType				ADD_ONE_NO						= 'N';

    ///////////////////////////////////////////////////////////////////////////
    //�˹���д������
    typedef char					TManualFeeType;
    //�˹�
    const TManualFeeType			MANUALFEE_YES					= 'Y';
    //�Զ�
    const TManualFeeType			MANUALFEE_NO					= 'N';

    ///////////////////////////////////////////////////////////////////////////
    //ί��״̬����
    typedef char					TOrderStateType;
    //ָ��ʧ��
    const TOrderStateType			ORDER_STATE_FAIL				= '0';
    //������
    const TOrderStateType			ORDER_STATE_ACCEPT				= '1';
    //�ѹ���
    const TOrderStateType			ORDER_STATE_SUPPENDED			= '2';
    //���Ŷ�
    const TOrderStateType			ORDER_STATE_QUEUED				= '3';
    //������(�Ŷ���ʱ״̬)
    const TOrderStateType			ORDER_STATE_DELETEING			= '4';
    //���޸�(�Ŷ���ʱ״̬)
    const TOrderStateType			ORDER_STATE_MODIFYING			= '5';
    //���ֳ���
    const TOrderStateType			ORDER_STATE_PARTDELETED			= '6';
    //��ȫ����
    const TOrderStateType			ORDER_STATE_DELETED				= '7';
    //���ֳɽ�
    const TOrderStateType			ORDER_STATE_PARTFINISHED		= '8';
    //��ȫ�ɽ�
    const TOrderStateType			ORDER_STATE_FINISHED			= '9';

    ///////////////////////////////////////////////////////////////////////////
    //�ɽ���ʽ����
    typedef char					TMatchModeType;
    //����
    const TMatchModeType			MATCH_MODE_NORMAL				= 'N';
    //����ί��
    const TMatchModeType			MATCH_MODE_UPDATE				= 'U';
    //����
    const TMatchModeType			MATCH_MODE_OTHER				= 'O';

    ///////////////////////////////////////////////////////////////////////////
    //������������
    typedef unsigned int			TStreamIdType;

    ///////////////////////////////////////////////////////////////////////////
    //�����ʺ���������
    typedef char					TTradeNoTypeType;
    //��������
    const TTradeNoTypeType			TRADENO_NORMAL					= 'N';
    //Ͷ��
    const TTradeNoTypeType			TRADENO_TOUJI					= 'T';
    //��ֵ
    const TTradeNoTypeType			TRADENO_BAOZHI					= 'B';
    //����
    const TTradeNoTypeType			TRADENO_SPREAD					= 'S';

    ///////////////////////////////////////////////////////////////////////////
    //�����ʺ�����
    typedef char					TTradeNoType[21];

    ///////////////////////////////////////////////////////////////////////////
    //�ʽ�����
    typedef double					TMoneyValueType;

    ///////////////////////////////////////////////////////////////////////////
    //�ֲֹؼ�������(��ʷ�ֲ�ȫ��Ψһ,���ճֲ�ÿ�β�ѯΨһ)
    typedef int						THoldKeyIdType;
    //����ؼ�������(���ս�������ȫ��Ψһ)
    typedef int						TDeliveryKeyIdType;

    ///////////////////////////////////////////////////////////////////////////
    //�������ˮ������
    typedef int						TCashSerialIdType;
    //�ʽ������ˮ������
    typedef int						TAdjustSerialIdType;

    ///////////////////////////////////////////////////////////////////////////
    //���������
    typedef char					TCashTypeType;
    //����
    const TCashTypeType				CASH_TYPE_OUT					= 'O';
    //���
    const TCashTypeType				CASH_TYPE_IN					= 'I';

    //�����״̬
    typedef char					TCashStateType;
    //δ���
    const TCashStateType			CASH_STATE_NOT_CHECK			= 'N';
    //�����
    const TCashStateType			CASH_STATE_CHECK				= 'Y';
    //���δͨ��
    const TCashStateType			CASH_STATE_FAIL					= 'F';

    ///////////////////////////////////////////////////////////////////////////
    //�����ʽ
    typedef char					TCashModeType;
    //ת��
    const TCashModeType				CASH_MODE_A						= 'A';
    //֧Ʊ
    const TCashModeType				CASH_MODE_B						= 'B';
    //�ֽ�
    const TCashModeType				CASH_MODE_C						= 'C';
    //����
    const TCashModeType				CASH_MODE_E						= 'E';

    ///////////////////////////////////////////////////////////////////////////
    //�����ע
    typedef char					TCashRemarkType[101];


	///////////////////////////////////////////////////////////////////////////
	//�Ƿ�ǿ�Ƴ�����
	typedef char					TForceCashOutFlagType;
	//ǿ�Ƴ����ʽ���ʱ���������
	const TForceCashOutFlagType	FORCE_CASH_OUT_YES			= 'Y';
	//��ǿ�Ƴ���,�ʽ���ʱ�����������
	const TForceCashOutFlagType	FORCE_CASH_OUT_NO			= 'N';


    //�ʽ����״̬
    typedef char					TAdjustStateType;
    //δ���
    const TAdjustStateType			ADJUST_STATE_NOT_CHECK			= 'N';
    //�����
    const TAdjustStateType			ADJUST_STATE_CHECK				= 'Y';
    //���δͨ��
    const TAdjustStateType			ADJUST_STATE_FAIL				= 'F';

    ///////////////////////////////////////////////////////////////////////////
    //�ʽ������������
    typedef char					TAdjustTypeType;

    ///////////////////////////////////////////////////////////////////////////
    //�ʽ������ע����
    typedef char					TAdjustRemarkType[101];

    ///////////////////////////////////////////////////////////////////////////
    //�ʽ�仯�ֶ�����(�ӵ�λ����λ,ÿλ��Ӧһ���仯�ֶ�)
    typedef unsigned int			TMoneyChgType;
	//�ʽ����                                                                 
	const TMoneyChgType				MONEY_CHG_ADJUST				= 0x00000001;
	//���                                                                     
	const TMoneyChgType				MONEY_CHG_CASHIN				= 0x00000002;
	//����                                                                     
	const TMoneyChgType				MONEY_CHG_CASHOUT   			= 0x00000004;
	//������                                                                   
	const TMoneyChgType				MONEY_CHG_FEE					= 0x00000008;
	//�����ʽ�                                                                 
	const TMoneyChgType				MONEY_CHG_FROZEN				= 0x00000010;
	//���ƽӯ                                                                 
	const TMoneyChgType				MONEY_CHG_COVERPROFIT			= 0x00000020;
	//����ƽӯ                                                                 
	const TMoneyChgType				MONEY_CHG_DAYCVERPROFIT			= 0x00000040;
	//��ʸ�ӯ                                                                 
	const TMoneyChgType				MONEY_CHG_FLOATPROFIT			= 0x00000080;
	//���и�ӯ                                                                 
	const TMoneyChgType				MONEY_CHG_DAYFLOATPROFIT		= 0x00000100;
	//δ��ƽӯ                                                                 
	const TMoneyChgType				MONEY_CHG_UNEXPIREDPROFIT		= 0x00000200;
	//Ȩ����                                                                   
	const TMoneyChgType				MONEY_CHG_PREMIUM				= 0x00000400;
	//��֤��                                                                   
	const TMoneyChgType				MONEY_CHG_DEPOSIT				= 0x00000800;
	//ά�ֱ�֤��                                                               
	const TMoneyChgType				MONEY_CHG_KEEPDEPOSIT			= 0x00001000;
	//��Ѻ�ʽ�                                                                 
	const TMoneyChgType				MONEY_CHG_PLEDGE				= 0x00002000;
	//�����ʽ�                                                                 
	const TMoneyChgType				MONEY_CHG_TAVAILABLE			= 0x00004000;
    //���ֽ��
    const TMoneyChgType				MONEY_CHG_Discount              = 0x00008000;
    //����������
    const TMoneyChgType				MONEY_CHG_TradeFee              = 0x00010000;
    //����������
    const TMoneyChgType				MONEY_CHG_DeliveryFee           = 0x00020000;
    //���������
    const TMoneyChgType				MONEY_CHG_ExchangeFee           = 0x00040000;
    //���ᱣ֤��
    const TMoneyChgType				MONEY_CHG_FrozenDeposit         = 0x00080000;
    //����������
    const TMoneyChgType				MONEY_CHG_FrozenFee             = 0x00100000;
    //��ӯ(��LME)
    const TMoneyChgType				MONEY_CHG_NewFloatProfit        = 0x00200000;
    //LME��ӯ
    const TMoneyChgType				MONEY_CHG_LmeFloatProfit        = 0x00400000;
    //��Ȩ��ֵ
    const TMoneyChgType				MONEY_CHG_OptionMarketValue     = 0x00800000;
    //����ԭʼ�����
    const TMoneyChgType				MONEY_CHG_OriCash               = 0x01000000;
    //���ʽ�        
    const TMoneyChgType				MONEY_CHG_TMoney                = 0x02000000;
    //��Ȩ��
    const TMoneyChgType				MONEY_CHG_TBalance              = 0x04000000;
    //�����
    const TMoneyChgType				MONEY_CHG_TCanCashOut           = 0x08000000;
    //������
    const TMoneyChgType				MONEY_CHG_RiskRate              = 0x10000000;
    //�˻���ֵ
    const TMoneyChgType				MONEY_CHG_AccountMarketValue    = 0x20000000;

    ///////////////////////////////////////////////////////////////////////////
    //��Ϣ����������
    typedef char					TMessageReceiverType;
    //ָ���ͻ�
    const TMessageReceiverType		MESSAGE_RECEIVER_CLIENT			= 'C';
    //ָ���ͻ���
    const TMessageReceiverType		MESSAGE_RECEIVER_GROUP			= 'G';
    //���пͻ�
    const TMessageReceiverType		MESSAGE_RECEIVER_ALL			= 'A';

    ///////////////////////////////////////////////////////////////////////////
    //Mac��ַ����
    typedef char					TMacAddressType[51];

    ///////////////////////////////////////////////////////////////////////////
    //��֤����㷽ʽ
    typedef char					TDepositModeType;
    //����
    const TDepositModeType			DEPOSIT_MODE_B					= '1';
    //����
    const TDepositModeType			DEPOSIT_MODE_D					= '2';
    //��ֵ����
    const TDepositModeType			DEPOSIT_MODE_CB					= '3';
    //��ֵ����
    const TDepositModeType			DEPOSIT_MODE_CD					= '4';
    //�ۿ�
    const TDepositModeType			DEPOSIT_MODE_Z					= '5';

    ///////////////////////////////////////////////////////////////////////////
    //��־��������
    typedef char					TLogTypeType;
    //��¼��־
    const TLogTypeType				LOGTYPE_LOGIN					= '1';

    ///////////////////////////////////////////////////////////////////////////
    //��־��������
    typedef char					TLogContentType[501];
    //��־��ˮ������
    typedef int						TSerialIdType;

    ///////////////////////////////////////////////////////////////////////////
    //ί�гɽ�ɾ�����
    typedef char					TDeletedType;
    //��
    const TDeletedType				DEL_YES							= 'Y';
    //��
    const TDeletedType				DEL_NO							= 'N';
    //����
    const TDeletedType				DEL_DISAPPEAR					= 'D';

    ///////////////////////////////////////////////////////////////////////////
    //�Ƿ�����ϼ�ֵ
    typedef char					TContainTotleType;
    //����
    const TContainTotleType			ContainTotle_Yes			= 'Y';
    //������
    const TContainTotleType			ContainTotle_No				= 'N';

    ///////////////////////////////////////////////////////////////////////////
    //���б�ʶ
    typedef char					TBankType[3];

    ///////////////////////////////////////////////////////////////////////////
    //��������
    typedef char					TBankNameType[21];

    ///////////////////////////////////////////////////////////////////////////
    //�����˺�
    typedef char					TAccountType[51];

    ///////////////////////////////////////////////////////////////////////////
    //����ұ�ʶ
    typedef char					TLWFlagType;
    //����������˻�
    const TLWFlagType				LWFlag_L						= 'L';
    //�ͻ���������˻�
    const TLWFlagType				LWFlag_W						= 'W';
    //��˾��������˻�
    const TLWFlagType				LJFFlag_J						= 'J';
    //��˾��������˻�
    const TLWFlagType				LJFFlag_F						= 'F';

	///////////////////////////////////////////////////////////////////////////
	//����¼��������
	typedef int						TEventSerialIdType;

	///////////////////////////////////////////////////////////////////////////
	//����¼���������
	typedef char					TEventTypeType;
	//�����¼�
	const TEventTypeType			EVENT_TYPE_Y					= 'Y';
	//�ǳ����¼�
	const TEventTypeType			EVENT_TYPE_N					= 'N';

	///////////////////////////////////////////////////////////////////////////
	//����¼��ȼ�����
	typedef char					TEventLevelType;
	//����
	const TEventLevelType			EVENT_TYPE_NORMAL				= 'N';
	//����
	const TEventLevelType			EVENT_TYPE_WARNNING				= 'W';
	//����
	const TEventLevelType			EVENT_TYPE_ERROR				= 'E';

	///////////////////////////////////////////////////////////////////////////
	//�¼���Դ����
	typedef char					TEventSourceType[21];

	///////////////////////////////////////////////////////////////////////////
	//�¼���������
	typedef char					TEventContentType[201];
}

#endif
