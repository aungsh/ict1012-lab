#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  // 1. ARGUMENT CHECK:
  // We need at least 3 parts: the program name (monitor), the mask (number), 
  // and the command to run (e.g., "grep"). 
  if (argc < 3) {
    fprintf(2, "Usage: monitor mask command [args...]\n");
    exit(1);
  }

  // 2. SET THE MASK:
  // Convert the first argument (string) to an integer (mask) using atoi. 
  // This calls your new system call 'monitor' to save the mask in the kernel's 
  // proc structure. [cite: 645, 663, 671]
  if (monitor(atoi(argv[1])) < 0) {
    fprintf(2, "monitor: failed to set mask\n");
    exit(1);
  }

  // 3. EXECUTE THE COMMAND:
  // argv[2] is the command to run (e.g., "grep"). [cite: 660, 695]
  // &argv[2] is the array of arguments starting from that command.
  // Example: monitor 32 grep hello README
  // argv[0] = "monitor", argv[1] = "32", argv[2] = "grep", argv[3] = "hello"...
  // exec will run "grep" with arguments "hello" and "README". 
  exec(argv[2], &argv[2]);
  
  // 4. ERROR HANDLING:
  // exec() only returns if it fails (e.g., the command doesn't exist). [cite: 695]
  // If it succeeds, this part of the code is never reached because the 
  // process has been replaced by the new command.
  fprintf(2, "monitor: exec %s failed\n", argv[2]);
  exit(1);
}



// 1. Modifying the Print Timing

// The current lab prints the information when the system call is about to return.
// The Change: Modify the kernel to print the information before the system call executes (on entry) instead of on exit.
// The Change: Print both the start and the end of the system call to show how long it took (using uptime).
// Look for: "Modify the kernel to print the syscall information at the start of the syscall and also at the end, including the duration in ticks."

// Phase 1: Modifying kernel/syscall.c (Timing & Formatting)

// In the standard lab, the printf is placed after the system call finishes. To change the timing and include the duration (how long it took), you need to capture the uptime (current ticks) at both the start and the end.
// 1. Modify the syscall(void) function

// Find this function in kernel/syscall.c. Here is the logic for printing on Entry, on Exit, and calculating the Duration.

// void
// syscall(void)
// {
//   int num;
//   struct proc *p = myproc(); // [cite: 670]
//   uint64 start_ticks, end_ticks; // Variables to store time

//   num = p->trapframe->a7; // The syscall number is stored in register a7

//   if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    
//     // --- PART 1: PRINT ON ENTRY ---
//     // Check if the bit for this syscall is set in the monitor_mask [cite: 647]
//     if((p->monitor_mask >> num) & 1) {
//       start_ticks = uptime(); // Record the start time in ticks [cite: 220]
//       printf("%d: syscall %s (START) at ticks %d\n", p->pid, syscall_names[num], start_ticks);
//     }

//     // This is the actual execution of the system call
//     p->trapframe->a0 = syscalls[num](); // [cite: 675]

//     // --- PART 2: PRINT ON EXIT & DURATION ---
//     if((p->monitor_mask >> num) & 1) {
//       end_ticks = uptime(); // Record the end time [cite: 220]
//       uint64 duration = end_ticks - start_ticks; // Calculate duration
      
//       // Print the PID, name, return value, and duration [cite: 648]
//       printf("%d: syscall %s -> %d (DURATION: %d ticks)\n", 
//              p->pid, syscall_names[num], p->trapframe->a0, duration);
//     }
//   } else {
//     printf("%d %s: unknown sys call %d\n", p->pid, p->name, num);
//     p->trapframe->a0 = -1;
//   }
// }


// Phase 2: Necessary Supporting Definitions

// For the code above to work, you must ensure the kernel can look up the system call names and has access to the uptime function.
// 2. Update syscall_names in kernel/syscall.c

// You must have this array defined so the kernel can print "read" instead of just a number like "5".

