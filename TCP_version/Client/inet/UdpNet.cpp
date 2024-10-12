#include"UdpNet.h"
#include"INet.h"
#include"UdpNetMediator.h"
using namespace std;

extern char*GetIpList();
UdpNet::UdpNet(UdpNetMediator* p_UdpNetMediator):m_sock(INVALID_SOCKET),m_handle(nullptr)
,bStopRecv(false)
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
    //pthread_t t_id;
	m_handle = (HANDLE)_beginthreadex(0//安全级别 0是默认
        , 0             //堆栈大小 0是默认
        , &RecvThread   //起始地址
        ,  this         //参数列表  --转换为void*并成为上一个参数(函数指针)的参数
        , 0             //线程状态 0是创建即运行);
        , 0);           //分配的线程ID

	if (!m_handle)
	{
		cout << "_beginthreadex failed" << endl;
		return false;
	}
	else {
		cout << "_beginthreadex succeed" << endl;
	}


    m_psetLongIpList = GetIpList();
	return true;
}

//接收数据的线程函数     //接收数据的线程函数（调用RecvData()） //x86要加上OS调用约定关键字__stdcall
unsigned __stdcall UdpNet::RecvThread(void* lpVoid)
{

    UdpNet* p = (UdpNet*)lpVoid;
    cout<<"Pthread is ready to recv"<<endl;
    p->RecvData();
    //Sleep(5000);


	return 0;
}


//发送数据(发送地址怎么决定？ UDP: ip      TCP: talksock)
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
        cout << "Send succeed " <<"SendData: "<<sendData<<" nLen: "<<nLen<< endl;
	}

    //bStopRecv = false;
	return true;
}

