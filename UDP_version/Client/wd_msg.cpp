#include "wd_msg.h"
#include "ui_wd_msg.h"

wd_msg::wd_msg(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::wd_msg)
{
    ui->setupUi(this);
}

wd_msg::~wd_msg()
{
    delete ui;
}

void wd_msg::setInfo(int iconId,QString content,QString name)
{
    QString path = QString(":res/icon/%1.png").arg(iconId);
    ui->pb_mem->setIcon(QIcon(path));
    ui->te_msg->setText(content);
    ui->lb_name->setText(name);
}
