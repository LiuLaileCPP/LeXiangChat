#ifndef NOCONNECTDIALOG_H
#define NOCONNECTDIALOG_H

#include <QDialog>

namespace Ui {
class noConnectDialog;
}

class noConnectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit noConnectDialog(QWidget *parent = nullptr);
    ~noConnectDialog();

private slots:
    void on_pb_clicked();

private:
    Ui::noConnectDialog *ui;
};

#endif // NOCONNECTDIALOG_H
