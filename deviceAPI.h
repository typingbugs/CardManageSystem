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
    bool is_connected();
    bool is_depositAllowed();
    void setDevice(QString name, Database *db);
    QString getName();
};

#endif // DEVICEAPI_H
