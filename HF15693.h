// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 HF15693_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// HF15693_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifndef __HF15693_H__
#define __HF15693_H__

#ifdef HF15693_EXPORTS
#define HF15693_API __declspec(dllexport)
#else
#define HF15693_API __declspec(dllimport)
#endif


#define OP_RESPONSE_FLAG_ERROR		0x01
#define OP_RESPONSE_FLAG_EXTENSION	0x08

#define	DATA_LEN	256
#define MAX_UID		32
#define UID_LEN		68
#define	DSFID_LEN	4
#define	CMD_LEN		2048

// 15693命令代码
#define	CC_INVENTORY	"01"
#define	CC_QUIET		"02"
#define	CC_READ_SB		"20"
#define	CC_WRITE_SB		"21"
#define	CC_LOCK			"22"
#define	CC_READ_MP		"23"
#define	CC_WRITE_MP		"24"
#define	CC_SELECT		"25"
#define	CC_READY		"26"
#define	CC_WRITE_API	"27"
#define	CC_LOCK_API		"28"
#define	CC_WRITE_DSFID	"29"
#define	CC_LOCK_DSFID	"2A"
#define	CC_GET_INFO		"2B"
#define	CC_GET_SEC_STAT	"2C"

//命令字
#define	INVENTORY		0
#define	READ_SB			1
#define	WRITE_SB		2
#define	LOCK_B			3
#define	READ_MB			4
#define	WRITE_MB		5
#define	QUIET			6
#define	SELECT			7
#define	READY			8
#define	WRITE_AFI		9
#define	LOCK_AFI		10
#define	WRITE_DSFID		11
#define	LOCK_DSFID		12
#define	T15SYSTEM_INFO	13
#define	MBS_STATUS		14
#define	PROTOCOL		15
#define	FLAG			16
#define	SET				17

#ifdef __cplusplus
extern "C" {
#endif

extern HF15693_API int  t15bDataCoding /*数据编码:0 - 1/4; 1 - 1/256*/, t15bFullPower /*全功率*/, t15bSubCarrier /*副载波*/,
						t15bDataRate /*高数据率*/, 
						t15bInventory /*盘点标志*/, t15bSelect /*选择标志/AFI标志*/, t15bAddress /*地址标志/槽数据标志*/, 
						t15bOption /*可选标志*/;

extern HF15693_API char t15UID[UID_LEN], t15FirstBN[3], t15NumBl[3], t15Data[DATA_LEN], 
						t15DSFID[DSFID_LEN], t15AFI[5];

// 与字节序相关的函数，在函数内部不处理，需使用者自行处理

//////////////////////////////
//工具函数
/*********************************************
/* 描述:打开串口
/* 输入:串口号，如若打开com1，则参数为 1
/* 输出:无
/* 返回:true 打开成功；false 打开失败
/* 其他:
/*********************************************/
HF15693_API bool t15portOpen(int comno);

/*********************************************
/* 描述:关闭串口
/* 输入:无
/* 输出:无
/* 返回:true 关闭成功；false 关闭失败
/* 其他:
/*********************************************/
HF15693_API bool  t15portClose();

/*********************************************
/* 描述:设置协议
/* 输入:无
/* 输出:无
/* 返回: 0 设置成功； ~0 设置失败
/* 其他:
/*********************************************/
HF15693_API int t15setProtocol();

/*********************************************
/* 描述:执行命令
/* 输入:cmdno - 命令字
/* 输出:cmdstr - 向下发出的命令字段；reply - 对命令回复的字段
/* 返回:(int) 命令回复字段中的有效字节数
/* 其他:
/*********************************************/
HF15693_API int t15execute(int cmdno, char *cmdstr,char *reply);

/////////////////////////////////////

/*********************************************
/* 描述:从一个字串中提取一个表示十六进制数字的子串，子串间以',' '[' 分隔
/* 输入: str - 原字串
/* 输出: str - 指向原串中提取到的子串的下一下字符， word - 提取到的子串
/* 返回: 0 - 成功 ， 1 - 失败
/* 其他:
/*********************************************/
HF15693_API int		getXWord (char **str, char *word);

/*********************************************
/* 描述:从一个字串中提取一个表示十六进制数字的子串，子串间以',' '[' 分隔
/* 输入: str - 原字串
/* 输出: word - 提取到的子串
/* 返回: 
/* 其他:
/*********************************************/
HF15693_API int		getXWord2 ( char *str, char *word);

/*********************************************
/* 描述:更改字节序
/* 输入:pin - 原字串 ， lenbytes - 字串数据长度
/* 输出:pout - 更改字节序后的字串
/* 返回:无
/* 其他:此函数会在pout后补0，使用者须保证其长度。
/*       如，字串"123456" 更改字节序后变为 "563412"
/*********************************************/
HF15693_API void	t15changeByteOrder(char *pout,const char *pin,int lenbytes);

/*********************************************
/* 描述:更改带块安全状态信息(block security status)的字串的字节序
/* 输入:pin - 原字串 ， lenbytes - 字串数据长度，不包含'块安全状态信息'的长度
/* 输出:sbss - 原字串中的bss值 ，pout - 更改字节序后的字串
/* 返回:无
/* 其他:字串的存储空间由使用者保证，会在pout后补0；sbss是用字串表示的一个字节的十六进制数
/*********************************************/
HF15693_API void	t15changeByteOrder2(char *sbss,char *pout, char *pin,int lenbytes);

/*********************************************
/* 描述:更改字串的字节序,字串包含多个块的数据
/* 输入:pin - 原字串 ， numblocks - 原字串中包含的块数目
/* 输出:pout - 更改字节序后的字串
/* 返回:无
/* 其他:每个块数据长度由pin长度和numblocks决定；会在pout后补0
/*********************************************/
HF15693_API void	t15changeByteOrder_MB(char *pout, char *pin, int numblocks);

/*********************************************
/* 描述:更改带块安全状态信息(block security status)的字串的字节序,字串包含多个块的数据
/* 输入:pin - 原字串;lenbytes - 字串数据长度，不包含'块安全状态信息'的长度; numblocks - 原字串中包含的块数目
/* 输出:mbss - 原字串中的bss值 ，pout - 更改字节序后的字串
/* 返回:无
/* 其他:各个块的bss依次存放于mbss中，多个块的数据依次存放于pout中
/*********************************************/
HF15693_API void	t15changeByteOrder_MB2(char *mbss,char *pout, char *pin,int lenbytes, int numblocks);

/*********************************************
/* 描述:解析返回的RSSI值
/* 输入:字串表示的RSSI
/* 输出:无
/* 返回:指向解析过的RSSI信息的内存块
/* 其他:返回的指针指向的内存由函数分配管理
/*********************************************/
HF15693_API char*	t15hex2rssi (char *str);

/*********************************************
/* 描述:解析标签信息字串
/* 输入:data - 原标签信息字串；flag - 标签信息标志
/* 输出:(其他参数)
/* 返回:0
/* 其他:
/*********************************************/
HF15693_API int		t15getTagSysInfo (char *data, const unsigned char flag,char *uid, char *dsfid, char *afi, 
									char *blSize, char *numBl, char *ic);

#ifdef __cplusplus
}
#endif

#endif