// static const char *syscall_names[] = {
//   [SYS_fork]    "fork",    // [cite: 684, 685]
//   [SYS_exit]    "exit",    // [cite: 686, 687]
//   [SYS_wait]    "wait",    // [cite: 688, 689]
//   [SYS_pipe]    "pipe",    // [cite: 690, 691]
//   [SYS_read]    "read",    // [cite: 692]
//   [SYS_kill]    "kill",    // [cite: 693]
//   [SYS_exec]    "exec",    // [cite: 694, 695]
//   [SYS_fstat]   "fstat",   // [cite: 696, 697]
//   [SYS_chdir]   "chdir",   // [cite: 698, 699]
//   [SYS_dup]     "dup",     // [cite: 700]
//   [SYS_getpid]  "getpid",  // [cite: 701]
//   [SYS_sbrk]    "sbrk",    // [cite: 702]
//   [SYS_sleep]   "pause",   // [cite: 703]
//   [SYS_uptime]  "uptime",  // [cite: 704]
//   [SYS_open]    "open",    // [cite: 705]
//   [SYS_write]   "write",   // [cite: 706]
//   [SYS_mknod]   "mknod",   // [cite: 707]
//   [SYS_unlink]  "unlink",  // [cite: 708]
//   [SYS_link]    "link",    // [cite: 709]
//   [SYS_mkdir]   "mkdir",   // [cite: 710]
//   [SYS_close]   "close",   // [cite: 711]
//   [SYS_monitor] "monitor", // [cite: 712]
// };

// 3. Ensure uptime() is accessible
// The uptime() function is a kernel function that returns the number of ticks since boot. It is usually declared in kernel/defs.h. If you get a "function not found" error, ensure it is included.






// 2. Output Formatting (The "memdump" Style)

// The lab requires printing the PID, the name, and the return value.
// The Change: Change the format of the output (e.g., "PID [X] called [NAME] and got [RESULT]").
// The Change: Convert the return value to Hexadecimal instead of decimal, similar to the memdump task.
// The Change: Print the system call name in ALL CAPS.


// Phase 1: Modifying the Output String
// In kernel/syscall.c, find the syscall(void) function. You will change the printf line to match the new descriptive format and use the %x (hexadecimal) specifier for the return value.

// // Inside kernel/syscall.c, within the syscall() function

// void
// syscall(void)
// {
//   int num;
//   struct proc *p = myproc(); // [cite: 670]

//   num = p->trapframe->a7; // Get syscall number from register a7
//   if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    
//     // Execute the syscall and store the return value in a0
//     p->trapframe->a0 = syscalls[num](); // [cite: 674]

//     // Check the bitmask to see if we should monitor this call [cite: 647]
//     if((p->monitor_mask >> num) & 1) {
      
//       // CHANGE 1: New descriptive format
//       // CHANGE 2: %x instead of %d for Hexadecimal return value
//       printf("PID [%d] called [%s] and got [0x%x]\n", 
//              p->pid, syscall_names[num], p->trapframe->a0);
//     }
//   } 
//   // ... rest of function
// }

// Phase 2: System Call Names in ALL CAPS
// To make the system call names appear in ALL CAPS, you must modify the strings within the syscall_names array.


// // Inside kernel/syscall.c
// // CHANGE 3: Capitalize the strings in the names array
// static const char *syscall_names[] = {
//   [SYS_fork]    "FORK",
//   [SYS_exit]    "EXIT",
//   [SYS_wait]    "WAIT",
//   [SYS_pipe]    "PIPE",
//   [SYS_read]    "READ",
//   [SYS_kill]    "KILL",
//   [SYS_exec]    "EXEC",
//   [SYS_fstat]   "FSTAT",
//   [SYS_chdir]   "CHDIR",
//   [SYS_dup]     "DUP",
//   [SYS_getpid]  "GETPID",
//   [SYS_sbrk]    "SBRK",
//   [SYS_pause]   "PAUSE",
//   [SYS_uptime]  "UPTIME",
//   [SYS_open]    "OPEN",
//   [SYS_write]   "WRITE",
//   [SYS_mknod]   "MKNOD",
//   [SYS_unlink]  "UNLINK",
//   [SYS_link]    "LINK",
//   [SYS_mkdir]   "MKDIR",
//   [SYS_close]   "CLOSE",
//   [SYS_monitor] "MONITOR",
// };


