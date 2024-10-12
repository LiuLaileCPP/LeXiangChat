#include "kernel.h"
#include "ui_kernel.h"
#include <iostream>
#include <QThread>
#include <QBasicTimer>
using namespace std;

kernel* kernel::m_pKernel = 0;
kernel::kernel(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::kernel)
{
    m_pKernel = this;
    m_id = m_no = m_iconId = -1;
    password = telOrEmail = "";
    m_pUser = new person;
    m_pMainDlg = new MainDlg;
    m_pReglogDlg = new ReglogDlg;

    //绑定信号槽---------------------------------------------------------------------------------------------------------
    connect(this,SIGNAL(SIG_regist(bool,QString)),m_pReglogDlg,SLOT(slot_regist(bool,QString)));
    connect(this,SIGNAL(SIG_login(bool,QString)),m_pReglogDlg,SLOT(slot_login(bool,QString)));
    connect(m_pReglogDlg,SIGNAL(SIG_loginCommit(QString,QString,int)),this,SLOT(slot_login(QString,QString,int)));
    connect(m_pReglogDlg,SIGNAL(SIG_registerCommit(QString,QString,int)),this,SLOT(slot_regist(QString,QString,int)));
    connect(this,SIGNAL(SIG_itemInfo(char*)),this,SLOT(slot_itemInfo(char*)),Qt::BlockingQueuedConnection);
    connect(this,&kernel::SIG_about,this,[=](QString title,QString drict){
            QMessageBox::about(0,title,drict);
        },Qt::BlockingQueuedConnection);

    connect(this,&kernel::SIG_setNewMsg,this,[=](QString msg,person* fromWho,int type,int id){
            if(type == 0)//好友信息
            {
                m_mapIdToFriDlgs[id]->setNewMsg(msg,fromWho,type,id);
            }
            else if(type == 1)//群聊信息
            {
                m_mapIdToGroDlgs[id]->setNewMsg(msg,fromWho,type,id);
            }
        }
        ,Qt::BlockingQueuedConnection);
    //-------------------------------------------------------------------------------------------------------------------

    //ui->setupUi(this);
    memset(m_arrTypeToFun, 0, _DEF_WORK_TYPE_SIZE_ * 4);
    setArrayTypeToFun();
    startService();

    m_pReglogDlg->show();
    qDebug()<<"KERNEL tid: "<<QThread::currentThreadId();
}

kernel::~kernel()
{
    closeService();
    delete ui;
    delete m_pUdp;
    delete m_pUser;
    delete m_pMainDlg;
    delete m_pReglogDlg;
    m_pKernel = 0;

    auto ite = m_mapIdToFriDlgs.begin();
    while(ite != m_mapIdToFriDlgs.end())
    {
        delete ite->second;
        ite++;
    }
    ite = m_mapIdToGroDlgs.begin();
    while(ite != m_mapIdToGroDlgs.end())
    {
        delete ite->second;
        ite++;
    }
    auto ite2 = m_mapIdToFriItems.begin();
    while(ite2 != m_mapIdToFriItems.end())
    {
        delete ite2->second;
        ite2++;
    }
    ite2 = m_mapIdToGroItems.begin();
    while(ite2 != m_mapIdToGroItems.end())
    {
        delete ite2->second;
        ite2++;
    }
    auto ite3 = m_mapIdToFriends.begin();
    while(ite3 != m_mapIdToFriends.end())
    {
        delete ite3->second;
        ite3++;
    }
    ite3 = m_mapIdToGroMembs.begin();
    while(ite3 != m_mapIdToGroMembs.end())
    {
        delete ite3->second;
        ite3++;
    }
    auto ite4 = m_mapIdToGroups.begin();
    while(ite4 != m_mapIdToGroups.end())
    {
        delete ite4->second;
        ite4++;
    }
}


//打开服务器
bool kernel::startService()
{
    // QThread workerThread;
    // QBasicTimer timer;
    // timer.start(1000, this);
    m_pUdp = new udpnet;
    if (!m_pUdp)
        return false;
    if (m_pUdp->InitNet())
    {
        cout << "kernel:: startServer succeed" << endl;
        return true;
    }
    else
    {
        cout << "kernel:: startServer failed" << endl;
        return false;
    }
    return false;
}

