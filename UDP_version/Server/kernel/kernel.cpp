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

//打开服务器
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

	//连接数据库
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

//关闭服务器
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

//核心工作
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

//处理注册请求
void kernel::dealRegRq(char* recvData, sockaddr_in fromAddr)
{
	cout << endl;
	cout << "[ kernel:: dealRegRq tel/email: "<<((recvBuf*)recvData)->buf1 <<" ]" << endl;
	cout << "[ kernel:: dealRegRq password: "<<((recvBuf*)recvData)->buf2 <<" ]" << endl;
	cout << endl;

	// 1 拆包
	recvBuf* rq = (recvBuf*)recvData;

	//msg.data1 0是手机号注册，1是邮箱注册
	if (rq->data1 == 1)
	{
		//检测邮箱格式

		//读数据库查看是否注册
		
		//写数据库

		//回复注册结果

		return;
	}

	//手机号注册
	
	// 2 校验手机号的合法性
	string tel = rq->buf1;
	string tel_temp = rq->buf1;
	string password = rq->buf2;
	string password_temp = rq->buf2;
	sendBuf msgrs;
	msgrs.type = _DEF_REG_RS_;
	msgrs.res1 = false;									 //res1是注册结果
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
		cout << "kernel:: dealRegRq 注册信息输入为空白，请重试" << endl;
		strcpy_s(msgrs.buf1, "注册信息输入为空白");						//buf1是错误码
		pToolUdp->SendData((char*)&msgrs,fromAddr);
		return;
	}
	//检查长度是否合法（tel = 11,password 3-15）
	if (tel.length() != 11 || password.length() > 15 || password.length() < 3)
	{
		cout << "kernel:: dealRegRq 注册信息输入长度有误，请重试" << endl;
		strcpy_s(msgrs.buf1, "注册信息输入长度有误");						//buf1是错误码
		pToolUdp->SendData((char*)&msgrs,fromAddr);
		return;
	}
	//检查内容是否合法(tel为数字 name password为大小写字母 数字 下划线的组合)
	if (0)
	{
		cout << "kernel:: dealRegRq 注册信息输入格式有误，请重试" << endl;
		strcpy_s(msgrs.buf1, "注册信息输入格式有误");						//buf1是错误码
		pToolUdp->SendData((char*)&msgrs,fromAddr);
		return;
	}

	sendBuf msg;
	//读数据库查看是否注册
	list<string> listRes;
	char sqlBuf[1024];
	sprintf(sqlBuf,"select tel from t_user where tel = '%s';",rq->buf1);
	if (!m_sql.SelectMySql(sqlBuf,1,listRes))
	{
		//查询失败
		cout << "kernel:: dealRegRq 数据库查询失败" << endl;

		//发一个注册回复包
		msg.type = _DEF_REG_RS_;							
		msg.res1 = false;									//res1是注册结果
		strcpy_s(msg.buf1,"注册查询数据库时出错，请重试");  //buf1是错误码
		pToolUdp->SendData((char*)&msg,fromAddr);
		return;
	}
	if (listRes.size() > 0)
	{
		//被注册过了

		cout << "kernel:: dealRegRq 手机号已被注册" << endl;
		msg.type = _DEF_REG_RS_;
		msg.res1 = false;						//res1是注册结果
		strcpy_s(msg.buf1,"手机号已被注册");	//buf1是错误码
		pToolUdp->SendData((char*)&msg,fromAddr);
		return;
	}

	//写入数据库
	sprintf(sqlBuf, "insert into t_user (tel,password) values ('%s','%s');",rq->buf1,rq->buf2);
	if (!m_sql.UpdateMySql(sqlBuf))
	{
		cout << "kernel:: dealRegRq 新用户写入数据库失败" << endl;
		msg.type = _DEF_REG_RS_;
		msg.res1 = false;						    //res1是注册结果
		strcpy_s(msg.buf1, "注册时写入数据库失败");	//buf1是错误码
		pToolUdp->SendData((char*)&msg,fromAddr);
		return;
	}

	//查找这个用户的id 给他发过去
	sprintf(sqlBuf, "select id from t_user where tel = '%s';", rq->buf1);
	if (!m_sql.SelectMySql(sqlBuf, 1, listRes))
	{
		//查询失败
		cout << "kernel:: dealRegRq 写入后查询id失败" << endl;

		//发一个注册回复包
		msg.type = _DEF_REG_RS_;
		msg.res1 = false;									 //res1是注册结果
		strcpy_s(msg.buf1, "注册查询数据库时出错，请重试");  //buf1是错误码
		pToolUdp->SendData((char*)&msg,fromAddr);
		return;
	}
	if (listRes.size() == 0)
	{
		cout << "kernel:: dealRegRq 查询id成功 listRes为空" << endl;
		return;
	}
	msg.data1 = stoi(listRes.front());			//data1存注册成功后的id

	//写入和注册成功
	cout << "kernel:: dealRegRq 新用户注册成功" << endl;
	msg.type = _DEF_REG_RS_;
	msg.res1 = true;						    //res1是注册结果
	strcpy_s(msg.buf1, "注册成功！登录吧");	    //buf1是错误码
	pToolUdp->SendData((char*)&msg,fromAddr);
	return;
}