// Feature	            Original (Lab 3)	            Modification (Quiz Style)
// Return Specifier	  %d (Decimal)                  %x or 0x%x (Hexadecimal) 
// String Case	        Lowercase (e.g., "read")      Uppercase (e.g., "READ")
// Format	            pid: syscall name -> result   PID [X] called [NAME] and got [RESULT]





// 3. Conditional Tracing (The "sixfive" Style)

// Instead of using a bitmask to decide which syscall to trace, the quiz might add secondary conditions.
// The Change: Only trace system calls if the return value is negative (indicating an error, like -1).
// The Change: Only trace a specific process ID (e.g., "Only trace if the PID is 5").
// The Change: Only trace system calls that were called more than a certain number of times (requiring a counter in the proc struct). 


// 1. Only Trace Negative Return Values (Errors)

// This is a common debugging modification. You only want to see the "failed" system calls.
// Where to modify: kernel/syscall.c inside the syscall() function.

// // Inside kernel/syscall.c, within the syscall() function
// void
// syscall(void)
// {
//   struct proc *p = myproc(); [cite: 670]
//   int num = p->trapframe->a7; // Get syscall number 

//   if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
//     p->trapframe->a0 = syscalls[num](); // Execute syscall 

//     // --- CONDITION: Bitmask IS set AND the return value is NEGATIVE ---
//     if(((p->monitor_mask >> num) & 1) && ((int)p->trapframe->a0 < 0)) {
//         printf("%d: syscall %s -> %d (ERROR DETECTED)\n", 
//                p->pid, syscall_names[num], p->trapframe->a0);
//     }
//   }
// }


// 2. Only Trace a Specific Process ID (PID)
// The quiz might ask you to monitor a process and its children only if the starting PID matches a specific number.
// Where to modify: kernel/syscall.c.

// // Inside kernel/syscall.c
// if(((p->monitor_mask >> num) & 1) && (p->pid == 5)) {
//     printf("%d: syscall %s -> %d\n", p->pid, syscall_names[num], p->trapframe->a0);
// }


// 3. Only Trace After X Calls (The Counter)

// This is the most complex because it requires a new variable in the process structure to keep track of the count.
// Step 1: Modify kernel/proc.h
// Add a counter to the proc structure.

// struct proc {
//   // ... existing fields
//   uint32 monitor_mask; [cite: 662]
//   int syscall_count;   // <--- NEW: To track how many calls have occurred
// };

// Step 2: Modify kernel/proc.c
// Initialize the counter to 0 when a process is created (in allocproc) and ensure it's copied or reset in fork.

// Step 3: Modify kernel/syscall.c
// Increment the counter and check the condition.

// // Inside kernel/syscall.c
// if((p->monitor_mask >> num) & 1) {
//     p->syscall_count++; // Increment count for every monitored call
    
//     // CONDITION: Only print if we have called syscalls more than 10 times
//     if(p->syscall_count > 10) {
//         printf("%d: [%d] syscall %s -> %d\n", 
//                p->pid, p->syscall_count, syscall_names[num], p->trapframe->a0);
//     }
// }


// Quiz Modification	             Requirement	                Logic to Use
// Error Only	                   Return value is -1	          if(p->trapframe->a0 == -1)
// PID Specific	                 Specific process only	      if(p->pid == X)
// Frequency Filter	             After N calls	              p->count++; if(p->count > N)
// Combined	                     Error in PID 5	              if(p->pid == 5 && p->trapframe->a0 < 0)


// Dumbass-Proof Tip for the Quiz

// In xv6, the return value of a system call is stored in p->trapframe->a0. If you want to check if it failed, always look at that specific register. Remember to cast it to an int if you are checking for negative values, as a0 is technically a uint64.
// Would you like to move on to the Logic/Mask Tweaks (like the "Negative Mask" where you trace everything except what's in the mask)?





