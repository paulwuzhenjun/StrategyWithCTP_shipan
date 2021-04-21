#include "StdAfx.h"
#include "ScreenDisplayThread.h"
#include "MessageList.h"
#include "MyStruct.h"
#include <map>

using namespace std;

extern MessageList ScreenDisplayMsgList;
extern CListCtrl* pPositionDetailsList;
extern CListCtrl* pQryOrdersList;
extern CListCtrl* pStraLocalCLList;
extern map<int, string> OrderIdToStrategyNameForDisplay;
extern map<string, string> MatchNoToStrategyNameForDisplay;
extern CString StrategyIDShowing;
extern HANDLE ScreenDisplaySem;
extern map<int, OrderDetailField> RecoverRspOrderMap;
extern HANDLE RecoverScreenDisplaySem;

ScreenDisplayThread::ScreenDisplayThread(void)
{
	m_bIsRunning = true;
}

ScreenDisplayThread::~ScreenDisplayThread(void)
{
}

void ScreenDisplayThread::ProcessRspMsg() {
	PositionDetailField* pPosi = 0;
	OrderDetailField* pOrder = 0;
	LocalCLForDisplayField* pCLField = 0;
	while (m_bIsRunning) {
		WaitForSingleObject(ScreenDisplaySem, INFINITE);

		if (!ScreenDisplayMsgList.MessageListCore.IsEmpty())
		{
			Message message = ScreenDisplayMsgList.GetHead();

			switch (message.type)
			{
			case ON_RSP_QRY_POSITION:
				pPosi = (PositionDetailField*)malloc(sizeof(PositionDetailField));
				message.GetData(pPosi, 0, sizeof(PositionDetailField));
				OnRspQryPositionDetail(pPosi);
				delete pPosi;
				pPosi = 0;
				break;
			case ON_RSP_QRY_ORDER:
				pOrder = (OrderDetailField*)malloc(sizeof(OrderDetailField));
				message.GetData(pOrder, 0, sizeof(OrderDetailField));
				OnRspQryOrder(pOrder);
				delete pOrder;
				pOrder = 0;
				break;
			case ON_RECOVER_QRY_ORDER:
				pOrder = (OrderDetailField*)malloc(sizeof(OrderDetailField));
				message.GetData(pOrder, 0, sizeof(OrderDetailField));
				OnRecoverRspQryOrder(pOrder);
				delete pOrder;
				pOrder = 0;
				break;
			case ON_RSP_LOCAL_CL:
				pCLField = (LocalCLForDisplayField*)malloc(sizeof(LocalCLForDisplayField));
				message.GetData(pCLField, 0, sizeof(LocalCLForDisplayField));
				OnRspQryLocalCLDetail(pCLField);
				delete pCLField;
				pCLField = 0;
				break;
			}
		}
	}
}

void ScreenDisplayThread::OnRspQryLocalCLDetail(LocalCLForDisplayField* pCLDetail)
{
	USES_CONVERSION;
	CString csxStrategyName("");
	string strStrategyName(pCLDetail->StrategyID);
	csxStrategyName = strStrategyName.c_str();

	if (StrategyIDShowing.CompareNoCase(csxStrategyName) == 0 || StrategyIDShowing.CompareNoCase(_T("None")) == 0) {
		int orderIndex = pStraLocalCLList->GetItemCount();
		CString itemname("");
		itemname.Format(_T("%d"), orderIndex);
		pStraLocalCLList->InsertItem(orderIndex, (LPCTSTR)itemname);

		pStraLocalCLList->SetItemText(orderIndex, 0, (LPCTSTR)csxStrategyName);

		CString instanceName(pCLDetail->InstanceName);
		pStraLocalCLList->SetItemText(orderIndex, 1, (LPCTSTR)instanceName);

		CString instCodeName(pCLDetail->InstCodeName);
		pStraLocalCLList->SetItemText(orderIndex, 2, (LPCTSTR)instCodeName);

		CString csCloseOrderSeqNo;
		csCloseOrderSeqNo.Format(_T("%d"), pCLDetail->CloseOrderSeqNo);
		pStraLocalCLList->SetItemText(orderIndex, 3, (LPCTSTR)csCloseOrderSeqNo);

		CString csID;
		csID.Format(_T("%d"), pCLDetail->OrderID);
		pStraLocalCLList->SetItemText(orderIndex, 4, (LPCTSTR)csID);

		CString offset("");
		if (pCLDetail->OffSet == MORDER_OPEN) {
			offset = _T("��");
		}
		else if (pCLDetail->OffSet == MORDER_CLOSE) {
			offset = _T("ƽ");
		}
		else if (pCLDetail->OffSet == MORDER_STOPLOSSCLOSE) {
			offset = _T("�޼�ֹ��ƽ");
		}

		pStraLocalCLList->SetItemText(orderIndex, 5, (LPCTSTR)offset);

		CString direction("");
		if (pCLDetail->Direction == MORDER_BUY) {
			direction = _T("��");
		}
		else if (pCLDetail->Direction == MORDER_SELL) {
			direction = _T("��");
		}
		pStraLocalCLList->SetItemText(orderIndex, 6, (LPCTSTR)direction);

		CString openDateTime(pCLDetail->OpenTime);
		pStraLocalCLList->SetItemText(orderIndex, 7, (LPCTSTR)openDateTime);

		CString vol;
		vol.Format(_T("%d"), pCLDetail->VolumeTotal);
		pStraLocalCLList->SetItemText(orderIndex, 8, (LPCTSTR)vol);

		CString openprice;
		openprice.Format(_T("%.4f"), pCLDetail->LimitPrice);
		pStraLocalCLList->SetItemText(orderIndex, 9, (LPCTSTR)openprice);

		CString openorderprice;
		openorderprice.Format(_T("%.4f"), pCLDetail->OpenOrderTradePrice);
		pStraLocalCLList->SetItemText(orderIndex, 10, (LPCTSTR)openorderprice);

		CString manualstopprice;
		manualstopprice.Format(_T("%.4f"), pCLDetail->ManualStopPrice);
		pStraLocalCLList->SetItemText(orderIndex, 11, (LPCTSTR)manualstopprice);

		CString mordertype;
		mordertype.Format(_T("%d"), pCLDetail->MOrderType);
		pStraLocalCLList->SetItemText(orderIndex, 12, (LPCTSTR)mordertype);

		pStraLocalCLList->EnsureVisible(pStraLocalCLList->GetItemCount() - 1, FALSE);
	}
}

