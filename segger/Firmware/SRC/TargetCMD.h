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
File    : TargetCMD.h
Purpose : Command definitions
---------------------------END-OF-HEADER------------------------------
*/

// target commands
#define CMD_ID             0
#define CMD_ERASE_SECTOR   1
#define CMD_PROGRAM        2
#define CMD_READ_BACK      3
#define CMD_GET_CRC        4
#define CMD_BLANK_CHECK    5
#define CMD_PROTECT_BLOCK  6
#define CMD_GET_SUM        7

#define ERROR_ERASE        1
#define ERROR_WRITE        2
#define ERROR_COMPARE      3
#define ERROR_BLANKCHECK   4
#define ERROR_NACK         5
#define ERROR_PROTECT      6
