#include "ckernel.h"
#include"UdpNetMediator.h"
#include"TcpClientMediator.h"
#include<QMessageBox>
#include<QDebug>
#include"INet.h"
#include"UdpNet.h"
#include"noconnectdialog.h"
#include<QTextCodec>
#include<QInputDialog>

using namespace std;
//bool nReConnectUDP = true;
//bool nReConnectTCP = true;
extern char* GetIpList();
unsigned __stdcall ReConnect(void* lpVoid);

//CKernel* CKernel::m_pKernel = nullptr;
//信号和槽:
//1 两个类必须直接或间接继承QObject 并且要有Q_OBJECT关键字
//2 在发送数据的类里面定义信号 signals 非函数不要实现 只需要在发送数据的地方Q_EMIT 信号
//3 在接收数据的类里面声明并实现槽函数 slots
//4 在接收数据的类里面绑定，在发送信号的对象new出来以后，绑定信号和槽 connect()
//ckernel.cpp:6:9: In included file: reference to 'byte' is ambiguous
                                // rpcndr.h:64:11: error occurred here
                                //rpcndr.h:63:25: candidate found by name lookup is 'byte'
                                //functional:979:14: candidate found by name lookup is 'std::byte'



CKernel::CKernel(QObject *parent)
    : QObject{parent},m_isOnline(false),userClickLogin(false)
    ,m_bReConnect(false),m_socket(0),m_TCPid(-1),m_tel(""),m_password("")
{
    //初始化协议头数组
    setProtocolMap();
    m_pImDlg = new IMS_Dialog;
    //绑定添加好友信号槽
    connect(m_pImDlg,SIGNAL(SIG_addFriend()),this,SLOT(slot_addFriend()));
    //绑定删除好友信号槽
    connect(m_pImDlg,SIGNAL(SIG_deleteFriend()),this,SLOT(slot_deleteFriend()));
    //绑定发起群聊信号槽
    connect(m_pImDlg,SIGNAL(SIG_createGroup()),this,SLOT(slot_createGroup()));
    //绑定加入群信号槽
    connect(m_pImDlg,SIGNAL(SIG_addGroup()),this,SLOT(slot_addGroup()));
    //绑定APP系统设置信号槽
    connect(m_pImDlg,SIGNAL(SIG_systemSet()),this,SLOT(slot_systemSet()));
    //绑定关闭窗口的信号和槽函数
    connect(m_pImDlg,SIGNAL(SIG_close()),this,SLOT(slot_close()));


    m_ploginDialog = new loginDialog;
    //绑定login窗口的关闭信号
    connect(m_ploginDialog,SIGNAL(SIG_closeLoginDlg())
            ,this,SLOT(slot_closeLoginDlg()));

    //绑定注册信息的槽函数
    connect(m_ploginDialog,SIGNAL(SIG_registerCommit(QString,QString,QString))
            ,this,SLOT(slot_registerCommit(QString,QString,QString)));
    //绑定登录信息的槽函数
    connect(m_ploginDialog,SIGNAL(SIG_loginCommit(QString,QString))
            ,this,SLOT(slot_loginCommit(QString,QString)));

    //m_pImDlg->show();
    m_ploginDialog->showNormal();




    //创建中介类的对象
    m_pMediator = new UdpNetMediator;

    //用中介者的成员---网络功能类对象打开网络
    // if(!m_pMediator->OpenNet())
    // {
    //     //打开失败 提示用户
    //     //QMessageBox::about(m_ploginDialog,"提示","UDP网络打开失败，重连中......");
    //     m_bReConnect = true;
    //     //exit(0);//退出主进程
    // }

    //绑定中介者发送的数据的信号，槽
    connect(m_pMediator,SIGNAL(SIG_ReadyData( char*,int,long))
            ,this,SLOT(slot_ReadyData( char*,int,long)));


    //测试：给自己发个hello
    //char s[4096] = "hello";

    // struct _STRU_ONLINE rq;
    // gethostname(rq.name,sizeof(rq));//获取当前主机名字
    // qDebug()<<rq.name<<" "<<sizeof(rq);
    // Sleep(5000);
    // m_pMediator->SendData((char*)&rq,sizeof(rq),inet_addr("192.168.202.215"));

    //TCP
    //new 一个TCP mediator的对象
    m_pTCPMediator = new TcpClientMediator;

    //打开网络
    if(!m_pTCPMediator->OpenNet())
    {
        //QMessageBox::about(m_ploginDialog,"提示","TCP打开网络失败，重连中......");
        //exit(0);
        m_bReConnect = true;
    }

    qDebug()<<m_pTCPMediator->m_pNet;
    //创建一个线程 每十秒重连一次服务器
    STRU_pINetMediator s;
    //s.pUDP = (void*)m_pMediator;
    s.pTCP = (void*)m_pTCPMediator;
    m_handleReConnect = (HANDLE)_beginthreadex(0//安全级别 0是默认
                                                , 0 //堆栈大小 0是默认
                                                , &ReConnect //起始地址
                                                ,this//参数列表  --转换为void*并成为上一个参数(函数指针)的参数
                                                , 0	//线程状态 0是创建即运行);
                                                , 0); //分配的线程ID





    //绑定中介者发送的数据的信号，槽
    connect(m_pTCPMediator,SIGNAL(SIG_ReadyData( char*,int,long))
            ,this,SLOT(slot_ReadyData( char*,int,long)));
    //测试代码 客户端向服务端发送一个hello world
   //m_pTCPMediator->SendData("Hello world",sizeof("Hello world"),inet_addr(_TCP_IP));
}

//提示断网了
void CKernel::ShowNoConnect()
{
    while(true)
    {
        // if( m_isOnline && m_bReConnect)
        // {
        //     m_pImDlg->setUserInfo(m_iconId,m_name,m_feeling);
        //     QMessageBox::about(m_pImDlg,"断网提示","无网络连接,重连中...");
        //     auto ite = m_mapTCPIdToUseritem.begin();
        //     while(ite != m_mapTCPIdToUseritem.end())
        //     {
        //         ite.value()->m_status = status_offline;
        //         ite++;
        //     }
        // }
        Sleep(5000);
    }
}


//线程函数 重连服务器
unsigned __stdcall ReConnect(void* lpVoid)
{
    noConnectDialog* pNoConnectDialog = nullptr;
    UdpNetMediator* p1;
    TcpClientMediator* p2;
    CKernel* pk = (CKernel*)lpVoid;
    //STRU_pINetMediator* s = (STRU_pINetMediator*)lpVoid;
    while(true)
    {
        if(!pk->m_bReConnect) return 0;
        //设置好友状态
        if( pk->m_isOnline && pk->m_bReConnect)
        {
            pk->m_pImDlg->m_bNoConnect = true;
            //QMessageBox::about(m_pImDlg,"断网提示","无网络连接,重连中...");
            auto ite = pk->m_mapTCPIdToUseritem.begin();
            while(ite != pk->m_mapTCPIdToUseritem.end())
            {
                ite.value()->m_status = status_offline;
                ite++;
            }
        }
        else
        {
            pk->m_pImDlg->m_bNoConnect = false;
        }
        //pNoConnectDialog = new noConnectDialog;
        // if(!((UdpNetMediator*)s.pUDP)->OpenNet())
        // {
        //     pNoConnectDialog->setWindowTitle(QString("UDP错误"));
        //     //pNoConnectDialog->show();
        //     //QMessageBox::about(0,"QQ提示","UDP网络打开失败，重连中......");
        // }

        if(pk && pk->m_pTCPMediator && pk->m_pMediator->m_pNet)
        {
            p2 = (TcpClientMediator*)pk->m_pTCPMediator;
            qDebug()<<p2->m_pNet;
        }
        else
            return 0;
        pk->userClickLogin = false;
        if(!(p2->ConnectNet()))
        {
            //pNoConnectDialog->setWindowTitle(QString("TCP错误"));
            //pNoConnectDialog->show();
            //QMessageBox::about(0,"QQ提示","TCP网络打开失败，重连中......");
            pk->m_bReConnect = true;
        }
        else
        {
            pk->m_bReConnect = false;
            _STRU_TCP_LOGIN_RQ rq;
            rq.userId = pk->m_TCPid;
            if(pk->m_isOnline)  rq.bOnlineStatus = true;
            pk->utf8ToGb2312(pk->m_tel,rq.tel,sizeof(rq.tel));
            pk->utf8ToGb2312(pk->m_password,rq.password,sizeof(rq.password));
            strcpy(rq.password,pk->m_password.toStdString().c_str());
            strcpy(rq.tel,pk->m_tel.toStdString().c_str());

            p2->SendData((char*)&rq,sizeof(rq),0);
        }


        Sleep(13000);
        if(pNoConnectDialog)
            delete pNoConnectDialog;
        pNoConnectDialog = nullptr;
    }

}




