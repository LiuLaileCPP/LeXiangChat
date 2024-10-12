#pragma once
#include"..\\net\\inet.h"
class kernel;

class udpnet:public inet {
public:
	sockaddr_in m_addrCli;
	int m_addrCli_len;
	kernel* m_pkernel;
	udpnet();
	//udpnet(kernel* p);
	virtual ~udpnet();
	virtual bool InitNet();
	virtual bool RecvData();
	virtual bool SendData(char* msg,sockaddr_in toAddr);
	virtual bool CloseNet();
	virtual void bzeroRecvBuf();
	virtual void bzeroSendBuf();
};

DWORD WINAPI job_udpAlive(LPVOID arg);
DWORD WINAPI job_udpRecv(LPVOID arg);