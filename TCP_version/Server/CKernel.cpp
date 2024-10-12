#include "CKernel.h"
#include"./mediator/TcpServiceMediator.h"
#include <process.h>
#include<random>
static random_device rd;

CKernel* CKernel::m_pKernel = nullptr;

CKernel::CKernel()
{
	setProtocolMap();
	m_pKernel = this;
	m_handle = 0;
	haveOnlineClient = false;
}


CKernel::~CKernel()
{

}

//�򿪷�����
bool CKernel::startService()
{
	cout << __func__ << endl;
	m_pMediator = new TcpServiceMediator;

	//������
	if (!m_pMediator->OpenNet())
	{
		cout << "CKernel::startService() ������ʧ��" << endl;
		return false;
	}

	//���ӵ����ݿ�
	if (!m_sql.ConnectMySql("127.0.0.1","root","colin123456","ims_tcp")) //root�û��Ǳ��ػ�����ַ
	{
		cout << "CKernel::startService() �������ݿ�ʧ��" << endl;
		return false;
	}
	cout << "CKernel::startService() ������ɹ�, �������ݿ�ɹ�..." << endl;

	//�����������ݵ��߳�
	m_handle = (HANDLE)_beginthreadex(0//��ȫ���� 0��Ĭ��
		, 0           //��ջ��С 0��Ĭ��
		, &ValidateClientLogin //��ʼ��ַ
		, this       //�����б�
		, 0	         //�߳�״̬ 0�Ǵ���������);
		, 0			//������߳�ID
	);
	return true;
}

//ѭ����֤ÿһ̨���߿ͻ����ĵ�¼״̬
unsigned __stdcall CKernel::ValidateClientLogin(void* lpVoid)
{
	CKernel* p = (CKernel*)lpVoid;
	map<int, SOCKET>::iterator ite; 
	_STRU_TCP_VALIDATE_RQ rq;
	while (true)
	{
		cout << "������ѭ����֤ÿһ̨���߻���" << endl;
		if (p->m_mapIdToSocket.size() == 0)
		{
			cout << "****  ��ǰ�޿ͻ�������...  ***" << endl;
			Sleep(5000);
			continue;
		}
		ite = p->m_mapIdToSocket.begin();
		while ( p->haveOnlineClient && ite != p->m_mapIdToSocket.end())
		{
			cout << "��ID --- SOCKET �б�" << endl;
			cout << "��     "<<ite->first << " -- "<<ite->second<<"     ��" << endl;
			rq.id = ite->first;
			p->m_pMediator->SendData((char*)&rq,sizeof(rq),ite->second);
			ite++;
		}
		Sleep(5000);
		auto ite2 = p->m_mapIdToClientStatus.begin();
		while (ite2 != p->m_mapIdToClientStatus.end())
		{
			if (ite2->second == false)
			{
				cout << "���ͻ���id: " << ite2->first << " ������ߣ���" << endl;
				ite2++;

				//ɾ���ͻ���
				_STRU_TCP_OFFLINE_RQ rq;
				rq.id = ite2->first;
				p->dealOfflineRq((char*)&rq,sizeof(rq),0);

				continue;
			}
			ite2->second = false;
			ite2++;
		}
	}
	//cout << "����֤������" << endl;
	return 0;
}

//�رշ�����
void CKernel::closeService()
{
	cout << __func__ << endl;

	//������Դ
	if (m_pMediator)
	{
		m_pMediator->CloseNet();
		delete m_pMediator;
		m_pMediator = nullptr;
	}
	//�ȴ��̵߳�while�ж�
	if (m_handle && WAIT_TIMEOUT == WaitForSingleObject(m_handle, 500/*ms*/))
	{
		//�ȴ�500msû���˳�ѭ����ǿ��ɱ���߳�
		TerminateThread(m_handle, -1);

		//�رվ��
		CloseHandle(m_handle);
		m_handle = NULL;
	}
	//�Ͽ����ݿ�����
	m_sql.DisConnect();
}

//�������н��յ�������
void CKernel::dealReadyData(char* recvData, int nLen, long lFrom)
{
	cout << __func__ << endl;
	//cout << "CKernel::dealReadyData : " << recvData << endl;
	//m_pMediator->SendData("CKernel::dealReadyData", sizeof("CKernel::dealReadyData"), lFrom);

	//ת�����ݰ�
	_STRU_TCP_REGISTER_RQ* rq;
	rq = (_STRU_TCP_REGISTER_RQ*)recvData;
	cout << "Service Kernel recv->one rq: type:" << rq->type /*<< " " << rq->name << " " << rq->tel << " " << rq->password*/ << endl;

	// 1 ȡ��Э��ͷ
	PackType type = *(PackType*)recvData;
	/*if (type == _DEF_TCP_ADD_FRIEND_RQ)
	{
		cout << "��type == _DEF_TCP_ADD_FRIEND_RQ��" << endl;
	}*/
    // 2 ����Э��ͷ�߲�ͬ�Ĵ�����
	//���������±�
	int index = type - (_DEF_PROTOCOL_BASE + 1);
	//�����±�ĺϷ���
	if (0 <= index && index < _DEF_PROTOCOL_COUNT)
	{
		pKernelDeal pfun = m_mapPackTypeToFun[index];
		if (pfun)
		{
			(this->*pfun)(recvData, nLen, lFrom);
		}
		else
		{
			cout << "CKernel::slot_ReadyData index value wrong : " << index << endl;
		}

	}
	else
	{
		cout<< "CKernel::slot_ReadyData index wrong : " << index << endl;
	}
	//delete������������
	delete[] recvData;

}

