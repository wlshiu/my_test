#ifndef __ZNFAT_H__
#define __ZNFAT_H__


#include <stdint.h>
#include "config.h"     //znFAT的配置文件
#include "cc_macro.h"   //此头文件中定义了znFAT中各种函数之间的以来关系, 加之编译控制可实现功能函数的裁减, 可减少程序存储器的使用量

#define MBR_SECTOR      (0) //MBR扇区
#define DBR_MARK        {0XEB, 0X58, 0X90} //DBR的标志码

#define FDI_NBYTES      (32) //文件目录项所占字节数
#define NFDI_PER_SEC    (16) //每扇区的文件目录项数

#define ZNFAT_BUF_SIZE  (512) //znFAT内部缓冲区大小

#define SOC(c)          (((c-2)*(pInit_Args->SectorsPerClust))+(pInit_Args->FirstDirSector)) //计算簇的开始扇区 Start Sector Of Cluster

#define NFATITEMBYTES   (4)    //FAT表项所占用的字节数
#define NITEMSINFATSEC  (128)  //FAT表一个扇区中包含的表项数

#define IS_END_CLU(cluster)                 ((0X0FFFFFFF)==(cluster)) //判断一个簇项的值是否是结束簇
#define IS_END_SEC_OF_CLU(sec, cluster)     ((sec-SOC(cluster))==(pInit_Args->SectorsPerClust-1)) //判断是否是簇的最后一个扇区
#define LAST_SEC_OF_CLU(cluster)            (SOC(cluster)+(pInit_Args->SectorsPerClust-1)) //簇的最后一个扇区的地址

#define IS_FAT32_TYPE(FST)                  (('F'==(FST[0])) && \
                                             ('A'==(FST[1])) && \
                                             ('T'==(FST[2])) && \
                                             ('3'==(FST[3])) && \
                                             ('2'==(FST[4]))) //检验文件系统是否FAT32

#define CHK_ATTR_FILE(attr)             ((!((attr & 0x10) != 0X00)) && \
                                         (0x0F != attr) &&           \
                                         (0x00 != attr) &&           \
                                         (0x08 != attr)) //属性字节第4位为0, 同时不是长名属性0X0F, 空项或卷标

#define CHK_ATTR_DIR(attr)              ((attr & 0x10) != 0X00) //属性字节第4位为1, 则视其为目录
#define CHK_ATTR_LFN(attr)              (0x0F == attr) //属性字节为0X0F, 则为长名项

#define Lower2Up(c)                     ((c >= 'a' && c<='z') ? (c-32) : c)
#define Upper2Low(C)                    ((C >= 'A' && C<='Z') ? (C+32) : C)
#define WLower2Up(c)                    ((c >= (uint16_t)'a' && c <= (uint16_t)'z') ? (c-32) : c)
#define WUpper2Low(C)                   ((C >= (uint16_t)'A' && C <= (uint16_t)'Z') ? (C+32) : C)
#define IS_ASC(c)                       ((c & 0x80) == 0)

#define MAKE_TIME(h,m,s)                ((((uint16_t)h)<<11) + (((uint16_t)m)<<5) + (((uint16_t)s)>>1))   //按时间的位段定义合成时间字
#define MAKE_DATE(y,m,d)                (((((uint16_t)(y%100)) +20)<<9) + (((uint16_t)m)<<5) + ((uint16_t)d))  //按日期的位段定义合成日期字

#define BOOL_TRUE       (1)
#define BOOL_FALSE      (0)
#define NUL_RET         (0)
#define NUL_PTR         ((void *)0)

//=====macro for format function========
#define NSECPERTRACK        (63)
#define NHEADER             (255)
#define NSECPERCYLINDER     (((uint32_t)NSECPERTRACK)*((uint32_t)NHEADER))
//==============================================

//==========================================================================================================================
//以下函数用于从FLASHROM中读取相应类型的数据

#define GET_PGM_BYTE(u)    (PGM_BYTE_FUN(u))
#define GET_PGM_WORD(u)    (PGM_WORD_FUN(u))
#define GET_PGM_DWORD(u)   (PGM_DWORD_FUN(u))

//===========================================================================================================================

//=================ERR code====错误码==========
#define ERR_SUCC                  (0)
#define ERR_FAIL                  (1)

