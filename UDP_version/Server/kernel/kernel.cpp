#include".\\kernel.h"
#include<iostream>
#include <process.h>
#include<random>
using namespace std;

static random_device rd;
kernel* kernel::m_pKernel = 0;
extern int nOnlineClients;


kernel::kernel()
{
	//nOnlineClients = 0;
	memset(m_arrTypeToFun, 0, sizeof(m_arrTypeToFun));
	m_mapIdToState.clear();
	m_mapNoToAdd.clear();
	m_mapGroInfo.clear();
	m_mapIdToNo.clear();
	m_handle = 0;
	setArrayTypeToFun();
	m_pKernel = this;
	startService();
}
kernel::~kernel()
{
	closeService();
	delete pToolUdp;
}

//�򿪷�����
bool kernel::startService()
{
	pToolUdp = new udpnet;
	if (!pToolUdp) 
		return false;
	if (pToolUdp->InitNet())
	{
		cout << "kernel:: startServer netInit succeed" << endl;
	}
	else
	{
		cout << "kernel:: startServer failed" << endl;
	}

	//�������ݿ�
	if (!m_sql.ConnectMySql("127.0.0.1", "root", "colin123456", "WeChat_DBS"))
	{
		cout << "kernel:: startServer connect sql failed" << endl;
		cout << "kernel:: startServer failed" << endl;
		return false;
	}
	else
	{
		cout << "kernel:: startServer connect sql succeed" << endl;
		cout << "kernel:: startServer succeed" << endl;
		return true;
	}
	return false;
}

//�رշ�����
void kernel::closeService()
{
	if (!pToolUdp)
		return;
	if (pToolUdp->CloseNet())
	{
		cout << "kernel:: closeServer succeed" << endl;
	}
	else
	{
		cout << "kernel:: closeServer failed" << endl;
	}
	return;
}

//���Ĺ���
void kernel::nuclearWork(char* recvData, sockaddr_in fromAddr)
{
	cout << "kernel:: nuclearWork working..." << endl;
	cout << "kernel:: nuclearWork msg.buf1: " << ((recvBuf*)recvData)->buf1;
	int type = *(int*)recvData;
	cout << " type:" << type << endl;
	if (type != TYPE_ALIVE && (type <= 0 || type > _DEF_PROTO_SIZE_))
	{
		cout << "kernel:: nuclearWork recv type wrong" << endl;
		return;
	}
	pWork p = m_arrTypeToFun[type];
	if (!p)
	{
		cout << "kernel:: nuclearWork p==nullptr" << endl;
		return;
	}
	(this->*p)(recvData, fromAddr);
}