//����ע������ĺ���
void CKernel::dealRegisterRq(char* recvData, int nLen, long lFrom)
{
	cout << __func__ << endl;
	// 1 ���
	_STRU_TCP_REGISTER_RQ* rq = (_STRU_TCP_REGISTER_RQ*)recvData;
	cout << "CKernel::dealRegisterRq : name:" << rq->name << " tel:" << rq->tel << " password:" << rq->password << endl;
	
	
	// 2 У�����ݵĺϷ���
	string name = rq->name;
	string name_temp = rq->name;
	string tel = rq->tel;
	string tel_temp = rq->tel;
	string password = rq->password;
	string password_temp = rq->password;
	//�ж��Ƿ��ǿ��ַ��� ������ȫ�ո�
	int ite = 0;
	while (ite != -1)
	{
		ite = name_temp.find(" ");
		if(ite != -1) name_temp.erase(ite,ite + 1);
	}
	while (ite != -1)
	{
		ite = tel_temp.find(" ");
		if (ite != -1) tel_temp.erase(ite, ite + 1);
	}
	while (ite != -1)
	{
		ite = password_temp.find(" ");
		if (ite != -1) password_temp.erase(ite, ite + 1);
	}
	if (name.empty() || password.empty() || tel.empty()
		|| name_temp.empty()
		|| password_temp.empty()
		|| tel_temp.empty())
	{
		//QMessageBox::about(this, "������ʾ", "����Ϊ�հף�������");
		cout << "Client ע����Ϣ����Ϊ�հף�������" << endl;
		return;
	}
	//��鳤���Ƿ�Ϸ���name <= 20,tel = 11,password <= 20��
	if (name.length() > 20 || tel.length() != 11 || password.length() > 20)
	{
		//QMessageBox::about(this, "������ʾ", "���볤������������");
		cout << "Client ע����Ϣ���볤������������" << endl;
		return;
	}
	//��������Ƿ�Ϸ�(telΪ���� name passwordΪ��Сд��ĸ ���� �»��ߵ����)
	if (0)
	{
		//QMessageBox::about(this, "������ʾ", "�����ʽ����������");
		cout << "Client ע����Ϣ�����ʽ����������" << endl;
		return;
	}





	// 3 �����ֻ��Ų�ѯ���ݿ� 
	_STRU_TCP_REGISTER_RS rs;
	list<string> listRes;
	char sqlBuf[1024] = ""; //��ѯ���
	sprintf(sqlBuf, "select tel from t_user where tel = '%s' ;",rq->tel);
	if (!m_sql.SelectMySql(sqlBuf,1,listRes))
	{
		//tel��ѯʧ��
		cout << "CKernel::dealRegisterRq SQL_select_failed" << endl;
		rs.m_nRegister = register_false_sql_select_error;

		m_pMediator->SendData((char*)&rs, sizeof(rs), lFrom);
		return;
	}


	// 4 �ֻ����Ѿ���ע����� ��ѯ���>0  ����ע��
	if ( listRes.size() > 0)
	{
		//ע��ʧ�� �ֻ����ѱ�ע��
		cout << "CKernel::dealRegisterRq SQL_select_result : tel is used" << endl;
		rs.m_nRegister = register_false_tel_used;

		m_pMediator->SendData((char*)&rs, sizeof(rs), lFrom);
		return;
	}
	


	// 5 �ֻ���δ��ע�� �����ǳƲ�ѯ���ݿ�
	sprintf(sqlBuf, "select name from t_user where name = '%s' ;", rq->name);
	if (!m_sql.SelectMySql(sqlBuf, 1, listRes))
	{
		//name��ѯʧ��
		cout << "CKernel::dealRegisterRq SQL_select_failed" << endl;
		rs.m_nRegister = register_false_sql_select_error;

		m_pMediator->SendData((char*)&rs, sizeof(rs), lFrom);
		return;
	}
	//�жϲ鵽����������
	if (listRes.size() > 0)//== 2)
	{
		//ע��ʧ�� �ǳ��ѱ�ע��
		cout << "CKernel::dealRegisterRq SQL_select_result : name is used" << endl;
		rs.m_nRegister = register_false_name_used;
		
		m_pMediator->SendData((char*)&rs, sizeof(rs), lFrom);
		return;
	}



	// 6 �����û���Ϣд�����ݿ�
	//�����һ��iconid
	srand((unsigned)time(0));
	int i = rd() % 36;
	if (i > 35) i = 7;
	sprintf(sqlBuf, "insert into t_user (name,tel,password,feeling,iconid) values ('%s','%s','%s','a bug_lover',%d);"
		,rq->name,rq->tel,rq->password,i);
	if (!m_sql.UpdateMySql(sqlBuf))
	{
		//д�����ݿ�ʧ��
		cout << "CKernel::dealRegisterRq SQL_update_failed, sqlBuf : " <<sqlBuf<< endl;
		rs.m_nRegister = register_false_sql_update_error;

		m_pMediator->SendData((char*)&rs, sizeof(rs), lFrom);
		return;
	}



	// 7 �ߵ������ע��ɹ���
	rs.m_nRegister = register_success;
	m_pMediator->SendData((char*)&rs, sizeof(rs), lFrom);
	return;
}

//�����¼����ĺ���
void CKernel::dealLoginRq(char* recvData, int nLen, long lFrom)
{
	cout << __func__ << endl;


	// 1 ���
	_STRU_TCP_LOGIN_RQ* rq = (_STRU_TCP_LOGIN_RQ*)recvData;
	cout << "CKernel::dealRegisterRq :" << " tel:" << rq->tel << " password:" << rq->password << endl;


	// 2 У�����ݵĺϷ���
	string tel = rq->tel;
	string tel_temp = rq->tel;
	string password = rq->password;
	string password_temp = rq->password;
	//�ж��Ƿ��ǿ��ַ��� ������ȫ�ո�
	int ite = 0;
	while (ite != -1)
	{
		ite = tel_temp.find(" ");
		if (ite != -1) tel_temp.erase(ite, ite + 1);
	}
	while (ite != -1)
	{
		ite = password_temp.find(" ");
		if (ite != -1) password_temp.erase(ite, ite + 1);
	}
	if (password.empty() || tel.empty()
		|| password_temp.empty()
		|| tel_temp.empty())
	{
		//QMessageBox::about(this, "������ʾ", "����Ϊ�հף�������");
		cout << "Client ��¼��Ϣ����Ϊ�հף�������" << endl;
		return;
	}
	//��鳤���Ƿ�Ϸ���name <= 20,tel = 11,password <= 20��
	if (tel.length() != 11 || password.length() > 20)
	{
		//QMessageBox::about(this, "������ʾ", "���볤������������");
		cout << "Client ��¼��Ϣ���볤������������" << endl;
		return;
	}
	//��������Ƿ�Ϸ�(telΪ���� name passwordΪ��Сд��ĸ ���� �»��ߵ����)
	if (0)
	{
		//QMessageBox::about(this, "������ʾ", "�����ʽ����������");
		cout << "Client ��¼��Ϣ�����ʽ����������" << endl;
		return;
	}


	// 3 �����ֻ��Ų�ѯ����
	_STRU_TCP_LOGIN_RS rs;
	rs.sock = lFrom;

	//���������״̬���Զ����µ�¼ �Ͳ������ݿ���
	if (rq->bOnlineStatus == true)
	{
		/*m_sql.DisConnect();
		m_sql.ConnectMySql("127.0.0.1", "root", "colin123456", "ims_tcp");*/
		rs.m_nLogin = login_success;
		rs.userId = rq->userId;
		if (rq->userId == -1)
		{
			cout << "CKernel::dealLoginRq ����״̬���Զ����µ�¼ʧ��" << endl;
			return;
		}
		m_mapIdToSocket[rq->userId] = lFrom;
		m_pMediator->SendData((char*)&rs,sizeof(rs),lFrom);
		//�����û�id��ȡ��ǰ�û��ĺ����б���Ϣ ��������͸���
		getFriendInfo(rs.userId);
		getAllGroupMember(rs.userId);
		cout << "�������û��Զ����µ�¼�ɹ� id: "<<rs.userId<<" ��" << endl;
		return;
	}
	list<string> listRes;
	char sqlBuf[1024] = ""; //��ѯ���
	sprintf(sqlBuf, "select id,password from t_user where tel = '%s' ;", rq->tel);
	if (!m_sql.SelectMySql(sqlBuf, 2, listRes))
	{
		//tel��ѯʧ��
		cout << "CKernel::dealLoginRq SQL_select_failed" << endl;
		cout << "��" << sqlBuf<<"��" << endl;
		rs.m_nLogin = login_false_sql_select_error;

		m_pMediator->SendData((char*)&rs, sizeof(rs), lFrom);
		return;
	}

	// 4 �жϲ�ѯ���
	//�ֻ���δע��� --- û�в�ѯ��
	if (listRes.size() == 0)
	{
		cout << "CKernel::dealLoginRq SQL_select_failed" << endl;
		rs.m_nLogin = login_false_no_tel;

		m_pMediator->SendData((char*)&rs, sizeof(rs), lFrom);
		return;
	}
	// ȡ���û�id
	int id = atoi(listRes.front().c_str());
	rs.userId = id;
	listRes.pop_front();
	// 4 ȡ������
	password = listRes.front();
	// 5 ��������
	if (strcmp(password.c_str(), rq->password) != 0)
	{
		//������������
		rs.m_nLogin = login_false_password_wrong;

		m_pMediator->SendData((char*)&rs, sizeof(rs), lFrom);
		return;
	}
	else
	{
		//�����������ȷ
		rs.m_nLogin = login_success;
		rs.sock = lFrom;
		//��֤��������Ƿ�������״̬
		if (m_mapIdToSocket.count(id) > 0)
		{
			rs.m_nLogin = login_false_illegal;
			m_pMediator->SendData((char*)&rs, sizeof(rs), lFrom);
			if (rq->bOnlineStatus)
			{
				//���浱ǰ�û���socket��id
				m_mapIdToSocket[id] = lFrom;
				m_mapIdToClientStatus[id] = true;

				//�����Լ�����Ϣ���͸���
				m_pMediator->SendData((char*)&rs, sizeof(rs), lFrom);
				//�����û�id��ȡ��ǰ�û��ĺ����б���Ϣ ��������͸���
				getFriendInfo(id);
				getAllGroupMember(id);
			}
			
			return;
		}
		//���浱ǰ�û���socket��id
		m_mapIdToSocket[id] = lFrom;
		m_mapIdToClientStatus[id] = true;

		//�����Լ�����Ϣ���͸���
		m_pMediator->SendData((char*)&rs, sizeof(rs), lFrom);
		//�����û�id��ȡ��ǰ�û��ĺ����б���Ϣ ��������͸���
		getFriendInfo(id);
		getAllGroupMember(id);
		Sleep(10);
		haveOnlineClient = true;
		return;
	}
}