//关闭服务器
void kernel::closeService()
{
    if (!m_pUdp)
        return;
    if (m_pUdp->CloseNet())
    {
        cout << "kernel:: closeServer succeed" << endl;
    }
    else
    {
        cout << "kernel:: closeServer failed" << endl;
    }
    return;
}




//槽函数--------------------------------------------------------------------------------------------------
void kernel::slot_regist(QString account,QString password,int typeRegist)
{
    //注册类型0是手机号注册，1是邮箱注册
    if(typeRegist == 0)
    {
        sendBuf msg;
        msg.no = m_no;
        msg.data1 = 0;//0是手机号注册，1是邮箱注册
        msg.type = _DEF_REG_RQ_;
        Tools::CodeConv_utf8ToGb2312(account, msg.buf1, sizeof(msg.buf1));
        Tools::CodeConv_utf8ToGb2312(password, msg.buf2, sizeof(msg.buf2));
        m_pUdp->SendData((char*)&msg,sizeof(msg));
    }
    else if(typeRegist == 1)
    {
        sendBuf msg;
        msg.no = m_no;
        msg.data1 = 1;//0是手机号注册，1是邮箱注册
        msg.type = _DEF_REG_RQ_;
        Tools::CodeConv_utf8ToGb2312(account, msg.buf1, sizeof(msg.buf1));
        Tools::CodeConv_utf8ToGb2312(password, msg.buf2, sizeof(msg.buf2));
        m_pUdp->SendData((char*)&msg,sizeof(msg));
    }
}

void kernel::slot_login(QString account,QString password,int typeRegist)
{
    qDebug()<<"LOGIN:: account"<<account<<" pwd"<<password;
    //登录类型0是手机号登录，1是邮箱登录
    if(typeRegist == 0)
    {
        sendBuf msg;
        msg.no = m_no;
        msg.data1 = 0;//0是手机号登录，1是邮箱登录
        msg.type = _DEF_LOG_RQ_;
        Tools::CodeConv_utf8ToGb2312(account, msg.buf1, sizeof(msg.buf1));
        Tools::CodeConv_utf8ToGb2312(password, msg.buf2, sizeof(msg.buf2));
        m_pUdp->SendData((char*)&msg,sizeof(msg));
    }
    else if(typeRegist == 1)
    {
        sendBuf msg;
        msg.no = m_no;
        msg.data1 = 1;//0是手机号登录，1是邮箱登录
        msg.type = _DEF_LOG_RQ_;
        Tools::CodeConv_utf8ToGb2312(account, msg.buf1, sizeof(msg.buf1));
        Tools::CodeConv_utf8ToGb2312(password, msg.buf2, sizeof(msg.buf2));
        m_pUdp->SendData((char*)&msg,sizeof(msg));
    }
}

//处理发送信息
void kernel::slot_sendMsg(int dlgType,int dlgId,QString content)
{
    if(dlgType == 1) //发送群消息
    {
        groMsg msg;
        msg.type = _DEF_CHAT_GRO_MSG_;
        msg.fromId = m_id;
        msg.toId = dlgId;
        msg.gId = dlgId;
        strcpy(msg.fromName,m_pUser->m_name.toStdString().c_str());
        strcpy(msg.buf,content.toStdString().c_str());

        m_pUdp->SendData((char*)&msg,sizeof(msg));
        qDebug()<<"kernel:: send GroMsg, toId: "<<dlgId<<" msg:"<<content;
    }
    else if(dlgType == 0) //发送好友信息
    {
        friMsg msg;
        msg.type = _DEF_CHAT_MSG_;
        msg.fromId = m_id;
        msg.toId = dlgId;
        strcpy(msg.fromName,m_pUser->m_name.toStdString().c_str());
        strcpy(msg.buf,content.toStdString().c_str());

        m_pUdp->SendData((char*)&msg,sizeof(msg));
        qDebug()<<"kernel:: send FriMsg, toId: "<<dlgId<<" msg:"<<content;
    }
}

