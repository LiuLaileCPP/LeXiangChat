#ifndef IMS_DIALOG_H
#define IMS_DIALOG_H

#include <QDialog>
#include<QCloseEvent>
#include<QVBoxLayout>
#include"./useritem.h"
#include"./groupitem.h"
#include<QMainWindow>
#include<QWidget>
#include<QAction>
#include<QMenu>
#include <Winsock2.h>
QT_BEGIN_NAMESPACE
namespace Ui {
class IMS_Dialog;
}
QT_END_NAMESPACE

class IMS_Dialog : public QDialog
{
    Q_OBJECT
signals:
    void SIG_close();
    //添加好友的信号 发给Kernel
    void SIG_addFriend();
    //删除好头的信号 发给Kernel
    void SIG_deleteFriend();
    //设置APP的信号 发给Kernel
    void SIG_systemSet();
    //发起群聊的信号 发给Kernel
    void SIG_createGroup();
    //加入群的信号  发给Kernel
    void SIG_addGroup();


public:
    IMS_Dialog(QWidget *parent = nullptr);
    ~IMS_Dialog();
    void showFriend();
    bool m_bNoConnect;

    //添加好友
    void addFriend(UserItem* item,int id = -1);

    //删除好友
    void deleteFriend(QString ip);

    //添加群聊
    void addGroup(GroupItem* item,int id = -1);

    //删除群聊
    void deleteGroup(QString ip,int id = -1);
    //设置用户信息到界面上
    void setUserInfo(int iconid,QString name,QString feeling,long sock = 0);

    //重写关闭窗口事件
    void closeEvent(QCloseEvent* event);
private slots:
    void on_pb_menu_clicked();
    //点击菜单添加的槽函数
    void slot_clickedMenuAddFriend(QAction* paction);

private:
    Ui::IMS_Dialog *ui;
    QVBoxLayout* m_pUDPlayout;    //UDP
    QVBoxLayout* m_pTCPlayout;    //TCP
    QVBoxLayout* m_pGrouplayout;  //群聊
    QMenu* m_pMenu;
    //QWidget *m_widget;
};
#endif // IMS_DIALOG_H