//接收数据
void UdpNet::RecvData()
{
	int nRecvNum = 0;
	char rBuf[4096] = "";
	sockaddr_in addr;
	int addrLen = sizeof(addr);
    cout<<"UdpNet::RecvData"<<endl;
    while (!bStopRecv)
	{
        if(1)
        {
            //cout<<"Pthread is recving"<<endl;
            nRecvNum = recvfrom(m_sock, rBuf, sizeof(rBuf), 0, (sockaddr*)&addr, &addrLen);
            //cout<<"nRecvNum is "<<nRecvNum<<endl;
            //cout<<"rBuf is "<<rBuf<<endl;
            if (nRecvNum <= 0)
            {
                //cout << "UdpNet::RecvData failed" << endl;
                //break;

            }
            else
            {
                //把数据拷贝到新的空间中
                char* packBuf = new char[nRecvNum];
                memcpy(packBuf,rBuf,nRecvNum);
                //新的空间传给中介
                //cout<<"Ready to send signal"<<endl;
                m_pMediator->DealData(packBuf,nRecvNum,addr.sin_addr.S_un.S_addr);
            }
        }
    }
     cout<<"Exit UdpNet::RecvData"<<endl;
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

//sock连接网络
bool UdpNet::ConnectNet()
{
    return true;
}

const SOCKET UdpNet::getSock()
{
    return m_sock;
}

//文件发送:
void UdpNet::SendData(SOCKET sock,char* buf,int nLen)
{
    if(sock == INVALID_SOCKET)
    {
        return;
    }
    //先发包大小
    int len = nLen;
    send(sock,(char*)&len,sizeof(int),0);
    //再发包内容
    send(sock,buf,nLen,0);
}

//发送文件
void UdpNet::SendFile(char* fileId)
{
    if(m_sock == INVALID_SOCKET)
    {
        return;
    }
    // 1 读取文件信息
    //打开文件管理器
    char FIEL_PATH[1024] = "";
    OPENFILENAMEA file = { 0 };
    file.lStructSize = sizeof(file);
    file.lpstrFilter = "所有文件(*.*)\0*.*\0文本(*.txt)\0*.txt\0"; //正则表达式 文件名过滤器
    file.lpstrFile = FIEL_PATH; //保存文件完整路径
    file.nMaxFile = sizeof(FIEL_PATH) / sizeof(FIEL_PATH[0]); //空间能装多少字符数
    file.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST ;
    QString::getOpenFileName (
        0, // parent，用于指定父组件。
        "请选择", // caption，是对话框的标题；
        const QString & dir = QString(), //dir，是对话框显示时默认打开的目录
        const QString & filter = QString(), // filter，是对话框的后缀名过滤器
        QString * selectedFilter = 0, // selectedFilter，是默认选择的过滤器
        Options options = 0 ) // options，是对话框的一些参数设定
    BOOL flag = GetOpenFileName();
    // if (FALSE == flag)
    // {
    //     return;
    // }
    cout << file.lpstrFile;

    // 2 发送文件信息请求
    STRU_FILE_INFO_RQ rq;
    //取出file.lpstrFile中的文件名
    string name = getName(file.lpstrFile);
    cout << name << endl;
    strcpy_s(rq.szFileName, _MAX_FILE_PATH, name.c_str());

    //获取系统时间
    SYSTEMTIME time;
    GetLocalTime(&time);
    sprintf(rq.szFileId, "%s_%02d_%02d_%02d", rq.szFileName, time.wMinute, time.wSecond, time.wMilliseconds);
    cout << rq.szFileId << endl;
    if(fileId) strcpy(fileId,rq.szFileId);

    //获取文件大小
    FILE* pFile = nullptr;
    fopen_s(&pFile, file.lpstrFile, "rb");
    fseek(pFile,0,SEEK_END); //文件光标移动到文件末尾才能计算文件大小
    rq.szFileSize = _ftelli64(pFile); //计算64位OS的文件大小
    cout << rq.szFileSize << endl;

    fseek(pFile, 0, SEEK_SET);
    SendData(m_sock, (char*)&rq, sizeof(rq));


    // 3 发送文件块
    STRU_FILE_BLOCK_RQ blockRq;
    long long nPos = 0; //读到那个字节了
    while (true)
    {
        int nRealRead = fread(blockRq.szFileContent, 1, _MAX_FILE_CONTENT_SIZE, pFile);
        nPos += nRealRead;
        blockRq.nBlockSize = nRealRead;
        strcpy(blockRq.szFileId, rq.szFileId);

        SendData(m_sock, (char*)&blockRq, sizeof(blockRq));
        if (nPos >= rq.szFileSize)
        {
            fclose(pFile);
            break;
        }
    }
    Sleep(20000);
}

string UdpNet::getName(char* str)
{
    if (str == nullptr || strlen(str) == 0)
    {
        return string();//返回空串
    }
    for (int i = strlen(str) - 1; i >= 0;i--)
    {
        if (str[i] == '\\' || str[i] == '/')
        {
            string res = str + i + 1;
            return res;
        }
    }
    return string();
}

//文件接收：
void UdpNet::DealData(char* recvData, int nLen)
{
    // 拆出协议头
    int type = *(int*)recvData;
    // 分类处理
    switch (type)
    {
    case _DEF_PROTOCOL_FILE_INFO_RQ:
        DealFileInfoRq(recvData, nLen);
        break;
    case _DEF_PROTOCOL_FILE_BLOCK_RQ:
        DealFileBlockRq(recvData, nLen);
        break;
    default:
        break;
    }
}


void UdpNet::DealFileInfoRq(char* recvData, int nLen)
{
    // 拆包
    STRU_FILE_INFO_RQ* rq = (STRU_FILE_INFO_RQ*)recvData;



    // 打开文件浏览窗口 选择存储路径
    char FIEL_PATH[1024] = "";
    OPENFILENAMEA file = { 0 };
    file.lStructSize = sizeof(file);
    file.lpstrFilter = "所有文件(*.*)\0*.*\0"; //正则表达式 文件名过滤器
    file.lpstrFile = FIEL_PATH; //保存文件完整路径
    file.nMaxFile = sizeof(FIEL_PATH) / sizeof(FIEL_PATH[0]); //空间能装多少字符数
    file.Flags = OFN_EXPLORER;
    /*BOOL flag = GetSaveFileNameA(&file);*/
    //自定义文件保存的路径名字
    strcpy(file.lpstrFile, rq->szFileName);
    if (/*flag == false*/ 1 )
    {
        //服务器不打开文件浏览器 默认保存
        strcpy(file.lpstrFile, rq->szFileName); //相对exe同级的路径
    }
    cout << file.lpstrFile << endl;


    // 把文件信息保存到STRU_FILE_RQ
    STRU_FILE_INFO* info = new STRU_FILE_INFO;
    info->nFileSize = rq->szFileSize;
    strcpy(info->szFilePath, file.lpstrFile);
    strcpy(info->szFileId, rq->szFileId);
    strcpy(info->szFileName, rq->szFileName);
    fopen_s(&info->pFile, info->szFilePath, "wb");

    // 存储文件信息
    if (m_mapIdToFileInfo.count(info->szFileId) <= 0)
        m_mapIdToFileInfo[info->szFileId] = info;
}

void UdpNet::DealFileBlockRq(char* recvData, int nLen)
{
    // 拆包
    STRU_FILE_BLOCK_RQ* rq = (STRU_FILE_BLOCK_RQ*)recvData;

    // 根据ID找到map中对应的文件信息
    STRU_FILE_INFO* info;
    FILE* pFile = nullptr;
    string path = "";
    auto ite = m_mapIdToFileInfo.find(rq->szFileId);
    if (ite != m_mapIdToFileInfo.end())
    {
        path = ite->second->szFilePath;
        pFile = ite->second->pFile;
        info = ite->second;
    }
    else
    {
        cout << "id : " << rq->szFileId << "未找到文件信息" << endl;
        return;
    }
    // 写文件
    int nWrite = fwrite(rq->szFileContent, 1, rq->nBlockSize, pFile);
    info->nPos += nWrite;
    // 判断文件是否写入完成
    STRU_RECV_FILE_RES *res = new STRU_RECV_FILE_RES;
    if (info->nPos >= rq->nBlockSize)
    {
        cout << "id ： " << rq->szFileId << " 写入成功" << endl;
        //发送Kernel一个接收文件成功的信息
        res->recvRes = recv_file_succeed;
        m_pMediator->DealData((char*)res,sizeof(*res),0);

        //关闭文件
        fclose(info->pFile);
        //从map中删除节点
        m_mapIdToFileInfo.erase(ite);
        //释放new出来的info
        delete info;
        info = nullptr;
    }
    else
    {
        //发送Kernel一个接收文件失败的信息
        res->recvRes = recv_file_failed;
        m_pMediator->DealData((char*)res,sizeof(*res),0);
        cout << "id ：" << rq->szFileId << " 未完全写入" << endl;
    }

}




























