#include <cstdlib>
#include "VCDOurs.h"

CVCDOurs::CVCDOurs(void)
{
	m_bComOpened = false;
}


CVCDOurs::~CVCDOurs(void)
{
	if( m_bComOpened )
	{
		t15portClose();
	}

}

// 使用串口和建立连接
int CVCDOurs::connectReaderByCOM( int comno )
{
	if(t15portOpen(comno))
	{
		t15bSubCarrier=m_ucSubCarrier;
		t15bDataCoding=m_ucDataCoding;
		t15bFullPower=1;
		t15bDataRate=m_ucDataRate;

		m_bComOpened = true;
		if( t15setProtocol() == 0 )
		{
			return 1;
		}
	}
	return 0;
}

void CVCDOurs::disconnectReaderByCOM( )
{
	if( m_bComOpened )
	{
		t15portClose( );
	}
}

int CVCDOurs::inventory( 
        bool bAFIUsed,
        uchar_t ucAFI,
        int nViccNum,				// 应用层的接收的UID数量;
        uchar_t aucDSFID[],
        uchar_t aucUID[][8] )
{
	t15bInventory=1;

	// t15bAddress & t15bSelect在inventory=1的情况下，表示: AFI 和 NBSlots
	// 而NBSlots为16，取值为0.
	t15bAddress=0;		// 本来就没有用
	t15bOption=0;		// 没用用.

	if( bAFIUsed )
	{
		t15bSelect=1;		// 此处的含义是没有AFI，其实可以有！
		sprintf( t15AFI, "%02x", ucAFI );
	}

	char* strUID[16];
	for( int i = 0; i < 16; i++ )
	{
		strUID[i] = new char[50];
	}
	int n = doInventory( strUID );
	n = ( n >= nViccNum ) ? nViccNum : n;
	// 开始解析.
	for( int i = 0; i < n; i++ )
	{
		StringToHex( strUID[i], aucUID[i] );
	}

	for( int i = 0; i < 16; i++ )
	{
		delete []strUID[i];
	}
	return n;
}

// Ours
// 1. 执行inventory时，因为：t15bSelect=0，因此没有AFI flag.
// 2. 没有处理返回的DSFID.
//
int CVCDOurs::inventory( char *pTagUID[] )
{
	t15bInventory=1;
	// t15bAddress & t15bSelect在inventory=1的情况下，表示: AFI 和 NBSlots
	// 而NBSlots为16，取值为0.
	t15bAddress=0;		// 本来就没有用
	t15bSelect=0;		// 此处的含义是没有AFI，其实可以有！
	t15bOption=0;		// 没用用.

	return doInventory( pTagUID );
}

int CVCDOurs::doInventory( char *pTagUID[] )
{
	// 对返回的UID
	// 每个[]，一个UID
	// []表示分配的slot数
	// [z,40]中， 'z'表示发生了冲突
	// 返回的序列是防冲突过程中的全部序列。
	// 
	char cmdsend[CMD_LEN],reply[CMD_LEN]/*="23[,40][2343334239,55][,49][abcderfdsdfs0d,30]"*/;
		
	t15execute(INVENTORY,cmdsend,reply);

	int k = 0;
	char *p,*q,tstr[50]={0};
	p=reply;

	while (1)
	{
		p = strchr(p,'[');

		if (NULL == p)
			break ;

		q = strchr(p+1,',');

		if (NULL == q)
			break ;

		// 'z'表示冲突
		if (q != (p+1) && *(p+1) != 'z' )
		{
			t15changeByteOrder(tstr,p+1,q-p-1);	//返回的数据是低字节在前，调整字节序
			strcpy( pTagUID[k++], tstr );

			// 获取某个Tag的详细信息：call GetSystemInfo().
			setVICCStr( pTagUID[k-1] );
		}

		p = q+1;
	}
	return k;
}

