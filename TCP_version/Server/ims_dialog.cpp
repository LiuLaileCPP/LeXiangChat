#include "ims_dialog.h"
#include "ui_ims_dialog.h"
#include<QString>
#include <Winsock2.h>
#include<QMessageBox>

IMS_Dialog::IMS_Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::IMS_Dialog)
    ,m_layout(new QVBoxLayout)
{
    ui->setupUi(this);

    //设置窗口的信息---为本机的Ip
    // char hostname[256];
    // struct hostent *host;
    // struct in_addr **addr_list;

    // gethostname(hostname, sizeof(hostname));
    // host = gethostbyname(hostname);
    // addr_list = (struct in_addr **)host->h_addr_list;
    setWindowTitle("本机ip【 】");

    //设置控件间的间隔
    m_layout->setSpacing(3);

    //设置小控件和大控件的间隔
    m_layout->setContentsMargins(0,3,0,3);

    //显示到大控件上
    ui->wdg_list->setLayout(m_layout);
    Qt::WindowFlags flags = Qt::Dialog;
    flags |= Qt::WindowMinMaxButtonsHint;
    flags |= Qt::WindowCloseButtonHint;
    setWindowFlags(flags);
}

IMS_Dialog::~IMS_Dialog()
{
    delete ui;
    delete m_layout;
}

//添加好友
void IMS_Dialog::addFriend(UserItem* item)
{
    m_layout->addWidget(item);
    //ui->wdg_list->layout()->addWidget(item);
}


//删除好友
void IMS_Dialog::deleteFriend(QString ip)
{
    //遍历layout上的所有控件useritem
    int i = 0;
    QLayoutItem* child = nullptr;
    while(nullptr != (child = m_layout->takeAt(i++)))
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


//展示朋友
void IMS_Dialog::showFriend()
{

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
