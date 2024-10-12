#include "noconnectdialog.h"
#include "ui_noconnectdialog.h"

noConnectDialog::noConnectDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::noConnectDialog)
{
    ui->setupUi(this);
}

noConnectDialog::~noConnectDialog()
{
    delete ui;
}

void noConnectDialog::on_pb_clicked()
{
    this->~noConnectDialog();
}

