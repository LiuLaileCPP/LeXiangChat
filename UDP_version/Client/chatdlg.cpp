#include "chatdlg.h"
#include "ui_chatdlg.h"
#include <QMessageBox>
#include <kernel.h>

chatdlg::chatdlg(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::chatdlg)
{
    ui->setupUi(this);
    m_type = m_id = m_iconId = -1;
    m_chatMsgLayout = new QVBoxLayout;
    ui->wd_msg->setLayout(m_chatMsgLayout);
}

chatdlg::~chatdlg()
{
    delete ui;
}

void chatdlg::slot_show()
{
    /*auto ite = kernel::m_pKernel->m_mapIdToFriDlgs.begin();
    while(ite != kernel::m_pKernel->m_mapIdToFriDlgs.end())
    {
        (*ite).second->hide();
        ite++;
    }
    ite = kernel::m_pKernel->m_mapIdToGroDlgs.begin();
    while(ite != kernel::m_pKernel->m_mapIdToGroDlgs.end())
    {
        (*ite).second->hide();
        ite++;
    }
    this->show();*/

    for(auto i =0;i<kernel::m_pKernel->m_pMainDlg->m_layout->count();i++){
        QWidget *w = kernel::m_pKernel->m_pMainDlg->m_layout->itemAt(i)->widget();
        if(w != nullptr){
            w->setVisible(false);
        }
    }
    kernel::m_pKernel->m_pMainDlg->m_layout->addWidget(this);
    this->show();
}

void chatdlg::setInfo(int type,bool state,int id,int iconId,QString name)
{
    m_type = type;
    m_id = id;
    ui->lb_name->setText(name);
    m_iconId = iconId;
}


void chatdlg::on_pb_send_clicked() //只用来发信号
{
    QString content = ui->te_send->toPlainText();
    QString tmp = content;
    if(content == "")
    {
        QMessageBox::about(this,"提示","请先输入内容");
        return;
    }
    if(tmp.remove(" ").isEmpty())
    {
        QMessageBox::about(this,"提示","内容为空，请更改");
        ui->te_send->clear();
        return;
    }
    //生成聊天气泡
    wd_msgMe* msg = new wd_msgMe;
    msg->setInfo(kernel::m_pKernel->m_iconId,content,"");
    m_chatMsgLayout->addWidget(msg);
    m_listMsgMe.push_back(msg);
    ui->te_send->clear();
    //发信号
    Q_EMIT SIG_sendMsg(m_type,m_id,content);// toId content
}

//显示新消息
void chatdlg::setNewMsg(QString content,person* fromWho,int type,int id)
{
    if(type != m_type)
        return;
    if(id != m_id)
        return;
    //生成聊天气泡
    qDebug()<<"**** coredialog setNewMsg:: 接收到新消息 "<<" fromWho:"<<fromWho->m_name<<" fromId:"<<fromWho->m_id<<" msg:"<<content;
    wd_msg* msg = new wd_msg;
    if(type == 0)
        msg->setInfo(fromWho->m_iconId,content,"");
    else if(type == 1)
        msg->setInfo(fromWho->m_iconId,content,fromWho->m_name);
    m_chatMsgLayout->addWidget(msg);
    //msg->show();
    m_listMsg.push_back(msg);
}
