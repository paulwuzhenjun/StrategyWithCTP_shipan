#ifndef _ESUNNY_FOREIGN_TRADE_API_STRUCT_H
#define _ESUNNY_FOREIGN_TRADE_API_STRUCT_H

#include "EsForeignApiType.h"

#pragma pack(push, 1)

namespace ESForeign
{
    //����IP���ӵĵ�ַ��Ϣ
    struct TEsAddressField
    {
        TIpType					Ip;             //IP��ַ
        TPortType				Port;           //�˿ں�
    };

    //��¼����ṹ
    struct TEsLoginReqField
    {    
        TIsCaLoginType				IsCaLogin;              //�Ƿ�CA��֤
        TEsLoginIdentityType		Identity;               //��¼�������,Ŀǰֻ֧�ֵ��ͻ�
        TIsForcePasswordType		IsForcePwd;             //�Ƿ�ǿ���޸�����
        /// ʹ�ò�ͬ�˻���¼ʱ���õ�¼��
        union														
        {
            TClientNoType				ClientNo;			//�ͻ���,����ͻ���
            TOperatorNoType				OperatorNo;			//����Ա��,�������Ա��
        };
        /// ��Ӧ��ʹ�õĵ�¼�˺ŵ�����
        TLoginPasswordType			LoginPwd;				//��¼����
        /// ǿ���޸�����ʱ��������
        TLoginPasswordType			NewPwd;					//ǿ���޸������¼ʱ,���޸�����
        TOtpPassType				OtpPass;                //otp��֤����
        TEsSizeType                 CaLen;                  //CA��Ϣ���ȣ�IsCaLoginΪ'Y'ʱ�����ֶ���Ч
        TCaInfoType					CaInfo;					//CA��¼ʱ��д,IsCaLoginΪ'Y'ʱ�����ֶ���Ч
    };
    //��¼Ӧ��ṹ
    struct TEsLoginRspField
    {
        TIsCaLoginType				IsCaLogin;              //�Ƿ�CA��֤
        TIsForcePasswordType		IsForcePwd;             //�Ƿ�ǿ���޸�����
        TLoginNoType				LoginNo;				//��¼��            
        TLoginNameType				LoginName;				//��¼���ʺż��
        TReservedInfoType			ReservedInfo;			//�ͻ�Ԥ����Ϣ,�ͻ��Լ�ʶ���̨ϵͳ
        TDateTimeType				LastLoginDateTime;		//�ϴε�¼ʱ��
        TDateTimeType				LastLogoutDateTime;		//�ϴεǳ�ʱ��
        TIpType						LastLoginIp;			//�ϴε�¼ip
        TPortType					LastLoginPort;			//�ϴε�¼port
        TMachineInfoType			LastLoginMachineInfo;	//�ϴε�¼������Ϣ
        TDateTimeType				ServerDateTime;			//ϵͳ��ǰʱ��(�ͻ���Уʱ)
    };

    //��������ṹ
    struct TEsOrderInsertReqField
    {
        TClientNoType				ClientNo;
        TCommodityNoType			CommodityNo;            //��Ʒ����
        TContractNoType				ContractNo;             //��Լ����
        TOrderTypeType				OrderType;				//ί������
        TOrderWayType				OrderWay;				//ί�з�ʽ,���ͻ����ù�ע���ֶ�
        TOrderModeType				OrderMode;				//ί��ģʽ
        TDateTimeType				ValidDateTime;			//��Ч����(GTD�����ʹ��)
        TIsRiskOrderType			IsRiskOrder;			//���ձ���
        TDirectType					Direct;					//��������
        TOffsetType					Offset;					//����ƽ��
        THedgeType					Hedge;					//Ͷ����ֵ
        TTradePriceType				OrderPrice;				//ί�м۸�
        TTradePriceType				TriggerPrice;			//�����۸�
        TTradeVolType				OrderVol;				//ί������
        TTradeVolType				MinMatchVol;			//��С�ɽ���

        TOrderSaveIntType			SaveInt;                //�ͻ������ֶ�1
        TOrderSaveDoubleType		SaveDouble;             //�ͻ������ֶ�2
        TOrderSaveStringType		SaveString;             //�ͻ������ֶ�3
    };

