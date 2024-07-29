#include "databaseAPI.h"


/**
 * @brief   使用QSqlDatabase类初始化对象
 * @param   database    传入的QSqlDatabase对象
 * @return  void
 * @author  柯劲帆
 * @date    2024-07-28
 */
Database::Database(QSqlDatabase database)
{
    db = database;
    db.setDatabaseName(databaseName);
    db.setUserName(userName);
}


/**
 * @brief   使用QSqlDatabase类、IP、端口、密码初始化对象
 * @param   database    传入的QSqlDatabase对象
 * @param   hostName    数据库IP
 * @param   port        数据库端口
 * @param   password    数据库密码
 * @return  void
 * @author  柯劲帆
 * @date    2024-07-28
 */
Database::Database(QSqlDatabase database, QString hostName, int port, QString password)
{
    db = database;
    db.setDatabaseName(databaseName);
    db.setUserName(userName);
    db.setHostName(hostName);
    db.setPort(port);
    db.setPassword(password);
}


/**
 * @brief   获取database属性
 * @param   void
 * @return  返回QSqlDatabase类database属性
 * @author  柯劲帆
 * @date    2024-07-28
 */
QSqlDatabase Database::getDatabase()
{
    return db;
}


/**
 * @brief   设置hostName属性
 * @param   hostName    数据库IP
 * @return  void
 * @author  柯劲帆
 * @date    2024-07-28
 */
void Database::setHostName(QString hostName)
{
    db.setHostName(hostName);
}


/**
 * @brief   获取hostName属性
 * @param   void
 * @return  返回QString类hostName属性
 * @author  柯劲帆
 * @date    2024-07-28
 */
QString Database::getHostName()
{
    return db.hostName();
}


/**
 * @brief   设置port属性
 * @param   port    数据库端口
 * @return  void
 * @author  柯劲帆
 * @date    2024-07-28
 */
void Database::setPort(int port)
{
    db.setPort(port);
}


/**
 * @brief   获取port属性
 * @param   void
 * @return  返回int类port属性
 * @author  柯劲帆
 * @date    2024-07-28
 */
int Database::getPort()
{
    return db.port();
}


/**
 * @brief   设置password属性
 * @param   password    数据库密码
 * @return  void
 * @author  柯劲帆
 * @date    2024-07-28
 */
void Database::setPassword(QString password)
{
    db.setPassword(password);
}


/**
 * @brief   数据库是否已经连接
 * @param   void
 * @return  数据库连接状态
 *  - true  已连接
 *  - false 未连接
 * @author  柯劲帆
 * @date    2024-07-28
 */
bool Database::is_connected()
{
    return connected;
}

/**
 * @brief   连接数据库
 *  会将连接结果更新到connect属性上。
 * @param   void
 * @return  数据库连接状态
 *  - true  已连接
 *  - false 未连接
 * @author  柯劲帆
 * @date    2024-07-28
 */
bool Database::open()
{
    connected = db.open();
    return connected;
}


/**
 * @brief   释放对象
 *  会将数据库连接关闭。
 * @param   void
 * @return  void
 * @author  柯劲帆
 * @date    2024-07-28
 */
Database::~Database()
{
    db.close();
}
