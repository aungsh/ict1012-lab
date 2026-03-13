#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  // These arrays will hold the "file descriptors" (IDs) for our tubes.
  // p2c[0] = Parent-to-Child Read end | p2c[1] = Parent-to-Child Write end
  int p2c[2]; 
  // c2p[0] = Child-to-Parent Read end | c2p[1] = Child-to-Parent Write end
  int c2p[2]; 
  
  char buf; // A tiny 1-byte bucket to hold our data during transport
  int pid;  // To store the ID of the process so we know if we are the Parent or Child

  // 1. CHECK INPUT: Did the user actually give us a character to send?
  if (argc != 2) {
    fprintf(2, "Usage: handshake <byte>\n");
    exit(1);
  }

  // 2. CREATE THE TUBES: We must build the pipes BEFORE we fork.
  // If we don't build them now, the Child won't "inherit" the same tubes.
  if (pipe(p2c) < 0 || pipe(c2p) < 0) {
    fprintf(2, "pipe failed\n");
    exit(1);
  }

  // 3. CLONE THE PROCESS: fork() creates a carbon copy of this program.
  pid = fork();

  if (pid < 0) {
    fprintf(2, "fork failed\n");
    exit(1);
  }

  // -------------------------------------------------------
  // CHILD PROCESS (The clone)
  // -------------------------------------------------------
  if (pid == 0) {
    // The Child is the "Listener" for p2c and the "Speaker" for c2p.
    // RULE: Always close the ends of the pipes you aren't using.
    close(p2c[1]); // Child will NOT write to Parent-to-Child pipe. Close it.
    close(c2p[0]); // Child will NOT read from Child-to-Parent pipe. Close it.

    // Child puts its ear to the p2c tube and waits for the Parent to drop a byte in.
    // read() is "blocking" – the Child sleeps here until data arrives.
    read(p2c[0], &buf, 1);

    printf("%d: received %c from parent\n", getpid(), buf);

    // Now the Child speaks into the c2p tube to send the byte back.
    write(c2p[1], &buf, 1);

    // Clean up: Close the ends we actually used before leaving.
    close(p2c[0]);
    close(c2p[1]);

    exit(0);
  }

  // -------------------------------------------------------
  // PARENT PROCESS (The original)
  // -------------------------------------------------------
  else {
    // The Parent is the "Speaker" for p2c and the "Listener" for c2p.
    close(p2c[0]); // Parent will NOT read from Parent-to-Child pipe.
    close(c2p[1]); // Parent will NOT write to Child-to-Parent pipe.

    // Grab the first character from the user's command line input.
    buf = argv[1][0];

    // Parent drops the byte into the p2c "speaking" end.
    write(p2c[1], &buf, 1);

    // Parent now puts its ear to the c2p "listening" end and waits for the Child's reply.
    read(c2p[0], &buf, 1);

    printf("%d: received %c from child\n", getpid(), buf);

    // Clean up.
    close(p2c[1]);
    close(c2p[0]);

    exit(0);
  }
}

// Plug-and-Play Template

// #include "kernel/types.h"
// #include "kernel/stat.h"
// #include "user/user.h"

// int main(int argc, char *argv[]) {
//   // --- SLOT A: VARIABLES (Change if using Strings/Ints/Multiple Pipes) ---
//   int p2c[2], c2p[2];
//   char buf; 
//   int pid;

//   // --- SLOT B: INPUT CHECK (Change if reading from stdin/looping) ---
//   if (argc != 2) { exit(1); }

//   // --- SLOT C: PIPES (Add more if doing a Ring/Grandchild) ---
//   pipe(p2c); pipe(c2p);

//   pid = fork();

//   if (pid == 0) { // CHILD
//     close(p2c[1]); close(c2p[0]);

//     // --- SLOT D: CHILD READ (Modify byte count if Int/String) ---
//     read(p2c[0], &buf, 1);

//     // --- SLOT E: THE LOGIC TWEAK (Add Case-Swap, Arithmetic, or Filter here) ---
//     // Example: buf = buf + 1;

//     // --- SLOT F: CHILD WRITE ---
//     write(c2p[1], &buf, 1);

//     close(p2c[0]); close(c2p[1]);
//     exit(0);
//   } 
//   else { // PARENT
//     close(p2c[0]); close(c2p[1]);

//     // --- SLOT G: PARENT WRITE (Change source if Interactive/Loop) ---
//     buf = argv[1][0];
//     write(p2c[1], &buf, 1);