#define FSTYPE_NOT_FAT32          (2)

#define ERR_NO_FILE               (3)
#define ERR_NO_DIR                (4)

#define ERR_FDI_ALREADY_EXISTING  (5)
#define ERR_FDI_NO_SPARE_SPACE    (6)

#define ERR_NO_SPACE              (7)

#define ERR_ILL_CHAR              (8)

#define ERR_SFN_ILL_LEN           (9)
#define ERR_SFN_DOT               (10)

#define ERR_SFN_SPEC_CHAR         (11)
#define ERR_SFN_ILL_LOWER         (12)

#define ERR_DIR_ALREADY_EXISTING  (13)

#define ERR_FMT_TOO_LOW_VOLUME    (14)

#define ERR_LFN_BUF_OUT           (15)
#define ERR_OEM_CHAR_NOT_COMPLETE (16)

#define ERR_FS_DIR                (17)

#define ERR_DEVICE_IO             (18)

#define ERR_MD_POS_OVER_FSIZE     ((uint32_t)(-3))

#define ERR_OVER_FILE_MAX_SIZE    ((uint32_t)(-2))

#define ERR_OVER_DISK_SPACE       ((uint32_t)(-1))

//================================================

//DPT:分区表记录结构如下
struct DPT_Item
{
    uint8_t Active;         //0x80表示此分区有效
    uint8_t StartHead;      //分区的开始磁头
    uint8_t StartSect;      //开始扇区
    uint8_t StartCyl;       //开始柱面
    uint8_t PartType;       //分区类型
    uint8_t EndHead;        //分区的结束头
    uint8_t EndSect;        //结束扇区
    uint8_t EndCyl;         //结束柱面
    uint8_t StartLBA[4];    //分区的第一个扇区
    uint8_t Size[4];        //分区的大小, 总扇区数
};

//MBR扇区(绝对0扇区)定义如下
struct MBR
{
    uint8_t PartCode[446]; //MBR的引导程序
    struct DPT_Item Part[4]; //4个分区记录
    uint8_t BootSectSig0;  //55
    uint8_t BootSectSig1;  //AA
};

//znFAT中对DBR的定义如下  一共占用90个字节
struct DBR
{
    uint8_t BS_jmpBoot[3];     //跳转指令            offset: 0
    uint8_t BS_OEMName[8];     //OEM名称             offset: 3

    uint8_t BPB_BytesPerSec[2];//每扇区字节数        offset:11
    uint8_t BPB_SecPerClus;    //每簇扇区数          offset:13
    uint8_t BPB_RsvdSecCnt[2]; //保留扇区数目        offset:14
    uint8_t BPB_NumFATs;       //此卷中FAT表数       offset:16
    uint8_t BPB_RootEntCnt[2]; //FAT32为0            offset:17
    uint8_t BPB_TotSec16[2];   //FAT32为0            offset:19
    uint8_t BPB_Media;         //存储介质            offset:21
    uint8_t BPB_FATSz16[2];    //FAT32为0            offset:22
    uint8_t BPB_SecPerTrk[2];  //磁道扇区数          offset:24
    uint8_t BPB_NumHeads[2];   //磁头数              offset:26
    uint8_t BPB_HiddSec[4];    //FAT区前隐扇区数     offset:28
    uint8_t BPB_TotSec32[4];   //该卷总扇区数        offset:32
    uint8_t BPB_FATSz32[4];    //一个FAT表扇区数     offset:36
    uint8_t BPB_ExtFlags[2];   //FAT32特有           offset:40
    uint8_t BPB_FSVer[2];      //FAT32特有           offset:42
    uint8_t BPB_RootClus[4];   //根目录簇号          offset:44
    uint8_t FSInfo[2];         //保留扇区FSINFO扇区数offset:48
    uint8_t BPB_BkBootSec[2];  //通常为6             offset:50
    uint8_t BPB_Reserved[12];  //扩展用              offset:52
    uint8_t BS_DrvNum;         //                    offset:64
    uint8_t BS_Reserved1;      //                    offset:65
    uint8_t BS_BootSig;        //                    offset:66
    uint8_t BS_VolID[4];       //                    offset:67
    uint8_t BS_FilSysType[11]; //                 offset:71
    uint8_t BS_FilSysType1[8]; //"FAT32    "         offset:82
};

