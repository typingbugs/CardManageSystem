#ifndef DEVICEAPI_H
#define DEVICEAPI_H

#include <QString>
#include <QSqlQuery>
#include <databaseAPI.h>

class Device
{
private:
    bool connected = false;
    bool depositAllowed = false;
    QString name = QString("未指定设备名");

public:
    bool is_connected()
    {
        return connected;
    }

    bool is_depositAllowed()
    {
        return depositAllowed;
    }

    void setDevice(QString name, Database *db)
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

    QString getName()
    {
        if (connected) return name;
        else return QString("未指定设备名");
    }
};

#endif // DEVICEAPI_H
