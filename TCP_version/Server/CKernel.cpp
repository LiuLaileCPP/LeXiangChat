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

//打开服务器
bool CKernel::startService()
{
	cout << __func__ << endl;
	m_pMediator = new TcpServiceMediator;

	//打开网络
	if (!m_pMediator->OpenNet())
	{
		cout << "CKernel::startService() 打开网络失败" << endl;
		return false;
	}

	//连接到数据库
	if (!m_sql.ConnectMySql("127.0.0.1","root","colin123456","ims_tcp")) //root用户是本地环网地址
	{
		cout << "CKernel::startService() 连接数据库失败" << endl;
		return false;
	}
	cout << "CKernel::startService() 打开网络成功, 连接数据库成功..." << endl;

	//创建接收数据的线程
	m_handle = (HANDLE)_beginthreadex(0//安全级别 0是默认
		, 0           //堆栈大小 0是默认
		, &ValidateClientLogin //起始地址
		, this       //参数列表
		, 0	         //线程状态 0是创建即运行);
		, 0			//分配的线程ID
	);
	return true;
}

//循环验证每一台在线客户机的登录状态
unsigned __stdcall CKernel::ValidateClientLogin(void* lpVoid)
{
	CKernel* p = (CKernel*)lpVoid;
	map<int, SOCKET>::iterator ite; 
	_STRU_TCP_VALIDATE_RQ rq;
	while (true)
	{
		cout << "【正在循环验证每一台在线机】" << endl;
		if (p->m_mapIdToSocket.size() == 0)
		{
			cout << "****  当前无客户机在线...  ***" << endl;
			Sleep(5000);
			continue;
		}
		ite = p->m_mapIdToSocket.begin();
		while ( p->haveOnlineClient && ite != p->m_mapIdToSocket.end())
		{
			cout << "【ID --- SOCKET 列表】" << endl;
			cout << "【     "<<ite->first << " -- "<<ite->second<<"     】" << endl;
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
				cout << "【客户端id: " << ite2->first << " 意外掉线！】" << endl;
				ite2++;

				//删除客户端
				_STRU_TCP_OFFLINE_RQ rq;
				rq.id = ite2->first;
				p->dealOfflineRq((char*)&rq,sizeof(rq),0);

				continue;
			}
			ite2->second = false;
			ite2++;
		}
	}
	//cout << "【验证结束】" << endl;
	return 0;
}

//关闭服务器
void CKernel::closeService()
{
	cout << __func__ << endl;

	//回收资源
	if (m_pMediator)
	{
		m_pMediator->CloseNet();
		delete m_pMediator;
		m_pMediator = nullptr;
	}
	//等待线程到while判断
	if (m_handle && WAIT_TIMEOUT == WaitForSingleObject(m_handle, 500/*ms*/))
	{
		//等待500ms没有退出循环就强制杀死线程
		TerminateThread(m_handle, -1);

		//关闭句柄
		CloseHandle(m_handle);
		m_handle = NULL;
	}
	//断开数据库连接
	m_sql.DisConnect();
}

//处理所有接收到的数据
void CKernel::dealReadyData(char* recvData, int nLen, long lFrom)
{
	cout << __func__ << endl;
	//cout << "CKernel::dealReadyData : " << recvData << endl;
	//m_pMediator->SendData("CKernel::dealReadyData", sizeof("CKernel::dealReadyData"), lFrom);

	//转换数据包
	_STRU_TCP_REGISTER_RQ* rq;
	rq = (_STRU_TCP_REGISTER_RQ*)recvData;
	cout << "Service Kernel recv->one rq: type:" << rq->type /*<< " " << rq->name << " " << rq->tel << " " << rq->password*/ << endl;

	// 1 取出协议头
	PackType type = *(PackType*)recvData;
	/*if (type == _DEF_TCP_ADD_FRIEND_RQ)
	{
		cout << "【type == _DEF_TCP_ADD_FRIEND_RQ】" << endl;
	}*/
    // 2 根据协议头走不同的处理方法
	//计算数组下标
	int index = type - (_DEF_PROTOCOL_BASE + 1);
	//检验下标的合法性
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
	//delete传过来的数据
	delete[] recvData;

}