//     // --- SLOT H: PARENT READ ---
//     read(c2p[0], &buf, 1);
//     printf("Result: %c\n", buf);

//     // --- SLOT I: CLEANUP (Add wait(0) here if output is garbled) ---
//     wait(0); 
//     exit(0);
//   }
// }


// Predictions for possible modifications

// 1. Data Transformation (Arithmetic or Case Swap)
// In this version, the child modifies the data before sending it back. This mimics the "Logic Tweak" pattern from Quiz 1.
// Modification: Child increments the byte (char) before sending back
// Look for: "The child should modify the data before returning it."

// if (pid == 0) {
//     close(p2c[1]); close(c2p[0]);
//     read(p2c[0], &buf, 1);
    
//     // LOGIC TWEAK: Increment the character (e.g., 'A' becomes 'B')
//     // Or use: if(buf >= 'a' && buf <= 'z') buf -= 32; for Uppercase
//     buf = buf + 1; 

//     printf("%d: child transformed data to %c\n", getpid(), buf);
//     write(c2p[1], &buf, 1);
//     close(p2c[0]); close(c2p[1]);
//     exit(0);
// }


// 2. String Exchange (Handling Buffers)
// If the quiz asks you to handle a whole string instead of a single byte, you must change the buffer size and the read/write byte counts.
// Look for: "Send the entire string" or "Send an integer."

// int main(int argc, char *argv[]) {
//     char buf[128]; // Use a buffer instead of a single char
//     // ... pipe and fork logic ...

//     if (pid == 0) {
//         close(p2c[1]); close(c2p[0]);
//         int n = read(p2c[0], buf, sizeof(buf)); // Read the whole string
        
//         // Example logic: Reverse the string
//         for(int i = 0; i < n/2; i++) {
//             char temp = buf[i];
//             buf[i] = buf[n-1-i];
//             buf[n-1-i] = temp;
//         }

//         write(c2p[1], buf, n);
//         close(p2c[0]); close(c2p[1]);
//         exit(0);
//     } else {
//         close(p2c[0]); close(c2p[1]);
//         write(p2c[1], argv[1], strlen(argv[1])); // Write the whole argument
//         int n = read(c2p[0], buf, sizeof(buf));
//         write(1, buf, n); // Print the result
//         // ...
//     }
// }



// 3. The "Ring" (Parent → Child → Grandchild → Parent)
// This is a high-probability "Challenge" modification. It tests if you can manage multiple pipes without deadlocking.
// Look for: "Three processes" or "Circular communication."

// int main(int argc, char *argv[]) {
//     int p2c[2], c2g[2], g2p[2]; // Three pipes for the ring
//     pipe(p2c); pipe(c2g); pipe(g2p);
//     char buf = argv[1][0];

//     if (fork() == 0) { // Child
//         if (fork() == 0) { // Grandchild
//             close(p2c[0]); close(p2c[1]); close(c2g[1]); close(g2p[0]);
//             read(c2g[0], &buf, 1);
//             printf("Grandchild got %c, passing to Parent\n", buf);
//             write(g2p[1], &buf, 1);
//             exit(0);
//         }
//         close(p2c[1]); close(g2p[0]); close(g2p[1]); close(c2g[0]);
//         read(p2c[0], &buf, 1);
//         printf("Child got %c, passing to Grandchild\n", buf);
//         write(c2g[1], &buf, 1);
//         wait(0); // Wait for grandchild
//         exit(0);
//     }
//     // Parent
//     close(p2c[0]); close(c2g[0]); close(c2g[1]); close(g2p[1]);
//     write(p2c[1], &buf, 1);
//     read(g2p[0], &buf, 1);
//     printf("Parent received back: %c\n", buf);
//     wait(0); // Wait for child
//     exit(0);
// }

// 4. Continuous Loop (Interactive)
// The parent reads from the user's keyboard (stdin) and sends it to the child until the user types 'q'.
// Look for: "Redirect stdin" or "Use file descriptor 0."

// // Inside Parent
// while(read(0, &buf, 1) > 0) { // Read from stdin (fd 0)
//     if (buf == 'q') break;
//     write(p2c[1], &buf, 1);
//     read(c2p[0], &buf, 1);
//     printf("Echo: %c\n", buf);
// }



// 5. Proper Synchronization (wait)
// A very simple tweak is requiring the parent to wait for the child to finish its print statements to avoid "garbled" output where the shell prompt appears before the child is done.
// Look for: "Ensure the child finishes printing before the parent exits."

