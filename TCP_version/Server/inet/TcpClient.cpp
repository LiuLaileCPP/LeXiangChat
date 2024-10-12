#include"TcpClient.h"

TcpClient::TcpClient(TcpClientMediator* p_UdpNetMediator)
{
	m_bStop = true;
	m_handle = NULL;
	m_sock = INVALID_SOCKET;
	m_pMediator = p_UdpNetMediator;
}


TcpClient::~TcpClient()
{

}


//初始化网络----加载库 创建套接字 连接服务端 创建接受数据的线程
bool TcpClient::InitINet()
{
	// 1
	WSADATA data = {};
	int err = WSAStartup(MAKEWORD(2,2),&data);
	if (err != 0)
	{
		cout << "Client WSAStartup failed" << endl;
		return false;
	}
	else
	{
		cout << "Client WSAStartup succeed" << endl;
	}
	if (2 != HIBYTE(data.wVersion) || 2 != LOBYTE(data.wVersion))
	{
		cout << "Client WSA version wrong" << WSAGetLastError() << endl;
		return false;
	}
	else
	{
		cout << "Client WSA version right" << WSAGetLastError() << endl;
	}

	// 2
	m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == m_sock)
	{
		cout << "Client socket invalid" << endl;
		return false;
	}
	else
	{
		cout << "Client socket succeed" << endl;
	}


	// 3sock连接服务端
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(_TCP_PORT);
	addr.sin_addr.S_un.S_addr = inet_addr(_TCP_IP);
	int n_con = connect(m_sock, (sockaddr*)&addr, sizeof(addr));
	if (0 != n_con)
	{
		cout << "Client connect failed" << WSAGetLastError() << endl;
		return false;
	}
	else
	{
		cout << "Client connect succeed"  << endl;
	}

	

	// 4
	u_int id = 0;
	m_handle = (HANDLE)_beginthreadex(
		  0           //安全级别 0是默认
		, 0           //堆栈大小 0是默认
		, &RecvThread //起始地址
		, this        //参数列表
		, 0	          //线程状态 0是创建即运行);
		, &id         //分配的线程ID
	);
	return true;
}


//发送数据(发送地址怎么决定？ UDP:ip TCP:talksock)
bool TcpClient::SendData(char* sendData, int nLen, long lSend)
{
	//校验参数合法性
	if (!sendData || nLen == 0)
	{
		cout << "Service SendData 参数 wrong" << endl;
		return false;
	}

	//解决粘包问题---先发包大小 再发包内容
	if (send(m_sock, (char*)&nLen, sizeof(int), 0) <= 0)
	{
		cout << "Client send data_len failed" << WSAGetLastError() << endl;
		return false;
	}
	else {
		cout << "Client send data_len succeed" << endl;
	}
	if (send(m_sock, sendData, nLen, 0) <= 0)
	{
		cout << "Client send data failed" << WSAGetLastError() << endl;
		return false;
	}
	else
	{
		cout << "Client send data succeed" << endl;
	}
	return true;

	/*if (!sendData || nLen == 0)
	{
		cout << "Client SendData 参数 wrong" << endl;
		return false;
	}

	int nPackSize = nLen;
	int nOffset = 0;
	int nSend = send(lSend, (char*)nPackSize, sizeof(int), 0);
	if (nSend > 0)
	{
		cout << "Client send data_len succeed" << endl;
		while (nPackSize > 0)
		{
			nSend = send(lSend, sendData + nOffset, nPackSize, 0);
			if (nSend > 0)
			{
				cout << "Client send data succeed" << endl;
				nOffset += nSend;
				nPackSize -= nSend;
			}
			else
			{
				cout << "Client send data failed" << endl;
				return false;
			}
		}
		nOffset = 0;
	}
	else
	{
		cout<< "Client send data_len failed" << endl;
		return false;
	}
	return true;*/
}


unsigned __stdcall TcpClient::RecvThread(void* lpVoid)
{
	TcpClient* p = (TcpClient*)lpVoid;
	p->RecvData();
	return 0;
}


//接收数据
void TcpClient::RecvData()
{
	int nRecv = 0;
	char rBuf[4096] = "";
	sockaddr_in addr;
	int addrLen = sizeof(addr);

	int nPackSize = 0;
	int nOffset = 0;
	while (m_bStop)
	{
		nRecv = recv(m_sock, (char*)&nPackSize, sizeof(int), 0);
		if (nRecv > 0)
		{
			cout << "Client recv data_len succeed" << endl;
			while (nPackSize > 0)
			{
				nRecv = recv(m_sock, rBuf + nOffset, nPackSize, 0);
				if (nRecv <= 0)
				{
					cout << "Client recv data failed" << WSAGetLastError() << endl;
					return;
				}
				else
				{
					nOffset += nRecv;
					nPackSize -= nRecv;
				}
			}
			cout << "Client recv data succeed" << endl;

			//把数据拷贝到新的空间中
			char* packBuf = new char[nRecv];
			memcpy(packBuf, rBuf, nRecv);
			//新的空间传给中介
			//cout << "Ready to send signal" << endl;
			m_pMediator->DealData(packBuf, nRecv, m_sock);
		}
		else
		{
			cout << "Client recv data_len failed" << WSAGetLastError()<<endl;
			return;
		}
		nOffset = 0;
	}
}

//关闭网络
void TcpClient::UninitNet()
{
	//回收线程资源（回收内核对象，结束线程工作，关闭句柄）
	m_bStop = false;

	//等待线程到while判断
	if (m_handle && WAIT_TIMEOUT == WaitForSingleObject(m_handle, 500/*ms*/))
	{
		//等待500ms没有退出循环就强制杀死线程
		TerminateThread(m_handle, -1);
		
		//关闭句柄
		CloseHandle(m_handle);
		m_handle = NULL;
	}

	// 2关闭套接字
	if(m_sock && INVALID_SOCKET != m_sock)
	closesocket(m_sock);
	// 3卸载库
	WSACleanup();
}

//发送文件
void TcpClient::SendFile(char* fileId)
{

}
string TcpClient::getName(char* str)
{
	return string();
}
//接收
void TcpClient::DealFileBlockRq(char* recvData, int nLen)
{

}
void TcpClient::DealFileInfoRq(char* recvData, int nLen)
{

}
void TcpClient::DealData(char* recvData, int nLen)
{

}

void TcpClient::RefreshSocket(SOCKET sock)
{
	m_sock = sock;
}

//处理文件的：
void TcpClient::SendData(SOCKET sock, char* buf, int nLen)
{

}