//�����û�id��ȡ���ĺ�����Ϣ ��������͸���
void CKernel::getFriendInfo(int id)
{
	_STRU_TCP_FRIEND_INFO info;
	//id��ȡ���Լ�����Ϣ
	getInfoFromId(id, &info);
	//���͵��ͻ���
	if (m_mapIdToSocket[id])
	{
		m_pMediator->SendData((char*)&info, sizeof(info), m_mapIdToSocket[id]);
	}
	else
	{
		cout << "CKernel::getFriendInfo map_no_your_id-ip" << endl;
		return;
	}
	//id��ȡ���ĺ���id�б�
	list<string> listRes;
	list<int> listFriendId;
	char sqlBuf[1024] = ""; //��ѯ���
	sprintf(sqlBuf, "select idB from t_friend where idA = %d ;", id);
	if (!m_sql.SelectMySql(sqlBuf, 1, listRes))
	{
		cout << "CKernel::getFriendInfo sql_select_friendId_failed" << endl;
		return;
	}
	else
	{
		//cout << "Start listFriendId" << endl;
		list<string>::iterator ite = listRes.begin();
		//list<int>::iterator itef = listFriendId.begin();
		while (listRes.size() > 0 && ite != listRes.end())
		{
			listFriendId.push_back(stoi(listRes.front()));
			ite = listRes.erase(ite);
		}

	}



	//������ѯ�����һ��һ������
	int fId;
	int send;
	while (listFriendId.size() > 0)
	{
		//cout << "Start ȡ������id" << endl;
		//ȡ������id
		fId = listFriendId.front();
		listFriendId.pop_front();
		//id->��ѯ���ѵ���Ϣ
		getInfoFromId(fId, &info);
		//���͵��ͻ���
		if (m_mapIdToSocket[id])
		{
			send = m_pMediator->SendData((char*)&info, sizeof(info), m_mapIdToSocket[id]);
			cout << id << " ��һ������idΪ :" << fId << ",sendNum : " << send << endl;
		}
		else
		{
			cout << "CKernel::getFriendInfo map_no_friend_id-ip, id : " << "id" << endl;
		}
		//֪ͨ��ǰ�û������к�����������
		//�ж�ÿ�������Ƿ����� ���߾ͷ������Լ�����Ϣ
		if (m_mapIdToSocket.count(fId) > 0)
		{
			getInfoFromId(id, &info);
			info.status = status_online;
			send = m_pMediator->SendData((char*)&info, sizeof(info), m_mapIdToSocket[fId]);
			cout <<id<< " ���Լ�����Ϣ���͸������ߵĺ��� : " <<fId<<",sendNum : "<<send << endl;
		}
	}
}