//����ע������
void kernel::dealRegRq(char* recvData, sockaddr_in fromAddr)
{
	cout << endl;
	cout << "[ kernel:: dealRegRq tel/email: "<<((recvBuf*)recvData)->buf1 <<" ]" << endl;
	cout << "[ kernel:: dealRegRq password: "<<((recvBuf*)recvData)->buf2 <<" ]" << endl;
	cout << endl;

	// 1 ���
	recvBuf* rq = (recvBuf*)recvData;

	//msg.data1 0���ֻ���ע�ᣬ1������ע��
	if (rq->data1 == 1)
	{
		//��������ʽ

		//�����ݿ�鿴�Ƿ�ע��
		
		//д���ݿ�

		//�ظ�ע����

		return;
	}

	//�ֻ���ע��
	
	// 2 У���ֻ��ŵĺϷ���
	string tel = rq->buf1;
	string tel_temp = rq->buf1;
	string password = rq->buf2;
	string password_temp = rq->buf2;
	sendBuf msgrs;
	msgrs.type = _DEF_REG_RS_;
	msgrs.res1 = false;									 //res1��ע����
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
		cout << "kernel:: dealRegRq ע����Ϣ����Ϊ�հף�������" << endl;
		strcpy_s(msgrs.buf1, "ע����Ϣ����Ϊ�հ�");						//buf1�Ǵ�����
		pToolUdp->SendData((char*)&msgrs,fromAddr);
		return;
	}
	//��鳤���Ƿ�Ϸ���tel = 11,password 3-15��
	if (tel.length() != 11 || password.length() > 15 || password.length() < 3)
	{
		cout << "kernel:: dealRegRq ע����Ϣ���볤������������" << endl;
		strcpy_s(msgrs.buf1, "ע����Ϣ���볤������");						//buf1�Ǵ�����
		pToolUdp->SendData((char*)&msgrs,fromAddr);
		return;
	}
	//��������Ƿ�Ϸ�(telΪ���� name passwordΪ��Сд��ĸ ���� �»��ߵ����)
	if (0)
	{
		cout << "kernel:: dealRegRq ע����Ϣ�����ʽ����������" << endl;
		strcpy_s(msgrs.buf1, "ע����Ϣ�����ʽ����");						//buf1�Ǵ�����
		pToolUdp->SendData((char*)&msgrs,fromAddr);
		return;
	}

	sendBuf msg;
	//�����ݿ�鿴�Ƿ�ע��
	list<string> listRes;
	char sqlBuf[1024];
	sprintf(sqlBuf,"select tel from t_user where tel = '%s';",rq->buf1);
	if (!m_sql.SelectMySql(sqlBuf,1,listRes))
	{
		//��ѯʧ��
		cout << "kernel:: dealRegRq ���ݿ��ѯʧ��" << endl;

		//��һ��ע��ظ���
		msg.type = _DEF_REG_RS_;							
		msg.res1 = false;									//res1��ע����
		strcpy_s(msg.buf1,"ע���ѯ���ݿ�ʱ����������");  //buf1�Ǵ�����
		pToolUdp->SendData((char*)&msg,fromAddr);
		return;
	}
	if (listRes.size() > 0)
	{
		//��ע�����

		cout << "kernel:: dealRegRq �ֻ����ѱ�ע��" << endl;
		msg.type = _DEF_REG_RS_;
		msg.res1 = false;						//res1��ע����
		strcpy_s(msg.buf1,"�ֻ����ѱ�ע��");	//buf1�Ǵ�����
		pToolUdp->SendData((char*)&msg,fromAddr);
		return;
	}

	//д�����ݿ�
	sprintf(sqlBuf, "insert into t_user (tel,password) values ('%s','%s');",rq->buf1,rq->buf2);
	if (!m_sql.UpdateMySql(sqlBuf))
	{
		cout << "kernel:: dealRegRq ���û�д�����ݿ�ʧ��" << endl;
		msg.type = _DEF_REG_RS_;
		msg.res1 = false;						    //res1��ע����
		strcpy_s(msg.buf1, "ע��ʱд�����ݿ�ʧ��");	//buf1�Ǵ�����
		pToolUdp->SendData((char*)&msg,fromAddr);
		return;
	}

	//��������û���id ��������ȥ
	sprintf(sqlBuf, "select id from t_user where tel = '%s';", rq->buf1);
	if (!m_sql.SelectMySql(sqlBuf, 1, listRes))
	{
		//��ѯʧ��
		cout << "kernel:: dealRegRq д����ѯidʧ��" << endl;

		//��һ��ע��ظ���
		msg.type = _DEF_REG_RS_;
		msg.res1 = false;									 //res1��ע����
		strcpy_s(msg.buf1, "ע���ѯ���ݿ�ʱ����������");  //buf1�Ǵ�����
		pToolUdp->SendData((char*)&msg,fromAddr);
		return;
	}
	if (listRes.size() == 0)
	{
		cout << "kernel:: dealRegRq ��ѯid�ɹ� listResΪ��" << endl;
		return;
	}
	msg.data1 = stoi(listRes.front());			//data1��ע��ɹ����id

	//д���ע��ɹ�
	cout << "kernel:: dealRegRq ���û�ע��ɹ�" << endl;
	msg.type = _DEF_REG_RS_;
	msg.res1 = true;						    //res1��ע����
	strcpy_s(msg.buf1, "ע��ɹ�����¼��");	    //buf1�Ǵ�����
	pToolUdp->SendData((char*)&msg,fromAddr);
	return;
}