//处理朋友 群聊 群成员item信息
void kernel::slot_itemInfo(char* recvData)
{
    qDebug()<<"DEAL item SLOT tid: "<<QThread::currentThreadId();
    recvBuf info;
    memcpy(&info,recvData,sizeof(recvBuf));
    int type = *(int*)&info;
    qDebug()<<"ItemInfo type: "<<type;
    strcpy(info.buf1,Tools::CodeConv_gb2312ToUtf8(info.buf1).toStdString().c_str());
    strcpy(info.buf2,Tools::CodeConv_gb2312ToUtf8(info.buf2).toStdString().c_str());
    strcpy(info.buf3,Tools::CodeConv_gb2312ToUtf8(info.buf3).toStdString().c_str());
    strcpy(info.buf4,Tools::CodeConv_gb2312ToUtf8(info.buf4).toStdString().c_str());
    //发信号 0好友信息 1群聊信息 2群成员信息
    if(type == _DEF_FRI_INFO_)
    {
        if(info.data1 == m_id)
            return;
        person* pon = 0;
        if(m_mapIdToFriends.count(info.data1) > 0)
        {
            pon = m_mapIdToFriends[info.data1];
            pon->initInfo(info.res1,info.data1,info.data2,info.buf3,"",info.buf1,info.buf2,"",info.buf4);

            if(m_mapIdToFriDlgs.count(pon->m_id) > 0)
            {
                m_mapIdToFriDlgs[pon->m_id]->setInfo(0,pon->m_state,pon->m_id,pon->m_iconId,pon->m_name);
            }
            if(m_mapIdToFriItems.count(pon->m_id) > 0)
            {
                m_mapIdToFriItems[pon->m_id]->setInfo(pon->m_state,pon->m_iconId,pon->m_name);
                m_mapIdToFriItems[pon->m_id]->show();
            }
        }
        else
        {
            pon = new person;            //信息存在mapIdToFriItem中
            pon->initInfo(info.res1,info.data1,info.data2,info.buf3,"",info.buf1,info.buf2,"",info.buf4);
            item* item = new class item;
            item->setInfo(pon->m_state,pon->m_iconId,pon->m_name);
            chatdlg* dlg = new class chatdlg;
            dlg->setInfo(0,pon->m_state,pon->m_id,pon->m_iconId,pon->m_name);

            connect(item,SIGNAL(SIG_clicked()),dlg,SLOT(slot_show()));//绑定点击显示聊天dlg
            connect(dlg,SIGNAL(SIG_sendMsg(int,int,QString)),this,SLOT(slot_sendMsg(int,int,QString)));
            //connect(this,SIGNAL(SIG_setNewMsg(QString,person*,int,int)),dlg,SLOT(setNewMsg(QString,person*,int,int)),Qt::BlockingQueuedConnection);
            m_mapIdToFriends[pon->m_id] = pon;
            m_mapIdToFriItems[pon->m_id] = item;
            m_mapIdToFriDlgs[pon->m_id] = dlg;
            m_pMainDlg->addItem(item);
            item->show();
            m_pMainDlg->addDlg(dlg);
            dlg->hide();
        }
    }
    else if(type == _DEF_GRO_INFO_)
    {
        QString name = info.buf2;
        name += QString("（%1）").arg(info.data3);
        group* pon = 0;
        if(m_mapIdToGroups.count(info.data1) > 0)
        {
            pon = m_mapIdToGroups[info.data1];
            //group::initInfo(int id,int iconId,int size,QString name,QString num,QString sign)
            pon->initInfo(info.data1,info.data2,info.data3,info.buf2,info.buf1,info.buf3);
            if(m_mapIdToGroDlgs.count(pon->m_gId) > 0)
            {
                m_mapIdToGroDlgs[pon->m_gId]->setInfo(1,1,pon->m_gId,pon->m_iconId,name); //群聊的状态默认在线
            }
            if(m_mapIdToGroItems.count(pon->m_gId) > 0)
            {
                m_mapIdToGroItems[pon->m_gId]->setInfo(1,pon->m_iconId,name);
                m_mapIdToGroItems[pon->m_gId]->show();
            }
        }
        else
        {
            pon = new group;            //信息存在mapIdToGroItem中
            pon->initInfo(info.data1,info.data2,info.data3,info.buf2,info.buf1,info.buf3);
            item* item = new class item;
            item->setInfo(1,pon->m_iconId,name);
            chatdlg* dlg = new class chatdlg;
            dlg->setInfo(1,1,pon->m_gId,pon->m_iconId,name);

            connect(item,SIGNAL(SIG_clicked()),dlg,SLOT(slot_show()));
            connect(dlg,SIGNAL(SIG_sendMsg(int,int,QString)),this,SLOT(slot_sendMsg(int,int,QString)));
            //connect(this,SIGNAL(SIG_setNewMsg(QString,person*,int,int)),dlg,SLOT(setNewMsg(QString,person*,int,int)),Qt::BlockingQueuedConnection);

            m_mapIdToGroups[pon->m_gId] = pon;
            m_mapIdToGroItems[pon->m_gId] = item;
            m_mapIdToGroDlgs[pon->m_gId] = dlg;

            m_pMainDlg->addItem(item);
            item->show();
            m_pMainDlg->addDlg(dlg);
            dlg->hide();
        }
    }
    else if(type == _DEF_GROMEM_INFO_)
    {
        //信息存在maoIdToGroMembs的map表中
        if(m_mapIdToGroMembs.count(info.data1) > 0)
        {
            //initInfo(bool state,int id,int iconId,
            //      QString name,QString account,QString tel,
            //      QString email,QString password,QString sign);
            m_mapIdToGroMembs[info.data1]->initInfo(info.res1,info.data1,info.data2,info.buf3,"",info.buf1,info.buf2,"",info.buf4);
        }
        else
        {
            person* memb = new person;
            memb->initInfo(info.res1,info.data1,info.data2,info.buf3,"",info.buf1,info.buf2,"",info.buf4);
            m_mapIdToGroMembs[info.data1] = memb;
        }
    }
    else
    {
        qDebug()<<"kernel::dealItemInfo TYPE WRONG";
        return;
    }
}