//处理注册请求的函数
void CKernel::dealRegisterRq(char* recvData, int nLen, long lFrom)
{
	cout << __func__ << endl;
	// 1 拆包
	_STRU_TCP_REGISTER_RQ* rq = (_STRU_TCP_REGISTER_RQ*)recvData;
	cout << "CKernel::dealRegisterRq : name:" << rq->name << " tel:" << rq->tel << " password:" << rq->password << endl;
	
	
	// 2 校验数据的合法性
	string name = rq->name;
	string name_temp = rq->name;
	string tel = rq->tel;
	string tel_temp = rq->tel;
	string password = rq->password;
	string password_temp = rq->password;
	//判断是否是空字符串 或者是全空格
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
		//QMessageBox::about(this, "错误提示", "输入为空白，请重试");
		cout << "Client 注册信息输入为空白，请重试" << endl;
		return;
	}
	//检查长度是否合法（name <= 20,tel = 11,password <= 20）
	if (name.length() > 20 || tel.length() != 11 || password.length() > 20)
	{
		//QMessageBox::about(this, "错误提示", "输入长度有误，请重试");
		cout << "Client 注册信息输入长度有误，请重试" << endl;
		return;
	}
	//检查内容是否合法(tel为数字 name password为大小写字母 数字 下划线的组合)
	if (0)
	{
		//QMessageBox::about(this, "错误提示", "输入格式有误，请重试");
		cout << "Client 注册信息输入格式有误，请重试" << endl;
		return;
	}





	// 3 根据手机号查询数据库 
	_STRU_TCP_REGISTER_RS rs;
	list<string> listRes;
	char sqlBuf[1024] = ""; //查询语句
	sprintf(sqlBuf, "select tel from t_user where tel = '%s' ;",rq->tel);
	if (!m_sql.SelectMySql(sqlBuf,1,listRes))
	{
		//tel查询失败
		cout << "CKernel::dealRegisterRq SQL_select_failed" << endl;
		rs.m_nRegister = register_false_sql_select_error;

		m_pMediator->SendData((char*)&rs, sizeof(rs), lFrom);
		return;
	}


	// 4 手机号已经被注册过了 查询结果>0  不予注册
	if ( listRes.size() > 0)
	{
		//注册失败 手机号已被注册
		cout << "CKernel::dealRegisterRq SQL_select_result : tel is used" << endl;
		rs.m_nRegister = register_false_tel_used;

		m_pMediator->SendData((char*)&rs, sizeof(rs), lFrom);
		return;
	}
	


	// 5 手机号未被注册 根据昵称查询数据库
	sprintf(sqlBuf, "select name from t_user where name = '%s' ;", rq->name);
	if (!m_sql.SelectMySql(sqlBuf, 1, listRes))
	{
		//name查询失败
		cout << "CKernel::dealRegisterRq SQL_select_failed" << endl;
		rs.m_nRegister = register_false_sql_select_error;

		m_pMediator->SendData((char*)&rs, sizeof(rs), lFrom);
		return;
	}
	//判断查到的名字数量
	if (listRes.size() > 0)//== 2)
	{
		//注册失败 昵称已被注册
		cout << "CKernel::dealRegisterRq SQL_select_result : name is used" << endl;
		rs.m_nRegister = register_false_name_used;
		
		m_pMediator->SendData((char*)&rs, sizeof(rs), lFrom);
		return;
	}



	// 6 把新用户信息写入数据库
	//随机数一个iconid
	srand((unsigned)time(0));
	int i = rd() % 36;
	if (i > 35) i = 7;
	sprintf(sqlBuf, "insert into t_user (name,tel,password,feeling,iconid) values ('%s','%s','%s','a bug_lover',%d);"
		,rq->name,rq->tel,rq->password,i);
	if (!m_sql.UpdateMySql(sqlBuf))
	{
		//写入数据库失败
		cout << "CKernel::dealRegisterRq SQL_update_failed, sqlBuf : " <<sqlBuf<< endl;
		rs.m_nRegister = register_false_sql_update_error;

		m_pMediator->SendData((char*)&rs, sizeof(rs), lFrom);
		return;
	}



	// 7 走到这里就注册成功了
	rs.m_nRegister = register_success;
	m_pMediator->SendData((char*)&rs, sizeof(rs), lFrom);
	return;
}

