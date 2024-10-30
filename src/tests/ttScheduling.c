//-------------------------------------------------
//          TestSuite: Scheduling Strategies
//-------------------------------------------------
// Checks if scheduling strategies correctly
// schedule processes
//-------------------------------------------------
#include "progs.h"
#if defined(TESTTASK_ENABLED) && TESTTASK == TT_SCHEDULING

#include "lib/lcd.h"
#include "lib/util.h"
#include "os_scheduler.h"
#include "os_scheduling_strategies.h"
#include <avr/interrupt.h>

#define PHASE_1 1
#define PHASE_2 1
#define PHASE_3 1
#define PHASE_4 1

volatile uint8_t capture[32];
volatile uint8_t i = 0;

#define LCD_DELAY 2000
#define NUM_EXECUTIONS_SCHEDULABILITY 3

ISR(TIMER2_COMPA_vect);

// Array containing the correct output values for all four scheduling strategies.
const uint8_t scheduling[2][32] PROGMEM = {
    {1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2}, // Round Robin
    {1, 2, 1, 3, 2, 1, 3, 1, 2, 1, 3, 2, 1, 3, 1, 2, 1, 3, 2, 1, 3, 1, 2, 1, 3, 2, 1, 3, 1, 2, 1, 3}, // Dynamic Priority Round Robin
};

//! Tests if strategy is implemented (default return is 0)
uint8_t strategyImplemented()
{
  process_id_t nextId = 0;
  process_t processes[MAX_NUMBER_OF_PROCESSES];

  // Copy current os_processes
  for (uint8_t i = 0; i < MAX_NUMBER_OF_PROCESSES; ++i)
  {
    processes[i] = *os_getProcessSlot(i);
  }

  // Request next processId without changing anything
  // Current process is always 1
  switch (os_getSchedulingStrategy())
  {

  case OS_SS_ROUND_ROBIN:
    nextId = os_scheduler_RoundRobin(processes, os_getCurrentProc());
    break;
  case OS_SS_DYNAMIC_PRIORITY_ROUND_ROBIN:
    nextId = os_scheduler_DynamicPriorityRoundRobin(processes, os_getCurrentProc());
    break;
  default:
    lcd_clear();
    lcd_writeProgString(PSTR("Invalid strategy"));
    while (1)
    {
    }
  }

  os_resetSchedulingInformation(os_getSchedulingStrategy());

  if (nextId == 0)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}

/*!
 *  Function that sets the current strategy to the given one
 *  and outputs the name of the strategy on the LCD.
 */
uint8_t setActiveStrategy(scheduling_strategy_t strategy)
{
  uint8_t strategyIndex = 0;
  switch (strategy)
  {
  case OS_SS_ROUND_ROBIN:
    strategyIndex = 0;
    lcd_writeProgString(PSTR("RoundRobin"));
    os_setSchedulingStrategy(OS_SS_ROUND_ROBIN);
    break;

  case OS_SS_DYNAMIC_PRIORITY_ROUND_ROBIN:
    strategyIndex = 1;
    lcd_writeProgString(PSTR("DynamicPriority RoundRobin"));
    os_setSchedulingStrategy(OS_SS_DYNAMIC_PRIORITY_ROUND_ROBIN);
    break;

  default:
    break;
  }
  delayMs(LCD_DELAY);

  return strategyIndex;
}

/*!
 *  Function that performs the given strategy for 32 steps
 *  and checks if the processes were scheduled correctly
 */
void performStrategyTest(scheduling_strategy_t strategy)
{
  lcd_clear();

  // Change scheduling strategy
  uint8_t strategyIndex = setActiveStrategy(strategy);

  // Test if first step is 0
  if (!strategyImplemented())
  {
    lcd_clear();
    lcd_writeProgString(PSTR("Not impl. or idle returned"));
    delayMs(LCD_DELAY);
    return;
  }

  // Perform scheduling test.
  // Save the id of the running process and call the scheduler.
  i = 0;
  while (i < 32)
  {
    capture[i++] = 1;
    TIMER2_COMPA_vect();
  }

  // Print captured schedule
  lcd_clear();
  for (i = 0; i < 32; i++)
  {
    lcd_writeDec(capture[i]);
  }

  // Check captured schedule
  for (i = 0; i < 32; i++)
  {
    if (capture[i] != pgm_read_byte(&scheduling[strategyIndex][i]))
    {
      // Move cursor
      lcd_goto((i >= 16), (i % 16));
      // Show cursor without underlining the position
      lcd_blinkOn();
      while (1)
      {
      }
    }
    if (i == 31)
    {
      delayMs(LCD_DELAY);
      lcd_clear();
      lcd_writeProgString(PSTR("OK"));
    }
  }

  delayMs(LCD_DELAY);
}

