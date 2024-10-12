#include "chatdialog.h"
#include "ui_chatdialog.h"
#include<QTime>

ChatDialog::ChatDialog(QWidget *parent)
    : QDialog(parent)
    , m_ip("")
    ,ui(new Ui::ChatDialog)
{
    ui->setupUi(this);
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
void ChatDialog::setChatDialogInfo(QString ip)
{
    //保存信息
    m_ip = ip;
    //设置窗口title
    setWindowTitle(QString("【%1】").arg(m_ip));
}

void ChatDialog::on_pb_send_clicked()
{
    //获取输入的文本
    QString content = ui->te_chat->toPlainText(); //获取纯文本
    //判断是否是空 或者全是空格
    if(content.isEmpty() || content.remove(" ").isEmpty())
    {
        ui->te_chat->clear();
        return;
    }

    //获取带格式的文本
    content = ui->te_chat->toHtml();

    //清空编辑窗口
    ui->te_chat->clear();

    //把文本显示到浏览窗口上
    ui->tb_chat->append(QString("【I say at %1】")
    .arg(QTime::currentTime().toString("hh:mm:ss")));
    ui->tb_chat->append(content);

    //把数据发给CKernel 由它转发数据
    Q_EMIT SIG_sendMsg(content,m_ip);
}

//把聊天内容设置到窗口上
void ChatDialog::setContent(QString content)
{
    //把文本显示到浏览窗口上
    ui->tb_chat->append(QString("【%1 say at %2】")
                            .arg(m_ip)
                            .arg(QTime::currentTime().toString("hh:mm:ss")));
    ui->tb_chat->append(content);

}