//处理登录请求的函数
void CKernel::dealLoginRq(char* recvData, int nLen, long lFrom)
{
	cout << __func__ << endl;


	// 1 拆包
	_STRU_TCP_LOGIN_RQ* rq = (_STRU_TCP_LOGIN_RQ*)recvData;
	cout << "CKernel::dealRegisterRq :" << " tel:" << rq->tel << " password:" << rq->password << endl;


	// 2 校验数据的合法性
	string tel = rq->tel;
	string tel_temp = rq->tel;
	string password = rq->password;
	string password_temp = rq->password;
	//判断是否是空字符串 或者是全空格
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
		//QMessageBox::about(this, "错误提示", "输入为空白，请重试");
		cout << "Client 登录信息输入为空白，请重试" << endl;
		return;
	}
	//检查长度是否合法（name <= 20,tel = 11,password <= 20）
	if (tel.length() != 11 || password.length() > 20)
	{
		//QMessageBox::about(this, "错误提示", "输入长度有误，请重试");
		cout << "Client 登录信息输入长度有误，请重试" << endl;
		return;
	}
	//检查内容是否合法(tel为数字 name password为大小写字母 数字 下划线的组合)
	if (0)
	{
		//QMessageBox::about(this, "错误提示", "输入格式有误，请重试");
		cout << "Client 登录信息输入格式有误，请重试" << endl;
		return;
	}


	// 3 根据手机号查询密码
	_STRU_TCP_LOGIN_RS rs;
	rs.sock = lFrom;

	//如果是在线状态的自动重新登录 就不查数据库了
	if (rq->bOnlineStatus == true)
	{
		/*m_sql.DisConnect();
		m_sql.ConnectMySql("127.0.0.1", "root", "colin123456", "ims_tcp");*/
		rs.m_nLogin = login_success;
		rs.userId = rq->userId;
		if (rq->userId == -1)
		{
			cout << "CKernel::dealLoginRq 在线状态的自动重新登录失败" << endl;
			return;
		}
		m_mapIdToSocket[rq->userId] = lFrom;
		m_pMediator->SendData((char*)&rs,sizeof(rs),lFrom);
		//根据用户id获取当前用户的好友列表信息 并逐个发送给他
		getFriendInfo(rs.userId);
		getAllGroupMember(rs.userId);
		cout << "【在线用户自动重新登录成功 id: "<<rs.userId<<" 】" << endl;
		return;
	}
	list<string> listRes;
	char sqlBuf[1024] = ""; //查询语句
	sprintf(sqlBuf, "select id,password from t_user where tel = '%s' ;", rq->tel);
	if (!m_sql.SelectMySql(sqlBuf, 2, listRes))
	{
		//tel查询失败
		cout << "CKernel::dealLoginRq SQL_select_failed" << endl;
		cout << "【" << sqlBuf<<"】" << endl;
		rs.m_nLogin = login_false_sql_select_error;

		m_pMediator->SendData((char*)&rs, sizeof(rs), lFrom);
		return;
	}

	// 4 判断查询结果
	//手机号未注册过 --- 没有查询到
	if (listRes.size() == 0)
	{
		cout << "CKernel::dealLoginRq SQL_select_failed" << endl;
		rs.m_nLogin = login_false_no_tel;

		m_pMediator->SendData((char*)&rs, sizeof(rs), lFrom);
		return;
	}
	// 取出用户id
	int id = atoi(listRes.front().c_str());
	rs.userId = id;
	listRes.pop_front();
	// 4 取出密码
	password = listRes.front();
	// 5 检验密码
	if (strcmp(password.c_str(), rq->password) != 0)
	{
		//输入的密码错误
		rs.m_nLogin = login_false_password_wrong;

		m_pMediator->SendData((char*)&rs, sizeof(rs), lFrom);
		return;
	}
	else
	{
		//输入的密码正确
		rs.m_nLogin = login_success;
		rs.sock = lFrom;
		//验证这个号码是否是上线状态
		if (m_mapIdToSocket.count(id) > 0)
		{
			rs.m_nLogin = login_false_illegal;
			m_pMediator->SendData((char*)&rs, sizeof(rs), lFrom);
			if (rq->bOnlineStatus)
			{
				//保存当前用户的socket和id
				m_mapIdToSocket[id] = lFrom;
				m_mapIdToClientStatus[id] = true;

				//把他自己的信息发送给他
				m_pMediator->SendData((char*)&rs, sizeof(rs), lFrom);
				//根据用户id获取当前用户的好友列表信息 并逐个发送给他
				getFriendInfo(id);
				getAllGroupMember(id);
			}
			
			return;
		}
		//保存当前用户的socket和id
		m_mapIdToSocket[id] = lFrom;
		m_mapIdToClientStatus[id] = true;

		//把他自己的信息发送给他
		m_pMediator->SendData((char*)&rs, sizeof(rs), lFrom);
		//根据用户id获取当前用户的好友列表信息 并逐个发送给他
		getFriendInfo(id);
		getAllGroupMember(id);
		Sleep(10);
		haveOnlineClient = true;
		return;
	}
}

//根据用户id获取他的好友信息 并逐个发送给他
void CKernel::getFriendInfo(int id)
{
	_STRU_TCP_FRIEND_INFO info;
	//id获取他自己的信息
	getInfoFromId(id, &info);
	//发送到客户端
	if (m_mapIdToSocket[id])
	{
		m_pMediator->SendData((char*)&info, sizeof(info), m_mapIdToSocket[id]);
	}
	else
	{
		cout << "CKernel::getFriendInfo map_no_your_id-ip" << endl;
		return;
	}
	//id获取他的好友id列表
	list<string> listRes;
	list<int> listFriendId;
	char sqlBuf[1024] = ""; //查询语句
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



	//遍历查询结果，一个一个发送
	int fId;
	int send;
	while (listFriendId.size() > 0)
	{
		//cout << "Start 取出好友id" << endl;
		//取出好友id
		fId = listFriendId.front();
		listFriendId.pop_front();
		//id->查询好友的信息
		getInfoFromId(fId, &info);
		//发送到客户端
		if (m_mapIdToSocket[id])
		{
			send = m_pMediator->SendData((char*)&info, sizeof(info), m_mapIdToSocket[id]);
			cout << id << " 有一个好友id为 :" << fId << ",sendNum : " << send << endl;
		}
		else
		{
			cout << "CKernel::getFriendInfo map_no_friend_id-ip, id : " << "id" << endl;
		}
		//通知当前用户的所有好友我上线了
		//判断每个好友是否在线 在线就发送你自己的信息
		if (m_mapIdToSocket.count(fId) > 0)
		{
			getInfoFromId(id, &info);
			info.status = status_online;
			send = m_pMediator->SendData((char*)&info, sizeof(info), m_mapIdToSocket[fId]);
			cout <<id<< " 把自己的信息发送给了在线的好友 : " <<fId<<",sendNum : "<<send << endl;
		}
	}
}

