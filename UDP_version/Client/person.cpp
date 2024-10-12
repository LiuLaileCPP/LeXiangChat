#include "person.h"

person::person(QObject *parent)
    : QObject{parent}
{
    m_state = false;
    m_id = m_iconId = -1;
    m_name = m_account = m_tel = m_email = m_password = m_sign = "";

}

void person::initInfo(bool state,int id,int iconId,QString name,QString account,QString tel,QString email,QString password,QString sign)
{
    m_state = state;
    m_id = id;
    m_iconId = iconId;
    m_name = name;
    m_account = account;
    m_tel = tel;
    m_email = email;
    m_password = password;
    m_sign = sign;
}
