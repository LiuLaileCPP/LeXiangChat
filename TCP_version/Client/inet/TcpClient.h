#pragma once
#include"INet.h"
#include"../mediator/TcpClientMediator.h"

//子类--UDP网络功能类
class TcpClient :public INet {
public:
    SOCKET m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	TcpClient(TcpClientMediator* p_UdpNetMediator);
	~TcpClient();
	//初始化网络
	bool InitINet();

	//发送数据(发送地址怎么决定？ UDP:ip TCP:talksock)
	bool SendData(char* sendData, int nLen, long lSend);

	//接收数据
	void RecvData();

	//关闭网络
	void UninitNet();

    //socket连接网络
    bool ConnectNet();

    const SOCKET getSock();

    //处理文件的：
    void SendData(SOCKET sock,char* buf,int nLen);
    //发送文件
    void SendFile(char* fileId);
    string getName(char* str);
    //接收
    void DealFileBlockRq(char* recvData, int nLen);
    void DealFileInfoRq(char* recvData, int nLen);
    void DealData(char* recvData, int nLen);


private:


	HANDLE m_handle;
	bool m_bStop;


	//接收数据的线程函数（调用RecvData()）

	static unsigned __stdcall RecvThread(void* lpVoid);
	//__stdcall 是和OS的调用约定 32位要加


};