//根据用户id把他参加的群聊信息 及其所有成员信息发给他
void CKernel::getAllGroupMember(int id)
{
	//测试打印
	int row = 1;
	auto iteMap = m_mapGroupIdToMember.begin();
	while (iteMap != m_mapGroupIdToMember.end())
	{
		auto iteMem = iteMap->second->begin();
		while (iteMem != iteMap->second->end())
		{
			cout << "【row: "<<row<<" 群id: "<<iteMap->first<<" 里面有一个成员id: "<<*iteMem<<"】"<<endl;
			iteMem++;
			row++;
		}
		iteMap++;
	}

	_STRU_TCP_FRIEND_INFO info;
	getInfoFromId(id,&info);
	auto ite = info.hisGroupMap.begin();
	//把他的群-id信息 存在服务器
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
	//通过群名在数据库里找到所有的成员加进来
	
	//遍历服务器的map 发送信息
	_STRU_TCP_SEND_CLIENT_GROUP_MEMBERS_INFO memberInfo;
	_STRU_TCP_SEND_CLIENT_GROUP_INFO groupInfo;
	ite = info.hisGroupMap.begin();
	while (ite != info.hisGroupMap.end())
	{
		int gId = ite->first;
		//查表获得群聊的iconId
		char sqlBuf[1024];
		sprintf(sqlBuf,"select distinct gIconId from t_group where groupId = %d;",gId);
		list<string>listRes;
		if (!m_sql.SelectMySql(sqlBuf,1,listRes))
		{
			cout << "CKernel::getAllGroupMember id为 "<<gId <<" 查找icon失败" << endl;
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
			cout << "CKernel::getAllGroupMember id为 " << gId << " 查找icon结果：无/多出" << endl;
			ite++;
			continue;
		}
		strcpy(memberInfo.groupName, (ite->second).c_str());
		strcpy(groupInfo.groupName, (ite->second).c_str());
		memberInfo.groupId = gId;
		groupInfo.groupId = gId;

		//群的成员只有他自己 群是灰色头像 不在线
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
			//去扒数据库
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
					cout << m_mapIdToSocket.size() << " num1: "<<num << memberInfo.groupName <<"---"<<memberInfo.memberName<<" 离线1: " << memberInfo.memberStatus << endl;
					memberInfo.memberStatus = status_offline;
				}
			}
			else
			{
				cout << "map只有他自己，去扒数据库失败" << endl;
			}
			ite++;
			continue;
		}
		else  //群里有其他成员在线 彩色群头像
		{
			groupInfo.groupStatus = status_online;
			int send3 = m_pMediator->SendData((char*)&groupInfo, sizeof(groupInfo), m_mapIdToSocket[id]);

			//发送自己在群的信息
			memberInfo.memberStatus = status_online;
			memberInfo.memberId = info.id;
			strcpy(memberInfo.memberName, info.name);
			memberInfo.memberIconid = info.iconId;
			int send4 = m_pMediator->SendData((char*)&memberInfo, sizeof(groupInfo), m_mapIdToSocket[id]);
			cout << "send3: " << send3 <<" send4: "<<send4 << endl;

			//去扒数据库
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
				cout << "map不是只有他自己，去扒数据库失败" << endl;
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

	//遍历服务器的群聊map 发给所有在线的群成员 他上线了
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
			//找到了他的群
			if (ite2->first == ite->first)
			{
				infoToMer.groupId = ite->first;
				strcpy(infoToMer.groupName,ite->second.c_str());
				//遍历在线的成员
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
						cout << "【成员id: " << info.id << "发给了群: " << infoToMer.groupId << " 的成员id: " << *ite3 << " 我上线了】" << endl;
					}
					ite3++;
				}
			}
			ite2++;
		}
		ite++;
	}

}

