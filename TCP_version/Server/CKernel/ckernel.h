#ifndef CKERNEL_H
#define CKERNEL_H
//核心处理类

#include <QObject>
#include"ims_dialog.h"
#include"INetMediator.h"
#include<chatdialog.h>
#include<QString>
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
signals:

private:
    //主窗口
    IMS_Dialog* m_pImDlg;
    //中介者实现数据收发 中介者的构造函数里自动生成网络功能类的对象
    INetMediator* m_pMediator;

    //根据ip保存的许多聊天窗口
    QMap<long,ChatDialog*> m_mapIpToChatdlg;
public slots:
    //处理所有接收到的数据
    void slot_ReadyData(char* recvData,int nLen,long lFrom);
    //显示与当前好友的聊天窗口
    void slot_showChatDialog(QString ip);
    //处理聊天内容
    void slot_sendMsg(QString content,QString  ip);
    //确认关闭应用程序
    void slot_close();
};

#endif // CKERNEL_H
