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
File    : CRC.h
Purpose : Interface definition
---------------------------END-OF-HEADER------------------------------
*/

#ifndef CRC_H
#define CRC_H

#if defined(__cplusplus)
  extern "C" {
#endif

void CRC_Init(unsigned short * pCRC);
void CRC_Calc(unsigned short * pCRC, unsigned char * pData, unsigned long len);

#if defined(__cplusplus)
  }
#endif

#endif //CRC_H
