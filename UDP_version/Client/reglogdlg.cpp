#include "reglogdlg.h"
#include "ui_reglogdlg.h"
#include <kernel.h>
#include <QMessageBox>
#include <./config/tools.h>

ReglogDlg::ReglogDlg(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ReglogDlg)
{
    ui->setupUi(this);
}

ReglogDlg::~ReglogDlg()
{
    delete ui;
}

void ReglogDlg::setInfo(int id,int iconId,char* name,char* sign)
{

}

void ReglogDlg::slot_regist(bool res,QString errMsg)
{
    if(res)
    {
        QMessageBox::about(this,"注册提示","注册成功，请登录");
    }
    else
    {
        QMessageBox::about(this,"注册提示",errMsg);
    }
    ui->pb_login->setText("登录");
}

void ReglogDlg::slot_login(bool res,QString errMsg)
{
    ui->pb_login->setText("登录");
    if(res)
    {
        QMessageBox::about(this,"登录提示","登录成功，欢迎使用");
        hide();
        kernel::m_pKernel->m_pMainDlg->showNormal();
    }
    else
    {
        QMessageBox::about(this,"登录提示",errMsg);
    }
}


void ReglogDlg::on_pb_regist_clicked()
{
    QString str = "请正确输入信息";
    ui->pb_login->setText(str);
    QString account = ui->le_account->text();
    QString password = ui->le_password->text();
    QString QSres;
    int res[9];
    memset(res,0,9);

    //1. 判断注册类型
    //邮箱注册
    if(account.right(3) == "com" || account.right(3) == "cn")
    {
        //用邮箱注册

        //检测格式是否正确

        //发信号
        //注册类型0是手机号注册，1是邮箱注册

        str = "正在注册中...";
        ui->pb_login->setText(str);
        return;
    }

    //手机号注册
    Tools::Check_telAndPwd(account,password,res,9);
    //返回值 0格式全正确 1手机号为空 2密码为空 3手机号有空格 4密码有空格
    //5手机号长度不对 6密码长度不对 7手机号不规范 8密码不规范


    if(res[0] == 1)
    {
        // 3 发送给Kernel 发送到服务器 ---- 信号槽
        //   注册类型0是手机号注册，1是邮箱注册
        Q_EMIT SIG_registerCommit(account,password,0);
        str = "正在注册中...";
        ui->pb_login->setText(str);
        return;
    }
    if(res[1] == 1)
    {
        QSres += "手机号未输入";
        ui->le_account->clear();
    }
    if(res[2] == 1)
    {
        QSres += " 密码未输入";
        ui->le_password->clear();
    }
    if(res[3] == 1)
    {
        QSres += " 手机号不能有空格";
    }
    if(res[4] == 1)
    {
        QSres += " 密码不能有空格";
    }
    if(res[5] == 1)
    {
        QSres += " 手机号长度有误";
    }
    if(res[6] == 1)
    {
        QSres += " 密码长度有误";
    }
    if(res[7] == 1)
    {
        QSres += " 手机号不规范";
    }
    if(res[8] == 1)
    {
        QSres += " 密码不规范 请重试";
    }
    if(QSres != "")
        QMessageBox::about(this,"错误提示",QSres);
    str = "登录";
    ui->pb_login->setText(str);
}


void ReglogDlg::on_pb_login_clicked()
{
    QString account = ui->le_account->text();
    QString password = ui->le_password->text();
    QString str = "";
    int res[9];
    QString QSres = "";
    memset(res,0,9);

    //1. 判断log类型
    //邮箱log
    if(account.right(3) == "com" || account.right(3) == "cn")
    {
        //用邮箱注册

        //检测格式是否正确

        //发信号
        //注册类型0是手机号注册，1是邮箱注册

        str = "正在登录中...";
        ui->pb_login->setText(str);
        return;
    }

    //手机号log
    Tools::Check_telAndPwd(account,password,res,9);
    //返回值 0格式全正确 1手机号为空 2密码为空 3手机号有空格 4密码有空格
    //5手机号长度不对 6密码长度不对 7手机号不规范 8密码不规范


    if(res[0] == 1)
    {
        // 3 发送给Kernel 发送到服务器 ---- 信号槽

        Q_EMIT SIG_loginCommit(account,password,0);
        str = "正在登录中...";
        ui->pb_login->setText(str);
        return;
    }
    if(res[1] == 1)
    {
        QSres += "手机号未输入";
        ui->le_account->clear();
    }
    if(res[2] == 1)
    {
        QSres += " 密码未输入";
        ui->le_password->clear();
    }
    if(res[3] == 1)
    {
        QSres += " 手机号不能有空格";
    }
    if(res[4] == 1)
    {
        QSres += " 密码不能有空格";
    }
    if(res[5] == 1)
    {
        QSres += " 手机号长度有误";
    }
    if(res[6] == 1)
    {
        QSres += " 密码长度有误";
    }
    if(res[7] == 1)
    {
        QSres += " 手机号不规范";
    }
    if(res[8] == 1)
    {
        QSres += " 密码不规范 请重试";
    }
    if(QSres != "")
        QMessageBox::about(this,"错误提示",QSres);
}

