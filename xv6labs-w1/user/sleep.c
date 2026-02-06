// Provides basic kernel type definitions (e.g. uint, ushort)
#include "kernel/types.h"

// Provides file status structures (not strictly needed here,
// but commonly included in xv6 user programs)
#include "kernel/stat.h"

// Provides user-space system calls and library functions
// such as exit(), atoi(), pause(), fprintf()
#include "user/user.h"

int
main(int argc, char *argv[])
{
  // argc = argument count
  // argv = argument vector (array of strings)

  // This program expects exactly ONE argument:
  // the number of clock ticks to sleep
  if(argc != 2){
    // File descriptor 2 = standard error
    // Print usage message if argument count is wrong
    fprintf(2, "Usage: sleep ticks\n");

    // Exit with non-zero value to indicate error
    exit(1);
  }

  // Convert the argument (string) to an integer
  // argv[1] contains the number of ticks as text
  int ticks = atoi(argv[1]);

  // pause() is a system call that blocks the process
  // for the specified number of clock ticks
  pause(ticks);

  // Exit successfully after sleeping
  exit(0);
}
