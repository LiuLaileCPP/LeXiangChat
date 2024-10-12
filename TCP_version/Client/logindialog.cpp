#include "logindialog.h"
#include "ui_logindialog.h"
#include<QMessageBox>
loginDialog::loginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::loginDialog)
{
    ui->setupUi(this);

    setWindowTitle(QString("QQ"));

    //设置显示最小化 最大化功能
    Qt::WindowFlags flags = Qt::Dialog;
    flags |= Qt::WindowMinMaxButtonsHint;
    flags |= Qt::WindowCloseButtonHint;
    setWindowFlags(flags);
}

loginDialog::~loginDialog()
{
    delete ui;
}

void loginDialog::on_pb_clear_r_clicked()
{
     ui->le_password_r->setText("");
     ui->le_tel_r->setText("");
     ui->le_name_r->setText("");
}


void loginDialog::on_pb_register_clicked()
{
    // 1 获取用户输入的数据
    QString name = ui->le_name_r->text();
    QString password = ui->le_password_r->text();
    QString tel = ui->le_tel_r->text();
    QString name_temp = name;
    QString password_temp = password;
    QString tel_temp = tel;
    // 2 校验数据的合法性
    //判断是否是空字符串 或者是全空格
    if(name.isEmpty() || password.isEmpty() || tel.isEmpty()
        ||name_temp.remove(" ").isEmpty()
        || password_temp.remove(" ").isEmpty()
        ||tel_temp.remove(" ").isEmpty() )
    {
        QMessageBox::about(this,"错误提示","输入为空白，请重试");
        return;
    }
    //检查长度是否合法（name <= 20,tel = 11,password <= 20）
    if(name.length() > 20 || tel.length() != 11 || password.length() > 20)
    {
        QMessageBox::about(this,"错误提示","输入长度有误，请重试");
        return;
    }
    //检查内容是否合法(tel为数字 name password为大小写字母 数字 下划线的组合)
    if(0)
    {
        QMessageBox::about(this,"错误提示","输入格式有误，请重试");
        return;
    }
    // 3 发送给Kernel 发送到服务器 ---- 信号槽
    Q_EMIT SIG_registerCommit(name,tel,password);
}


void loginDialog::on_pb_clear_clicked()
{
    ui->le_password->setText("");
    ui->le_tel->setText("");
}


void loginDialog::on_pb_login_clicked()
{
    // 1 获取用户输入的数据
    QString password = ui->le_password->text();
    QString tel = ui->le_tel->text();
    QString password_temp = password;
    QString tel_temp = tel;
    // 2 校验数据的合法性
    //判断是否是空字符串 或者是全空格
    if(password.isEmpty() || tel.isEmpty()
        || password_temp.remove(" ").isEmpty()
        ||tel_temp.remove(" ").isEmpty() )
    {
        QMessageBox::about(this,"错误提示","输入为空白，请重试");
        return;
    }
    //检查长度是否合法（name <= 20,tel = 11,password <= 20）
    if(tel.length() != 11 || password.length() > 20)
    {
        QMessageBox::about(this,"错误提示","输入长度有误，请重试");
        return;
    }
    //检查内容是否合法(tel为数字 name password为大小写字母 数字 下划线的组合)
    if(0)
    {
        QMessageBox::about(this,"错误提示","输入格式有误，请重试");
        return;
    }
    // 3 发送给Kernel 发送到服务器 ---- 信号槽
    Q_EMIT SIG_loginCommit(tel,password);
}

//重写关闭窗口事件
void loginDialog::closeEvent(QCloseEvent*)
{
    //给kernel发信号 关闭进程
    Q_EMIT SIG_closeLoginDlg();
}













