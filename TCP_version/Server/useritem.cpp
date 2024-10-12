#include "useritem.h"
#include "ui_useritem.h"

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

void UserItem::setFriendInfo(int iconid,QString name,QString feeling)
{
     //保存好友信息
    m_iconid = iconid;
    m_name = name;
    m_feeling = feeling;
     //把好友信息显示在控件上

    ui->lb_name->setText(m_name);
    ui->lb_feeling->setText(m_feeling);
    QString path = QString(":/res/head.jpg");
    ui->pb_icon->setIcon(QIcon(path));
 }

void UserItem::on_pb_icon_clicked()
{
    //给CKernel发消息 显示与当前好友的连天窗口
    Q_EMIT SIG_showChatDialog(m_feeling);
}

const QString& UserItem::getM_feeling()
{
    return m_feeling;
 }
