#include "udpthread.h"
#include <kernel.h>
#include <QDebug>

udpThread::udpThread(QObject *parent)
    : QObject{parent}
{
    m_pUdp = 0;
}

void udpThread::recvWork()
{
    udpnet* p = m_pUdp;
    if (!p)
    {
        qDebug() << "** thread_recv end, arg==nullptr";
        return;
    }

    while (1)
    {
        qDebug() << "** thread_recv running...";
        if (!p->m_netState)
        {
            qDebug() << "** thread_recv nonet...";
            p->InitNet();
            Sleep(1000);
            continue;
        }
        if (p->RecvData())
        {
            kernel::m_pKernel->nuclearWork((char*)&(p->m_recvbuf));
        }

        p->bzeroRecvBuf();//测试
    }
}


void udpThread::aliveWork()
{
    kernel::m_pKernel->checkAlive(0);
}
