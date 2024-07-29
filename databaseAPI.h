#ifndef DATABASEAPI_H
#define DATABASEAPI_H

#include <QSqlDatabase>
#include <QString>


/**
 * @class   Database
 * @brief   数据库操作相关接口类
 *  固定了databaseName为"cardManageSystem"，userName为"cardManageSystem"。
 *  控制本应用中数据库相关变量。
 * @author  柯劲帆
 * @date    2024-07-28
 */
class Database
{
private:
    QSqlDatabase db;
    bool connected = false;
    QString databaseName = QString("cardManageSystem");
    QString userName = QString("cardManageSystem");

public:
    Database(QSqlDatabase database);
    Database(QSqlDatabase database, QString hostName, int port, QString password);
    ~Database();

    void setHostName(QString hostName);
    void setPort(int port);
    void setPassword(QString password);

    QSqlDatabase getDatabase();
    int getPort();
    QString getHostName();

    bool is_connected();

    bool open();
};


#endif // DATABASEAPI_H