// // Inside Parent block, before exit(0)
// close(p2c[1]);
// close(c2p[0]);
// wait(0); // This ensures child finishes printing before parent exits
// exit(0);



// 6. The "Bit-Flip" Challenge (Logic Tweak)
// In Quiz 1, they asked for an "Endian Swap." A common variation is asking the child to perform a bitwise operation before returning the byte.
// The Task: The parent sends a byte. The child must flip all bits (bitwise NOT) and send it back.
// Look for: "The child should flip all bits of the byte before returning it."

// // Inside Child
// read(p2c[0], &buf, 1);
// buf = ~buf; // Bitwise NOT: 10101010 becomes 01010101
// printf("%d: flipped bits to %x\n", getpid(), (unsigned char)buf);
// write(c2p[1], &buf, 1);



// 7. The "Filter" Handshake (Conditional Logic)
// This tests your ability to use if statements within the IPC flow.
// The Task: The parent sends a character. If it is a vowel, the child sends back 'V'. If it is a consonant, the child sends back 'C'. If it's a number, the child sends back 'N'.
// Look for: "The child should respond with 'V', 'C', or 'N' based on the character type."

// // Inside Child
// read(p2c[0], &buf, 1);
// char response;
// if (buf >= '0' && buf <= '9') {
//     response = 'N';
// } else if (buf == 'a' || buf == 'e' || buf == 'i' || buf == 'o' || buf == 'u') {
//     response = 'V';
// } else {
//     response = 'C';
// }
// write(c2p[1], &response, 1);



// 8. The "Sync-Point" (Process Synchronization)
// This modification tests if you understand how to use pipes to force processes to wait for each other without using wait().
// The Task: The Parent must print "A", then the Child must print "B", then the Parent must print "C".
// Look for: "Ensure the parent prints 'A', then the child prints 'B', and finally the parent prints 'C' using pipes for synchronization."

// // This requires two pipes to act as "signals"
// if (pid == 0) {
//     read(p2c[0], &buf, 1); // Wait for Parent's signal
//     printf("B\n");
//     write(c2p[1], "x", 1); // Signal Parent to continue
//     exit(0);
// } else {
//     printf("A\n");
//     write(p2c[1], "x", 1); // Signal Child to start
//     read(c2p[0], &buf, 1); // Wait for Child to finish
//     printf("C\n");
//     exit(0);
// }



// 9. Pipe Redirection with dup()
// The "sniffer" and "monitor" tasks in Lab 3 use dup(). There is a high chance the quiz will ask you to apply dup() to the handshake logic.
// The Task: Instead of using read(p2c[0], ...) explicitly, use dup() to redirect the child's Standard Input (0) to the pipe.
// Look for: "Redirect the child's standard input to read from the pipe using dup()."

// if (pid == 0) {
//     close(0);          // Close stdin
//     dup(p2c[0]);       // Pipe read end becomes the new stdin
//     close(p2c[0]);     // Close original
//     close(p2c[1]);
    
//     // Now the child can use 'gets' or 'read' on FD 0
//     read(0, &buf, 1); 
//     printf("%d: received %c via redirected stdin\n", getpid(), buf);
    
//     write(c2p[1], &buf, 1);
//     exit(0);
// }



// 10. Multi-Byte Integer Handshake
// Passing an int (4 bytes) instead of a char (1 byte) is a frequent trap.
// The Task: Send the integer 1234 from parent to child.
// Look for: "Send the integer 1234 from parent to child. Remember to use sizeof(int) in your read/write calls."

// // Parent side
// int val = 1234;
// write(p2c[1], &val, sizeof(int)); // Use sizeof(int), not 1!

// // Child side
// int received_val;
// read(p2c[0], &received_val, sizeof(int));
// printf("Child received integer: %d\n", received_val);





// 10 . This task requires careful management of file descriptors (pipes) and precise buffer handling to ensure exactly 4 bytes are transferred and padded/truncated correctly.

// Below is the complete implementation for user/handshake.c and the instructions for the Makefile.

// 1. Implementation: user/handshake.c

// This code follows the xv6 system call requirements and handles the 4-byte logic using a fixed-size buffer initialized to zero.
// C

// #include "kernel/types.h"
// #include "kernel/stat.h"
// #include "user/user.h"

// int main(int argc, char \*argv[]) {
// int p1[2], p2[2]; // p1: parent to child, p2: child to parent
// char buf[4] = {0, 0, 0, 0}; // Initialize with 0s for automatic padding

//     if (argc < 2) {
//         fprintf(2, "Usage: handshake <word>\n");
//         exit(1);
//     }

