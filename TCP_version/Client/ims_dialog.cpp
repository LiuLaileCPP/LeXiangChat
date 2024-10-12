#include "ims_dialog.h"
#include "ui_ims_dialog.h"
#include<QString>
#include<QMessageBox>

IMS_Dialog::IMS_Dialog(QWidget *parent)
    : QDialog(parent)
    , m_bNoConnect(false)
    ,ui(new Ui::IMS_Dialog)
    ,m_pUDPlayout(new QVBoxLayout)
    ,m_pTCPlayout(new QVBoxLayout)
    ,m_pGrouplayout(new QVBoxLayout)
    ,m_pMenu(new QMenu)
{
    ui->setupUi(this);
    ui->pb_menu->setText("+");

    //绑定点击菜单栏添加好友
    connect(m_pMenu,SIGNAL(triggered(QAction*)),this,SLOT(slot_clickedMenuAddFriend(QAction*)));
    //设置窗口的信息---为本机的Ip
    // char hostname[256];
    // struct hostent *host;
    // struct in_addr **addr_list;

    // gethostname(hostname, sizeof(hostname));
    // host = gethostbyname(hostname);
    // addr_list = (struct in_addr **)host->h_addr_list;


    //设置小控件间的间隔
    m_pUDPlayout->setSpacing(3);
    m_pTCPlayout->setSpacing(3);
    m_pGrouplayout->setSpacing(3);


    //设置小控件和大控件的间隔
    m_pUDPlayout->setContentsMargins(0,3,0,3);
    m_pTCPlayout->setContentsMargins(0,3,0,3);
    m_pGrouplayout->setContentsMargins(0,3,0,3);

    //显示到大控件上
    ui->wdg_list->setLayout(m_pUDPlayout);
    ui->wdg_TCPlist->setLayout(m_pTCPlayout);
    ui->wdg_TCP_grouplist->setLayout(m_pGrouplayout);

    //设置显示最小化 最大化功能
    Qt::WindowFlags flags = Qt::Dialog;
    flags |= Qt::WindowMinMaxButtonsHint;
    flags |= Qt::WindowCloseButtonHint;
    setWindowFlags(flags);

    //menu添加菜单项
    m_pMenu->addAction("添加好友");
    m_pMenu->addAction("删除好友");
    m_pMenu->addAction("创建群聊");
    m_pMenu->addAction("加入群");
    m_pMenu->addAction("设置");

}

IMS_Dialog::~IMS_Dialog()
{
    delete ui;
    delete m_pUDPlayout;
    delete m_pTCPlayout;
    delete m_pGrouplayout;
}

//添加好友
void IMS_Dialog::addFriend(UserItem* item,int id)
{
    if(item == nullptr)
    {
        return;
    }
    if(id == -1)
    {
        m_pUDPlayout->addWidget(item);
    }
    else
    {
        m_pTCPlayout->addWidget(item);
    }


    //ui->wdg_list->layout()->addWidget(item);
}

//添加群聊
void IMS_Dialog::addGroup(GroupItem* item,int id)
{
    if(item == nullptr)
    {
        return;
    }
    if(id == -1)
    {
        //m_pUDPlayout->addWidget(item);
    }
    else
    {
        m_pGrouplayout->addWidget(item);
    }


    //ui->wdg_list->layout()->addWidget(item);
}

//删除好友
void IMS_Dialog::deleteFriend(QString ip)
{
    //遍历layout上的所有控件useritem
    int i = 0;
    QLayoutItem* child = nullptr;
    while(nullptr != (child = m_pUDPlayout->takeAt(i++)))
    {
        UserItem* item = (UserItem*)child->widget();
        if (item->getM_feeling() == ip)
        {
            //先将子类的parent赋空
            item->setParent(nullptr);
            delete child;
            break;
        }
    }

    i = 0;
    child = nullptr;
    while(nullptr != (child = m_pTCPlayout->takeAt(i++)))
    {
        UserItem* item = (UserItem*)child->widget();
        if (item->getM_feeling() == ip)
        {
            //先将子类的parent赋空
            item->setParent(nullptr);
            delete child;
            break;
        }
    }
}

//删除群聊
void IMS_Dialog::deleteGroup(QString ip,int id)
{
    //遍历layout上的所有控件groupitem
    int i = 0;
    QLayoutItem* child = nullptr;
    while(nullptr != (child = m_pGrouplayout->takeAt(i++)))
    {
        GroupItem* item = (GroupItem*)child->widget();
        if (item->m_id == id)
        {
            //先将子类的parent赋空
            item->setParent(nullptr);
            delete child;
            break;
        }
    }
}


//展示朋友
void IMS_Dialog::showFriend()
{

}


//设置用户信息到界面上
void IMS_Dialog::setUserInfo(int iconid,QString name,QString feeling,long sock)
{
    setWindowTitle(QString("本机socket【%1】").arg(sock));
    QString path = QString(":/res/tx/%1.png").arg(iconid);
    ui->lb_name->setText(name);
    ui->le_feeling->setText(feeling);
    if(m_bNoConnect)
    {
        QBitmap bp;
        bp.load(path);
        ui->pb_icon->setIcon(bp);//设置为黑白灰显示
    }
    else
    {
        ui->pb_icon->setIcon(QIcon(path));
    }

}

//重写关闭窗口事件
void IMS_Dialog::closeEvent(QCloseEvent* event)
{
    //忽略用户的操作 不走父类的关闭
    event->ignore();
    //询问用户是否要关闭程序
    if(QMessageBox::Yes == QMessageBox::question(this,"提示","要退出吗?"))
    {
        //确定关闭窗口
        Q_EMIT SIG_close();
    }
}


void IMS_Dialog::on_pb_menu_clicked()
{
    //在鼠标点击的位置 向上显示菜单栏
    QPoint/*坐标类*/ pos = QCursor/*鼠标*/::pos();

    //获取菜单栏的绝对大小
    QSize size = m_pMenu->sizeHint();
    const int btnPosX = ui->pb_menu->mapToGlobal(QPoint(0, 0)).x();
    //int btnPosY = snder->mapToGlobal(QPoint(0, 0)).y()
    m_pMenu->exec(QPoint(btnPosX,pos.y() - size.height()));
}

//点击菜单添加的槽函数
void IMS_Dialog::slot_clickedMenuAddFriend(QAction* paction)
{
    //判断点击的菜单项
    if("添加好友" == paction->text())
    {
        //给Kernel发送添加好友的信号
        Q_EMIT SIG_addFriend();
    }
    else if("设置" == paction->text())
    {
        Q_EMIT SIG_systemSet();
    }
    else if("删除好友" == paction->text())
    {
        //删除好头的信号 发给Kernel
        Q_EMIT SIG_deleteFriend();
    }
    else if("创建群聊" == paction->text())
    {
        //发起群聊的信号 发给Kernel
        Q_EMIT SIG_createGroup();
    }
    else if("加入群" == paction->text())
    {
        //加入群聊的信号 发给Kernel
        Q_EMIT SIG_addGroup();
    }
}











