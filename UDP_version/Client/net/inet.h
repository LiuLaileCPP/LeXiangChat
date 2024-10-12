#pragma once
#include"..\\config\\Cli_config.h"


class inet :public QObject{
    Q_OBJECT
public:
	bool m_netState;
	recvBuf m_recvbuf;
	sendBuf m_sendbuf;
	SOCKET m_sockToSer;
    sockaddr_in m_addrOfSer;
    int m_nAddr;

	HANDLE m_hthread_alive;
	DWORD m_id_alive;
	HANDLE m_hthread_recv;
	DWORD m_id_recv;
	inet(){}
	virtual~inet(){}
	virtual bool InitNet() = 0;
	virtual bool RecvData() = 0;
    virtual bool SendData(char* msg,int msgLen) = 0;
	virtual bool CloseNet() = 0;
	virtual void bzeroRecvBuf() = 0;
	virtual void bzeroSendBuf() = 0;
};



