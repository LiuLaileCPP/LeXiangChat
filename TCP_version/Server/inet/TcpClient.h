#pragma once
#include"INet.h"
#include"../mediator/TcpClientMediator.h"

//����--UDP���繦����
class TcpClient :public INet {
public:

	TcpClient(TcpClientMediator* p_UdpNetMediator);
	~TcpClient();
	//��ʼ������
	bool InitINet();

	//��������(���͵�ַ��ô������ UDP:ip TCP:talksock)
	bool SendData(char* sendData, int nLen, long lSend);

	//��������
	void RecvData();

	//�ر�����
	void UninitNet();

	//�����ļ��ģ�
	void SendData(SOCKET sock, char* buf, int nLen);
	//�����ļ�
	void SendFile(char* fileId);
	string getName(char* str);
	//����
	void DealFileBlockRq(char* recvData, int nLen);
	void DealFileInfoRq(char* recvData, int nLen);
	void DealData(char* recvData, int nLen);

	void RefreshSocket(SOCKET sock);
private:

	SOCKET m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	HANDLE m_handle;
	bool m_bStop;


	//�������ݵ��̺߳���������RecvData()��

	static unsigned __stdcall RecvThread(void* lpVoid);
	//__stdcall �Ǻ�OS�ĵ���Լ�� 32λҪ��


};
