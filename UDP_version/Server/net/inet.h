#include"..\\config\\Ser_config.h"


class inet {
public:
	bool m_netState;
	recvBuf m_recvbuf;
	sendBuf m_sendbuf;
	SOCKET m_sockToCli;
	sockaddr_in m_addrOfSer;
	HANDLE m_hthread_recv;
	DWORD m_id_recv;
	inet(){}
	virtual ~inet(){}
	virtual bool InitNet() = 0;
	virtual bool RecvData() = 0;
	virtual bool SendData(char* msg, sockaddr_in toAddr) = 0;	//发什么(sendBuf) 往哪里发
	virtual bool CloseNet() = 0;
	virtual void bzeroRecvBuf() = 0;
	virtual void bzeroSendBuf() = 0;
};