//根据id获取他的信息
void CKernel::getInfoFromId(int id, _STRU_TCP_FRIEND_INFO* pinfo/*传出参数*/)
{
	pinfo->id = id;
	//判断当前用户是否在线
	if (m_mapIdToSocket.count(id) > 0)
	{
		pinfo->status = status_online;
	}
	else
	{
		pinfo->status = status_offline;
	}
	//从数据库查信息
	list<string> listRes;
	char sqlBuf[1024] = ""; //查询语句
	sprintf(sqlBuf, "select name,iconid,feeling from t_user where id = %d ;", id);
	if (!m_sql.SelectMySql(sqlBuf, 3, listRes))
	{
		cout << "CKernel::getInfoFromId sql_select_failed" << endl;
		return;
	}
	//判断查询结果是否正确
	if (listRes.size() == 3)
	{
		//cout << "CKernel::getInfoFromId sql_select_success" << endl;

		//取出信息
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
				cout << "CKernel::getInfoFromId 查找它的群聊不完整" << endl;
				return;
			}
			string gName = listRes.front();
			listRes.pop_front();
			pinfo->hisGroupMap[gId] = gName;
		}
	}
	else
	{
		cout << "CKernel::getInfoFromId 查找它的群聊信息失败" << endl;
	}
}

//转发群聊信息
void CKernel::dealGroupMsg(char* recvData, int nLen, long lFrom)
{
	_STRU_TCP_GROUP_CHAT_MSG* msg = (_STRU_TCP_GROUP_CHAT_MSG*)recvData;

	//找到这个群
	if (m_mapGroupIdToMember.count(msg->groupId) > 0)
	{
		//遍历发给每一个在线的成员
		auto ite = m_mapGroupIdToMember[msg->groupId]->begin();
		while (ite != m_mapGroupIdToMember[msg->groupId]->end())
		{
			if (m_mapIdToSocket.count(*ite) > 0 && *ite != msg->memberId) //成员在线就发
			{
				m_pMediator->SendData((char*)msg,sizeof(*msg),m_mapIdToSocket[*ite]);
				cout << "【转发了群消息！ 群号:"<<msg->groupId <<" 来自id:"<<msg->memberId<<" 发给id:"<<*ite <<"】" << endl;
			}
			ite++;
		}
	}
	else
	{
		cout << "CKernel::dealGroupMsg 这个群没有人在线 groupId: "<<msg->groupId << endl;
		return;
	}
	
}

//处理下线请求的函数
void CKernel::dealOfflineRq(char* recvData, int nLen, long lFrom)
{
	// 1 拆包
	_STRU_TCP_OFFLINE_RQ* rq = (_STRU_TCP_OFFLINE_RQ*)recvData;
	int id = rq->id;

	// 2 根据id取出好友id列表 向每个好友发送一个自己的信息info（status = offline）
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

		//如果好友在线就发送自己的下线信息---info
		if (m_mapIdToSocket.count(id) > 0)
		{
			int send = m_pMediator->SendData((char*)&info, sizeof(info), m_mapIdToSocket[id]);
			cout << rq->id << " 要下线了，告诉了他的在线好友 " <<id <<",sendNum : " << send << endl;
		}
		
	}

	

	// 4 把自己从群的map中移除掉
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
	//遍历服务器的群聊map 发给所有在线的群成员 他下线了
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
			//找到了他的群
			if (ite2->first == ite->first)
			{
				infoToMer.groupId = ite->first;
				strcpy(infoToMer.groupName, ite->second.c_str());
				//遍历在线的成员
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
						cout << "【成员id: " << info.id << "发给了群: " << infoToMer.groupId << " 的成员id: " << *ite3 << " 我下线了】" << endl;
					}
					ite3++;
				}
			}
			ite2++;
		}
		ite++;
	}
	// 3 把自己从map中移除掉
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

//处理聊天内容 转发聊天消息
void CKernel::dealChatRq(char* recvData, int nLen, long lFrom)
{
	_STRU_TCP_CHAT_RQ* rq = (_STRU_TCP_CHAT_RQ*)recvData;
	int fromId = rq->fromId;
	int toId = rq->toId;
	string content = rq->content;
	cout << fromId << " 对 " << toId << " 说 : " << content << endl;
	//判断接收方是否在线
	if (m_mapIdToSocket.count(toId) > 0)
	{
		m_pMediator->SendData((char*)rq, sizeof(*rq), m_mapIdToSocket[toId]);
	}
	else
	{
		cout << "但是 " << toId << " 并不在线" << endl;
		strcpy(rq->content,"【系统提示】好友不在线呦>_<");
		m_pMediator->SendData((char*)rq, sizeof(*rq), m_mapIdToSocket[fromId]);

		_STRU_TCP_CHAT_RS rs;
		rs.fromId = rq->toId;
		m_pMediator->SendData((char*)&rs, sizeof(rs), m_mapIdToSocket[fromId]);
	}
}