// 4. Logic/Mask Tweaks
// The current mask uses bits to specify the syscall.
// The Change: Instead of a bitmask, use a negative mask (trace everything except the calls specified in the mask).
// The Change: Implement a "Trace Level." For example, Level 1 prints only the name; Level 2 prints the name and the return value.


// 1. The Negative Mask (Inverted Logic)

// In the original lab, a bit is 1 to trace that call. In a "Negative Mask" scenario, the logic is flipped: if a bit is 1, you do not trace it.
// Where to modify: kernel/syscall.c inside the syscall() function.

// // Inside kernel/syscall.c
// void
// syscall(void)
// {
//   struct proc *p = myproc();
//   int num = p->trapframe->a7;

//   if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
//     p->trapframe->a0 = syscalls[num]();

//     // --- LOGIC TWEAK: NEGATIVE MASK ---
//     // Original: if ((p->monitor_mask >> num) & 1)
//     // Negative: We only print if the bit is 0
//     if (!((p->monitor_mask >> num) & 1)) {
//         printf("%d: syscall %s -> %d\n", p->pid, syscall_names[num], p->trapframe->a0);
//     }
//   }
// }


// 2. Trace Levels (Detail Control)

// The quiz might ask you to change how much info is printed based on the "level" provided in the mask. For example, if the user calls monitor(1), show names only; if they call monitor(2), show names and return values.

// Step 1: Update user/monitor.c (The Wrapper)
// The user-level program usually passes the mask directly. If the quiz uses "levels," you might need to handle two arguments.

// Step 2: Update kernel/syscall.c (The Detail Logic)
// Instead of just checking if a bit is ON or OFF, you check the value of the mask.


// // Inside kernel/syscall.c
// if(p->monitor_mask > 0) { // Check if monitoring is enabled at all
    
//     if(p->monitor_mask == 1) {
//         // Level 1: Name only
//         printf("PID [%d] called [%s]\n", p->pid, syscall_names[num]);
//     } 
//     else if(p->monitor_mask == 2) {
//         // Level 2: Name and Return Value (Full detail)
//         printf("PID [%d] called [%s] -> Return: %d\n", 
//                p->pid, syscall_names[num], p->trapframe->a0);
//     }
// }

// 3. The "Exclude List" (Combining both)

// A more advanced version might ask you to monitor everything by default, but exclude specific calls.
// User Command: monitor 32 grep hello (where 32 is 1 << SYS_read).
// Kernel Logic: Trace every syscall except read.

// // In kernel/syscall.c
// // Check if the bit is NOT set. 
// // If it's NOT set, it's not in our "exclude list," so we print it.
// if (!((p->monitor_mask >> num) & 1)) {
//     printf("%d: syscall %s -> %d\n", p->pid, syscall_names[num], p->trapframe->a0);
// }


// Quiz Requirement	      Kernel Condition Change
// Normal (Lab 3)	        if ((mask >> num) & 1) 
// Negative/Exclude	      if (!((mask >> num) & 1))
// Trace Everything	      if (mask == 0x7FFFFFFF) 
// Trace Nothing	          if (mask == 0)

// Pro-Tip for the Quiz:
// Remember that monitor_mask is a uint32. If the quiz asks for a level, they might just use a simple number like 1 or 2. If they ask for specific calls, you must use the bit-shifting logic (1 << SYS_name).
// Would you like me to show you the "Inheritance" modifications next? This involves changing fork() so that kids don't automatically "spy" like their parents.













// 5. Inheritance and Process Control

// The current lab copies the mask from parent to child in fork().
// The Change: Disable inheritance. The child should start with a mask of 0 even if the parent is being monitored.
// The Change: Automatically stop monitoring after a certain number of system calls have been printed.
// The Change: Add a system call to "clear" the monitor mask for the current process. 


// 1. Disable Inheritance in fork()
// In the standard Lab 3 task, you are instructed to modify fork() (specifically kfork() in kernel/proc.c) to ensure children inherit the monitor_mask. To disable this, you simply ensure the child's mask is initialized to zero.
// Where to modify: kernel/proc.c inside the fork() function.