//处理登录请求,回复能否登录 能的话回复：好友信息 群聊信息 群聊成员信息
void kernel::dealLogRq(char* recvData, sockaddr_in fromAddr)
{
	cout << endl;
	cout << "[ kernel:: dealLogRq tel/email: " << ((recvBuf*)recvData)->buf1 << " ]" << endl;
	cout << "[ kernel:: dealLogRq password: " << ((recvBuf*)recvData)->buf2 << " ]" << endl;
	cout << endl;

	// 1 拆包
	recvBuf* rq = (recvBuf*)recvData;

	//msg.data1 0是手机号登录，1是邮箱登录
	if (rq->data1 == 1)
	{
		cout << "[ kernel:: dealLogRq 邮箱账号登录中... ]" << endl;
		//检测邮箱格式

		//读数据库查看是否登录

		//写数据库

		//回复登录结果

		return;
	}

	//手机号登录
	cout << "[ kernel:: dealLogRq 手机号登录中... ]" << endl;

	// 2 校验手机号的合法性
	string tel = rq->buf1;
	string tel_temp = rq->buf1;
	string password = rq->buf2;
	string password_temp = rq->buf2;
	sendBuf msgrs;
	msgrs.type = _DEF_LOG_RS_;
	msgrs.res1 = false;									 //res1是注册结果
	//判断是否是空字符串 或者是全空格
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
		cout << "kernel:: dealLOGRq 登录信息输入为空白，请重试" << endl;
		strcpy_s(msgrs.buf1, "登录信息输入为空白");						//buf1是错误码
		pToolUdp->SendData((char*)&msgrs,fromAddr);
		return;
	}
	//检查长度是否合法（tel = 11,password 3-15）
	if (tel.length() != 11 || password.length() > 15 || password.length() < 3)
	{
		cout << "kernel:: dealLOGRq 登录信息输入长度有误，请重试" << endl;
		strcpy_s(msgrs.buf1, "登录信息输入为空白");						//buf1是错误码
		pToolUdp->SendData((char*)&msgrs,fromAddr);
		return;
	}
	//检查内容是否合法(tel为数字 name password为大小写字母 数字 下划线的组合)
	if (0)
	{
		cout << "kernel:: dealLOGRq 登录信息输入格式有误，请重试" << endl;
		strcpy_s(msgrs.buf1, "登录信息输入为空白");						//buf1是错误码
		pToolUdp->SendData((char*)&msgrs,fromAddr);
		return;
	}

	sendBuf msg;
	list<string> listRes;
	char sqlBuf[1024];

	//查询用户id,email,password,name,iconId,sign
	sprintf(sqlBuf, "select id,email,password,name,iconId,sign from t_user where tel = '%s';", rq->buf1);
	if (!m_sql.SelectMySql(sqlBuf, 6, listRes))
	{
		//查询失败
		cout << "kernel:: dealLogRq 数据库查询失败" << endl;

		//发一个登录回复包
		msg.type = _DEF_LOG_RS_;
		msg.res1 = false;									//res1是登录结果
		strcpy_s(msg.buf1, "登录查询数据库时出错，请重试");  //buf1是错误码
		pToolUdp->SendData((char*)&msg,fromAddr);
		return;
	}

	//查看用户是否注册
	if (listRes.size() == 0)
	{
		cout << "kernel:: dealLOGRq 手机号未注册" << endl;
		msg.type = _DEF_LOG_RS_;
		msg.res1 = false;										//res1是登录结果
		strcpy_s(msg.buf1, "手机号未注册，请先注册");			//buf1是错误码
		pToolUdp->SendData((char*)&msg,fromAddr);
		return;
	}

	//根据id查看服务器是否登录
	int id = stoi(listRes.front());
	if (listRes.size() > 0 && m_mapIdToState.count(id) > 0)
	{
		//被登录过了

		cout << "kernel:: dealLogRq 手机号已登录" << endl;
		msg.type = _DEF_LOG_RS_;
		msg.res1 = false;						//res1是登录结果
		strcpy_s(msg.buf1, "手机号已被登录");	//buf1是错误码
		pToolUdp->SendData((char*)&msg,fromAddr);
		return;
	}
	
	//摘取信息
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
	//验证密码
	if (pwd != password)
	{
		cout << "kernel:: dealLOGRq 密码错误 登录失败" << endl;
		msg.type = _DEF_LOG_RS_;
		msg.res1 = false;						//res1是登录结果
		strcpy_s(msg.buf1, "密码错误");			//buf1是错误码
		pToolUdp->SendData((char*)&msg,fromAddr);
		return;
	}

	//登录成功
	msg.type = _DEF_LOG_RS_;
	msg.data1 = id;								//data1存登录成功后的id
	msg.data2 = stoi(iconId);					//data2是iconId
	msg.res1 = true;						    //res1是登录结果
	msg.res2 = 0;								//res2是登录方式 0手机号 1邮箱
	strcpy_s(msg.buf1, "登录成功！欢迎使用");	//buf1是错误码
	strcpy_s(msg.buf2,email.c_str());			//buf2是email
	strcpy_s(msg.buf3,name.c_str());			//buf3是name
	strcpy_s(msg.buf4,sign.c_str());			//buf4是sign
	pToolUdp->SendData((char*)&msg,fromAddr);
	cout <<"id,email,name,sign: " <<id<<" " << email << " " << name << " " << sign << endl;

	//写入服务器记录
	m_mapIdToState[id] = true;
	m_mapIdToNo[id] = rq->no;
	cout << "kernel:: dealLOGRq 用户登录成功" << endl;

	//登录成功后发送给他好友信息和群聊信息，并告诉所有好友所有群成员他上线了
	msg.type = _DEF_FRI_INFO_;
	msg.res1 = true;
	strcpy_s(msg.buf1, rq->buf1);	//手机号考进去
	ConnectRelationships(id,&msg);

	return;
}

