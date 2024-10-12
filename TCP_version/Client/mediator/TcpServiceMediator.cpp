#include"./TcpServiceMediator.h"
#include"../inet/TcpService.h"
#include"./CKernel/CKernel.h"

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
//打开网络
bool TcpServiceMediator::OpenNet()
{

	return m_pNet->InitINet();
}
//转发数据
bool TcpServiceMediator::SendData(char* sendData, int nLen, long lSend)
{
	return m_pNet->SendData(sendData, nLen, lSend);
}
//处理数据
void TcpServiceMediator::DealData(char* recvData, int nLen, long lFrom)
{
	//把收到的数据传给Kernel
	
	//使用信号和槽的机制
	//in_addr addr;
	//addr.S_un.S_addr = lFrom;
	//
	//cout << "TCP ServiceMediator RecvData: ip is " << inet_ntoa(addr) << " say: " << recvData << endl;
	////Q_EMIT SIG_ReadyData(recvData, nLen, lFrom);
	//char ch[100] = "TcpServiceMediator::DealData";
	//SendData(ch, sizeof("TcpServiceMediator::DealData"), lFrom);

	CKernel::m_pKernel->dealReadyData(recvData,nLen,lFrom);

}
//关闭网络
void TcpServiceMediator::CloseNet()
{
	return m_pNet->UninitNet();
}