//�����û�id�����μӵ�Ⱥ����Ϣ �������г�Ա��Ϣ������
void CKernel::getAllGroupMember(int id)
{
	//���Դ�ӡ
	int row = 1;
	auto iteMap = m_mapGroupIdToMember.begin();
	while (iteMap != m_mapGroupIdToMember.end())
	{
		auto iteMem = iteMap->second->begin();
		while (iteMem != iteMap->second->end())
		{
			cout << "��row: "<<row<<" Ⱥid: "<<iteMap->first<<" ������һ����Աid: "<<*iteMem<<"��"<<endl;
			iteMem++;
			row++;
		}
		iteMap++;
	}

	_STRU_TCP_FRIEND_INFO info;
	getInfoFromId(id,&info);
	auto ite = info.hisGroupMap.begin();
	//������Ⱥ-id��Ϣ ���ڷ�����
	while (ite != info.hisGroupMap.end())
	{
		int gId = ite->first;
		if (m_mapGroupIdToMember.size() == 0 || m_mapGroupIdToMember.count(gId) <= 0)
		{
			list<int>* pList = new list<int>;
			pList->push_back(info.id);
			m_mapGroupIdToMember[gId] = pList;
		}
		else
		{
			auto iteList = m_mapGroupIdToMember[gId]->begin();
			while (iteList != m_mapGroupIdToMember[gId]->end())
			{
				if (*iteList == info.id) break;
				iteList++;
			}
			if(iteList == m_mapGroupIdToMember[gId]->end()) m_mapGroupIdToMember[gId]->push_front(info.id);
		}
		ite++;
	}
	//ͨ��Ⱥ�������ݿ����ҵ����еĳ�Ա�ӽ���
	
	//������������map ������Ϣ
	_STRU_TCP_SEND_CLIENT_GROUP_MEMBERS_INFO memberInfo;
	_STRU_TCP_SEND_CLIENT_GROUP_INFO groupInfo;
	ite = info.hisGroupMap.begin();
	while (ite != info.hisGroupMap.end())
	{
		int gId = ite->first;
		//�����Ⱥ�ĵ�iconId
		char sqlBuf[1024];
		sprintf(sqlBuf,"select distinct gIconId from t_group where groupId = %d;",gId);
		list<string>listRes;
		if (!m_sql.SelectMySql(sqlBuf,1,listRes))
		{
			cout << "CKernel::getAllGroupMember idΪ "<<gId <<" ����iconʧ��" << endl;
			ite++;
			continue;
		}
		if (listRes.size() == 1)
		{
			memberInfo.groupIconid = stoi(listRes.front());
			groupInfo.groupIconid = memberInfo.groupIconid;
			listRes.pop_front();
		}
		else
		{
			cout << "CKernel::getAllGroupMember idΪ " << gId << " ����icon�������/���" << endl;
			ite++;
			continue;
		}
		strcpy(memberInfo.groupName, (ite->second).c_str());
		strcpy(groupInfo.groupName, (ite->second).c_str());
		memberInfo.groupId = gId;
		groupInfo.groupId = gId;

		//Ⱥ�ĳ�Աֻ�����Լ� Ⱥ�ǻ�ɫͷ�� ������
		if (m_mapGroupIdToMember[gId]->size() == 1)
		{
			memberInfo.memberId = id;
			memberInfo.memberStatus = status_online;
			strcpy(memberInfo.memberName,info.name);
			memberInfo.memberIconid = info.iconId;
			memberInfo.groupStatus = status_offline;
			groupInfo.groupStatus = status_offline;
			int send1 = m_pMediator->SendData((char*)&groupInfo,sizeof(memberInfo),m_mapIdToSocket[id]);
			int send2 = m_pMediator->SendData((char*)&memberInfo, sizeof(groupInfo), m_mapIdToSocket[id]);
			cout << "send1: "<<send1 <<" send2: "<<send2 << endl;
			//ȥ�����ݿ�
			memberInfo.memberStatus = status_offline;
			sprintf(sqlBuf, "select distinct userId,name from t_group inner join t_user on t_group.userid = t_user.id where groupId = %d and userId != %d;", gId,id);
			list<string>lRes;
			if (m_sql.SelectMySql(sqlBuf,2,lRes))
			{
				while (lRes.size() > 0)
				{
					memberInfo.memberId = stoi(lRes.front()); 
					lRes.pop_front();
					if (m_mapIdToSocket.count(memberInfo.memberId) > 0)
					{
						memberInfo.memberStatus = status_online;
					}
					strcpy(memberInfo.memberName,lRes.front().c_str());
					lRes.pop_front();
					_STRU_TCP_FRIEND_INFO info;
					getInfoFromId(memberInfo.memberId, &info);
					memberInfo.memberIconid = info.iconId;
					int num = m_pMediator->SendData((char*)&memberInfo, sizeof(groupInfo), m_mapIdToSocket[id]);
					cout << m_mapIdToSocket.size() << " num1: "<<num << memberInfo.groupName <<"---"<<memberInfo.memberName<<" ����1: " << memberInfo.memberStatus << endl;
					memberInfo.memberStatus = status_offline;
				}
			}
			else
			{
				cout << "mapֻ�����Լ���ȥ�����ݿ�ʧ��" << endl;
			}
			ite++;
			continue;
		}
		else  //Ⱥ����������Ա���� ��ɫȺͷ��
		{
			groupInfo.groupStatus = status_online;
			int send3 = m_pMediator->SendData((char*)&groupInfo, sizeof(groupInfo), m_mapIdToSocket[id]);

			//�����Լ���Ⱥ����Ϣ
			memberInfo.memberStatus = status_online;
			memberInfo.memberId = info.id;
			strcpy(memberInfo.memberName, info.name);
			memberInfo.memberIconid = info.iconId;
			int send4 = m_pMediator->SendData((char*)&memberInfo, sizeof(groupInfo), m_mapIdToSocket[id]);
			cout << "send3: " << send3 <<" send4: "<<send4 << endl;

			//ȥ�����ݿ�
			memberInfo.memberStatus = status_offline;
			sprintf(sqlBuf, "select distinct userId,name from t_group inner join t_user on t_group.userid = t_user.id where groupId = %d and userId != %d;", gId, id);
			list<string>lRes;
			if (m_sql.SelectMySql(sqlBuf, 2, lRes))
			{
				while (lRes.size() > 0)
				{
					memberInfo.memberId = stoi(lRes.front());
					lRes.pop_front();
					if (m_mapIdToSocket.count(memberInfo.memberId) > 0)
					{
						memberInfo.memberStatus = status_online;
					}
					strcpy(memberInfo.memberName, lRes.front().c_str());
					lRes.pop_front();
					_STRU_TCP_FRIEND_INFO info;
					getInfoFromId(memberInfo.memberId,&info);
					memberInfo.memberIconid = info.iconId;
					int num = m_pMediator->SendData((char*)&memberInfo, sizeof(groupInfo), m_mapIdToSocket[id]);
					cout << m_mapIdToSocket.size()<<" num2: "<<num<<memberInfo.groupName << "---" << memberInfo.memberName<< " "<<memberInfo.memberStatus << endl;
					memberInfo.memberStatus = status_offline;
				}
			}
			else
			{
				cout << "map����ֻ�����Լ���ȥ�����ݿ�ʧ��" << endl;
			}
			/*memberInfo.groupStatus = status_online;
			auto ite2 = (m_mapGroupIdToMember[gId])->begin();
			while (ite2 != m_mapGroupIdToMember[gId]->end())
			{
				memberInfo.memberId = *ite2;
				if (m_mapIdToSocket.count(*ite2) > 0)
				{
					memberInfo.memberStatus = status_online;
				}
				else
				{
					memberInfo.memberStatus = status_offline;
				}
				_STRU_TCP_FRIEND_INFO mInfo;
				getInfoFromId(*ite2,&mInfo);
				strcpy(memberInfo.memberName, mInfo.name);
				m_pMediator->SendData((char*)&memberInfo, sizeof(memberInfo), m_mapIdToSocket[id]);
				ite2++;
			}*/
		}
		ite++;
	}

	//������������Ⱥ��map �����������ߵ�Ⱥ��Ա ��������
	_STRU_TCP_SEND_CLIENT_GROUP_MEMBERS_INFO infoToMer;
	getInfoFromId(id, &info);
	ite = info.hisGroupMap.begin();
	infoToMer.groupIconid = -1;
	infoToMer.groupStatus = status_online;
	while (ite != info.hisGroupMap.end())
	{
		auto ite2 = m_mapGroupIdToMember.begin();
		while (ite2 != m_mapGroupIdToMember.end())
		{
			//�ҵ�������Ⱥ
			if (ite2->first == ite->first)
			{
				infoToMer.groupId = ite->first;
				strcpy(infoToMer.groupName,ite->second.c_str());
				//�������ߵĳ�Ա
				auto ite3 = m_mapGroupIdToMember[ite2->first]->begin();
				while (ite3 != m_mapGroupIdToMember[ite2->first]->end())
				{
					if (*ite3 != info.id)
					{
						_STRU_TCP_FRIEND_INFO mInfo;
						getInfoFromId(*ite3, &mInfo);
						infoToMer.memberStatus = status_online;
						infoToMer.memberId = id;
						infoToMer.memberIconid = info.iconId;
						strcpy(infoToMer.memberName, info.name);
						m_pMediator->SendData((char*)&infoToMer, sizeof(infoToMer), m_mapIdToSocket[*ite3]);
						cout << "����Աid: " << info.id << "������Ⱥ: " << infoToMer.groupId << " �ĳ�Աid: " << *ite3 << " �������ˡ�" << endl;
					}
					ite3++;
				}
			}
			ite2++;
		}
		ite++;
	}

}

//����id��ȡ������Ϣ
void CKernel::getInfoFromId(int id, _STRU_TCP_FRIEND_INFO* pinfo/*��������*/)
{
	pinfo->id = id;
	//�жϵ�ǰ�û��Ƿ�����
	if (m_mapIdToSocket.count(id) > 0)
	{
		pinfo->status = status_online;
	}
	else
	{
		pinfo->status = status_offline;
	}
	//�����ݿ����Ϣ
	list<string> listRes;
	char sqlBuf[1024] = ""; //��ѯ���
	sprintf(sqlBuf, "select name,iconid,feeling from t_user where id = %d ;", id);
	if (!m_sql.SelectMySql(sqlBuf, 3, listRes))
	{
		cout << "CKernel::getInfoFromId sql_select_failed" << endl;
		return;
	}
	//�жϲ�ѯ����Ƿ���ȷ
	if (listRes.size() == 3)
	{
		//cout << "CKernel::getInfoFromId sql_select_success" << endl;

		//ȡ����Ϣ
		strcpy(pinfo->name, listRes.front().c_str());
		listRes.pop_front();

		pinfo->iconId = stoi(listRes.front());
		listRes.pop_front();

		strcpy(pinfo->feeling, listRes.front().c_str());
		listRes.pop_front();
		//cout << "getInfoFromId:: listRes.size() == 3" << endl;
	}
	else
	{
		cout << "CKernel::getInfoFromId sql_selected_res_wrong" << endl;
	}
	sprintf(sqlBuf, "select groupId,groupName from t_group where userId = %d ;", id);
	if (m_sql.SelectMySql(sqlBuf, 2, listRes))
	{
		while (listRes.size() > 0)
		{
			int gId = stoi(listRes.front());
			listRes.pop_front();

			if (listRes.size() == 0)
			{
				cout << "CKernel::getInfoFromId ��������Ⱥ�Ĳ�����" << endl;
				return;
			}
			string gName = listRes.front();
			listRes.pop_front();
			pinfo->hisGroupMap[gId] = gName;
		}
	}
	else
	{
		cout << "CKernel::getInfoFromId ��������Ⱥ����Ϣʧ��" << endl;
	}
}