//�����¼����,�ظ��ܷ��¼ �ܵĻ��ظ���������Ϣ Ⱥ����Ϣ Ⱥ�ĳ�Ա��Ϣ
void kernel::dealLogRq(char* recvData, sockaddr_in fromAddr)
{
	cout << endl;
	cout << "[ kernel:: dealLogRq tel/email: " << ((recvBuf*)recvData)->buf1 << " ]" << endl;
	cout << "[ kernel:: dealLogRq password: " << ((recvBuf*)recvData)->buf2 << " ]" << endl;
	cout << endl;

	// 1 ���
	recvBuf* rq = (recvBuf*)recvData;

	//msg.data1 0���ֻ��ŵ�¼��1�������¼
	if (rq->data1 == 1)
	{
		cout << "[ kernel:: dealLogRq �����˺ŵ�¼��... ]" << endl;
		//��������ʽ

		//�����ݿ�鿴�Ƿ��¼

		//д���ݿ�

		//�ظ���¼���

		return;
	}

	//�ֻ��ŵ�¼
	cout << "[ kernel:: dealLogRq �ֻ��ŵ�¼��... ]" << endl;

	// 2 У���ֻ��ŵĺϷ���
	string tel = rq->buf1;
	string tel_temp = rq->buf1;
	string password = rq->buf2;
	string password_temp = rq->buf2;
	sendBuf msgrs;
	msgrs.type = _DEF_LOG_RS_;
	msgrs.res1 = false;									 //res1��ע����
	//�ж��Ƿ��ǿ��ַ��� ������ȫ�ո�
	int iter = 0;

	while (iter != -1)
	{
		iter = tel_temp.find(" ");
		if (iter != -1) tel_temp.erase(iter, iter + 1);
	}
	while (iter != -1)
	{
		iter = password_temp.find(" ");
		if (iter != -1) password_temp.erase(iter, iter + 1);
	}
	if (password.empty() || tel.empty()
		|| password_temp.empty()
		|| tel_temp.empty())
	{
		cout << "kernel:: dealLOGRq ��¼��Ϣ����Ϊ�հף�������" << endl;
		strcpy_s(msgrs.buf1, "��¼��Ϣ����Ϊ�հ�");						//buf1�Ǵ�����
		pToolUdp->SendData((char*)&msgrs,fromAddr);
		return;
	}
	//��鳤���Ƿ�Ϸ���tel = 11,password 3-15��
	if (tel.length() != 11 || password.length() > 15 || password.length() < 3)
	{
		cout << "kernel:: dealLOGRq ��¼��Ϣ���볤������������" << endl;
		strcpy_s(msgrs.buf1, "��¼��Ϣ����Ϊ�հ�");						//buf1�Ǵ�����
		pToolUdp->SendData((char*)&msgrs,fromAddr);
		return;
	}
	//��������Ƿ�Ϸ�(telΪ���� name passwordΪ��Сд��ĸ ���� �»��ߵ����)
	if (0)
	{
		cout << "kernel:: dealLOGRq ��¼��Ϣ�����ʽ����������" << endl;
		strcpy_s(msgrs.buf1, "��¼��Ϣ����Ϊ�հ�");						//buf1�Ǵ�����
		pToolUdp->SendData((char*)&msgrs,fromAddr);
		return;
	}

	sendBuf msg;
	list<string> listRes;
	char sqlBuf[1024];

	//��ѯ�û�id,email,password,name,iconId,sign
	sprintf(sqlBuf, "select id,email,password,name,iconId,sign from t_user where tel = '%s';", rq->buf1);
	if (!m_sql.SelectMySql(sqlBuf, 6, listRes))
	{
		//��ѯʧ��
		cout << "kernel:: dealLogRq ���ݿ��ѯʧ��" << endl;

		//��һ����¼�ظ���
		msg.type = _DEF_LOG_RS_;
		msg.res1 = false;									//res1�ǵ�¼���
		strcpy_s(msg.buf1, "��¼��ѯ���ݿ�ʱ����������");  //buf1�Ǵ�����
		pToolUdp->SendData((char*)&msg,fromAddr);
		return;
	}

	//�鿴�û��Ƿ�ע��
	if (listRes.size() == 0)
	{
		cout << "kernel:: dealLOGRq �ֻ���δע��" << endl;
		msg.type = _DEF_LOG_RS_;
		msg.res1 = false;										//res1�ǵ�¼���
		strcpy_s(msg.buf1, "�ֻ���δע�ᣬ����ע��");			//buf1�Ǵ�����
		pToolUdp->SendData((char*)&msg,fromAddr);
		return;
	}

	//����id�鿴�������Ƿ��¼
	int id = stoi(listRes.front());
	if (listRes.size() > 0 && m_mapIdToState.count(id) > 0)
	{
		//����¼����

		cout << "kernel:: dealLogRq �ֻ����ѵ�¼" << endl;
		msg.type = _DEF_LOG_RS_;
		msg.res1 = false;						//res1�ǵ�¼���
		strcpy_s(msg.buf1, "�ֻ����ѱ���¼");	//buf1�Ǵ�����
		pToolUdp->SendData((char*)&msg,fromAddr);
		return;
	}
	
	//ժȡ��Ϣ
	string  email, pwd, name, iconId, sign;
	email = pwd = name = iconId = sign = "";
	list<string>::iterator ite = listRes.begin();
	for(int i = 0; i < 5 && ite != listRes.end() ;i++)
	{
		if (i == 0)	email = *(++ite);
		else if (i == 1) pwd = *(++ite);
		else if (i == 2) name = *(++ite);
		else if (i == 3) iconId = *(++ite);
		else if (i == 4) sign = *(++ite);
	}
	//��֤����
	if (pwd != password)
	{
		cout << "kernel:: dealLOGRq ������� ��¼ʧ��" << endl;
		msg.type = _DEF_LOG_RS_;
		msg.res1 = false;						//res1�ǵ�¼���
		strcpy_s(msg.buf1, "�������");			//buf1�Ǵ�����
		pToolUdp->SendData((char*)&msg,fromAddr);
		return;
	}

	//��¼�ɹ�
	msg.type = _DEF_LOG_RS_;
	msg.data1 = id;								//data1���¼�ɹ����id
	msg.data2 = stoi(iconId);					//data2��iconId
	msg.res1 = true;						    //res1�ǵ�¼���
	msg.res2 = 0;								//res2�ǵ�¼��ʽ 0�ֻ��� 1����
	strcpy_s(msg.buf1, "��¼�ɹ�����ӭʹ��");	//buf1�Ǵ�����
	strcpy_s(msg.buf2,email.c_str());			//buf2��email
	strcpy_s(msg.buf3,name.c_str());			//buf3��name
	strcpy_s(msg.buf4,sign.c_str());			//buf4��sign
	pToolUdp->SendData((char*)&msg,fromAddr);
	cout <<"id,email,name,sign: " <<id<<" " << email << " " << name << " " << sign << endl;

	//д���������¼
	m_mapIdToState[id] = true;
	m_mapIdToNo[id] = rq->no;
	cout << "kernel:: dealLOGRq �û���¼�ɹ�" << endl;

	//��¼�ɹ����͸���������Ϣ��Ⱥ����Ϣ�����������к�������Ⱥ��Ա��������
	msg.type = _DEF_FRI_INFO_;
	msg.res1 = true;
	strcpy_s(msg.buf1, rq->buf1);	//�ֻ��ſ���ȥ
	ConnectRelationships(id,&msg);

	return;
}

