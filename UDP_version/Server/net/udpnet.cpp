#pragma once
#include"..\\net\\udpnet.h"
#include"..\\kernel\\kernel.h"
#include<iostream>
using namespace std;
#pragma comment(lib,"Ws2_32.lib")


udpnet::udpnet()
{
	m_netState = false;
	memset(&m_addrCli,0,sizeof(m_addrCli));
	memset(&m_addrOfSer,0,sizeof(m_addrOfSer));
	m_addrCli_len = sizeof(m_addrCli);
	m_pkernel = nullptr;
}

//udpnet::udpnet(kernel* p)
//{
//	m_pkernel = p;
//	m_netState = false;
//	m_hthread_recv = CreateThread(0, 0, job_udpRecv, (LPVOID)this, 0, &m_id_recv);
//	memset(&m_addrCli, 0, sizeof(m_addrCli));
//	memset(&m_addrOfSer, 0, sizeof(m_addrOfSer));
//	int m_addrCli_len = 0;
//}

udpnet::~udpnet()
{
	cout << "udpnet:: ~udpnet" << endl;
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

	if ((m_sockToCli = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		cout<<"udpnet:: InitNet socket failed"<<endl;
		WSACleanup();
		return false;
	}
	else
	{
		cout << "udpnet:: InitNet socket succeed" << endl;
	}

	//绑定好server的信息，为收发数据做准备
	m_addrOfSer.sin_family = AF_INET;
	m_addrOfSer.sin_port = htons(SERVER_PORT);
	m_addrOfSer.sin_addr.S_un.S_addr = ADDR_ANY;
	//绑定所有本机ip, 让客户端往任意的地址和端口号发送 服务器都能收得到
	

	if (bind(m_sockToCli,(sockaddr*)&m_addrOfSer,sizeof(m_addrOfSer)) == -1)
	{
		cout << "udpnet:: InitNet bind failed" << endl;
		WSACleanup();
		closesocket(m_sockToCli);
		return false;
	}
	else
	{
		cout << "udpnet:: InitNet bind succeed" << endl;
	}
	/*u_long mode = 1;
	ioctlsocket(m_sockToCli, FIONBIO, &mode);*/

	m_netState = true;
	m_hthread_recv = CreateThread(0, 0, job_udpRecv, (LPVOID)this, 0, &m_id_recv);
	return true;
}

bool udpnet::RecvData()
{
	/*if (m_netState && m_recvbuf.type != 0)
	{
		cout << "udpnet:: RecvData recvbuf has data to treat, fail to recv" << endl;
		return false;
	}*/
	int recvlen = 0;
	if ((recvlen = recvfrom(m_sockToCli,(char*) &m_recvbuf,sizeof(m_recvbuf),0,(sockaddr*)&m_addrCli,&m_addrCli_len)) > 0)
	{
		cout << "udpnet:: RecvData succeed, msg type: "<<m_recvbuf.type << endl;	//外界使用完数据后把数据赋空
		cout << "no is:" << m_recvbuf.no << endl;
		return true;
	}
	else if (recvlen == 0)
	{
		cout << "udpnet:: RecvData Server closed " << WSAGetLastError();
		perror(" perror:");
		return false;
	}
	else if (recvlen < 0)
	{
		cout<<"udpnet:: RecvData failed "<<WSAGetLastError();
		perror(" perror:");
		return false;
	}

	return false;
}

bool udpnet::SendData(char* msg,sockaddr_in toAddr)
{
	/*if (m_netState && m_sendbuf.type != 0)
	{
		cout << "udpnet:: SendData sendbuf has data, fail to send" << endl;
		return false;
	}*/
	cout << "*** SendData:: " << " IP:" << inet_ntoa(toAddr.sin_addr) << " PORT:" << ntohs(toAddr.sin_port) << endl;


	m_addrCli = toAddr;
	int err = 0;
	/*cout << "udpnet:: SendData msg:: buf1:" << ((sendBuf*)msg)->buf1 << " buf2:"<< ((sendBuf*)msg)->buf2
		<<" buf3:"<< ((sendBuf*)msg)->buf3<<" buf4:"<< ((sendBuf*)msg)->buf4<< endl;*/
	if ((err = sendto(m_sockToCli,msg,sizeof(sendBuf),0,(sockaddr*)&m_addrCli,m_addrCli_len)) > 0)	//阻塞发送
	{
		cout << "udpnet:: SendData succeed, msg type: "<<((sendBuf*)msg)->type << endl;
		bzeroSendBuf();
		return true;
	}
	else if (err == 0)
	{
		cout << "udpnet:: SendData Server closed "<<WSAGetLastError();
		perror(" perror:");
		return false;
	}
	else	//err < 0
	{
		cout << "udpnet:: SendData failed " << WSAGetLastError();
		perror(" perror:");
		return false;
	}
	return false;
}

bool udpnet::CloseNet()
{
	WSACleanup();
	cout << "udpnet:: CloseNet sock: "<<m_sockToCli << endl;
	if (!closesocket(m_sockToCli))
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
	//应该由kernel开启此线程

	//udpnet* p = (udpnet*)arg;
	//if (!p)
	//	return 0;
	//while (1)
	//{
	//	cout << "** udpthread_alive:: running..." << endl;
	//	if (!p->m_netState)
	//	{
	//		cout << "** udpthread_alive:: nonet..." << endl;
	//		Sleep(30000);
	//		continue;
	//	}
	//	char live_buf[1024] = "alive";
	//	sendBuf msg;
	//	msg.type = TYPE_ALIVE;
	//	strcpy_s(msg.buf1,live_buf);
	//	//p->SendData((char*)&msg,fromAddr);
	//	Sleep(30000);
	//}
	return 0;
}

DWORD WINAPI job_udpRecv(LPVOID arg)
{
	udpnet* p = (udpnet*)arg;
	while (1)
	{
		cout << "** udpthread_recv:: running..." << endl;
		if (!p)
		{
			cout << "** udpthread_recv:: arg==0, thread exit." << endl;
			return 0;
		}
		

		if (!p->m_netState)
		{
			cout << "** udpthread_recv:: nonet..." << endl;
			p->InitNet();
			Sleep(1000);
			continue;
		}
		if (p->RecvData())
		{
			cout << "** thread_recv:: deliver data to kernel..." << endl;
			kernel::m_pKernel->nuclearWork((char*)&(p->m_recvbuf), p->m_addrCli);	//接收了什么 从哪里接收的
		}
	}
	cout << "thread_recv exit." << endl;
}