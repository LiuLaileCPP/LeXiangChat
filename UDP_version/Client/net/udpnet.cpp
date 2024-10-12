#pragma once
#include<iostream>
#include<kernel.h>
#include"..\\net\\udpnet.h"
#include <udpthread.h>
using namespace std;
//#pragma comment(lib,"Ws2_32.lib")


udpnet::udpnet()
{
	m_netState = false;
    memset(&m_addrOfSer,0,sizeof(m_addrOfSer));
    m_nAddr = sizeof(m_addrOfSer);
}

udpnet::~udpnet()
{

}

bool udpnet::InitNet()
{

	WSADATA data = {};
	int err;
	if ((err = WSAStartup(MAKEWORD(2, 2), &data)) != 0)
	{
		cout << "udpnet:: InitNet WSAStart failed" << endl;
		return false;
	}
	else
	{
		cout << "udpnet:: InitNet WSAStart succeed" << endl;
	}

	if ((m_sockToSer = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		cout<<"udpnet:: InitNet socket failed"<<endl;
		return false;
	}
	else
	{
		cout << "udpnet:: InitNet socket succeed" << endl;
	}

	//绑定好server的信息，为收发数据做准备
	m_addrOfSer.sin_family = AF_INET;
	m_addrOfSer.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET,SERVER_IP,&m_addrOfSer.sin_addr);

    //如果不手动绑定，cli_fd通信的方式：IP和端口号由操作系统自动分配(此时自动分配端口号)

 //    u_long mode = 1;
 //    ioctlsocket(m_sockToSer, FIONBIO, &mode);
    m_netState = true;

    recvBuf msg;
    msg.type = 0;
    strcpy(msg.buf1,"I am a new Client first connected.");
    SendData((char*)&msg,sizeof(msg));

    m_hthread_alive = CreateThread(0, 0, job_udpAlive, (LPVOID)this, 0, &m_id_alive);
    m_hthread_recv = CreateThread(0, 0, job_udpRecv, (LPVOID)this, 0, &m_id_recv);
    return true;
}

bool udpnet::RecvData()
{
	int recvlen = 0;
    if ((recvlen = recvfrom(m_sockToSer,(char*)&m_recvbuf,sizeof(m_recvbuf),0,(sockaddr*)&m_addrOfSer,&m_nAddr)) > 0)
	{
		cout << "udpnet:: RecvData succeed" << endl;	//外界使用完数据后把数据赋空
		return true;
	}
	else if (recvlen == 0)
	{
        cout << "udpnet:: RecvData Server closed " <<WSAGetLastError();
        perror(" ");
		return false;
	}
	else if (recvlen < 0)
	{
        cout<<"udpnet:: RecvData failed "<<WSAGetLastError();
        perror(" ");
		return false;
	}
	return false;
}

bool udpnet::SendData(char* msg,int nMsg)
{
	int err = 0;
    if ((err = sendto(m_sockToSer,msg,nMsg,0,(sockaddr*)&m_addrOfSer,sizeof(m_addrOfSer))) > 0)	//阻塞发送
	{
		cout << "udpnet:: SendData succeed" << endl;
		bzeroSendBuf();
		return true;
	}
	else if (err == 0)
	{
        cout << "udpnet:: SendData Server closed "<<WSAGetLastError();
        perror(" ");
		return false;
	}
	else	//err < 0
	{
        cout << "udpnet:: SendData failed "<<WSAGetLastError();
        perror(" ");
		return false;
	}
	return false;
}

bool udpnet::CloseNet()
{
	if (!closesocket(m_sockToSer))
	{
		return true;
	}
	return false;
}

void udpnet::bzeroRecvBuf()
{
	memset(&m_recvbuf,0,sizeof(m_recvbuf));
}

void udpnet::bzeroSendBuf()
{
	memset(&m_sendbuf, 0, sizeof(m_sendbuf));
}

DWORD WINAPI job_udpAlive(LPVOID arg)
{
    udpThread tool;
    tool.m_pUdp = (udpnet*)arg;
    tool.aliveWork();
}

DWORD WINAPI job_udpRecv(LPVOID arg)
{
    udpThread tool;
    tool.m_pUdp = (udpnet*)arg;
    tool.recvWork();
}