//��¼�ɹ����͸���������Ϣ��Ⱥ����Ϣ�����������к�������Ⱥ��Ա��������
void kernel::ConnectRelationships(int id,sendBuf* userInfo)
{
	int no = m_mapIdToNo[id];
	if (m_mapIdToState.count(id) <= 0)
		return;
	//��ѯ���м�������,��¼����id
	list<string>listRes;
	list<string>listInfo;
	char sqlbuf[1024] = "";
	sprintf(sqlbuf,"select id2 from t_friend where id1 = %d;",id);
	if (!m_sql.SelectMySql(sqlbuf, 1, listRes))
	{
		cout << "kernel:: ConnectRelationships ��ѯ����idʧ��" << endl;
		return;
	}
	sendBuf friInfo;
	string tel, email, name, sign;
	int iconId;
	while (listRes.size() > 0)
	{
		int friId = stoi(listRes.back());
		listRes.pop_back();
		
		sprintf(sqlbuf,"select tel,email,name,iconId,sign from t_user where id = %d;",friId);
		if (!m_sql.SelectMySql(sqlbuf, 5, listInfo))
		{
			cout << "kernel:: ConnectRelationships ��ѯһ��������Ϣʧ��" << endl;
			return;
		}
		if (listInfo.size() > 0)
		{
			tel = listInfo.front();
			listInfo.pop_front();
		}
		if (listInfo.size() > 0)
		{
			email = listInfo.front();
			listInfo.pop_front();
		}
		if (listInfo.size() > 0)
		{
			name = listInfo.front();
			listInfo.pop_front();
		}
		if (listInfo.size() > 0)
		{
			iconId = stoi(listInfo.front());
			listInfo.pop_front();
		}
		if (listInfo.size() > 0)
		{
			sign = listInfo.front();
			listInfo.pop_front();
		}
		friInfo.type = _DEF_FRI_INFO_;
		if (m_mapIdToState.count(friId) > 0)
			friInfo.res1 = true;				//��������
		else
			friInfo.res1 = false;
		friInfo.data1 = friId;
		friInfo.data2 = iconId;
		strcpy_s(friInfo.buf1, tel.c_str());
		strcpy_s(friInfo.buf2, email.c_str());
		strcpy_s(friInfo.buf3, name.c_str());
		strcpy_s(friInfo.buf4, sign.c_str());

		pToolUdp->SendData((char*)&friInfo,m_mapNoToAdd[no]);
		if(m_mapIdToState.count(friId) > 0 && friId != id)							 //����������һ�ݸ�����
			pToolUdp->SendData((char*)userInfo, m_mapNoToAdd[m_mapIdToNo[friId]]);
		cout << "kernel::ConnectRelationships ����һ��������Ϣ��id: " << id << ", ������Ϣ: id" 
			<< friId << " icon" << iconId << " name:"<<name <<" tel:" << tel << " email:" << email << " sign:" << sign << endl;
	}


	//��ѯ���м���Ⱥ��,��¼Ⱥ��id
	listRes.clear();
	listInfo.clear();
	memset(sqlbuf,0,1024);
	sprintf(sqlbuf, "select groupId from t_groupmem where memId = %d;", id);
	if (!m_sql.SelectMySql(sqlbuf, 1, listRes))
	{
		cout << "kernel:: ConnectRelationships ��ѯȺ��idʧ��" << endl;
		return;
	}
	sendBuf groInfo;
	string groupNum, groupName, gSign;
	int gIcon = 0;
	list<string>::iterator ite = listRes.begin();
	while (ite != listRes.end() && listRes.size() > 0)
	{
		int groId = stoi(*ite);
		int gmembNums = 0;

		//��������¼Ⱥ���߳�Ա��Ϣ
		m_mapGroInfo[groId].push_back(id);

		list<string> listNums;
		sprintf(sqlbuf, "select groupNum,groupName,iconId,sign from t_group where groupid = %d;", groId);
		if (!m_sql.SelectMySql(sqlbuf, 4, listInfo))
		{
			cout << "kernel:: ConnectRelationships ��ѯһ��Ⱥ����Ϣʧ��" << endl;
			return;
		}
		sprintf(sqlbuf, "select count(*) from t_groupmem where groupid = %d;", groId);
		if (!m_sql.SelectMySql(sqlbuf, 1, listNums))
		{
			cout << "kernel:: ConnectRelationships ��ѯһ��Ⱥ��memNumsʧ��" << endl;
			return;
		}
		if(listNums.size() > 0)
			gmembNums = stoi(listNums.front());	//���list��С  
		if (listInfo.size() > 0)
		{
			groupNum = listInfo.front();
			listInfo.pop_front();
		}
		if (listInfo.size() > 0)
		{
			groupName = listInfo.front();
			listInfo.pop_front();
		}
		if (listInfo.size() > 0)
		{
			gIcon = stoi(listInfo.front());
			listInfo.pop_front();
		}
		if (listInfo.size() > 0)
		{
			gSign = listInfo.front();
			listInfo.pop_front();
		}

		groInfo.type = _DEF_GRO_INFO_;
		groInfo.data1 = groId;
		groInfo.data2 = gIcon;
		groInfo.data3 = gmembNums;	//group size
		strcpy_s(groInfo.buf1, groupNum.c_str());
		strcpy_s(groInfo.buf2, groupName.c_str());
		strcpy_s(groInfo.buf3, gSign.c_str());

		//���Լ�����Ⱥ����Ϣ
		pToolUdp->SendData((char*)&groInfo, m_mapNoToAdd[no]);
		cout << "kernel::ConnectRelationships ����һ��Ⱥ����Ϣ��id: " << id << ", Ⱥ��Ϣ: id"
			<< groId << " icon" << gIcon << " name:" << groupName <<" size:"<<gmembNums << " num:" << groupNum << " gSign:" << gSign << endl;


		//�����е�Ⱥ�������û������Լ�����Ϣ ���Լ���������Ⱥ�ĳ�Ա����Ϣ
		//list<int>::iterator iteMem = m_mapGroInfo[groId].begin();
		//while (iteMem != m_mapGroInfo[groId].end())
		//{
		//	int memId = *iteMem;
		//	//�жϳ�Ա�Ƿ�����
		//	if (memId != id && m_mapIdToState.count(memId) > 0)
		//		pToolUdp->SendData((char*)userInfo,m_mapIdToAdd[memId]);
		//}
		ite++;
	}

	//���û���������Ⱥ��Ա����Ϣ
	sendBuf memInfo;
	ite = listRes.begin();
	while (ite != listRes.end() && listRes.size() > 0)
	{
		int gId = stoi(*ite);
		//��ѯ����Ⱥ�ĳ�Աid;
		list<string>listMemIds;
		sprintf(sqlbuf, "select memid from t_groupmem where groupid = %d;", gId);
		if (!m_sql.SelectMySql(sqlbuf,1,listMemIds))
		{
			cout << "kernel:: ConnectRelationships ��ѯȺ�ĵ����г�Աidʧ��" << endl;
		}

		//�ó�Աid��ѯ��Ա��Ϣ �����͸��Լ�
		//���Լ�����Ϣ���͸�ÿһ�����ߵ�Ⱥ��Ա
		for (string str : listMemIds)
		{
			int memId = stoi(str);
			if (memId == id)
				continue;
			if (getInfoFromId(memId, &memInfo))
			{
				memInfo.type = _DEF_GROMEM_INFO_;
				memInfo.data3 = gId;						//data3���Ա����Ⱥ�ĵ�id
				if (m_mapIdToState.count(memId) > 0)		//�ж�����״̬
					memInfo.res1 = true;
				else
					memInfo.res1 = false;


				//��Ա��Ϣ ���͸��Լ�
				pToolUdp->SendData((char*)&memInfo, m_mapNoToAdd[no]);
				cout << "kernel::ConnectRelationships ����һ��Ⱥ��Ա��Ϣ��id: " << id << ", ��Ա��Ϣ: id"
					<< memId <<" gid"<<memInfo.data3 << " icon" << memInfo.data2 << " name:" << memInfo.buf3 
					<< " tel:" << memInfo.buf1 << " email:" << memInfo.buf2 << " sign:" << memInfo.buf4 << endl;
				
				//���Լ�����Ϣ���͸�ÿһ�����ߵ�Ⱥ��Ա
				if (m_mapIdToState.count(memId) > 0 && memId != id)
				{
					userInfo->type = _DEF_GROMEM_INFO_;
					pToolUdp->SendData((char*)userInfo,m_mapNoToAdd[m_mapIdToNo[memId]]);
				}
			}
			else
			{
				cout << "kernel:: ConnectRelationships ��ѯȺ��id: "<<gId<<",��һ����Ա��Ϣʧ��, memId: "<<memId << endl;
			}
		}
		ite++;
	}
}


