#include"UdpNetMediator.h"
#include"UdpNet.h"
using namespace std;

UdpNetMediator::UdpNetMediator()
{
	m_pNet = new UdpNet(this);
}
UdpNetMediator::~UdpNetMediator()
{
	if (m_pNet)
	{
        //m_pNet->UninitNet();
		delete m_pNet; m_pNet = nullptr;
	}
}
//打开网络
bool UdpNetMediator::OpenNet()
{

	return m_pNet->InitINet();
}
//转发数据
bool UdpNetMediator::SendData(char* sendData, int nLen, long lSend)
{
	return m_pNet->SendData(sendData,nLen,lSend);
}
//处理数据
void UdpNetMediator::DealData(char* recvData, int nLen, long lFrom)
{
    //把收到的数据传给Kernel
    //使用信号和槽的机制----发送信号
    cout << "UdpNetMediator::DealData ip is " << lFrom << " say: " << recvData << endl;
    Q_EMIT SIG_ReadyData(recvData,nLen,lFrom);

}
//关闭网络
void UdpNetMediator::CloseNet()
{
	return m_pNet->UninitNet();
}