//登录成功后发送给他好友信息和群聊信息，并告诉所有好友所有群成员他上线了
void kernel::ConnectRelationships(int id,sendBuf* userInfo)
{
	int no = m_mapIdToNo[id];
	if (m_mapIdToState.count(id) <= 0)
		return;
	//查询他有几个好友,记录好友id
	list<string>listRes;
	list<string>listInfo;
	char sqlbuf[1024] = "";
	sprintf(sqlbuf,"select id2 from t_friend where id1 = %d;",id);
	if (!m_sql.SelectMySql(sqlbuf, 1, listRes))
	{
		cout << "kernel:: ConnectRelationships 查询好友id失败" << endl;
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
			cout << "kernel:: ConnectRelationships 查询一个好友信息失败" << endl;
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
			friInfo.res1 = true;				//好友在线
		else
			friInfo.res1 = false;
		friInfo.data1 = friId;
		friInfo.data2 = iconId;
		strcpy_s(friInfo.buf1, tel.c_str());
		strcpy_s(friInfo.buf2, email.c_str());
		strcpy_s(friInfo.buf3, name.c_str());
		strcpy_s(friInfo.buf4, sign.c_str());

		pToolUdp->SendData((char*)&friInfo,m_mapNoToAdd[no]);
		if(m_mapIdToState.count(friId) > 0 && friId != id)							 //好友在线则发一份给好友
			pToolUdp->SendData((char*)userInfo, m_mapNoToAdd[m_mapIdToNo[friId]]);
		cout << "kernel::ConnectRelationships 发送一个好友信息给id: " << id << ", 好友信息: id" 
			<< friId << " icon" << iconId << " name:"<<name <<" tel:" << tel << " email:" << email << " sign:" << sign << endl;
	}


	//查询他有几个群聊,记录群聊id
	listRes.clear();
	listInfo.clear();
	memset(sqlbuf,0,1024);
	sprintf(sqlbuf, "select groupId from t_groupmem where memId = %d;", id);
	if (!m_sql.SelectMySql(sqlbuf, 1, listRes))
	{
		cout << "kernel:: ConnectRelationships 查询群聊id失败" << endl;
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

		//服务器记录群在线成员信息
		m_mapGroInfo[groId].push_back(id);

		list<string> listNums;
		sprintf(sqlbuf, "select groupNum,groupName,iconId,sign from t_group where groupid = %d;", groId);
		if (!m_sql.SelectMySql(sqlbuf, 4, listInfo))
		{
			cout << "kernel:: ConnectRelationships 查询一个群聊信息失败" << endl;
			return;
		}
		sprintf(sqlbuf, "select count(*) from t_groupmem where groupid = %d;", groId);
		if (!m_sql.SelectMySql(sqlbuf, 1, listNums))
		{
			cout << "kernel:: ConnectRelationships 查询一个群聊memNums失败" << endl;
			return;
		}
		if(listNums.size() > 0)
			gmembNums = stoi(listNums.front());	//检测list大小  
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

		//给自己发送群聊信息
		pToolUdp->SendData((char*)&groInfo, m_mapNoToAdd[no]);
		cout << "kernel::ConnectRelationships 发送一个群聊信息给id: " << id << ", 群信息: id"
			<< groId << " icon" << gIcon << " name:" << groupName <<" size:"<<gmembNums << " num:" << groupNum << " gSign:" << gSign << endl;


		//给所有的群聊在线用户发送自己的信息 给自己发送所有群聊成员的信息
		//list<int>::iterator iteMem = m_mapGroInfo[groId].begin();
		//while (iteMem != m_mapGroInfo[groId].end())
		//{
		//	int memId = *iteMem;
		//	//判断成员是否在线
		//	if (memId != id && m_mapIdToState.count(memId) > 0)
		//		pToolUdp->SendData((char*)userInfo,m_mapIdToAdd[memId]);
		//}
		ite++;
	}

	//给用户发送所有群成员的信息
	sendBuf memInfo;
	ite = listRes.begin();
	while (ite != listRes.end() && listRes.size() > 0)
	{
		int gId = stoi(*ite);
		//查询所有群的成员id;
		list<string>listMemIds;
		sprintf(sqlbuf, "select memid from t_groupmem where groupid = %d;", gId);
		if (!m_sql.SelectMySql(sqlbuf,1,listMemIds))
		{
			cout << "kernel:: ConnectRelationships 查询群聊的所有成员id失败" << endl;
		}

		//用成员id查询成员信息 并发送给自己
		//把自己的信息发送给每一个在线的群成员
		for (string str : listMemIds)
		{
			int memId = stoi(str);
			if (memId == id)
				continue;
			if (getInfoFromId(memId, &memInfo))
			{
				memInfo.type = _DEF_GROMEM_INFO_;
				memInfo.data3 = gId;						//data3存成员所属群聊的id
				if (m_mapIdToState.count(memId) > 0)		//判断在线状态
					memInfo.res1 = true;
				else
					memInfo.res1 = false;


				//成员信息 发送给自己
				pToolUdp->SendData((char*)&memInfo, m_mapNoToAdd[no]);
				cout << "kernel::ConnectRelationships 发送一个群成员信息给id: " << id << ", 成员信息: id"
					<< memId <<" gid"<<memInfo.data3 << " icon" << memInfo.data2 << " name:" << memInfo.buf3 
					<< " tel:" << memInfo.buf1 << " email:" << memInfo.buf2 << " sign:" << memInfo.buf4 << endl;
				
				//把自己的信息发送给每一个在线的群成员
				if (m_mapIdToState.count(memId) > 0 && memId != id)
				{
					userInfo->type = _DEF_GROMEM_INFO_;
					pToolUdp->SendData((char*)userInfo,m_mapNoToAdd[m_mapIdToNo[memId]]);
				}
			}
			else
			{
				cout << "kernel:: ConnectRelationships 查询群聊id: "<<gId<<",的一个成员信息失败, memId: "<<memId << endl;
			}
		}
		ite++;
	}
}


