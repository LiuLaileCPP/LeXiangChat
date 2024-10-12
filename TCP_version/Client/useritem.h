#ifndef USERITEM_H
#define USERITEM_H

#include <QWidget>
#include<QString>
#include"dataDef.h"

namespace Ui {
class UserItem;
}

class UserItem : public QWidget
{
    Q_OBJECT
signals:
    void SIG_showChatDialog(QString ip,int id = -1);

public:
    explicit UserItem(QWidget *parent = nullptr);
    ~UserItem();
    const QString& getM_feeling();
    int m_status;

    //设置好友的信息
    void setFriendInfo(int iconid,QString name,QString feeling,int status = status_online,int id = -1);

private slots:
    void on_pb_icon_clicked();
    //点击一下头像进入到这个槽函数
    //此函数发送一个信号 让kernel显示此人的聊天窗口

public:
    Ui::UserItem *ui;
    int m_iconid;
    QString m_feeling;
    int m_id;
    QString m_name;
};

#endif // USERITEM_H
