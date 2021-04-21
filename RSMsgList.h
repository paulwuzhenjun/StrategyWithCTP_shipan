#pragma once
#include <Afxtempl.h>

#define STAT_LOGIN_MSG 1
#define STAT_SETDONE_MSG 2
#define STAT_HEART_CHECK_MSG 3
#define STAT_STOP_OPEN_CLOSE 4
#define CLOSE_ACTION 5

#define ACCT_RULE 0
#define STRAT_RULE 1

#pragma pack(push,1)
struct rs_msg
{
	int info_socket_id; //socket id
	char info_from[20]; //发送者ID
	char info_to[20]; //接收者ID
	int info_type;   //消息类别
	int info_length; //发送的消息主体的长度
	char info_content[512]; //消息主体
};
#pragma pack(pop)

#pragma pack(push,1)
struct strategy_reg_info
{
	char strategy_id[30];
	char instance_name[30];
	char acct_id[60];
	char inst_name[100];
};
#pragma pack(pop)

#pragma pack(push,1)
struct stop_action
{
	char strategy_id[50];
	char instance_name[30];
	char acct_id[60];
	int rule_type; // 0 - ACCT_RULE, 1 - STRAT_RULE
	double percent; //平仓比例, 1 -100%, 0.5 - 50%
};
#pragma pack(pop)

#pragma pack(push,1)
struct strategy_heart_check
{
	char strategy_id[30];
	char instance_name[30];
	char acct_id[60];
	char inst_name[100];
};
#pragma pack(pop)

#pragma pack(push,1)
struct setdone_action
{
	char strategy_id[50];
	char instance_name[30];
	char acct_id[60];
	int rule_type; // 0 - ACCT_RULE, 1 - STRAT_RULE
};
#pragma pack(pop)

#pragma pack(push,1)
struct close_action
{
	char strategy_id[50];
	char instance_name[50];
	char acct_id[60];
	char close_inst_name[50];
	int vol;
};
#pragma pack(pop)

class RSMsgList
{
public:
	CList<rs_msg, rs_msg&> DataListCore;
public:
	RSMsgList(void);
	~RSMsgList(void);
	void AddTail(rs_msg m);
	rs_msg GetHead();
private:
	void RemoveHead();
};
