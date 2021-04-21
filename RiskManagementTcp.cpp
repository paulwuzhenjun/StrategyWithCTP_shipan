#include "StdAfx.h"
#include "RiskManagementTcp.h"
#include <vector>
#include "Strategy.h"
#include "GlobalFunc.h"

using namespace std;

extern char RSServerIP[];
extern int RSServerPort;

unsigned int __stdcall	pTcpClientThread(void*);
int tcpstate;

extern CStrategy* gStrategyImpl[MAX_RUNNING_STRATEGY];
extern int gStrategyImplIndex;
extern GlobalFunc globalFuncUtil;
extern char LoginUserTDCTP[];
extern char LoginUserTDEsun[];

RiskManagementTcp::RiskManagementTcp(char x_Address[32],
	int x_tcpPort)
{
	m_bRunning = true;
	strcpy(m_Address, x_Address);
	m_tcpPort = x_tcpPort;

	tcpstate = TCP_DISCONNNECTED;
}

RiskManagementTcp::~RiskManagementTcp(void)
{
}

void RiskManagementTcp::Release()
{
	m_bRunning = false;
}

void RiskManagementTcp::ProcessTCPMsg()
{
	rs_msg cur_msg;
	close_action* p_action = 0;
	stop_action* p_stop = 0;
	strategy_heart_check* p_heartcheck = 0;
	while (m_bRunning) {
		if (!rcvlist.DataListCore.IsEmpty()) {
			rs_msg cur_msgx = rcvlist.GetHead();
			memcpy(&cur_msg, &cur_msgx, sizeof(rs_msg));//
			switch (cur_msg.info_type) {
			case STAT_STOP_OPEN_CLOSE:
				p_stop = (stop_action*)malloc(sizeof(stop_action));
				memcpy(p_stop, cur_msg.info_content, sizeof(stop_action));
				OnStopOpenCloseAction(p_stop);
				free(p_stop);
				p_stop = 0;
				break;
			case STAT_HEART_CHECK_MSG:
				p_heartcheck = (strategy_heart_check*)malloc(sizeof(strategy_heart_check));
				memcpy(p_heartcheck, cur_msg.info_content, sizeof(strategy_heart_check));
				OnStrategyHeartCheckMsg(p_heartcheck);
				free(p_heartcheck);
				p_heartcheck = 0;
				break;
			case 2:
				break;
			}
		}

		if (tcpstate == TCP_DISCONNNECTED || strcmp(RSServerIP, m_Address) != 0) {
			tcpstate = TCP_CONNNECTING;
			strcpy(m_Address, RSServerIP);
			globalFuncUtil.WriteMsgToLogList(RSServerIP);
			_beginthreadex(NULL, 0, pTcpClientThread, this, 0, NULL);
		}
		Sleep(5000);
	}
}

void RiskManagementTcp::OnStopOpenCloseAction(stop_action* p_stop) {
	char xlog[200];
	sprintf(xlog, "RS Tcp OnStopOpenCloseAction=%s,%s,%s,%.2f\n", p_stop->strategy_id, p_stop->instance_name, p_stop->acct_id, p_stop->percent);
	globalFuncUtil.WriteMsgToLogList(xlog);

	//收到停止开平信息,若Percent>0 则需要策略平仓,操作结束则发送已处理信息
	setdone_action setdone_msg;
	strcpy(setdone_msg.acct_id, p_stop->acct_id);
	strcpy(setdone_msg.strategy_id, p_stop->strategy_id);
	strcpy(setdone_msg.instance_name, p_stop->instance_name);
	setdone_msg.rule_type = p_stop->rule_type;

	for (int i = 0; i < gStrategyImplIndex; i++) {
		if (gStrategyImpl[i] != NULL && gStrategyImpl[i]->GetShmindex() >= 0 && gStrategyImpl[i]->m_bIsRunning
			&& strcmp(gStrategyImpl[i]->mStrategyID, p_stop->strategy_id) == 0) {
			ParamNode node;
			strcpy(node.ParamValue, "0");
			strcpy(node.ParamName, "OpenBuyAllow");
			gStrategyImpl[i]->SetParamValue(node);
			strcpy(node.ParamValue, "0");
			strcpy(node.ParamName, "OpenSellAllow");
			gStrategyImpl[i]->SetParamValue(node);
			strcpy(node.ParamValue, "0");
			strcpy(node.ParamName, "CloseAllow");
			gStrategyImpl[i]->SetParamValue(node);
		}
	}
	rs_msg cur_msg;
	strcpy(cur_msg.info_from, m_Address);
	strcpy(cur_msg.info_to, "server");
	cur_msg.info_type = STAT_SETDONE_MSG;
	memcpy(cur_msg.info_content, &setdone_msg, sizeof(setdone_msg));
	cur_msg.info_length = sizeof(setdone_msg);
	sendlist.AddTail(cur_msg);
}

