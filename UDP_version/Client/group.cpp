#include "group.h"

group::group(QObject *parent)
    : QObject{parent}
{
    m_gId = m_iconId = -1;
    m_size = 0;
    m_name = m_num = m_sign = "";
    m_mapIdToMembs.clear();
}

void group::initInfo(int id,int iconId,int size,QString name,QString num,QString sign)
{
    m_gId = id;
    m_iconId = iconId;
    m_name = name;
    m_num = num;
    m_sign = sign;
}
