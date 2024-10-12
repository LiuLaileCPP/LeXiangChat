#pragma once
#include"..\\net\\inet.h"


class udpnet:public inet {
    Q_OBJECT
public:
	udpnet();
	virtual ~udpnet();
	virtual bool InitNet();
	virtual bool RecvData();
    virtual bool SendData(char* msg,int msgLen);
	virtual bool CloseNet();
	virtual void bzeroRecvBuf();
	virtual void bzeroSendBuf();
};

DWORD WINAPI job_udpAlive(LPVOID arg);
DWORD WINAPI job_udpRecv(LPVOID arg);
