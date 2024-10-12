#include "groupitem.h"
#include "ui_groupitem.h"
#include<QBitmap>

GroupItem::GroupItem(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GroupItem)
{
    ui->setupUi(this);
}

GroupItem::~GroupItem()
{
    delete ui;
}

//点击一下群头像进入到这个槽函数
//此函数发送一个信号 让kernel显示此群的聊天窗口
void GroupItem::on_pb_icon_clicked()
{
    Q_EMIT SIG_showGroupChatDialog("",m_id);
}

//设置群聊的信息
void GroupItem::setGroupInfo(int iconid,QString name
                  ,int status,int id)
{
    //保存群聊信息
    if(iconid != -1) m_iconid = iconid;
    m_name = name;
    m_status = status;
    m_id = id;
    //把群聊信息显示在控件上
    ui->lb_gName->setText(m_name);
    ui->lb_gId->setText(QString::number(m_id));
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