CKernel:: ~CKernel()
{
    if(m_pImDlg)
    {
        m_pImDlg->hide();
        delete m_pImDlg;
        m_pImDlg = nullptr;
    }
    if(m_pMediator)
    {
        m_pMediator->CloseNet();
        delete m_pMediator;
        m_pMediator = nullptr;
    }

    //等待线程到while判断
    if (m_handleReConnect && WAIT_TIMEOUT == WaitForSingleObject(m_handleReConnect, 500/*ms*/))
    {
        //等待500ms没有退出循环就强制杀死线程
        TerminateThread(m_handleReConnect, -1);
        CloseHandle(m_handleReConnect); //关闭句柄
    }
}

//槽函数 接收并处理中介者发送来的信息
void CKernel::slot_ReadyData(char* recvData,int nLen,long lFrom)
{

    //qDebug()<<nLen<<" "<<"recv acc __func__"<<recvData;

    //取出协议头 走不同的流程
   PackType type =  *(PackType*)recvData; //转为int* 取出前四个字节的数据
   cout<<"【CKernel::slot_ReadyData, type : "<<type<<"】"<<endl;
   cout<<*recvData<<endl;

   switch(type){
   case _DEF_UDP_ONLINE_RQ://上线请求
    {
       dealOnlineRq(recvData, nLen, lFrom);
        break;
    }
   case _DEF_UDP_ONLINE_RS://上线回复
   {
        dealOnlineRs(recvData, nLen, lFrom);
       break;
   }
   case _DEF_UDP_CHAT_RQ://聊天请求(聊天回复就是下一个聊天请求)
   {
       dealChatRq(recvData, nLen, lFrom);
       break;
   }
   case _DEF_UDP_OFFLINE_RQ://下线请求
   {
       dealOfflineRq(recvData, nLen, lFrom);
       break;
   }
   case _DEF_TCP_CHAT_RQ://TCP 聊天
   {
       dealTCPChatRq(recvData, nLen, lFrom);
       break;
   }
   default:
       break;
   }

   if(type <= _DEF_UDP_OFFLINE_RQ || type == _DEF_TCP_CHAT_RQ) return;
   // 1 取出协议头
   type = *(PackType*)recvData;
   // 2 根据协议头走不同的处理方法
   //计算数组下标
   int index = type - (_DEF_PROTOCOL_BASE + 1);
   //检验下标的合法性
   if(0 <= index && index < _DEF_PROTOCOL_COUNT)
   {
       pKernelDeal pfun = m_mapPackTypeToFun[index];
       if(pfun)
       {
           (this->*pfun)(recvData,nLen,lFrom);
       }
       else
       {
           qDebug()<<"CKernel::slot_ReadyData index value wrong : "<<index;
       }

   }
   else
   {
       qDebug()<<"CKernel::slot_ReadyData index wrong : "<<index;
   }
   //delete传过来的数据
   delete[] recvData;
}


//生成不同的界面  处理不同的请求

//处理上线请求
void CKernel::dealOnlineRq(char* recvData,int nLen,long lFrom)
{
    qDebug()<<"CKernel :: dealOnlineRq";


    // 1，转换数据报
    struct _STRU_ONLINE* rq = (_STRU_ONLINE*)recvData;
    qDebug()<<rq->type<<" "<<rq->name;



    // 2，添加到好友列表

    //new 一个好友信息对象
    UserItem* item = new UserItem;
    struct sockaddr_in addr;
    addr.sin_addr.S_un.S_addr = lFrom;


    item->setFriendInfo(26,rq->name,inet_ntoa(addr.sin_addr));
    //好友添加显示到主窗口的成员层上
    m_pImDlg->addFriend(item);
    //m_pImDlg->showFriend();




    //绑定显示窗口的信号和槽函数
    connect(item,SIGNAL(SIG_showChatDialog(QString,int))
            ,this,SLOT(slot_showChatDialog(QString,int)));




    //3，new 一个和这个人聊天的窗口
    ChatDialog* chat = new ChatDialog;
    m_mapIpToChatdlg[lFrom] = chat;//保存聊天窗口
    chat->setChatDialogInfo(inet_ntoa(addr.sin_addr));



    //绑定点击send发送按钮时的发送信息信号和槽函数
    connect(chat,SIGNAL( SIG_sendMsg(QString,QString,int))
            ,this,SLOT(slot_sendMsg(QString,QString,int)));

    //绑定发送文件信号
    connect(chat,SIGNAL( SIG_sendFriendFile(int))
            ,this,SLOT(slot_sendFriendFile(int)));


    // 4，判断是不是自己发的广播
    //INet::m_psetLongIpList = (char*)GetIpList();
    set<long> iplist = *(set<long>*)UdpNet::m_psetLongIpList;
    if(iplist.find(lFrom) != iplist.end())
    {
        return;
    }


    // 5，写一个上线回复包rs
    struct _STRU_ONLINE rs;
    rs.type = _DEF_UDP_ONLINE_RS;
    gethostname(rs.name,sizeof(rs));
    m_pMediator->SendData((char*)&rs,sizeof(rs),lFrom);
}




//处理上线回复
void CKernel::dealOnlineRs(char* recvData,int nLen,long lFrom)
{
     qDebug()<<"CKernel :: dealOnlineRs";

    //删除map_chat
     auto ite = m_mapIdToChatdlg.begin();
     while(ite != m_mapIdToChatdlg.end())
     {
         ite = m_mapIdToChatdlg.erase(ite);
     }
     auto ite2 = m_mapTCPIdToUseritem.begin();
     while(ite2 != m_mapTCPIdToUseritem.end())
     {
         ite2 = m_mapTCPIdToUseritem.erase(ite2);
     }

    //1，转换数据报
    struct _STRU_ONLINE* rq = (_STRU_ONLINE*)recvData;
    qDebug()<<rq->type<<" "<<rq->name;   



    //2，添加到好友列表
    //new 一个好友对象
    UserItem* item = new UserItem;
    struct sockaddr_in addr;
    addr.sin_addr.S_un.S_addr = lFrom;

    item->setFriendInfo(26,rq->name,inet_ntoa(addr.sin_addr));

    m_pImDlg->addFriend(item);




    //绑定显示窗口的信号和槽函数
    connect(item,SIGNAL(SIG_showChatDialog(QString))
            ,this,SLOT(slot_showChatDialog(QString)));




    //new 一个和这个人聊天的窗口
    ChatDialog* chat = new ChatDialog;
     chat->setChatDialogInfo(inet_ntoa(addr.sin_addr));
    m_mapIpToChatdlg[lFrom] = chat;




     //绑定点击send发送按钮时的发送信息信号和槽函数
     connect(chat,SIGNAL( SIG_sendMsg(QString,QString,int))
             ,this,SLOT(slot_sendMsg(QString,QString,int)));
    //绑定发送文件信号
    connect(chat,SIGNAL( SIG_sendFriendFile(int))
            ,this,SLOT(slot_sendFriendFile(int)));
}





//处理聊天请求
void CKernel::dealChatRq(char* recvData,int nLen,long lFrom)
{
     qDebug()<<"CKernel :: dealChatRq";
    //拆包
     struct _STRU_CHAT_RQ* rq = (_STRU_CHAT_RQ*)recvData;


     //找到和这个人的聊天窗口
     if(m_mapIpToChatdlg.count(lFrom) > 0)
     {
         ChatDialog* chat = m_mapIpToChatdlg[lFrom];
         //把聊天内容设置到窗口上
         chat->setContent(rq->content);
         //显示聊天内容
         chat->showNormal();
     }

}



//处理下线请求
void CKernel::dealOfflineRq(char* recvData,int nLen,long lFrom)
{
     qDebug()<<"CKernel :: dealOfflineRq";
    //拆包
     struct _STRU_OFFLINE_RQ* rq = (_STRU_OFFLINE_RQ*)recvData;

    //回收聊天窗口 从map中移除节点
     auto ite = m_mapIpToChatdlg.find(lFrom);
     if(ite != m_mapIpToChatdlg.end())
     {
         if(*ite)
         {
             (*ite)->hide();
             delete (*ite);
             (*ite) = nullptr;
         }
         m_mapIpToChatdlg.erase(ite);
     }
    //从好友列表上删除
     in_addr addr;
     addr.S_un.S_addr = lFrom;
     m_pImDlg->deleteFriend(inet_ntoa(addr));
}





