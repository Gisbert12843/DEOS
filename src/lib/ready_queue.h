/*! \file
 *  \brief Struct specifying a process queue
 *
 *  Contains the struct and its functions that implement a circular buffer to store processes
 *
 *  \author   Fachbereich 5 - FH Aachen
 *  \date     2024
 *  \version  1.0
 */

#include "../os_process.h"
#include "defines.h"

#include <stdbool.h>

#ifndef _READY_QUEUE_H
#define _READY_QUEUE_H

#define READY_QUEUE_CAPACITY (MAX_NUMBER_OF_PROCESSES)
#define READY_QUEUE_SIZE (READY_QUEUE_CAPACITY + 1)

//! structure used to store processes and additional logic to implement a circular buffer
typedef struct ready_queue_t
{
	process_id_t processes[READY_QUEUE_SIZE];
	uint8_t head;
	uint8_t tail;
} ready_queue_t;

//! initializes a ready queue to be empty
void rq_init(ready_queue_t *queue);

//! pushes a ready onto the queue
void rq_push(ready_queue_t *queue, process_id_t process);

//! pops one ready of the queue and returns it
process_id_t rq_pop(ready_queue_t *queue);

//! check if queue is empty
bool rq_isEmpty(ready_queue_t *queue);

//! check if queue is full
bool rq_isFull(ready_queue_t *queue);

//! empties a ready queue
void rq_clear(ready_queue_t *queue);

//! removes process from the queue, returns true if succeeded
bool rq_remove(ready_queue_t *queue, process_id_t process);

//! prints all elements separated by ', '
void rq_print(ready_queue_t *queue);

#endif