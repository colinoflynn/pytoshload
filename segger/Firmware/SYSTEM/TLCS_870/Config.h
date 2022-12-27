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
File    : FlashConf.h
Purpose : Flash configuration file
---------------------------END-OF-HEADER------------------------------
*/

#if defined(TMP86FM49)
  #define DISABLE_GET_CRC       1
  #define DISABLE_ERASE_SECTOR  0
  #define DISABLE_PROTECT_BLOCK 1
  #define DISABLE_BLANK_CHECK   0
  #define DISABLE_GET_SUM       0
#else
  #define DISABLE_GET_CRC       1
  #define DISABLE_ERASE_SECTOR  1
  #define DISABLE_PROTECT_BLOCK 1
  #define DISABLE_BLANK_CHECK   1
  #define DISABLE_GET_SUM       0
#endif