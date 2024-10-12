#ifndef GROUP_H
#define GROUP_H

#include <QObject>
#include <list>
#include <person.h>

class group : public QObject
{
    Q_OBJECT
public:
    int m_gId;
    int m_iconId;
    int m_size;
    QString m_name;
    QString m_num;
    QString m_sign;
    std::map<int,person*> m_mapIdToMembs;
public:
    explicit group(QObject *parent = nullptr);
    void initInfo(int gId,int iconId,int size,QString name,QString num,QString sign);
signals:
};

#endif // GROUP_H
