#ifndef READERAPI_H
#define READERAPI_H

#include <HF15693.h>
#include <QString>
#include <QStringList>
#include <VCDOurs.h>

typedef unsigned char uchar_t;


/**
 * @class   Reader
 * @brief   读卡器管理类
 *  该类用于管理RFID读卡器的连接和操作，包括设置COM口号、连接读卡器、获取卡片UID列表等功能。
 * @author  柯劲帆
 * @date    2024-07-27
 */
class Reader : private CVCDOurs
{
private:
    int comNumber = -1; ///< 硬件连接com口号，若未连接为-1

public:
    void setComNumber(int comNumber);
    int getComNumber();

    bool is_connected();

    bool connect();
    QStringList inventory(int maxViccNum);
};

#endif // READERAPI_H