//处理添加好友的数据报
void CKernel::dealAddFriendRq(char* recvData, int nLen, long lFrom)
{
	// 1 拆包
	_STRU_TCP_ADD_FRIEND_RQ* rq = (_STRU_TCP_ADD_FRIEND_RQ*)recvData;
	cout << rq->fromName << " 正在添加好友 : " << rq->toName<<endl;

	
	
	/*for (map<int,SOCKET>::iterator ite = m_mapIdToSocket.begin();
		ite != m_mapIdToSocket.end();ite++)
	{
		cout << "【"<<ite->first << "--" << ite->second <<"】" << endl;
	}*/
	if (m_mapIdToSocket[rq->fromId] != lFrom)
	{
		cout << "【" << rq->fromId << " 的 id--sock 未能及时更新到map中】id--sock: " << rq->fromId << "--" << lFrom << endl;
	}
	//重新绑定sock
	m_mapIdToSocket[rq->fromId] = lFrom;

	// 2 校验
	if (rq->fromName == rq->toName)
	{
		cout << rq->fromName << " 添加自己为好友，荒唐！" << endl;
		return;
	}
	string name = rq->fromName;
	string name_temp = name;
	//判断是否是空字符串 或者是全空格
	int ite = 0;
	while (ite != -1)
	{
		ite = name_temp.find(" ");
		if (ite != -1) name_temp.erase(ite, ite + 1);
	}
	
	if (name.empty() || name_temp.empty())
	{
		//QMessageBox::about(this, "错误提示", "输入为空白，请重试");
		cout << "Client 注册信息输入为空白，请重试" << endl;
		return;
	}
	//检查长度是否合法（name <= 20,tel = 11,password <= 20）
	if (name.length() > 20)
	{
		//QMessageBox::about(this, "错误提示", "输入长度有误，请重试");
		cout << "Client 注册信息输入长度有误，请重试" << endl;
		return;
	}
	//检查内容是否合法(tel为数字 name password为大小写字母 数字 下划线的组合)
	if (0)
	{
		//QMessageBox::about(this, "错误提示", "输入格式有误，请重试");
		cout << "Client 注册信息输入格式有误，请重试" << endl;
		return;
	}

	// 3 获取好友的id  查询好友是否存在  是否在线
	_STRU_TCP_FRIEND_INFO info;
	_STRU_TCP_ADD_FRIEND_RS res;
	strcpy(res.friendName , rq->toName);
	res.fromId = rq->fromId;

	char sqlBuf[100];
	list<string>listRes;
	cout << "select id from t_user where name = "<<rq->toName << endl;
	sprintf(sqlBuf,"select id from t_user where name = '%s';",rq->toName);
	if (!m_sql.SelectMySql(sqlBuf,1,listRes))//好友不存在
	{
		cout << "数据库查询错误" << endl;
		return;
	}
	else //好友存在判断是否在线
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
			cout << "但是好友 : " << rq->toName << "不存在";
			return;
			
		}
		
		if (m_mapIdToSocket.count(info.id) <= 0) //好友不在线
		{
			res.m_nAddRes = add_friend_false_offline;
			m_pMediator->SendData((char*)&res, sizeof(res), m_mapIdToSocket[rq->fromId]);
			cout << "但是好友 : " << rq->toName << "不在线";
			return;
		}
		else //在线向好友转发好友申请
		{
			m_pMediator->SendData((char*)rq, sizeof(*rq), m_mapIdToSocket[info.id]);
			cout << "向好友 : " << rq->toName << " 转发了 "<<rq->fromName<<" 的好友申请"<<endl;
			return;
		}
	}

}

