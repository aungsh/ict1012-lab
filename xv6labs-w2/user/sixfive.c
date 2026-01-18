#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define BUFSIZE 32

int
main(int argc, char *argv[])
{
  char ch;
  char numbuf[BUFSIZE];
  char *separators = " -\r\t\n./,";

  if (argc < 2) {
    printf("usage: sixfive filename\n");
    exit(1);
  }

  for (int i = 1; i < argc; i++) {
    int fd = open(argv[i], 0);
    if (fd < 0) {
      printf("sixfive: cannot open %s\n", argv[i]);
      continue;
    }

    int idx = 0;

    while (read(fd, &ch, 1) == 1) {
      if (ch >= '0' && ch <= '9') {
        if (idx < BUFSIZE - 1) {
          numbuf[idx++] = ch;
        }
      } else if (strchr(separators, ch)) {
        if (idx > 0) {
          numbuf[idx] = '\0';
          int num = atoi(numbuf);

          if (num % 5 == 0 || num % 6 == 0) {
            printf("%d\n", num);
          }
          idx = 0;
        }
      }
    }

    if (idx > 0) {
      numbuf[idx] = '\0';
      int num = atoi(numbuf);

      if (num % 5 == 0 || num % 6 == 0) {
        printf("%d\n", num);
      }
    }

    close(fd);
  }

  exit(0);
}
