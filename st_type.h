#pragma once

// define standard types
typedef signed   char             S08;
typedef unsigned char             U08;
typedef signed   short int        S16;
typedef unsigned short int        U16;
typedef signed   long int         S32;
typedef unsigned long int         U32;
typedef float                     F32;
typedef double                    D64;

typedef signed   char            *PS08;
typedef unsigned char            *PU08;
typedef signed   short int       *PS16;
typedef unsigned short int       *PU16;
typedef signed   long int        *PS32;
typedef unsigned long int        *PU32;
typedef float                    *PF32;
typedef double                   *PD64;

//Windows SDK types
typedef unsigned long             DWORD;
typedef const char               *LPCSTR;
typedef unsigned char             BYTE;
typedef unsigned short            WORD;
typedef unsigned int              UINT;


#define MAKEWORD(a, b)      ((WORD)(((BYTE)(((DWORD)(a)) & 0xff)) | ((WORD)((BYTE)(((DWORD)(b)) & 0xff))) << 8))
#define MAKELONG(a, b)      ((LONG)(((WORD)(((DWORD)(a)) & 0xffff)) | ((DWORD)((WORD)(((DWORD)(b)) & 0xffff))) << 16))
#define LOWORD(l)           ((WORD)(((DWORD)(l)) & 0xffff))
#define HIWORD(l)           ((WORD)((((DWORD)(l)) >> 16) & 0xffff))
#define LOBYTE(w)           ((BYTE)(((DWORD)(w)) & 0xff))
#define HIBYTE(w)           ((BYTE)((((DWORD)(w)) >> 8) & 0xff))

//#define NULL    0
// define standard type limits
#include <limits.h>

#define S08_MIN   SCHAR_MIN
#define S08_MAX   SCHAR_MAX
#define U08_MAX   UCHAR_MAX

#define S16_MIN   SHRT_MIN
#define S16_MAX   SHRT_MAX
#define U16_MAX   USHRT_MAX

#define S32_MIN   LONG_MIN
#define S32_MAX   LONG_MAX
#define U32_MAX   ULONG_MAX