    //����Ӧ��ṹ
    struct TEsOrderInsertRspField
    {
        TStreamIdType				OrderStreamId;					//ί������
        TOrderIdType				OrderId;						//ί�к�
        TFeLocalNoType				LocalNo;						//���غ�
        TTradeNoType				TradeNo;						//�ͻ������ʺ�

        TOperatorNoType				InsertNo;						//�µ���
        TDateTimeType				InsertDateTime;					//�µ�ʱ��
        TOrderStateType				OrderState;						//ί��״̬

        TEsOrderInsertReqField      ReqData;                        //������������
    };

    //ί��״̬���֪ͨ
    struct TEsOrderStateNoticeField
    {	
        TOrderTypeType				OrderType;						//ί������
        TOffsetType					Offset;							//��ƽ����
        THedgeType					Hedge;							//Ͷ����ֵ����

        TTradePriceType				OrderPrice;				        //ί�м�
        TTradePriceType				TriggerPrice;                   //������
        TTradeVolType				OrderVol;						//ί������

        TStreamIdType				OrderStreamId;                  //ί������,ÿ��״̬֪ͨΨһ
        TOrderIdType				OrderId;                        //ί��ID
        TFeLocalNoType				LocalNo;                        //���غ�
        TSystemNoType				SystemNo;                       //ϵͳ��

        TOperatorNoType				UpdateNo;		                //����Ա���
        TDateTimeType				UpdateDateTime;					//����ʱ��
        TOrderStateType				OrderState;                     //ί��״̬
        TTradePriceType				MatchPrice;                     //�ɽ�����
        TTradeVolType				MatchVol;                       //�ɽ�����
        TErrorCodeType				ErrorCode;                      //��������ʱ��Ӧ������

        TFeLocalNoType				ActionLocalNo;                  //���������ı��غ�

        TEsOrderInsertReqField      ReqData;                        //������������
		TSystemNoType				ExchangeSystemNo;				//������ϵͳ��
    };

    //ί����Ϣ���֪ͨ
    struct TEsOrderInfoNoticeField
    {
        TEsOrderInsertReqField	ReqData;                            //��������������

        TStreamIdType				OrderStreamId;                  //ί������
        TOrderIdType				OrderId;                        //ί��ID
        TFeLocalNoType				LocalNo;                        //���غ�
        TSystemNoType				SystemNo;                       //ϵͳ��
        TTradeNoType				TradeNo;                        //�����˺�

        TOperatorNoType				InsertNo;                       //¼�����Ա��
        TDateTimeType				InsertDateTime;                 //¼��ʱ��
        TOperatorNoType				UpdateNo;						//���һ�α����
        TDateTimeType				UpdateDateTime;					//���һ�α��ʱ��
        TOrderStateType				OrderState;                     //ί��״̬
        TTradePriceType				MatchPrice;                     //�ɽ��۸�
        TTradeVolType				MatchVol;                       //�ɽ�����
        TErrorCodeType				ErrorCode;						//���һ�β���������Ϣ��

        TFeLocalNoType				ActionLocalNo;                  //���������ı��غ�
        TOrderInputType				OrderInput;						//�Ƿ�¼��
        TDeletedType				Deleted;                        //�Ƿ�ɾ��
        TAddOneType					AddOne;							//T+1��־
		TSystemNoType				ExchangeSystemNo;				//������ϵͳ��
    };

    //��������ṹ
    struct TEsOrderDeleteReqField
    {
        TOrderIdType				OrderId;                        //ί��ID
    };

    //����Ӧ��ṹ
    //typedef TEsOrderStateNoticeField TEsOrderDeleteRspField;
    struct  TEsOrderDeleteRspField
    {
        TEsOrderStateNoticeField    OrderStateField;

        TEsOrderInsertReqField      ReqData;                        //������������
    };

    //�ĵ�����ṹ
    struct TEsOrderModifyReqField
    {
        TOrderIdType				OrderId;                        //ί��ID
        TTradePriceType				OrderPrice;				        //ί�м۸�
        TTradePriceType				TriggerPrice;                   //�����۸�
        TTradeVolType				OrderVol;	                    //ί������
    };

