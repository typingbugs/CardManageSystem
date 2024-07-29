#ifndef DEVICEAPI_H
#define DEVICEAPI_H

#include <QString>
#include <QSqlQuery>
#include <databaseAPI.h>


/**
 * @class   Device
 * @brief   设备名相关接口类
 *  控制本应用中设备名相关变量。
 * @author  柯劲帆
 * @date    2024-07-28
 */
class Device
{
private:
    bool verified = false;
    bool depositAllowed = false;
    QString name = QString("未指定设备名");

public:
    void setDevice(QString name, Database *db);

    QString getName();
    QString getNameAndDepositAllowed();

    bool is_verified();
    bool is_depositAllowed();
};

#endif // DEVICEAPI_H
