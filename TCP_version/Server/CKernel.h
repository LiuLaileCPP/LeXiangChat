#pragma once
#include"./mediator/INetMediator.h"
#include<iostream>
#include"./MySQL/CMySql.h"
#include"./dataDef/dataDef.h"
#include<map>
#include<set>
using namespace std;


//���庯��ָ��
class CKernel;
typedef void(CKernel::* pKernelDeal)(char*, int, long);


class CKernel {
public:
	CKernel();
	~CKernel();

	//�򿪷�����
	bool startService();
	//�رշ�����
	void closeService();

	//�������н��յ�������
	void dealReadyData(char* recvData,int nLen,long lFrom);

	//����ע������ĺ���
	void dealRegisterRq(char* recvData, int nLen, long lFrom);

	//�����¼����ĺ���
	void dealLoginRq(char* recvData, int nLen, long lFrom);

	//�����¼����2---�����μӵ�Ⱥ�ĵĳ�Ա��Ϣ������
	void getAllGroupMember(int id);

	//ת��Ⱥ����Ϣ
	void dealGroupMsg(char* recvData, int nLen, long lFrom);

	//������������ ת��������Ϣ
	void dealChatRq(char* recvData, int nLen, long lFrom);

	//������Ӻ��ѵ����ݱ�
	void dealAddFriendRq(char* recvData, int nLen, long lFrom);

	//����ظ���Ӻ��ѵ����ݱ�
	void dealAddFriendRs(char* recvData, int nLen, long lFrom);

	//����ɾ�����ѵ�����
	void dealDeleteFriendRq(char* recvData, int nLen, long lFrom);

	//������������ĺ���
	void dealOfflineRq(char* recvData, int nLen, long lFrom);

	//����ͻ��˵ĵ�¼��֤�ظ�
	void dealValidateRs(char* recvData, int nLen, long lFrom);

	//�����û�id��ȡ���ĺ�����Ϣ
	void getFriendInfo(int);

	//����id��ȡ������Ϣ
	void getInfoFromId(int,_STRU_TCP_FRIEND_INFO*/*��������*/);

	//�����ļ� ��ת���ļ�
	void dealRecvAndResendFile(char* recvData, int nLen, long lFrom);

	//�����ļ����յ���������ϵ��ź�
	void dealFileRecvAcc(char* recvData, int nLen, long lFrom);

	//�����ļ��ɷ�����ת����ϵ��ź�
	void dealFileResendAcc(char* recvData, int nLen, long lFrom);

	//�����ļ������ߵ��ļ����ջظ�
	void dealFileRecverRs(char* recvData, int nLen, long lFrom);
public:
	bool haveOnlineClient;
	INetMediator *m_pMediator;
	CMySql m_sql;
	pKernelDeal m_mapPackTypeToFun[_DEF_PROTOCOL_COUNT];

	//����ÿһ�����߿ͻ����ѵ�¼��socket �� id(ת����Ϣʱȡ����Ӧ��socket���ж�ĳ���ͻ����Ƿ�����)
	map<int, SOCKET> m_mapIdToSocket;
	//ѭ����֤�ͻ��˵�¼״̬�ļ�¼
	map<int, bool> m_mapIdToClientStatus;
	//����ÿһ�����߿ͻ����ѵ�¼��Ⱥ����Ϣ--
	map<int, list<int>*> m_mapGroupIdToMember;
	//������յ����ص��ļ�id
	set<string> m_setFileRecvAcc;
	//����ת���ɹ����ļ�id
	set<string> m_setFileResendAcc;
	//��ʼ��Э��ͷ����
	void setProtocolMap();
	//ѭ����֤ÿ�����߿ͻ����Ƿ��¼
	HANDLE m_handle;
	static unsigned __stdcall ValidateClientLogin(void* lpVoid);
public:
	static CKernel* m_pKernel;
};