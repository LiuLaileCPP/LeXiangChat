#include"./TcpServiceMediator.h"
#include"../inet/TcpService.h"
#include"../CKernel.h"


TcpServiceMediator::TcpServiceMediator()
{
	m_pNet = new TcpService(this);
}
TcpServiceMediator::~TcpServiceMediator()
{
	if (m_pNet)
	{
		delete m_pNet; m_pNet = nullptr;
	}
}
//������
bool TcpServiceMediator::OpenNet()
{

	return m_pNet->InitINet();
}
//ת������
bool TcpServiceMediator::SendData(char* sendData, int nLen, long lSend)
{
	return m_pNet->SendData(sendData, nLen, lSend);
}
//��������
void TcpServiceMediator::DealData(char* recvData, int nLen, long lFrom)
{
	//���յ������ݴ���Kernel
	
	//ʹ���źźͲ۵Ļ���
	//in_addr addr;
	//addr.S_un.S_addr = lFrom;
	//
	//cout << "TCP ServiceMediator RecvData: ip is " << inet_ntoa(addr) << " say: " << recvData << endl;
	////Q_EMIT SIG_ReadyData(recvData, nLen, lFrom);
	//char ch[100] = "TcpServiceMediator::DealData";
	//SendData(ch, sizeof("TcpServiceMediator::DealData"), lFrom);

	CKernel::m_pKernel->dealReadyData(recvData,nLen,lFrom);

}
//�ر�����
void TcpServiceMediator::CloseNet()
{
	return m_pNet->UninitNet();
}

//�����ļ�
void TcpServiceMediator::SendFile(char* fileId)
{
	return m_pNet->SendFile(fileId);
}

void TcpServiceMediator::RefreshSocket(long sock)
{
	return m_pNet->RefreshSocket(sock);
}