#ifndef _BTLIB_TYPE_H
#define _BTLIB_TYPE_H

//#include "bt_sys_cfg.h"
#define BTM_NAME_MAX_LEN                    24

#ifndef NULL
	#define NULL 0
#endif

#ifndef SUCCESS
	#define SUCCESS     0
#endif

#ifndef FAILURE
	#define FAILURE     1
#endif

#ifndef TRUE
	#define TRUE 1
#endif

#ifndef FALSE
	#define FALSE 0
#endif

#define BIT0                            0x00000001
#define BIT1                            0x00000002
#define BIT2                            0x00000004
#define BIT3                            0x00000008
#define BIT4                            0x00000010
#define BIT5                            0x00000020
#define BIT6                            0x00000040
#define BIT7                            0x00000080
#define BIT8                            0x00000100
#define BIT9                            0x00000200
#define BIT10                           0x00000400
#define BIT11                           0x00000800
#define BIT12                           0x00001000
#define BIT13                           0x00002000
#define BIT14                           0x00004000
#define BIT15                           0x00008000
#define BIT16                           0x00010000
#define BIT17                           0x00020000
#define BIT18                           0x00040000
#define BIT19                           0x00080000
#define BIT20                           0x00100000
#define BIT21                           0x00200000
#define BIT22                           0x00400000
#define BIT23                           0x00800000
#define BIT24                           0x01000000
#define BIT25                           0x02000000
#define BIT26                           0x04000000
#define BIT27                           0x08000000
#define BIT28                           0x10000000
#define BIT29                           0x20000000
#define BIT30                           0x40000000
#define BIT31                           0x80000000

typedef unsigned char  byte;                    /* Unsigned  8 bit quantity                           */
typedef unsigned char  uint8;                    /* Unsigned  8 bit quantity                           */
typedef signed   char  int8;                    /* Signed    8 bit quantity                           */
typedef unsigned short uint16;                   /* Unsigned 16 bit quantity                           */
typedef signed   short int16;                   /* Signed   16 bit quantity                           */
typedef unsigned int   uint32;                   /* Unsigned 32 bit quantity                           */
typedef signed   int   int32;                   /* Signed   32 bit quantity                           */

typedef unsigned char  BYTE;                    /* Unsigned  8 bit quantity                           */
typedef unsigned char  UINT8;
typedef unsigned short UINT16;
typedef unsigned int   UINT32;

typedef unsigned char  INT8U;
typedef unsigned short INT16U;
typedef unsigned int   INT32U;

typedef signed char    INT8S;
typedef signed short   INT16S;
typedef signed int     INT32S;

typedef unsigned char  BOOLEAN;

typedef unsigned short CNH;

typedef struct 
{
    uint8  A[6];
}BD_ADDR;

typedef struct
{
	uint8 A[10];
}CHANMAP;

typedef struct 
{
    uint8  A[3];
}LAP;

typedef struct 
{
    uint8  A[3];
}CLASS;

typedef struct 
{
    uint8  A[16];
}PIN_CODE;

typedef struct 
{
    uint8  A[16];
}LINK_KEY;

typedef struct 
{
    uint8  A[4];
}SRES;

typedef struct 
{
    uint8  A[12];
}ACO;

typedef struct 
{
    uint8   A[8];
}FEATURES;

typedef struct 
{
    uint8  A[14];
}NAME_VEC;
 
struct bdaddr_t 
{
    uint8 addr[6];
};
 
struct class_of_device_t 
{
    uint8  A[3];
};

struct link_key_t
{
    uint8  A[16];
};

typedef struct 
{
    unsigned char A[BTM_NAME_MAX_LEN];
}DEVICE_NAME;

/*********************** add the followings when add MP3 program ********************/
typedef enum
{
    RES_SUCCESS		  = 0,
    RES_ERROR		  = 1,
    RES_INVALID_PTR    = 2,
    RES_INVALID_PARAM  = 3,
    RES_MALLOC_FAILED  = 4,

    /* SD */
    RES_SD_ERR_CMD_BUSY			= 0xA0,
    RES_SD_ERR_TIMEOUT			= 0xA1,
    RES_SD_ERR_NO_CARD			= 0xA2,
    RES_SD_ERR_DATA_BUSY			= 0xA3,
    RES_SD_ERR_DATA			= 0xA4,
    RES_SD_ERR_WRITEPROTECT		= 0xA5,

    /*USBH*/
    RES_USBH_ERR_CONN		= 0xB0,
    RES_USBH_ERR_ENUM_FAIL	= 0xB1,
    RES_USBH_ERR_NOT_SUPPORT = 0xB2,
    RES_USBH_ERR_READ_SECTOR	= 0xB3,

    RES_UNKNOWN
}RES_T;

typedef enum
{

    PHY_SD = 0x0,
    PHY_USBHOST = 0X1,

}PHYSTATUS;

typedef enum
{
    SD_OUT = 0,
    SD_IN = 1,
}SDSTATUS;

/* These types must be 16-bit, 32-bit or larger integer */
typedef int				INT;
typedef unsigned int	UINT;

/* These types must be 8-bit integer */
typedef signed char	    CHAR;
typedef unsigned char	UCHAR;
typedef unsigned char	BYTE;

/* These types must be 16-bit integer */
typedef short			SHORT;
typedef unsigned short	USHORT;
typedef unsigned short	WORD;
typedef unsigned short	WCHAR;

/* These types must be 32-bit integer */
typedef long			LONG;
typedef unsigned long	ULONG;
typedef unsigned long	DWORD;

/************************* MP3 defination end **********************/
#endif