//已经通过UID，获得了VICC的信息.
int CVCDOurs::getSystemInfo(
		uchar_t *pucUID,		
		uchar_t *pinfoFlag,
		uchar_t *pDSFID, uchar_t *pAFI, uchar_t *pBlockNum, uchar_t *pBlockSize, uchar_t *pICRef 
		)
{
	char str[100];

	t15bOption = 0;
	t15bInventory = 0;
	if( pucUID )
	{
		t15bAddress=1;
		t15bSelect = 0;
		HexToString( pucUID, 8, str );
		t15changeByteOrder( t15UID, str, 16 );
		t15UID[16]=0;
	}
	else
	{
		t15bAddress=0;
		t15bSelect =1;
	}

	//执行命令
	char cmdstr[CMD_LEN] = {0},reply[CMD_LEN] = {0};

	if (!t15execute(T15SYSTEM_INFO,cmdstr,reply))
	{	
		return 0;
	}

	// 开始解释数据.
	char *p;
	char tflag[5], iflag, rflag;
	{
		if (! (p = strchr (reply, '[') ) )
			return 0;

		p++;

		if (*p && (*p == ']' || *p == 'z' || *p == 'Z') )
			return 0;

		tflag[0]=*p++;
		tflag[1]=*p++;
		tflag[2]='\0';
		rflag=(char)strtol(tflag,NULL,16);
		tflag[0]=*p++;
		tflag[1]=*p++;
		tflag[2]='\0';

		if( rflag != 0x00)	//Err-Flag is set
		{
			return rflag;
		}
		else
		{
			iflag=(char)strtol(tflag,NULL,16);
			*pinfoFlag = iflag;

			p += 16;
			if( iflag & 0x01 )
			{
				tflag[0]=*p++;
				tflag[1]=*p++;
				tflag[2]='\0';
				*pDSFID =(char)strtol(tflag,NULL,16);
			}
			if( iflag & 0x02 )
			{
				tflag[0]=*p++;
				tflag[1]=*p++;
				tflag[2]='\0';
				*pAFI =(char)strtol(tflag,NULL,16);
			}
			if( iflag & 0x04 )
			{
				tflag[0]=*p++;
				tflag[1]=*p++;
				tflag[2]='\0';
				*pBlockNum =(char)strtol(tflag,NULL,16);
				tflag[0]=*p++;
				tflag[1]=*p++;
				tflag[2]='\0';
				*pBlockSize = ((char)strtol(tflag,NULL,16))&0x1F;
			}
			if( iflag & 0x08 )
			{
				tflag[0]=*p++;
				tflag[1]=*p++;
				tflag[2]='\0';
				*pICRef = ((char)strtol(tflag,NULL,16))&0x1F;
			}
		}
	}
	return rflag;
}


// VCD不处理一些参数的合法性判断，比如nFirstBlock等
// 因为：1. VCD没有必要保留；2.射频场中可能存在不同的VICC，从而导致参数不同;
//      3. 不要说VCD知道每个UID的参数，所以判断。虽然可以，但不合理，即使pucUID=NULL，即处理SELECT模式的VICC时，虽然
//         VCD的确知道是哪个VICC。这个任务应该交给VICC，VICC会返回错误通知用户；
// 用户可以在inventory后，使用get system info获取需要的信息;

int CVCDOurs::readBlocks( int nFirstBlock, int nBlockNum, uchar_t *pDat, uchar_t *pSecurity, uchar_t *pucUID  )
{
// m_nBlockNum想表示 VICC 的可用block数。
// 可以不做这种判断，毕竟，m_nBlockNum不是VCD的参数，而是VICC的属性，应该由VICC判断并返回.
	char str[100];

	t15bOption = 0;
	t15bInventory = 0;
	if( pucUID )
	{
		t15bSelect = 0;
		t15bAddress=1;
		HexToString( pucUID, 8, str );
		t15changeByteOrder( t15UID, str, 16 );
		t15UID[16]=0;
	}
	else
	{
		t15bSelect = 1;
		t15bAddress =0;
	}

	sprintf( t15FirstBN, "%02d", nFirstBlock );
	sprintf( t15NumBl, "%02d", nBlockNum - 1 );

	//执行命令
	char cmdstr[CMD_LEN] = {0},reply[CMD_LEN] = {0};
	
	int cmdno = ( nBlockNum == 1 )?READ_SB:READ_MB;
	if (!t15execute(cmdno,cmdstr,reply))
	{	
		return false;
	}

	//处理返回数据
	char *p,*q,tstr[CMD_LEN];
	p=strchr(reply,'[')+1;

	if(NULL == p)
		return 0;

	q=strchr(p,']');

	if(NULL == q) 
		return 0;

	*q=0;
	
//	if('z' == *p ) 
	// 2023/7/4，修改:
	// 返回的reply，出现过如下的值: “[]”
	// 此时*q=0,导致']'被0替换；
	// 后续代码 tf[0/1] = *p++，导致 p > q;
	// 再调用t15changeByteOrder()时，q-p = -2 < 0;
	// 而：t15changeByteOrder()函数定义如下：
/*
HF15693_API void  t15changeByteOrder(char *pout, const char *pin,int lenbytes)
{
    int i;
    for (i = 0; i < lenbytes; i += 2)
    {
        pout[lenbytes-2-i] = pin[i];
        pout[lenbytes-1-i] = pin[i+1];
    }
    pout[lenbytes] = 0;
}
*/
	// 在 lenbytes =-2 < 0的情况下，直接访问了pout[-2]，导致程序直接崩溃！

	// 修改如下：
	if('z' == *p || p == q ) 
	{
		return 0;
	}
	// 同时放开该判断！
	if(2 < q-p)
	{
		char tf[3];
		tf[0]=*p++;
		tf[1]=*p++;
		tf[2]=0;
		int tfi=strtoul(tf,NULL,10);

		// 因为代码：*q = 0，导致tf是空串；
		// 因此tfi=0, 所以下述语句起不到判断作用；
		if(OP_RESPONSE_FLAG_ERROR & tfi)	//检查标志位，是否发生错误
			return 0;

		if(READ_SB == cmdno)
		{
			t15changeByteOrder(tstr,p,q-p);
		}
		else
		{
			int i=strtoul(t15NumBl,NULL,10)+1;
			t15changeByteOrder_MB(tstr,p,i);
		}

		StringToHex( tstr, pDat );
		return strlen( tstr )/2;
	}
	return 0;
}

