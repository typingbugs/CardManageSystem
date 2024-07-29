#include "readerAPI.h"


/**
 * @brief   判断读卡器是否已连接
 *  通过comNumber是否大于0判断。
 * @param   void
 * @return  读卡器连接状态
 *  - true  已连接
 *  - false 未连接
 * @author  柯劲帆
 * @date    2024-07-27
 */
/// \brief 判断读卡器是否已连接
/// \return 如果comNumber大于0，表示已连接，返回true；否则返回false
bool Reader::is_connected()
{
    return comNumber > 0;
}


/**
 * @brief   连接读卡器
 *  尝试通过comNumber连接读卡器。如果连接成功，返回true；否则返回false并重置comNumber为-1。
 * @param   void
 * @return  连接成功返回true，失败返回false
 * @author  柯劲帆
 * @date    2024-07-27
 */
bool Reader::connect()
{
    if (CVCDOurs::connectReaderByCOM(comNumber))
    {
        return true;
    }
    else
    {
        comNumber = -1;
        return false;
    }
}


/**
 * @brief   设置COM口号
 * @param   comNumber   要设置的COM口号
 * @return  void
 * @author  柯劲帆
 * @date    2024-07-27
 */
void Reader::setComNumber(int comNumber)
{
    this->comNumber = comNumber;
}


/**
 * @brief   获取当前COM口号
 * @param   void
 * @return  当前的COM口号
 *  - -1    未连接
 *  - >0    已连接
 * @author  柯劲帆
 * @date    2024-07-27
 */
int Reader::getComNumber()
{
    return comNumber;
}


/**
 * @brief   获取读卡器中卡片的UID列表
 *  通过读卡器的inventory方法获取最大数量为maxViccNum的卡片UID，并将其转换为QStringList返回。
 * @param   maxViccNum  最大卡片数量
 * @return  包含卡片UID的QStringList
 * @author  柯劲帆
 * @date    2024-07-29
 */
QStringList Reader::inventory(int maxViccNum)
{
    // 分配内存用于存储卡片的UID
    uchar_t (*aucUID)[8] = (uchar_t (*)[8])malloc(maxViccNum * sizeof(*aucUID));;

    // 调用CVCDOurs::inventory方法获取卡片数量
    int receivedViccNum = CVCDOurs::inventory(false, '\0', maxViccNum, nullptr, aucUID);

    // 用于存储UID的QStringList
    QStringList uidList;
    for (int i = 0; i < receivedViccNum; ++i)
    {
        char uidStr[8];
        // 将UID从十六进制转换为字符串
        CVCDOurs::HexToString(aucUID[i], 8, uidStr);
        // 将字符串转换为QString并添加到uidList中
        uidList.push_back(QString::fromStdString(uidStr));
    }

    // 释放分配的内存
    free(aucUID);
    return uidList;
}
