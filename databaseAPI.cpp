#include "databaseAPI.h"

Database::Database(QSqlDatabase database)
{
    db = database;
    db.setDatabaseName(databaseName);
    db.setUserName(userName);
}

Database::Database(QSqlDatabase database, QString hostName, int port, QString password)
{
    db = database;
    db.setDatabaseName(databaseName);
    db.setUserName(userName);
    db.setHostName(hostName);
    db.setPort(port);
    db.setPassword(password);
}

QSqlDatabase Database::getDatabase()
{
    return db;
}

void Database::setHostName(QString hostName)
{
    db.setHostName(hostName);
}

QString Database::getHostName()
{
    return db.hostName();
}

void Database::setPort(int port)
{
    db.setPort(port);
}

int Database::getPort()
{
    return db.port();
}

void Database::setPassword(QString password)
{
    db.setPassword(password);
}

bool Database::is_connected()
{
    return connected;
}

bool Database::open()
{
    connected = db.open();
    return connected;
}

Database::~Database()
{
    db.close();
}
