#include "useritem.h"
#include "ui_useritem.h"
#include<QBitmap>

UserItem::UserItem(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::UserItem)
{
    ui->setupUi(this);
}

UserItem::~UserItem()
{
    delete ui;
}

void UserItem::setFriendInfo(int iconid,QString name
                             ,QString feeling,int status,int id)
{
     //保存好友信息
    m_iconid = iconid;
    m_name = name;
    m_feeling = feeling;
    m_status = status;
    m_id = id;
    //把好友信息显示在控件上
    ui->lb_name->setText(m_name);
    ui->lb_feeling->setText(m_feeling);
    QString path = QString(":/res/tx/%1.png").arg(iconid);
    //判断好友状态 在线---亮显 离线---暗显
    if(m_status == status_online)
    {

        ui->pb_icon->setIcon(QIcon(path));
    }
    else
    {
        QBitmap bp;
        bp.load(path);
        ui->pb_icon->setIcon(bp);//设置为黑白灰显示
    }
 }



 //点击一下头像进入到这个槽函数
 //此函数发送一个信号 让kernel显示此人的聊天窗口
void UserItem::on_pb_icon_clicked()
{
    //给CKernel发消息 显示与当前好友的连天窗口
    if(m_id == -1)
    {
        Q_EMIT SIG_showChatDialog(m_feeling);
    }
    else
    {
        Q_EMIT SIG_showChatDialog(m_feeling,m_id);
    }
}

const QString& UserItem::getM_feeling()
{
    return m_feeling;
 }
