#pragma once
#include<set>
#include<map>
#include<list>
#include <process.h>
#include<Winsock2.h>
#include"../dataDef/dataDef.h"
#pragma comment(lib,"Ws2_32.lib")
#include<pthread.h>
#pragma comment(lib,"pthreadVC2.lib")

#include<iostream>

using namespace std;
class INetMediator;
//struct addrNode {
//	SOCKET s;
//	string strIp;
//};


//抽象父类--网络功能类
class INet {
public:
    SOCKET m_sock = INVALID_SOCKET;
	INet(){}
	~INet(){}
	//初始化网络
	virtual bool InitINet() = 0;

    //获取本机ip列表
    static set<long> GetIpList()
    {
        set<long> iplist;
        char name[100] = "";
        gethostname(name,sizeof(name));
        struct hostent *remoteHost = gethostbyname(name);
        int i = 0;
        unsigned long ip = 0;
        while(remoteHost->h_addr_list[i])
        {
            ip = *(u_long*)remoteHost->h_addr_list[i++];
            iplist.insert(ip);
        }
        return iplist;
    }

	//发送数据(发送地址怎么决定？ UDP:ip TCP:talksock)
	virtual bool SendData(char* sendData,int nLen,long lSend) = 0;

	//关闭网络
	virtual void UninitNet() = 0;

	//处理文件的：
	virtual void SendData(SOCKET sock, char* buf, int nLen) = 0;
	//发送文件
	virtual void SendFile(char* fileId) = 0;
	virtual string getName(char* str) = 0;
	//接收
	virtual void DealFileBlockRq(char* recvData, int nLen) = 0;
	virtual void DealFileInfoRq(char* recvData, int nLen) = 0;
	virtual void DealData(char* recvData, int nLen) = 0;

	virtual void RefreshSocket(SOCKET sock) = 0;
protected:
	//接收数据
	virtual void RecvData() = 0;

	INetMediator* m_pMediator;
    
};
