#ifndef	__GLOBLES_H__
#define	__GLOBLES_H__
/*********************************************************************
*   Filename:       Globles.h
*   Revised:        $Date: 2009-01-08	20:15 (Fri) $
*   Revision:       $
*   Writer:         $Roger-WY
*
*
*   Notes:
*					QQ:261313062
*
*
*********************************************************************/

/*********************************************************************
* INCLUDES
*/

/*********************************************************************
* CONSTANTS
*/
#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef ON
#define ON  0
#endif

#ifndef OFF
#define OFF 1
#endif


/*********************************************************************
* TYPEDEFS
*/
/***********************************************
* 描述：数据类型定义
*/
typedef  unsigned char		    BOOL;
typedef  unsigned char		    BYTE;
typedef  unsigned char          uint8;        	// 无符号8位整型变量
typedef  signed   char  		int8;           // 有符号8位整型变量
typedef  unsigned short		    WORD;
typedef  unsigned short 		uint16;         // 无符号16位整型变量
typedef  signed   short 		int16;          // 有符号16位整型变量
typedef  unsigned long   		uint32;         // 无符号32位整型变量
typedef  signed   long   		int32;          // 有符号32位整型变量
typedef  float          		fp32;           // 单精度浮点数（32位长度）
typedef  double         		fp64;           // 双精度浮点数（64位长度）

#define	    null	            ((void *)0)

#define     CHAR                char
#define     PCHAR               char*
#define     CONST               const

#define	    INT8S		        int8
#define	    INT08S		        int8
#define     pINT8S		        INT8S*
#define	    INT8U		        uint8
#define	    INT08U		        uint8
#define     pINT8U		        INT8U*
#define	    INT16S		        int16
#define     pINT16S		        INT16S*
#define	    INT16U		        uint16
#define	    pINT16U		        INT16U*
#define	    INT32S		        int32
#define	    pINT32S		        INT32S*
#define	    INT32U		        uint32
#define	    pINT32U		        INT32U*
#define     fp32                FP32
#define     fp64                FP64

/* ------------------------------------------------------------------------------------------------
 *                                             Macros
 * ------------------------------------------------------------------------------------------------
 */
/***********************************************
* 描述：常用宏定义
*/
#ifndef BV
#define BV(n)      (1 << (n))
#endif

#ifndef BF
#define BF(x,b,s)  (((x) & (b)) >> (s))
#endif

#ifndef MIN
#define MIN(n,m)   (((n) < (m)) ? (n) : (m))
#endif

#ifndef MAX
#define MAX(n,m)   (((n) < (m)) ? (m) : (n))
#endif

#ifndef ABS
#define ABS(n)     (((n) < 0) ? -(n) : (n))
#endif

#ifndef UBOUND
#define UBOUND(__ARR)    (sizeof(__ARR)/sizeof(__ARR[0]))
#endif

/***********************************************
* 描述：数据组合拆分
*/
/* takes a byte out of a uint32 : var - uint32,  ByteNum - byte to take out (0 - 3) */
#define BREAK_UINT32( var, ByteNum ) \
          (uint8)((uint32)(((var) >>((ByteNum) * 8)) & 0x00FF))

#define BUILD_UINT32(Byte0, Byte1, Byte2, Byte3) \
          ((uint32)((uint32)((Byte0) & 0x00FF)   \
        + ((uint32)((Byte1) & 0x00FF) << 8)      \
        + ((uint32)((Byte2) & 0x00FF) << 16)     \
        + ((uint32)((Byte3) & 0x00FF) << 24)))

#define BUILD_UINT16(loByte, hiByte) \
          ((uint16)(((loByte) & 0x00FF) + (((hiByte) & 0x00FF) << 8)))
#define BUILD_INT16U  BUILD_UINT16

#define HI_UINT16(a) (((a) >> 8) & 0xFF)
#define LO_UINT16(a) ((a) & 0xFF)
#define HI_INT16U   HI_UINT16
#define LO_INT16U   LO_UINT16

#define BUILD_UINT8(hiByte, loByte) \
          ((uint8)(((loByte) & 0x0F) + (((hiByte) & 0x0F) << 4)))
#define BUILD_INT8U  BUILD_UINT8

