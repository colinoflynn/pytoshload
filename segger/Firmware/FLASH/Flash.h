/*********************************************************************
*                SEGGER MICROCONTROLLER SYSTEME GmbH                 *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*           (C) 2000    SEGGER Microcontroller Systeme GmbH          *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************
----------------------------------------------------------------------
File    : Flash.h
Purpose : Interface Flash routines TLCS-900 family
---------------------------END-OF-HEADER------------------------------
*/

// basic data types
#ifndef I8
  #define I8    signed char  //   signed 8  bits
#endif
#ifndef U8
  #define U8  unsigned char  // unsigned 8  bits
#endif
#ifndef I16
  #define I16   signed short //   signed 16 bits
#endif
#ifndef U16
  #define U16 unsigned short // unsigned 16 bits
#endif
#ifndef I32
  #define I32   signed long  //   signed 32 bits
#endif
#ifndef U32
  #define U32 unsigned long  // unsigned 32 bits
#endif

// interface
int  FLASH_Write       (U32 Addr, void * pData, int Len);
int  FLASH_EraseBlock  (U32 Addr);
void FLASH_ProtectBlock(U32 Addr);
