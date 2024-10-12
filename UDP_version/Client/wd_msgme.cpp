#include "wd_msgme.h"
#include "ui_wd_msgme.h"

wd_msgMe::wd_msgMe(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::wd_msgMe)
{
    ui->setupUi(this);
}

wd_msgMe::~wd_msgMe()
{
    delete ui;
}

void wd_msgMe::setInfo(int iconId,QString content,QString name)
{
    QString path = QString(":res/icon/%1.png").arg(iconId);
    ui->pb_me->setIcon(QIcon(path));
    ui->te_msg->setText(content);
}