//转发群聊信息
void kernel::dealGroupMsg(char* recvData, sockaddr_in fromAddr)
{
	groMsg* rq = (groMsg*)recvData;
	//转发群聊信息
	int gId = rq->gId;

	//发送给所有这个群在线的成员
	auto ite = m_mapGroInfo[gId].begin();
	while (ite != m_mapGroInfo[gId].end())
	{
		if (*ite != rq->fromId && m_mapIdToState.count(*ite) > 0)
		{
			pToolUdp->SendData(recvData, m_mapNoToAdd[m_mapIdToNo[*ite]]);
			cout << "发送了群消息，来自:" << rq->fromId << " 发往群:" << rq->gId << " msg:" << rq->buf << endl;
		}
		ite++;
	}
	if (ite == m_mapGroInfo[gId].begin())
	{
		rq->res = false;
		sprintf(rq->buf, "群id:%d 里只有你在线哦/群成员只有你自己，快邀请别人吧",rq->gId);
		pToolUdp->SendData(recvData, fromAddr);
		cout << "发送群消息失败，无人在线/只有一个成员，来自:" << rq->fromId << " 发往群:" << rq->gId << " msg:" << rq->buf << endl;
	}
}

//处理聊天 转发聊天消息
void kernel::dealChatMsg(char* recvData, sockaddr_in fromAddr)
{
	friMsg* rq = (friMsg*)recvData;
	int fromId = rq->fromId;
	//转发好友信息
	int toId = rq->toId;
	//判断好友是否在线
	if (m_mapIdToState.count(toId) <= 0)
	{
		rq->res = false;
		sprintf(rq->buf, "好友id:%d 离线",rq->toId);
		pToolUdp->SendData(recvData, fromAddr);
		cout << "发送好友消息失败，好友离线，来自:" << rq->fromId 
			<< " 发往好友:" << rq->toId << " msg:" << rq->buf << endl;
		return;
	}
	pToolUdp->SendData((char*)recvData, m_mapNoToAdd[m_mapIdToNo[toId]]);
	cout << "发送了好友信息，来自:" << rq->fromId
		<< " 发往好友:" << rq->toId << " msg:" << rq->buf << endl;
}

