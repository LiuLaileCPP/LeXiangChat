#pragma once
#include"..\\config\\Ser_config.h"
#include"..\\net\\tcpnet.h"
#include"..\\net\\udpnet.h"
#include"..\\MySQL\\CMySql.h"

class kernel;
typedef void(kernel::* pWork)(char* recvData, sockaddr_in fromAddr);
class kernel
{
public:
	udpnet* pToolUdp;
	kernel();
	~kernel();

	CMySql m_sql;

	//��������ָ������
	pWork m_arrTypeToFun[_DEF_WORK_TYPE_SIZE_];

	//�û�id���������������
	//�û����߰����ӽ�����״̬Ϊ1����������������ʱ������״̬��Ϊ0���ط��޻ظ�ʱɾ������û�
	map<int, bool> m_mapIdToState;

	//�������пͻ��˵�id-sockaddr_in��Ϣ
	//int nOnlineClients;
	map<int, sockaddr_in> m_mapNoToAdd;			//���߿ͻ����ڷ������ı�ź�����addr
	//map<int, sockaddr_in> m_mapIdToAdd;			//���߿ͻ����ڷ�������id������addr
	map<int, int> m_mapIdToNo;					//���߿ͻ����ڷ������ı�ź�����id


	//�����������ߵ�Ⱥid
	std::map<int, std::list<int>> m_mapGroInfo;
	//oneGroInfo m_arrGroInfo[_DEF_MAX_GROUP_SIZE];

	//ѭ����֤ÿ�����߿ͻ����Ƿ��¼
	HANDLE m_handle;

	//�ⲿ�ӿ�
	static kernel* m_pKernel;

	//�򿪷�����
	bool startService();

	//�رշ�����
	void closeService();

	//���Ĺ���
	void nuclearWork(char* recvData, sockaddr_in fromAddr);

	//����ע������
	void dealRegRq(char* recvData, sockaddr_in fromAddr);

	//�����¼����,�ظ��ܷ��¼ �ܵĻ��ظ���������Ϣ Ⱥ����Ϣ Ⱥ�ĳ�Ա��Ϣ
	void dealLogRq(char* recvData, sockaddr_in fromAddr);

	//��¼�ɹ����͸���������Ϣ��Ⱥ����Ϣ�����������к�������Ⱥ��Ա��������
	void ConnectRelationships(int id, sendBuf* userInfo);

	//ת��Ⱥ����Ϣ
	void dealGroupMsg(char* recvData, sockaddr_in fromAddr);

	//�������� ת��������Ϣ
	void dealChatMsg(char* recvData, sockaddr_in fromAddr);

	//������Ӻ�������
	void dealAddFriendRq(char* recvData, sockaddr_in fromAddr);

	//�ظ���Ӻ���
	void dealAddFriendRs(char* recvData, sockaddr_in fromAddr);

	//����ɾ�����ѵ�����
	void dealDeleteFriendNoti(char* recvData, sockaddr_in fromAddr);

	//��������֪ͨ
	void dealOfflineNoti(char* recvData, sockaddr_in fromAddr);

	//�����������
	void keepAlive(char* recvData, sockaddr_in fromAddr);

	//��¼��¼�ɹ��Ŀͻ�����addr
	void recordAddr(char* recvData,sockaddr_in addr);

	//�����û�id��ȡ���ĺ�����Ϣ
	void getAllFriendInfo(int);

	//����id��ȡ������Ϣ
	bool getInfoFromId(int id,sendBuf* info);

	//��ʼ��
	void setArrayTypeToFun();

};