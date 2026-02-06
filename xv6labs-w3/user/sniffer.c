#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"

int
main(int argc, char *argv[])
{
  int pages = 100;
  int size = pages * 4096;
  char *p = sbrk(size);
  
  if(p == (char*)-1) {
    printf("sbrk failed\n");
    exit(1);
  }

  char *marker = "This may help.";

  for (int i = 0; i < size - 64; i++) {
    if (p[i] == 'T' && memcmp(&p[i], marker, 14) == 0) {
      printf("%s\n", &p[i] + 16);
      exit(0);
    }
  }
  
  exit(1);
}