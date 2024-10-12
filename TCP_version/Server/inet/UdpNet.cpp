#include"UdpNet.h"
#include"../mediator/UdpNetMediator.h"

UdpNet::UdpNet(UdpNetMediator* p_UdpNetMediator):m_sock(INVALID_SOCKET),m_handle(nullptr)
,bStopRecv(true)
{
	m_pMediator =  p_UdpNetMediator;
}
UdpNet::~UdpNet()
{
	UninitNet();
}

//初始化网络---加载库 创建套接字 绑定ip 创建接收数据的线程
bool UdpNet::InitINet()
{
	//加载库
	WSADATA data = {};
	int err = WSAStartup(MAKEWORD(2, 2), &data);
	if (0 != err)
	{
		cout << "Wsastartup failed: " <<WSAGetLastError()<< endl;
		return false;
	}
	if (2 != HIBYTE(data.wVersion) || 2 != LOBYTE(data.wVersion))
	{
		cout << "Wsa's version is wrong" << endl;
		//WSACleanup();
		return false;
	}
	else {
		cout << "Wsastartup suceed" << endl;
	}




	//创建套接字
	m_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == m_sock)
	{
		cout << "Create scoket failed: " << WSAGetLastError() << endl;
		//WSACleanup();
		return false;
	}
	else {
		cout << "Create socket suceed" << endl;
	}
	u_long mode = 1; ioctlsocket(m_sock, FIONBIO, &mode); //设置sock为非阻塞模式




	//套接字绑定ip
	sockaddr_in addrService; 
	int nlenService = sizeof(addrService);
	addrService.sin_family = AF_INET;
	addrService.sin_port = htons(_UDP_PORT);
	addrService.sin_addr.S_un.S_addr = INADDR_ANY;//绑定本机ip
	err = bind(m_sock, (sockaddr*)&addrService, nlenService);

	if (SOCKET_ERROR == err)
	{
		cout << "IP bind failed" << WSAGetLastError() << endl;
		//closesocket(sock); WSACleanup();
		return false;
	}
	else {
		cout << "IP bind succeed" << endl;
	}




    //申请广播权限
    bool val = true;
    setsockopt(m_sock, SOL_SOCKET, SO_BROADCAST, (char*)&val, sizeof(val));





    //创建接收数据的线程--CreateThread， ExitThread(OS自动调用) I如果在线程中使用C++运行时库的函数(例如strcpy ExitThread),
	// 函数会申请空间并且自己不回收 造成内存泄漏
	//安全函数：_beginthreadex和_endthreadex是一对，_endthreadex会先回收内存空间，再调用ExitThread。
	pthread_t t_id;
	m_handle = (HANDLE)_beginthreadex(0//安全级别 0是默认
		, 0 //堆栈大小 0是默认
		, &RecvThread //起始地址
		,  this //参数列表  --转换为void*并成为上一个参数(函数指针)的参数
		, 0	//线程状态 0是创建即运行);
		, 0); //分配的线程ID

	if (!m_handle)
	{
		cout << "_beginthreadex failed" << endl;
		return false;
	}
	else {
		cout << "_beginthreadex succeed" << endl;
	}


	return true;
}

//接收数据的线程函数     //接收数据的线程函数（调用RecvData()） //x86要加上OS调用约定关键字__stdcall
unsigned __stdcall UdpNet::RecvThread(void* lpVoid)
{
    while(1)
    {
        UdpNet* p = (UdpNet*)lpVoid;
        cout<<"Pthread is ready to recv"<<endl;
        p->RecvData();
        //Sleep(5000);
    }

	return 0;
}


//发送数据(发送地址怎么决定？ UDP:ip TCP:talksock)
bool UdpNet::SendData(char* sendData, int nLen, long lSend)
{
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(_UDP_PORT);
	addr.sin_addr.S_un.S_addr = lSend;   //inet_addr((const char*)lSend);

	if (sendto(m_sock,sendData,nLen,0,(sockaddr*)&addr,sizeof(addr)) <= 0)
	{
		cout << "Send failed" << WSAGetLastError() << endl;
		return false;
	}
	else {
        cout << "Send succeed " <<"SendData: "<<sendData<< endl;
	}

    bStopRecv = false;
	return true;
}

//接收数据
void UdpNet::RecvData()
{
	int nRecvNum = 0;
	char rBuf[4096] = "";
	sockaddr_in addr;
	int addrLen = sizeof(addr);
	while (!bStopRecv)
	{
        nRecvNum = recvfrom(m_sock, rBuf, sizeof(rBuf), 0, (sockaddr*)&addr, &addrLen);
        cout<<"nRecvNum is "<<nRecvNum<<endl;
        cout<<"rBuf is "<<rBuf<<endl;
        if (nRecvNum <= 0)
		{
            cout << "UdpNet::RecvData failed" << endl;
            break;

		}
		else
		{
            //把数据拷贝到新的空间中
            char* packBuf = new char[nRecvNum];
            memcpy(packBuf,rBuf,nRecvNum);
            //新的空间传给中介
            cout<<"Ready to send signal"<<endl;
            m_pMediator->DealData(packBuf,nRecvNum,addr.sin_addr.S_un.S_addr);
		}
	}

}

//关闭网络
void UdpNet::UninitNet()
{
	//回收线程资源（回收内核对象，结束线程工作，关闭句柄）
	bStopRecv = true;

	//等待线程到while判断
	if (m_handle && WAIT_TIMEOUT == WaitForSingleObject(m_handle, 500/*ms*/))
	{
		//等待500ms没有退出循环就强制杀死线程
		TerminateThread(m_handle, -1);
		CloseHandle(m_handle); //关闭句柄
	}
		
	//关闭套接字
	if (m_sock && INVALID_SOCKET != m_sock)
	{
		closesocket(m_sock);
	}
	//卸载库
	WSACleanup();
}

//发送文件
void UdpNet::SendFile(char* fileId){}
string UdpNet::getName(char* str) { return string(); }
//接收
void UdpNet::DealFileBlockRq(char* recvData, int nLen){}
void UdpNet::DealFileInfoRq(char* recvData, int nLen){}
void UdpNet::DealData(char* recvData, int nLen){}

void UdpNet::RefreshSocket(SOCKET sock)
{
	m_sock = sock;
}

//处理文件的：
void UdpNet::SendData(SOCKET sock, char* buf, int nLen)
{

}
