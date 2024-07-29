//-----------------------------------------
// 关于类说明
// 类 CVCD 用于 描述 VCD
// 应用软件调用类的成员函数，可以对VICC进行访问
// 
// 派生类：CVCD_Sim 用软件仿真VCD，通过transponderEnter函数仿真VICC进入了射频场
// class CVCD_Sim : public CVCD
// 
// 派生类：CVCD_Ours 封装了Ours的reader.
// class CVCD_Ours : public CVCD
// 
// 
//#pragma once

#ifndef _VCD_H_
#define _VCD_H_

#include "HLog.h"
#include <cstring>

// 
// 并部分实现函数的部分功能
// 比如select request，寻址模式一定是：address mode
// 
// 有些命令，可以由用户确定寻址模式
// 00: 所有的卡都应答
// 01: 被选择卡应答
// 02: 特定地址的卡应答
// else: 不支持;
//
//---------------------------------------------------
// 下面定义了reader成员函数的返回结果
// 
// 比如一个addressed的命令，收到多个应答，或者没有应答
// 注意：此处没有应答，不指VICC没有影带，有些场景，VICC不应答，也是正确的
// 比如stay quiet，收不到任何响应，其实是正确的。
// 此处没有应答，指VICC应该应答而没有应答
// 
#define ISO15693_READER_OK				0xA0
#define ISO15693_READER_ER_LESS_ANSWER	0xA1	// 没有应答
#define ISO15693_READER_ER_MORE_ANSWERS	0xA2	// 收到多个应答
#define ISO15693_READER_ER_FRAME		0xA3	// 收到frame格式错，可能发生了冲突／软件表示frame错误.

// 副载波，0: for single subcarrier / 1: for double subcarrier
#define ISO15693_REQ_FLAG_SUB_CARRIER_S	0 
#define ISO15693_REQ_FLAG_SUB_CARRIER_D	1
// 速率,  0: for low data rate / 1: for high data rate
#define ISO15693_REQ_FLAG_DATA_RATE_H 	0
#define ISO15693_REQ_FLAG_DATA_RATE_H 	1
// 编码方式，0: for 1/4 / 1 for 1/256;
#define ISO15693_CODING_1_FROM_4		0
#define ISO15693_CODING_1_FROM_256		1

#define ISO15693_RSP_ERR_NO				0

#define MAX_TRANSPONDERS	16
#define MAX_PKT_LEN			1024

// 寻址模式
// 除inventory命令外，其他都涉及到寻址模式.

#define ISO15693_INVENTORY_NO_VICC		0x00
#define ISO15693_INVENTORY_ONE_VICC		0x01
#define ISO15693_INVENTORY_MORE_VICCS	0x02

// VCD的射频场最多支持的VICC数
// 定义为16个，是因为inventory采用16slots的情况下，最多一次可以清晰收到16个VICC的应答。
// 如果发生了碰撞，或者访问清晰收到的，或者继续寻找碰撞的。
#define MAX_NUM_VICCS						16

typedef unsigned char uchar_t;

typedef enum
{
	mode_all,
	mode_select,
	mode_addressed
}ISO15693_MODE_E;

// VICC的部分信息.
typedef struct tag_VICCInfo
{
	uchar_t aucUID[8];
	uchar_t ucValid;			// 0 for unvalid, 1 for valid.
	uchar_t ucDSFID;
	uchar_t ucAFI;
	uchar_t ucBlockNum;
	uchar_t ucBlockSize;
	uchar_t ucICRef;
}VICCInfo;

class CVCD
{
public:
	CVCD(void)
	{
		m_ucSubCarrier = ISO15693_REQ_FLAG_SUB_CARRIER_D;	// 默认双载波
		m_ucDataRate = ISO15693_REQ_FLAG_DATA_RATE_H;		// 默认高速率
		m_ucDataCoding = ISO15693_CODING_1_FROM_4;
		m_ucSlots = 16;	
		m_pLog = NULL;
		for( int i = 0; i < MAX_NUM_VICCS; i++ )
		{
			m_astVICCs[i].ucValid = 0;
		}
	}
	virtual ~CVCD(void)
	{
	}

public:
	// 正式执行inventory前，设置这些参数
	int set( uchar_t ucSubCarrier, uchar_t ucDataRate, uchar_t ucSlots  )
	{
		m_ucSubCarrier = ucSubCarrier;
		m_ucDataRate = ucDataRate;
		m_ucSlots = ucSlots;
	}
	int setSubCarrier( uchar_t ucSubCarrier )
	{
		m_ucSubCarrier = ucSubCarrier;
	}
	int setDataRate( uchar_t ucDataRate )
	{
		m_ucDataRate = ucDataRate;
	}
	int setSlots( uchar_t ucSlots )
	{
		m_ucSlots = ucSlots;
	}
	void setDataCoding_1from4()
	{
		m_ucDataCoding = ISO15693_CODING_1_FROM_4;
	}
	void setDataCoding_1from256()
	{
		m_ucDataCoding = ISO15693_CODING_1_FROM_256;
	}
	void setLog( CHLog *plog )
	{
		m_pLog = plog;
	}