    //�ĵ�Ӧ��ṹ
    //typedef TEsOrderStateNoticeField TEsOrderModifyRspField;
    struct TEsOrderModifyRspField
    {
        TEsOrderStateNoticeField    OrderStateField;

        TEsOrderInsertReqField      ReqData;                        //������������
    };

    //�ɽ�״̬֪ͨ
    struct TEsMatchStateNoticeField
    {
        TStreamIdType				MatchStreamId;              //�ɽ�����
        TClientNoType				ClientNo;                   //�ͻ���
        TSystemNoType				SystemNo;                   //ϵͳ��
        TMatchNoType				MatchNo;                    //�ɽ���
        TMatchModeType				MatchMode;                  //�ɽ�ģʽ
        TMatchWayType				MatchWay;                   //�ɽ���ʽʽ
        TTradePriceType				MatchPrice;                 //�ɽ���
        TTradeVolType				MatchVol;                   //�ɽ�����
        TDateTimeType				MatchDateTime;              //�ɽ�ʱ��
        TMoneyValueType				MatchFee;                   //�ɽ�����
        TCurrencyNoType				CurrencyNo;					//�����ѱ���
        TAddOneType					AddOne;						//T+1���
        TManualFeeType				ManualFee;					//�ֹ�������
        TDeletedType				Deleted;                    //ɾ����־
        TOrderIdType				OrderId;                    //ί��ID
        TOrderIdType				MatchId;                    //�ɽ�ID
        TTradePriceType				CoverPrice;                 //ƽ�ּ۸�
    };

    //�ɽ���Ϣ֪ͨ
    struct TEsMatchInfoNoticeField
    {
        TEsMatchStateNoticeField    StateData;                  //�ɽ�״̬
        TCommodityNoType			CommodityNo;				//��Ʒ��������Ҫ
        TContractNoType				ContractNo;					//����������Ҫ
        TDirectType					Direct;						//����������Ҫ
        TOffsetType					Offset;						//����������Ҫ
    };

    struct TEsOrderQryReqField
    {
        TStreamIdType				OrderStreamId;					//��ѯ�������ش��ڴ����ŵ�ί������
        TClientNoType				ClientNo;                       //�ͻ���
        TExchangeNoType				ExchangeNo;                     //������
        TCommodityNoType			CommodityNo;                    //��Ʒ
        TContractNoType				ContractNo;                     //��Լ
        TOrderTypeType				OrderType;						//ί������
        TOrderModeType				OrderMode;						//ί��ģʽ
        TIsRiskOrderType			IsRiskOrder;					//���ձ���
        THedgeType					Hedge;							//Ͷ����ֵ
        TOrderIdType				OrderId;                        //ί��ID
        TFeLocalNoType				LocalNo;                        //���غ�
        TSystemNoType				SystemNo;                       //ϵͳ��
        TOperatorNoType				OperNo;							//�µ��˻�����˺�
        TDateTimeType				BeginInsertDateTime;            //��ʼʱ��
        TDateTimeType				EndInsertDateTime;              //����ʱ��
        TOrderStateType				OrderState;                     //ί��״̬
    };
    //ί������Ӧ������
    typedef TEsOrderInfoNoticeField TEsOrderDataQryRspField;

    //�ɽ���ѯ��������
    struct TMatchQryReqField
    {
        TStreamIdType				MatchStreamId;              //�ɽ�����
        TClientNoType				ClientNo;                   //�ͻ���
        TSystemNoType				SystemNo;                   //ϵͳ��
        TMatchNoType				MatchNo;                    //�ɽ���
        TDateTimeType				BeginMatchDateTime;         //��ʼʱ��
        TDateTimeType				EndMatchDateTime;           //����ʱ��
        TExchangeNoType				ExchangeNo;                 //������
        TCommodityNoType			CommodityNo;				//��Ʒ��������Ҫ
        TContractNoType				ContractNo;					//����������Ҫ
    };

    //�ɽ�����Ӧ������
    typedef TEsMatchInfoNoticeField TEsMatchDataQryRspField;

