#include"TcpService.h"

TcpService::TcpService(TcpServiceMediator* p_UdpNetMediator):m_handleList{}
{
	m_sock = INVALID_SOCKET;
	//m_handle = NULL;
	m_bStop = true;
	m_pMediator = p_UdpNetMediator;
}


TcpService::~TcpService()
{
	
}


//初始化网络: 加载库 创建套接字 绑定IP 监听 多线程连接Client 
bool TcpService::InitINet()
{
	//加载库
	WSADATA data = {};
	int err = WSAStartup(MAKEWORD(2, 2), &data);
	if (err != 0)
	{
		cout << "Service WSAStartup failed" << endl;
		return false;
	}
	else {
		cout << "Service WSAStartup succeed" << endl;
	}
	if (2 != HIBYTE(data.wVersion) || 2 != LOBYTE(data.wVersion))
	{
		cout << "Service WSA version wrong" << endl;
		return false;
	}
	else {
		cout << "Service WSA version right" << endl;
	}



	//创建套接字
	m_sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if (m_sock == INVALID_SOCKET)
	{
		cout << "Service Socket invalid" << endl;
		return false;
	}
	else
	{
		cout << "Service Socket succeed" << endl;
	}



	//绑定IP
	sockaddr_in addr;
	addr.sin_port = htons(_TCP_PORT);
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = INADDR_ANY;
	err = bind(m_sock, (sockaddr*)&addr, sizeof(addr));

	if (SOCKET_ERROR == err)
	{
		cout << "Service Bind failed" << WSAGetLastError() << endl;
		return false;
	}
	else
	{
		cout << "Service Bind succeed" << endl;
	}



	//监听
	err = listen(m_sock, 10); //监听10个socket
	if (SOCKET_ERROR == err)
	{
		cout << "Service Listen failed" << WSAGetLastError() << endl;
		return false;
	}
	else
	{
		cout << "Service Listen succeed" << endl;
	}



	//多线程连接客户端
	u_int id;
	HANDLE a_handle = (HANDLE)_beginthreadex(0//安全级别 0是默认
		, 0 //堆栈大小 0是默认
		, &AcceptThread //起始地址
		, this //参数列表
		, 0	//线程状态 0是创建即运行);
		, &id//分配的线程ID
		);
	if (a_handle) {
		m_handleList.push_back(a_handle);
	}
	//

	//

	//
	return true;
}


//接收连接的线程函数
unsigned __stdcall TcpService:: AcceptThread(void* lpVoid)
{
	TcpService* pThis = (TcpService*)lpVoid;
	//一直接收连接
	sockaddr_in addr; //传出参数
	int addr_len = sizeof(addr);
	SOCKET sockTalk = INVALID_SOCKET;
	u_int  p_id = 0;
	while (pThis->m_bStop)
	{
		sockTalk = accept(pThis->m_sock, (sockaddr*)&addr, &addr_len);
		if (SOCKET_ERROR == sockTalk)
		{
			cout << "Service SockTalk accept failed" <<WSAGetLastError()<< endl;
			continue;
		}
		else
		{
			cout << "Service SockTalk accept succeed IP:"<<inet_ntoa(addr.sin_addr) << endl;
		}

		//创建接收数据的线程
		HANDLE handle = (HANDLE)_beginthreadex(0//安全级别 0是默认
			, 0           //堆栈大小 0是默认
			, &RecvThread //起始地址
			, pThis       //参数列表
			, 0	          //线程状态 0是创建即运行);
			, &p_id       //分配的线程ID
		);
		//保存接收数据的线程句柄
		if (handle)
		{
			pThis->m_handleList.push_back(handle);
		}
		//<通信socket和接收数据的线程ID>对 加入到map中
		pThis->m_mapThreadIdToSocket[p_id] = sockTalk;
	}
	return 1;
}



//接收数据包的线程函数（调用RecvData()）

unsigned __stdcall TcpService:: RecvThread(void* lpVoid)
{
	TcpService* pThis = (TcpService*)lpVoid;
	pThis->RecvData();
	return 0;
}




