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


//��ʼ������: ���ؿ� �����׽��� ��IP ���� ���߳�����Client 
bool TcpService::InitINet()
{
	//���ؿ�
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



	//�����׽���
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
	//u_long mode = 1; ioctlsocket(m_sock, FIONBIO, &mode); //����sockΪ������ģʽ


	//��IP
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



	



	//���̼߳��� ���ӿͻ���
	u_int id;
	HANDLE a_handle = (HANDLE)_beginthreadex(0//��ȫ���� 0��Ĭ��
		, 0 //��ջ��С 0��Ĭ��
		, &AcceptThread //��ʼ��ַ
		, this //�����б�
		, 0	//�߳�״̬ 0�Ǵ���������);
		, &id//������߳�ID
		);
	if (a_handle) {
		m_handleList.push_back(a_handle);
	}
	//���̷߳������ݱ� ˢ��mapsock
	u_int id2;
	HANDLE a_handle2 = (HANDLE)_beginthreadex(0//��ȫ���� 0��Ĭ��
		, 0 //��ջ��С 0��Ĭ��
		, &ReConnectSock //��ʼ��ַ
		, this //�����б�
		, 0	//�߳�״̬ 0�Ǵ���������);
		, &id2//������߳�ID
	);
	if (a_handle2) {
		m_handleList.push_back(a_handle2);
	}

	//

	//
	return true;
}


//���� �������ӵ��̺߳���
unsigned __stdcall TcpService:: AcceptThread(void* lpVoid)
{
	TcpService* pThis = (TcpService*)lpVoid;

	
	
	//һֱ��������
	sockaddr_in addr; //��������
	int addr_len = sizeof(addr);
	
	u_int  p_id = 0;
	while (pThis->m_bStop)
	{
		SOCKET sockTalk = INVALID_SOCKET;

		//����
		cout << "Service is Listening......" << endl;
		int err = listen(pThis->m_sock, 10); //����10��socket
		if (SOCKET_ERROR == err)
		{
			cout << "Service Listen failed" << WSAGetLastError() << endl;
			continue;
		}
		else
		{
			cout << "Service Listen succeed" << endl;
		}


		//u_long mode = 1; ioctlsocket(sockTalk, FIONBIO, &mode); //����sockΪ������ģʽ
		sockTalk = accept(pThis->m_sock, (sockaddr*)&addr, &addr_len);
		cout << "��sockTalk : " << sockTalk << "��" << endl;
		if (SOCKET_ERROR == sockTalk)
		{
			cout << "Service SockTalk accept failed" <<WSAGetLastError()<< endl;
			continue;
		}
		else
		{
			cout << "Service SockTalk accept succeed IP:"<<inet_ntoa(addr.sin_addr) <<" ServeSock : "<<sockTalk<< endl;
		}

		//�����������ݵ��߳�
		HANDLE handle = (HANDLE)_beginthreadex(0//��ȫ���� 0��Ĭ��
			, 0           //��ջ��С 0��Ĭ��
			, &RecvThread //��ʼ��ַ
			, pThis       //�����б�
			, 0	          //�߳�״̬ 0�Ǵ���������);
			, &p_id       //������߳�ID
		);
		//����������ݵ��߳̾��
		if (handle)
		{
			pThis->m_handleList.push_back(handle);
		}
		//<ͨ��socket�ͽ������ݵ��߳�ID>�� ���뵽map��
		pThis->m_mapThreadIdToSocket[p_id] = sockTalk;
	}
	return 1;
}