//ת��Ⱥ����Ϣ
void kernel::dealGroupMsg(char* recvData, sockaddr_in fromAddr)
{
	groMsg* rq = (groMsg*)recvData;
	//ת��Ⱥ����Ϣ
	int gId = rq->gId;

	//���͸��������Ⱥ���ߵĳ�Ա
	auto ite = m_mapGroInfo[gId].begin();
	while (ite != m_mapGroInfo[gId].end())
	{
		if (*ite != rq->fromId && m_mapIdToState.count(*ite) > 0)
		{
			pToolUdp->SendData(recvData, m_mapNoToAdd[m_mapIdToNo[*ite]]);
			cout << "������Ⱥ��Ϣ������:" << rq->fromId << " ����Ⱥ:" << rq->gId << " msg:" << rq->buf << endl;
		}
		ite++;
	}
	if (ite == m_mapGroInfo[gId].begin())
	{
		rq->res = false;
		sprintf(rq->buf, "Ⱥid:%d ��ֻ��������Ŷ/Ⱥ��Աֻ�����Լ�����������˰�",rq->gId);
		pToolUdp->SendData(recvData, fromAddr);
		cout << "����Ⱥ��Ϣʧ�ܣ���������/ֻ��һ����Ա������:" << rq->fromId << " ����Ⱥ:" << rq->gId << " msg:" << rq->buf << endl;
	}
}

