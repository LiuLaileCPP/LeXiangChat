#ifndef CKERNEL_H
#define CKERNEL_H
//核心处理类
#include"./mediator/INetMediator.h"
#include <QObject>
#include"ims_dialog.h"
#include"INetMediator.h"
#include<chatdialog.h>
#include"groupchatdialog.h"
#include<QString>
#include"logindialog.h"
#include"INet.h"
#include"useritem.h"
#include<groupitem.h>
#include"dataDef.h"
//定义函数指针
class CKernel;
typedef void(CKernel::*pKernelDeal)(char*,int,long) ;
class CKernel : public QObject
{
    Q_OBJECT
public:
    explicit CKernel(QObject *parent = nullptr);
    ~CKernel();


    //生成不同的界面  处理不同的请求
    //处理上线请求
    void dealOnlineRq(char* recvData,int nLen,long lFrom);
    //处理上线回复
     void dealOnlineRs(char* recvData,int nLen,long lFrom);
    //处理聊天请求
     void dealChatRq(char* recvData,int nLen,long lFrom);
    //处理下线请求
     void dealOfflineRq(char* recvData,int nLen,long lFrom);
     //初始化协议头数组
     void setProtocolMap();
     //处理注册回复包rs
     void dealRegisterRs(char* recvData,int nLen,long lFrom);
     //处理登录回复包rs
     void dealLoginRs(char* recvData,int nLen,long lFrom);
     //处理聊天内容 显示到界面
     void dealTCPChatRq(char* recvData, int nLen, long lFrom);
     //处理聊天失败 弹窗报错
     void dealTCPChatRs(char* recvData, int nLen, long lFrom);
     //处理好友信息数据报
     void dealFriendInfo(char* recvData,int nLen,long lFrom);
     //处理添加好友申请数据报
     void dealAddFriendRq(char* recvData,int nLen,long lFrom);
     //处理添加好友回复数据报
     void dealAddFriendRs(char* recvData,int nLen,long lFrom);
     //处理删除好友回复
     void dealDeleteFriendRs(char* recvData,int nLen,long lFrom);
     //处理被朋友删除的函数
     void dealFriendDeleteYouMsg(char* recvData,int nLen,long lFrom);
     //处理服务器的验证登录状态请求
     void dealValidateLogined(char* recvData,int nLen,long lFrom);
     //接收服务器的删除好友回复 （此客户端被某个好友删除了好友关系）
     void dealDeleteYouMsg(char* recvData,int nLen,long lFrom);
     //处理自己加入的群的信息
     void dealMyGroupInfo(char* recvData,int nLen,long lFrom);
     //处理自己加入的群的成员的信息
     void dealMyGroupMemberInfo(char* recvData,int nLen,long lFrom);
     //处理群聊消息
     void dealGroupMsg(char* recvData,int nLen,long lFrom);
     //处理服务器的转发文件询问
     void dealServerSendFileMsg(char* recvData,int nLen,long lFrom);
     //处理发送文件结果
     void dealSendFileResult(char* recvData,int nLen,long lFrom);
     //处理内核接受文件的结果
     void dealRecvFileResult(char* recvData,int nLen,long lFrom);
     //UTF-8转GB2312
     void utf8ToGb2312(QString,char*,int);
     //GB2312转UTF-8
     QString gb2312ToUtf8(char* src);

signals:

public:
    //中介者实现数据收发 中介者的构造函数里自动生成网络功能类的对象
    INetMediator* m_pMediator;
    INetMediator* m_pTCPMediator;

    bool m_isOnline;
    bool userClickLogin;
    bool m_bReConnect;

    long m_socket;
    int m_TCPid;
    int m_iconId;
    QString m_name;
    QString m_tel;
    QString m_password;
    QString m_feeling;

    //提示断网了
    void ShowNoConnect();
    IMS_Dialog* m_pImDlg;
    //根据id保存的TCP许多好友信息
    QMap<int,UserItem*> m_mapTCPIdToUseritem;
    //根据id保存的TCP许多群聊信息
    QMap<int,GroupItem*> m_mapGroupIdToGroupitem;


private:
    loginDialog* m_ploginDialog;
    HANDLE m_handleReConnect;
    //根据ip保存的UDP许多聊天窗口
    QMap<long,ChatDialog*> m_mapIpToChatdlg;
    //根据id保存的TCP许多聊天窗口
    QMap<int,ChatDialog*> m_mapIdToChatdlg;
    //根据id保存的TCP许多群聊的聊天窗口
    QMap<int,GroupChatDialog*> m_mapGroupIdToChatdlg;


    pKernelDeal m_mapPackTypeToFun[_DEF_PROTOCOL_COUNT];
    void setNetStatus(int);
public slots:
    //处理所有接收到的数据
    void slot_ReadyData(char* recvData,int nLen,long lFrom);
    //显示与当前好友的聊天窗口
    void slot_showChatDialog(QString ip,int id);
    //显示当前群聊的聊天窗口
    void slot_showGroupChatDialog(QString ip,int id);
    //处理好友聊天内容
    void slot_sendMsg(QString content,QString  ip,int id);
    //处理群的聊天内容
    void slot_sendGroupMsg(QString content,QString  ip,int id);
    //确认关闭应用程序
    void slot_close();
    //处理注册按钮的信号
    void slot_registerCommit(QString name,QString tel,QString password);
    //处理登录按钮的信号
    void slot_loginCommit(QString tel,QString password);
    //处理loginDialog发送的关闭信号
    void slot_closeLoginDlg();
    //处理添加好友的信号
    void slot_addFriend();
    //处理删除好友的信号
    void slot_deleteFriend();
    //处理系统设置的信号
    void slot_systemSet();
    //处理发起群聊的信号
    void slot_createGroup();
    //处理加入群聊的信号
    void slot_addGroup();
    //处理和群成员聊天的信号
    void slot_memberChatToKernel(int id,QString name,GroupChatDialog* p);
    //处理加群成员为好友的信号
    void slot_memberAddToKernel(int id,QString name,GroupChatDialog* p);
    //处理发送给好友文件的信号
    void slot_sendFriendFile(int id);
    //处理发送给群聊文件的信号
    void slot_sendGroupFile(int id);
    //TCP服务端的Kernel QT里的是TCP的客户端

    //打开服务器
    //bool startService();
    //关闭服务器
    //void closeService();

    //处理所有接收到的数据
    //void dealReadyData(char* recvData,int nLen,long lFrom);
//private:
    //CMySql m_sql;
//public:
    //static CKernel* m_pKernel;

    //线程函数 重连服务器
private:


    //断开连接时显示提示窗口
    void ShowNoConnected();
};

#endif // CKERNEL_H
