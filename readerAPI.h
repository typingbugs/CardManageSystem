#ifndef READERAPI_H
#define READERAPI_H

#include <HF15693.h>
#include <cstdio>
#include <cstring>
#include <QString>
#include <QStringList>
#include <VCDOurs.h>

typedef unsigned char uchar_t;

class Reader : private CVCDOurs
{
private:
    int comNumber = -1; // com口号，若未连接为-1

public:
    bool is_connected();
    bool connect();
    void setComNumber(int comNumber);
    int getComNumber();
    QStringList inventory(int maxViccNum);
};

#endif // READERAPI_H
