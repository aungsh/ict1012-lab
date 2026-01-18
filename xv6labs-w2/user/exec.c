#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// exec.c: A simple wrapper that forks, execs a command, and waits.
// Usage inside xv6 shell:
//   exec echo hi ./wc
// Produces:
//   hi ./wc
// This mirrors the behavior needed by find's "-exec cmd" feature,
// where find will invoke: cmd <matched-file>.

int
main(int argc, char *argv[])
{
  if(argc < 2){
    fprintf(2, "usage: exec cmd [args...]\n");
    exit(1);
  }

  int pid = fork();
  if(pid < 0){
    fprintf(2, "exec: fork failed\n");
    exit(1);
  }
  if(pid == 0){
    // Child: run the requested command with given arguments.
    // argv[1] is the program; argv[1..] are its args; terminated by NULL.
    exec(argv[1], &argv[1]);
    // If exec returns, it failed.
    fprintf(2, "exec: exec %s failed\n", argv[1]);
    exit(1);
  }
  // Parent: wait for the child to finish.
  wait(0);
  exit(0);
}