void CKernel::slot_showChatDialog(QString ip,int id)
{
    if(id == -1) //UDP好友
    {
        long l = inet_addr(ip.toStdString().c_str());
        if(m_mapIpToChatdlg.count(l) > 0)
        {
            ChatDialog* chat = m_mapIpToChatdlg[l];
            chat->showNormal();
        }
    }
    else //TCP好友
    {
        if(m_mapIdToChatdlg.count(id) > 0)
        {
            ChatDialog* chat = m_mapIdToChatdlg[id];
            chat->showNormal();
        }
    }

}
//char*是基础类型 可以给std::string QString 直接赋值
//std::string ---> char* :   str.c_str()
//QString ---> std::string : QStr.toStdString()

//显示当前群聊的聊天窗口
void CKernel::slot_showGroupChatDialog(QString ip,int id)
{
    if(id == -1) //UDP好友
    {
        // long l = inet_addr(ip.toStdString().c_str());
        // if(m_mapIpToChatdlg.count(l) > 0)
        // {
        //     GroupChatDialog* chat = m_mapIpToChatdlg[l];
        //     chat->showNormal();
        // }
        QMessageBox::about(m_pImDlg,"群聊界面","groupId == -1");
    }
    else //TCP好友
    {
        if(m_mapGroupIdToChatdlg.count(id) > 0)
        {
            GroupChatDialog* chat = m_mapGroupIdToChatdlg[id];
            chat->showNormal();
        }
        else
        {
            QMessageBox::about(m_pImDlg,"群聊界面","没存这个groupId");
        }
    }
}



//chat_dialog 的发送按钮的槽函数
void CKernel::slot_sendMsg(QString content,QString  ip,int id)
{
    // if(m_bReConnect)
    // {
    //     ChatDialog* chat = m_mapIdToChatdlg[id];
    //     chat->showNormal();
    //     chat->setContent("",true);
    //     QMessageBox::about(chat,"TCP网络错误","正在重连服务器...");
    //     return;
    // }

    if(id == -1)//UDP好友信息
    {
        //写一个聊天请求发给对端
        struct _STRU_CHAT_RQ rq;
        strcpy(rq.content,content.toStdString().c_str());

        //发给对端
        long lip = inet_addr(ip.toStdString().c_str());
        m_pMediator->SendData((char*)&rq,sizeof(rq),lip);
    }
    else //TCP好友信息
    {
        //写一个聊天请求
        struct _STRU_TCP_CHAT_RQ rq;
        rq.fromId = m_TCPid;
        rq.toId = id;
        utf8ToGb2312(content,rq.content,sizeof(rq.content));

        //发给服务器
        setNetStatus(m_pTCPMediator->SendData((char*)&rq,sizeof(rq),0));

    }
 }

//处理群的聊天内容
 void CKernel::slot_sendGroupMsg(QString content,QString  ip,int id)
{
     //QMessageBox::about(m_pImDlg,"发送群聊信息","发信息");
    _STRU_TCP_GROUP_CHAT_MSG msg;
     msg.groupId = id;
    msg.memberId = m_TCPid;
    utf8ToGb2312(content,msg.content,sizeof(msg.content));
    m_pTCPMediator->SendData((char*)&msg,sizeof(msg),0);
}

//确认关闭应用程序
 void CKernel::slot_close()
 {
    //UDP发送广播 通知大家我下线了
     struct _STRU_OFFLINE_RQ rq;
    m_pMediator->SendData((char*)&rq,sizeof(rq)
                           ,inet_addr("255.255.255.255"));
    //TCP发送一个下线请求给服务器
    _STRU_TCP_OFFLINE_RQ trq;
    trq.id = m_TCPid;
    m_pTCPMediator->SendData((char*)&trq,sizeof(trq),0);
    //回收资源
    if(m_pImDlg)
    {
        m_pImDlg->hide();
        delete m_pImDlg;
        m_pImDlg = nullptr;
    }
    if(m_ploginDialog)
    {
        delete m_ploginDialog;
        m_ploginDialog = nullptr;
    }
    if(m_pMediator)
    {
        m_pMediator->CloseNet();
        delete m_pMediator;
        m_pMediator = nullptr;
    }
    if(m_pTCPMediator)
    {
        m_pTCPMediator->CloseNet();
        delete m_pTCPMediator;
        m_pTCPMediator = nullptr;
    }
    for(auto ite = m_mapIpToChatdlg.begin();ite != m_mapIpToChatdlg.end();)
    {
        ChatDialog* pchat = *ite;
        if(pchat)
        {
            pchat->hide();
            delete pchat;
            pchat = nullptr;
        }
        //移除ite
        ite = m_mapIpToChatdlg.erase(ite);
    }
    for(auto ite = m_mapIdToChatdlg.begin();ite != m_mapIdToChatdlg.end();)
    {
        ChatDialog* pchat = *ite;
        if(pchat)
        {
            pchat->hide();
            delete pchat;
            pchat = nullptr;
        }
        //移除ite
        ite = m_mapIdToChatdlg.erase(ite);
    }
    //退出进程
    exit(0);
}

 //处理loginDialog发送的关闭信号
void CKernel::slot_closeLoginDlg()
{
    //回收资源
    if(m_pImDlg)
    {
        m_pImDlg->hide();
        delete m_pImDlg;
        m_pImDlg = nullptr;
    }
    if(m_ploginDialog)
    {
        delete m_ploginDialog;
        m_ploginDialog = nullptr;
    }
    if(m_pMediator)
    {
        m_pMediator->CloseNet();
        delete m_pMediator;
        m_pMediator = nullptr;
    }
    if(m_pTCPMediator)
    {
        m_pTCPMediator->CloseNet();
        delete m_pTCPMediator;
        m_pTCPMediator = nullptr;
    }
    for(auto ite = m_mapIpToChatdlg.begin();ite != m_mapIpToChatdlg.end();)
    {
        ChatDialog* pchat = *ite;
        if(pchat)
        {
            pchat->hide();
            delete pchat;
            pchat = nullptr;
        }
        //移除ite
        ite = m_mapIpToChatdlg.erase(ite);
    }
    for(auto ite = m_mapIdToChatdlg.begin();ite != m_mapIdToChatdlg.end();)
    {
        ChatDialog* pchat = *ite;
        if(pchat)
        {
            pchat->hide();
            delete pchat;
            pchat = nullptr;
        }
        //移除ite
        ite = m_mapIdToChatdlg.erase(ite);
    }
    //退出进程
    exit(0);
}

//处理注册按钮的信号
 void CKernel::slot_registerCommit(QString name,QString tel,QString password)
{

     cout<<__func__<<endl;
    // 打包
     _STRU_TCP_REGISTER_RQ rq;

     //转码
     utf8ToGb2312(name/*源*/,rq.name/*目标*/,sizeof(rq.name));
     utf8ToGb2312(tel/*源*/,rq.tel/*目标*/,sizeof(rq.tel));
     utf8ToGb2312(password/*源*/,rq.password/*目标*/,sizeof(rq.password));

     //strcpy(rq.name,name.toStdString().c_str());
     //strcpy(rq.tel,tel.toStdString().c_str());
     //strcpy(rq.password,password.toStdString().c_str());

    // 发送给服务器
    m_pTCPMediator->SendData((char*)&rq,sizeof(rq),0);
}

//处理登录按钮的信号
void CKernel::slot_loginCommit(QString tel,QString password)
{
    userClickLogin = true;

    //判断是否连接服务器
    if(m_bReConnect)
    {
        QMessageBox::about(m_ploginDialog,"TCP网络错误","未连接服务器,修复中...");
        m_isOnline = false;
        return;
    }
    m_tel = tel;
    m_password = password;
    cout<<__func__<<endl;
    // 打包
    _STRU_TCP_LOGIN_RQ rq;
    //转码
    utf8ToGb2312(tel/*源*/,rq.tel/*目标*/,sizeof(rq.tel));
    utf8ToGb2312(password/*源*/,rq.password/*目标*/,sizeof(rq.password));

    //strcpy(rq.tel,tel.toStdString().c_str());
    //strcpy(rq.password,password.toStdString().c_str());

    // 发送给服务器
    if(m_pTCPMediator->SendData((char*)&rq,sizeof(rq),0) <= 0)
    {
        QMessageBox::about(m_pImDlg,"发送失败","TCP网络未连接,重连中...");
    }
}