//处理回复添加好友的数据报
void CKernel::dealAddFriendRs(char* recvData, int nLen, long lFrom)
{
	
	
	// 1 拆包
	_STRU_TCP_ADD_FRIEND_RS* rs = (_STRU_TCP_ADD_FRIEND_RS*)recvData;
	
	
	if (m_mapIdToSocket[rs->friendId] != lFrom)
	{
		cout << "【" << rs->friendId << " 的 id--sock 未能及时更新到map中】id--sock: " << rs->friendId << "--" << lFrom << endl;
	}
	//重新绑定sock
	m_mapIdToSocket[rs->friendId] = lFrom;
	if (rs->m_nAddRes == add_friend_success)
	{
		cout << "【friendId: " << rs->friendId<<"--"<<m_mapIdToSocket[rs->friendId] 
			<< " 同意被fromId: " << rs->fromId << "--" << m_mapIdToSocket[rs->fromId] << " 添加】" << endl;
	}
	else if (rs->m_nAddRes == add_friend_false_disagree)
	{
		cout << "【friendId: " << rs->friendId << "--" << m_mapIdToSocket[rs->friendId]
			<< " 不同意被fromId: " << rs->fromId << "--" << m_mapIdToSocket[rs->fromId] << " 添加】" << endl;
	}
	
	
	//判断是否添加那个人为好友 是的话 发给好友一个加好友成功回复/回一个好友信息/回好友一个自己的信息  不是的话回那个好友一个好友添加结果disagree
	if (rs->m_nAddRes == add_friend_success)
	{
		//回自己一个好友信息
		_STRU_TCP_FRIEND_INFO info;
		getInfoFromId(rs->friendId,&info);
		cout << info.id << "--" << m_mapIdToSocket[info.id] <<" 被我添加了，我: "<<rs->fromId<<"--"<<m_mapIdToSocket[rs->fromId] << endl;
		m_pMediator->SendData((char*)&info, sizeof(info), m_mapIdToSocket[rs->fromId]);
		//回好友一个自己的信息
		_STRU_TCP_FRIEND_INFO userInfo;
		getInfoFromId(rs->fromId, &userInfo);
		cout << rs->friendId << "--" << m_mapIdToSocket[rs->friendId] << " ->我, 我被别人添加了，别人: " << userInfo.id << "--" << m_mapIdToSocket[userInfo.id] << endl;
		m_pMediator->SendData((char*)&userInfo, sizeof(userInfo), m_mapIdToSocket[rs->friendId]);
		//发给好友一个加自己成功回复
		_STRU_TCP_ADD_FRIEND_RS fRs;
		fRs.friendId = userInfo.id;
		strcpy(fRs.friendName , userInfo.name);
		fRs.fromId = info.id;
		fRs.m_nAddRes = add_friend_success;
		cout << fRs.fromId << "--" << m_mapIdToSocket[fRs.fromId] << " ->我, 我被别人添加成功，别人: " << fRs.friendId << "--" << m_mapIdToSocket[info.id] << endl;
		m_pMediator->SendData((char*)&fRs, sizeof(fRs), m_mapIdToSocket[info.id]);
		//发给自己一个加好友成功回复
		fRs.friendId = info.id;
		strcpy(fRs.friendName, info.name);
		fRs.fromId = userInfo.id;
		fRs.m_nAddRes = add_friend_success;
		cout << fRs.friendId << "--" << m_mapIdToSocket[fRs.friendId] << " 被我添加成功，我: " << rs->fromId << "--" << m_mapIdToSocket[userInfo.id] << endl;
		m_pMediator->SendData((char*)&fRs, sizeof(fRs), m_mapIdToSocket[userInfo.id]);
		//好友关系写入数据库
		char sqlBuf[100];
		list<string>listRes;
		sprintf(sqlBuf,"insert into t_friend (idA,idB) values (%d,%d);",rs->friendId,rs->fromId);
		if (!m_sql.UpdateMySql(sqlBuf))
		{
			cout << "CKernel::dealAddFriendRs 新增好友关系1写入数据库失败" << endl;
		}
		sprintf(sqlBuf, "insert into t_friend (idA,idB) values (%d,%d);", rs->fromId, rs->friendId);
		if (!m_sql.UpdateMySql(sqlBuf))
		{
			cout << "CKernel::dealAddFriendRs 新增好友关系2写入数据库失败" << endl;
		}
	}
	else if(rs->m_nAddRes == add_friend_false_disagree)
	{
		//不同意就转发数据报
		m_pMediator->SendData((char*)rs,sizeof(*rs),m_mapIdToSocket[rs->fromId]);
		cout << rs->friendId <<"--" << m_mapIdToSocket[rs->friendId] << " 不同意被 " << rs->fromId << "--" << m_mapIdToSocket[rs->fromId] << " 添加" << endl;
	}
	else
	{
		cout << "CKernel::dealAddFriendRs:: 客户端处理错误" << endl;
	}
}

//处理删除好友的请求
void CKernel::dealDeleteFriendRq(char* recvData, int nLen, long lFrom)
{
	_STRU_TCP_DELETE_FRIEND_RQ* rq = (_STRU_TCP_DELETE_FRIEND_RQ*)recvData;
	m_mapIdToSocket[rq->userId] = lFrom;
	_STRU_TCP_DELETE_FRIEND_RS rs;
	strcpy(rs.friendName, rq->friendName);
	rs.userId = rq->userId;
	
	//根据盆友的name去找朋友ID
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

	//删除数据库的好友关系
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

	//重发好友信息
	getFriendInfo(info.id);
	_STRU_TCP_FRIEND_DELETE_YOU_MSG msg;
	msg.friendId = info.id;
	m_pMediator->SendData((char*)&msg,sizeof(msg),m_mapIdToSocket[fId]);
	if(m_mapIdToSocket.count(rs.friendId) > 0) getFriendInfo(rs.friendId);
												
}

//处理客户端的登录验证回复
void CKernel::dealValidateRs(char* recvData, int nLen, long lFrom)
{
	// 拆包
	_STRU_TCP_VALIDATE_RS* rs = (_STRU_TCP_VALIDATE_RS*)recvData;
	m_mapIdToClientStatus[rs->id] = true;
	// 处理
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
	cout << "【客户端id: "<<rs->id<<" 处于良好在线状态】" << endl;
}