void RiskManagementTcp::OnStrategyHeartCheckMsg(strategy_heart_check* p_stop) {
	//收到心跳检查则发送已处理信息
	strategy_heart_check p_action;
	strcpy(p_action.acct_id, p_stop->acct_id);
	strcpy(p_action.strategy_id, p_stop->strategy_id);
	strcpy(p_action.instance_name, p_stop->instance_name);

	rs_msg cur_msg;
	strcpy(cur_msg.info_from, m_Address);
	strcpy(cur_msg.info_to, "server");
	cur_msg.info_type = STAT_HEART_CHECK_MSG;
	memcpy(cur_msg.info_content, &p_action, sizeof(strategy_heart_check));
	cur_msg.info_length = sizeof(p_action);
	sendlist.AddTail(cur_msg);
}

void RiskManagementTcp::OnCloseAction(close_action* p_action) {
	char xlog[200];
	sprintf(xlog, "RS Tcp OnCloseAction=%s,%s,%d\n", p_action->strategy_id, p_action->close_inst_name, p_action->vol);
	globalFuncUtil.WriteMsgToLogList(xlog);
	setdone_action setdone_msg;
	strcpy(setdone_msg.acct_id, p_action->acct_id);
	strcpy(setdone_msg.strategy_id, p_action->strategy_id);

	rs_msg cur_msg;
	strcpy(cur_msg.info_from, m_Address);
	strcpy(cur_msg.info_to, "server");
	cur_msg.info_type = STAT_SETDONE_MSG;
	memcpy(cur_msg.info_content, &setdone_msg, sizeof(setdone_msg));
	cur_msg.info_length = sizeof(setdone_msg);
	sendlist.AddTail(cur_msg);
}