//ת��Ⱥ����Ϣ
void CKernel::dealGroupMsg(char* recvData, int nLen, long lFrom)
{
	_STRU_TCP_GROUP_CHAT_MSG* msg = (_STRU_TCP_GROUP_CHAT_MSG*)recvData;

	//�ҵ����Ⱥ
	if (m_mapGroupIdToMember.count(msg->groupId) > 0)
	{
		//��������ÿһ�����ߵĳ�Ա
		auto ite = m_mapGroupIdToMember[msg->groupId]->begin();
		while (ite != m_mapGroupIdToMember[msg->groupId]->end())
		{
			if (m_mapIdToSocket.count(*ite) > 0 && *ite != msg->memberId) //��Ա���߾ͷ�
			{
				m_pMediator->SendData((char*)msg,sizeof(*msg),m_mapIdToSocket[*ite]);
				cout << "��ת����Ⱥ��Ϣ�� Ⱥ��:"<<msg->groupId <<" ����id:"<<msg->memberId<<" ����id:"<<*ite <<"��" << endl;
			}
			ite++;
		}
	}
	else
	{
		cout << "CKernel::dealGroupMsg ���Ⱥû�������� groupId: "<<msg->groupId << endl;
		return;
	}
	
}

//������������ĺ���
void CKernel::dealOfflineRq(char* recvData, int nLen, long lFrom)
{
	// 1 ���
	_STRU_TCP_OFFLINE_RQ* rq = (_STRU_TCP_OFFLINE_RQ*)recvData;
	int id = rq->id;

	// 2 ����idȡ������id�б� ��ÿ�����ѷ���һ���Լ�����Ϣinfo��status = offline��
	char sqlBuf[100];
	list<string>listRes;
	list<string>listFriend;
	_STRU_TCP_FRIEND_INFO info;
	info.id = id;
	info.status = status_offline;
	sprintf(sqlBuf, "select name,feeling,iconid from t_user where id = %d;", id);
	if (!m_sql.SelectMySql(sqlBuf, 3, listFriend))
	{
		cout << "CKernel::dealOfflineRq sql_select_friend_failed" << endl;
	}
	strcpy(info.name, listFriend.front().c_str());
	listFriend.pop_front();
	strcpy(info.feeling, listFriend.front().c_str());
	listFriend.pop_front();
	info.iconId = stoi(listFriend.front());
	listFriend.pop_front();
	
	sprintf(sqlBuf, "select idB from t_friend where idA = %d;", id);
	if(!m_sql.SelectMySql(sqlBuf, 1, listRes))
	{
		cout << "CKernel::dealOfflineRq sql_select_failed" << endl;
		return;
	}
	while (listRes.size() > 0)
	{
		id = stoi(listRes.front());
		listRes.pop_front();

		//����������߾ͷ����Լ���������Ϣ---info
		if (m_mapIdToSocket.count(id) > 0)
		{
			int send = m_pMediator->SendData((char*)&info, sizeof(info), m_mapIdToSocket[id]);
			cout << rq->id << " Ҫ�����ˣ��������������ߺ��� " <<id <<",sendNum : " << send << endl;
		}
		
	}

	

	// 4 ���Լ���Ⱥ��map���Ƴ���
	auto iteG = m_mapGroupIdToMember.begin();
	while (iteG != m_mapGroupIdToMember.end())
	{
		auto iteL = iteG->second->begin();
		while (iteL != iteG->second->end())
		{
			if (*iteL == rq->id)
			{
				iteL = iteG->second->erase(iteL);
				continue;
			}
			iteL++;
		}
		iteG++;
	}
	//������������Ⱥ��map �����������ߵ�Ⱥ��Ա ��������
	_STRU_TCP_SEND_CLIENT_GROUP_MEMBERS_INFO infoToMer;
	getInfoFromId(rq->id, &info);
	auto ite = info.hisGroupMap.begin();
	infoToMer.groupIconid = -1;
	infoToMer.groupStatus = status_online;
	while (ite != info.hisGroupMap.end())
	{
		auto ite2 = m_mapGroupIdToMember.begin();
		while (ite2 != m_mapGroupIdToMember.end())
		{
			//�ҵ�������Ⱥ
			if (ite2->first == ite->first)
			{
				infoToMer.groupId = ite->first;
				strcpy(infoToMer.groupName, ite->second.c_str());
				//�������ߵĳ�Ա
				auto ite3 = m_mapGroupIdToMember[ite2->first]->begin();
				while (ite3 != m_mapGroupIdToMember[ite2->first]->end())
				{
					if (*ite3 != info.id)
					{
						if (m_mapGroupIdToMember[ite2->first]->size() == 1)
						{
							infoToMer.groupStatus = status_offline;
						}
						_STRU_TCP_FRIEND_INFO mInfo;
						getInfoFromId(*ite3, &mInfo);
						infoToMer.memberStatus = status_offline;
						infoToMer.memberId = rq->id;
						infoToMer.memberIconid = info.iconId;
						strcpy(infoToMer.memberName, info.name);
						m_pMediator->SendData((char*)&infoToMer, sizeof(infoToMer), m_mapIdToSocket[*ite3]);
						cout << "����Աid: " << info.id << "������Ⱥ: " << infoToMer.groupId << " �ĳ�Աid: " << *ite3 << " �������ˡ�" << endl;
					}
					ite3++;
				}
			}
			ite2++;
		}
		ite++;
	}
	// 3 ���Լ���map���Ƴ���
	if (m_mapIdToSocket.count(rq->id) > 0)
	{
		closesocket(m_mapIdToSocket[rq->id]);
		//auto ite = m_mapIdToSocket.find(rq->id);
		m_mapIdToSocket.erase(rq->id);
	}
	if (m_mapIdToClientStatus.count(rq->id) > 0)
	{
		m_mapIdToClientStatus.erase(rq->id);
	}
}

//������������ ת��������Ϣ
void CKernel::dealChatRq(char* recvData, int nLen, long lFrom)
{
	_STRU_TCP_CHAT_RQ* rq = (_STRU_TCP_CHAT_RQ*)recvData;
	int fromId = rq->fromId;
	int toId = rq->toId;
	string content = rq->content;
	cout << fromId << " �� " << toId << " ˵ : " << content << endl;
	//�жϽ��շ��Ƿ�����
	if (m_mapIdToSocket.count(toId) > 0)
	{
		m_pMediator->SendData((char*)rq, sizeof(*rq), m_mapIdToSocket[toId]);
	}
	else
	{
		cout << "���� " << toId << " ��������" << endl;
		strcpy(rq->content,"��ϵͳ��ʾ�����Ѳ�������>_<");
		m_pMediator->SendData((char*)rq, sizeof(*rq), m_mapIdToSocket[fromId]);

		_STRU_TCP_CHAT_RS rs;
		rs.fromId = rq->toId;
		m_pMediator->SendData((char*)&rs, sizeof(rs), m_mapIdToSocket[fromId]);
	}
}

