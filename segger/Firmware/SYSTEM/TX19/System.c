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
File    : System.c
Purpose : Initialisation and call main
---------------------------END-OF-HEADER------------------------------
*/

#include "System.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

#define INTCLR  (*(volatile U8  *)0xFFFFE060) /* Interrupt clear control */
#define IVR     (*(volatile U32 *)0xFFFFE040) /* Interrupt vector register */
#define SC0BUF  (*(volatile U8  *)0xFFFFF200) /* Serial channel1 buffer */
#define IMCCL   (*(volatile U16 *)0xFFFFE030) /* Interrupt mask control */
#define SC0MOD0 (*(volatile U8  *)0xFFFFF202) /* SIO0 Mode Register 0 */

/*********************************************************************
*
*       Forward declarations
*
**********************************************************************
*/

void main(void);

/*********************************************************************
*
*       Static functions
*
**********************************************************************
*/

static void __adecl _Startup(void) {
  __DI();                             // disable interrupts
  IMCCL         = 0x3333;             // enable rx & tx interrupt
  SC0MOD0       = 0x29;               // enable receiver, 8 bit UART-mode, baud rate generator
  main();
}

/*********************************************************************
*
*       Exported functions
*
**********************************************************************
*/

/*********************************************************************
*
*       SIO_Get1
*/
U8 SYSTEM_SIO_Get1(void) {
  while(IVR != 0x300);
  INTCLR = 0x30;
  return(SC0BUF);
}

/*********************************************************************
*
*       SIO_Send1
*/
void SYSTEM_SIO_Send1(U8 b) {
  SC0BUF = b;
  while(IVR != 0x310);
  INTCLR = 0x31;
}

