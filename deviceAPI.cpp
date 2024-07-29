#include "deviceAPI.h"

bool Device::is_connected()
{
    return connected;
}


bool Device::is_depositAllowed()
{
    return depositAllowed;
}


void Device::setDevice(QString name, Database *db)
{
    QSqlQuery query(db->getDatabase());
    QString sql = QString("select * from device where `name` = '%1';").arg(name);
    query.exec(sql);
    if (query.next())
    {
        connected = true;
        this->name = name;
        depositAllowed = query.value(2).toBool();
    }
    else
    {
        connected = false;
        depositAllowed = false;
    }
}


QString Device::getName()
{
    if (connected) {
        if (depositAllowed) return name + QString("（可充值）");
        else return name + QString("（仅可消费）");
    }
    else return QString("未指定设备名");
}
