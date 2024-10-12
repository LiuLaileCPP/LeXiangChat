#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include<QCloseEvent>

namespace Ui {
class loginDialog;
}

class loginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit loginDialog(QWidget *parent = nullptr);
    ~loginDialog();

    //重写关闭窗口事件
    void closeEvent(QCloseEvent*);

private slots:
    void on_pb_clear_r_clicked();

    void on_pb_register_clicked();

    void on_pb_clear_clicked();

    void on_pb_login_clicked();

private:
    Ui::loginDialog *ui;
signals:
    void SIG_registerCommit(QString,QString,QString);
    // 3 发送给Kernel 发送到服务器 ---- 信号槽
    void SIG_loginCommit(QString tel,QString password);

    //关闭信号 发给Kernel
    void SIG_closeLoginDlg();
};

#endif // LOGINDIALOG_H