    //�ʽ��ѯ����ṹ
    struct TEsMoneyQryReqField
    {
        TClientNoType				ClientNo;                   //�ͻ���
    };

    //�ʽ��ѯӦ��ṹ(ע��νṹ���ʽ�仯֪ͨ�ṹ�Ĺ�ϵ)
    struct TEsMoneyQryRspField
    {
        TClientNoType				ClientNo;                   //�ͻ���
        TCurrencyNoType				CurrencyNo;                 //���ұ��
        TMoneyValueType				YAvailable;					//�����
        TMoneyValueType				YCanCashOut;				//�����
        TMoneyValueType				YMoney;						//������
        TMoneyValueType				YBalance;					//��Ȩ��
        TMoneyValueType				YUnExpiredProfit;			//��δ��ƽӯ
        TMoneyValueType				Adjust;						//�ʽ����0
        TMoneyValueType				CashIn;						//���1
        TMoneyValueType				CashOut;					//����2
        TMoneyValueType				Fee;						//������3
        TMoneyValueType				Frozen;						//�����ʽ�4
        TMoneyValueType				CoverProfit;				//���ƽӯ5
        TMoneyValueType				DayCoverProfit;				//����ƽӯ6
        TMoneyValueType				FloatProfit;				//��ʸ�ӯ7
        TMoneyValueType				DayFloatProfit;				//���и�ӯ8
        TMoneyValueType				UnExpiredProfit;			//δ��ƽӯ9
        TMoneyValueType				Premium;					//Ȩ����10
        TMoneyValueType				Deposit;					//��֤��11
        TMoneyValueType				KeepDeposit;				//ά�ֱ�֤��12
        TMoneyValueType				Pledge;						//��Ѻ�ʽ�13
        TMoneyValueType				TAvailable;					//�����ʽ�14
        TMoneyValueType				Discount;					//���ֽ��15
        TMoneyValueType				TradeFee;					//����������16
        TMoneyValueType				DeliveryFee;				//����������17
        TMoneyValueType				ExchangeFee;				//���������18
        TMoneyValueType				FrozenDeposit;				//���ᱣ֤��19
        TMoneyValueType				FrozenFee;					//����������20
        TMoneyValueType				NewFloatProfit;				//��ӯ(��LME)21
        TMoneyValueType				LmeFloatProfit;				//LME��ӯ22
        TMoneyValueType				OptionMarketValue;			//��Ȩ��ֵ23
        TMoneyValueType				OriCash;					//����ԭʼ�����24(���Զ�����ʽ�)
        TMoneyValueType             TMoney;                     //���ʽ�
        TMoneyValueType		        TBalance;					//��Ȩ��
        TMoneyValueType		        TCanCashOut;				//�����
        TMoneyValueType		        RiskRate;					//������
        TMoneyValueType		        AccountMarketValue;			//�˻���ֵ
    };
    //�ʽ�仯֪ͨ�ṹ
	struct TMoneyChgItem
	{
		TMoneyChgType				MoneyChg;
		TMoneyValueType				MoneyValue;
	};

    struct TEsMoneyChgNoticeField
    {
        TClientNoType				ClientNo;                   //�ͻ���
        TCurrencyNoType				CurrencyNo;                 //���ұ��
        u_short						MoneyChgNum;                //�ʽ�仯��ĸ���
        TMoneyChgItem				MoneyItem[1];				//�ʽ�仯����
    };

    //�ֲֲ�ѯ����ṹ
    struct TEsHoldQryReqField
    {
        TClientNoType				ClientNo;                   //�ͻ���
        TExchangeNoType				ExchangeNo;                 //������
        TCommodityNoType			CommodityNo;                //��Ʒ
        TContractNoType				ContractNo;                 //��Լ
    };
    //�ֲֲ�ѯӦ��ṹ
    struct TEsHoldQryRspField
    {
        THoldKeyIdType				HoldKeyId;                  //�ֲֹؼ���
        TClientNoType				ClientNo;                   //�ͻ���
        TCommodityNoType			CommodityNo;                //��Ʒ
        TContractNoType				ContractNo;                 //��Լ
        TDirectType					Direct;                     //��������
        THedgeType					Hedge;                      //Ͷ����ֵ
        TTradePriceType				TradePrice;                 //�ֲ־���
        TTradeVolType				TradeVol;                   //�ֲ���
        TQuotePriceType				YSettlePrice;               //������
        TQuotePriceType				TNewPrice;                  //���¼�
        TDateTimeType				MatchDateTime;              //�ɽ�ʱ��
        TMatchNoType				MatchNo;                    //�ɽ���
        TMoneyValueType				Deposit;                    //��֤��
        TMoneyValueType				KeepDeposit;                //ά�ֱ�֤��
    };

