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

#if   defined(TMP95FW54)
  #define INTES (*(volatile unsigned char* )0x007d) /* Interrupt enable serial 1 */
  #define SCBUF (*(volatile unsigned char* )0x0050) /* Serial channel1 buffer */
#elif defined(TMP95FY64)
  #define INTES (*(volatile unsigned char* )0x007e) /* Interrupt enable serial 2 */
  #define SCBUF (*(volatile unsigned char* )0x0054) /* Serial channel2 buffer */
#else
  #error Unsupported CPU
#endif

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
  __asm("ld XSP,0x200");        // init stack pointer
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
  while (!(INTES & 0x08));
  return(SCBUF);
}

/*********************************************************************
*
*       SIO_Send1
*/
void SYSTEM_SIO_Send1(U8 b) {
  INTES &= 0x7f;
  SCBUF = b;
  while (!(INTES & 0x80));
}

