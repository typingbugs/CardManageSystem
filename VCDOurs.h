//#pragma once

#ifndef _VCD_OURS_H_
#define _VCD_OURS_H_

#include "VCD.h"

#include "HF15693.h"

class CVCDOurs :
	public CVCD
{
public:
	CVCDOurs(void);
	~CVCDOurs(void);

public:
	virtual int connectReaderByCOM( int comno );
	virtual void disconnectReaderByCOM( );
public:
	int inventory( char *pTagUID[] );

	// 执行inventory!
	// ours reader: 支持AFI
	// 但：不返回DSFID.
	// 
	virtual int inventory( 
        bool bAFIUsed,
        uchar_t ucAFI,
        int nViccNum,				// 应用层的接收的UID数量;
        uchar_t aucDSFID[],
        uchar_t aucUID[][8] );

	// 通过该命令，或者VICC信息
	// 
	virtual int getSystemInfo(
		uchar_t *pucUID,		
		uchar_t *pinfoFlag,
		uchar_t *pDSFID, uchar_t *pAFI, uchar_t *pBlockNum, uchar_t *pBlockSize, uchar_t *pICRef 
		);
	//------------------------------------------
	// 操作
	// 操作类命令，寻址模式只能：地址，或者select
	// 

	// 0x20
	virtual int readSingleBlock( 
		uchar_t *pucUID,		// NULL表示select模式
		uchar_t optional_flag,	// optional bit = 1, 要求返回block的安全状态
		uchar_t ucBlkno,		// blockno
		uchar_t buf[],			// 返回读取的数据
		uchar_t *pucSecurity	// 如果optional_flag = 1, 返回该block的安全状态：1表示locked.
	)
	{
		// 没有处理optional-flag
		// 
		t15bOption = optional_flag;
		return readBlocks( ucBlkno, 1, buf, pucSecurity, pucUID );
	}

	// 0x21
	virtual int writeSingleBlock( 
		uchar_t *pucUID,		// NULL表示select模式
		uchar_t ucBlkno,			// 
		uchar_t ucBlksize,
		uchar_t buf[] )
	{
		return writeBlocks( ucBlkno, 1, buf, pucUID );
	}
	// 0x23
	virtual int readMultipleBlocks(
		uchar_t *pucUID,		// NULL表示select模式
		uchar_t optional_flag,	// optional bit = 1, 要求返回block的安全状态
        uchar_t ucBlkno,		// 开始block no
        uchar_t blocknum,		// 实际读的block是：blocknum+1;
		uchar_t buf[],			// 返回读取的数据
		uchar_t aucSecurity[]	// 如果optional_flag = 1, 返回该block的安全状态：1表示locked.
		)
    {
		t15bOption = optional_flag;
		return readBlocks( ucBlkno, blocknum+1, buf, aucSecurity, pucUID );
	}

	// 0x24
	virtual int writeMultipleBlocks( 
		uchar_t *pucUID,		// NULL表示select模式
		uchar_t ucBlkno,		// 开始block no
		uchar_t blocknum,		// 实际写的block是：blocknum+1;
		uchar_t ucBlksize,
		uchar_t buf[] )
	{
		return writeBlocks( ucBlkno, blocknum, buf, pucUID );
	}
	// 返回UID的长度
	int StringToHex( char *str, uchar_t aucUID[] );
	bool opened( )
	{
		return m_bComOpened;
	}

protected:
	int doInventory( char *pTagUID[] );
	// CMD: 0x2B
	// 
protected:
	void HexToString( uchar_t aucDat[], int len, char *str );
	int getValueFromChar( char c );

	// 数据保存方式：
	// block从 L->H
	// 每个block中，从byte 0 - byte N-1
	// 返回：<0 失败
	// block序号，从0开始.
	// pTagUID = NULL，表示采用select模式。
	// nBlockNum: 1...
	int readBlocks( int nFirstBlock, int nBlockNum, uchar_t *pDat, uchar_t *pSecurity, uchar_t *pucTagUID = NULL );
	int writeBlocks( int nFirstBlock, int nBlockNum, uchar_t *pDat,uchar_t *pucTagUID = NULL );

	// nxp dosn't support write multiblocks;
	int writeBlock( int nFirstBlock, uchar_t *pDat,uchar_t *pucTagUID = NULL );


	int getVICCstr( char *szUID )
	{
		uchar_t aucUID[8];
		if( StringToHex( szUID, aucUID ))
		{
			return getVICC( aucUID );
		}
		return -1;
	}

	bool setVICCStr( char *szUID )
	{
		uchar_t aucUID[8];
		if( StringToHex( szUID, aucUID ))
		{
			return setVICC( aucUID );
		}
		return false;
	}
protected:
	bool m_bComOpened;
};

#endif;
