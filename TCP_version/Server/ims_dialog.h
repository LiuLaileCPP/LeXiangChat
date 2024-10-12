#ifndef IMS_DIALOG_H
#define IMS_DIALOG_H

#include <QDialog>
#include<QCloseEvent>
#include<QVBoxLayout>
#include"./useritem.h"
#include<QMainWindow>
#include<QWidget>
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
public:
    IMS_Dialog(QWidget *parent = nullptr);
    ~IMS_Dialog();
    void showFriend();

    //添加好友
    void addFriend(UserItem* item);

    //删除好友
    void deleteFriend(QString ip);

    //重写关闭窗口事件
    void closeEvent(QCloseEvent* event);
private:
    Ui::IMS_Dialog *ui;
    QVBoxLayout* m_layout;
    //QWidget *m_widget;
};
#endif // IMS_DIALOG_H