/*!
 *  Function that performs the given strategy and checks
 *  if all processes could be scheduled
 */
void performSchedulabilityTest(scheduling_strategy_t strategy, uint8_t expectation)
{

  lcd_clear();

  // Change scheduling strategy
  setActiveStrategy(strategy);

  // Test if first step is 0
  if (!strategyImplemented())
  {
    lcd_clear();
    lcd_writeProgString(PSTR("Not impl. or idle returned"));
    delayMs(LCD_DELAY);
    return;
  }

  uint8_t captured = 0;

  // Doing this multiple times will make sure we have a high probability of
  // scheduling every process (relevant for random strategy)
  for (uint8_t j = 0; j < NUM_EXECUTIONS_SCHEDULABILITY; j++)
  {
    // Perform strategy 32 times
    i = 0;
    while (i < 32)
    {
      capture[i++] = 1;
      TIMER2_COMPA_vect();
    }

    // Calculate which processes were scheduled
    for (uint8_t k = 0; k < 32; k++)
    {
      captured |= (1 << capture[k]);
    }

    // Check if all processes other than the idle process
    // were scheduled
    if (captured == expectation)
    {
      lcd_clear();
      lcd_writeProgString(PSTR("OK"));
      delayMs(LCD_DELAY);

      // Everything fine, so we can stop the test here
      return;
    }
  }

  // Find processes that were not scheduled (but should be)
  uint8_t notScheduled = (expectation & captured) ^ expectation;

  // Find processes that were scheduled (but should not be)
  uint8_t wrongScheduled = (captured & ~expectation);

  // Output the error to the user
  lcd_clear();
  switch (strategy)
  {
  case OS_SS_ROUND_ROBIN:
    lcd_writeProgString(PSTR("Error RoundRobin: "));
    break;

  case OS_SS_DYNAMIC_PRIORITY_ROUND_ROBIN:
    lcd_writeProgString(PSTR("Error DPRR: "));
    break;
  }
  lcd_line2();

  // Find the first incorrect process id
  for (uint8_t k = 1; k < MAX_NUMBER_OF_PROCESSES; k++)
  {
    if (notScheduled & (1 << k))
    {
      lcd_writeDec(k);
      lcd_line2();
      lcd_writeProgString(PSTR("not schedulable"));
      break;
    }
    if (wrongScheduled & (1 << k))
    {
      lcd_writeDec(k);
      lcd_line2();
      lcd_writeProgString(PSTR("falsely sched."));
      break;
    }
  }

  // Wait forever
  while (1)
  {
  }
}

/*!
 *  Function that tests the given strategy on an artificial empty process array
 *  This ensures the strategy does schedule the idle process if necessary.
 */
void performScheduleIdleTest(scheduling_strategy_t strategy)
{

  lcd_clear();

  // Change scheduling strategy
  setActiveStrategy(strategy);

  // Test if first step is 0
  if (!strategyImplemented())
  {
    lcd_clear();
    lcd_writeProgString(PSTR("Not impl. or idle returned"));
    delayMs(LCD_DELAY);
    return;
  }

  // Backup process states
  process_state_t states[MAX_NUMBER_OF_PROCESSES];

  for (uint8_t i = 0; i < MAX_NUMBER_OF_PROCESSES; i++)
  {
    states[i] = os_getProcessSlot(i)->state;
    os_getProcessSlot(i)->state = OS_PS_UNUSED;
  }

  os_resetSchedulingInformation(strategy);

  uint8_t result = 0;
  for (uint8_t i = 0; i < MAX_NUMBER_OF_PROCESSES; i++)
  {

    switch (strategy)
    {
    case OS_SS_ROUND_ROBIN:
      result = os_scheduler_RoundRobin(os_getProcessSlot(0), i);
      break;
    case OS_SS_DYNAMIC_PRIORITY_ROUND_ROBIN:
      result = os_scheduler_DynamicPriorityRoundRobin(os_getProcessSlot(0), i);
      break;
    }

    if (result != 0)
      break;
  }

  // Restore process states
  for (uint8_t i = 0; i < MAX_NUMBER_OF_PROCESSES; i++)
  {
    os_getProcessSlot(i)->state = states[i];
  }

  os_resetSchedulingInformation(strategy);

  // Output success
  if (result == 0)
  {
    lcd_clear();
    lcd_writeProgString(PSTR("OK"));
    delayMs(LCD_DELAY * 0.5);

    return;
  }

  // Output the error to the user
  lcd_clear();
  switch (strategy)
  {
  case OS_SS_ROUND_ROBIN:
    lcd_writeProgString(PSTR("RoundRobin: "));
    break;

  case OS_SS_DYNAMIC_PRIORITY_ROUND_ROBIN:
    lcd_writeProgString(PSTR("DynamicPriority RoundRobin: "));
    break;
  }
  lcd_writeProgString(PSTR("Idle not scheduled"));

  // Wait forever
  while (1)
  {
  }
}

