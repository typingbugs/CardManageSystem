#ifndef DATABASEAPI_H
#define DATABASEAPI_H


#include <QSqlDatabase>
#include <QString>


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
    QSqlDatabase getDatabase();
    void setHostName(QString hostName);
    QString getHostName();
    void setPort(int port);
    int getPort();
    void setPassword(QString password);
    bool is_connected();
    bool open();
    ~Database();
};


#endif // DATABASEAPI_H