// // Inside kernel/proc.c
// int
// fork(void)
// {
//   // ... existing code to allocate child process np ...

//   // THE CHANGE: Explicitly set child mask to 0
//   // Instead of: np->monitor_mask = p->monitor_mask;
//   np->monitor_mask = 0; 

//   // ... rest of the fork logic ...
// }


// 2. Auto-Stop Monitoring (The "Self-Destruct" Counter)

// This modification requires the kernel to turn off the monitor_mask automatically once a limit is reached. You will need a counter in the proc structure to track this.
// Step 1: Update kernel/proc.h

// struct proc {
//   // ... existing fields
//   uint32 monitor_mask;
//   int monitor_limit; // How many more calls to trace before stopping
// };


// Step 2: Update kernel/syscall.c
// Modify the logic so that every time a print occurs, the limit decreases. When it hits zero, the mask is wiped.

// // Inside kernel/syscall.c, within syscall()
// if((p->monitor_mask >> num) & 1) {
//   printf("%d: syscall %s -> %d\n", p->pid, syscall_names[num], p->trapframe->a0);

//   // THE CHANGE: Decrement limit and check if we should stop
//   if(p->monitor_limit > 0) {
//     p->monitor_limit--;
//     if(p->monitor_limit == 0) {
//       p->monitor_mask = 0; // Turn off monitoring automatically
//     }
//   }
// }


// 3. Add a "Clear" System Call
// This is a "Challenge" style task similar to adding the hello or endianswap syscalls from previous labs. You would create a specific system call to reset a process's monitoring status.
// Kernel Implementation (kernel/sysproc.c):

// uint64
// sys_monitor_clear(void)
// {
//   struct proc *p = myproc();
//   p->monitor_mask = 0; // Wipe the mask
//   return 0; // Success
// }

// User-Space Test Program (user/unmonitor.c):

// #include "kernel/types.h"
// #include "user/user.h"

// int main() {
//   monitor_clear(); // Call the new syscall
//   printf("Monitoring disabled for this process.\n");
//   exit(0);
// }


// Modification	            File to Change	            Key Logic
// No Inheritance	          kernel/proc.c	              Set np->monitor_mask = 0; in fork(). 
// Auto-Stop	                kernel/syscall.c	          if(--p->limit == 0) p->mask = 0;
// Clear Mask	               kernel/sysproc.c	          New syscall that sets p->monitor_mask = 0.






// 6. Argument Extraction (Advanced)

// The lab specifically says "No need to print the system call arguments".
// The Change: Challenge Task: Print the first argument of specific system calls (e.g., if tracing open, print the filename string). This would require using argstr() or argint() inside syscall.c.


// Step-by-Step Implementation in kernel/syscall.c

// You need to modify the syscall() function to check for specific system calls and then pull the arguments from the trapframe.
// 1. Modify the syscall() logic

// Inside kernel/syscall.c, add logic to extract the first argument if the call is SYS_open.

// void
// syscall(void)
// {
//   int num;
//   struct proc *p = myproc();
//   char path[128]; // Buffer to hold the filename string

//   num = p->trapframe->a7; // System call number [cite: 646]
  
//   if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    
//     // --- ADVANCED ARGUMENT EXTRACTION ---
//     // If monitoring is ON and the call is 'open', grab the filename
//     if (((p->monitor_mask >> num) & 1) && num == SYS_open) {
//         // argstr(0, ...) fetches the 0th argument (the string pointer)
//         // and copies it into our 'path' buffer 
//         if(argstr(0, path, sizeof(path)) < 0) {
//             path[0] = '\0'; // Failed to fetch
//         }
//     }

//     // Execute the system call [cite: 675]
//     p->trapframe->a0 = syscalls[num]();

