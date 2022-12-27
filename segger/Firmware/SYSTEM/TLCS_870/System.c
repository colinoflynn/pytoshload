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

#if   defined(TMP86FP24) || defined(TMP86FM48)
  #define UARTCR1  (*(volatile unsigned char *)0x1FDD) /* UART control register 1 (write only) */
  #define UARTSR1  (*(volatile unsigned char *)0x1FDD) /* UART status register (read only) */
  #define RDBUF    (*(volatile unsigned char *)0x1FDF) /* UART receive buffer (read only) */
  #define TDBUF    (*(volatile unsigned char *)0x1FDF) /* UART transmit buffer (write only) */
  #define TC5CR    (*(volatile unsigned char *)0x0014) /* timer/counter 5 control register */
  #define TC5DR    (*(volatile unsigned char *)0x0015) /* timer register */
#elif defined(TMP86FM29) || defined(TMP86FM25)
  #define UARTCR1  (*(volatile unsigned char *)0x0025) /* UART control register 1 (write only) */
  #define UARTSR1  (*(volatile unsigned char *)0x0025) /* UART status register (read only) */
  #define RDBUF    (*(volatile unsigned char *)0x0F9B) /* UART receive buffer (read only) */
  #define TDBUF    (*(volatile unsigned char *)0x0F9B) /* UART transmit buffer (write only) */
  #define TC5CR    (*(volatile unsigned char *)0x001A) /* timer/counter 5 control register */
  #define TTREG5   (*(volatile unsigned char *)0x001E) /* timer register */
#elif defined(TMP86FM49)
  #define UARTCR1  (*(volatile unsigned char *)0x0F95) /* UART control register 1 (write only) */
  #define UARTSR1  (*(volatile unsigned char *)0x0F95) /* UART status register (read only) */
  #define RDBUF    (*(volatile unsigned char *)0x0F97) /* UART receive buffer (read only) */
  #define TDBUF    (*(volatile unsigned char *)0x0F97) /* UART transmit buffer (write only) */
  #define TC3CR    (*(volatile unsigned char *)0x0027) /* timer/counter 3 control register */
  #define TTREG3   (*(volatile unsigned char *)0x0014) /* timer register */
#else
  #error Unsupported CPU!
#endif

/*********************************************************************
*
*       Forward declarations
*
**********************************************************************
*/

const U8 TimerInit; // This const variable is used to initialize the timer.
                    // It is initialized by ToshLoad during the RAM image transmission.

void main(void);

/*********************************************************************
*
*       Static functions
*
**********************************************************************
*/

static void _Startup(void) {
  unsigned int i;
  __asm("di");            // Disable interrupts
  __asm("ld SP,0x7F");    // Init stack pointer
  #if   defined(TMP86FM29)
    TTREG5   = TimerInit;
    TC5CR    = (0 << 0)   // Timer/event counter mode
            |  (1 << 3)   // Start
            |  (6 << 4);  // TC5 source clock select: Fc
    UARTCR1  = (6 << 0)   // Transmit clock select: TC5
            |  (0 << 3)   // no parity
            |  (0 << 4)
            |  (0 << 5)   // 1 stop bit
            |  (1 << 6)   // receive enable
            |  (1 << 7);  // transmit enable
  #elif defined(TMP86FM25)
    TTREG5   = TimerInit;
    TC5CR    = (0 << 0)   // Timer/event counter mode
            |  (1 << 3)   // Start
            |  (6 << 4);  // TC5 source clock select: Fc
    UARTCR1  = (6 << 0)   // Transmit clock select: TC5
            |  (0 << 3)   // no parity
            |  (0 << 4)
            |  (0 << 5)   // 1 stop bit
            |  (1 << 6)   // receive enable
            |  (1 << 7);  // transmit enable
  #elif defined(TMP86FP24)
    TC5DR    = TimerInit;
    TC5CR    = (0 << 0)   // Timer/event counter mode
            |  (6 << 2)   // TC5 source clock select: Fc
            |  (1 << 5);  // Start
    UARTCR1  = (6 << 0)   // Transmit clock select: TC5
            |  (0 << 3)   // no parity
            |  (0 << 4)
            |  (0 << 5)   // 1 stop bit
            |  (1 << 6)   // receive enable
            |  (1 << 7);  // transmit enable
  #elif  defined(TMP86FM48)
    TC5DR    = TimerInit;
    TC5CR    = (0 << 0)   // Timer/event counter mode
            |  (6 << 2)   // TC5 source clock select: Fc
            |  (1 << 5);  // Start
    UARTCR1  = (6 << 0)   // Transmit clock select: TC5
            |  (0 << 3)   // no parity
            |  (0 << 4)
            |  (0 << 5)   // 1 stop bit
            |  (1 << 6)   // receive enable
            |  (1 << 7);  // transmit enable
  #elif  defined(TMP86FM49)
    TTREG3   = TimerInit;
    TC3CR    = (0 << 0)   // Timer/event counter mode
            |  (1 << 3)   // Start
            |  (6 << 4);  // TC5 source clock select: Fc
    UARTCR1  = (6 << 0)   // Transmit clock select: TC5
            |  (0 << 3)   // no parity
            |  (0 << 4)
            |  (0 << 5)   // 1 stop bit
            |  (1 << 6)   // receive enable
            |  (1 << 7);  // transmit enable
  #else
    #error Unsupported CPU!
  #endif
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
  while (!(UARTSR1 & 0x10));
  return RDBUF;
}

/*********************************************************************
*
*       SIO_Send1
*/
void SYSTEM_SIO_Send1(U8 b) {
  TDBUF = b;
  while (!(UARTSR1 & 0x04));
}

