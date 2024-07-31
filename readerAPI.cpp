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
 * @details
 * 通过读卡器的inventory方法获取最大数量为maxViccNum的卡片UID，并将其转换为QStringList返回。
 * 该函数分配内存来存储卡片的UID，然后调用CVCDOurs::inventory来获取卡片信息。
 * 获取到的UID会转换为字符串并添加到QStringList中，最后返回该列表。
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


/**
 * @brief   读取记录数量和最近一条记录的index
 * 该函数用于从卡片的block0中读取记录数量和最近一条记录的索引。
 * @param   recordNum   记录数量，通过引用返回，类型为 int
 * @param   recordIndex 最近一条记录的索引，通过引用返回，类型为 int
 * @param   cardId      要读取记录的卡片ID，类型为 QString
 * @return  bool        是否读取成功
 * - true   读取成功
 * - false  读取失败
 * @details
 * 函数首先将卡片ID转换为 `uchar_t` 类型的字符数组，然后调用 `readBlocks` 函数从卡片的block0中读取记录数量和索引。
 * 如果 `readBlocks` 返回的hex数量为0，则表示读取失败，函数返回 `false`。
 * 否则，函数将读取的结果转换为整数，并通过引用参数 `recordNum` 和 `recordIndex` 返回。
 * @author  柯劲帆
 * @date    2024-07-31
 */
bool Reader::readRecordNumber(int &recordNum, int &recordIndex, QString cardId)
{
    // 记录数量在block0的byte0的bit0-3中
    // 最近一条记录的index在block0的byte0的bit4-7中

    // 分配内存用于存储读取结果字符串
    uchar_t recordIndexHex[4] = {0};    // 一个block有4个byte，1个byte有两个hex，会返回8个hex存在4个uchar_t中

    uchar_t cardIdHex[8] = {0};
    QByteArray ba = cardId.toLatin1();
    StringToHex(ba.data(), cardIdHex);

    int hexNum = readBlocks(0, 1, recordIndexHex, nullptr, cardIdHex);
    if (hexNum == 0) return false;

    recordNum = (int)(recordIndexHex[0] & 0x0F);
    recordIndex = (int)(recordIndexHex[1] >> 4);

    return true;
}


/**
 * @brief   写入记录数量和最近一条记录的index
 * 该函数用于将记录数量和最近一条记录的索引写入到卡片的block0中。
 * @param   recordNum   记录数量，类型为 int
 * @param   recordIndex 最近一条记录的索引，类型为 int
 * @param   cardId      要写入记录的卡片ID，类型为 QString
 * @return  bool        是否写入成功
 * - true   写入成功
 * - false  写入失败
 * @details
 * 函数首先将记录数量和记录索引转换为 `uchar_t` 类型，并存储在 `recordIndexStr` 数组中。
 * 然后，函数将卡片ID转换为 `uchar_t` 类型的字符数组，并调用 `writeBlock` 函数将记录数量和索引写入到卡片的block0中。
 * 如果 `writeBlock` 函数返回的写入行数为1，则表示写入成功，函数返回 `true`；否则，函数返回 `false`。
 * @author  柯劲帆
 * @date    2024-07-31
 */
bool Reader::writeRecordNumber(int recordNum, int recordIndex, QString cardId)
{
    // 记录数量在block0的byte0的bit0-3中
    // 最近一条记录的index在block0的byte0的bit4-7中

    // 分配内存用于存储读取结果字符串
    uchar_t recordIndexStr[4] = {0};
    recordIndexStr[0] = (uchar_t)(recordIndex << 4);
    recordIndexStr[0] += (uchar_t)(recordNum);

    uchar_t cardIdHex[8] = {0};
    QByteArray ba = cardId.toLatin1();
    StringToHex(ba.data(), cardIdHex);

    int success = writeBlock(0, recordIndexStr, cardIdHex);
    return success == 1;
}


/**
 * @brief   插入一条交易记录
 * 该函数用于将一条新的交易记录插入到卡片中。
 * @param   record  要插入的交易记录字符串，类型为 QString
 * @param   cardId  卡片ID，类型为 QString
 * @return  bool    是否插入成功
 * - true   插入成功
 * - false  插入失败
 * @details
 * 函数首先读取卡片的当前记录数量和记录索引。如果读取失败，函数将返回 `false`。
 * 然后，函数更新记录数量和记录索引，并将新的交易记录转换为 `uchar_t` 类型的字符数组。
 * 最后，函数调用 `writeBlocks` 将记录写入卡片。如果写入行数不为0，则表示插入成功，函数返回 `true`；否则，函数返回 `false`。
 * @author  柯劲帆
 * @date    2024-07-31
 */
