#pragma once
class CLockVariable
{
public:
	CLockVariable(void);
	~CLockVariable(void);

	int ThostNextOrderRef;
	void setThostNextOrderRef(int m_ThostNextOrderRef);
	int getThostNextOrderRef();
};
