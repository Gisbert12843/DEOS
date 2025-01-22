// Testtasks for exercise 1
#define TT_INIT					10
#define TT_RESUME				11
#define TT_MULTIPLE				12

// Testtasks for exercise 2
#define TT_SCHEDULING			20
#define TT_TERMINATION			21
#define TT_STACK_COLLISION		22
#define TT_STACK_CONSISTENCY	23
#define TT_YIELD				24
#define TT_ISR_Benchmark		25

// Testtasks for exercise 3
#define TT_COMMUNICATION		30
#define TT_PROTOCOLSTACK        31
#define TT_CONIFGXBEE           32

// Testtasks for exercise 4
#define TT_SENSOR_DATA			40
#define TT_TLCD					41

///////////////////////////////////////////////////////////////////////////////
// Configure what program-set should be active: testtasks or your user progs
///////////////////////////////////////////////////////////////////////////////

// Set to 1 to run test tasks, set to 0 to run user programs
#define ENABLE_TESTTASK	0

// Will run user_progs/user_progx.c if ENABLE_TESTTASK is set to 0
#define USER_PROGRAM	5

// Will run tests/testx.c
#define TESTTASK TT_SENSOR_DATA

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


// You don't need to bother what's here
#if ENABLE_TESTTASK == 1
#define TESTTASK_ENABLED
#else
#define USER_PROGRAM_ENABLED
#endif