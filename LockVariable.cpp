#include "StdAfx.h"
#include "LockVariable.h"

CCriticalSection lockvariable_cs;

CLockVariable::CLockVariable(void)
{
}

CLockVariable::~CLockVariable(void)
{
}

void CLockVariable::setThostNextOrderRef(int m_ThostNextOrderRef)
{
	lockvariable_cs.Lock();
	ThostNextOrderRef = m_ThostNextOrderRef;
	lockvariable_cs.Unlock();
}

int CLockVariable::getThostNextOrderRef()
{
	int t;
	lockvariable_cs.Lock();
	t = ThostNextOrderRef;
	lockvariable_cs.Unlock();
	return t;
}