unsigned int __stdcall pTcpClientThread(void* arg)
{
	tcpstate = TCP_CONNNECTING;
	//globalFuncUtil.WriteMsgToLogList("RS Tcp Connecting.\n");
	RiskManagementTcp* pRSThreadX = (RiskManagementTcp*)arg;
	//加载套接字库
	WSADATA wsaData;
	int iRet = 0;
	iRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iRet != 0)
	{
		tcpstate = TCP_DISCONNNECTED;
		return 0;
	}
	if (2 != LOBYTE(wsaData.wVersion) || 2 != HIBYTE(wsaData.wVersion))
	{
		WSACleanup();
		tcpstate = TCP_DISCONNNECTED;
		return 0;
	}

	//创建套接字
	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (clientSocket == INVALID_SOCKET)
	{
		tcpstate = TCP_DISCONNNECTED;
		return 0;
	}

	//初始化服务器端地址族变量
	SOCKADDR_IN srvAddr;
	srvAddr.sin_addr.S_un.S_addr = inet_addr(RSServerIP);
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons(RSServerPort);

	//连接服务器
	iRet = connect(clientSocket, (SOCKADDR*)&srvAddr, sizeof(SOCKADDR));

	if (0 != iRet)
	{
		tcpstate = TCP_DISCONNNECTED;
		return 0;
	}

	////Send 注册信息 From Client
	for (int i = 0; i < gStrategyImplIndex; i++) {
		if (gStrategyImpl[i] != NULL && gStrategyImpl[i]->GetShmindex() >= 0 && gStrategyImpl[i]->m_bIsRunning) {
			rs_msg cur_msg;
			strategy_reg_info strategy_info;
			memset(&cur_msg, 0, sizeof(rs_msg));
			cur_msg.info_socket_id = -1;
			strcpy(cur_msg.info_from, "TL");
			strcpy(cur_msg.info_to, "server");
			cur_msg.info_type = STAT_LOGIN_MSG;
			memset(&strategy_info, 0, sizeof(strategy_reg_info));
			strcpy(strategy_info.strategy_id, gStrategyImpl[i]->mStrategyID);
			gStrategyImpl[i]->GetInstanceName(strategy_info.instance_name);
			char xAcctID[50];
			sprintf(xAcctID, "%s-%s", LoginUserTDEsun, LoginUserTDCTP);
			strcpy(strategy_info.acct_id, xAcctID);
			char xCodeName[100];
			sprintf(xCodeName, "%s", gStrategyImpl[i]->InstCodeName);
			strcpy(strategy_info.inst_name, xCodeName);
			memcpy(cur_msg.info_content, &strategy_info, sizeof(strategy_info));
			cur_msg.info_length = sizeof(strategy_info);
			pRSThreadX->sendlist.AddTail(cur_msg);
		}
	}
	//rs_msg cur_msg;
	//strcpy(cur_msg.info_from,"192.168.130.10");
	//strcpy(cur_msg.info_to,"server");
	//cur_msg.info_type=STAT_LOGIN_MSG;
	//strategy_reg_info strategy_info;
	//strcpy(strategy_info.strategy_id,"CL-GD-00006-3");
	//strcpy(strategy_info.instance_name,"instance1");
	//strcpy(strategy_info.acct_id,"C1171");
	//strcpy(strategy_info.inst_name,"NYMEX CL 1908");
	//memcpy(cur_msg.info_content,&strategy_info,sizeof(strategy_info));
	//cur_msg.info_length=sizeof(strategy_info);
	//pRSThreadX->sendlist.AddTail(cur_msg);
	//接收消息
	char recvBuf[1024];
	int retVal;
	//发送消息
	char sendBuf[1024] = "123456\n";

	fd_set	wfds, rfds, efds;
	timeval tv;
	tv.tv_sec = 10;
	tv.tv_usec = 500;
	int count = -1;
	globalFuncUtil.WriteMsgToLogList("RS Tcp connected.");
	globalFuncUtil.WriteMsgToLogList(RSServerIP);
	tcpstate = TCP_CONNNECTED;
	while (pRSThreadX->m_bRunning)
	{
		FD_ZERO(&wfds);
		FD_ZERO(&rfds);
		FD_ZERO(&efds);
		FD_SET(clientSocket, &wfds);
		FD_SET(clientSocket, &rfds);
		FD_SET(clientSocket, &efds);
		int Rtn = select(FD_SETSIZE, &rfds, &wfds, &efds, &tv);
		if (Rtn > 0)
		{
			if (FD_ISSET(clientSocket, &rfds) > 0)//表示有读的
			{
				//struct send_info clt; //(1)定义结构体变量
				rs_msg rcv_msg;
				memset(recvBuf, 0, 1024);//清空缓存
				count = recv(clientSocket, recvBuf, 1024, 0);
				if (count > 0) {
					memset(&rcv_msg, 0, sizeof(rs_msg));//清空结构体
					memcpy(&rcv_msg, recvBuf, sizeof(rs_msg));//(3)把接收到的信息转换成结构体
					rcv_msg.info_content[rcv_msg.info_length] = '\0';
					//消息内容结束，没有这句的话，可能导致消息乱码或输出异常,有网友建议说传递的结构体中尽量不要有string类型的字段，估计就是串尾符定位的问题
					pRSThreadX->rcvlist.AddTail(rcv_msg);
					//if(rcv_msg.info_content) //判断接收内容并输出
					//	printf("nclt.info_from is %s,nclt.info_to is %s nclt.info_content is %s nclt.info_length is %d \n",rcv_msg.info_from,rcv_msg.info_to,rcv_msg.info_content,rcv_msg.info_length);
				}
				else {
					closesocket(clientSocket); //关闭套接字
					WSACleanup(); //释放套接字资源
					tcpstate = TCP_DISCONNNECTED;
					return 0;
				}
			}

			if (FD_ISSET(clientSocket, &wfds) > 0)//表示可以写
			{
				if (!pRSThreadX->sendlist.DataListCore.IsEmpty())
				{
					rs_msg cur_msg = pRSThreadX->sendlist.GetHead();
					memcpy(sendBuf, &cur_msg, sizeof(rs_msg));//
					retVal = send(clientSocket, sendBuf, sizeof(sendBuf), 0);
					if (SOCKET_ERROR == retVal)
					{
						closesocket(clientSocket); //关闭套接字
						WSACleanup(); //释放套接字资源
						tcpstate = TCP_DISCONNNECTED;
						return 0;
					}
					count = -1;
				}
			}

			if (FD_ISSET(clientSocket, &efds) > 0)//表示有异常
			{
				globalFuncUtil.WriteMsgToLogList("RS Tcp error\n");
			}
		}
		else
		{
		}
		Sleep(10);
	}

	//清理
	closesocket(clientSocket);
	WSACleanup();
	globalFuncUtil.WriteMsgToLogList("RS Tcp disconnected.\n");
	tcpstate = TCP_DISCONNNECTED;
}