#pragma once
#include"..\\config\\Ser_config.h"
#include"..\\net\\tcpnet.h"
#include"..\\net\\udpnet.h"
#include"..\\MySQL\\CMySql.h"

class kernel;
typedef void(kernel::* pWork)(char* recvData, sockaddr_in fromAddr);
class kernel
{
public:
	udpnet* pToolUdp;
	kernel();
	~kernel();

	CMySql m_sql;

	//工作函数指针数组
	pWork m_arrTypeToFun[_DEF_WORK_TYPE_SIZE_];

	//用户id和与他的在线情况
	//用户上线把他加进来，状态为1；不符和心跳机制时把他的状态置为0，重发无回复时删除这个用户
	map<int, bool> m_mapIdToState;

	//保存所有客户端的id-sockaddr_in信息
	//int nOnlineClients;
	map<int, sockaddr_in> m_mapNoToAdd;			//在线客户机在服务器的编号和他的addr
	//map<int, sockaddr_in> m_mapIdToAdd;			//在线客户机在服务器的id和他的addr
	map<int, int> m_mapIdToNo;					//在线客户机在服务器的编号和他的id


	//保存所有在线的群id
	std::map<int, std::list<int>> m_mapGroInfo;
	//oneGroInfo m_arrGroInfo[_DEF_MAX_GROUP_SIZE];

	//循环验证每个在线客户端是否登录
	HANDLE m_handle;

	//外部接口
	static kernel* m_pKernel;

	//打开服务器
	bool startService();

	//关闭服务器
	void closeService();

	//核心工作
	void nuclearWork(char* recvData, sockaddr_in fromAddr);

	//处理注册请求
	void dealRegRq(char* recvData, sockaddr_in fromAddr);

	//处理登录请求,回复能否登录 能的话回复：好友信息 群聊信息 群聊成员信息
	void dealLogRq(char* recvData, sockaddr_in fromAddr);

	//登录成功后发送给他好友信息和群聊信息，并告诉所有好友所有群成员他上线了
	void ConnectRelationships(int id, sendBuf* userInfo);

	//转发群聊信息
	void dealGroupMsg(char* recvData, sockaddr_in fromAddr);

	//处理聊天 转发聊天消息
	void dealChatMsg(char* recvData, sockaddr_in fromAddr);

	//处理添加好友请求
	void dealAddFriendRq(char* recvData, sockaddr_in fromAddr);

	//回复添加好友
	void dealAddFriendRs(char* recvData, sockaddr_in fromAddr);

	//处理删除好友的请求
	void dealDeleteFriendNoti(char* recvData, sockaddr_in fromAddr);

	//处理下线通知
	void dealOfflineNoti(char* recvData, sockaddr_in fromAddr);

	//心跳机制验活
	void keepAlive(char* recvData, sockaddr_in fromAddr);

	//记录登录成功的客户机的addr
	void recordAddr(char* recvData,sockaddr_in addr);

	//根据用户id获取他的好友信息
	void getAllFriendInfo(int);

	//根据id获取他的信息
	bool getInfoFromId(int id,sendBuf* info);

	//初始化
	void setArrayTypeToFun();

};