//�������� ת��������Ϣ
void kernel::dealChatMsg(char* recvData, sockaddr_in fromAddr)
{
	friMsg* rq = (friMsg*)recvData;
	int fromId = rq->fromId;
	//ת��������Ϣ
	int toId = rq->toId;
	//�жϺ����Ƿ�����
	if (m_mapIdToState.count(toId) <= 0)
	{
		rq->res = false;
		sprintf(rq->buf, "����id:%d ����",rq->toId);
		pToolUdp->SendData(recvData, fromAddr);
		cout << "���ͺ�����Ϣʧ�ܣ��������ߣ�����:" << rq->fromId 
			<< " ��������:" << rq->toId << " msg:" << rq->buf << endl;
		return;
	}
	pToolUdp->SendData((char*)recvData, m_mapNoToAdd[m_mapIdToNo[toId]]);
	cout << "�����˺�����Ϣ������:" << rq->fromId
		<< " ��������:" << rq->toId << " msg:" << rq->buf << endl;
}

//������Ӻ�������
void kernel::dealAddFriendRq(char* recvData, sockaddr_in fromAddr)
{
	
}

//�ظ���Ӻ���
void kernel::dealAddFriendRs(char* recvData, sockaddr_in fromAddr)
{

}
//����ɾ�����ѵ�����
void kernel::dealDeleteFriendNoti(char* recvData, sockaddr_in fromAddr)
{

}