//处理注册回复包rs
void CKernel::dealRegisterRs(char* recvData,int nLen,long lFrom)
{
    // 1 拆包
    _STRU_TCP_REGISTER_RS* rs = (_STRU_TCP_REGISTER_RS*)recvData;
    switch(rs->m_nRegister)
    {
    case register_false_name_used:
        QMessageBox::about(m_ploginDialog,"注册结果提示","用户名已被占用，请重试");
        break;
    case register_false_tel_used:
        QMessageBox::about(m_ploginDialog,"注册结果提示","手机号已被注册，请更换");
        break;
    case register_false_sql_select_error:
        QMessageBox::about(m_ploginDialog,"注册结果提示","服务器数据库查询错误，请重试");
        break;
    case register_false_sql_update_error:
        QMessageBox::about(m_ploginDialog,"注册结果提示","服务器数据库写入错误，请重试");
        break;
    case register_success:
        QMessageBox::about(m_ploginDialog,"注册结果提示","新用户注册成功，请登录");
        break;
    default:
        break;
    }
}

//处理登录回复包rs
void CKernel::dealLoginRs(char* recvData,int nLen,long lFrom)
{
    cout<<"*** m_isOnline : "<<m_isOnline<<" ***"<<endl;
    // 1 拆包
    _STRU_TCP_LOGIN_RS* rs = (_STRU_TCP_LOGIN_RS*)recvData;
    qDebug()<<"***[ SOCK: "<<rs->sock<<" ]***";
    switch(rs->m_nLogin)
    {
    case login_success:
    {
        //隐藏登录页面
        m_ploginDialog->hide();
        //跳转页面
        m_pImDlg->showNormal();
        Sleep(20);
        //保存自己的id
        m_TCPid = rs->userId;
        m_socket = rs->sock;
        cout<<"***ACC m_sock : "<<m_socket<<" ***"<<endl;
        m_isOnline = true;
        setNetStatus(1);
    }
         break;
    case login_false_no_tel:
    {
        m_socket = rs->sock;
        m_isOnline = false;
        setNetStatus(-1);
        QMessageBox::about(m_ploginDialog,"登录结果提示","手机号未被注册，请注册");
    }
        break;
    case login_false_password_wrong:
    {
        m_socket = rs->sock;
        m_isOnline = false;
        setNetStatus(-1);
        QMessageBox::about(m_ploginDialog,"登录结果提示","密码错误，请重试");
    }
        break;
    case login_false_sql_select_error:
    {
        m_socket = rs->sock;
        m_isOnline = false;
        setNetStatus(-1);
        QMessageBox::about(m_ploginDialog,"登录结果提示","服务器数据库查询错误，请重试");
    }
        break;
    case login_false_illegal:
    {
        if(!m_isOnline && userClickLogin)
        {
            m_socket = rs->sock;
            m_isOnline = false;
            setNetStatus(-1);
            QMessageBox::about(m_ploginDialog,"登录结果提示","该账号已登录，请更换");
            return;
        }
        m_socket = rs->sock;
        cout<<"***ILL m_sock : "<<m_socket<<" ***"<<endl;
        setNetStatus(1);

    }
        break;
    default:
        break;
    }
}

//处理好友信息数据包
void CKernel::dealFriendInfo(char* recvData,int nLen,long lFrom)
{

    // 1 拆包
    _STRU_TCP_FRIEND_INFO *rs = (_STRU_TCP_FRIEND_INFO*)recvData;
    //转码
    QString name = gb2312ToUtf8(rs->name);
    QString feeling = gb2312ToUtf8(rs->feeling);

    // 2 判断是不是发给自己的
    if(rs->id == m_TCPid)
    {
        m_name = name;
        m_iconId = rs->iconId;
        m_feeling = feeling;
        //把自己的信息设置到界面上
        m_pImDlg->setUserInfo(rs->iconId,name,feeling,m_socket);
        return;
    }
    else
    {
        qDebug()<<"CKernel::dealFriendInfo Not my rq";
    }



    // 4 判断好友是否已经存在列表上
    if(m_mapTCPIdToUseritem.count(rs->id) <= 0)
    {
        if(rs->id == m_TCPid) return;
        if(gb2312ToUtf8(rs->name) == m_name)
        {
            QMessageBox::about(m_pImDlg,"客户端错误",QString("你的id: %1|列表【%2】id: %3").arg(m_TCPid).arg(m_name).arg(rs->id));
        }
       // 不存在 new一个好友
        UserItem* item = new UserItem;
       //绑定显示窗口的信号和槽函数
       connect(item,SIGNAL(SIG_showChatDialog(QString,int))
               ,this,SLOT(slot_showChatDialog(QString,int)));

       // 设置好友信息
       item->setFriendInfo(rs->iconId,name,feeling,rs->status,rs->id);
       // 保存好友信息到map中
        m_pImDlg->addFriend(item,rs->id);
        m_mapTCPIdToUseritem[rs->id] = item;
       // new一个和该好友的聊天窗口
        ChatDialog* chat = new ChatDialog;
        //绑定点击send发送按钮时的发送信息信号和槽函数
        connect(chat,SIGNAL( SIG_sendMsg(QString,QString,int))
                ,this,SLOT(slot_sendMsg(QString,QString,int)));
        //绑定发送文件信号
        connect(chat,SIGNAL( SIG_sendFriendFile(int))
                ,this,SLOT(slot_sendFriendFile(int)));
       // 设置聊天窗口的信息
        chat->setChatDialogInfo("",rs->id,name);

       // 聊天窗口保存到map中
        m_mapIdToChatdlg[rs->id] = chat;
    }
    else
    {
       // 存在 更新好友信息
       UserItem* item = m_mapTCPIdToUseritem[rs->id];
       item->setFriendInfo(rs->iconId,name,feeling,rs->status,rs->id);
       //m_pImDlg->addFriend(item,rs->id);
    }
}

//处理聊天内容 显示到界面
void CKernel::dealTCPChatRq(char* recvData, int nLen, long lFrom)
{
    qDebug()<<"【CKernel::dealTCPChatRq】";
    _STRU_TCP_CHAT_RQ *rq = (_STRU_TCP_CHAT_RQ*)recvData;
    if(rq->toId != m_TCPid && rq->fromId != m_TCPid)
    {
        QMessageBox::about(m_pImDlg,"接收错误","服务器错误！");
        return;
    }
    QString content = gb2312ToUtf8(rq->content);
    qDebug()<<"Before【"<<content<<"】";
    if(m_mapIdToChatdlg.count(rq->fromId) > 0)
    {
        ChatDialog* chat = m_mapIdToChatdlg[rq->fromId];
        //把聊天内容设置到窗口上
        chat->setContent(content);
        //显示聊天内容
        chat->showNormal();

        qDebug()<<"【"<<content<<"】";
        return;
    }


    if(m_mapIdToChatdlg.count(rq->toId) > 0)
    {
        ChatDialog* chat = m_mapIdToChatdlg[rq->toId];
        //把聊天内容设置到窗口上
        chat->setContent(content);
        //显示聊天内容
        chat->showNormal();

        qDebug()<<"【"<<content<<"】";
    }
}

//处理聊天失败 弹窗报错
void CKernel::dealTCPChatRs(char* recvData, int nLen, long lFrom)
{
    _STRU_TCP_CHAT_RS *rs = (_STRU_TCP_CHAT_RS*)recvData;

    if(m_mapIdToChatdlg.count(rs->fromId) > 0)
    {
        ChatDialog* chat = m_mapIdToChatdlg[rs->fromId];
        chat->showNormal();
        QMessageBox::about(chat,"发送失败",QString("好友%1离线！")
                                                 .arg( m_mapTCPIdToUseritem[rs->fromId]->m_name ));
    }
    else
    {
        QMessageBox::about(m_pImDlg,"发送失败","服务器错误！");
    }
}

