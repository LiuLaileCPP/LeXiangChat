#ifndef KERNEL_H
#define KERNEL_H

#include <./config/Cli_config.h>
#include <QMainWindow>
#include <./net/udpnet.h>
#include <person.h>
#include <group.h>
#include <reglogdlg.h>
#include <maindlg.h>
#include <item.h>
#include <chatdlg.h>
#include <map>

class kernel;
typedef void(kernel::* pWork)(char* recvData);
QT_BEGIN_NAMESPACE
namespace Ui {
class kernel;
}
QT_END_NAMESPACE

class kernel : public QMainWindow
{
    Q_OBJECT
public:
    int m_no;           //本机在服务器的addr编号
    int m_id;
    int m_iconId;
    QString telOrEmail;
    QString password;
    person* m_pUser;
    std::map<int,person*> m_mapIdToFriends;
    std::map<int,person*> m_mapIdToGroMembs;
    std::map<int,group*> m_mapIdToGroups;
    std::map<int,item*> m_mapIdToFriItems;
    std::map<int,chatdlg*> m_mapIdToFriDlgs;
    std::map<int,item*> m_mapIdToGroItems;
    std::map<int,chatdlg*> m_mapIdToGroDlgs;

public:
    //窗口类
    MainDlg* m_pMainDlg;
    ReglogDlg* m_pReglogDlg;


    //工具类
    udpnet* m_pUdp;

    //工作函数指针数组
    pWork m_arrTypeToFun[_DEF_WORK_TYPE_SIZE_];


public:
    //外部接口，对外使用
    static kernel* m_pKernel;

    //打开服务器
    bool startService();

    //关闭服务器
    void closeService();
public:
    kernel(QWidget *parent = nullptr);
    ~kernel();

signals:
    void SIG_regist(bool res,QString errMsg);
    void SIG_login(bool res,QString errLog);
    // void SIG_chatMsg(int type,int fromId,int fromIcon,recvBuf info);
    void SIG_itemInfo(char* recvData);
    void SIG_about(const QString title,const QString drict);
    void SIG_setNewMsg(QString content,person* p,int type,int id);
    //-------------------------------------------------------




    //槽函数--------------------------------------------------
public slots:

    //处理登录窗口的注册按下
    void slot_regist(QString tel, QString password, int typeRegist);

    //处理登录窗口的登录按下
    void slot_login(QString tel, QString password, int typeLogin);

    //处理发送信息
    void slot_sendMsg(int dlgType,int dlgId,QString content);

    //处理朋友 群聊 群成员item信息
    void slot_itemInfo(char* recvData);


    //-------------------------------------------------------



    //核心工作-------------------------------------------------
    void nuclearWork(char* recvData);

    //第一次连接时服务器发来自己的地址编号 存下来
    void setClientNo(char* recvData);

    //处理注册回复
    void dealRegRs(char* recvData);

    //处理登录回复,接收消息：能否登录 能的话回复：好友信息 群聊信息 群聊成员信息
    void dealLogRs(char* recvData);

    //处理服务器发来的item信息：好友 群聊 群聊成员
    void dealItemInfo(char* recvData);

    //转发群聊信息
    void dealChatGroMsg(char* recvData);

    //处理聊天 转发聊天消息
    void dealChatMsg(char* recvData);

    //处理添加好友请求
    void dealAddFriendRq(char* recvData);

    //服务器对你添加好友的回复
    void dealAddFriendRs(char* recvData);

    //服务器对你删除好友的回复
    void dealDeleteFriendRs(char* recvData);

    //心跳机制验活
    void checkAlive(char* recvData);
    //----------------------------------------------------------------



    //show keep runnig
    void showRunning();

    //初始化工作数组
    void setArrayTypeToFun();

private:
    Ui::kernel *ui;
};
#endif // KERNEL_H
