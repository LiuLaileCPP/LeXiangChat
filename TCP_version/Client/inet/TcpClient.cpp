#include"TcpClient.h"
using namespace std;
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


    ConnectNet();

	return true;
}

//socket连接网络
bool TcpClient::ConnectNet()
{
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

const SOCKET TcpClient::getSock()
{
    return m_sock;
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


//文件发送:
void TcpClient::SendData(SOCKET sock,char* buf,int nLen)
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
void TcpClient::SendFile(char* fileId)
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
    //BOOL flag = GetOpenFileNameA(&file);
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

string TcpClient::getName(char* str)
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
void TcpClient::DealData(char* recvData, int nLen)
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


void TcpClient::DealFileInfoRq(char* recvData, int nLen)
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

void TcpClient::DealFileBlockRq(char* recvData, int nLen)
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






