//处理添加好友申请数据报
void CKernel::dealAddFriendRq(char* recvData,int nLen,long lFrom)
{
    // 1 拆包
    _STRU_TCP_ADD_FRIEND_RQ *rq = (_STRU_TCP_ADD_FRIEND_RQ*)recvData;
    // 2 发给服务器同意与否
    QString toName = gb2312ToUtf8(rq->toName);
    if(toName != m_name)
    {
        QMessageBox::about(m_pImDlg,"接受添加申请错误",QString("您id:%1,接收了%2").arg(m_TCPid).arg(rq->fromId));
        return;
    }
    QString name = gb2312ToUtf8(rq->fromName);
    QMessageBox:: StandardButton result= QMessageBox::question(m_pImDlg,"好友申请",QString("【%1】想添加您为好友").arg(name));
    if(result == QMessageBox::Yes) //同意
    {
        _STRU_TCP_ADD_FRIEND_RS rs;
        rs.friendId = m_TCPid;
        char myName[50];
        utf8ToGb2312(m_name,myName,sizeof(myName));
        strcpy(rs.friendName , myName);
        rs.fromId = rq->fromId;
        rs.m_nAddRes = add_friend_success;
        qDebug()<<"我同意了【"<<name<<"】的好友申请";
        if(rs.friendId == rq->fromId)
        {
            QMessageBox::about(m_pImDlg,"接受添加申请错误","同意时id冲突");
            return;
        }
        m_pTCPMediator->SendData((char*)&rs,sizeof(rs),0);

    }
    else //不同意
    {
        _STRU_TCP_ADD_FRIEND_RS rs;
        rs.friendId = m_TCPid;
        char myName[50];
        utf8ToGb2312(m_name,myName,sizeof(myName));
        strcpy(rs.friendName , myName);
        rs.fromId = rq->fromId;
        rs.m_nAddRes = add_friend_false_disagree;
        qDebug()<<"我拒绝了【"<<name<<"】的好友申请";
        if(rs.friendId == rq->fromId)
        {
            QMessageBox::about(m_pImDlg,"接受添加申请错误","同意时id冲突");
            return;
        }
        m_pTCPMediator->SendData((char*)&rs,sizeof(rs),0);
    }
}

//处理添加好友回复数据报
void CKernel::dealAddFriendRs(char* recvData,int nLen,long lFrom)
{
    // 1 拆包
    _STRU_TCP_ADD_FRIEND_RS *rs = (_STRU_TCP_ADD_FRIEND_RS*)recvData;
    QString name = gb2312ToUtf8(rs->friendName);
    // 2 判断回复结果是否同意
    if(rs->fromId != m_TCPid)
    {
        QMessageBox::about(m_pImDlg,"添加回复接受错误",QString("您id:%1,接收了%2").arg(m_TCPid).arg(rs->fromId));
        return;
    }
    if(rs->m_nAddRes == add_friend_false_disagree)
    {
        QMessageBox::about(m_pImDlg,"添加结果",QString("用户【%1】未同意您的好友申请").arg(name));
        return;
    }
    else if(rs->m_nAddRes == add_friend_success)
    {
        QMessageBox::about(m_pImDlg,"添加结果",QString("已添加【%1】为好友").arg(name));
        return;
    }
    else if(rs->m_nAddRes == add_friend_false_no_exist)
    {
        QMessageBox::about(m_pImDlg,"添加结果",QString("用户名【%1】未注册!").arg(name));
        return;
    }
    else if(rs->m_nAddRes == add_friend_false_offline)
    {
        QMessageBox::about(m_pImDlg,"添加结果",QString("用户【%1】离线,请稍后重试...").arg(name));
        return;
    }
}

//处理服务器的验证登录状态请求
void CKernel::dealValidateLogined(char* recvData,int nLen,long lFrom)
{
    // 拆包
    _STRU_TCP_VALIDATE_RQ *rq = (_STRU_TCP_VALIDATE_RQ*)recvData;
    // 处理
    if(rq->id == m_TCPid)
    {
        _STRU_TCP_VALIDATE_RS rs;
        rs.id = m_TCPid;
        rs.status = true;
        m_pTCPMediator->SendData((char*)&rs,sizeof(rs),0);
    }
    else
    {
        QMessageBox::about(m_pImDlg,"服务器错误",QString("服务器发送验证登录错误,toId:%1").arg(rq->id));
    }
}

//处理删除好友回复
void CKernel::dealDeleteFriendRs(char* recvData,int nLen,long lFrom)
{
    _STRU_TCP_DELETE_FRIEND_RS *rs = (_STRU_TCP_DELETE_FRIEND_RS*)recvData;
    if(rs->deleteRes == delete_friend_false)
    {
        QMessageBox::about(m_pImDlg,"删除失败","服务器删除失败");
        return;
    }
    if(m_mapIdToChatdlg.count(rs->friendId) > 0)
    {
        ChatDialog* chat = m_mapIdToChatdlg[rs->friendId];
        chat->hide();
        m_mapIdToChatdlg.remove(rs->friendId);
        delete chat;
    }
    else
    {
        QMessageBox::about(m_pImDlg,"删除失败","客户端删除好友失败1");
        return;
    }
    if(m_mapTCPIdToUseritem.count(rs->friendId) > 0)
    {
        UserItem* item = m_mapTCPIdToUseritem[rs->friendId];
        item->hide();
        m_mapTCPIdToUseritem.remove(rs->friendId);
        delete item;
    }
    else
    {
        QMessageBox::about(m_pImDlg,"删除失败","客户端删除好友失败2");
        return;
    }
    QString fName = gb2312ToUtf8(rs->friendName);
    QMessageBox::about(m_pImDlg,"删除成功",QString("好友【%1】已被删除!").arg(fName));
}

//处理被朋友删除的函数
void CKernel::dealFriendDeleteYouMsg(char* recvData,int nLen,long lFrom)
{
    _STRU_TCP_FRIEND_DELETE_YOU_MSG* msg = (_STRU_TCP_FRIEND_DELETE_YOU_MSG*)recvData;
    if(m_mapIdToChatdlg.count(msg->friendId) > 0)
    {
        ChatDialog* chat = m_mapIdToChatdlg[msg->friendId];
        chat->hide();
        m_mapIdToChatdlg.remove(msg->friendId);
        delete chat;
    }
    else
    {
        QMessageBox::about(m_pImDlg,"删除失败","客户端被好友删除失败1");
        return;
    }
    if(m_mapTCPIdToUseritem.count(msg->friendId) > 0)
    {
        UserItem* item = m_mapTCPIdToUseritem[msg->friendId];
        item->hide();
        m_mapTCPIdToUseritem.remove(msg->friendId);
        delete item;
    }
    else
    {
        QMessageBox::about(m_pImDlg,"删除失败","客户端被好友删除失败2");
        return;
    }
}

//处理自己加入的群的信息
void CKernel::dealMyGroupInfo(char* recvData,int nLen,long lFrom)
{
    _STRU_TCP_SEND_CLIENT_GROUP_INFO* info = (_STRU_TCP_SEND_CLIENT_GROUP_INFO*)recvData;
    if(info->groupStatus)
    {
        qDebug()<<"我加入了一个群："<<gb2312ToUtf8(info->groupName)<<" groupId: "<<info->groupId<<" 无其他成员在线";
    }
    else
    {
        qDebug()<<"我加入了一个群："<<gb2312ToUtf8(info->groupName)<<" groupId: "<<info->groupId<<" 有别的成员在线";
    }

    QString groupName = gb2312ToUtf8(info->groupName);
    // 4 判断群聊是否已经存在列表上
    if(m_mapGroupIdToGroupitem.count(info->groupId) <= 0)
    {
        // 不存在 new一个群聊组件
        GroupItem* item = new GroupItem;
        //绑定显示窗口的信号和槽函数
        connect(item,SIGNAL(SIG_showGroupChatDialog(QString,int))
                ,this,SLOT(slot_showGroupChatDialog(QString,int)));

        // 设置群聊信息
        item->setGroupInfo(info->groupIconid,groupName,info->groupStatus,info->groupId);
        // 保存群聊信息到map中
        m_pImDlg->addGroup(item,info->groupId);
        m_mapGroupIdToGroupitem[info->groupId] = item;
        // new一个和该群聊的聊天窗口
        GroupChatDialog* chat = new GroupChatDialog;
        //绑定点击send发送按钮时的发送信息信号和槽函数
        connect(chat,SIGNAL( SIG_sendGroupMsg(QString,QString,int))
                ,this,SLOT(slot_sendGroupMsg(QString,QString,int)));
        //绑定和群成员聊天 加群聊的信号槽
        connect(chat,SIGNAL(SIG_memberChatToKernel(int,QString,GroupChatDialog*)),this,SLOT(slot_memberChatToKernel(int,QString,GroupChatDialog*)));
        connect(chat,SIGNAL(SIG_memberAddToKernel(int,QString,GroupChatDialog*)),this,SLOT(slot_memberAddToKernel(int,QString,GroupChatDialog*)));
        //绑定发送文件信号
        connect(chat,SIGNAL( SIG_sendGroupFile(int))
                ,this,SLOT(slot_sendGroupFile(int)));
        // 设置聊天窗口的信息
        chat->setG_ChatDialogInfo("",info->groupId,groupName);

        // 聊天窗口保存到map中
        m_mapGroupIdToChatdlg[info->groupId] = chat;
    }
    else
    {
        // 存在 更新群聊信息
        GroupItem* item = m_mapGroupIdToGroupitem[info->groupId];
        item->setGroupInfo(info->groupIconid,groupName,info->groupStatus,info->groupId);
    }
}

