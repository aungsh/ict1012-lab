#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int p2c[2]; // parent to child pipe
  int c2p[2]; // child to parent pipe
  char buf;
  int pid;

  if (argc != 2) {
    fprintf(2, "Usage: handshake <byte>\n");
    exit(1);
  }

  if (pipe(p2c) < 0 || pipe(c2p) < 0) {
    fprintf(2, "pipe failed\n");
    exit(1);
  }

  pid = fork();

  if (pid < 0) {
    fprintf(2, "fork failed\n");
    exit(1);
  }

  if (pid == 0) {
    close(p2c[1]);
    close(c2p[0]);

    read(p2c[0], &buf, 1);

    printf("%d: received %c from parent\n", getpid(), buf);

    write(c2p[1], &buf, 1);

    close(p2c[0]);
    close(c2p[1]);

    exit(0);
  }

  else {
    close(p2c[0]);
    close(c2p[1]);

    buf = argv[1][0];

    write(p2c[1], &buf, 1);

    read(c2p[0], &buf, 1);

    printf("%d: received %c from child\n", getpid(), buf);

    close(p2c[1]);
    close(c2p[0]);

    exit(0);
  }
}
