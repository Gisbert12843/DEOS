/*! \file
 *  \brief Scheduling library for the OS.
 *
 *  Contains the scheduler and context switching functionality for the OS.
 *
 *  \author   Fachbereich 5 - FH Aachen
 *  \date     2024
 *  \version  1.0
 */

#ifndef _OS_SCHEDULING_STRATEGIES_H
#define _OS_SCHEDULING_STRATEGIES_H


#include <stdint.h>
#include <stdio.h>
#include "lib/defines.h"
#include "lib/ready_queue.h"
#include "os_scheduler.h"

//! Structure used to store specific scheduling informations
typedef struct SchedulingInformation
{
	ready_queue_t queues_ready[PRIORITY_COUNT];
} scheduling_information_t;

//! Used to reset the SchedulingInfo for one process
void os_resetProcessSchedulingInformation(scheduling_strategy_t strategy, process_id_t id);

//! Used to reset the SchedulingInfo for a strategy
void os_resetSchedulingInformation(scheduling_strategy_t strategy);

//! RoundRobin strategy
process_id_t os_scheduler_RoundRobin(process_t const processes[], process_id_t current);

//! DynamicPriorityRoundRobin strategy
process_id_t os_scheduler_DynamicPriorityRoundRobin(process_t const processes[], process_id_t current);

#endif