//处理添加好友请求
void kernel::dealAddFriendRq(char* recvData, sockaddr_in fromAddr)
{
	
}

//回复添加好友
void kernel::dealAddFriendRs(char* recvData, sockaddr_in fromAddr)
{

}
//处理删除好友的请求
void kernel::dealDeleteFriendNoti(char* recvData, sockaddr_in fromAddr)
{

}

//处理下线通知
void kernel::dealOfflineNoti(char* recvData, sockaddr_in fromAddr)
{

}

//心跳机制验活
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

	if (msg->no == -1)					//新客户机第一次连接 记录addr 并给客户机发过去他的服务器编号
	{
		//记下他的no和addr
		m_mapNoToAdd[nOnlineClients] = fromAddr;
		sendBuf noMsg;
		noMsg.type = _DEF_CLIENT_NO_;
		noMsg.no = nOnlineClients;
		strcpy_s(noMsg.buf1, "欢迎您的第一次上线！");
		pToolUdp->SendData((char*)&noMsg, fromAddr);

		cout << "<< kernel:: keepAlive  Client first connect!	 IP: " << inet_ntoa(fromAddr.sin_addr)
			<< " PORT: " << ntohs(fromAddr.sin_port) << endl;
		++nOnlineClients;
		return;
	}

	//非第一次连接 判断他是正常连接还是断线重连 重连的话更新addr
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
			cout << "<< kernel:: keepAlive 客户机id: ["<<msg->data1<<"] 断线重连，地址已存入本地map. IP: " << inet_ntoa(fromAddr.sin_addr)
				<< " PORT: " << ntohs(fromAddr.sin_port) << endl;
			m_mapNoToAdd[msg->no] = fromAddr;
		}
	}
}

//记录登录成功的客户机的addr
void kernel::recordAddr(char* recvData, sockaddr_in addr)
{
	recvBuf* info = (recvBuf*)recvData;
	int no = info->no;
	if (m_mapNoToAdd.count(no) > 0)
	{
		m_mapNoToAdd[no] = addr;
		cout << "kernel:: recordAddr 客户机登录成功，addr已更新; id: " << info->data1 << endl;
	}
	/*else
	{
		m_mapNoToAdd[no] = addr;
		cout << "kernel:: recordAddr 添加新客户机addr成功，id: "<<info->data1 << endl;
	}*/
}

//根据用户id获取他的好友信息
void kernel::getAllFriendInfo(int)
{

}

//根据id获取他的信息
//传出的info的 res1是在线状态 data1是id data2是iconID buf1是tel buf2是email buf3是name buf4是sign
//info的协议需要进一步定义
bool kernel::getInfoFromId(int id, sendBuf* info)
{
	list<string> listInfo;
	char sqlbuf[1024];
	string tel, email, name, sign;
	int iconId = 0;
	sprintf(sqlbuf, "select tel,email,name,iconId,sign from t_user where id = %d;", id);
	if (!m_sql.SelectMySql(sqlbuf, 5, listInfo))
	{
		cout << "kernel:: getInfoFromId 查询一个person信息失败" << endl;
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
		info->res1 = true;				//好友在线
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



//初始化工作函数数组
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