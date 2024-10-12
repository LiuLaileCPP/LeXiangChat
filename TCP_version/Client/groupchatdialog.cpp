#include "groupchatdialog.h"
#include "ui_groupchatdialog.h"
#include<QTime>
#include<QMessageBox>

GroupChatDialog::GroupChatDialog(QWidget *parent)
    : QDialog(parent)
    , m_id(-1)
    , m_ip("")
    , m_name("")
    , ui(new Ui::GroupChatDialog)
    , m_pGroupMemberlayout(new QVBoxLayout)
{
    ui->setupUi(this);

    m_pGroupMemberlayout->setSpacing(0);
    m_pGroupMemberlayout->setContentsMargins(0,0,0,0);
    ui->wdg_Merlist->setLayout(m_pGroupMemberlayout);

    //设置显示最小化 最大化功能
    Qt::WindowFlags flags = Qt::Dialog;
    flags |= Qt::WindowMinMaxButtonsHint;
    flags |= Qt::WindowCloseButtonHint;
    setWindowFlags(flags);
}

GroupChatDialog::~GroupChatDialog()
{
    delete ui;
    delete m_pGroupMemberlayout;
}

//添加群成员到层
void GroupChatDialog::addMember(MemberItem* item,int id)
{
    if(item == nullptr)
    {
        return;
    }
    if(id == -1)
    {
        //m_pUDPlayout->addWidget(item);
        QMessageBox::about(this,"添加失败","群成员id == -1");
    }
    else
    {
        m_pGroupMemberlayout->addWidget(item);
        //QMessageBox::about(this,QString("群:%1添加了成员").arg(m_id),QString("群成员id：%1").arg(id));
    }
}

//删除群成员从层
void GroupChatDialog::deleteMember(QString ip,int id)
{
    //遍历layout上的所有控件memberitem
    int i = 0;
    QLayoutItem* child = nullptr;
    while(nullptr != (child = m_pGroupMemberlayout->takeAt(i++)))
    {
        MemberItem* item = (MemberItem*)child->widget();
        if (item->m_id == id)
        {
            //先将子类的parent赋空
            item->setParent(nullptr);
            delete child;
            break;
        }
    }
}

//设置聊天窗口的信息--设置为ip
void GroupChatDialog::setG_ChatDialogInfo(QString ip,int id,QString name)
{
    //保存信息
    m_id = id; //TCP
    m_name = name; //TCP
    //设置窗口title
    if(m_id < 0) //UDP
    {
        setWindowTitle(QString("【%1】---UDP").arg(m_ip));
    }
    else //TCP
    {
        setWindowTitle(QString("【群聊<%1>__ID:%2】---TCP").arg(m_name).arg(m_id));
    }
}

//处理点击发送按钮的功能
void GroupChatDialog::on_pb_send_clicked()
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
    // ui->tb_chat->append(QString("【我 say at %1】")
    //                         .arg(QTime::currentTime().toString("hh:mm:ss")));
    // ui->tb_chat->append(content);

    //把数据发给CKernel 由它转发数据
    if(m_id == -1)//UDP
    {
        Q_EMIT SIG_sendGroupMsg(content,m_ip);
    }
    else//TCP
    {
        Q_EMIT SIG_sendGroupMsg(content,m_ip,m_id);
    }
}

//把聊天内容设置到窗口上
void GroupChatDialog::setG_Content(QString content,QString memberName,bool noConnect)
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
        ui->tb_chat->append(QString("【%1[%2] say at %3】")
                                .arg(m_ip)
                                .arg(memberName)
                                .arg(QTime::currentTime().toString("hh:mm:ss")));
        ui->tb_chat->append(content);
    }
    else //TCP
    {
        //把文本显示到浏览窗口上
        ui->tb_chat->append(QString("【%1[%2] say at %3】")
                                .arg(m_name)
                                .arg(memberName)
                                .arg(QTime::currentTime().toString("hh:mm:ss")));
        ui->tb_chat->append(content);
    }
}

//处理memberitem 发送的和成员聊天 加好友的信号
void GroupChatDialog::slot_memberChat(int id,QString name)
{
    Q_EMIT SIG_memberChatToKernel(id,name,this);
}

void GroupChatDialog::slot_memberAdd(int id,QString name)
{
    Q_EMIT SIG_memberAddToKernel(id,name,this);
}









void GroupChatDialog::on_pb_to3_clicked()
{
    //发信号到Kernel
    Q_EMIT SIG_sendGroupFile(m_id);
}