//������Ӻ��ѵ����ݱ�
void CKernel::dealAddFriendRq(char* recvData, int nLen, long lFrom)
{
	// 1 ���
	_STRU_TCP_ADD_FRIEND_RQ* rq = (_STRU_TCP_ADD_FRIEND_RQ*)recvData;
	cout << rq->fromName << " ������Ӻ��� : " << rq->toName<<endl;

	
	
	/*for (map<int,SOCKET>::iterator ite = m_mapIdToSocket.begin();
		ite != m_mapIdToSocket.end();ite++)
	{
		cout << "��"<<ite->first << "--" << ite->second <<"��" << endl;
	}*/
	if (m_mapIdToSocket[rq->fromId] != lFrom)
	{
		cout << "��" << rq->fromId << " �� id--sock δ�ܼ�ʱ���µ�map�С�id--sock: " << rq->fromId << "--" << lFrom << endl;
	}
	//���°�sock
	m_mapIdToSocket[rq->fromId] = lFrom;

	// 2 У��
	if (rq->fromName == rq->toName)
	{
		cout << rq->fromName << " ����Լ�Ϊ���ѣ����ƣ�" << endl;
		return;
	}
	string name = rq->fromName;
	string name_temp = name;
	//�ж��Ƿ��ǿ��ַ��� ������ȫ�ո�
	int ite = 0;
	while (ite != -1)
	{
		ite = name_temp.find(" ");
		if (ite != -1) name_temp.erase(ite, ite + 1);
	}
	
	if (name.empty() || name_temp.empty())
	{
		//QMessageBox::about(this, "������ʾ", "����Ϊ�հף�������");
		cout << "Client ע����Ϣ����Ϊ�հף�������" << endl;
		return;
	}
	//��鳤���Ƿ�Ϸ���name <= 20,tel = 11,password <= 20��
	if (name.length() > 20)
	{
		//QMessageBox::about(this, "������ʾ", "���볤������������");
		cout << "Client ע����Ϣ���볤������������" << endl;
		return;
	}
	//��������Ƿ�Ϸ�(telΪ���� name passwordΪ��Сд��ĸ ���� �»��ߵ����)
	if (0)
	{
		//QMessageBox::about(this, "������ʾ", "�����ʽ����������");
		cout << "Client ע����Ϣ�����ʽ����������" << endl;
		return;
	}

	// 3 ��ȡ���ѵ�id  ��ѯ�����Ƿ����  �Ƿ�����
	_STRU_TCP_FRIEND_INFO info;
	_STRU_TCP_ADD_FRIEND_RS res;
	strcpy(res.friendName , rq->toName);
	res.fromId = rq->fromId;

	char sqlBuf[100];
	list<string>listRes;
	cout << "select id from t_user where name = "<<rq->toName << endl;
	sprintf(sqlBuf,"select id from t_user where name = '%s';",rq->toName);
	if (!m_sql.SelectMySql(sqlBuf,1,listRes))//���Ѳ�����
	{
		cout << "���ݿ��ѯ����" << endl;
		return;
	}
	else //���Ѵ����ж��Ƿ�����
	{
		if (listRes.size() > 0)
		{
			info.id = stoi(listRes.front());
			listRes.pop_front();
		}
		else
		{
			res.m_nAddRes = add_friend_false_no_exist;
			m_pMediator->SendData((char*)&res, sizeof(res), m_mapIdToSocket[rq->fromId]);
			cout << "���Ǻ��� : " << rq->toName << "������";
			return;
			
		}
		
		if (m_mapIdToSocket.count(info.id) <= 0) //���Ѳ�����
		{
			res.m_nAddRes = add_friend_false_offline;
			m_pMediator->SendData((char*)&res, sizeof(res), m_mapIdToSocket[rq->fromId]);
			cout << "���Ǻ��� : " << rq->toName << "������";
			return;
		}
		else //���������ת����������
		{
			m_pMediator->SendData((char*)rq, sizeof(*rq), m_mapIdToSocket[info.id]);
			cout << "����� : " << rq->toName << " ת���� "<<rq->fromName<<" �ĺ�������"<<endl;
			return;
		}
	}

}

//����ظ���Ӻ��ѵ����ݱ�
void CKernel::dealAddFriendRs(char* recvData, int nLen, long lFrom)
{
	
	
	// 1 ���
	_STRU_TCP_ADD_FRIEND_RS* rs = (_STRU_TCP_ADD_FRIEND_RS*)recvData;
	
	
	if (m_mapIdToSocket[rs->friendId] != lFrom)
	{
		cout << "��" << rs->friendId << " �� id--sock δ�ܼ�ʱ���µ�map�С�id--sock: " << rs->friendId << "--" << lFrom << endl;
	}
	//���°�sock
	m_mapIdToSocket[rs->friendId] = lFrom;
	if (rs->m_nAddRes == add_friend_success)
	{
		cout << "��friendId: " << rs->friendId<<"--"<<m_mapIdToSocket[rs->friendId] 
			<< " ͬ�ⱻfromId: " << rs->fromId << "--" << m_mapIdToSocket[rs->fromId] << " ��ӡ�" << endl;
	}
	else if (rs->m_nAddRes == add_friend_false_disagree)
	{
		cout << "��friendId: " << rs->friendId << "--" << m_mapIdToSocket[rs->friendId]
			<< " ��ͬ�ⱻfromId: " << rs->fromId << "--" << m_mapIdToSocket[rs->fromId] << " ��ӡ�" << endl;
	}
	
	
	//�ж��Ƿ�����Ǹ���Ϊ���� �ǵĻ� ��������һ���Ӻ��ѳɹ��ظ�/��һ��������Ϣ/�غ���һ���Լ�����Ϣ  ���ǵĻ����Ǹ�����һ��������ӽ��disagree
	if (rs->m_nAddRes == add_friend_success)
	{
		//���Լ�һ��������Ϣ
		_STRU_TCP_FRIEND_INFO info;
		getInfoFromId(rs->friendId,&info);
		cout << info.id << "--" << m_mapIdToSocket[info.id] <<" ��������ˣ���: "<<rs->fromId<<"--"<<m_mapIdToSocket[rs->fromId] << endl;
		m_pMediator->SendData((char*)&info, sizeof(info), m_mapIdToSocket[rs->fromId]);
		//�غ���һ���Լ�����Ϣ
		_STRU_TCP_FRIEND_INFO userInfo;
		getInfoFromId(rs->fromId, &userInfo);
		cout << rs->friendId << "--" << m_mapIdToSocket[rs->friendId] << " ->��, �ұ���������ˣ�����: " << userInfo.id << "--" << m_mapIdToSocket[userInfo.id] << endl;
		m_pMediator->SendData((char*)&userInfo, sizeof(userInfo), m_mapIdToSocket[rs->friendId]);
		//��������һ�����Լ��ɹ��ظ�
		_STRU_TCP_ADD_FRIEND_RS fRs;
		fRs.friendId = userInfo.id;
		strcpy(fRs.friendName , userInfo.name);
		fRs.fromId = info.id;
		fRs.m_nAddRes = add_friend_success;
		cout << fRs.fromId << "--" << m_mapIdToSocket[fRs.fromId] << " ->��, �ұ�������ӳɹ�������: " << fRs.friendId << "--" << m_mapIdToSocket[info.id] << endl;
		m_pMediator->SendData((char*)&fRs, sizeof(fRs), m_mapIdToSocket[info.id]);
		//�����Լ�һ���Ӻ��ѳɹ��ظ�
		fRs.friendId = info.id;
		strcpy(fRs.friendName, info.name);
		fRs.fromId = userInfo.id;
		fRs.m_nAddRes = add_friend_success;
		cout << fRs.friendId << "--" << m_mapIdToSocket[fRs.friendId] << " ������ӳɹ�����: " << rs->fromId << "--" << m_mapIdToSocket[userInfo.id] << endl;
		m_pMediator->SendData((char*)&fRs, sizeof(fRs), m_mapIdToSocket[userInfo.id]);
		//���ѹ�ϵд�����ݿ�
		char sqlBuf[100];
		list<string>listRes;
		sprintf(sqlBuf,"insert into t_friend (idA,idB) values (%d,%d);",rs->friendId,rs->fromId);
		if (!m_sql.UpdateMySql(sqlBuf))
		{
			cout << "CKernel::dealAddFriendRs �������ѹ�ϵ1д�����ݿ�ʧ��" << endl;
		}
		sprintf(sqlBuf, "insert into t_friend (idA,idB) values (%d,%d);", rs->fromId, rs->friendId);
		if (!m_sql.UpdateMySql(sqlBuf))
		{
			cout << "CKernel::dealAddFriendRs �������ѹ�ϵ2д�����ݿ�ʧ��" << endl;
		}
	}
	else if(rs->m_nAddRes == add_friend_false_disagree)
	{
		//��ͬ���ת�����ݱ�
		m_pMediator->SendData((char*)rs,sizeof(*rs),m_mapIdToSocket[rs->fromId]);
		cout << rs->friendId <<"--" << m_mapIdToSocket[rs->friendId] << " ��ͬ�ⱻ " << rs->fromId << "--" << m_mapIdToSocket[rs->fromId] << " ���" << endl;
	}
	else
	{
		cout << "CKernel::dealAddFriendRs:: �ͻ��˴������" << endl;
	}
}