int CVCDOurs::getValueFromChar( char c )
{
	if( c >= '0' && c <= '9' )
	{
		return c-'0';
	}
	else if( c >= 'a' && c <= 'f' )
	{
		return c-'a' + 10;
	}
	else if( c >= 'A' && c <= 'F' )
	{
		return c - 'A' + 10;
	}
	else
	{
		return -1;
	}
}
// 返回UID的字节长度
// 如果不是偶数，把str最后一个字符丢弃
int CVCDOurs::StringToHex( char *str, uchar_t aucUID[] )
{
	int k = 0;
	int t1, t2;

	int len = strlen( str )/2;
	for( int i = 0; i < len; i++ )
	{
		t1 = getValueFromChar(str[k++]);
		t2 = getValueFromChar(str[k++]);
		aucUID[i] = ( t1 << 4 ) + t2;
	}

	return len;
}
void CVCDOurs::HexToString( uchar_t aucDat[], int len, char *str )
{
	int k = 0;
	int t;
	for( int i = 0; i < len; i++ )
	{
		t = aucDat[i] >> 4;
		if( t <= 9 )
		{
			str[k++] = t + '0';
		}
		else
		{
			str[k++] = t - 10 + 'A';
		}

		t = aucDat[i] & 0x0F;
		if( t <= 9 )
		{
			str[k++] = t + '0';
		}
		else
		{
			str[k++] = t - 10 + 'A';
		}
	}
	str[k] = '\0';
}
int CVCDOurs::writeBlocks( int nFirstBlock, int nBlockNum, uchar_t *pDat, uchar_t *pucUID )
{
	int k = 0;
	for( int i = 0; i < nBlockNum; i++ )
	{
		k += writeBlock( nFirstBlock + i, pDat+i*8, pucUID );
	}
	return k;
}
int CVCDOurs::writeBlock( int nFirstBlock, uchar_t *pDat, uchar_t *pucUID )
{
	char str[100];

	t15bOption = 0;
	t15bInventory = 0;
	t15bSelect = 0;
	if( pucUID )
	{
		t15bAddress = 1;
		HexToString( pucUID, 8, str );
		t15changeByteOrder( t15UID, str, 16 );
		t15UID[16]=0;
	}
	else
	{
		t15bAddress = 0;
	}

	sprintf( t15FirstBN, "%02d", nFirstBlock );
	sprintf( t15NumBl, "00" );

	// 此处4是blocksize.
	HexToString( pDat, 4, str );
	t15changeByteOrder( t15Data, str, 8 );
	t15Data[8]=0;
	strcpy( str, t15Data );

	char cmdstr[CMD_LEN] = {0}, reply[CMD_LEN] = {0};

	if (!t15execute(WRITE_SB,cmdstr,reply))
    {
//		MessageBox("执行错误,可能参数错误", 0, MB_ICONSTOP);
		return 0 ;
	}

	char *p,*e;

	if (! (p = strchr (reply, '[') ) )
		return 0;

	p++;

	//if (*p && (*p == ']' || *p == 'z' || *p == 'Z') )	return 2;
	if (! (e = strchr(p, ']')))
		return 0;

	if (e == p)
		return 0;

//	if( 2 <= e-p)
	{
		char tf[3];
		tf[0] = *p++;
		tf[1] = *p++;
		tf[2] = '\0';

		char tfi=(char)strtol(tf,NULL,16);//

		if (OP_RESPONSE_FLAG_ERROR & tfi)
			return 0;
		else
			return 1;
	}
}
