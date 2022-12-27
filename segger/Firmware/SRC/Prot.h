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
File    : Prot.h
Purpose : Interface of low level protocoll
---------------------------END-OF-HEADER------------------------------
*/

#ifndef PROT_H
#define PROT_H

#if defined(__cplusplus)
  extern "C" {
#endif

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

// commands
#define CMD_NACK  0xF1 // not acknowledge
#define CMD_ACK   0xF2 // acknowledge
#define CMD_ERROR 0xF3 // error

// types
typedef void tcbPROTSend1 (U8 Byte);
typedef void tcbPROTNotify(U8 * pBytes, int Size);

// interface
void PROT_Send    (int Size);
void PROT_SendData(int Size, int SizePara);
void PROT_OnRx    (U8 Byte);

#if defined(__cplusplus)
  }
#endif

#endif //PROT_H