//����ɾ�����ѵ�����
void CKernel::dealDeleteFriendRq(char* recvData, int nLen, long lFrom)
{
	_STRU_TCP_DELETE_FRIEND_RQ* rq = (_STRU_TCP_DELETE_FRIEND_RQ*)recvData;
	m_mapIdToSocket[rq->userId] = lFrom;
	_STRU_TCP_DELETE_FRIEND_RS rs;
	strcpy(rs.friendName, rq->friendName);
	rs.userId = rq->userId;
	
	//�������ѵ�nameȥ������ID
	char sqlBuf[1024];
	list<string>listRes;
	sprintf(sqlBuf,"select id from t_user where name = '%s';",rq->friendName);
	if (!m_sql.SelectMySql(sqlBuf,1,listRes))
	{
		rs.deleteRes = delete_friend_false;
		m_pMediator->SendData((char*)&rs,sizeof(rs),m_mapIdToSocket[rq->userId]);
		return;
	}
	
	if (listRes.size() == 0)
	{
		rs.deleteRes = delete_friend_false;
		m_pMediator->SendData((char*)&rs, sizeof(rs), m_mapIdToSocket[rq->userId]);
		return;
	}
	int fId = stoi(listRes.front());
	rs.friendId = fId;

	//ɾ�����ݿ�ĺ��ѹ�ϵ
	_STRU_TCP_FRIEND_INFO info;
	getInfoFromId(rq->userId,&info);
	sprintf(sqlBuf, "delete  from t_friend where idA = %d and idB = %d;", fId,info.id);
	if (!m_sql.UpdateMySql(sqlBuf))
	{
		rs.deleteRes = delete_friend_false;
		m_pMediator->SendData((char*)&rs, sizeof(rs), m_mapIdToSocket[rq->userId]);
		return;
	}
	sprintf(sqlBuf, "delete  from t_friend where idA = %d and idB = %d;", info.id, fId);
	if (!m_sql.UpdateMySql(sqlBuf))
	{
		rs.deleteRes = delete_friend_false;
		m_pMediator->SendData((char*)&rs, sizeof(rs), m_mapIdToSocket[rq->userId]);
		return;
	}
	rs.deleteRes = delete_friend_success;
	m_pMediator->SendData((char*)&rs, sizeof(rs), m_mapIdToSocket[rq->userId]);

	//�ط�������Ϣ
	getFriendInfo(info.id);
	_STRU_TCP_FRIEND_DELETE_YOU_MSG msg;
	msg.friendId = info.id;
	m_pMediator->SendData((char*)&msg,sizeof(msg),m_mapIdToSocket[fId]);
	if(m_mapIdToSocket.count(rs.friendId) > 0) getFriendInfo(rs.friendId);
												
}

//����ͻ��˵ĵ�¼��֤�ظ�
void CKernel::dealValidateRs(char* recvData, int nLen, long lFrom)
{
	// ���
	_STRU_TCP_VALIDATE_RS* rs = (_STRU_TCP_VALIDATE_RS*)recvData;
	m_mapIdToClientStatus[rs->id] = true;
	// ����
	//if (/*rs->status == false && */m_mapIdToSocket.count(rs->id) > 0)
	//{
	//	auto ite = m_mapIdToSocket.find(rs->id);
	//	if (ite != m_mapIdToSocket.end())
	//	{
	//		m_mapIdToSocket.erase(ite);
	//	}
	//}
	//else if(rs->status)
	//{
	//	m_mapIdToSocket[rs->id] = lFrom;
	//}
	cout << "���ͻ���id: "<<rs->id<<" ������������״̬��" << endl;
}



//�����ļ� ������ת���ļ�����
void CKernel::dealRecvAndResendFile(char* recvData, int nLen, long lFrom)
{
	//�����ļ���Ϣ
	_STRU_TCP_CLIENT_SEND_FILE_RQ* rq = (_STRU_TCP_CLIENT_SEND_FILE_RQ*)recvData;
	int toId = rq->id;
	//�ж��ļ��Ƿ������� ѭ���ȴ���Сʱ
	bool recvfile = false;
	for(int i = 0; i<180;i++ ) 
	{
		if (m_setFileRecvAcc.count(rq->szFileId) > 0)
		{
			recvfile = true;
			break;
		}
		Sleep(10000);
	}
	//�����������ļ�ʧ��
	if (recvfile == false)
	{
		//�ظ��ļ�������һ��ת��ʧ�ܻظ�
		_STRU_TCP_SEND_FILE_RES res;
		res.id = rq->id;
		res.sendRes = send_file_failed_ser_recv;
		res.toFriOrGop = rq->toFriOrGop;
		m_pMediator->SendData((char*)&res,sizeof(res),m_mapIdToSocket[rq->userId]);
		return;
	}

	//����ת���ļ�����
	if (rq->toFriOrGop == file_friend)
	{
		//�жϺ����Ƿ�����
		if (m_mapIdToSocket.count(rq->id) <= 0)
		{
			_STRU_TCP_SEND_FILE_RES res;
			res.id = rq->id;
			res.sendRes = send_file_failed_fri_offline;
			res.toFriOrGop = rq->toFriOrGop;
			m_pMediator->SendData((char*)&res, sizeof(res), m_mapIdToSocket[rq->userId]);
			return;
		}
		//��������
		_STRU_TCP_CLIENT_RECV_FILE_MSG msg;
		msg.fromFriOrGop = rq->toFriOrGop;
		msg.id = rq->id;
		strcpy(msg.szFileId, rq->szFileId);
		m_pMediator->SendData((char*)&msg, sizeof(msg), m_mapIdToSocket[rq->id]);
	}
	else //ת�������Ⱥ����������߳�Ա
	{
		//Ⱥ��û�����ߵĳ�Ա
		if (m_mapGroupIdToMember[rq->id]->size() == 1)
		{
			_STRU_TCP_SEND_FILE_RES res;
			res.id = rq->id;
			res.sendRes = send_file_failed_gro_offline;
			res.toFriOrGop = rq->toFriOrGop;
			m_pMediator->SendData((char*)&res, sizeof(res), m_mapIdToSocket[rq->userId]);
			return;
		}
		auto ite1 = m_mapGroupIdToMember[rq->id]->begin();
		while (ite1 != m_mapGroupIdToMember[rq->id]->end())
		{
			if (m_mapIdToSocket.count(*ite1) > 0)
			{
				_STRU_TCP_CLIENT_RECV_FILE_MSG msg;
				msg.fromFriOrGop = rq->toFriOrGop;
				msg.id = rq->id;
				strcpy(msg.szFileId, rq->szFileId);
				m_pMediator->SendData((char*)&msg, sizeof(msg), m_mapIdToSocket[*ite1]);
			}
			ite1++;
		}
	}
}

