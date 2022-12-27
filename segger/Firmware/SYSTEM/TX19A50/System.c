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

#define BASE_SIO2 0xFFFFC4C0
#define BASE_IRC  0xFFFFD000

#define ICLR    (*(volatile U32 *)(BASE_IRC  + 0x84)) /* Interrupt clear control */
#define IVR     (*(volatile U32 *)(BASE_IRC  + 0x80)) /* Interrupt vector register */
#define SC0BUF  (*(volatile U8  *)(BASE_SIO2 + 0x10)) /* SIO channel 2 buffer */
#define IMR34   (*(volatile U8  *)(BASE_IRC  + 0x34)) /* Interrupt mode register, SIO channel 2 Tx */
#define IMR35   (*(volatile U8  *)(BASE_IRC  + 0x35)) /* Interrupt mode register, SIO channel 2 Rx */
#define SC0MOD0 (*(volatile U8  *)(BASE_SIO2 + 0x00)) /* SIO channel 2 Mode Register 0 */

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

static void __adecl _Startup(void) { /* Stack pointer needs not to be initialized, r29 (sp) points to 0xFFFFBFF0 */
  __DI();                            /* Disable interrupts */
  IMR34         = 0x03;              /* Enable tx interrupt */
  IMR35         = 0x03;              /* Enable rx interrupt */
  SC0MOD0       = 0x29;              /* Enable receiver, 8 bit UART-mode, baud rate generator */
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
  while(IVR != 0x0D4);
  ICLR = 0x0D4;
  return(SC0BUF);
}

/*********************************************************************
*
*       SIO_Send1
*/
void SYSTEM_SIO_Send1(U8 b) {
  SC0BUF = b;
  while(IVR != 0x0D0);
  ICLR = 0x0D0;
}