void ScreenDisplayThread::OnRspQryPositionDetail(PositionDetailField* pPosi)
{
	USES_CONVERSION;
	CString csxStrategyName("");
	CString csStrategyNameTmp("");
	string strmatchno(pPosi->MatchNo);
	map<string, string>::iterator iter;
	iter = MatchNoToStrategyNameForDisplay.find(strmatchno);
	if (iter != MatchNoToStrategyNameForDisplay.end()) {
		string str_Strategyname = iter->second;
		csStrategyNameTmp = str_Strategyname.c_str();
		//�����»��߷ָ�,������_ʵ����
		csxStrategyName = csStrategyNameTmp.Left(csStrategyNameTmp.Find(_T("_")));
	}
	else {
		csStrategyNameTmp = _T("Manual");
		csxStrategyName = _T("Manual");
	}

	if (StrategyIDShowing.CompareNoCase(csxStrategyName) == 0 || StrategyIDShowing.CompareNoCase(_T("None")) == 0) {
		int orderIndex = pPositionDetailsList->GetItemCount();
		CString itemname("");
		itemname.Format(_T("%d"), orderIndex);
		pPositionDetailsList->InsertItem(orderIndex, (LPCTSTR)itemname);

		CString csCommodityNo(pPosi->CommodityNo);
		pPositionDetailsList->SetItemText(orderIndex, 0, (LPCTSTR)csCommodityNo);

		CString InstrumentID(pPosi->InstrumentID);
		pPositionDetailsList->SetItemText(orderIndex, 1, (LPCTSTR)InstrumentID);

		CString direction("");
		if (pPosi->Direction == MORDER_BUY) {
			direction = _T("��");
		}
		else if (pPosi->Direction == MORDER_SELL) {
			direction = _T("��");
		}
		pPositionDetailsList->SetItemText(orderIndex, 2, (LPCTSTR)direction);

		CString openDateTime(pPosi->TradeDateTime);
		pPositionDetailsList->SetItemText(orderIndex, 3, (LPCTSTR)openDateTime);

		CString vol;
		vol.Format(_T("%d"), pPosi->TradeVol);
		pPositionDetailsList->SetItemText(orderIndex, 4, (LPCTSTR)vol);
		CString openprice;
		openprice.Format(_T("%.5f"), pPosi->TradePrice);
		pPositionDetailsList->SetItemText(orderIndex, 5, (LPCTSTR)openprice);

		pPositionDetailsList->SetItemText(orderIndex, 6, (LPCTSTR)csStrategyNameTmp);

		pPositionDetailsList->EnsureVisible(pPositionDetailsList->GetItemCount() - 1, FALSE);
	}
}

void ScreenDisplayThread::OnRecoverRspQryOrder(OrderDetailField* pOrder)
{
	RecoverRspOrderMap.insert(std::pair<int, OrderDetailField>(pOrder->OrderId, *pOrder));
}

