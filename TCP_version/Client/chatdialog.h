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
    int m_id;
    QString m_name;
    //设置聊天窗口的信息--设置为ip
    void setChatDialogInfo(QString ip,int id = -1,QString name = "");

    //把聊天内容设置到窗口上
    void setContent(QString content,bool noConnect = false);

private slots:
    void on_pb_send_clicked();

    void on_pb_to3_clicked();

signals:
    void SIG_sendMsg(QString content,QString ip,int id = -1);

    //发送文件的信号
    void SIG_sendFriendFile(int id);
private:
    Ui::ChatDialog *ui;
};

#endif // CHATDIALOG_H
