///////////////////////////////////// MyTimer.cpp: implementation of the CMyTimer class.
#include "stdafx.h"
#include "MyTimer.h"

CTimerMap CMyTimer::m_sTimeMap;

CMyTimer::CMyTimer()
{
	m_nTimerID = 0;
}

CMyTimer::~CMyTimer()
{
}

void CALLBACK CMyTimer::MyTimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
	/*
		CString sz;
		sz.Format(_T("%dTimer:%s"),idEvent,m_sTimeMap[idEvent]->szContent);
		AfxMessageBox(sz);
	*/

	CMyTimer::KillMyTimer(idEvent);
}

void CMyTimer::SetMyTimer(UINT nElapse, CString sz)
{
	szContent = sz;
	m_nTimerID = SetTimer(NULL, NULL, nElapse, MyTimerProc);
	m_sTimeMap[m_nTimerID] = this;
}

void CMyTimer::KillMyTimer(UINT idEvent)
{
	KillTimer(NULL, idEvent);
	m_sTimeMap.RemoveKey(idEvent);
}