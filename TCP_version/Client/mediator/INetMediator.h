#pragma once
#include<QObject>



class INet;//先声明再使用 防止头文件重复循环包含

//抽象中介者父类
class INetMediator :public QObject{
    Q_OBJECT
public:
    INetMediator();
    virtual ~INetMediator();
	//打开网络
	virtual bool OpenNet() = 0;
	//转发数据
	virtual bool SendData(char* sendData, int nLen, long lSend) = 0;
	//处理数据
	virtual void DealData(char* sendData, int nLen, long lFrom) = 0;
	//关闭网络
	virtual void CloseNet() = 0;
//protected:
	//由谁发送来数据
	INet* m_pNet;

signals:
    //把接收到的数据传给Kernel
    void SIG_ReadyData(char* recvData,int nLen,long lFrom);


};	
