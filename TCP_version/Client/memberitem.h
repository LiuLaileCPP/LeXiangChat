#ifndef MEMBERITEM_H
#define MEMBERITEM_H

#include <QWidget>
#include"dataDef.h"

namespace Ui {
class MemberItem;
}

class MemberItem : public QWidget
{
    Q_OBJECT

public:
    explicit MemberItem(QWidget *parent = nullptr);
    ~MemberItem();

signals:
    void SIG_memberChat(int id,QString name);
    void SIG_memberAdd (int id,QString name);
private slots:
    void on_pb_chat_clicked();

    void on_pb_add_clicked();

public:
    Ui::MemberItem *ui;
    int m_iconid;
    int m_id;
    int m_status;
    QString m_name;
    //设置群成员的信息
    void setGroupMemberInfo(int iconid,QString name,int status = status_online,int id = -1);

};

#endif // MEMBERITEM_H
