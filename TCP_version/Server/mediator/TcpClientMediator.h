#pragma once
#include"INetMediator.h"
//#include<QObject>

class TcpClientMediator :public INetMediator {
	//Q_OBJECT
public:
	TcpClientMediator();
	~TcpClientMediator();
	//������
	bool OpenNet();
	//ת������
	bool SendData(char* sendData, int nLen, long lSend);
	//��������
	void DealData(char* sendData, int nLen, long lFrom);
	//�ر�����
	void CloseNet();
	//�����ļ�
	void SendFile(char* fileId);

	void RefreshSocket(long);

//signals:
	//�ѽ��յ������ݴ���Kernel
	//void SIG_ReadyData(char* recvData, int nLen, long lFrom);

};