//发送数据(发送地址怎么决定？ UDP:ip TCP:talksock)
bool TcpService::SendData(char* sendData, int nLen, long lSend) //TCP的lSend是socket
{
	//校验参数合法性
	if (!sendData || nLen == 0)
	{
		cout << "Service SendData 参数 wrong" << endl;
		return false;
	}

	//解决粘包问题---先发包大小 再发包内容
	if (send(lSend,(char*)&nLen,sizeof(int),0) <= 0)
	{
		cout << "Service send data_len failed" << WSAGetLastError() << endl;
		return false;
	}
	if (send(lSend,sendData,nLen,0 ) <= 0)
	{
		cout << "Service send data failed" << WSAGetLastError() << endl;
		return false;
	}
	return true;
}




//接收数据
void TcpService::RecvData()
{
	//休眠一会，等线程加载到map中再运行
	Sleep(100);
	
	//获取当前线程对应的socket
	SOCKET sockTalk = m_mapThreadIdToSocket[GetCurrentThreadId()];
	if (!sockTalk || INVALID_SOCKET == sockTalk)
	{
		cout << "Service Get thread sockTalk failed" << WSAGetLastError() << endl;
		return;
	}
	else
	{
		cout << "Service Get thread sockTalk succeed"<< endl;
	}

	int nRecv = 0;
	int nPackSize = 0;
	int nOffset = 0; //拆包的偏移量
	while (m_bStop)
	{
		//先接收包的大小 成功再接收报数据
		nRecv = recv(sockTalk, (char*) & nPackSize, sizeof(int), 0);
		if (nRecv > 0)
		{
			cout << "Service recv data_len succeed" << endl;
			//数据报的大小接收成功
			char* recvBuf = new char[nPackSize];

			while (nPackSize > 0)
			{
				//接收包内容
				nRecv = recv(sockTalk, recvBuf + nOffset, nPackSize, 0);
				if (nRecv > 0)
				{
					cout << "Service recv data succeed, nOffset == " <<nOffset<< endl;
					nOffset += nRecv;
					nPackSize -= nRecv;
				}
				else
				{
					cout << "Service recv data failed" << WSAGetLastError()<<endl;
					return;
				}
			}

			//把一个完整的包发送给中介者
			//m_pMediator->DealData(recvBuf, nOffset, sockTalk);
			//把数据拷贝到新的空间中
			char* packBuf = new char[nRecv];
			memcpy(packBuf, recvBuf, nRecv);
			//新的空间传给中介
			//cout << "Ready to send signal" << endl;
			m_pMediator->DealData(packBuf, nRecv, sockTalk);


			//给偏移量offset清零
			nOffset = 0;
		}
		else
		{
			cout << "Service recv data_len failed" << endl;
			break;
		}
	}
}

//关闭网络
void TcpService::UninitNet()
{
	//回收线程资源（关闭线程函数 回收句柄）
	m_bStop = false;
	for (auto ite = m_handleList.begin();ite != m_handleList.end();)
	{
		HANDLE h = *ite;
		if (h && WAIT_TIMEOUT == WaitForSingleObject(h, 500))//等待500ms 最好让线程自动退出 未退出强制杀死线程
		{
			TerminateThread(h, -1);
			//关闭句柄
			CloseHandle(h);
			h = NULL;
		}

		//删除无效map节点
		ite = m_handleList.erase(ite);
	}
	//关闭套接字
	if (m_sock && INVALID_SOCKET != m_sock)
	{
		closesocket(m_sock);
	}
	for (auto ite = m_mapThreadIdToSocket.begin(); ite != m_mapThreadIdToSocket.end();) {
		SOCKET sock_i = ite->second;
		if (sock_i && INVALID_SOCKET != sock_i)
		{
			closesocket(sock_i);
		}
		ite = m_mapThreadIdToSocket.erase(ite);
	}
	//卸载库
	WSACleanup();
}