#include "kernel/types.h"   // Basic type definitions
#include "kernel/stat.h"    // File statistics (not used directly here)
#include "user/user.h"      // User-space functions: printf, read, fork, exec, wait, etc.
#include "kernel/param.h"   // System parameters (e.g., MAXARG for max number of arguments)

int main(int argc, char *argv[]) {
    char *cmdargv[MAXARG];   // Array to hold command + arguments (like argv[])
    int cmd_argc = 0;        // Number of fixed command-line arguments (before reading input)
    int n_flag = 0;          // Flag for -n option (run command for each input separately)
    int arg_start = 1;       // Index in argv[] where actual command starts

    // Check if first argument is "-n" (optional flag)
    if (argc >= 3 && strcmp(argv[1], "-n") == 0) {
        n_flag = 1;          // Set the flag
        arg_start = 3;       // Command arguments start after "-n" and command itself
    }

    // Copy command arguments from argv[] into cmdargv[]
    for (int i = arg_start; i < argc; i++) {
        cmdargv[cmd_argc++] = argv[i];
    }

    char buf[512];              // Buffer to read input from stdin
    int n;                      // Number of bytes read
    int current_cmd_argc = cmd_argc; // Keep track of current argument count including input
    static char argbuf[512];    // Temporary buffer to build one argument from stdin
    int p = 0;                  // Position in argbuf

    // Read from standard input (stdin) in chunks
    while ((n = read(0, buf, sizeof(buf))) > 0) {
        for (int i = 0; i < n; i++) {
            // If character is a space, tab, or newline → argument separator
            if (buf[i] == ' ' || buf[i] == '\t' || buf[i] == '\n') {
                if (p == 0) continue; // Skip consecutive separators

                argbuf[p] = '\0';     // Null-terminate argument string

                // Allocate memory and copy argument into a new string
                char *t = malloc(p + 1);
                memcpy(t, argbuf, p + 1);

                // Add argument to cmdargv
                cmdargv[current_cmd_argc++] = t;

                p = 0;                 // Reset buffer position for next argument

                // If -n flag is set, execute command for this single argument
                if (n_flag) {
                    if (fork() == 0) {     // Child process
                        cmdargv[current_cmd_argc] = 0; // Null-terminate cmdargv
                        exec(cmdargv[0], cmdargv);    // Run the command
                        exit(1);                     // Exit child if exec fails
                    }
                    wait(0);                   // Parent waits for child to finish
                    current_cmd_argc = cmd_argc; // Reset argument count for next input
                }
            } else {
                // If not a separator, add character to argbuf
                argbuf[p++] = buf[i];
            }
        }
    }

    // If -n is not set, execute command once with all accumulated arguments
    if (!n_flag && current_cmd_argc > cmd_argc) {
        if (fork() == 0) {                // Child process
            cmdargv[current_cmd_argc] = 0; // Null-terminate cmdargv
            exec(cmdargv[0], cmdargv);     // Execute command with all arguments
            exit(1);                       // Exit child if exec fails
        }
        wait(0);                           // Parent waits for child to finish
    }

    exit(0);                               // Exit program successfully
}


// #include "kernel/types.h"
// #include "kernel/stat.h"
// #include "user/user.h"
// #include "kernel/param.h"

// int main(int argc, char *argv[]) {
//     char *cmdargv[MAXARG];
//     int cmd_argc = 0;
//     int n_flag = 0;
//     int arg_start = 1;

//     if (argc >= 3 && strcmp(argv[1], "-n") == 0) {
//         n_flag = 1;
//         arg_start = 3; 
//     }

//     for (int i = arg_start; i < argc; i++) {
//         cmdargv[cmd_argc++] = argv[i];
//     }

//     char buf[512];
//     int n;
//     int current_cmd_argc = cmd_argc;
//     static char argbuf[512];
//     int p = 0;

//     while ((n = read(0, buf, sizeof(buf))) > 0) {
//         for (int i = 0; i < n; i++) {
//             if (buf[i] == ' ' || buf[i] == '\t' || buf[i] == '\n') {
//                 if (p == 0) continue;
//                 argbuf[p] = '\0';
                
//                 char *t = malloc(p + 1);
//                 memcpy(t, argbuf, p + 1);
//                 cmdargv[current_cmd_argc++] = t;
//                 p = 0;

//                 if (n_flag) {
//                     if (fork() == 0) {
//                         cmdargv[current_cmd_argc] = 0;
//                         exec(cmdargv[0], cmdargv);
//                         exit(1);
//                     }
//                     wait(0);
//                     current_cmd_argc = cmd_argc;
//                 }
//             } else {
//                 argbuf[p++] = buf[i];
//             }
//         }
//     }

//     if (!n_flag && current_cmd_argc > cmd_argc) {
//         if (fork() == 0) {
//             cmdargv[current_cmd_argc] = 0;
//             exec(cmdargv[0], cmdargv);
//             exit(1);
//         }
//         wait(0);
//     }

//     exit(0);
// }