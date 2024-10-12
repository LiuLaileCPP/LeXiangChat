#include "item.h"
#include "ui_item.h"
#include <QMouseEvent>
#include <kernel.h>

item::item(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::item)
{
    ui->setupUi(this);
}

item::~item()
{
    delete ui;
}

void item::setInfo(bool state,int iconId,QString name)
{
    ui->lb_msg->setText("");
    ui->lb_name->setText(name);
    QString path = QString(":res/icon/%1.png").arg(iconId);
    if(state)
    {
        ui->pb_icon->setIcon(QIcon(path));
    }
    else
    {
        QBitmap bp;
        bp.load(path);
        ui->pb_icon->setIcon(bp);   //设置为黑白灰显示
    }
}

void item::setClrName(int i)
{
    if(i == 1)
    {
        ui->lb_name->setStyleSheet("color:rgb(0,0,0)");
    }
    else if(i == 0)
        ui->lb_name->setStyleSheet("color:rgb(0,0,0)");
    else if(i == 2)
        ui->lb_name->setStyleSheet("color:rgb(255,0,0)");
}

void item::mousePressEvent(QMouseEvent* ev)
{
    qDebug()<<"Press item";
    qDebug()<<"fItems: "<<kernel::m_pKernel->m_mapIdToFriItems.size();
    qDebug()<<"gItems: "<<kernel::m_pKernel->m_mapIdToGroItems.size();
    auto ite = kernel::m_pKernel->m_mapIdToFriItems.begin();
    while(ite != kernel::m_pKernel->m_mapIdToFriItems.end())
    {
        //变浅
        ite->second->setStyleSheet("background-color:rgb(230,229,228)");
        ite->second->setStyleSheet("font-weight: normal");
        (*ite).second->setClrName(0);
        ite++;
    }
    ite = kernel::m_pKernel->m_mapIdToGroItems.begin();
    while(ite != kernel::m_pKernel->m_mapIdToGroItems.end())
    {
        //变浅
        ite->second->setStyleSheet("background-color:rgb(230,229,228)");
        ite->second->setStyleSheet("font-weight: normal");
        (*ite).second->setClrName(0);
        qDebug()<<"Origin a group item";
        ite++;
    }
    this->setStyleSheet("background-color:rgb(196,196,196)");
    this->setStyleSheet("font-weight: bold;");
    setClrName(1);

    emit SIG_clicked();
}