	// 如何设置和VCD/Reader的连接？
	// 串口.

	virtual int connectReader()
	{
		return 1;
	}
	virtual int disconnectReader( )
	{
		return 1;
	}

/*
	virtual int connectReaderByCOM( int comno )
	{
		return 1;
	}
*/
	// 在执行了inventory后，用户可以获得芯片的信息。
	// 其中获得的blocksize和blocknum在readblocks()和write block()时，有用.
	VICCInfo getVICCInfo( uchar_t aucUID[8] )
	{
		VICCInfo vicc;

		memset( (uchar_t *)&vicc, 0x00, sizeof(vicc)); 
		int k = getVICC( aucUID );
		if( k != -1 )
		{
			return m_astVICCs[k];
		}
		return vicc;
	}
public:
	// 协议命令
	// 返回：>0表示读到的Transponder数量;
	// 
	virtual int inventory( 
        bool bAFIUsed,
        uchar_t ucAFI,
        int nViccNum,				// 应用层可以接收的UID数量;
        uchar_t aucDSFID[],
        uchar_t aucUID[][8] )
	{
		m_bAFIUsed = bAFIUsed;
		if( m_bAFIUsed )
		{
			m_ucAFI = ucAFI;
		}
		return ISO15693_RSP_ERR_NO;
	}

	//------------------------------------------
	// 状态转换
	// stayquiet
	// cmd=0x02;
	// 寻址方式：只能是地址模式, UID必须;
	virtual int stayQuiet( uchar_t *pucUID )
	{
		return ISO15693_RSP_ERR_NO;
	}

	// select命令，
	// cmd=0x25
	// 寻址方式一定是地址模式, UID必须;
	virtual int select( uchar_t *pucUID )
	{
		return ISO15693_RSP_ERR_NO;

	}

	// reset to ready
	// cmd=0x26
	// 可以使地址模式，也可以是选择模式
	virtual int resetToReady( uchar_t *pucUID )
	{ 
		return ISO15693_RSP_ERR_NO;
	}

	//------------------------------------------
	// 操作
	// 操作类命令，寻址模式只能：地址，或者select
	// 

	// 0x20
	virtual int readSingleBlock( 
		uchar_t *pucUID,		// NULL表示select模式
		uchar_t optional_flag,	// optional bit = 1, 要求返回block的安全状态
		uchar_t ucBlkno,			// blockno
		uchar_t buf[],			// 返回读取的数据
		uchar_t *pucSecurity	// 如果optional_flag = 1, 返回该block的安全状态：1表示locked.
	)
	{
		return ISO15693_RSP_ERR_NO;
	}

	// 0x21
	virtual int writeSingleBlock( 
		uchar_t *pucUID,		// NULL表示select模式
		uchar_t ucBlkno,			// 
		uchar_t ucBlksize,
		uchar_t buf[] )
	{
		return ISO15693_RSP_ERR_NO;
	}

	// 0x22
	virtual int lockBlock( 
		uchar_t *pucUID,		// NULL表示select模式
		int blockno )
	{
		return ISO15693_RSP_ERR_NO;
	}

	// 0x23
	virtual int readMultipleBlocks(
		uchar_t *pucUID,		// NULL表示select模式
		uchar_t optional_flag,	// optional bit = 1, 要求返回block的安全状态
		uchar_t ucBlkno,		// 开始block no
		uchar_t blocknum,			// 实际读的block是：blocknum+1;
		uchar_t buf[],			// 返回读取的数据
		uchar_t aucSecurity[]	// 如果optional_flag = 1, 返回该block的安全状态：1表示locked.
		)
	{
		return ISO15693_RSP_ERR_NO;
	}

	// 0x24
	virtual int writeMultipleBlocks( 
		uchar_t *pucUID,		// NULL表示select模式
		uchar_t ucBlkno,		// 开始block no
		uchar_t blocknum,		// 实际写的block是：blocknum+1;
		uchar_t ucBlksize,
		uchar_t buf[] )
	{
		return ISO15693_RSP_ERR_NO;
	}


	//----------------------------------------------------------
	// AFI & DSFID
	// 
	// CMD: 0x27
	// optional 不处理.
	virtual int writeAFI( 
		uchar_t *pucUID,		// NULL表示select模式
		uchar_t ucAFI )
	{
		return ISO15693_RSP_ERR_NO;
	}


