
#ifndef __BASICTYPE_H__
#define __BASICTYPE_H__

#define code
#define idata

#define IRQL_1
#define IRQL_0

#define FALSE   0
#define TRUE    1

//*************************************************************************
// basic typedefs
//*************************************************************************

#define CHAR    char
#define SHORT   short
#define UCHAR   unsigned char
#define USHORT  unsigned short
#define ULONG   unsigned long
#define BOOL    unsigned char


#define PCHAR   CHAR *
#define PUCHAR  UCHAR *
#define PUSHORT USHORT *
#define PULONG  ULONG *
#define BOOLEAN UCHAR

typedef union _Address {
    USHORT  IoPort;     /* IoData Port */
    UCHAR * pData;      /* Memory Address */
} ADDRESS;

typedef struct _HID_KEYS_REPORT {
	UCHAR modifier;
	UCHAR reserved;
	UCHAR keycode;
} HID_KEYS_REPORT, *PHID_KEYS_REPORT;

typedef struct _HID_KEYS_REPORT2 {
	UCHAR modifier;
	UCHAR reserved;
	UCHAR keycode[6];
} HID_KEYS_REPORT2, *PHID_KEYS_REPORT2;
#endif
