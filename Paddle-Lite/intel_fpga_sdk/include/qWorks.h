#ifndef _QWORKS_H_
#define _QWORKS_H_

#ifdef __cplusplus
extern "C" {
#endif

#if	!defined(NULL)
# if defined __GNUG__
#  define NULL (__null)
# else
#  if !defined(__cplusplus) && 0
#   define NULL ((void*)0)
#  else
#   define NULL (0)
#  endif
# endif
#endif

/* Boolean Values. */
#if	!defined(TRUE) || (TRUE!=1)
# define TRUE 1
#endif
#if	!defined(FALSE) || (FALSE!=0)
# define FALSE 0
#endif

#define NONE (-1) /* for times when NULL won't do */
#define EOS '\0' /* C string terminator */

/* Time-out defines */
#define NO_WAIT 0
#define WAIT_FOREVER (-1)

/* Common macros defines. */
#define MSB(x)	(((x) >> 8) & 0xff)	  	/* Most signif byte of 2-byte integer */
#define LSB(x)	((x) & 0xff)		  	/* Least signif byte of 2-byte integer*/
#define MSW(x) 	(((x) >> 16) & 0xffff) 	/* Most signif word of 2-word integer */
#define LSW(x) 	((x) & 0xffff) 		  	/* Least signif byte of 2-word integer*/

/* Swap the MSW with the LSW of a 32 bit integer. */
#define WORDSWAP(x) (MSW(x) | (LSW(x) << 16))

/* 32bit word byte/word swap macros. */
#define LLSB(x)		((x) & 0xff)
#define LNLSB(x) 	(((x) >> 8 ) & 0xff)
#define LNMSB(x) 	(((x) >> 16) & 0xff)
#define LMSB(x)	 	(((x) >> 24) & 0xff)
#define LONGSWAP(x) ((LLSB(x) << 24) | (LNLSB(x) << 16) | (LNMSB(x) << 8) | (LMSB(x))) 

/* Byte offset of member in structure */
#define OFFSETOF(structure, member) ((int)&(((structure*)0)->member))

/* Size of a member of a structure */
#define MEMBER_SIZE(structure, member) (sizeof(((structure*)0)->member))

/* Number of elements in an array */
#define NELEMENTS(array) (sizeof(array)/sizeof((array)[0]))

#ifndef MIN
# define MIN(a,b) ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
# define MAX(a,b) ((a) < (b) ? (b) : (a))
#endif

#ifndef ABS
# define ABS(a) (((a) < 0) ? (-a) : (a))
#endif

/* Storage class specifier definitions */
#if defined(_WIN32)
# include <windows.h>
#else
typedef	signed char INT8;
typedef	unsigned char UINT8;
typedef	short INT16;
typedef	unsigned short UINT16;
typedef	int INT32;
typedef	unsigned int UINT32;
typedef long LONG;
typedef unsigned long ULONG;
typedef	long long INT64;
typedef	unsigned long long UINT64;
typedef void* HANDLE;
#endif
typedef	int BOOL;
typedef	int STATUS;

/* Round up/down macros. */
#define ROUND_UP(x, align)	 (((int)(x) + (align - 1)) & ~(align - 1))
#define ROUND_DOWN(x, align) ((int) (x) & ~(align - 1))
#define ALIGNED(x, align)	 (((int)(x) & (align - 1)) == 0)

#ifdef __cplusplus
}
#endif

#endif

/* End of file */