//接收文件 并发送转发文件请求
void CKernel::dealRecvAndResendFile(char* recvData, int nLen, long lFrom)
{
	//接收文件信息
	_STRU_TCP_CLIENT_SEND_FILE_RQ* rq = (_STRU_TCP_CLIENT_SEND_FILE_RQ*)recvData;
	int toId = rq->id;
	//判断文件是否接收完毕 循环等待半小时
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
	//服务器接收文件失败
	if (recvfile == false)
	{
		//回复文件发送者一个转发失败回复
		_STRU_TCP_SEND_FILE_RES res;
		res.id = rq->id;
		res.sendRes = send_file_failed_ser_recv;
		res.toFriOrGop = rq->toFriOrGop;
		m_pMediator->SendData((char*)&res,sizeof(res),m_mapIdToSocket[rq->userId]);
		return;
	}

	//发送转发文件请求
	if (rq->toFriOrGop == file_friend)
	{
		//判断好友是否在线
		if (m_mapIdToSocket.count(rq->id) <= 0)
		{
			_STRU_TCP_SEND_FILE_RES res;
			res.id = rq->id;
			res.sendRes = send_file_failed_fri_offline;
			res.toFriOrGop = rq->toFriOrGop;
			m_pMediator->SendData((char*)&res, sizeof(res), m_mapIdToSocket[rq->userId]);
			return;
		}
		//好友在线
		_STRU_TCP_CLIENT_RECV_FILE_MSG msg;
		msg.fromFriOrGop = rq->toFriOrGop;
		msg.id = rq->id;
		strcpy(msg.szFileId, rq->szFileId);
		m_pMediator->SendData((char*)&msg, sizeof(msg), m_mapIdToSocket[rq->id]);
	}
	else //转发请求给群里的所有在线成员
	{
		//群里没有在线的成员
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

//处理文件接收到服务器完毕的信号
void CKernel::dealFileRecvAcc(char* recvData, int nLen, long lFrom)
{
	STRU_FILE_RECV_ACC_MSG* msg = (STRU_FILE_RECV_ACC_MSG*)recvData;
	//保存文件id到set
	string fileId = msg->szFileId;
	m_setFileRecvAcc.insert(fileId);
	delete msg;
}

//处理文件接受者的文件接收回复
void CKernel::dealFileRecverRs(char* recvData, int nLen, long lFrom)
{
	_STRU_TCP_CLIENT_RECV_FILE_RES* res = (_STRU_TCP_CLIENT_RECV_FILE_RES*)recvData;
	if (res->userRecvRes == user_recv_file_no)
	{
		//回复文件发送者一个回复
	_STRU_TCP_SEND_FILE_RES resSender;
	resSender.id = res->id;
	resSender.sendRes = send_file_failed_fri_refuse;
	resSender.toFriOrGop = res->fromFriOrGop;
	m_pMediator->SendData((char*)&resSender, sizeof(resSender), m_mapIdToSocket[res->userId]);
	}
	else
	{
		//转发文件
		m_pMediator->RefreshSocket(m_mapIdToSocket[res->id]);
		m_pMediator->SendFile(res->szFileId);

		//判断文件是否转发完毕 循环等待半小时
		bool recvsend = false;
		for (int i = 0; i < 180; i++)
		{
			if (m_setFileResendAcc.count(res->szFileId) > 0)
			{
				recvsend = true;
				//立刻删掉这个发送记录
				m_setFileResendAcc.clear();
				break;
			}
			Sleep(10000);
		}
		//服务器接收文件失败
		if (recvsend == false)
		{
			//回复文件发送者一个转发失败回复
			_STRU_TCP_SEND_FILE_RES resSender;
			resSender.id = res->id;
			resSender.sendRes = send_file_failed_ser_send;
			resSender.toFriOrGop = res->fromFriOrGop;
			m_pMediator->SendData((char*)&resSender, sizeof(resSender), m_mapIdToSocket[res->userId]);
			return;
		}


		//回复文件发送者一个转发成功回复
		_STRU_TCP_SEND_FILE_RES resSender;
		resSender.id = res->id;
		resSender.sendRes = send_file_succeed;
		resSender.toFriOrGop = res->fromFriOrGop;
		m_pMediator->SendData((char*)&resSender, sizeof(resSender), m_mapIdToSocket[res->userId]);
	}
}

//处理文件由服务器转发完毕的信号
void CKernel::dealFileResendAcc(char* recvData, int nLen, long lFrom)
{
	STRU_FILE_RESEND_ACC_MSG* msg = (STRU_FILE_RESEND_ACC_MSG*)recvData;
	//保存文件id到set
	string fileId = msg->szFileId;
	m_setFileResendAcc.insert(fileId);
	delete msg;
}

//初始化协议头数组
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


