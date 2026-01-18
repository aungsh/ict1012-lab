#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

int main(int argc, char *argv[]) {
    char *cmdargv[MAXARG];
    int cmd_argc = 0;
    int n_flag = 0;
    int arg_start = 1;

    if (argc >= 3 && strcmp(argv[1], "-n") == 0) {
        n_flag = 1;
        arg_start = 3; 
    }

    for (int i = arg_start; i < argc; i++) {
        cmdargv[cmd_argc++] = argv[i];
    }

    char buf[512];
    int n;
    int current_cmd_argc = cmd_argc;
    static char argbuf[512];
    int p = 0;

    while ((n = read(0, buf, sizeof(buf))) > 0) {
        for (int i = 0; i < n; i++) {
            if (buf[i] == ' ' || buf[i] == '\t' || buf[i] == '\n') {
                if (p == 0) continue;
                argbuf[p] = '\0';
                
                char *t = malloc(p + 1);
                memcpy(t, argbuf, p + 1);
                cmdargv[current_cmd_argc++] = t;
                p = 0;

                if (n_flag) {
                    if (fork() == 0) {
                        cmdargv[current_cmd_argc] = 0;
                        exec(cmdargv[0], cmdargv);
                        exit(1);
                    }
                    wait(0);
                    current_cmd_argc = cmd_argc;
                }
            } else {
                argbuf[p++] = buf[i];
            }
        }
    }

    if (!n_flag && current_cmd_argc > cmd_argc) {
        if (fork() == 0) {
            cmdargv[current_cmd_argc] = 0;
            exec(cmdargv[0], cmdargv);
            exit(1);
        }
        wait(0);
    }

    exit(0);
}