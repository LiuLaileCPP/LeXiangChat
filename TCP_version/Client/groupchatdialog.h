#ifndef GROUPCHATDIALOG_H
#define GROUPCHATDIALOG_H

#include <QDialog>
#include<QVBoxLayout>
#include"memberitem.h"

namespace Ui {
class GroupChatDialog;
}

class GroupChatDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GroupChatDialog(QWidget *parent = nullptr);
    ~GroupChatDialog();
    int m_id;
    QString m_ip;
    QString m_name;
    //保存它的所有成员的信息
    QMap<int,MemberItem*> m_mapMemberIdToMemberItem;
    //设置聊天窗口的信息--设置为ip
    void setG_ChatDialogInfo(QString ip,int id = -1,QString name = "");

    //把聊天内容设置到窗口上
    void setG_Content(QString content,QString memberName,bool noConnect = false);

    //添加群成员到层
    void addMember(MemberItem* item,int id);
    //删除群成员从层
    void deleteMember(QString ip,int id);

signals:
    void SIG_memberChatToKernel(int id,QString name,GroupChatDialog*);
    void SIG_memberAddToKernel (int id,QString name,GroupChatDialog*);

private slots:
    void on_pb_send_clicked();
    //处理memberitem 发送的和成员聊天 加好友的信号
    void slot_memberChat(int id,QString name);
    void slot_memberAdd(int id,QString name);

    void on_pb_to3_clicked();

signals:
    void SIG_sendGroupMsg(QString content,QString ip,int id = -1);
    void SIG_sendGroupFile(int);
private:
    Ui::GroupChatDialog *ui;
    QVBoxLayout* m_pGroupMemberlayout;  //群聊的成员
};

#endif // GROUPCHATDIALOG_H
