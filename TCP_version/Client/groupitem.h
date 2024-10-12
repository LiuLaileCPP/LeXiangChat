#ifndef GROUPITEM_H
#define GROUPITEM_H
#include"dataDef.h"
#include <QWidget>

namespace Ui {
class GroupItem;
}

class GroupItem : public QWidget
{
    Q_OBJECT
signals:
    void SIG_showGroupChatDialog(QString ip,int id = -1);


public:
    Ui::GroupItem *ui;
    int m_iconid;
    int m_id;
    int m_status;
    QString m_name;
    explicit GroupItem(QWidget *parent = nullptr);
    ~GroupItem();
    //设置群聊的信息
    void setGroupInfo(int iconid,QString name,int status = status_online,int id = -1);


private slots:
    //点击一下头像进入到这个槽函数
    //此函数发送一个信号 让kernel显示此人的聊天窗口
    void on_pb_icon_clicked();
};

#endif // GROUPITEM_H
