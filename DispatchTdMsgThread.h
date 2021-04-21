#pragma once
class DispatchTdMsgThread
{
public:
	DispatchTdMsgThread(void);
	~DispatchTdMsgThread(void);
	char beginrun_date[10];
	bool m_bIsRunning;

	void DispatchTdMsg();
};
