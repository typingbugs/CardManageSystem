#ifndef READERAPI_H
#define READERAPI_H

#include <HF15693.h>


class Reader
{
private:
    int comNumber = -1; // com口号，若未连接为-1

public:
    bool is_connected()
    {
        return comNumber > 0;
    }

    bool connectReader()
    {
        if (t15portOpen(comNumber))
        {
            return true;
        }
        else
        {
            comNumber = -1;
            return false;
        }
    }

    void setComNumber(int comNumber)
    {
        this->comNumber = comNumber;
    }

    int getComNumber()
    {
        return comNumber;
    }
};


#endif // READERAPI_H
