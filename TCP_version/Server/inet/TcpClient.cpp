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


//��ʼ������----���ؿ� �����׽��� ���ӷ���� �����������ݵ��߳�
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


	// 3sock���ӷ����
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
		  0           //��ȫ���� 0��Ĭ��
		, 0           //��ջ��С 0��Ĭ��
		, &RecvThread //��ʼ��ַ
		, this        //�����б�
		, 0	          //�߳�״̬ 0�Ǵ���������);
		, &id         //������߳�ID
	);
	return true;
}


//��������(���͵�ַ��ô������ UDP:ip TCP:talksock)
bool TcpClient::SendData(char* sendData, int nLen, long lSend)
{
	//У������Ϸ���
	if (!sendData || nLen == 0)
	{
		cout << "Service SendData ���� wrong" << endl;
		return false;
	}

	//���ճ������---�ȷ�����С �ٷ�������
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
		cout << "Client SendData ���� wrong" << endl;
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


//��������
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

			//�����ݿ������µĿռ���
			char* packBuf = new char[nRecv];
			memcpy(packBuf, rBuf, nRecv);
			//�µĿռ䴫���н�
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

//�ر�����
void TcpClient::UninitNet()
{
	//�����߳���Դ�������ں˶��󣬽����̹߳������رվ����
	m_bStop = false;

	//�ȴ��̵߳�while�ж�
	if (m_handle && WAIT_TIMEOUT == WaitForSingleObject(m_handle, 500/*ms*/))
	{
		//�ȴ�500msû���˳�ѭ����ǿ��ɱ���߳�
		TerminateThread(m_handle, -1);
		
		//�رվ��
		CloseHandle(m_handle);
		m_handle = NULL;
	}

	// 2�ر��׽���
	if(m_sock && INVALID_SOCKET != m_sock)
	closesocket(m_sock);
	// 3ж�ؿ�
	WSACleanup();
}

//�����ļ�
void TcpClient::SendFile(char* fileId)
{

}
string TcpClient::getName(char* str)
{
	return string();
}
//����
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

//�����ļ��ģ�
void TcpClient::SendData(SOCKET sock, char* buf, int nLen)
{

}