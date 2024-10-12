#include "maindlg.h"
#include "ui_maindlg.h"

MainDlg::MainDlg(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainDlg)
{
    ui->setupUi(this);
    m_layout = new QVBoxLayout;
    m_layout2 = new QVBoxLayout;

    m_layout->setDirection(QBoxLayout::TopToBottom);
    m_layout2->setDirection(QBoxLayout::TopToBottom);
    ui->pb_search->addAction("添加好友");
    ui->pb_search->addAction("删除好友");
    ui->pb_search->addAction("加入群");
    ui->pb_search->addAction("发起群聊");
    ui->wd_chatdlg->setLayout(m_layout);
    ui->wd_item->setLayout(m_layout2);

    //设置小控件间的间隔
    m_layout->setSpacing(0);
    m_layout2->setSpacing(0);

    //设置小控件和大控件的间隔
    m_layout->setContentsMargins(0,0,0,0);
    m_layout2->setContentsMargins(0,0,0,0);



    //设置显示最小化 最大化功能
    Qt::WindowFlags flags = Qt::Dialog;
    flags |= Qt::WindowMinMaxButtonsHint;
    flags |= Qt::WindowCloseButtonHint;
    setWindowFlags(flags);
}

MainDlg::~MainDlg()
{
    delete ui;
}

void MainDlg::setInfo(int id,int iconId,char* name,char* sign)
{
    ui->lb_name->setText(name);
    ui->pb_icon->setIcon(QIcon(QString(":res/icon/%1.png").arg(iconId)));
}

void MainDlg::addItem(item* item)
{
    m_layout2->addWidget(item);
}


void MainDlg::addDlg(chatdlg* dlg)
{
    m_layout->addWidget(dlg);
}
