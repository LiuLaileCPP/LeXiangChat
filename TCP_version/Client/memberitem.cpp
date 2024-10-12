#include "memberitem.h"
#include "ui_memberitem.h"

MemberItem::MemberItem(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MemberItem)
{
    ui->setupUi(this);
}

MemberItem::~MemberItem()
{
    delete ui;
}

//设置群成员的信息
void MemberItem::setGroupMemberInfo(int iconid,QString name
                             ,int status,int id)
{
    //保存群聊信息
    if(iconid != -1) m_iconid = iconid;
    m_name = name;
    m_status = status;
    m_id = id;
    //把群聊信息显示在控件上
    ui->lb_mName->setText(m_name);
    ui->lb_mId->setText(QString::number(m_id));
    QString path = QString(":/res/tx/%1.png").arg(m_iconid);
    //判断群聊状态 在线---亮显 离线---暗显
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

//和群成员聊天
void MemberItem::on_pb_chat_clicked()
{
    //发信号给groupchatdialog
    Q_EMIT SIG_memberChat(m_id,m_name);
}


//添加群成员为好友
void MemberItem::on_pb_add_clicked()
{
    //发信号给groupchatdialog
    Q_EMIT SIG_memberAdd (m_id,m_name);
}

