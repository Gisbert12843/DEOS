//-------------------------------------------------
//          TestSuite: Protocol Stack
//-------------------------------------------------
// Tests the software stack of rfAdapter, serialAdapter, and xbee.
//-------------------------------------------------
#include "../progs.h"
#if defined(TESTTASK_ENABLED) && TESTTASK == TT_PROTOCOLSTACK

#include "../../lib/lcd.h"
#include "../../lib/util.h"
#include "../../communication/xbee.h"
#include "../../communication/rfAdapter.h"
#include "../../os_scheduler.h"

//! Remove or comment in these lines to deactivate a phase
#define PHASE_1 1
#define PHASE_2 1
#define PHASE_3 1
#define PHASE_4 1
#define PHASE_5 1

//! Displays a counter in the right corner of the LCD.
void displayCounter(int i);

//! This program includes the sending procedure
PROGRAM(1, AUTOSTART)
{
    rfAdapter_init();

#if PHASE_1 == 1
    /*!
     * Sending Toggle LED to Board Address
     */

    lcd_writeProgString(PSTR("Phase 1: Toggle"));
    lcd_line2();
    lcd_writeProgString(PSTR("Onboard LED"));

    //! Toggle for 15 seconds and display Countdown
    for (int i = 6; i > 0; --i) {
        displayCounter(i);
        rfAdapter_sendToggleLed(serialAdapter_address);
        delayMs(1000);
    }

#endif
#if PHASE_2 == 1
    /*!
     * Sending setLED to Board Address
     */

    lcd_clear();
    lcd_writeProgString(PSTR("Phase 2:"));

    //! LED on for 10 seconds and display Countdown
    lcd_line2();
    lcd_writeProgString(PSTR("LED On"));

    rfAdapter_sendSetLed(serialAdapter_address, 1);
    for (int i = 10; i > 0; --i) {
        displayCounter(i);
        delayMs(1000);
    }

    //! LED off for 10 seconds and display Countdown
    lcd_line2();
    lcd_writeProgString(PSTR("LED Off"));

    rfAdapter_sendSetLed(serialAdapter_address, 0);
    for (int i = 10; i > 0; --i) {
        displayCounter(i);
        delayMs(1000);
    }

#endif
#if PHASE_3 == 1
    /*!
     * Send LCD Print and LCD Clear to Board Address
     */

    lcd_clear();
    lcd_writeProgString(PSTR("Phase 3:"));
    delayMs(1000);
    lcd_line2();

    //! Sending Text
    rfAdapter_sendLcdPrint(serialAdapter_address, "lcd_clear() in");
    delayMs(50);

    //! Countdown for Clear
    for (int i = 3; i >= 0; --i) {
        displayCounter(i);
        delayMs(1000);
    }

    //! Sending LCDClear
    rfAdapter_sendLcdClear(serialAdapter_address);
    delayMs(2000);

#endif
#if PHASE_4 == 1
    /*!
     * Send LCD GoTo and LCD Print ProcMem to Broadcast Address
     */

    lcd_clear();
    lcd_writeProgString(PSTR("Phase 4:"));
    lcd_line2();
    lcd_writeProgString(PSTR("    -->  <--"));
    delayMs(2000);

    //! Sending Okay between the Errors
    rfAdapter_sendLcdGoto(ADDRESS_BROADCAST, 1, 7);
    rfAdapter_sendLcdPrintProcMem(ADDRESS_BROADCAST, PSTR("OK"));
    delayMs(3000);

#endif
#if PHASE_5 == 1
    /*!
     * Sends a reference frame to check compatibility with the other implementation.
     * Addressed to Broadcast Address
     */

    lcd_clear();
    lcd_writeProgString(PSTR("Phase 5:  refer-ence frame "));
    lcd_goto(1,14);
    delayMs(1000);

    //! The reference frame is equal to rfAdapter_sendLcdPrint(ADDRESS_BROADCAST, "OK");
    //! Created by using lcd_writeHexByte() in xbee_write().
    uint8_t data[] = {0x46, 0x52, 0x08, 0xFF, 0x04, 0x12, 0x02, 0x4F, 0x4B, 0xF3};
    uint8_t length = sizeof(data);

    //! Write directly to UART
    xbee_writeData(data, length);

    delayMs(2000);

#endif

    //! Show finished screen
    lcd_clear();
    lcd_writeProgString(PSTR(" Test finished"));
    while(1){}
}

//! This program includes the receiving procedure
PROGRAM(2, AUTOSTART)
{
    while(1)
    {
        rfAdapter_worker();
    }
}

/*!
 * Displays a counter in the right corner of the LCD.
 *
 * @param counter The counter value to display (0-99).
 */
void displayCounter(int counter){
    if (counter >= 10){
        lcd_goto(2,14);
        lcd_writeDec(counter);
    }

    //! Write 0 to the decade
    if(counter == 9){
        lcd_goto(2,14);
        lcd_writeDec(0);
        lcd_goto(2,15);
        lcd_writeDec(counter);
    }

    if(counter <= 9){
        lcd_goto(2,15);
        lcd_writeDec(counter);
    }
}
#endif