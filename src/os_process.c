#include "os_process.h"

/*!
 *  Checks whether given process is runnable
 *
 *  \param process A pointer on the process
 *  \return The state of the process, true if it is ready or running, false otherwise
 */
bool os_isRunnable(process_t const *process)
{
	if (process && (process->state == OS_PS_READY || process->state == OS_PS_RUNNING))
		return true;

	return false;
}
