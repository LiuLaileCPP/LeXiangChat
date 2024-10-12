#pragma once
#include"INet.h"
#include"../mediator/TcpServiceMediator.h"

//子类--UDP网络功能类
class TcpService :public INet {
public:
	//保存文件块
	map<string, STRU_FILE_INFO*> m_mapIdToFileInfo;

	TcpService(TcpServiceMediator* p_UdpNetMediator);
	~TcpService();
	//初始化网络
	bool InitINet();

	//发送数据(发送地址怎么决定？ UDP:ip TCP:talksock)
	bool SendData(char* sendData, int nLen, long lSend);

	//接收数据
	void RecvData();

	//关闭网络
	void UninitNet();

	//处理文件的：
	void SendData(SOCKET sock, char* buf, int nLen);
	//发送文件
	void SendFile(char* fileId);
	string getName(char* str);
	//接收
	void DealFileBlockRq(char* recvData, int nLen);
	void DealFileInfoRq(char* recvData, int nLen);
	void DealData(char* recvData, int nLen);

	void RefreshSocket(SOCKET sock);
private:

	SOCKET m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//HANDLE m_handle;
	bool m_bStop;
	
	//保存sockTalk 和客户端（接收数据的线程ID）的关系
	map<unsigned int, SOCKET>m_mapThreadIdToSocket;
	//保存所有线程句柄的链表
	list<HANDLE>m_handleList;

	//监听数据包的线程函数（调用accept()）
	static unsigned __stdcall AcceptThread(void* lpVoid);
	//__stdcall 是和OS的调用约定 32位要加

	//接收数据包的线程函数（调用RecvData()）
	static unsigned __stdcall RecvThread(void* lpVoid);

	//重连sock 刷新m_mapThreadIdToSocket
	static unsigned __stdcall ReConnectSock(void* lpVoid);

};
