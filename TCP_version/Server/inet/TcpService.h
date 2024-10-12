#pragma once
#include"INet.h"
#include"../mediator/TcpServiceMediator.h"

//����--UDP���繦����
class TcpService :public INet {
public:
	//�����ļ���
	map<string, STRU_FILE_INFO*> m_mapIdToFileInfo;

	TcpService(TcpServiceMediator* p_UdpNetMediator);
	~TcpService();
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
	//HANDLE m_handle;
	bool m_bStop;
	
	//����sockTalk �Ϳͻ��ˣ��������ݵ��߳�ID���Ĺ�ϵ
	map<unsigned int, SOCKET>m_mapThreadIdToSocket;
	//���������߳̾��������
	list<HANDLE>m_handleList;

	//�������ݰ����̺߳���������accept()��
	static unsigned __stdcall AcceptThread(void* lpVoid);
	//__stdcall �Ǻ�OS�ĵ���Լ�� 32λҪ��

	//�������ݰ����̺߳���������RecvData()��
	static unsigned __stdcall RecvThread(void* lpVoid);

	//����sock ˢ��m_mapThreadIdToSocket
	static unsigned __stdcall ReConnectSock(void* lpVoid);

};