//     // --- ENHANCED PRINTING ---
//     if((p->monitor_mask >> num) & 1) {
//       if(num == SYS_open) {
//         // Print the PID, name, extracted filename, and result
//         printf("%d: syscall %s (\"%s\") -> %d\n", 
//                p->pid, syscall_names[num], path, p->trapframe->a0);
//       } else {
//         // Default print for other calls [cite: 648]
//         printf("%d: syscall %s -> %d\n", 
//                p->pid, syscall_names[num], p->trapframe->a0);
//       }
//     }
//   }
// }


// How to Extract Different Argument Types

// Depending on the quiz, you might need to extract an integer or a pointer instead of a string.

//     For Integers (e.g., read fd or sleep ticks):
//     Use argint(int n, int *ip).
//     Example: argint(0, &fd); // Gets the file descriptor.

//     For Strings (e.g., open path or exec path):
//     Use argstr(int n, char *buf, int max).
//     Example: argstr(0, buf, sizeof(buf)); // Gets the filename string.

//     For Pointers/Addresses:
//     Use argaddr(int n, uint64 *ip).
//     Example: argaddr(1, &addr); // Gets the buffer address for a read/write call.


// Quiz Goal	                  Key Function	                  Context / Use Case
// Log Files Opened	          argstr(0, ...)	                Find out which file grep or cat is looking at.
// Log Write Sizes	            argint(2, ...)	                For write(fd, buf, n), this gets n (the byte count).
// Log Memory Growth	          argint(0, ...)	                For sbrk(n), this shows how much memory was requested.



// original code

// #include "kernel/types.h"
// #include "user/user.h"

// int main(int argc, char *argv[]) {
//   if (argc < 3) {
//     fprintf(2, "Usage: monitor mask command [args...]\n");
//     exit(1);
//   }

//   if (monitor(atoi(argv[1])) < 0) {
//     fprintf(2, "monitor: failed to set mask\n");
//     exit(1);
//   }

//   // Execute the command starting at argv[2]
//   exec(argv[2], &argv[2]);
  
//   // If exec returns, it failed
//   exit(0);
// }


// Here is the step-by-step guide on what to modify and where.
// Phase 1: Registering the System Call

// Xv6 needs to know that a new system call called monitor exists before it can compile any code that uses it.

//     kernel/syscall.h:
//         Action: Add a unique number for your new system call.
//         What to change: Add #define SYS_monitor 22 (or the next available number) at the end of the list.

//     user/user.h:
//         Action: Add the function prototype so user programs (like monitor.c) can call it.
//         What to change: Add int monitor(int); to the system calls section.

//     user/usys.pl:
//         Action: This script generates the assembly code that actually triggers the system call.
//         What to change: Add entry("monitor"); at the bottom of the file.

//     Makefile:
//         Action: Add your test program to the OS image.
//         What to change: Add $U/_monitor to the UPROGS list.

// Phase 2: Modifying the Process Structure

// The kernel needs a way to "remember" which system calls a specific process wants to monitor.

//     kernel/proc.h:
//         Action: Update the proc structure (the kernel's record for each process).
//         What to change: Inside struct proc, add uint32 monitor_mask;.

//     kernel/proc.c:
//         Action: Ensure that when a process forks, the child inherits the "monitor" settings from the parent.
//         What to change: In the fork() function, find where the new process (np) is being initialized and add np->monitor_mask = p->monitor_mask;.

// Phase 3: Implementing Kernel Logic

// Now you implement the actual "brain" of the monitor feature.

//     kernel/sysproc.c:
//         Action: Write the handler function that runs when a user calls monitor(mask).
//         What to change: Implement sys_monitor(void). It must use argint() to grab the mask from the user and save it into the current process's monitor_mask.

//     kernel/syscall.c:
//         Action: This is where the actual printing happens.
//         What to change:
//             Add extern uint64 sys_monitor(void); at the top.
//             Add [SYS_monitor] sys_monitor, to the syscalls[] array.
//             Create an array of strings called syscall_names[] so the kernel can print "read" instead of just "SYS_5".
//             Crucial Step: Modify the syscall() function. Before it returns, check if the current bit in the monitor_mask is set. If it is, printf the PID, the syscall name, and the return value.