	// 
	// CMD: 0x28
	// optional 不处理.
	virtual int lockAFI(
		uchar_t *pucUID		// NULL表示select模式
		)
	{
		return ISO15693_RSP_ERR_NO;
	}


	// 
	// CMD: 0x29
	// optional 不处理.
	virtual int writeDSFID( 
		uchar_t *pucUID,		// NULL表示select模式
		uchar_t ucDSFID )
	{
		return ISO15693_RSP_ERR_NO;
	}


	// 
	// CMD: 0x2A
	// optional 不处理.
	virtual int lockDSFID(
		uchar_t *pucUID			// NULL表示select模式
		)
	{
		return ISO15693_RSP_ERR_NO;
	}

	// CMD: 0x2B
	// 
	virtual int getSystemInfo(
		uchar_t *pucUID,		
		uchar_t *pinfoFlag,
		uchar_t *pDSFID, uchar_t *pAFI, uchar_t *pBlockNum, uchar_t *pBlockSize, uchar_t *pICRef 
		)
	{
		return ISO15693_RSP_ERR_NO;
	}
protected:
	void CRC16( int buflen, uchar_t buf[], uchar_t *pucCRCL, uchar_t *pucCRCH );
	bool ISO15693_checkCRC16( int buflen, uchar_t buf[] );
	void ISO15693_getCRC16( int buflen, uchar_t buf[], uchar_t *pucCRCL, uchar_t *pucCRCH );

	/**
	 *	在inventory后，获取某个VICC的systeminfo
	 *	生成CVICC.
	 *	其中：blocksize & blocknum 用于以后执行read,write命令用。
	 *	输入：pucUID. 8字节的UID.
	 */
	virtual bool setVICC( uchar_t aucUID[8] )
	{
		uchar_t infoFlag;
		uchar_t ucDSFID, ucAFI, ucBlockNum, ucBlockSize;
		uchar_t ucICRef;

		if( getSystemInfo( aucUID, &infoFlag, &ucDSFID, &ucAFI, &ucBlockNum, &ucBlockSize, &ucICRef ) != ISO15693_RSP_ERR_NO )
		{
			return false;
		}

		// OK.
		for( int i = 0; i < MAX_NUM_VICCS; i++ )
		{
			if( m_astVICCs[i].ucValid == 0 )
			{
				m_astVICCs[i].ucValid = 1;
				if( infoFlag & (1 << 0) )
				{
					m_astVICCs[i].ucDSFID = ucDSFID;
				}
				else if( infoFlag & ( 1 << 1 ))
				{
					m_astVICCs[i].ucAFI = ucAFI;
				}
				else if( infoFlag & ( 1 << 2 ) )
				{
					m_astVICCs[i].ucBlockNum = ucBlockNum;
					m_astVICCs[i].ucBlockSize = ucBlockSize;
				}
				else if( infoFlag & ( 1 << 3 ))
				{
					m_astVICCs[i].ucICRef = ucICRef;
				}
				memcpy( m_astVICCs[i].aucUID, aucUID, 8 );
				return true;
			}
		}
		return false;
	}
	int getVICC( uchar_t aucUID[8] )
	{
		for( int i = 0; i < MAX_NUM_VICCS; i++ )
		{
			if( memcmp(m_astVICCs[i].aucUID, aucUID, 8 ) == 0 )
			{
				return i;
			}
		}
		return -1;
	}
protected:
	void log( char *str )
	{
		if( m_pLog )
		{
			m_pLog->logstr( str );
		}
	}

	void logDIM( int len, uchar_t aucDat[] )
	{
		if( m_pLog )
		{
			m_pLog->logDIM( len, aucDat );
		}
	}

protected:
	// 全局性配置
	// VCD自己的选择
	uchar_t	m_ucDataCoding;		// 编码方式，0: for 1/4 / 1 for 1/256;
	uchar_t m_ucSlots;			// 0：16 slots or 1 slots;
	// VCD对VICC的要求
	uchar_t	m_ucSubCarrier;		// 副载波，0: for single subcarrier / 1: for double subcarrier 
	uchar_t	m_ucDataRate;		// 速率,  0: for low data rate / 1: for high data rate

	// 执行防碰撞循环时，可能多次发送inventory request，每个都涉及是否使用AFI
	// 因此设置为成员变量；
	// 
	uchar_t	m_ucAFI;
	bool m_bAFIUsed;

	// 记录日志.
	CHLog*	m_pLog;

	// 通过inventory命令后，发现的viccs.
	VICCInfo m_astVICCs[MAX_NUM_VICCS];
};

#endif
