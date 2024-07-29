#include "readerAPI.h"


bool Reader::is_connected()
{
    return comNumber > 0;
}


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

void Reader::setComNumber(int comNumber)
{
    this->comNumber = comNumber;
}

int Reader::getComNumber()
{
    return comNumber;
}

QStringList Reader::inventory(int maxViccNum)
{
    uchar_t (*aucUID)[8] = (uchar_t (*)[8])malloc(maxViccNum * sizeof(*aucUID));;
    int receivedViccNum = CVCDOurs::inventory(false, '\0', maxViccNum, nullptr, aucUID);
    QStringList uidList;
    for (int i = 0; i < receivedViccNum; ++i)
    {
        char uidStr[8];
        CVCDOurs::HexToString(aucUID[i], 8, uidStr);
        uidList.push_back(QString::fromStdString(uidStr));
    }
    free(aucUID);
    return uidList;
}