//��������֪ͨ
void kernel::dealOfflineNoti(char* recvData, sockaddr_in fromAddr)
{

}

//�����������
void kernel::keepAlive(char* recvData, sockaddr_in fromAddr)
{
	auto ite = m_mapNoToAdd.begin();
	while (ite != m_mapNoToAdd.end())
	{
		cout << "*** kernel_map:: No: "<<ite->first<<" IP:"<<inet_ntoa(ite->second.sin_addr) <<" PORT:"<<ntohs(ite->second.sin_port) << endl;
		ite++;
	}
	recvBuf* msg = (recvBuf*)recvData;
	cout << endl<<"*** Client no:" << msg->no<<" Clients:"<<nOnlineClients << endl << endl;

	if (msg->no == -1)					//�¿ͻ�����һ������ ��¼addr �����ͻ�������ȥ���ķ��������
	{
		//��������no��addr
		m_mapNoToAdd[nOnlineClients] = fromAddr;
		sendBuf noMsg;
		noMsg.type = _DEF_CLIENT_NO_;
		noMsg.no = nOnlineClients;
		strcpy_s(noMsg.buf1, "��ӭ���ĵ�һ�����ߣ�");
		pToolUdp->SendData((char*)&noMsg, fromAddr);

		cout << "<< kernel:: keepAlive  Client first connect!	 IP: " << inet_ntoa(fromAddr.sin_addr)
			<< " PORT: " << ntohs(fromAddr.sin_port) << endl;
		++nOnlineClients;
		return;
	}

	//�ǵ�һ������ �ж������������ӻ��Ƕ������� �����Ļ�����addr
	sockaddr_in localAddr;
	if (m_mapNoToAdd.count(msg->no) > 0 )
	{
		localAddr = m_mapNoToAdd[msg->no];
		if (localAddr.sin_family == fromAddr.sin_family && localAddr.sin_port == fromAddr.sin_port
			&& localAddr.sin_addr.S_un.S_addr == fromAddr.sin_addr.S_un.S_addr)
		{
			cout << "<< kernel:: keepAlive user_id: [" << msg->data1 << "] is living...	 IP: " << inet_ntoa(fromAddr.sin_addr)
				<< " PORT: " << ntohs(fromAddr.sin_port) << endl;
		}
		else
		{
			cout << "<< kernel:: keepAlive �ͻ���id: ["<<msg->data1<<"] ������������ַ�Ѵ��뱾��map. IP: " << inet_ntoa(fromAddr.sin_addr)
				<< " PORT: " << ntohs(fromAddr.sin_port) << endl;
			m_mapNoToAdd[msg->no] = fromAddr;
		}
	}
}

