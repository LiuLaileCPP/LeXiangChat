#include "chatdialog.h"
#include "ui_chatdialog.h"
#include<QTime>
#include<QFont>
#define TEXT_COLOR_RED(STRING)         "<font color=red>" STRING "</font>" "<font color=black> </font>"
#define TEXT_COLOR_BLUE(STRING)        "<font color=blue>" STRING "</font>" "<font color=black> </font>"
#define TEXT_COLOR_GREEN(STRING)        "<font color=green>" STRING "</font>" "<font color=black> </font>"



ChatDialog::ChatDialog(QWidget *parent)
    : QDialog(parent)
    , m_ip("")
    , m_id(-1)
    , m_name("")
    , ui(new Ui::ChatDialog)
{
    ui->setupUi(this);

    //实现最大化 最小化的功能
    Qt::WindowFlags flags = Qt::Dialog;
    flags |= Qt::WindowMinMaxButtonsHint;
    flags |= Qt::WindowCloseButtonHint;
    setWindowFlags(flags);
}

ChatDialog::~ChatDialog()
{
    delete ui;
}

//设置聊天窗口的信息--设置为ip
void ChatDialog::setChatDialogInfo(QString ip,int id,QString name)
{
    //保存信息
    m_ip = ip; //UDP
    m_id = id; //TCP
    m_name = name; //TCP
    //设置窗口title
    if(m_id < 0) //UDP
    {
        setWindowTitle(QString("【%1】---UDP").arg(m_ip));
    }
    else //TCP
    {
        setWindowTitle(QString("【好友<%1>__ID:%2】---TCP").arg(m_name).arg(m_id));
    }
}

//处理点击发送按钮的功能
void ChatDialog::on_pb_send_clicked()
{
    //获取输入的文本
    QString content = ui->te_chat->toPlainText(); //获取纯文本
    //判断是否是空 或者全是空格
    if(content.isEmpty() || content.remove(" ").isEmpty())
    {
        ui->te_chat->clear(); //清除所有空格
        return;
    }

    //获取带格式的文本
    content = ui->te_chat->toHtml();

    //清空编辑窗口
    ui->te_chat->clear();

    //把文本显示到浏览窗口上
    QString tiShi = QString("<font color=red>" "【我 say at %1】" "</font>" "<font color=black> </font>")
                        .arg(QTime::currentTime().toString("hh:mm:ss"));
    QString corContent = QString("<font color=red>" "%1" "</font>" "<font color=black> </font>").arg(content);
    ui->tb_chat->append(tiShi);
    ui->tb_chat->append(corContent);

    // QFont font("SimHei",9,QFont::Bold);
    // font.setStyleStrategy(QFont::PreferAntialias);//偏好抗锯齿
    // QColor color(Qt::red);

    //把数据发给CKernel 由它转发数据
    if(m_id == -1)//UDP
    {
        Q_EMIT SIG_sendMsg(content,m_ip);
    }
    else//TCP
    {
        Q_EMIT SIG_sendMsg(content,m_ip,m_id);
    }
}

//把聊天内容设置到窗口上
void ChatDialog::setContent(QString content,bool noConnect)
{
    if(noConnect)
    {
        //把文本显示到浏览窗口上
        ui->tb_chat->append(QString("【系统提示:无网络连接!重连中...】"));
        return;
    }
    if(m_id == -1) //UDP
    {
        //把文本显示到浏览窗口上
        ui->tb_chat->append(QString("【好友<%1> say at %2】")
                                .arg(m_ip)
                                .arg(QTime::currentTime().toString("hh:mm:ss")));
        ui->tb_chat->append(content);
    }
    else //TCP
    {
        //把文本显示到浏览窗口上
        ui->tb_chat->append(QString("【好友<%1> say at %2】")
                                .arg(m_name)
                                .arg(QTime::currentTime().toString("hh:mm:ss")));
        ui->tb_chat->append(content);
    }
}

void ChatDialog::on_pb_to3_clicked()
{
    //发送发文件的信号给Kernel
    Q_EMIT SIG_sendFriendFile(m_id);
}

