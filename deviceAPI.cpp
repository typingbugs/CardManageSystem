#include "deviceAPI.h"


/**
 * @brief   设备是否已经认证
 * @param   void
 * @return  设备认证状态
 *  - true  已认证
 *  - false 未认证
 * @author  柯劲帆
 * @date    2024-07-28
 */
bool Device::is_verified()
{
    return verified;
}


/**
 * @brief   设备是否可充值
 * @param   void
 * @return  设备充值权限
 *  - true  可充值
 *  - false 不可充值（仅可消费）
 * @author  柯劲帆
 * @date    2024-07-29
 */
bool Device::is_depositAllowed()
{
    return depositAllowed;
}


/**
 * @brief   设置并认证设备
 *  认证设备会更新设备名和充值权限。
 * @param   name    设备名
 * @param   db  数据库
 * @return  void
 * @author  柯劲帆
 * @date    2024-07-28
 */
void Device::setDevice(QString name, Database* db)
{
    QSqlQuery query(db->getDatabase());
    query.prepare(QString("select depositAllowed from device "
                          "where name = :name;"));
    query.bindValue(":name", name);
    query.exec();
    if (query.next())
    {
        verified = true;
        this->name = name;
        depositAllowed = query.value(0).toBool();
    }
    else
    {
        verified = false;
        depositAllowed = false;
    }
}


/**
 * @brief   获取设备名及其充值权限
 * @param   void
 * @return  返回QString类name属性
 *  - 若设备未认证返回"未指定设备名"
 *  - 若设备已认证
 *      - 若设备可充值，返回设备名加"（可充值）"
 *      - 若设备不可充值，返回设备名加"（仅可消费）
 * @author  柯劲帆
 * @date    2024-07-28
 */
QString Device::getNameAndDepositAllowed()
{
    if (verified) {
        if (depositAllowed) return name + QString("（可充值）");
        else return name + QString("（仅可消费）");
    }
    else return QString("未指定设备名");
}


/**
 * @brief   获取设备名
 * @param   void
 * @return  返回QString类name属性
 *  - 若设备未认证返回"未指定设备名"
 *  - 若设备已认证返回设备名
 * @author  柯劲帆
 * @date    2024-07-29
 */
QString Device::getName()
{
    if (verified) return name;
    else return QString("未指定设备名");
}