//---------------------------------------------------------------------------------------------------








//核心工作--------------------------------------------------------------------------------------------
void kernel::nuclearWork(char* recvData)
{
    cout << "kernel:: 核心处理..." << endl;
    cout << "kernel:: nuclearWork type:"<<((recvBuf*)recvData)->type<<" MSG: " << ((recvBuf*)recvData)->buf1 <<" "<<((recvBuf*)recvData)->buf2
         <<" "<<((recvBuf*)recvData)->buf3<<" "<<((recvBuf*)recvData)->buf4<< endl;
    int type = *(int*)recvData;
    if (type <= 0 || type > _DEF_PROTO_SIZE_)
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
    (this->*p)(recvData);
}

//第一次连接时服务器发来自己的地址编号 存下来
void kernel::setClientNo(char* recvData)
{
    recvBuf* rs = (recvBuf*)recvData;
    m_no = rs->no;
    qDebug()<<rs->buf1;
}

//处理注册回复
void kernel::dealRegRs(char* recvData)
{
    cout << "kernel:: dealRegRs msg: " << ((recvBuf*)recvData)->buf1 << endl;
    recvBuf* rs = (recvBuf*)recvData;
    //res1是注册结果 data1存的id buf1存的服务器信息
    QString msg = Tools::CodeConv_gb2312ToUtf8(rs->buf1);
    //给注册窗口发信号
    Q_EMIT SIG_regist(rs->res1,msg);
}