void ScreenDisplayThread::OnRspQryOrder(OrderDetailField* pOrder)
{
	USES_CONVERSION;
	CString csxStrategyName("");
	CString csStrategyNameTmp("");
	map<int, string>::iterator iter;
	iter = OrderIdToStrategyNameForDisplay.find(pOrder->OrderId);
	if (iter != OrderIdToStrategyNameForDisplay.end()) {
		string str_Strategyname = iter->second;
		csStrategyNameTmp = str_Strategyname.c_str();
		//�����»��߷ָ�,������_ʵ����
		csxStrategyName = csStrategyNameTmp.Left(csStrategyNameTmp.Find(_T("_")));
	}
	else {
		csxStrategyName = _T("Manual");
	}

	if (StrategyIDShowing.CompareNoCase(csxStrategyName) == 0 || StrategyIDShowing.CompareNoCase(_T("None")) == 0) {
		int orderIndex = pQryOrdersList->GetItemCount();
		CString itemname("");
		itemname.Format(_T("%d"), orderIndex);
		pQryOrdersList->InsertItem(orderIndex, (LPCTSTR)itemname);

		CString orderId("");
		orderId.Format(_T("%d"), pOrder->OrderId);
		pQryOrdersList->SetItemText(orderIndex, 0, (LPCTSTR)orderId);

		CString csCommodityNo(pOrder->CommodityNo);
		pQryOrdersList->SetItemText(orderIndex, 1, (LPCTSTR)csCommodityNo);

		CString InstrumentID(pOrder->InstrumentID);
		pQryOrdersList->SetItemText(orderIndex, 2, (LPCTSTR)InstrumentID);

		CString insertTime(pOrder->InsertDateTime);
		pQryOrdersList->SetItemText(orderIndex, 3, (LPCTSTR)insertTime);

		CString vol("");
		vol.Format(_T("%d"), pOrder->VolumeTotalOriginal);
		pQryOrdersList->SetItemText(orderIndex, 4, (LPCTSTR)vol);

		CString direction("");
		if (pOrder->Direction == MORDER_BUY) {
			direction = _T("��");
		}
		else if (pOrder->Direction == MORDER_SELL) {
			direction = _T("��");
		}
		pQryOrdersList->SetItemText(orderIndex, 5, (LPCTSTR)direction);

		CString price;
		price.Format(_T("%.5f"), pOrder->SubmitPrice);
		pQryOrdersList->SetItemText(orderIndex, 6, (LPCTSTR)price);

		CString csOrderStatus("");
		if (pOrder->OrderStatus == MORDER_FAIL) {
			csOrderStatus = _T("ָ��ʧ��");
		}
		else if (pOrder->OrderStatus == MORDER_ACCEPTED) {
			csOrderStatus = _T("������");
		}
		else if (pOrder->OrderStatus == MORDER_QUEUED) {
			csOrderStatus = _T("���Ŷ�");
		}
		else if (pOrder->OrderStatus == MORDER_PART_TRADED) {
			csOrderStatus = _T("���ֳɽ�");
		}
		else if (pOrder->OrderStatus == MORDER_FULL_TRADED) {
			csOrderStatus = _T("��ȫ�ɽ�");
		}
		else if (pOrder->OrderStatus == MORDER_PART_CANCELLED) {
			csOrderStatus = _T("���ֳ���");
		}
		else if (pOrder->OrderStatus == MORDER_AMBUSH) {
			csOrderStatus = _T("Ԥ��");
		}
		else if (pOrder->OrderStatus == MORDER_CANCELLED) {
			csOrderStatus = _T("��ȫ����");
		}
		else {
			csOrderStatus = _T("����");
		}
		pQryOrdersList->SetItemText(orderIndex, 7, (LPCTSTR)csOrderStatus);

		CString tradeprice("");
		tradeprice.Format(_T("%.5f"), pOrder->TradePrice);
		pQryOrdersList->SetItemText(orderIndex, 8, (LPCTSTR)tradeprice);

		CString tradevol("");
		tradevol.Format(_T("%d"), pOrder->VolumeTraded);
		pQryOrdersList->SetItemText(orderIndex, 9, (LPCTSTR)tradevol);

		CString csStrategyName("");
		map<int, string>::iterator iter;
		iter = OrderIdToStrategyNameForDisplay.find(pOrder->OrderId);
		if (iter != OrderIdToStrategyNameForDisplay.end()) {
			string str_Strategyname = iter->second;
			csStrategyName = str_Strategyname.c_str();
		}
		else {
			if (pOrder->OrderStatus == MORDER_AMBUSH) {
				csStrategyName = _T("Ambush");
			}
			else
				csStrategyName = _T("Manual");
		}
		pQryOrdersList->SetItemText(orderIndex, 10, (LPCTSTR)csStrategyName);

		CString csOrderLocalRef("");
		csOrderLocalRef.Format(_T("%d"), pOrder->OrderLocalRef);
		pQryOrdersList->SetItemText(orderIndex, 11, (LPCTSTR)csOrderLocalRef);

		CString csFrontID("");
		csFrontID.Format(_T("%d"), pOrder->FrontID);
		pQryOrdersList->SetItemText(orderIndex, 12, (LPCTSTR)csFrontID);

		CString csSessionID("");
		csSessionID.Format(_T("%d"), pOrder->SessionID);
		pQryOrdersList->SetItemText(orderIndex, 13, (LPCTSTR)csSessionID);

		pQryOrdersList->EnsureVisible(pQryOrdersList->GetItemCount() - 1, FALSE);
	}
}