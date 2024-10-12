#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QDialog>

namespace Ui {
class ChatDialog;
}

class ChatDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChatDialog(QWidget *parent = nullptr);
    ~ChatDialog();
    QString m_ip;
    //设置聊天窗口的信息--设置为ip
    void setChatDialogInfo(QString ip);

    //把聊天内容设置到窗口上
    void setContent(QString content);

private slots:
    void on_pb_send_clicked();

signals:
    void SIG_sendMsg(QString content,QString ip);

private:
    Ui::ChatDialog *ui;
};

#endif // CHATDIALOG_H