//处理登录回复,接收消息：能否登录 能的话回复：好友信息 群聊信息 群聊成员信息
void kernel::dealLogRs(char* recvData)
{
    cout << "kernel:: dealLogRs msg: " << ((recvBuf*)recvData)->buf1 << endl;
    recvBuf* rs = (recvBuf*)recvData;
    recvBuf msg = *rs;
    //res1是登录结果 data1存的id buf1存的服务器信息
    if(rs->res1 == true)
    {
        //登录成功 给服务器发消息 记录自己的addr
        recvBuf info;
        info.type = TYPE_RECORD_ADDR;   //protocol
        info.data1 = rs->data1;         //id
        info.no = m_no;
        m_pUdp->SendData((char*)&info,sizeof(recvBuf));
    }
    strcpy(msg.buf1,Tools::CodeConv_gb2312ToUtf8(rs->buf1).toStdString().c_str());
    strcpy(msg.buf2,Tools::CodeConv_gb2312ToUtf8(rs->buf2).toStdString().c_str());
    strcpy(msg.buf3,Tools::CodeConv_gb2312ToUtf8(rs->buf3).toStdString().c_str());
    strcpy(msg.buf4,Tools::CodeConv_gb2312ToUtf8(rs->buf4).toStdString().c_str());
    //给注册窗口发信号
    Q_EMIT SIG_login(rs->res1,msg.buf2);

    m_id = rs->data1;
    m_iconId = rs->data2;
    //initInfo(bool state,int id,int iconId,QString name,QString account,
    //QString tel,QString email,QString password,QString sign
    m_pUser->initInfo(rs->res1,rs->data1,rs->data2,msg.buf3,"",telOrEmail,"",password,msg.buf4);
    m_pMainDlg->setInfo(rs->data1,rs->data2,msg.buf3,msg.buf4);
}

//处理服务器发来的item信息：好友 群聊 群聊成员
void kernel::dealItemInfo(char* recvData)
{
    qDebug()<<"DEAL item SIG tid: "<<QThread::currentThreadId();
    recvBuf msg;
    memcpy(&msg,recvData,sizeof(recvBuf));
    emit SIG_itemInfo((char*)&msg);
}


//处理聊天 转发聊天消息
void kernel::dealChatMsg(char* recvData)
{
    friMsg* msg = (friMsg*)recvData;
    if(msg->fromId == m_id)
    {
        QString err = Tools::CodeConv_gb2312ToUtf8(msg->buf);
        emit SIG_about("发送失败",err);
        return;
    }
    int i = 0;
    if(m_mapIdToFriends.count(msg->fromId) <= 0)
    {
        qDebug()<<"kernel::dealChatMsg 找不到这个好友的本地person对象";
        i++;
    }
    if(m_mapIdToFriItems.count(msg->fromId) <= 0)
    {
        qDebug()<<"kernel::dealChatMsg 找不到这个好友的本地item对象";
        i++;
    }
    if(m_mapIdToFriDlgs.count(msg->fromId) <= 0)
    {
        qDebug()<<"kernel::dealChatMsg 找不到这个好友的本地dlg对象";
        i++;
    }
    if(i > 0)
        return;
    person* p = m_mapIdToFriends[msg->fromId];
    emit SIG_setNewMsg(msg->buf,p,0,p->m_id);
    m_mapIdToFriItems[msg->fromId]->setClrName(2);
    qDebug()<<"kernel:: recv and set new msg: "<<msg->buf;
}

//转发群聊信息
void kernel::dealChatGroMsg(char* recvData)
{
    groMsg* msg = (groMsg*)recvData;
    if(msg->fromId == m_id)
    {
        QString err = Tools::CodeConv_gb2312ToUtf8(msg->buf);
        emit SIG_about("发送失败",err);
        return;
    }
    int i = 0;
    if(m_mapIdToGroups.count(msg->toId) <= 0)
    {
        qDebug()<<"kernel::dealChatMsg 找不到这个群聊的本地group对象";
        i++;
    }
    if(m_mapIdToGroMembs.count(msg->fromId) <= 0)
    {
        qDebug()<<"kernel::dealChatMsg 找不到这个群聊成员的本地person对象";
        i++;
    }
    if(m_mapIdToGroItems.count(msg->fromId) <= 0)
    {
        qDebug()<<"kernel::dealChatMsg 找不到这个群聊的本地item对象";
        i++;
    }
    if(m_mapIdToGroDlgs.count(msg->fromId) <= 0)
    {
        qDebug()<<"kernel::dealChatMsg 找不到这个群聊的本地dlg对象";
        i++;
    }
    if(i > 0)
        return;
    person* p = m_mapIdToGroMembs[msg->fromId];
    m_mapIdToGroItems[msg->toId]->setClrName(2);
    emit SIG_setNewMsg(msg->buf,p,1,msg->gId);
    qDebug()<<"kernel:: recv and set new msg: "<<msg->buf;
}