//��¼��¼�ɹ��Ŀͻ�����addr
void kernel::recordAddr(char* recvData, sockaddr_in addr)
{
	recvBuf* info = (recvBuf*)recvData;
	int no = info->no;
	if (m_mapNoToAdd.count(no) > 0)
	{
		m_mapNoToAdd[no] = addr;
		cout << "kernel:: recordAddr �ͻ�����¼�ɹ���addr�Ѹ���; id: " << info->data1 << endl;
	}
	/*else
	{
		m_mapNoToAdd[no] = addr;
		cout << "kernel:: recordAddr ����¿ͻ���addr�ɹ���id: "<<info->data1 << endl;
	}*/
}

//�����û�id��ȡ���ĺ�����Ϣ
void kernel::getAllFriendInfo(int)
{

}

//����id��ȡ������Ϣ
//������info�� res1������״̬ data1��id data2��iconID buf1��tel buf2��email buf3��name buf4��sign
//info��Э����Ҫ��һ������
bool kernel::getInfoFromId(int id, sendBuf* info)
{
	list<string> listInfo;
	char sqlbuf[1024];
	string tel, email, name, sign;
	int iconId = 0;
	sprintf(sqlbuf, "select tel,email,name,iconId,sign from t_user where id = %d;", id);
	if (!m_sql.SelectMySql(sqlbuf, 5, listInfo))
	{
		cout << "kernel:: getInfoFromId ��ѯһ��person��Ϣʧ��" << endl;
		return false;
	}
	if (listInfo.size() == 5)
	{
		tel = listInfo.front();
		listInfo.pop_front();
		email = listInfo.front();
		listInfo.pop_front();
		name = listInfo.front();
		listInfo.pop_front();
		iconId = stoi(listInfo.front());
		listInfo.pop_front();
		sign = listInfo.front();
		listInfo.pop_front();
	}
	else
		return false;
	if (m_mapIdToState.count(id) > 0)
		info->res1 = true;				//��������
	else
		info->res1 = false;
	info->data1 = id;
	info->data2 = iconId;
	strcpy_s(info->buf1, tel.c_str());
	strcpy_s(info->buf2, email.c_str());
	strcpy_s(info->buf3, name.c_str());
	strcpy_s(info->buf4, sign.c_str());

	return true;
}



//��ʼ��������������
void kernel::setArrayTypeToFun()
{
	m_arrTypeToFun[_DEF_REG_RQ_] = &kernel::dealRegRq;
	m_arrTypeToFun[_DEF_LOG_RQ_] = &kernel::dealLogRq;
	m_arrTypeToFun[_DEF_CHAT_MSG_] = &kernel::dealChatMsg;
	m_arrTypeToFun[_DEF_ADD_FRIEND_RQ_] = &kernel::dealAddFriendRq;
	m_arrTypeToFun[_DEF_ADD_FRIEND_RS_] = &kernel::dealAddFriendRs;
	m_arrTypeToFun[_DEF_DELETE_FRIEND_NOTICE_] = &kernel::dealDeleteFriendNoti;
	m_arrTypeToFun[_DEF_OFFLINE_NOTICE_] = &kernel::dealOfflineNoti;
	m_arrTypeToFun[TYPE_ALIVE] = &kernel::keepAlive;
	m_arrTypeToFun[TYPE_RECORD_ADDR] = &kernel::recordAddr;
	m_arrTypeToFun[_DEF_CHAT_MSG_] = &kernel::dealChatMsg;
	m_arrTypeToFun[_DEF_CHAT_GRO_MSG_] = &kernel::dealGroupMsg;
	//m_arrTypeToFun[_DEF_REG_RQ_] = dealRegRq;
	//m_arrTypeToFun[_DEF_REG_RQ_] = dealRegRq;
	//m_arrTypeToFun[_DEF_REG_RQ_] = dealRegRq;
	//m_arrTypeToFun[_DEF_REG_RQ_] = dealRegRq;
	//m_arrTypeToFun[_DEF_REG_RQ_] = dealRegRq;
	//m_arrTypeToFun[_DEF_REG_RQ_] = dealRegRq;
	//m_arrTypeToFun[_DEF_REG_RQ_] = dealRegRq;
	//m_arrTypeToFun[_DEF_REG_RQ_] = dealRegRq;
	//m_arrTypeToFun[_DEF_REG_RQ_] = dealRegRq;
}