/*!
 * Program that deactivates the scheduler, spawns two programs
 * and performs the test
 */
PROGRAM(1, AUTOSTART)
{
  // Disable scheduler-timer
  cbi(TCCR2B, CS22);
  cbi(TCCR2B, CS21);
  cbi(TCCR2B, CS20);

  os_getProcessSlot(os_getCurrentProc())->priority = OS_PRIO_HIGH;
  os_exec(2, OS_PRIO_NORMAL);
  os_exec(3, OS_PRIO_LOW);

  scheduling_strategy_t strategies[] = {
      OS_SS_ROUND_ROBIN,
      OS_SS_DYNAMIC_PRIORITY_ROUND_ROBIN};

  uint8_t k = 0;
  uint8_t numStrategies = sizeof(strategies) / sizeof(scheduling_strategy_t);

#if PHASE_1 == 1

  lcd_clear();
  lcd_line1();
  lcd_writeProgString(PSTR("Phase 1:"));
  lcd_line2();
  lcd_writeProgString(PSTR("Strategies"));
  delayMs(LCD_DELAY);

  // Start strategies test
  for (k = 0; k < numStrategies; k++)
  {
    performStrategyTest(strategies[k]);
  }

#endif
#if PHASE_2 == 1

  lcd_clear();
  lcd_line1();
  lcd_writeProgString(PSTR("Phase 2:"));
  lcd_line2();
  lcd_writeProgString(PSTR("Idle"));
  delayMs(LCD_DELAY);

  // Start strategies test
  for (k = 0; k < numStrategies; k++)
  {
    performScheduleIdleTest(strategies[k]);
  }

#endif

  // Execute programs so all process slots are in use
  // (only if there are more than 4 process slots)
  os_exec(4, DEFAULT_PRIORITY);
  os_exec(5, DEFAULT_PRIORITY);
  os_exec(6, DEFAULT_PRIORITY);
  os_exec(7, DEFAULT_PRIORITY);

#if PHASE_3 == 1

  lcd_clear();
  lcd_line1();
  lcd_writeProgString(PSTR("Phase 3:"));
  lcd_line2();
  lcd_writeProgString(PSTR("Sched. All"));
  delayMs(LCD_DELAY);

  // Start schedulability test
  for (k = 0; k < numStrategies; k++)
  {
    performSchedulabilityTest(strategies[k], 0b11111110);
  }

#endif
#if PHASE_4 == 1

  lcd_clear();
  lcd_line1();
  lcd_writeProgString(PSTR("Phase 4:"));
  lcd_line2();
  lcd_writeProgString(PSTR("Sched. Partial"));
  delayMs(LCD_DELAY);

  // Reset some processes
  os_getProcessSlot(3)->state = OS_PS_UNUSED;

  // Start schedulability test
  for (k = 0; k < numStrategies; k++)
  {
    os_resetSchedulingInformation(strategies[k]);
    performSchedulabilityTest(strategies[k], 0b11110110);
  }

#endif

  // All tests passed
  while (1)
  {
    lcd_clear();
    lcd_writeProgString(PSTR("  TEST PASSED   "));
    delayMs(0.5 * LCD_DELAY);
    lcd_clear();
    delayMs(0.5 * LCD_DELAY);
  }
}

/*!
 *  Writes a given program ID to the capture array
 *  and calls the ISR manually afterwards
 *
 *  \param programID The ID which will be written to the capture array
 */
void testProgram(uint8_t programID)
{
  while (1)
  {
    if (i < 32)
    {
      capture[i++] = programID;
    }
    TIMER2_COMPA_vect();
  }
}

PROGRAM(2, DONTSTART)
{
  testProgram(2);
}

PROGRAM(3, DONTSTART)
{
  testProgram(3);
}

PROGRAM(4, DONTSTART)
{
  testProgram(4);
}

PROGRAM(5, DONTSTART)
{
  testProgram(5);
}

PROGRAM(6, DONTSTART)
{
  testProgram(6);
}

PROGRAM(7, DONTSTART)
{
  testProgram(7);
}

#endif