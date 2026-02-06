#include "kernel/types.h"   // Basic type definitions (like int, char)
#include "kernel/stat.h"    // For file statistics (not used directly here)
#include "user/user.h"      // User-space library: open, read, printf, exit

#define BUFSIZE 32          // Maximum size for storing a number as a string

int main(int argc, char *argv[])
{
    char ch;                // Variable to store each character read from file
    char numbuf[BUFSIZE];   // Buffer to accumulate digits of a number
    char *separators = " -\r\t\n./,"; // Characters that separate numbers in text

    // Check if at least one filename is provided
    if (argc < 2) {
        printf("usage: sixfive filename\n");
        exit(1);            // Exit program if no filename is given
    }

    // Loop over all filenames provided as command-line arguments
    for (int i = 1; i < argc; i++) {
        int fd = open(argv[i], 0);   // Open file in read-only mode (0)
        if (fd < 0) {                // If opening fails
            printf("sixfive: cannot open %s\n", argv[i]);
            continue;                // Skip to next file
        }

        int idx = 0;                 // Index to keep track of position in numbuf

        // Read file **one character at a time**
        while (read(fd, &ch, 1) == 1) {
            if (ch >= '0' && ch <= '9') {       // If character is a digit
                if (idx < BUFSIZE - 1) {        // Make sure we don't overflow buffer
                    numbuf[idx++] = ch;        // Add digit to buffer and increment index
                }
            } else if (strchr(separators, ch)) { // If character is a separator
                if (idx > 0) {                  // Only process if buffer has digits
                    numbuf[idx] = '\0';        // Null-terminate string to form a number
                    int num = atoi(numbuf);    // Convert string to integer

                    // Check if number is divisible by 5 or 6
                    if (num % 5 == 0 || num % 6 == 0) {
                        printf("%d\n", num);   // Print number if condition met
                    }
                    idx = 0;                    // Reset buffer index for next number
                }
            }
            // Any other characters are ignored
        }

        // After reading file, check if there's a number left in the buffer
        if (idx > 0) {
            numbuf[idx] = '\0';
            int num = atoi(numbuf);
            if (num % 5 == 0 || num % 6 == 0) {
                printf("%d\n", num);
            }
        }

        close(fd);   // Close file descriptor before moving to next file
    }

    exit(0);        // Exit program successfully
}




// #include "kernel/types.h"
// #include "kernel/stat.h"
// #include "user/user.h"

// #define BUFSIZE 32

// int
// main(int argc, char *argv[])
// {
//   char ch;
//   char numbuf[BUFSIZE];
//   char *separators = " -\r\t\n./,";

//   if (argc < 2) {
//     printf("usage: sixfive filename\n");
//     exit(1);
//   }

//   for (int i = 1; i < argc; i++) {
//     int fd = open(argv[i], 0);
//     if (fd < 0) {
//       printf("sixfive: cannot open %s\n", argv[i]);
//       continue;
//     }

//     int idx = 0;

//     while (read(fd, &ch, 1) == 1) {
//       if (ch >= '0' && ch <= '9') {
//         if (idx < BUFSIZE - 1) {
//           numbuf[idx++] = ch;
//         }
//       } else if (strchr(separators, ch)) {
//         if (idx > 0) {
//           numbuf[idx] = '\0';
//           int num = atoi(numbuf);

//           if (num % 5 == 0 || num % 6 == 0) {
//             printf("%d\n", num);
//           }
//           idx = 0;
//         }
//       }
//     }

//     if (idx > 0) {
//       numbuf[idx] = '\0';
//       int num = atoi(numbuf);

//       if (num % 5 == 0 || num % 6 == 0) {
//         printf("%d\n", num);
//       }
//     }

//     close(fd);
//   }

//   exit(0);
// }