#define HI_UINT8(a) (((a) >> 4) & 0x0F)
#define LO_UINT8(a) ((a) & 0x0F)
#define HI_INT8U    HI_UINT8
#define LO_INT8U    LO_UINT8

// Write the 32bit value of 'val' in little endian format to the buffer pointed
// to by pBuf, and increment pBuf by 4
#define UINT32_TO_BUF_LITTLE_ENDIAN(pBuf,val) \
	do { \
		*(pBuf)++ = (((val) >>  0) & 0xFF); \
		*(pBuf)++ = (((val) >>  8) & 0xFF); \
		*(pBuf)++ = (((val) >> 16) & 0xFF); \
		*(pBuf)++ = (((val) >> 24) & 0xFF); \
	} while (0)

// Return the 32bit little-endian formatted value pointed to by pBuf, and increment pBuf by 4
#define BUF_TO_UINT32_LITTLE_ENDIAN(pBuf) (((pBuf) += 4), BUILD_UINT32((pBuf)[-4], (pBuf)[-3], (pBuf)[-2], (pBuf)[-1]))


/*
 *  This macro is for use by other macros to form a fully valid C statement.
 *  Without this, the if/else conditionals could show unexpected behavior.
 *
 *  For example, use...
 *    #define SET_REGS()  st( ioreg1 = 0; ioreg2 = 0; )
 *  instead of ...
 *    #define SET_REGS()  { ioreg1 = 0; ioreg2 = 0; }
 *  or
 *    #define  SET_REGS()    ioreg1 = 0; ioreg2 = 0;
 *  The last macro would not behave as expected in the if/else construct.
 *  The second to last macro will cause a compiler error in certain uses
 *  of if/else construct
 *
 *  It is not necessary, or recommended, to use this macro where there is
 *  already a valid C statement.  For example, the following is redundant...
 *    #define CALL_FUNC()   st(  func();  )
 *  This should simply be...
 *    #define CALL_FUNC()   func()
 *
 * (The while condition below evaluates false without generating a
 *  constant-controlling-loop type of warning on most compilers.)
 */
#define st(x)      do { x } while (__LINE__ == -1)



/***********************************************
* 描述： IAR编译优化等级指令，可指定函数或者变量的优化等级
*/
#ifndef __IAR_OPTIMIZE_H__
#define __IAR_OPTIMIZE_H__

#define OPTIMIZE_SIZE_NONE      _Pragma("optimize=z 2")
#define OPTIMIZE_SIZE_LOW       _Pragma("optimize=z 3")
#define OPTIMIZE_SIZE_MEDIUM    _Pragma("optimize=z 6")
#define OPTIMIZE_SIZE_HIGH      _Pragma("optimize=z 9")

#define OPTIMIZE_SPEED_NONE     _Pragma("optimize=s 2")
#define OPTIMIZE_SPEED_LOW      _Pragma("optimize=s 3")
#define OPTIMIZE_SPEED_MEDIUM   _Pragma("optimize=s 6")
#define OPTIMIZE_SPEED_HIGH     _Pragma("optimize=s 9")

#define OZ_2                    OPTIMIZE_SIZE_NONE
#define OZ_3                    OPTIMIZE_SIZE_LOW
#define OZ_6                    OPTIMIZE_SIZE_MEDIUM
#define OZ_9                    OPTIMIZE_SIZE_HIGH

#define OS_2                    OPTIMIZE_SPEED_NONE
#define OS_3                    OPTIMIZE_SPEED_LOW
#define OS_6                    OPTIMIZE_SPEED_MEDIUM
#define OS_9                    OPTIMIZE_SPEED_HIGH

#define OPTIMIZE_NONE           _Pragma("optimize=none")
#define OPTIMIZE_LOW            _Pragma("optimize=low")
#define OPTIMIZE_MEDIUM         _Pragma("optimize=medium")
#define OPTIMIZE_HIGH           _Pragma("optimize=high")

#endif
/*********************************************************************
* GLOBAL FUNCTIONS
*/

/*********************************************************************
* EXTERN VARIABLES
*/




/*********************************************************************
* 				end of file
*********************************************************************/
#endif	/* GLOBLES_H */
