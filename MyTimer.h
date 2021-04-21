/////////////////////////////////////////// MyTimer.h: interface for the CMyTimer class.
#pragma once
#include <afxtempl.h>
class CMyTimer;
typedef CMap<UINT, UINT, CMyTimer*, CMyTimer*> CTimerMap;

class CMyTimer
{
public:
	void SetMyTimer(UINT nElapse, CString sz);//设置定时器，nElapse表示时间间隔，sz表示要提示的内容
	static void KillMyTimer(UINT idEvent);//销毁该实例的定时器
	UINT m_nTimerID;//保存该实例的定时器标志值
	CString szContent;//静态数据成员要提示的内容
	static CTimerMap m_sTimeMap;//静态数据成员，映射表类，用于保存所有的定时器信息
	static void CALLBACK MyTimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime);//静态成员函数，用于处理定时器的消息
	CMyTimer();
	virtual ~CMyTimer();
};