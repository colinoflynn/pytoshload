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
File    : Flash8.c
Purpose : Flash routines for 8 bit flash
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

#define FLSCR (*(volatile U8 *)0x0FFF) /* Flash Memory Control Register */

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
U16 FLASH_RemapAddr(U16 Addr) {
  volatile U8 * pDest;
  if (Addr < 0x8000) {
    Addr += 0x8000;
    FLSCR = 0x30; /* Select BANK0 (1000H-7FFFH) and enable flash memory write  */
  } else {
    FLSCR = 0x38; /* Select BANK1 (8000H-FFFFH) and Enable flash memory write */
  }
  return Addr;
}

/*********************************************************************
*
*       Exported functions
*
**********************************************************************
*/

/*********************************************************************
*
*       FLASH_Write
*/
int FLASH_Write(U32 Addr, void   * pData, int Len) {
  volatile U8 * pDest;
  U8 * pSrc  = (U8 *)pData;
  while(Len--) {
    U16 i = 0xFFFF;
    U8 Data = *pSrc++;
    pDest = (volatile U8 *)FLASH_RemapAddr((U16)Addr);
    *((volatile U8 *)(INT_ROMBASEADDR + 0x555)) = 0xAA;
    *((volatile U8 *)(INT_ROMBASEADDR + 0xAAA)) = 0x55;
    *((volatile U8 *)(INT_ROMBASEADDR + 0x555)) = 0xA0;
    *pDest = Data;
    while (i--) {
      if ((U8)Data == *((volatile U8 *)pDest)) {
        break;
      }
    }
    if (!i) {
      return 1; // error
    }
    Addr++;
  }
  return 0; // ok
}

/*********************************************************************
*
*       FlashBlockErase
*/
int FLASH_EraseBlock(U32 Addr) {
  volatile U8 * pDest;
  U16 i = 0xFFFF;
  pDest = (volatile U8 *)FLASH_RemapAddr((U16)Addr);
  *((volatile U8 *)(INT_ROMBASEADDR + 0x555)) = 0xAA;
  *((volatile U8 *)(INT_ROMBASEADDR + 0xAAA)) = 0x55;
  *((volatile U8 *)(INT_ROMBASEADDR + 0x555)) = 0x80;
  *((volatile U8 *)(INT_ROMBASEADDR + 0x555)) = 0xAA;
  *((volatile U8 *)(INT_ROMBASEADDR + 0xAAA)) = 0x55;
  *pDest = 0x30;
  while (i--) {
    U16 j = 0xFFFF;
    if (0xff == *pDest) {
      return 0; // ok
    }
    while (j--);
  }
  *((volatile U8 *)(INT_ROMBASEADDR)) = 0xF0;
  return 1;
}
