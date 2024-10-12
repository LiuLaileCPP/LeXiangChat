#include"./TcpClientMediator.h"
#include"../inet/TcpClient.h"


TcpClientMediator::TcpClientMediator()
{
	m_pNet = new TcpClient(this);
}
TcpClientMediator::~TcpClientMediator()
{
	if (m_pNet)
	{
		delete m_pNet; m_pNet = nullptr;
	}
}
//������
bool TcpClientMediator::OpenNet()
{
	return m_pNet->InitINet();
}
//ת������
bool TcpClientMediator::SendData(char* sendData, int nLen, long lSend)
{
	return m_pNet->SendData(sendData, nLen, lSend);
}
//��������
void TcpClientMediator::DealData(char* recvData, int nLen, long lFrom)
{
	//���յ������ݴ���Kernel
	//ʹ���źźͲ۵Ļ���
	in_addr addr;
	addr.S_un.S_addr = lFrom;
	cout << "TCP ClientMediator RecvData: ip is " << inet_ntoa(addr) << " say: " << recvData << endl;
	//Q_EMIT SIG_ReadyData(recvData, nLen, lFrom);

}
//�ر�����
void TcpClientMediator::CloseNet()
{
	return m_pNet->UninitNet();
}

//�����ļ�
void TcpClientMediator::SendFile(char* fileId)
{

}

void TcpClientMediator::RefreshSocket(long)
{

}