//znFAT中对文件目录项(振南叫它FDI)的定义如下  一共占用32个字节
struct FDI
{
    uint8_t Name[8];            // 文件名, 不足部分以空格补充
    uint8_t Extension[3];       // 扩展名, 不足部分以空格补充
    uint8_t Attributes;         // 文件属性
    uint8_t LowerCase;          // 0
    uint8_t CTime10ms;          // 创建时间的10毫秒位
    uint8_t CTime[2];           // 创建时间
    uint8_t CDate[2];           // 创建日期
    uint8_t ADate[2];           // 访问日期
    uint8_t HighClust[2];       // 开始簇的高字
    uint8_t MTime[2];           // 最近的修改时间
    uint8_t MDate[2];           // 最近的修改日期
    uint8_t LowClust[2];        // 开始簇的低字
    uint8_t FileSize[4];        // 文件大小
};

struct LFN_FDI //长名的文件目录项结构定义
{
    uint8_t AttrByte[1]; //属性字节
    uint8_t Name1[10];   //第一部分长名
    uint8_t LFNSign[1];  //长名项标志
    uint8_t Resv[1];     //保留
    uint8_t ChkVal[1];   //检验值, 与SFN的绑定校验
    uint8_t Name2[12];   //第二部分长名
    uint8_t StartClu[2]; //取0
    uint8_t Name3[4];    //第三部分长名
};

struct FSInfo //znFAT中对文件系统信息结构的定义
{
    uint8_t Head[4];            //"RRaA"
    uint8_t Resv1[480];
    uint8_t Sign[4];            //"rrAa"
    uint8_t Free_Cluster[4];    //剩余空簇数
    uint8_t Next_Free_Cluster[4]; //下一空簇参考值
    uint8_t Resv2[14];
    uint8_t Tail[2];            //"55 AA"
};

struct FDIesInSEC
{
    struct FDI FDIes[NFDI_PER_SEC]; //扇区中的文件目录项数组
};

struct Date  //用于存储日期信息
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
};

struct Time  //用于存储时间信息
{
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
};

struct DateTime //日期与时间
{
    struct Date date; //日期
    struct Time time; //时间
};

struct FAT_Item  //znFAT中对FAT表项的结构定义
{
    uint8_t Item[NFATITEMBYTES]; //FAT32中FAT表项占用4个字节, 即32位
};

struct FAT_Sec  //znFAT中对FAT表扇区结构的定义
{
    struct FAT_Item items[NITEMSINFATSEC]; //FAT扇区包含128个FAT表项
    //FAT扇区的结构就是有128个FAT表项类型的数组
};

#define DATE_YEAR_BASE      (1980)

#define TIME_HOUR_MARK      (0X001F)
#define TIME_MIN_MARK       (0X002F)
#define TIME_SEC_MARK       (0X001F)

#define DATE_YEAR_MARK      (0X007F)
#define DATE_MONTH_MARK     (0X000F)
#define DATE_DAY_MARK       (0X001F)

#define TIME_HOUR_NBITS     (5)
#define TIME_MIN_NBITS      (6)
#define TIME_SEC_NBITS      (5)

#define DATE_YEAR_NBITS     (7)
#define DATE_MONTH_NBITS    (4)
#define DATE_DAY_NBITS      (5)

//==============================================================================

//znFAT初始化时初始参数装入如下结构体中
struct znFAT_Init_Args
{
    uint32_t BPB_Sector_No;   //DBR(BPB)所在扇区号

    uint32_t Total_SizeKB;    //磁盘的总容量, 单位为KB
    uint32_t BytesPerSector;   //每个扇区的字节数
    uint32_t FATsectors;      //FAT表所占扇区数
    uint32_t SectorsPerClust; //每簇的扇区数
    uint32_t FirstFATSector;   //第一个FAT表所在扇区
    uint32_t FirstDirSector;   //第一个目录所在扇区

    uint32_t FSINFO_Sec;      //FSINFO扇区所在的扇区
    uint32_t Free_nCluster;   //空闲簇的个数
    uint32_t Next_Free_Cluster; //下一空簇
};

