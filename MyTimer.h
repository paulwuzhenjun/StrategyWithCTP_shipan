/////////////////////////////////////////// MyTimer.h: interface for the CMyTimer class.
#pragma once
#include <afxtempl.h>
class CMyTimer;
typedef CMap<UINT, UINT, CMyTimer*, CMyTimer*> CTimerMap;

class CMyTimer
{
public:
	void SetMyTimer(UINT nElapse, CString sz);//���ö�ʱ����nElapse��ʾʱ������sz��ʾҪ��ʾ������
	static void KillMyTimer(UINT idEvent);//���ٸ�ʵ���Ķ�ʱ��
	UINT m_nTimerID;//�����ʵ���Ķ�ʱ����־ֵ
	CString szContent;//��̬���ݳ�ԱҪ��ʾ������
	static CTimerMap m_sTimeMap;//��̬���ݳ�Ա��ӳ����࣬���ڱ������еĶ�ʱ����Ϣ
	static void CALLBACK MyTimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime);//��̬��Ա���������ڴ���ʱ������Ϣ
	CMyTimer();
	virtual ~CMyTimer();
};