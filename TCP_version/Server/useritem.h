#ifndef USERITEM_H
#define USERITEM_H

#include <QWidget>
#include<QString>

namespace Ui {
class UserItem;
}

class UserItem : public QWidget
{
    Q_OBJECT
signals:
    void SIG_showChatDialog(QString ip);

public:
    explicit UserItem(QWidget *parent = nullptr);
    ~UserItem();
    const QString& getM_feeling();

    //设置好友的信息
    void setFriendInfo(int iconid,QString name,QString feeling);
private slots:
    void on_pb_icon_clicked();

private:
    Ui::UserItem *ui;
    int m_iconid;
    QString m_name;
    QString m_feeling;
};

#endif // USERITEM_H
