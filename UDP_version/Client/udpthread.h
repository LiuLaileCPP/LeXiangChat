#ifndef UDPTHREAD_H
#define UDPTHREAD_H

#include <QObject>
#include <./net/udpnet.h>

class udpThread : public QObject
{
    Q_OBJECT
public:
    explicit udpThread(QObject *parent = nullptr);
    udpnet* m_pUdp;
    void recvWork();
    void aliveWork();
signals:
};

#endif // UDPTHREAD_H
