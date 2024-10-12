#pragma once
//#define _HAS_STD_BYTE 0
#include<Winsock2.h>
#include<WS2tcpip.h>
#include<set>
#include<map>
#include<list>
#include <process.h>
#include"../dataDef/dataDef.h"
//#pragma comment(lib,"Ws2_32.lib")
#include<pthread.h>
//#pragma comment(lib,"pthreadVC2.lib")
#include<iostream>
#include <windows.h>
#include <commdlg.h>

class INetMediator;
//struct addrNode {
//	SOCKET s;
//	string strIp;
//};


//抽象父类--网络功能类
class INet {
    //using byte = std::byte;
public:
	INet(){}
    virtual ~INet(){}
    SOCKET m_sock = INVALID_SOCKET;
	//初始化网络
	virtual bool InitINet() = 0;

    //获取本机ip列表
    static char* m_psetLongIpList;


	//发送数据(发送地址怎么决定？ UDP:ip TCP:talksock)
	virtual bool SendData(char* sendData,int nLen,long lSend) = 0;

	//关闭网络
	virtual void UninitNet() = 0;

    //sock连接网络
    virtual bool ConnectNet() = 0;

    virtual const SOCKET getSock() = 0;

    //处理文件的：
    virtual void SendData(SOCKET sock,char* buf,int nLen);
    //发送文件
    virtual void SendFile(char* fileId);
    virtual string getName(char* str);
    //接收
    virtual void DealFileBlockRq(char* recvData, int nLen);
    virtual void DealFileInfoRq(char* recvData, int nLen);
    virtual void DealData(char* recvData, int nLen);

protected:
	//接收数据
	virtual void RecvData() = 0;

    //保存文件块
    map<string, STRU_FILE_INFO*> m_mapIdToFileInfo;
	INetMediator* m_pMediator;

};


