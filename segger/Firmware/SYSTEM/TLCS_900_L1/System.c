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
File    : Startup.c
Purpose : Initialisation and call main
---------------------------END-OF-HEADER------------------------------
*/

#include "System.h"
#include "Config.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

#define INTCLR (*(volatile unsigned char* )0x0088) /* Interrupt clear control */
#define INTES1 (*(volatile unsigned char* )0x009d) /* Interrupt enable serial 1 */
#define SC1BUF (*(volatile unsigned char* )0x0208) /* Serial channel1 buffer */

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

static void _Startup(void) {
  unsigned int i;
  __asm("di");                  // disable interrupts
  __asm("ld XSP,0x1100");       // init stack pointer
  for (i = 0; i < 40000; i++) {          
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
  }
  main();
}

/*********************************************************************
*
*       SIO_Get1
*/
U8 SYSTEM_SIO_Get1(void) {
  while (!(INTES1 & 0x08));
  return(SC1BUF);
}

/*********************************************************************
*
*       SIO_Send1
*/
void SYSTEM_SIO_Send1(U8 b) {
  INTCLR = 0x24;
  SC1BUF = b;
  while (!(INTES1 & 0x80));
}

