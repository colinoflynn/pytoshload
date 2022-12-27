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
File    : Flash32.c
Purpose : Flash routines for 32 bit flash
---------------------------END-OF-HEADER------------------------------
*/
                                                                 
#include "Flash.h"
#include "Config.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

#ifndef INT_ROMBASEADDR
  #define INT_ROMBASEADDR 0
#endif

#ifndef NOP
  #define NOP()
#endif

#ifndef FLASH_SWAP_ADR
  #define FLASH_SWAP_ADR 0
#endif

#if FLASH_SWAP_ADR
  #define ADR0 0x5554
  #define ADR1 0xAAA8
#else
  #define ADR0 0xAAA8
  #define ADR1 0x5554
#endif

/*********************************************************************
*
*       Exported functions FLASH_32
*
**********************************************************************
*/

/*********************************************************************
*
*       FLASH_Write
*/
int FLASH_Write(U32 Addr, void * pData, int Len) {
  U32 * pDest = (U32 *)Addr;
  U8  * pSrc  = (U8  *)pData;
  Len = 1 + (Len + 3) >> 2;
  while(--Len) {
    U32 Data = *pSrc | ((U16)(*(pSrc + 1)) << 8) | ((U32)(*(pSrc + 2)) << 16) | ((U32)(*(pSrc + 3)) << 24);
    U32 i = 0xFFFFFF;
    pSrc += 4;
    *((volatile U32 *)(INT_ROMBASEADDR + ADR0)) = 0xAA;
    NOP(); // required by TX19
    *((volatile U32 *)(INT_ROMBASEADDR + ADR1)) = 0x55;
    NOP(); // required by TX19
    *((volatile U32 *)(INT_ROMBASEADDR + ADR0)) = 0xA0;
    NOP(); // required by TX19
     *pDest = Data;
    while (--i) {
      if ((U8)Data == *((volatile U8 *)pDest)) {
        goto ProgrammedOK;
      }
    }
    return 1; // error
ProgrammedOK:
    pDest++;
  }
  return 0; // ok
}

/*********************************************************************
*
*       FLASH_EraseBlock
*/
int FLASH_EraseBlock(U32 Addr) {
  U32 i = 0x5FFF;
  *((volatile U32 *)(INT_ROMBASEADDR + ADR0)) = 0xAA;
  NOP(); // required by TX19
  *((volatile U32 *)(INT_ROMBASEADDR + ADR1)) = 0x55;
  NOP(); // required by TX19
  *((volatile U32 *)(INT_ROMBASEADDR + ADR0)) = 0x80;
  NOP(); // required by TX19
  *((volatile U32 *)(INT_ROMBASEADDR + ADR0)) = 0xAA;
  NOP(); // required by TX19
  *((volatile U32 *)(INT_ROMBASEADDR + ADR1)) = 0x55;
  NOP(); // required by TX19
  *((volatile U32 *)Addr) = 0x30;
  while (--i) {
    int j = 4096;
    if (0xff == *((volatile U8 *)Addr)) {
      return 0; // ok
    }
    while (--j); // delay
  }
  *((volatile U32 *)(INT_ROMBASEADDR)) = 0xF0; // reset
  return 1; // error
}

/*********************************************************************
*
*       FLASH_ProtectBlock
*/
void FLASH_ProtectBlock(U32 Addr) {
  U16 i;
  i = 0x3;
  // block protect command
  while(--i) {
    *((volatile U32 *)(INT_ROMBASEADDR + ADR0)) = 0xAA;
    NOP(); // required by TX19
    *((volatile U32 *)(INT_ROMBASEADDR + ADR1)) = 0x55;
    NOP(); // required by TX19
    *((volatile U32 *)(INT_ROMBASEADDR + ADR0)) = 0x9A;
    NOP(); // required by TX19
  }
  *((volatile U32 *)Addr) = 0x00;
  while(--i); // wait at least 4 us
  *((volatile U32 *)(INT_ROMBASEADDR)) = 0x00;
  while(--i); // wait at least 100 us
  *((volatile U32 *)(INT_ROMBASEADDR)) = 0x00;
  while(--i); // wait at least 8 us
  *((volatile U32 *)(INT_ROMBASEADDR)) = 0x00;
  NOP(); // required by TX19
  // reset
  *((volatile U32 *)(INT_ROMBASEADDR)) = 0xF0; // reset
}