    //��Ʒ��ѯ����ṹ
    struct TEsCommodityQryReqField
    {
    };
    //��Ʒ��ѯӦ��ṹ
    struct TEsCommodityQryRspField
    {
        TExchangeNoType				ExchangeNo;				//������
        TCommodityTypeType			CommodityType;			//��Ʒ����
        TCommodityNoType			CommodityNo;			//��Ʒ
        TCommodityNoType			RelateCommodityNo;		//������Լ
        TCommodityNameType			CommodityName;          //��Ʒ����
        TCommodityAttributeType		CommodityAttribute;     //��Ʒ����
        TCommodityStateType			CommodityState;         //��Ʒ״̬
        TProductDotType				ProductDot;             //ÿ�ֳ���
        TUpperTickType				UpperTick;              //��С�䶯�۷���
        TLowerTickType				LowerTick;              //��С�䶯�۷�ĸ
        TCurrencyNoType				CurrencyNo;				//��Ʒʹ�ñ���
        TDeliveryModeType			DeliveryMode;			//���ʽ
        TDeliveryDaysType			DeliveryDays;           //������ƫ��
        TDepositCalculateModeType	DepositCalculateMode;	//��֤����㷽ʽ
        TTradeVolType				MaxSingleOrderVol;		//��������µ���
        TTradeVolType				MaxHoldVol;				//���ֲ���
        TTimeType					AddOneTime;				//T+1ʱ��,���ڴ�ʱ��ΪT+1����
        TDirectType					CmbDirect;				//�����������(��һ��)
        TCoverModeType				CoverMode;              //ƽ�ַ�ʽ
    };

    //��Ʒ״̬�仯֪ͨ�ṹ
    struct TEsCommodityStateModNoticeField
    {
        TCommodityNoType			CommodityNo;					//�ϱ������ֶ�Ϊ��Ʒ�ؼ���
        TCommodityStateType			CommodityState;
        TTradeVolType				MaxSingleOrderVol;				//��������µ���
        TTradeVolType				MaxHoldVol;						//���ֲ���
    };

    //��Լ��ѯ����ṹ
    struct TEsContractQryReqField
    {
        TCommodityNoType			CommodityNo;            //��Ʒ
        TContractLastDays			LastDays;		        //�ٽ�������

    };
    //��Լ��ѯӦ��ṹ
    struct TEsContractQryRspField
    {
        TCommodityNoType			CommodityNo;			//��Ʒ
        TContractNoType				ContractNo;				//��Լ
        TContractNameType			ContractName;			//��Լ����
        TContractTypeType			ContractType;			//��Լ����
        TContractStateType			ContractState;			//��Լ״̬
        TDateType					ExpiryDate;				//������
        TDateType					LastTradeDate;			//�������
        TDateType					FirstNoticeDate;		//�״�֪ͨ��
    };

    //��Լ����֪ͨ�ṹ
    typedef TEsContractQryRspField	TEsContractAddNoticeField;

    //�ͻ������޸�����ṹ
    struct TEsClientPasswordModifyReqField
    {
        TClientNoType				ClientNo;               //�ͻ���
        TPasswordTypeType			PasswordType;           //��������
        TLoginPasswordType			OldPassword;			//����Ա�޸Ŀͻ�����ʱ,��Ȩ�޲���Ա���Բ���д������					
        TLoginPasswordType			NewPassword;            //������
    };
    //�ͻ������޸�Ӧ��ṹ
    struct TEsClientPasswordModifyRspField
    {
        TClientNoType				ClientNo;               //�ͻ���
        TPasswordTypeType			PasswordType;           //��������
    };

