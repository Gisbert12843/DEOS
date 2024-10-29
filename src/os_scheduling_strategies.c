/*! \file

 *  Scheduling strategies used by the Interrupt Service RoutineA from Timer 2 (in scheduler.c)
 *  to determine which process may continue its execution next.

 *  The file contains two strategies:
 *  -round-robin
 *  -dynamic-priority-round-robin
*/

#include "os_scheduling_strategies.h"
#include "defines.h"
#include "ready_queue.h"

#include <stdbool.h>
#include <stdlib.h>

//----------------------------------------------------------------------------
// Globals
//----------------------------------------------------------------------------

scheduling_information_t schedulingInfo; // initialization to 0 fits our needs

//----------------------------------------------------------------------------
// Given functions
//----------------------------------------------------------------------------

/*!
 *  Function used to determine whether there is any process ready (except the idle process)
 *
 *  \param processes[] The array of processes that it supposed to be looked through for processes that are ready
 *  \return True if there is a process ready which is not the idle proc
 */
bool isAnyProcReady(process_t const processes[])
{
  process_id_t i;
  for (i = 1; i < MAX_NUMBER_OF_PROCESSES; i++)
  {
    // The moment we find a single process that is ready/running, we can already return True
    if (processes[i].state == OS_PS_READY)
    {
      return true;
    }
  }

  // Not a single process that is ready/running has been found in the loop, so there is none
  return false;
}

//----------------------------------------------------------------------------
// Your Homework
//----------------------------------------------------------------------------

/*!
 *  This function implements the round-robin strategy. Every process gets the same
 *  amount of processing time and is rescheduled after each scheduler call
 *  if there are other processes running other than the idle process.
 *  The idle process is executed if no other process is ready for execution
 *
 *  \param processes An array holding the processes to choose the next process from.
 *  \param current The id of the current process.
 *  \return The next process to be executed determined on the basis of the even strategy.
 */
process_id_t os_scheduler_RoundRobin(process_t const processes[], process_id_t current)
{
  os_setSchedulingStrategy(OS_SS_ROUND_ROBIN);

  process_id_t currentProc = os_getCurrentProc();

  // Look for the next proc that is ready, there has to be at least one, this has been checked before. Don't choose 0 as it is the idle process
  for (uint8_t i = currentProc; i < MAX_NUMBER_OF_PROCESSES; i++)
  {
    if (processes[i].state == OS_PS_READY)
    {
      return i; // Return resulting process id
    }
  }

  // If no process except idle process ready, choose idle process
  return 0; // Return resulting process id
}

/*!
 * Reset the scheduling information for a specific process slot
 * This is necessary when a new process is started to clear out any
 * leftover data from a process that previously occupied that slot
 *
 * \param id  The process slot to erase state for
 */
void os_resetProcessSchedulingInformation(scheduling_strategy_t strategy, process_id_t id)
{
#warning[Praktikum 2] Implement here
}

/*!
 *  Reset the scheduling information for a specific strategy
 *  This is only relevant for DynamicPriorityRoundRobin
 *  and is done when the strategy is changed through os_setSchedulingStrategy
 *
 * \param strategy  The strategy to reset information for
 */
void os_resetSchedulingInformation(scheduling_strategy_t strategy)
{
#warning[Praktikum 2] Implement here
}

/*!
 *  This function implements the dynamic-priority-round-robin strategy.
 *  In this strategy, process priorities will matter that's achieved through multiple ready queues
 *
 *  \param processes An array holding the processes to choose the next process from.
 *  \param current The id of the current process.
 *  \return The next process to be executed determined on the basis of the dynamic priority round-robin strategy.
 */
process_id_t os_scheduler_DynamicPriorityRoundRobin(process_t const processes[], process_id_t current)
{
#warning[Praktikum 2] Implement here

  // 1. Move processes one higher in priority

  // 2. Push current process to the ready queue

  // 3. Get next process from ready queue
}
