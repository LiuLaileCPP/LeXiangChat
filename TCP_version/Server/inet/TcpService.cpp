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
	//u_long mode = 1; ioctlsocket(m_sock, FIONBIO, &mode); //设置sock为非阻塞模式


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



	



	//多线程监听 连接客户端
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
	//多线程发送数据报 刷新mapsock
	u_int id2;
	HANDLE a_handle2 = (HANDLE)_beginthreadex(0//安全级别 0是默认
		, 0 //堆栈大小 0是默认
		, &ReConnectSock //起始地址
		, this //参数列表
		, 0	//线程状态 0是创建即运行);
		, &id2//分配的线程ID
	);
	if (a_handle2) {
		m_handleList.push_back(a_handle2);
	}

	//

	//
	return true;
}


//监听 接收连接的线程函数
unsigned __stdcall TcpService:: AcceptThread(void* lpVoid)
{
	TcpService* pThis = (TcpService*)lpVoid;

	
	
	//一直接收连接
	sockaddr_in addr; //传出参数
	int addr_len = sizeof(addr);
	
	u_int  p_id = 0;
	while (pThis->m_bStop)
	{
		SOCKET sockTalk = INVALID_SOCKET;

		//监听
		cout << "Service is Listening......" << endl;
		int err = listen(pThis->m_sock, 10); //监听10个socket
		if (SOCKET_ERROR == err)
		{
			cout << "Service Listen failed" << WSAGetLastError() << endl;
			continue;
		}
		else
		{
			cout << "Service Listen succeed" << endl;
		}


		//u_long mode = 1; ioctlsocket(sockTalk, FIONBIO, &mode); //设置sock为非阻塞模式
		sockTalk = accept(pThis->m_sock, (sockaddr*)&addr, &addr_len);
		cout << "【sockTalk : " << sockTalk << "】" << endl;
		if (SOCKET_ERROR == sockTalk)
		{
			cout << "Service SockTalk accept failed" <<WSAGetLastError()<< endl;
			continue;
		}
		else
		{
			cout << "Service SockTalk accept succeed IP:"<<inet_ntoa(addr.sin_addr) <<" ServeSock : "<<sockTalk<< endl;
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

//重连sock 刷新m_mapThreadIdToSocket
unsigned __stdcall TcpService::ReConnectSock(void* lpVoid)
{
	TcpService* p = (TcpService*)lpVoid;
	map<unsigned int, SOCKET>::iterator ite = p->m_mapThreadIdToSocket.begin();
	int sendMsg = -1;
	while (ite != p->m_mapThreadIdToSocket.end())
	{
		if (p->SendData((char*)sendMsg,sizeof(int),ite->second) <= 0)
		{
			//关闭套接字 线程
			//等待线程到while判断
			//if (m_handle && WAIT_TIMEOUT == WaitForSingleObject(m_handle, 500/*ms*/))
			//{
			//	//等待500ms没有退出循环就强制杀死线程
			//	TerminateThread(m_handle, -1);

			//	//关闭句柄
			//	CloseHandle(m_handle);
			//	m_handle = NULL;
			//}
			closesocket(ite->second);
			ite = p->m_mapThreadIdToSocket.erase(ite);
		}
		else
		{
			ite++;
		}
	}
	return 0;
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
		cout << "Service send data_len failed : " << WSAGetLastError()<<" len : "<< nLen << " sock: " << lSend << endl;
		return false;
	}
	else
	{
		//cout << "Service send data_len succeed" <<" sock: " << lSend<< endl;
	}
	if (send(lSend,sendData,nLen,0 ) <= 0)
	{
		cout << "Service send data failed : " << WSAGetLastError() <<" data : " <<sendData<<" sock: "<<lSend << endl;
		return false;
	}
	else
	{
		//cout << "Service send data succeed"<<" sock: " << lSend<< endl;
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
			//cout << "Service recv data_len succeed,nLen : " <<nPackSize<<" socket: "<<sockTalk << endl;
			//数据报的大小接收成功
			char* recvBuf = new char[nPackSize];

			while (nPackSize > 0)
			{
				//接收包内容
				nRecv = recv(sockTalk, recvBuf + nOffset, nPackSize, 0);
				if (nRecv > 0)
				{
					//cout << "Service recv data succeed, nOffset == " <<nOffset<< endl;
					nOffset += nRecv;
					nPackSize -= nRecv;
				}
				else
				{
					cout << "Service recv data failed" << WSAGetLastError()<<endl;
					return;
				}
			}
			//cout << "Service recv data succeed, data : " << recvBuf << " socket: " << sockTalk << endl;
			//把一个完整的包发送给中介者
			//m_pMediator->DealData(recvBuf, nOffset, sockTalk);
			//把数据拷贝到新的空间中
			char* packBuf = new char[nRecv];
			memcpy(packBuf, recvBuf, nRecv);
			//新的空间传给中介
			//cout << "Ready to send signal" << endl;
			int fileType = *(int*)packBuf;
			if (fileType == _DEF_PROTOCOL_FILE_INFO_RQ)       //发送文件的请求
			{
				DealFileInfoRq(packBuf,nOffset);
			}
			else if (fileType == _DEF_PROTOCOL_FILE_BLOCK_RQ) //文件块
			{
				DealFileBlockRq(packBuf, nOffset);
			}
			else m_pMediator->DealData(packBuf, nRecv, sockTalk);


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

//文件发送: 
void TcpService::SendData(SOCKET sock, char* buf, int nLen)
{
	if (sock == INVALID_SOCKET)
	{
		return;
	}
	//先发包大小
	int len = nLen;
	send(sock, (char*)&len, sizeof(int), 0);
	//再发包内容
	send(sock, buf, nLen, 0);
}

void TcpService::SendFile(char* fileId)
{
	if (m_sock == INVALID_SOCKET)
	{
		return;
	}
	// 1 读取文件信息
	//打开文件管理器
	//char FIEL_PATH[1024] = "";
	//OPENFILENAMEA file = { 0 };
	//file.lStructSize = sizeof(file);
	//file.lpstrFilter = "所有文件(*.*)\0*.*\0文本(*.txt)\0*.txt\0"; //正则表达式 文件名过滤器
	//file.lpstrFile = FIEL_PATH; //保存文件完整路径
	//file.nMaxFile = sizeof(FIEL_PATH) / sizeof(FIEL_PATH[0]); //空间能装多少字符数
	//file.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	//BOOL flag = GetOpenFileNameA(&file);
	//if (FALSE == flag)
	//{
	//	return;
	//}
	//cout << file.lpstrFile;


	//服务器不打开文件浏览器

	// 2 发送文件信息请求
	STRU_FILE_INFO_RQ rq;
	//取出file.lpstrFile中的文件名
	/*string name = getName(file.lpstrFile);
	cout << name << endl;
	strcpy_s(rq.szFileName, _MAX_FILE_PATH, name.c_str());*/

	//获取系统时间
	/*SYSTEMTIME time;
	GetLocalTime(&time);
	sprintf(rq.szFileId, "%s_%02d_%02d_%02d", rq.szFileName, time.wMinute, time.wSecond, time.wMilliseconds);
	cout << rq.szFileId << endl;*/

	char FILE_PATH[1024] = "D:\\Learn\\Qt\\TCP_IMSystem\\TCP_IMSystem\\FileCache\\";
	strcat(FILE_PATH, fileId);
	strcpy(rq.szFileId,fileId);
	//获取文件大小
	FILE* pFile;
	fopen_s(&pFile, FILE_PATH, "rb");
	fseek(pFile, 0, SEEK_END); //文件光标移动到文件末尾才能计算文件大小
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
			cout << "【转发文件成功了 fileId: "<<rq.szFileId<<"】" << endl;

			//发送消息给Kernel表示某个文件转发成功了
			STRU_FILE_RESEND_ACC_MSG* msg = new STRU_FILE_RESEND_ACC_MSG;
			strcpy(msg->szFileId, fileId);
			m_pMediator->DealData((char*)msg,sizeof(*msg),0);
			break;
		}
	}
	
}

string TcpService::getName(char* str)
{
	if (str == nullptr || strlen(str) == 0)
	{
		return string();//返回空串
	}
	for (int i = strlen(str) - 1; i >= 0; i--)
	{
		if (str[i] == '\\' || str[i] == '/')
		{
			string res = str + i + 1;
			return res;
		}
	}
	return string();
}


void TcpService::DealData(char* recvData, int nLen)
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

void TcpService::RefreshSocket(SOCKET sock)
{
	m_sock = sock;
}

void TcpService::DealFileInfoRq(char* recvData, int nLen)
{
	// 拆包
	STRU_FILE_INFO_RQ* rq = (STRU_FILE_INFO_RQ*)recvData;



	// 打开文件浏览窗口 选择存储路径
	char FILE_PATH[1024] = "D:\\Learn\\Qt\\TCP_IMSystem\\TCP_IMSystem\\FileCache\\";
	//OPENFILENAMEA file = { 0 };
	//file.lStructSize = sizeof(file);
	//file.lpstrFilter = "所有文件(*.*)\0*.*\0"; //正则表达式 文件名过滤器
	//file.lpstrFile = FIEL_PATH; //保存文件完整路径
	//file.nMaxFile = sizeof(FIEL_PATH) / sizeof(FIEL_PATH[0]); //空间能装多少字符数
	//file.Flags = OFN_EXPLORER;
	/*BOOL flag = GetSaveFileNameA(&file);*/
	//自定义文件保存的路径名字
	//strcpy(file.lpstrFile, rq->szFileName);
	//if (flag == false)
	//{
		//服务器不打开文件浏览器 默认保存
		//strcpy(file.lpstrFile, rq->szFileName); //相对exe同级的路径
	//}
	//cout << file.lpstrFile << endl;

	strcat(FILE_PATH, rq->szFileId);
	// 把文件信息保存到STRU_FILE_RQ
	STRU_FILE_INFO* info = new STRU_FILE_INFO;
	info->nFileSize = rq->szFileSize;
	strcpy(info->szFilePath, FILE_PATH);
	strcpy(info->szFileId, rq->szFileId);
	strcpy(info->szFileName, rq->szFileName);
	fopen_s(&info->pFile, info->szFilePath, "wb");

	// 存储文件信息
	if (m_mapIdToFileInfo.count(info->szFileId) <= 0)
		m_mapIdToFileInfo[info->szFileId] = info;
}

void TcpService::DealFileBlockRq(char* recvData, int nLen)
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
	if (info->nPos >= rq->nBlockSize)
	{
		cout << "【文件接收成功 fileId: " << rq->szFileId << "】" << endl;
		//关闭文件
		fclose(info->pFile);
		//从map中删除节点
		m_mapIdToFileInfo.erase(ite);
		//释放new出来的info
		delete info;
		info = nullptr;

		//写入成功发给Kernel一个消息 哪个文件写入成功
		STRU_FILE_RECV_ACC_MSG* msg = new STRU_FILE_RECV_ACC_MSG;
		strcpy(msg->szFileId, rq->szFileId);
		m_pMediator->DealData((char*)msg,sizeof(*msg),0);
	}
	else
	{
		cout << "id ：" << rq->szFileId << " 未完全写入" << endl;
	}

}
