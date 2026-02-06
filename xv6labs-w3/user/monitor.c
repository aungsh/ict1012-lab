#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  if (argc < 3) {
    fprintf(2, "Usage: monitor mask command [args...]\n");
    exit(1);
  }

  if (monitor(atoi(argv[1])) < 0) {
    fprintf(2, "monitor: failed to set mask\n");
    exit(1);
  }

  // Execute the command starting at argv[2]
  exec(argv[2], &argv[2]);
  
  // If exec returns, it failed
  exit(0);
}