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
File    : FlashEE.c
Purpose : Flash routines for EE flash
---------------------------END-OF-HEADER------------------------------
*/
                                                                 
#include "Flash.h"
#include "Config.h"

#if   defined(TMP86FP24) || defined(TMP86FM48)
  #define EEPCR (*(volatile unsigned char* )0x1FE0) /* FLASH control register */
  #define EEPSR (*(volatile unsigned char* )0x1FE1) /* FLASH status register */
#elif defined(TMP86FM29) || defined(TMP86FM25)
  #define EEPCR (*(volatile unsigned char* )0x0FE0) /* FLASH control register */
  #define EEPSR (*(volatile unsigned char* )0x0FE1) /* FLASH status register */
#else
  #error Unsupported CPU!
#endif

/*********************************************************************
*
*       Exported functions FLASH_EE
*
**********************************************************************
*/

/*********************************************************************
*
*       FLASH_Write
*/
int FLASH_Write(U32 Addr, void * pData, int Len) {
  char * pDest = (char *)Addr;
  char * pSrc  = (char *)pData;
  EEPCR = 0x3B;                 /* Enable writing */
  while(Len--) {
    *pDest = *(pSrc++);         /* Destination address needs not to be incremented */
  }
  while (EEPSR & 1);            /* Wait while busy */
  EEPCR = 0xCB;                 /* Disable writing */
  return 0; // ok
}