//�����ļ����յ���������ϵ��ź�
void CKernel::dealFileRecvAcc(char* recvData, int nLen, long lFrom)
{
	STRU_FILE_RECV_ACC_MSG* msg = (STRU_FILE_RECV_ACC_MSG*)recvData;
	//�����ļ�id��set
	string fileId = msg->szFileId;
	m_setFileRecvAcc.insert(fileId);
	delete msg;
}

//�����ļ������ߵ��ļ����ջظ�
void CKernel::dealFileRecverRs(char* recvData, int nLen, long lFrom)
{
	_STRU_TCP_CLIENT_RECV_FILE_RES* res = (_STRU_TCP_CLIENT_RECV_FILE_RES*)recvData;
	if (res->userRecvRes == user_recv_file_no)
	{
		//�ظ��ļ�������һ���ظ�
	_STRU_TCP_SEND_FILE_RES resSender;
	resSender.id = res->id;
	resSender.sendRes = send_file_failed_fri_refuse;
	resSender.toFriOrGop = res->fromFriOrGop;
	m_pMediator->SendData((char*)&resSender, sizeof(resSender), m_mapIdToSocket[res->userId]);
	}
	else
	{
		//ת���ļ�
		m_pMediator->RefreshSocket(m_mapIdToSocket[res->id]);
		m_pMediator->SendFile(res->szFileId);

		//�ж��ļ��Ƿ�ת����� ѭ���ȴ���Сʱ
		bool recvsend = false;
		for (int i = 0; i < 180; i++)
		{
			if (m_setFileResendAcc.count(res->szFileId) > 0)
			{
				recvsend = true;
				//����ɾ��������ͼ�¼
				m_setFileResendAcc.clear();
				break;
			}
			Sleep(10000);
		}
		//�����������ļ�ʧ��
		if (recvsend == false)
		{
			//�ظ��ļ�������һ��ת��ʧ�ܻظ�
			_STRU_TCP_SEND_FILE_RES resSender;
			resSender.id = res->id;
			resSender.sendRes = send_file_failed_ser_send;
			resSender.toFriOrGop = res->fromFriOrGop;
			m_pMediator->SendData((char*)&resSender, sizeof(resSender), m_mapIdToSocket[res->userId]);
			return;
		}


		//�ظ��ļ�������һ��ת���ɹ��ظ�
		_STRU_TCP_SEND_FILE_RES resSender;
		resSender.id = res->id;
		resSender.sendRes = send_file_succeed;
		resSender.toFriOrGop = res->fromFriOrGop;
		m_pMediator->SendData((char*)&resSender, sizeof(resSender), m_mapIdToSocket[res->userId]);
	}
}

//�����ļ��ɷ�����ת����ϵ��ź�
void CKernel::dealFileResendAcc(char* recvData, int nLen, long lFrom)
{
	STRU_FILE_RESEND_ACC_MSG* msg = (STRU_FILE_RESEND_ACC_MSG*)recvData;
	//�����ļ�id��set
	string fileId = msg->szFileId;
	m_setFileResendAcc.insert(fileId);
	delete msg;
}

//��ʼ��Э��ͷ����
void CKernel::setProtocolMap()
{
	cout << __func__ << endl;
	memset(m_mapPackTypeToFun, 0, sizeof(m_mapPackTypeToFun));
	
	m_mapPackTypeToFun[_DEF_TCP_REGISTER_RQ -(_DEF_PROTOCOL_BASE + 1)]
		= &CKernel::dealRegisterRq;
	m_mapPackTypeToFun[_DEF_TCP_LOGIN_RQ-(_DEF_PROTOCOL_BASE + 1)]
		= &CKernel::dealLoginRq;
	m_mapPackTypeToFun[_DEF_TCP_OFFLINE_RQ-(_DEF_PROTOCOL_BASE + 1)]
		= &CKernel::dealOfflineRq;
	m_mapPackTypeToFun[_DEF_TCP_CHAT_RQ-(_DEF_PROTOCOL_BASE + 1)]
		= &CKernel::dealChatRq;
	m_mapPackTypeToFun[_DEF_TCP_ADD_FRIEND_RQ-(_DEF_PROTOCOL_BASE + 1)]
		= &CKernel::dealAddFriendRq;
	m_mapPackTypeToFun[_DEF_TCP_ADD_FRIEND_RS-(_DEF_PROTOCOL_BASE + 1)]
		= &CKernel::dealAddFriendRs;
	m_mapPackTypeToFun[_DEF_TCP_VALIDATE_LOGINED_RS-(_DEF_PROTOCOL_BASE + 1)]
		= &CKernel::dealValidateRs;
	m_mapPackTypeToFun[_DEF_TCP_DELETE_FRIEND_RQ-(_DEF_PROTOCOL_BASE + 1)]
		= &CKernel::dealDeleteFriendRq;    
	m_mapPackTypeToFun[_DEF_TCP_GROUP_CHAT_MSG-(_DEF_PROTOCOL_BASE + 1)]
		= &CKernel::dealGroupMsg;
	m_mapPackTypeToFun[_DEF_TCP_SEND_FILE_RQ -(_DEF_PROTOCOL_BASE + 1)]
		= &CKernel::dealRecvAndResendFile;
	m_mapPackTypeToFun[_DEF_PROTOCOL_FILE_RECV_ACC-(_DEF_PROTOCOL_BASE + 1)]
		= &CKernel::dealFileRecvAcc;
	m_mapPackTypeToFun[_DEF_TCP_RECV_FILE_RES - (_DEF_PROTOCOL_BASE + 1)]
		= &CKernel::dealFileRecverRs;
	m_mapPackTypeToFun[_DEF_PROTOCOL_FILE_RESEND_ACC - (_DEF_PROTOCOL_BASE + 1)]
		= &CKernel::dealFileResendAcc;
	//m_mapPackTypeToFun[ - (_DEF_PROTOCOL_BASE + 1)]
		//= &CKernel::;
	//m_mapPackTypeToFun[ - (_DEF_PROTOCOL_BASE + 1)]
		//= &CKernel::;
	//m_mapPackTypeToFun[ - (_DEF_PROTOCOL_BASE + 1)]
		//= &CKernel::;
	//m_mapPackTypeToFun[ - (_DEF_PROTOCOL_BASE + 1)]
		//= &CKernel::;
	//m_mapPackTypeToFun[ - (_DEF_PROTOCOL_BASE + 1)]
		//= &CKernel::;
	//m_mapPackTypeToFun[ - (_DEF_PROTOCOL_BASE + 1)]
		//= &CKernel::;
}


