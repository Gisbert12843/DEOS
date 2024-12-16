//-------------------------------------------------
//          TestSuite: Conifg Xbee
//-------------------------------------------------
// Überträgt Daten zwischen UART1 und UART2 zur Kommunikation mit dem XBee Controller.
//-------------------------------------------------
#include "../progs.h"
#if defined(TESTTASK_ENABLED) && TESTTASK == TT_CONIFGXBEE

#include "../../lib/lcd.h"
#include "../../lib/util.h"
#include "../../lib/uart.h"
#include "../../lib/terminal.h"
#include "../../os_scheduler.h"

//! This programme transfers data between UART0 and UART1.
PROGRAM(1, AUTOSTART)
{
    os_enterCriticalSection();

    // init UART1 Xbee
    uart1_init(UART_BAUD_SELECT(38400, F_CPU));

    // init UART2 USB
    uart2_init(UART_BAUD_SELECT(38400, F_CPU));

    lcd_writeProgString(PSTR(" Configure Xbee"));


    while(1)
    {
        // Byte-weise Übertragung
        int byte;

        // UART2 -> UART1
        if(uart2_getrxcount() >= 1){
            byte = uart2_getc();
            uart1_putc(byte);
        }

        // UART1 -> UART2
        if(uart1_getrxcount() >= 1){
            byte = uart1_getc();
            uart2_putc(byte);
        }
    }
    os_leaveCriticalSection();
}

#endif