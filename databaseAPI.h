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
    Database(QSqlDatabase database)
    {
        db = database;
        db.setDatabaseName(databaseName);
        db.setUserName(userName);
    }

    Database(QSqlDatabase database, QString hostName, int port, QString password)
    {
        db = database;
        db.setDatabaseName(databaseName);
        db.setUserName(userName);
        db.setHostName(hostName);
        db.setPort(port);
        db.setPassword(password);
    }

    QSqlDatabase getDatabase()
    {
        return db;
    }

    void setHostName(QString hostName)
    {
        db.setHostName(hostName);
    }

    QString getHostName()
    {
        return db.hostName();
    }

    void setPort(int port)
    {
        db.setPort(port);
    }

    int getPort()
    {
        return db.port();
    }

    void setPassword(QString password)
    {
        db.setPassword(password);
    }

    bool is_connected()
    {
        return connected;
    }

    bool open()
    {
        connected = db.open();
        return connected;
    }

    ~Database()
    {
        db.close();
    }
};


#endif // DATABASEAPI_H