//     // Task 1a: Handle padding and truncation
//     // Copy up to 4 characters from argv[1] into our buffer
//     for (int i = 0; i < 4 && argv[1][i] != '\0'; i++) {
//         buf[i] = argv[1][i];
//     }

//     if (pipe(p1) < 0 || pipe(p2) < 0) {
//         fprintf(2, "pipe failed\n");
//         exit(1);
//     }

//     int pid = fork();
//     if (pid < 0) {
//         fprintf(2, "fork failed\n");
//         exit(1);
//     }

//     if (pid == 0) {
//         // --- CHILD PROCESS ---
//         // Task 1d: Close unused pipe ends
//         close(p1[1]); // Close write end of parent->child
//         close(p2[0]); // Close read end of child->parent

//         // Task 1b: Read exactly 4 bytes
//         char child_buf[4];
//         if (read(p1[0], child_buf, 4) != 4) {
//             fprintf(2, "child read error\n");
//             exit(1);
//         }

//         // Print received word (using %s works as we padded with \0)
//         printf("%d: child received %s from parent\n", getpid(), child_buf);

//         // Send the same 4-byte word back
//         write(p2[1], child_buf, 4);

//         close(p1[0]);
//         close(p2[1]);
//         exit(0);

//     } else {
//         // --- PARENT PROCESS ---
//         // Task 1d: Close unused pipe ends
//         close(p1[0]); // Close read end of parent->child
//         close(p2[1]); // Close write end of child->parent

//         // Task 1a: Send 4-byte word to child
//         write(p1[1], buf, 4);

//         // Task 1c: Read exactly 4 bytes from child
//         char parent_buf[4];
//         if (read(p2[0], parent_buf, 4) != 4) {
//             fprintf(2, "parent read error\n");
//             exit(1);
//         }

//         printf("%d: parent received %s from child\n", getpid(), parent_buf);

//         close(p1[1]);
//         close(p2[0]);
//         wait(0); // Wait for child to finish
//         exit(0);
//     }

// }

// 2. Updating the Makefile

// To ensure the program compiles and is included in the xv6 file system, you must add it to the UPROGS list in your Makefile.

//     Open the Makefile in the root directory.

//     Find the UPROGS\ section.

//     Add $U/_handshake\ to the list (ensure the backslash is present if it's not the last item).

// Example:
// Makefile

// UPROGS=\
//  $U/\_cat\
//  $U/\_echo\
//  $U/\_handshake\
//  ...

// Key Logic Breakdown

//     Padding/Truncation: By declaring char buf[4] = {0, 0, 0, 0}, we ensure that any byte not explicitly overwritten by the loop remains 0. The loop for (int i = 0; i < 4 && argv[1][i] != '\0'; i++) handles both truncation (by stopping at i=4) and short strings (by stopping at \0).

//     Two Pipes: Two pipes are necessary for full-duplex communication (bidirectional).

//         p1 handles Parent → Child.

//         p2 handles Child → Parent.

//     File Descriptor Management: It is a requirement of xv6 (and good Unix practice) to close the ends of the pipes you aren't using. For example, the child should never try to write to p1 or read from p2.









// ORIGINAL CODE
// #include "kernel/types.h"
// #include "kernel/stat.h"
// #include "user/user.h"

// int
// main(int argc, char *argv[])
// {
//   int p2c[2]; // parent to child pipe
//   int c2p[2]; // child to parent pipe
//   char buf;
//   int pid;

//   if (argc != 2) {
//     fprintf(2, "Usage: handshake <byte>\n");
//     exit(1);
//   }

//   if (pipe(p2c) < 0 || pipe(c2p) < 0) {
//     fprintf(2, "pipe failed\n");
//     exit(1);
//   }

//   pid = fork();

//   if (pid < 0) {
//     fprintf(2, "fork failed\n");
//     exit(1);
//   }

//   if (pid == 0) {
//     close(p2c[1]);
//     close(c2p[0]);

//     read(p2c[0], &buf, 1);

//     printf("%d: received %c from parent\n", getpid(), buf);

//     write(c2p[1], &buf, 1);

//     close(p2c[0]);
//     close(c2p[1]);

//     exit(0);
//   }

//   else {
//     close(p2c[0]);
//     close(c2p[1]);

//     buf = argv[1][0];

//     write(p2c[1], &buf, 1);

//     read(c2p[0], &buf, 1);

//     printf("%d: received %c from child\n", getpid(), buf);

//     close(p2c[1]);
//     close(c2p[0]);

//     exit(0);
//   }
// }