//znFAT中对文件信息集合的定义
struct FileInfo
{
    int8_t File_Name[13];     //完整文件名（主文件名与扩展文件名）
    int8_t File_Attr;         //文件属性
    struct Time File_CTime;    //文件创建时间
    struct Date File_CDate;    //文件创建日期
    //struct Date File_ADate;    //文件访问日期
    //struct Time File_MTime;   //文件修改时间
    //struct Date File_MDate;   //文件修改日期
    uint32_t File_StartClust;    //文件开始簇
    uint32_t File_Size;           //文件大小

    uint32_t File_CurClust;   //文件当前簇
    uint32_t File_CurSec;     //文件当前扇区
    uint16_t File_CurPos;  //文件当前扇区偏移量

    uint8_t  File_IsEOF;      //文件是否到达结束位置

    uint32_t File_CurOffset;   //文件当前偏移量

    uint32_t FDI_Sec; //文件目录项所在的扇区
    uint8_t  nFDI; //文件目录项在扇区中的索引

#ifdef USE_LFN //如果使用长名, 则文件信息集合包含以下两项的下定义
    uint8_t have_lfn; //表示此文件是否有长名
    uint16_t longname[MAX_LFN_LEN + 1]; //用于装载长名的UNICODE码, 如果实际文件的长名长于MAX_LFN_LEN, 则直接截断
#endif

    //----ACCCB相关变量定义-----
#ifndef RT_UPDATE_CLUSTER_CHAIN
#ifdef USE_ALONE_CCCB
    uint32_t acccb_buf[CCCB_LEN]; //ACCCB的缓冲区, 以连续簇段的方式来记录簇链
    uint8_t  acccb_counter;
    uint32_t acccb_curval;
#endif
#endif
    //----------------------------

#ifdef USE_EXCHANGE_BUFFER
#ifdef USE_ALONE_EXB
    uint8_t exb_buf[ZNFAT_BUF_SIZE];
    uint32_t exb_cursec;
#endif
#endif
};

//以下是对用户可用的函数的声明

uint8_t znFAT_Device_Init(void); //存储设备初始化, 底层驱动接口
uint8_t znFAT_Init(void); //文件系统初始化
uint8_t znFAT_Select_Device(uint8_t devno, struct znFAT_Init_Args *pinitargs); //在多设备情况下, 用于选择某一个设备

uint32_t znFAT_ReadData(struct FileInfo *pFI, uint32_t offset, uint32_t len, uint8_t *app_Buffer); //数据读取
uint32_t znFAT_ReadDataX(struct FileInfo *pfi, uint32_t offset, uint32_t len);
uint8_t znFAT_Seek(struct FileInfo *pFI, uint32_t offset); //文件定位
uint8_t znFAT_Open_File(struct FileInfo *pFI, int8_t *filepath, uint32_t n, uint8_t is_file); //文件打开
uint8_t znFAT_Enter_Dir(int8_t *dirpath, uint32_t *pCluster, uint32_t *pos); //进入目录
uint8_t znFAT_Create_File(struct FileInfo *pfi, int8_t *pfn, struct DateTime *pdt); //创建文件
uint8_t znFAT_Create_Dir(int8_t *pdp, struct DateTime *pdt); //创建目录
uint8_t znFAT_Delete_Dir(int8_t *dirpath); //删除目录
uint8_t znFAT_Delete_File(int8_t *filepath); //删除文件
uint8_t znFAT_Make_FS(uint32_t tt_sec, uint16_t clu_sz); //格式化

uint32_t znFAT_WriteData(struct FileInfo *pfi, uint32_t len, uint8_t *pbuf); //写入数据
uint32_t znFAT_Modify_Data(struct FileInfo *pfi, uint32_t offset, uint32_t len, uint8_t *app_Buffer); //对文件数据进行修改
uint8_t znFAT_Dump_Data(struct FileInfo *pfi, uint32_t offset); //截断文件数据

uint8_t znFAT_Close_File(struct FileInfo *pfi); //关闭文件, 如果程序中没有打开实时文件大小更新, 则文件操作完后, 尤其是写入和更改操作, 必须调用此函数
uint8_t znFAT_Flush_FS(void); //刷新文件系统相关信息, 如果程序中没有打开实时文件系统信息更新, 则在程序中一定要调用此函数, 否则将导致文件系统相关参数错误
//======================

#endif
