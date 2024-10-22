#include "ready_queue.h"
#include "os_core.h"
#include "lcd.h"

#define next(index) ((index + 1) < READY_QUEUE_SIZE ? (index + 1) : 0)
#define prev(index) (index > 0 ? (index - 1) : (READY_QUEUE_SIZE - 1))

/*!
 *  Initializes a ready queue to be empty
 *
 *  \param queue The queue that needs to be initialized
 */
void rq_init(ready_queue_t *queue)
{
	queue->head = 0;
	queue->tail = 0;
}

/*!
 *  Pushes a process onto the queue
 *
 *  \param queue The queue it will push to
 *  \param process The process that will be pushed
 */
void rq_push(ready_queue_t *queue, process_id_t process)
{
	if (rq_isFull(queue))
	{
		os_error("Can't push on full ready queue");
	}
	queue->processes[queue->tail] = process;
	queue->tail = next(queue->tail);
}

/*!
 *  Pops one process of the queue and returns it
 *
 *  \param queue The queue it will push to
 *  \param process The process that will be pushed
 *  \return Popped process id
 */
process_id_t rq_pop(ready_queue_t *queue)
{
	if (rq_isEmpty(queue))
	{
		os_error("Can't pop from empty ready queue");
	}
	process_id_t process = queue->processes[queue->head];
	queue->head = next(queue->head);
	return process;
}

/*!
 *  Check if queue is empty
 *
 *  \param queue The queue it will check
 *  \return True if queue is empty, false if it's not empty
 */
bool rq_isEmpty(ready_queue_t *queue)
{
	return queue->head == queue->tail;
}

/*!
 *  Check if queue is full
 *
 *  \param queue The queue it will check
 *  \return True if queue is full, false if it's not completely filled
 */
bool rq_isFull(ready_queue_t *queue)
{
	return next(queue->tail) == queue->head;
}

/*!
 *  Empties a ready queue
 *
 *  \param queue The queue that should be emptied
 */
void rq_clear(ready_queue_t *queue)
{
	queue->head = 0;
	queue->tail = 0;
}

/*!
 *  Removes process from the queue, returns true if succeeded.
 *  Please use with care, it has an O(n) complexity
 *
 *  \param queue The queue that should be removed from
 *  \param process The process that should be removed
 *  \return True if the process was removed, false if it wasn't found
 */
bool rq_remove(ready_queue_t *queue, process_id_t process)
{
	for (int i = queue->head; i != queue->tail; i = next(i))
	{
		if (queue->processes[i] == process)
		{
			for (int j = i; j != queue->tail; j = next(j))
			{
				queue->processes[j] = queue->processes[next(j)];
			}
			queue->tail = prev(queue->tail);
			return true;
		}
	}
	return false;
}

/*!
 *  Prints all elements separated by ', '.
 *  Most left element would be popped first.
 *
 *  \param queue The queue that should be printed
 */
void rq_print(ready_queue_t *queue)
{
	for (int j = queue->head; j != queue->tail; j = next(j))
	{
		lcd_writeDec(queue->processes[j]);
		if (next(j) != queue->tail)
		{
			lcd_writeProgString(PSTR(", "));
		}
	}
}