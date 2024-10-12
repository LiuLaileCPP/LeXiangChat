#pragma once
#include"INetMediator.h"
//#include<QObject>

class UdpNetMediator:public INetMediator 
{
   // Q_OBJECT
public:
	UdpNetMediator();
	~UdpNetMediator();
	//打开网络
	bool OpenNet();
	//转发数据
	bool SendData(char* sendData, int nLen, long lSend) ;
	//处理数据
	void DealData(char* sendData, int nLen, long lFrom) ;
	//关闭网络
	void CloseNet();
	//发送文件
	void SendFile(char* fileId);

	void RefreshSocket(long);

//signals:
    //把接收到的数据传给Kernel
    //void SIG_ReadyData(char* recvData,int nLen,long lFrom);

};