//处理自己加入的群的成员的信息
void CKernel::dealMyGroupMemberInfo(char* recvData,int nLen,long lFrom)
{
    //等待群聊组件构建成功
    Sleep(20);
    _STRU_TCP_SEND_CLIENT_GROUP_MEMBERS_INFO* info = (_STRU_TCP_SEND_CLIENT_GROUP_MEMBERS_INFO*)recvData;

    char name1[100] = ""; char name2[100] = "";
    strcpy(name1, info->groupName);
    strcpy(name2, info->memberName);
    QString nameG = gb2312ToUtf8(name1);
    QString nameM = gb2312ToUtf8(name2);
    if(info->memberStatus == 1)
    {
        qDebug()<<"群："<<nameG<<" 里有 id: "<<info->memberId<<" name: "<<nameM<<" len:"<<strlen(info->memberName)<<" 离线";
    }
    else qDebug()<<"群："<<nameG<<" 里有 id: "<<info->memberId<<" name: "<<nameM<<" len"<<strlen(info->memberName)<<" 在线";


    //更新群信息
    GroupItem* item = m_mapGroupIdToGroupitem[info->groupId];
    item->setGroupInfo(info->groupIconid,nameG,info->groupStatus,info->groupId);

    //new MemberItem加到群的groupchatdialog上
    QString memberName = gb2312ToUtf8(info->memberName);

    // 4 找到对应群的groupchatdialog 判断成员是否已经存在列表上
    GroupChatDialog* gChat = m_mapGroupIdToChatdlg[info->groupId];
    if(gChat->m_mapMemberIdToMemberItem.count(info->memberId) <= 0)
    {
        // 不存在 new一个群成员组件
        MemberItem* item = new MemberItem;
        //绑定信号槽
        connect(item,SIGNAL(SIG_memberChat(int,QString)),gChat,SLOT(slot_memberChat(int,QString)));
        //绑定信号槽
        connect(item,SIGNAL(SIG_memberAdd(int,QString)),gChat,SLOT(slot_memberAdd(int,QString)));

        // 设置群成员信息
        item->setGroupMemberInfo(info->memberIconid,memberName,info->memberStatus,info->memberId);
        // 保存群成员信息到map中
        gChat->addMember(item,info->memberId);
        gChat->m_mapMemberIdToMemberItem[info->memberId] = item;

    }
    else
    {
        // 存在 更新成员信息
        MemberItem* item = gChat->m_mapMemberIdToMemberItem[info->memberId];
        item->setGroupMemberInfo(info->memberIconid,memberName,info->memberStatus,info->memberId);
    }
}

//处理群聊消息
void CKernel::dealGroupMsg(char* recvData,int nLen,long lFrom)
{
    _STRU_TCP_GROUP_CHAT_MSG *msg = (_STRU_TCP_GROUP_CHAT_MSG*)recvData;
    GroupChatDialog* chat = nullptr;
    //找到那个群的聊天界面
    if(m_mapGroupIdToGroupitem.count(msg->groupId) > 0)
    {
        chat = m_mapGroupIdToChatdlg[msg->groupId];
    }
    else
    {
        QMessageBox::about(m_pImDlg,QString("群聊%1信息接收").arg(msg->groupId),QString("没有这个群%1").arg(msg->groupId));
        return;
    }
    //验证是不是这个群的成员发的消息
    if(chat->m_mapMemberIdToMemberItem.count(msg->memberId) > 0)
    {
        chat->setG_Content(gb2312ToUtf8(msg->content)
                           ,chat->m_mapMemberIdToMemberItem[msg->memberId]->m_name);
        chat->showNormal();
    }
    else
    {
        QMessageBox::about(m_pImDlg,QString("群聊%1信息接收").arg(msg->groupId),QString("没有这个群成员%1").arg(msg->memberId));
        return;
    }
}

//处理服务器的转发文件询问
void CKernel::dealServerSendFileMsg(char* recvData,int nLen,long lFrom)
{
    _STRU_TCP_CLIENT_RECV_FILE_MSG* msg = (_STRU_TCP_CLIENT_RECV_FILE_MSG*)recvData;
    msg->fromFriOrGop; //是群还是好友
    msg->szFileId;     //文件id
    msg->id;           //接收端的id 即自己的id或者你加入的群的一个id
    msg->userId;       //发送端的id
    if(msg->fromFriOrGop == file_friend)
    {
        ChatDialog* chat = m_mapIdToChatdlg[msg->userId];
        UserItem* item = m_mapTCPIdToUseritem[msg->userId];
        chat->showNormal();
        QMessageBox:: StandardButton result= QMessageBox::question(chat,"接收文件",QString("是否接收好友【%1】的文件").arg(item->m_name));
        if(result == QMessageBox::No)
        {
            //回复拒绝接受文件的回复 回复时要把rs的id写为自己的id
            _STRU_TCP_CLIENT_RECV_FILE_RES res;
            strcpy(res.szFileId , msg->szFileId);
            res.userRecvRes = user_recv_file_no;
            res.id = m_TCPid;
            res.fromFriOrGop = msg->fromFriOrGop;
            res.userId = msg->userId;
            m_pTCPMediator->DealData((char*)&res,sizeof(res),0);
        }
        else
        {
            //回复接受文件的回复
            _STRU_TCP_CLIENT_RECV_FILE_RES res;
            strcpy(res.szFileId , msg->szFileId);
            res.userRecvRes = user_recv_file_yes;
            res.id = m_TCPid;
            res.fromFriOrGop = msg->fromFriOrGop;
            res.userId = msg->userId;
            m_pTCPMediator->DealData((char*)&res,sizeof(res),0);
        }
    }
    else
    {
        GroupChatDialog* chat = m_mapGroupIdToChatdlg[msg->id];
        GroupItem* item = m_mapGroupIdToGroupitem[msg->id];
        chat->showNormal();
        QMessageBox:: StandardButton result= QMessageBox::question(chat,"接收文件",QString("是否接收群聊【%1】的文件").arg(item->m_name));
        if(result == QMessageBox::No)
        {
            //回复拒绝接受文件的回复 回复时要把rs的id写为自己的id
            _STRU_TCP_CLIENT_RECV_FILE_RES res;
            strcpy(res.szFileId , msg->szFileId);
            res.userRecvRes = user_recv_file_no;
            res.id = msg->id;
            res.fromFriOrGop = msg->fromFriOrGop;
            res.userId = msg->userId;
            m_pTCPMediator->DealData((char*)&res,sizeof(res),0);
        }
        else
        {
            //回复接受文件的回复
            _STRU_TCP_CLIENT_RECV_FILE_RES res;
            strcpy(res.szFileId , msg->szFileId);
            res.userRecvRes = user_recv_file_yes;
            res.id = msg->id;
            res.fromFriOrGop = msg->fromFriOrGop;
            res.userId = msg->userId;
            m_pTCPMediator->DealData((char*)&res,sizeof(res),0);
        }
    }
}

//处理发送文件结果
void CKernel::dealSendFileResult(char* recvData,int nLen,long lFrom)
{
    _STRU_TCP_SEND_FILE_RES *res = (_STRU_TCP_SEND_FILE_RES*)recvData;
    ChatDialog* chat;
    GroupChatDialog* g_chat;
    if(res->toFriOrGop == file_friend)
    {
        chat = m_mapIdToChatdlg[res->id];
        if(res->sendRes == send_file_succeed)
        {
            QMessageBox::about(chat,"文件发送",QString("好友【%1】已接受文件！")
                                                     .arg(m_mapTCPIdToUseritem[res->id]->m_name));
        }
        else if(res->sendRes == send_file_failed_fri_refuse)
        {
            QMessageBox::about(chat,"文件发送",QString("好友【%1】拒收文件！")
                                                     .arg(m_mapTCPIdToUseritem[res->id]->m_name));
        }
        else if(res->sendRes == send_file_failed_fri_offline)
        {
            QMessageBox::about(chat,"文件发送",QString("好友【%1】离线，请稍后重试！")
                                                     .arg(m_mapTCPIdToUseritem[res->id]->m_name));
        }
        else if(res->sendRes == send_file_failed_ser_recv)
        {
            QMessageBox::about(chat,"文件发送","失败！服务器接收文件出错");
        }
        else if(res->sendRes == send_file_failed_ser_send)
        {
            QMessageBox::about(chat,"文件发送","失败！服务器转发文件出错");
        }
    }
    else
    {
        g_chat = m_mapGroupIdToChatdlg[res->id];
        if(res->sendRes == send_file_succeed || res->sendRes == send_file_failed_fri_refuse)
        {
            QMessageBox::about(g_chat,"文件发送","发送成功！");
        }
        else if(res->sendRes == send_file_failed_gro_offline)
        {
            QMessageBox::about(g_chat,"文件发送","群聊无人在线，请稍后重试！");
        }
        else if(res->sendRes == send_file_failed_ser_recv)
        {
            QMessageBox::about(g_chat,"文件发送","失败！服务器接收文件出错");
        }
        else if(res->sendRes == send_file_failed_ser_send)
        {
            QMessageBox::about(g_chat,"文件发送","失败！服务器转发文件出错");
        }
    }

}

