#pragma once
#include"./mediator/INetMediator.h"
#include<iostream>
#include"./MySQL/CMySql.h"
#include"./dataDef/dataDef.h"
#include<map>
#include<set>
using namespace std;


//定义函数指针
class CKernel;
typedef void(CKernel::* pKernelDeal)(char*, int, long);


class CKernel {
public:
	CKernel();
	~CKernel();

	//打开服务器
	bool startService();
	//关闭服务器
	void closeService();

	//处理所有接收到的数据
	void dealReadyData(char* recvData,int nLen,long lFrom);

	//处理注册请求的函数
	void dealRegisterRq(char* recvData, int nLen, long lFrom);

	//处理登录请求的函数
	void dealLoginRq(char* recvData, int nLen, long lFrom);

	//处理登录请求2---把他参加的群聊的成员信息发给他
	void getAllGroupMember(int id);

	//转发群聊信息
	void dealGroupMsg(char* recvData, int nLen, long lFrom);

	//处理聊天内容 转发聊天消息
	void dealChatRq(char* recvData, int nLen, long lFrom);

	//处理添加好友的数据报
	void dealAddFriendRq(char* recvData, int nLen, long lFrom);

	//处理回复添加好友的数据报
	void dealAddFriendRs(char* recvData, int nLen, long lFrom);

	//处理删除好友的请求
	void dealDeleteFriendRq(char* recvData, int nLen, long lFrom);

	//处理下线请求的函数
	void dealOfflineRq(char* recvData, int nLen, long lFrom);

	//处理客户端的登录验证回复
	void dealValidateRs(char* recvData, int nLen, long lFrom);

	//根据用户id获取他的好友信息
	void getFriendInfo(int);

	//根据id获取他的信息
	void getInfoFromId(int,_STRU_TCP_FRIEND_INFO*/*传出参数*/);

	//接收文件 并转发文件
	void dealRecvAndResendFile(char* recvData, int nLen, long lFrom);

	//处理文件接收到服务器完毕的信号
	void dealFileRecvAcc(char* recvData, int nLen, long lFrom);

	//处理文件由服务器转发完毕的信号
	void dealFileResendAcc(char* recvData, int nLen, long lFrom);

	//处理文件接受者的文件接收回复
	void dealFileRecverRs(char* recvData, int nLen, long lFrom);
public:
	bool haveOnlineClient;
	INetMediator *m_pMediator;
	CMySql m_sql;
	pKernelDeal m_mapPackTypeToFun[_DEF_PROTOCOL_COUNT];

	//保存每一个在线客户端已登录的socket 和 id(转发消息时取出对应的socket。判断某个客户端是否在线)
	map<int, SOCKET> m_mapIdToSocket;
	//循环验证客户端登录状态的记录
	map<int, bool> m_mapIdToClientStatus;
	//保存每一个在线客户端已登录的群聊信息--
	map<int, list<int>*> m_mapGroupIdToMember;
	//保存接收到本地的文件id
	set<string> m_setFileRecvAcc;
	//保存转发成功的文件id
	set<string> m_setFileResendAcc;
	//初始化协议头数组
	void setProtocolMap();
	//循环验证每个在线客户端是否登录
	HANDLE m_handle;
	static unsigned __stdcall ValidateClientLogin(void* lpVoid);
public:
	static CKernel* m_pKernel;
};