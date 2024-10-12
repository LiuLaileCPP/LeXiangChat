#include"./TcpClientMediator.h"
#include"../inet/TcpClient.h"
using namespace std;

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
//打开网络
bool TcpClientMediator::OpenNet()
{
	return m_pNet->InitINet();
}
//转发数据
bool TcpClientMediator::SendData(char* sendData, int nLen, long lSend)
{
	return m_pNet->SendData(sendData, nLen, lSend);
}
//处理数据
void TcpClientMediator::DealData(char* recvData, int nLen, long lFrom)
{
	//把收到的数据传给Kernel
    //使用信号和槽的机制 信号和槽是可以继承的
	in_addr addr;
	addr.S_un.S_addr = lFrom;
	cout << "TCP ClientMediator RecvData: ip is " << inet_ntoa(addr) << " say: " << recvData << endl;
    Q_EMIT SIG_ReadyData(recvData, nLen, lFrom);

}
//关闭网络
void TcpClientMediator::CloseNet()
{
	return m_pNet->UninitNet();
}

//重连网络
bool TcpClientMediator::ConnectNet()
{
    return m_pNet->ConnectNet();
}
