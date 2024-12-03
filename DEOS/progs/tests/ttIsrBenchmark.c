//-------------------------------------------------
//          TestSuite: ISR Benchmark
//-------------------------------------------------
// Measures performance of scheduler ISR
// Do not optimize by omitting safety features!
//-------------------------------------------------
#include "../progs.h"
#if defined(TESTTASK_ENABLED) && TESTTASK == TT_ISR_Benchmark

#include "../../lib/lcd.h"
#include "../../lib/stop_watch.h"
#include "../../lib/util.h"
#include "../../os_core.h"
#include "../../os_scheduler.h"
#include <avr/interrupt.h>

#define MAX_ISR_DURATION 200 // in micro seconds

// Internals:
#define BENCHMARK_SAMPLE_COUNT 100
#define TESTCASE_COUNT 6

time_t benchmarks[TESTCASE_COUNT];

void deactiveateAutoScheduling()
{
	cli();
	// set prescaler to 0 -> deactivated
	cbi(TCCR2B, CS22);
	cbi(TCCR2B, CS21);
	cbi(TCCR2B, CS20);
	sei();
}

time_t runBenchmark()
{
	time_t sum = 0;

	for (uint8_t i = 0; i < BENCHMARK_SAMPLE_COUNT; ++i)
	{
		stop_watch_handler_t handler = stopWatch_start();
		os_yield();
		sum += stopWatch_stop(handler);
	}

	return sum / BENCHMARK_SAMPLE_COUNT;
}

void stage1()
{
	INFO("Running stage 1");

	os_setSchedulingStrategy(OS_SS_ROUND_ROBIN);
	benchmarks[0] = runBenchmark();

	os_setSchedulingStrategy(OS_SS_DYNAMIC_PRIORITY_ROUND_ROBIN);
	benchmarks[1] = runBenchmark();
}

void stage2()
{
	INFO("Running stage 2");
	
	volatile uint8_t data[STACK_SIZE_PROC - 128] = {0}; //  If "stack pointer error" occurs, make `data` smaller until it fits
	data[0] = data[1];								   // prevent 'unused' warning

	os_setSchedulingStrategy(OS_SS_ROUND_ROBIN);
	benchmarks[2] = runBenchmark();

	os_setSchedulingStrategy(OS_SS_DYNAMIC_PRIORITY_ROUND_ROBIN);
	benchmarks[3] = runBenchmark();
}

void stage3()
{
	INFO("Running stage 3");

	process_id_t procs[MAX_NUMBER_OF_PROCESSES - 2]; // idle process and me are running already
	for (uint8_t i = 0; i < MAX_NUMBER_OF_PROCESSES - 2; ++i)
	{
		procs[i] = os_exec(2, OS_PRIO_HIGH);
	}

	os_setSchedulingStrategy(OS_SS_ROUND_ROBIN);
	benchmarks[4] = runBenchmark() / (MAX_NUMBER_OF_PROCESSES - 1);

	os_setSchedulingStrategy(OS_SS_DYNAMIC_PRIORITY_ROUND_ROBIN);
	benchmarks[5] = runBenchmark() / MAX_NUMBER_OF_PROCESSES;

	for (uint8_t i = 0; i < MAX_NUMBER_OF_PROCESSES - 2; ++i)
	{
		os_kill(procs[i]);
	}
}

// A process just so you have one that the ISR needs to handle
PROGRAM(2, DONTSTART)
{
	while (1)
	{
		os_yield();
	}
}

// Main program
PROGRAM(1, AUTOSTART)
{

	deactiveateAutoScheduling();

	INFO("Welcome to testcase 5!");

	stage1();
	stage2();
	stage3();

	// Test results
	uint8_t passed = 0;
	for (uint8_t i = 0; i < TESTCASE_COUNT; ++i)
	{
		if (benchmarks[i] <= MAX_ISR_DURATION)
		{
			passed++;
		}
	}

	// Output overall test result on LCD:
	lcd_clear();
	if (passed == TESTCASE_COUNT)
	{
		LCD("  TEST PASSED   ");
	}
	else
	{
		LCD("  TEST FAILED   ");
	}

	// Output results on terminal:
	INFO("");
	INFO("Test result: %d/%d testcases passed", passed, TESTCASE_COUNT);
	INFO("");
	INFO("Testcase   | Description                  | Number of processes | Stack usage | Result");
	INFO("-----------|------------------------------|---------------------|-------------|------------------------------------------------");
	INFO("Testcase 1 | Round Robin                  | 2 processes         | normal      | took %lu of max. %d microseconds - %s", (unsigned long)benchmarks[0], MAX_ISR_DURATION, benchmarks[0] <= MAX_ISR_DURATION ? "PASSED" : "FAILED");
	INFO("Testcase 2 | Dynamic Priority Round Robin | 2 processes         | normal      | took %lu of max. %d microseconds - %s", (unsigned long)benchmarks[1], MAX_ISR_DURATION, benchmarks[1] <= MAX_ISR_DURATION ? "PASSED" : "FAILED");
	INFO("Testcase 3 | Round Robin                  | 2 processes         | heavy       | took %lu of max. %d microseconds - %s", (unsigned long)benchmarks[2], MAX_ISR_DURATION, benchmarks[2] <= MAX_ISR_DURATION ? "PASSED" : "FAILED");
	INFO("Testcase 4 | Dynamic Priority Round Robin | 2 processes         | heavy       | took %lu of max. %d microseconds - %s", (unsigned long)benchmarks[3], MAX_ISR_DURATION, benchmarks[3] <= MAX_ISR_DURATION ? "PASSED" : "FAILED");
	INFO("Testcase 5 | Round Robin                  | 8 processes         | normal      | took %lu of max. %d microseconds - %s", (unsigned long)benchmarks[4], MAX_ISR_DURATION, benchmarks[4] <= MAX_ISR_DURATION ? "PASSED" : "FAILED");
	INFO("Testcase 6 | Dynamic Priority Round Robin | 8 processes         | normal      | took %lu of max. %d microseconds - %s", (unsigned long)benchmarks[5], MAX_ISR_DURATION, benchmarks[5] <= MAX_ISR_DURATION ? "PASSED" : "FAILED");

	// Delay for previous lcd output
	delayMs(1000);

	// Output results on LCD:
	while (1)
	{
		lcd_clear();

		for (uint8_t i = 0; i < TESTCASE_COUNT; ++i)
		{
			lcd_goto(i % 2, i / 2 % 2 * 8);

			char mark = benchmarks[i] > MAX_ISR_DURATION ? 'F' : 'P'; // F = failed, P = passed

			LCD("%d%c %d", i + 1, mark, benchmarks[i]);

			if (i % 4 == 3)
			{
				delayMs(3000);
				lcd_clear();
			}
		}

		delayMs(3000);
	}
}

#endif