//处理添加好友请求
void kernel::dealAddFriendRq(char* recvData)
{

}

//服务器对你添加好友的回复
void kernel::dealAddFriendRs(char* recvData)
{

}

//服务器对你删除好友的回复
void kernel::dealDeleteFriendRs(char* recvData)
{

}

//心跳机制验活
void kernel::checkAlive(char* recvData)
{
    udpnet* p = m_pUdp;
    while (1)
    {
        if (!p)
        {
            cout << "** thread_alive end, m_pUdp==nullptr" << endl;
            Sleep(10000);
            continue;
        }
        cout << "** thread_alive running..." << endl;
        if (!p->m_netState)
        {
            cout << "** thread_alive nonet..." << endl;
            p->InitNet();
            Sleep(1000);
            continue;
        }
        char live_buf[1024] = "alive";
        sendBuf buf;
        buf.type = TYPE_ALIVE;
        buf.data1 = m_id;
        strcpy_s(buf.buf1,live_buf);
        buf.no = m_no;
        qDebug()<<"*** Alive no:"<<m_no;
        p->SendData((char*)&buf,sizeof(buf));
        cout << "** thread_alive send succeed..." << endl;
        Sleep(10000);
    }
}
//----------------------------------------------------------------



//show keep runnig
void kernel::showRunning()
{

}

//初始化工作数组
void kernel::setArrayTypeToFun()
{
    m_arrTypeToFun[_DEF_REG_RS_] = &kernel::dealRegRs;
    m_arrTypeToFun[_DEF_LOG_RS_] = &kernel::dealLogRs;
    m_arrTypeToFun[_DEF_CHAT_MSG_] = &kernel::dealChatMsg;
    m_arrTypeToFun[_DEF_ADD_FRIEND_RQ_] = &kernel::dealAddFriendRq;
    m_arrTypeToFun[_DEF_ADD_FRIEND_RS_] = &kernel::dealAddFriendRs;
    m_arrTypeToFun[_DEF_DELETE_FRIEND_NOTICE_] = &kernel::dealDeleteFriendRs;
    m_arrTypeToFun[_DEF_FRI_INFO_] = &kernel::dealItemInfo;
    m_arrTypeToFun[_DEF_GRO_INFO_] = &kernel::dealItemInfo;
    m_arrTypeToFun[_DEF_GROMEM_INFO_] = &kernel::dealItemInfo;
    m_arrTypeToFun[_DEF_CLIENT_NO_] = &kernel::setClientNo;
    m_arrTypeToFun[_DEF_CHAT_MSG_] = &kernel::dealChatMsg;
    m_arrTypeToFun[_DEF_CHAT_GRO_MSG_] = &kernel::dealChatGroMsg;
    //m_arrTypeToFun[_DEF_REG_RQ_] = dealRegRq;
    //m_arrTypeToFun[_DEF_REG_RQ_] = dealRegRq;
    //m_arrTypeToFun[_DEF_REG_RQ_] = dealRegRq;
    //m_arrTypeToFun[_DEF_REG_RQ_] = dealRegRq;
    //m_arrTypeToFun[_DEF_REG_RQ_] = dealRegRq;
    //m_arrTypeToFun[_DEF_REG_RQ_] = dealRegRq;
    //m_arrTypeToFun[_DEF_REG_RQ_] = dealRegRq;
    //m_arrTypeToFun[_DEF_REG_RQ_] = dealRegRq;
}
