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
File    : Main.h
Purpose : 
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
U8   SYSTEM_SIO_Get1 (void);
void SYSTEM_SIO_Send1(U8 b);
U8 * SYSTEM_GetBuffer(void);
int  SYSTEM_GetSizeOfBuffer(void);