    //����Ա�����޸�����ṹ
    struct TEsOperatorPasswordModifyReqField
    {
        TOperatorNoType				OperatorNo;				//����Ա��
        TLoginPasswordType			OldPassword;		    //������
        TLoginPasswordType			NewPassword;			//������
    };
    //����Ա�����޸�Ӧ��ṹ
    struct TEsOperatorPasswordModifyRspField
    {
        TOperatorNoType				OperatorNo;			    //����Ա��
    };

    //�г���ѯ����ṹ
    struct TEsExchangeQryReqField
    {
    };
    //�г���ѯӦ��ṹ
    struct TEsExchangeQryRspField
    {
        TExchangeNoType				ExchangeNo;		        //������
        TExchangeNameType			ExchangeName;		    //����������
        TExchangeStateType			ExchangeState;		    //������״̬
    };

    //�г�״̬�޸�֪ͨ�ṹ
    struct TEsExchangeStateModifyNoticeField
    {
        TExchangeNoType				ExchangeNo;
        TExchangeStateType			ExchangeState;
    };

    //����Ա�����ͻ���ѯ����ṹ
    struct TEsOperatorClientQryReqField
    {
        TOperatorNoType				OperatorNo;			    //����Ա��
    };
    //����Ա�����ͻ���ѯӦ��ṹ
    struct TEsOperatorClientQryRspField
    {
        TClientNoType				ClientNo;			    //�ͻ���
    };

    //��ʷί�в�ѯ����ṹ
    struct TEsHisOrderQryReqField
    {
        TClientNoType				ClientNo;				//�ͻ���
        TDateType					BeginDate;				//��ʼ���ڣ�����
        TDateType					EndDate;				//�������ڣ�����
    };

    //��ʷί�в�ѯӦ��ṹ
    struct TEsHisOrderQryRspField
    {
        TEsOrderInfoNoticeField     Data;				    //ί������	
        TDateType					Date;                   //ί������
    };

    //��ʷ�ɽ���ѯ����ṹ
    struct TEsHisMatchQryReqField
    {
        TClientNoType				ClientNo;				//�ͻ���
        TDateType					BeginDate;				//��ʼ���ڣ�����
        TDateType					EndDate;				//�������ڣ�����
        TContainTotleType			IsContainTotle;			//�Ƿ�����ϼ�ֵ
    };
    //��ʷ�ɽ���ѯӦ��ṹ
    struct TEsHisMatchQryRspField
    {
        TDateType					Date;				    //��������
        TClientNoType				ClientNo;				//�ͻ���
        TMatchNoType				SettleNo;				//�����óɽ����
        TCommodityNoType			CommodityNo;			//��Ʒ
        TContractNoType				ContractNo;				//��Լ
        TMatchWayType				MatchWay;				//�ɽ���ʽ
        TDirectType					Direct;                 //��������
        TOffsetType					Offset;                 //��ƽ����
        TTradeVolType				MatchVol;               //�ɽ�����
        TTradePriceType				MatchPrice;             //�ɽ��۸�
        TMoneyValueType				Premium;                //Ȩ����
        TCurrencyNoType				CurrencyNo;             //����
        TCurrencyNoType				CommodityCurrencyNo;    //Ʒ�ֱ���
        TIsRiskOrderType			IsRiskOrder;            //�Ƿ���ձ���
        TSystemNoType				SystemNo;               //ϵͳ��
        TMatchNoType				MatchNo;                //�ɽ���
        TStreamIdType				MatchStreamID;          //�ɽ�����
        TDateTimeType				MatchDateTime;          //�ɽ�ʱ��
        TMatchModeType				MatchMode;              //�Ƿ���ձ���
        TOrderTypeType				OrderType;              //ί������
        TTradePriceType				CoverPrice;             //ƽ�ּ۸�
    };

    //ί�б�����̲�ѯ��������
    struct TEsOrderProcessQryReqField
    {
        TOrderIdType				OrderId;                //ί��ID
    };

    //ί�б�����̲�ѯӦ������
    typedef TEsOrderStateNoticeField TEsOrderProcessQryRspField;