//����sock ˢ��m_mapThreadIdToSocket
unsigned __stdcall TcpService::ReConnectSock(void* lpVoid)
{
	TcpService* p = (TcpService*)lpVoid;
	map<unsigned int, SOCKET>::iterator ite = p->m_mapThreadIdToSocket.begin();
	int sendMsg = -1;
	while (ite != p->m_mapThreadIdToSocket.end())
	{
		if (p->SendData((char*)sendMsg,sizeof(int),ite->second) <= 0)
		{
			//�ر��׽��� �߳�
			//�ȴ��̵߳�while�ж�
			//if (m_handle && WAIT_TIMEOUT == WaitForSingleObject(m_handle, 500/*ms*/))
			//{
			//	//�ȴ�500msû���˳�ѭ����ǿ��ɱ���߳�
			//	TerminateThread(m_handle, -1);

			//	//�رվ��
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


//�������ݰ����̺߳���������RecvData()��
unsigned __stdcall TcpService:: RecvThread(void* lpVoid)
{
	TcpService* pThis = (TcpService*)lpVoid;
	pThis->RecvData();
	return 0;
}




//��������(���͵�ַ��ô������ UDP:ip TCP:talksock)
bool TcpService::SendData(char* sendData, int nLen, long lSend) //TCP��lSend��socket
{
	//У������Ϸ���
	if (!sendData || nLen == 0)
	{
		cout << "Service SendData ���� wrong" << endl;
		return false;
	}

	//���ճ������---�ȷ�����С �ٷ�������
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




//��������
void TcpService::RecvData()
{
	//����һ�ᣬ���̼߳��ص�map��������
	Sleep(100);
	
	//��ȡ��ǰ�̶߳�Ӧ��socket
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
	int nOffset = 0; //�����ƫ����
	while (m_bStop)
	{
		//�Ƚ��հ��Ĵ�С �ɹ��ٽ��ձ�����
		nRecv = recv(sockTalk, (char*) & nPackSize, sizeof(int), 0);
		if (nRecv > 0)
		{
			//cout << "Service recv data_len succeed,nLen : " <<nPackSize<<" socket: "<<sockTalk << endl;
			//���ݱ��Ĵ�С���ճɹ�
			char* recvBuf = new char[nPackSize];

			while (nPackSize > 0)
			{
				//���հ�����
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
			//��һ�������İ����͸��н���
			//m_pMediator->DealData(recvBuf, nOffset, sockTalk);
			//�����ݿ������µĿռ���
			char* packBuf = new char[nRecv];
			memcpy(packBuf, recvBuf, nRecv);
			//�µĿռ䴫���н�
			//cout << "Ready to send signal" << endl;
			int fileType = *(int*)packBuf;
			if (fileType == _DEF_PROTOCOL_FILE_INFO_RQ)       //�����ļ�������
			{
				DealFileInfoRq(packBuf,nOffset);
			}
			else if (fileType == _DEF_PROTOCOL_FILE_BLOCK_RQ) //�ļ���
			{
				DealFileBlockRq(packBuf, nOffset);
			}
			else m_pMediator->DealData(packBuf, nRecv, sockTalk);


			//��ƫ����offset����
			nOffset = 0;
		}
		else
		{
			cout << "Service recv data_len failed" << endl;
			break;
		}
	}
}

//�ر�����
void TcpService::UninitNet()
{
	//�����߳���Դ���ر��̺߳��� ���վ����
	m_bStop = false;
	for (auto ite = m_handleList.begin();ite != m_handleList.end();)
	{
		HANDLE h = *ite;
		if (h && WAIT_TIMEOUT == WaitForSingleObject(h, 500))//�ȴ�500ms ������߳��Զ��˳� δ�˳�ǿ��ɱ���߳�
		{
			TerminateThread(h, -1);
			//�رվ��
			CloseHandle(h);
			h = NULL;
		}

		//ɾ����Чmap�ڵ�
		ite = m_handleList.erase(ite);
	}
	//�ر��׽���
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
	//ж�ؿ�
	WSACleanup();
}

//�ļ�����: 
void TcpService::SendData(SOCKET sock, char* buf, int nLen)
{
	if (sock == INVALID_SOCKET)
	{
		return;
	}
	//�ȷ�����С
	int len = nLen;
	send(sock, (char*)&len, sizeof(int), 0);
	//�ٷ�������
	send(sock, buf, nLen, 0);
}

void TcpService::SendFile(char* fileId)
{
	if (m_sock == INVALID_SOCKET)
	{
		return;
	}
	// 1 ��ȡ�ļ���Ϣ
	//���ļ�������
	//char FIEL_PATH[1024] = "";
	//OPENFILENAMEA file = { 0 };
	//file.lStructSize = sizeof(file);
	//file.lpstrFilter = "�����ļ�(*.*)\0*.*\0�ı�(*.txt)\0*.txt\0"; //������ʽ �ļ���������
	//file.lpstrFile = FIEL_PATH; //�����ļ�����·��
	//file.nMaxFile = sizeof(FIEL_PATH) / sizeof(FIEL_PATH[0]); //�ռ���װ�����ַ���
	//file.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	//BOOL flag = GetOpenFileNameA(&file);
	//if (FALSE == flag)
	//{
	//	return;
	//}
	//cout << file.lpstrFile;


	//�����������ļ������

	// 2 �����ļ���Ϣ����
	STRU_FILE_INFO_RQ rq;
	//ȡ��file.lpstrFile�е��ļ���
	/*string name = getName(file.lpstrFile);
	cout << name << endl;
	strcpy_s(rq.szFileName, _MAX_FILE_PATH, name.c_str());*/

	//��ȡϵͳʱ��
	/*SYSTEMTIME time;
	GetLocalTime(&time);
	sprintf(rq.szFileId, "%s_%02d_%02d_%02d", rq.szFileName, time.wMinute, time.wSecond, time.wMilliseconds);
	cout << rq.szFileId << endl;*/

	char FILE_PATH[1024] = "D:\\Learn\\Qt\\TCP_IMSystem\\TCP_IMSystem\\FileCache\\";
	strcat(FILE_PATH, fileId);
	strcpy(rq.szFileId,fileId);
	//��ȡ�ļ���С
	FILE* pFile;
	fopen_s(&pFile, FILE_PATH, "rb");
	fseek(pFile, 0, SEEK_END); //�ļ�����ƶ����ļ�ĩβ���ܼ����ļ���С
	rq.szFileSize = _ftelli64(pFile); //����64λOS���ļ���С
	cout << rq.szFileSize << endl;

	fseek(pFile, 0, SEEK_SET);
	SendData(m_sock, (char*)&rq, sizeof(rq));


	// 3 �����ļ���
	STRU_FILE_BLOCK_RQ blockRq;
	long long nPos = 0; //�����Ǹ��ֽ���
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
			cout << "��ת���ļ��ɹ��� fileId: "<<rq.szFileId<<"��" << endl;

			//������Ϣ��Kernel��ʾĳ���ļ�ת���ɹ���
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
		return string();//���ؿմ�
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
	// ���Э��ͷ
	int type = *(int*)recvData;
	// ���ദ��
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
	// ���
	STRU_FILE_INFO_RQ* rq = (STRU_FILE_INFO_RQ*)recvData;



	// ���ļ�������� ѡ��洢·��
	char FILE_PATH[1024] = "D:\\Learn\\Qt\\TCP_IMSystem\\TCP_IMSystem\\FileCache\\";
	//OPENFILENAMEA file = { 0 };
	//file.lStructSize = sizeof(file);
	//file.lpstrFilter = "�����ļ�(*.*)\0*.*\0"; //������ʽ �ļ���������
	//file.lpstrFile = FIEL_PATH; //�����ļ�����·��
	//file.nMaxFile = sizeof(FIEL_PATH) / sizeof(FIEL_PATH[0]); //�ռ���װ�����ַ���
	//file.Flags = OFN_EXPLORER;
	/*BOOL flag = GetSaveFileNameA(&file);*/
	//�Զ����ļ������·������
	//strcpy(file.lpstrFile, rq->szFileName);
	//if (flag == false)
	//{
		//�����������ļ������ Ĭ�ϱ���
		//strcpy(file.lpstrFile, rq->szFileName); //���exeͬ����·��
	//}
	//cout << file.lpstrFile << endl;

	strcat(FILE_PATH, rq->szFileId);
	// ���ļ���Ϣ���浽STRU_FILE_RQ
	STRU_FILE_INFO* info = new STRU_FILE_INFO;
	info->nFileSize = rq->szFileSize;
	strcpy(info->szFilePath, FILE_PATH);
	strcpy(info->szFileId, rq->szFileId);
	strcpy(info->szFileName, rq->szFileName);
	fopen_s(&info->pFile, info->szFilePath, "wb");

	// �洢�ļ���Ϣ
	if (m_mapIdToFileInfo.count(info->szFileId) <= 0)
		m_mapIdToFileInfo[info->szFileId] = info;
}

void TcpService::DealFileBlockRq(char* recvData, int nLen)
{
	// ���
	STRU_FILE_BLOCK_RQ* rq = (STRU_FILE_BLOCK_RQ*)recvData;

	// ����ID�ҵ�map�ж�Ӧ���ļ���Ϣ
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
		cout << "id : " << rq->szFileId << "δ�ҵ��ļ���Ϣ" << endl;
		return;
	}
	// д�ļ�
	int nWrite = fwrite(rq->szFileContent, 1, rq->nBlockSize, pFile);
	info->nPos += nWrite;
	// �ж��ļ��Ƿ�д�����
	if (info->nPos >= rq->nBlockSize)
	{
		cout << "���ļ����ճɹ� fileId: " << rq->szFileId << "��" << endl;
		//�ر��ļ�
		fclose(info->pFile);
		//��map��ɾ���ڵ�
		m_mapIdToFileInfo.erase(ite);
		//�ͷ�new������info
		delete info;
		info = nullptr;

		//д��ɹ�����Kernelһ����Ϣ �ĸ��ļ�д��ɹ�
		STRU_FILE_RECV_ACC_MSG* msg = new STRU_FILE_RECV_ACC_MSG;
		strcpy(msg->szFileId, rq->szFileId);
		m_pMediator->DealData((char*)msg,sizeof(*msg),0);
	}
	else
	{
		cout << "id ��" << rq->szFileId << " δ��ȫд��" << endl;
	}

}
