#ifndef CHATDLG_H
#define CHATDLG_H

#include <QDialog>
#include <wd_msg.h>
#include <wd_msgme.h>
#include <list>
#include <QVBoxLayout>
#include <person.h>

namespace Ui {
class chatdlg;
}

class chatdlg : public QDialog
{
    Q_OBJECT

public:
    int m_type; //这是群的chatdlg 还是好友的
    int m_id;
    std::list<wd_msg*> m_listMsg;
    std::list<wd_msgMe*> m_listMsgMe;
    explicit chatdlg(QWidget *parent = nullptr);
    ~chatdlg();
    void setInfo(int type,bool state,int id,int iconId,QString name);
    void setNewMsg(QString content,person* fromWho,int type,int id);
private:
    Ui::chatdlg *ui;
    int m_iconId;
    QVBoxLayout* m_chatMsgLayout;
    //---------------------------------------------------------------
signals:
    void SIG_sendMsg(int dlgType,int id,QString content);

    //---------------------------------------------------------------


    //---------------------------------------------------------------
private slots:
    void slot_show();
    void on_pb_send_clicked();


    //---------------------------------------------------------------
};

#endif // CHATDLG_H