    //��ʷί�����̲�ѯ����ṹ
    struct TEsHisOrderProcessQryReqField
    {
        TOrderIdType				OrderId;                //ί��ID
        TDateType					Date;                   //��ѯ����
    };

    //��ʷί�����̲�ѯӦ��ṹ
    struct TEsHisOrderProcessQryRspField
    {
        TEsOrderProcessQryRspField  Data;                   //ί����������
        TDateType			        Date;                   //����
    };

    //��������ѯ����ṹ
    struct TEsCashOperQryReqField
    {
        TClientNoType				ClientNo;               //�ͻ���
    };
    //��������ѯӦ��ṹ
    struct TEsCashOperQryRspField
    {
        TEsCashOperQryReqField      ReqData;                //��ѯ��������
        TCashSerialIdType			SerialId;               //�������ˮ��
        TCashStateType				CashState;              //�����״̬
        TDateTimeType				OperDateTime;           //����ʱ��
        TOperatorNoType				OperatorNo;             //������
        TDateTimeType				CheckDateTime;          //���ʱ��
        TOperatorNoType				CheckOperatorNo;        //�����

        TCashTypeType				CashType;               //���������
        TCashModeType				CashMode;               //�����ʽ
        TCurrencyNoType				CurrencyNo;             //���ұ��
        TMoneyValueType				CashValue;              //�������
        TCashRemarkType				CashRemark;             //��ע
        //20120616����
        TBankType					ClientBank;				//�ͻ����б�ʶ
        TAccountType				ClientAccount;			//�ͻ������˻�
        TLWFlagType					ClientLWFlag;			//�ͻ�������˻���ʶ
        TBankType					CompanyBank;			//��˾���б�ʶ
        TAccountType				CompanyAccount;			//��˾�����˻�
        TLWFlagType					CompanyLWFlag;			//��˾������˻���ʶ
    };

	//��������������ṹ
	struct TEsCashOperReqField
	{
		TClientNoType				ClientNo;				//�ͻ���
		TCashTypeType				CashType;				//���������
		TCashModeType				CashMode;				//�����ʽ
		TCurrencyNoType				CurrencyNo;				//���ұ��
		TMoneyValueType				CashValue;				//�������
		TCashRemarkType				CashRemark;				//��ע
		TBankType					ClientBank;				//�ͻ����б�ʶ
		TAccountType				ClientAccount;			//�ͻ������˻�
		TLWFlagType					ClientLWFlag;			//�ͻ�������˻���ʶ
		TBankType					CompanyBank;			//��˾���б�ʶ
		TAccountType				CompanyAccount;			//��˾�����˻�
		TLWFlagType					CompanyLWFlag;			//��˾������˻���ʶ
	};

	//����������Ӧ��ṹ
	struct TEsCashOperRspField
	{
		TEsCashOperReqField			ReqData;				//����������������
		TCashSerialIdType			SerialId;				//�������ˮ��
		TCashStateType				CashState;				//�����״̬
		TDateTimeType				OperDateTime;			//����ʱ��
		TOperatorNoType				OperatorNo;				//������
		TDateTimeType				CheckDateTime;			//���ʱ��
		TOperatorNoType				CheckOperatorNo;		//�����
	};

	//����������֪ͨ�ṹ
	typedef TEsCashOperRspField TEsCashOperNoticeField;

	//��������������ṹ
	struct TEsCashCheckReqField
	{
		TCashSerialIdType			SerialId;				//�������ˮ��
		TCashStateType				CashState;				//�����״̬
		TForceCashOutFlagType		ForceCashOutFlag;		//�Ƿ�ǿ�Ƴ�����
	};

	//����������Ӧ��ṹ
	typedef TEsCashOperRspField TEsCashCheckRspField;

	//����������֪ͨ�ṹ
	typedef TEsCashOperRspField TEsCashCheckNoticeField;

    //�ʽ������ѯ����ṹ
    struct TEsAdjustQryReqField
    {
        TClientNoType				ClientNo;               //�ͻ���
    };

