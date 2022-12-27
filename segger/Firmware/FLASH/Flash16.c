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
File    : Flash16.c
Purpose : Flash routines for 16 bit flash
---------------------------END-OF-HEADER------------------------------
*/
                                                                 
#include "Flash.h"
#include "Config.h"

/*********************************************************************
*
*       Exported functions FLASH_16
*
**********************************************************************
*/

/*********************************************************************
*
*       FLASH_Write
*/
int FLASH_Write(U32 Addr, void   * pData, int Len) {
  U16 * pDest = (U16 *)Addr;
  U16 * pSrc  = (U16 *)pData;
  Len = (Len + 1) >> 1;
  while(Len--) {
    U32 i = 0xFFFFFF;
    U16 Data = *pSrc++;
    *((volatile U16 *)(INT_ROMBASEADDR + 0xAAAA)) = 0xAA;
    *((volatile U16 *)(INT_ROMBASEADDR + 0x5554)) = 0x55;
    *((volatile U16 *)(INT_ROMBASEADDR + 0xAAAA)) = 0xA0;
    *pDest = Data;
    do {
      if ((U8)Data == *((volatile U8 *)pDest)) {
        break;
      }
      if (--i == 0) {
        return 1; // error
      }
    } while (1);
    if (*(volatile U16 *)pDest != Data) {
      return 1; // error
    }
    if (*(volatile U16 *)pDest != Data) {
      return 1; // error
    }
    pDest++;
  }
  return 0; // ok
}

/*********************************************************************
*
*       FlashBlockErase
*/
int FLASH_EraseBlock(U32 Addr) {
  U32 i = 0xFFFFFF;
  *((volatile U16 *)(INT_ROMBASEADDR + 0xAAAA)) = 0xAA;
  *((volatile U16 *)(INT_ROMBASEADDR + 0x5554)) = 0x55;
  *((volatile U16 *)(INT_ROMBASEADDR + 0xAAAA)) = 0x80;
  *((volatile U16 *)(INT_ROMBASEADDR + 0xAAAA)) = 0xAA;
  *((volatile U16 *)(INT_ROMBASEADDR + 0x5554)) = 0x55;
  *((volatile U16 *)Addr) = 0x30;
  while (i--) {
    int j = 2048;
    if (0xff == *((volatile U8 *)Addr)) {
      return 0; // ok
    }
    while (j--);
  }
  *((volatile U16 *)(INT_ROMBASEADDR)) = 0xF0;
  return 1;
}

/*********************************************************************
*
*       FLASH_ProtectBlock
*/
void FLASH_ProtectBlock(U32 Addr) {
  // dummy
}