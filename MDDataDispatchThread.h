#pragma once
#include "MyStruct.h"

class MDDataDispatchThread
{
public:
	MDDataDispatchThread(void);
	~MDDataDispatchThread(void);

	bool m_bIsRunning;
	void DispatchMdMsg();

	int openThreadShm();

private:
	HANDLE mfile_handle;
	int InitMapArea();

	ShmMsg md_msg;
	ShmTickInfo shm_tick;
	void DispatchMdTicks(TickInfo* pData);
};