//处理内核接受文件的结果
void CKernel::dealRecvFileResult(char* recvData,int nLen,long lFrom)
{
    STRU_RECV_FILE_RES *res = (STRU_RECV_FILE_RES*)recvData;
    if(res->recvRes == recv_file_succeed)
    {
        QMessageBox::about(m_pImDlg,"提示","文件接收成功！");
    }
    else
    {
        QMessageBox::about(m_pImDlg,"提示","文件接收失败！");
    }
    delete res;//res是new的
}

//初始化协议头数组
void CKernel::setProtocolMap()
{
    cout<<__func__<<endl;
    memset(m_mapPackTypeToFun,0,sizeof(m_mapPackTypeToFun));
    m_mapPackTypeToFun[_DEF_UDP_ONLINE_RQ - (_DEF_PROTOCOL_BASE + 1)]
        = &CKernel::dealOnlineRq;
    m_mapPackTypeToFun[_DEF_UDP_ONLINE_RS - (_DEF_PROTOCOL_BASE + 1)]
        = &CKernel::dealOnlineRs;
    m_mapPackTypeToFun[_DEF_UDP_CHAT_RQ - (_DEF_PROTOCOL_BASE + 1)]
        = &CKernel::dealChatRq;
    m_mapPackTypeToFun[_DEF_UDP_OFFLINE_RQ - (_DEF_PROTOCOL_BASE + 1)]
        = &CKernel::dealOfflineRq;
    m_mapPackTypeToFun[_DEF_TCP_REGISTER_RS - (_DEF_PROTOCOL_BASE + 1)]
        = &CKernel::dealRegisterRs;
    m_mapPackTypeToFun[_DEF_TCP_LOGIN_RS - (_DEF_PROTOCOL_BASE + 1)]
         = &CKernel::dealLoginRs;
     m_mapPackTypeToFun[_DEF_TCP_FRIEND_INFO - (_DEF_PROTOCOL_BASE + 1)]
         = &CKernel::dealFriendInfo;
    m_mapPackTypeToFun[_DEF_TCP_CHAT_RQ - (_DEF_PROTOCOL_BASE + 1)]
         = &CKernel::dealTCPChatRq;
    m_mapPackTypeToFun[_DEF_TCP_CHAT_RS - (_DEF_PROTOCOL_BASE + 1)]
         = &CKernel::dealTCPChatRs;
    m_mapPackTypeToFun[_DEF_TCP_ADD_FRIEND_RS - (_DEF_PROTOCOL_BASE + 1)]
         = &CKernel::dealAddFriendRs;
     m_mapPackTypeToFun[_DEF_TCP_ADD_FRIEND_RQ - (_DEF_PROTOCOL_BASE + 1)]
         = &CKernel::dealAddFriendRq;
     m_mapPackTypeToFun[_DEF_TCP_VALIDATE_LOGINED_RQ - (_DEF_PROTOCOL_BASE + 1)]
         = &CKernel::dealValidateLogined;
    m_mapPackTypeToFun[_DEF_TCP_DELETE_FRIEND_RS - (_DEF_PROTOCOL_BASE + 1)]
         = &CKernel::dealDeleteFriendRs;
    m_mapPackTypeToFun[_DEF_TCP_FRIEND_DELETE_YOU_MSG - (_DEF_PROTOCOL_BASE + 1)]
         = &CKernel::dealFriendDeleteYouMsg;
    m_mapPackTypeToFun[_DEF_TCP_SEND_CLIENT_GROUP_INFO - (_DEF_PROTOCOL_BASE + 1)]
         = &CKernel::dealMyGroupInfo;
    m_mapPackTypeToFun[_DEF_TCP_SEND_CLIENT_GROUP_MEMBERS_INFO - (_DEF_PROTOCOL_BASE + 1)]
        = &CKernel::dealMyGroupMemberInfo;
    m_mapPackTypeToFun[_DEF_TCP_GROUP_CHAT_MSG - (_DEF_PROTOCOL_BASE + 1)]
        = &CKernel::dealGroupMsg;
    m_mapPackTypeToFun[_DEF_TCP_RECV_FILE_MSG - (_DEF_PROTOCOL_BASE + 1)]
        = &CKernel::dealServerSendFileMsg;
    m_mapPackTypeToFun[_DEF_TCP_SEND_FILE_RES - (_DEF_PROTOCOL_BASE + 1)]
    = &CKernel::dealSendFileResult;
    m_mapPackTypeToFun[_DEF_PROTOCOL_FILE_RECV_RES - (_DEF_PROTOCOL_BASE + 1)]
    = &CKernel::dealRecvFileResult;
    //m_mapPackTypeToFun[ - (_DEF_PROTOCOL_BASE + 1)]
    //= &CKernel::;
    //m_mapPackTypeToFun[ - (_DEF_PROTOCOL_BASE + 1)]
    //= &CKernel::;
}

//处理和群成员聊天的信号
void CKernel::slot_memberChatToKernel(int id,QString name,GroupChatDialog* p)
{
    //判断是不是自己 自己的好友
    if(strcmp(m_name.toStdString().c_str(),name.toStdString().c_str()) == 0)
    {
        QMessageBox::about(p,"错误操作",QString("不能和自己聊天"));
        return;
    }
    if(m_mapTCPIdToUseritem.count(id) <= 0)
    {
        QMessageBox::about(p,"错误操作",QString("请先添加【%1】为好友").arg(name));
        return;
    }
    slot_showChatDialog("",id);
}

//处理加群成员为好友的信号
void CKernel::slot_memberAddToKernel(int id,QString name,GroupChatDialog* p)
{
    //判断是不是自己的好友
    if(m_mapTCPIdToUseritem.count(id) > 0)
    {
        QMessageBox::about(p,"错误操作",QString("【%1】已经是您的好友啦").arg(name));
        return;
    }
    //调用添加好友函数
    QString name_temp = name;
    // 2 校验输入的数据是否合法
    //判断是否是空字符串 或者是全空格
    if(name.isEmpty()||name_temp.remove(" ").isEmpty())
    {
        QMessageBox::about(p,"错误提示","输入为空白，请重试");
        return;
    }
    //检查长度是否合法（name <= 20,tel = 11,password <= 20）
    if(name.length() > 20)
    {
        QMessageBox::about(p,"错误提示","输入长度有误，请重试");
        return;
    }
    //检查内容是否合法(tel为数字 name password为大小写字母 数字 下划线的组合)
    if(0)
    {
        QMessageBox::about(p,"错误提示","输入格式有误，请重试");
        return;
    }
    // 3 判断名字是不是自己的
    if(name == m_name)
    {
        QMessageBox::about(p,"错误提示","请勿输入本机昵称");
        return;
    }
    // 4 判断是否已经是好友了
    for(auto ite = m_mapTCPIdToUseritem.begin()
         ;ite != m_mapTCPIdToUseritem.end();ite++)
    {
        if(ite.value() && ite.value()->m_name == name)
        {
            QMessageBox::about(p,"系统提示",QString("【%1】已经是你的好友啦").arg(name));
            return;
        }
    }
    // 5 向服务器发送一个添加好友申请
    _STRU_TCP_ADD_FRIEND_RQ rq;
    rq.fromId = m_TCPid;
    utf8ToGb2312(m_name,rq.fromName,sizeof(rq.fromName));
    utf8ToGb2312(name,rq.toName,sizeof(rq.toName));
    setNetStatus(m_pTCPMediator->SendData((char*)&rq,sizeof(rq),0));

    qDebug()<<m_name<<" 向 "<<name<<" 发送了好友申请";
}

