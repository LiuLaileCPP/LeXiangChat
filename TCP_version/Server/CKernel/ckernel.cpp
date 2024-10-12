#include "ckernel.h"
#include"UdpNetMediator.h"
#include<QMessageBox>
#include<QDebug>
#include"INet.h"
#include"dataDef.h"
#include"useritem.h"
//信号和槽:
//1 两个类必须直接或间接继承QObject 并且要有Q_OBJECT关键字
//2 在发送数据的类里面定义信号 signals 非函数不要实现 只需要在发送数据的地方Q_EMIT 信号
//3 在接收数据的类里面声明并实现槽函数 slots
//4 在接收数据的类里面绑定，在发送信号的对象new出来以后，绑定信号和槽 connect()

CKernel::CKernel(QObject *parent)
    : QObject{parent}
{
    m_pImDlg = new IMS_Dialog;
    m_pImDlg->show();

    //绑定关闭窗口的信号和槽函数
    connect(m_pImDlg,SIGNAL(SIG_close()),this,SLOT(slot_close()));

    //创建中介类的对象
    m_pMediator = new UdpNetMediator;

    //用中介者的成员---网络功能类对象打开网络
    if(!m_pMediator->OpenNet())
    {
        //打开失败 提示用户
        QMessageBox::about(m_pImDlg,"提示","网络打开失败，请重试");
        exit(0);//退出主进程
    }

    //绑定中介者发送的数据的信号，槽
    connect(m_pMediator,SIGNAL(SIG_ReadyData( char*,int,long))
            ,this,SLOT(slot_ReadyData( char*,int,long)));


    //测试：给自己发个hello
    //char s[4096] = "hello";

    struct _STRU_ONLINE rq;
    gethostname(rq.name,sizeof(rq));//获取当前主机名字
    m_pMediator->SendData((char*)&rq,sizeof(rq),inet_addr("192.168.14.215"));


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

}

//槽函数 接收并处理中介者发送来的信息
void CKernel::slot_ReadyData(char* recvData,int nLen,long lFrom)
{
    //cout<<"Open slot"<<endl;
    //qDebug()<<nLen<<" "<<"recv acc __func__"<<recvData;

    //取出协议头 走不同的流程
   PackType type =  *(PackType*)recvData; //转为int* 取出前四个字节的数据

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
   default:
       break;
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
    connect(item,SIGNAL(SIG_showChatDialog(QString))
            ,this,SLOT(slot_showChatDialog(QString)));




    //3，new 一个和这个人聊天的窗口
    ChatDialog* chat = new ChatDialog;
    m_mapIpToChatdlg[lFrom] = chat;//保存聊天窗口
    chat->setChatDialogInfo(inet_ntoa(addr.sin_addr));



    //绑定点击send发送按钮时的发送信息信号和槽函数
    connect(chat,SIGNAL( SIG_sendMsg(QString,QString))
            ,this,SLOT(slot_sendMsg(QString,QString)));




    // 4，判断是不是自己发的广播
    set<long> iplist = INet::GetIpList();
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
     connect(chat,SIGNAL( SIG_sendMsg(QString,QString))
             ,this,SLOT(slot_sendMsg(QString,QString)));
}





//处理聊天请求
void CKernel::dealChatRq(char* recvData,int nLen,long lFrom)
{
     qDebug()<<"CKernel :: dealChatRq";
    //拆包
     struct _STRU_CHAT_RQ* rq = (_STRU_CHAT_RQ*)recvData;


     //找到和这个人的聊天窗口
     if(m_mapIpToChatdlg.count() > 0)
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





void CKernel::slot_showChatDialog(QString ip)
{
    if(m_mapIpToChatdlg.count() > 0)
    {
        long l = inet_addr(ip.toStdString().c_str());
        ChatDialog* chat = m_mapIpToChatdlg[l];
        chat->showNormal();
    }
}
//char*是基础类型 可以给std::string QString 直接赋值
//std::string ---> char* :   str.c_str()
//QString ---> std::string : QStr.toStdString()





//chat_dialog 的发送按钮的槽函数
void CKernel::slot_sendMsg(QString content,QString  ip)
{
    //写一个聊天请求发给对端
    struct _STRU_CHAT_RQ rq;
    strcpy(rq.content,content.toStdString().c_str());

    //发给对端
    long lip = inet_addr(ip.toStdString().c_str());
    m_pMediator->SendData((char*)&rq,sizeof(rq),lip);
 }



//确认关闭应用程序
 void CKernel::slot_close()
 {
    //发送广播 通知大家我下线了
     struct _STRU_OFFLINE_RQ rq;
    m_pMediator->SendData((char*)&rq,sizeof(rq)
                           ,inet_addr("255.255.255.255"));


    //回收资源
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



    //退出进程
    exit(0);
}











