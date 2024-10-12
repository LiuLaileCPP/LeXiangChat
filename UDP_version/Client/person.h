#ifndef PERSON_H
#define PERSON_H

#include <QObject>
#include <QString>
class person : public QObject
{
    Q_OBJECT
public:
    bool m_state;
    int m_id;
    int m_iconId;
    QString m_name;
    QString m_account;
    QString m_tel;
    QString m_email;
    QString m_password;
    QString m_sign;
public:
    explicit person(QObject *parent = nullptr);
    void initInfo(bool state,int id,int iconId,
                  QString name,QString account,QString tel,
                  QString email,QString password,QString sign);

signals:
};

#endif // PERSON_H