    //�ʽ������ѯӦ��ṹ
    struct TEsAdjustQryRspField
    {
        TEsAdjustQryReqField        ReqData;                //��ѯ��������
        TAdjustSerialIdType			SerialId;               //�������ˮ��
        TAdjustStateType			AdjustState;            //�ʽ����״̬   
        TDateTimeType				OperDateTime;           //����ʱ��
        TOperatorNoType				OperatorNo;             //������
        TDateTimeType				CheckDateTime;          //���ʱ��
        TOperatorNoType				CheckOperatorNo;        //�����

        TAdjustTypeType				AdjustType;             //�ʽ��������
        TCurrencyNoType				CurrencyNo;             //���ұ��
        TMoneyValueType				AdjustValue;            //�������
        TAdjustRemarkType			AdjustRemark;           //��ע
        //20120616����
        TBankType					ClientBank;				//���б�ʶ
        TAccountType				ClientAccount;			//�����˻�
        TLWFlagType					ClientLWFlag;			//������˻���ʶ
        TBankType					CompanyBank;			//���б�ʶ
        TAccountType				CompanyAccount;			//�����˻�
        TLWFlagType					CompanyLWFlag;			//������˻���ʶ
    };

    //��ʷ������ѯ����ṹ
    struct TEsHisCashOperQryReqField
    {
        TClientNoType				ClientNo;				//�ͻ���
        TDateType					BeginDate;				//��ʼ����
        TDateType					EndDate;				//��ֹ����
    };

    //��ʷ������ѯӦ��ṹ
    struct TEsHisCashOperQryRspField
    {
        TEsCashOperQryRspField		Data;
        TDateType					Date;
    };

    //��ʷ�ʽ������ѯ����ṹ
    struct TEsHisAdjustQryReqField
    {
        TClientNoType				ClientNo;				//�ͻ���
        TDateType					BeginDate;				//��ʼ����
        TDateType					EndDate;				//��ֹ����
    };
    //��ʷ�ʽ������ѯӦ��ṹ
    struct TEsHisAdjustQryRspField
    {
        TEsAdjustQryRspField		Data;
        TDateType					Date;
    };

    //�ͻ���֤������֤����ṹ
    struct TEsClientPasswordAuthReqField
    {
        TClientNoType				ClientNo;
        TLoginPasswordType			Password;
    };
    //�ͻ���֤������֤Ӧ��ṹ
    struct TEsClientPasswordAuthRspField
    {
        TClientNoType				ClientNo;
    };

    //���ֲ�ѯ����ṹ
    struct TEsCurrencyQryReqField
    {
    };
    //���ֲ�ѯӦ��ṹ
    struct TEsCurrencyQryRspField
    {
        TCurrencyNoType				CurrencyNo;
        TCurrencyNameType			CurrencyName;
        TIsPrimaryCurrencyType		IsPrimary;
        TCurrencyGroupFlagType		CurrencyGroup;
        TExchangeRateType			ExchangeRate;
    };

    //ɾ��ί��֪ͨ�ṹ
    struct TEsOrderRemoveNoticeField
    {
        TClientNoType               ClientNo;
        TOrderIdType				OrderId;
    };

    //ɾ���ɽ�֪ͨ�ṹ
    struct TEsMatchRemoveNoticeField
    {
        TClientNoType               ClientNo;
        TSystemNoType				SystemNo;
        TMatchNoType				MatchNo;
        TOrderIdType				MatchId;
    };

    //���ʱ��֪ͨ
    struct TEsExchangeRateModifyNoticeField
    {
        TCurrencyNoType				CurrencyNo;
        TExchangeRateType			ExchangeRate;
    };

	//����¼���ѯ����ṹ
	struct TEsMonitorEventQryReqField
	{
	};

	//����¼���ѯӦ��ṹ
	struct TEsMonitorEventQryRspField
	{
		TEventTypeType				EventType;
		TEventLevelType				EventLevel;
		TEventSourceType			EventSource;
		TEventContentType			EventContent;
		TEventSerialIdType			SerialId;
		TDateTimeType				EventDateTime;	
	};

	//����¼�֪ͨ�ṹ
	typedef TEsMonitorEventQryRspField	TEsMonitorEventNoticeField;
}

#pragma pack(pop)

#endif