bool Reader::insertRecord(QString record, QString cardId)
{
    int recordNum, recordIndex;
    bool success = readRecordNumber(recordNum, recordIndex, cardId);
    if (!success) return false;
    recordNum = std::min(maxRecordNum, ++recordNum);
    recordIndex = (recordIndex + 1) % maxRecordNum;
    success = writeRecordNumber(recordNum, recordIndex, cardId);
    if (!success) return false;

    int blockIndex = 1 + 4 * recordIndex;

    uchar_t cardIdHex[8] = {0};
    QByteArray ba = cardId.toLatin1();
    StringToHex(ba.data(), cardIdHex);

    uchar_t recordHex[4 * 4] = {0};
    ba = record.toLatin1();
    StringToHex(ba.data(), recordHex);

    int writeLineNumber = writeBlocks(blockIndex, 4, recordHex, cardIdHex);

    return writeLineNumber != 0;
}


/**
 * @brief   获取全部记录
 * 该函数用于从卡片中获取所有记录。
 * @param   cardId  要读取记录的卡片ID，类型为 QString
 * @param   ok      操作是否成功的标志，类型为 bool 的引用
 * @return  QStringList  包含所有记录的字符串列表
 * @details
 * 函数首先读取卡片的记录数量和起始索引。如果读取失败，函数将设置 `ok` 为 `false` 并返回空列表。
 * 然后，函数将卡片ID转换为 `uchar_t` 类型的字符数组，并逐个读取记录块。
 * 每条记录由4个block组成，每个block包含8个hex字符。函数将每个block的内容转换为字符串并拼接成完整的记录字符串。
 * 最终，函数将所有记录字符串添加到 `QStringList` 中并返回。
 * @author  柯劲帆
 * @date    2024-07-31
 */
QStringList Reader::getRecords(QString cardId, bool &ok)
{
    QStringList recordList;

    int recordNum, recordStartIndex;
    bool success = readRecordNumber(recordNum, recordStartIndex, cardId);
    if (!success)
    {
        ok = false;
        return recordList;
    }

    uchar_t cardIdHex[8] = {0};
    QByteArray ba = cardId.toLatin1();
    StringToHex(ba.data(), cardIdHex);

    for (int i = 0; i < recordNum; i++)
    {
        QString recordStr = "";
        int recordIndex = 1 + 4 * ((recordStartIndex + i) % maxRecordNum);
        for (int j = 0; j < 4; j++)
        {
            uchar_t blockHex[4] = {0};    // 一个block有4个byte，1个byte有两个hex，会返回8个hex存在4个uchar_t中
            int hexNum = readBlocks(recordIndex + j, 1, blockHex, nullptr, cardIdHex);
            if (hexNum == 0)
            {
                ok = false;
                return recordList;
            }

            char blockStr[9] = {0};
            HexToString(blockHex, 4, blockStr);

            recordStr += QString(blockStr);
        }
        recordList.push_back(recordStr);
    }
    return recordList;
}


/**
 * @brief   卡初始化
 * 将第1个block初始化为全0。
 * @param   cardId  要初始化的卡片ID，类型为 QString
 * @return  bool    是否初始化成功
 * - true   初始化成功
 * - false  初始化失败
 * @details
 * 函数首先将传入的 `QString` 类型的卡片ID转换为 `uchar_t` 类型的字符数组。
 * 然后，函数定义一个全为0的字符数组，并调用 `writeBlocks` 函数将其写入卡片的第1个block。
 * 如果 `writeBlocks` 函数返回的写入行数不为0，则表示初始化成功，函数返回 `true`；否则，函数返回 `false`。
 * @author  柯劲帆
 * @date    2024-07-31
 */
bool Reader::initCard(QString cardId)
{
    uchar_t cardIdHex[8] = {0};
    QByteArray ba = cardId.toLatin1();
    StringToHex(ba.data(), cardIdHex);

    uchar_t allZeroHex[4] = {0};
    int writeLineNumber = writeBlock(0, allZeroHex, cardIdHex);
    return writeLineNumber != 0;
}
