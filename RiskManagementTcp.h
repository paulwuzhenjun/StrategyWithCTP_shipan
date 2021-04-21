#pragma once
#include <vector>
#include <winsock2.h>
#include "RSMsgList.h"

using namespace std;

#define TCP_CONNNECTED 1
#define TCP_CONNNECTING 2
#define TCP_DISCONNNECTED 3

class RiskManagementTcp
{
#pragma pack(push,1)
	struct send_info
	{
		char info_from[20]; //发送者ID
		char info_to[20]; //接收者ID
		int info_type;   //消息类别
		int info_length; //发送的消息主体的长度
		char info_content[512]; //消息主体
	};
#pragma pack(pop)

public:
	RiskManagementTcp(char x_Address[32],
		int x_tcpPort);
	~RiskManagementTcp(void);

	void ProcessTCPMsg();
	bool m_bRunning;

	RSMsgList rcvlist;
	RSMsgList sendlist;

	void OnStopOpenCloseAction(stop_action* p_stop);
	void OnStrategyHeartCheckMsg(strategy_heart_check* p_stop);
	void OnCloseAction(close_action* p_action);

	char m_Address[32];
	int m_tcpPort;

	void Release();
};
