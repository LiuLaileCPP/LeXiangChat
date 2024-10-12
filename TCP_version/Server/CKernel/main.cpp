//#include "ims_dialog.h"
//#include"CKernel/ckernel.h"
#include"../mediator/TcpClientMediator.h"
#include"../mediator/TcpServiceMediator.h"
#include<Windows.h>
#include"../dataDef/dataDef.h"
#include"../CKernel.h"

//#include <QApplication>

int main(int argc, char *argv[])
{
    //QApplication a(argc, argv);
    // IMS_Dialog w;
    // w.show();

    //创建核心处理类的对象
    //CKernel k;

    //return a.exec();


    ////打开服务端
    //INetMediator* p = new TcpServiceMediator;
    //if (!p->OpenNet())
    //{
    //    cout << "S打开网络失败" << endl;
    //    return -1;
    //}
    ////打开客户端
    //INetMediator*q = new TcpClientMediator;
    //if (!q->OpenNet())
    //{
    //    cout << "C打开网络失败" << endl;
    //    return -1;
    //}
    //char ch[10] = "Hello";
    //Sleep(199);
    //q->SendData(ch, 10, 0);

    //打开服务器
    CKernel kernel;
    if (!kernel.startService())
    {
        cout << "打开网络失败"<<endl;

        //关闭服务器
        kernel.closeService();
        return - 1;
    }
    while (1)
    {
        Sleep(10000);
        cout << "Is running" << endl;
    }


    return 0;
}
