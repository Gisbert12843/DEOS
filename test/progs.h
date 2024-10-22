
// Set to 1 to run test tasks, set to 0 to run user programs
#define ENABLE_TESTTASK 1

// Will run user_progs/user_progx.c if ENABLE_TESTTASK is set to 0
#define USER_PROGRAM 1

// Will run tests/testx.c
#define TESTTASK 5





// You don't need to bother what's here
#if ENABLE_TESTTASK == 1
#define TESTTASK_ENABLED
#else
#define USER_PROGRAM_ENABLED
#endif