//编码转换(QT---utf-8 VS数据库---gb2312)
//UTF-8转GB2312
void CKernel::utf8ToGb2312(QString src,char*res ,int nRes)
{
    QTextCodec* dc = QTextCodec::codecForName("gb2312");
    QByteArray ba = dc->fromUnicode(src);
    if(ba.length() <= nRes)
        memcpy(res,ba.data(),ba.length());
    else
    {
        qDebug()<<"CKernel::utf8ToGb2312 Src is too long";
    }
}

//GB2312转UTF-8
QString CKernel::gb2312ToUtf8(char* src)
{
    QTextCodec* dc = QTextCodec::codecForName("gb2312");
    return dc->toUnicode(src).toStdString().c_str(); //转换为Unicode的UTF-8格式
}

//断开连接时显示提示窗口
void CKernel::ShowNoConnected()
{
    while(true)
    {
        if(m_bReConnect)
        {
            m_ploginDialog->setWindowTitle(QString("网络错误，QQ重连中"));
            //QMessageBox::about(m_ploginDialog,"QQ提示","网络打开失败，重连中......");
        }
        Sleep(5000);
    }
}

//处理添加好友的信号
void CKernel::slot_addFriend()
{
    // 1 弹出窗口提示用户输入好友信息(name)
    QString name = QInputDialog::getText(m_pImDlg,"搜索好友","请输入好友昵称");
    QString name_temp = name;
    // 2 校验输入的数据是否合法
    //判断是否是空字符串 或者是全空格
    if(name.isEmpty()||name_temp.remove(" ").isEmpty())
    {
        QMessageBox::about(m_pImDlg,"错误提示","输入为空白，请重试");
        return;
    }
    //检查长度是否合法（name <= 20,tel = 11,password <= 20）
    if(name.length() > 20)
    {
        QMessageBox::about(m_pImDlg,"错误提示","输入长度有误，请重试");
        return;
    }
    //检查内容是否合法(tel为数字 name password为大小写字母 数字 下划线的组合)
    if(0)
    {
        QMessageBox::about(m_pImDlg,"错误提示","输入格式有误，请重试");
        return;
    }
    // 3 判断名字是不是自己的
    if(name == m_name)
    {
        QMessageBox::about(m_pImDlg,"错误提示","请勿输入本机昵称");
        return;
    }
    // 4 判断是否已经是好友了
    for(auto ite = m_mapTCPIdToUseritem.begin()
         ;ite != m_mapTCPIdToUseritem.end();ite++)
    {
        if(ite.value() && ite.value()->m_name == name)
        {
            QMessageBox::about(m_pImDlg,"系统提示",QString("【%1】已经是你的好友啦").arg(name));
            return;
        }
    }
    // 5 向服务器发送一个添加好友申请
    _STRU_TCP_ADD_FRIEND_RQ rq;
    rq.fromId = m_TCPid;
    utf8ToGb2312(m_name,rq.fromName,sizeof(rq.fromName));
    utf8ToGb2312(name,rq.toName,sizeof(rq.toName));
    setNetStatus(m_pTCPMediator->SendData((char*)&rq,sizeof(rq),0));

    qDebug()<<m_name<<" 向 "<<name<<" 发送了好友申请";
}

//处理删除好友的信号
void CKernel::slot_deleteFriend()
{
    // 1 弹出窗口提示用户输入好友信息(name)
    QString name = QInputDialog::getText(m_pImDlg,"删除好友","请输入好友昵称");
    QString name_temp = name;
    // 2 校验输入的数据是否合法
    //判断是否是空字符串 或者是全空格
    if(name.isEmpty()||name_temp.remove(" ").isEmpty())
    {
        QMessageBox::about(m_pImDlg,"错误提示","输入为空白，请重试");
        return;
    }
    //检查长度是否合法（name <= 20,tel = 11,password <= 20）
    if(name.length() > 20)
    {
        QMessageBox::about(m_pImDlg,"错误提示","输入长度有误，请重试");
        return;
    }
    //检查内容是否合法(tel为数字 name password为大小写字母 数字 下划线的组合)
    if(0)
    {
        QMessageBox::about(m_pImDlg,"错误提示","输入格式有误，请重试");
        return;
    }
    // 3 判断名字是不是自己的
    if(name == m_name)
    {
        QMessageBox::about(m_pImDlg,"错误提示","请勿输入本机昵称");
        return;
    }
    // 4 判断是否已经是好友了
    for(auto ite = m_mapTCPIdToUseritem.begin()
         ;;)
    {
        if(ite == m_mapTCPIdToUseritem.end())
        {
            QMessageBox::about(m_pImDlg,"错误提示","没有这个好友哦");
            return;
        }
        if(ite.value() && ite.value()->m_name == name)
        {

            break;
        }
        ite++;
    }
    // 5 向服务器发送一个删除好友申请
    _STRU_TCP_DELETE_FRIEND_RQ rq;
    rq.userId = m_TCPid;
    utf8ToGb2312(name,rq.friendName,sizeof(rq.friendName));
    setNetStatus(m_pTCPMediator->SendData((char*)&rq,sizeof(rq),0));

    qDebug()<<m_name<<" 向 "<<name<<" 发送了删除申请";
}

//处理系统设置的信号
void CKernel::slot_systemSet()
{

}

//处理发起群聊的信号
void CKernel::slot_createGroup()
{
    //获取群聊的名字 群号码

    //检查群聊的名字 群号码 和已经加入的群是否冲突

    //不冲突 发送一个建群申请给服务器
}

//处理加入群聊的信号
void CKernel::slot_addGroup()
{
    //获取群聊的名字 群号码

    //检查群聊的名字 群号码 和已经加入的群是否冲突

    //不冲突 发送一个入群申请给服务器
}

//处理发送给好友文件的信号
void CKernel::slot_sendFriendFile(int fid)
{
    char fileId[100];
    //文件发给服务器
    m_pTCPMediator->m_pNet->SendFile(fileId);

    //发送文件转发请求 由服务器转发
    _STRU_TCP_CLIENT_SEND_FILE_RQ rq;
    strcpy(rq.szFileId,fileId);
    rq.id = fid;
    rq.toFriOrGop = file_friend;
    rq.userId = m_TCPid;

    m_pTCPMediator->SendData((char*)&rq,sizeof(rq),0);
}

//处理发送给群聊文件的信号
void CKernel::slot_sendGroupFile(int gid)
{
    char fileId[100];
    //文件发给服务器
    m_pTCPMediator->m_pNet->SendFile(fileId);

    //发送文件转发请求 由服务器转发
    _STRU_TCP_CLIENT_SEND_FILE_RQ rq;
    strcpy(rq.szFileId,fileId);
    rq.id = gid;
    rq.toFriOrGop = file_group;
    rq.userId = m_TCPid;

    m_pTCPMediator->SendData((char*)&rq,sizeof(rq),0);
}

//设置APP在线 离线状态
void CKernel::setNetStatus(int sendNum)
{
    if(sendNum <= 0)
    {
        m_pImDlg->m_bNoConnect = true;
        auto ite = m_mapTCPIdToUseritem.begin();
        while(ite != m_mapTCPIdToUseritem.end())
        {
            ite.value()->m_status = status_offline;
            ite.value()->setFriendInfo(ite.value()->m_iconid,ite.value()->m_name
                                     ,ite.value()->m_feeling,ite.value()->m_status,ite.value()->m_id);
            ite++;
        }
        auto ite2 = m_mapGroupIdToGroupitem.begin();
        while(ite2 != m_mapGroupIdToGroupitem.end())
        {
            GroupChatDialog* chat = m_mapGroupIdToChatdlg[ite2.key()];
            auto ite3 = chat->m_mapMemberIdToMemberItem.begin();
            while(ite3 != chat->m_mapMemberIdToMemberItem.end())
            {
                ite3.value()->m_status = status_offline;
                ite3.value()->setGroupMemberInfo(ite3.value()->m_iconid,ite3.value()->m_name
                                           ,ite3.value()->m_status,ite3.value()->m_id);
                ite3++;
            }
            ite2.value()->m_status = status_offline;
            ite2.value()->setGroupInfo(ite2.value()->m_iconid,ite2.value()->m_name
                                       ,ite2.value()->m_status,ite2.value()->m_id);
            ite2++;
        }
        m_pImDlg->setUserInfo(m_iconId,m_name,m_feeling,m_socket);
        QMessageBox::about(m_pImDlg,"断网提示","TCP无网络连接,重连中...");
    }
    else
    {
        m_pImDlg->m_bNoConnect = false;
        m_pImDlg->setUserInfo(m_iconId,m_name,m_feeling,m_socket);
    }

}



















