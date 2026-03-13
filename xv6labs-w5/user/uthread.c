#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

/* Thread States */
#define FREE        0x0 // Slot is empty
#define RUNNING     0x1 // Currently using the CPU
#define RUNNABLE    0x2 // Waiting for a turn to run

#define STACK_SIZE  8192
#define MAX_THREAD  4

/* THE VAULT: Stores registers when a thread is NOT running */
struct thread_context {
  uint64 ra;  // Return Address: Where to jump when thread resumes [cite: 831, 834]
  uint64 sp;  // Stack Pointer: The thread's private memory "workspace" [cite: 831, 833]
  uint64 s0;  // Frame pointer [cite: 831]
  uint64 s1;  // s1-s11 are "Saved Registers" that must be preserved [cite: 831, 832]
  uint64 s2;
  uint64 s3;
  uint64 s4;
  uint64 s5;
  uint64 s6;
  uint64 s7;
  uint64 s8;
  uint64 s9;
  uint64 s10;
  uint64 s11;
};

struct thread {
  char       stack[STACK_SIZE]; /* 8KB of private memory for this thread  */
  int        state;             
  struct thread_context context; /* The "snapshot" of the CPU for this thread [cite: 829, 835] */
};

struct thread all_thread[MAX_THREAD];
struct thread *current_thread;

/* Assembly function to do the actual swapping  */
extern void thread_switch(uint64, uint64);
              
void 
thread_init(void)
{
  // Main() is the "first" thread. It starts as RUNNING.
  current_thread = &all_thread[0];
  current_thread->state = RUNNING;
}

void 
thread_schedule(void)
{
  struct thread *t, *next_thread;

  /* FINDER: Look for the next RUNNABLE thread in a circle (Round Robin)  */
  next_thread = 0;
  t = current_thread + 1;
  for(int i = 0; i < MAX_THREAD; i++){
    if(t >= all_thread + MAX_THREAD)
      t = all_thread;
    if(t->state == RUNNABLE) {
      next_thread = t;
      break;
    }
    t = t + 1;
  }

  if (next_thread == 0) {
    printf("thread_schedule: no runnable threads\n");
    exit(-1);
  }

  /* THE SWAP: If we found someone else to run [cite: 849] */
  if (current_thread != next_thread) {         
    next_thread->state = RUNNING;
    t = current_thread; // t is the "old" thread we are leaving
    current_thread = next_thread; // current_thread is the "new" one we are entering

    /* * a0 = address of old context (&t->context) 
     * a1 = address of new context (&next_thread->context) [cite: 857, 863]
     */
    thread_switch((uint64)&t->context, (uint64)&next_thread->context);
  } else
    next_thread = 0;
}

void 
thread_create(void (*func)())
{
  struct thread *t;

  // Find an empty FREE slot [cite: 823]
  for (t = all_thread; t < all_thread + MAX_THREAD; t++) {
    if (t->state == FREE) break;
  }
  t->state = RUNNABLE;

  /* STACK SETUP: RISC-V stacks grow DOWNWARD.
   * We must point 'sp' to the very END of the stack buffer.
   */
  t->context.sp = (uint64)(t->stack + STACK_SIZE);

  /* ENTRY POINT: When thread_switch calls 'ret', jump to this function [cite: 846, 874] */
  t->context.ra = (uint64) func;
}

void 
thread_yield(void)
{
  current_thread->state = RUNNABLE; // Put myself back in the "waiting" line
  thread_schedule(); // Ask the scheduler to pick someone else
}





// .text
//     .globl thread_switch
// thread_switch:
//     # ---------------------------------------------------------
//     # PART 1: SAVE THE PAST (Old Thread)
//     # ---------------------------------------------------------
//     # a0 points to the old thread's context struct.
//     # sd = "Store Doubleword" (Register -> Memory) [cite: 854, 855]
//     # Numbers (0, 8, 16) are byte-offsets in the struct[cite: 861].
    
//     sd ra, 0(a0)   # Save old Return Address [cite: 857]
//     sd sp, 8(a0)   # Save old Stack Pointer [cite: 858]
//     sd s0, 16(a0)  # Save old Frame Pointer
//     sd s1, 24(a0)  # Save saved register s1
//     # ... (saves s2 through s10) ...
//     sd s11, 104(a0)

//     # ---------------------------------------------------------
//     # PART 2: RESTORE THE FUTURE (New Thread)
//     # ---------------------------------------------------------
//     # a1 points to the new thread's context struct.
//     # ld = "Load Doubleword" (Memory -> Register) [cite: 854, 862]
    
//     ld ra, 0(a1)   # Load the new Return Address [cite: 863]
//     ld sp, 8(a1)   # Load the new Stack Pointer. WE ARE NOW ON NEW STACK! [cite: 863, 865]
//     ld s0, 16(a1)  
//     ld s1, 24(a1)
//     # ... (loads s2 through s10) ...
//     ld s11, 104(a1)

//     # ---------------------------------------------------------
//     # PART 3: THE JUMP
//     # ---------------------------------------------------------
//     # ret jumps to the address currently in 'ra'[cite: 867, 871].
//     # Since we just loaded 'ra' from the new thread, we jump into 
//     # that thread's code (e.g., thread_a)[cite: 871, 874].
//     ret


// If the quiz asks to...	              You should modify...
// Change Stack Size	                    #define STACK_SIZE in uthread.c.
// Increase Max Threads	                #define MAX_THREAD in uthread.c.
// Add a Register to Context	            Add it to struct thread_context and update offsets in .S.
// Debug Stack issues	                  Check if thread_create uses stack + STACK_SIZE (Top of stack).
// Change Scheduler Logic	               The for loop inside thread_schedule.








// 1. Register Set Modifications (Context Extension)

// Adding specific registers: The quiz might ask you to save and restore more registers than the callee-saved ones (e.g., adding tp or gp).
// Floating Point support: You could be asked to add support for floating-point registers (ft0-ft11) to the thread_context struct.
// Context Structure reordering: Forcing a specific order in the thread_context struct to test if you can correctly calculate offsets (0, 8, 16...) in the assembly file.

// 1. Adding Specific Registers (tp, gp)

// These registers are often used for thread pointers or global pointers. Adding them requires expanding the "snapshot" your code takes of the CPU.
// Update user/uthread.c:
// Add the new registers to your structure.

// struct thread_context {
//   uint64 ra;
//   uint64 sp;
//   uint64 gp; // NEW: Global Pointer (+16 offset)
//   uint64 tp; // NEW: Thread Pointer (+24 offset)
//   uint64 s0; // Now at +32 offset
//   // ... rest of the saved registers ...
// };


// Update user/uthread_switch.S:
// You must update the offsets for all registers that follow the newly inserted ones.

// thread_switch:
//     # Save old
//     sd ra, 0(a0)
//     sd sp, 8(a0)
//     sd gp, 16(a0)  # NEW
//     sd tp, 24(a0)  # NEW
//     sd s0, 32(a0)  # OFFSET CHANGED from 16 to 32
//     # ...
    
//     # Restore new
//     ld ra, 0(a1)
//     ld sp, 8(a1)
//     ld gp, 16(a1)
//     ld tp, 24(a1)
//     ld s0, 32(a1)
//     # ...
//     ret

// 2. Floating Point Support (ft0-ft11)

// If the quiz asks for floating-point support, you use the same logic but must use specific floating-point instructions (fsd to store and fld to load).
// Update user/uthread.c:

// struct thread_context {
//   // ... integer registers ...
//   uint64 ft0; // Floating point register 0
//   uint64 ft1; 
//   // ... up to ft11 ...
// };


// Update user/uthread_switch.S:

// # Save Floating Point (fsd = Floating-point Store Doubleword)
//     fsd ft0, 112(a0) 
//     fsd ft1, 120(a0)
    
//     # Restore Floating Point (fld = Floating-point Load Doubleword)
//     fld ft0, 112(a1)
//     fld ft1, 120(a1)


// 3. Context Structure Reordering

// The quiz may swap the order (e.g., putting s11 first and ra last) to see if you are just memorizing the lab code or if you actually understand how sd and ld offsets work.
// The Rule for Offsets:
// The offset in assembly must be exactly Index of Variable in Struct * 8.


// If Struct is...	                Then Offset in Assembly is...
// uint64 s11; (1st)	              sd s11, 0(a0) 
// uint64 ra; (2nd)	              sd ra, 8(a0) 
// uint64 sp; (3rd)	              sd sp, 16(a0)

// Dumbass-Proof Tip: If you add a register at the bottom of the struct, 
// you don't have to change any of the existing assembly offsets. 
// If you add it at the top or middle, you must recount and rewrite every single line of the assembly file to match the new positions.







// 2. Stack Pointer and Memory Logic

// Stack Growth direction: While RISC-V stacks grow downward, a trick question might ask you to simulate a system where stacks grow upward, requiring you to change how sp is initialized in thread_create.
// Dynamic Stack Size: Modifying the program to accept a stack size as a command-line argument using atoi, similar to the sleep lab.
// Stack Guard/Overflow detection: Adding a "magic number" at the bottom of the stack buffer and checking if it has been overwritten whenever a thread yields.


// 1. Upward-Growing Stacks

// In standard RISC-V, stacks grow downward, meaning the sp starts at the highest address and decreases. If a quiz asks for an upward-growing stack, you must flip this logic so the sp starts at the lowest address.
// Modification in user/uthread.c:
// Instead of adding STACK_SIZE to the base address, you point directly to the base of the array.

// void 
// thread_create(void (*func)())
// {
//   struct thread *t;
//   // ... (code to find FREE thread) ...

//   // TRICK: Point to the START of the array for an upward-growing stack
//   t->context.sp = (uint64)(t->stack); 

//   t->context.ra = (uint64)func;
//   t->state = RUNNABLE;
// }


// 2. Dynamic Stack Size

// Just like the sleep lab used atoi to set ticks, the quiz might ask you to set a custom stack size via command-line arguments.
// Steps to modify:
//     Remove the #define STACK_SIZE and replace it with a global variable.
//     Use malloc (from user/user.h) to allocate the stack instead of a fixed-size char array.
// Modification in user/uthread.c:


// int dynamic_stack_size = 8192; // Default

// int main(int argc, char *argv[]) {
//   // Check for command line argument
//   if(argc >= 2) {
//     dynamic_stack_size = atoi(argv[1]);
//   }
//   // ... (rest of init) ...
// }

// void thread_create(void (*func)()) {
//   struct thread *t;
//   // ... (find FREE thread) ...
  
//   // Allocate memory dynamically
//   char *new_stack = malloc(dynamic_stack_size);
//   t->context.sp = (uint64)(new_stack + dynamic_stack_size); // Downward
//   t->context.ra = (uint64)func;
// }



// 3. Stack Guard / Overflow Detection

// This is a security feature to ensure a thread doesn't "leak" out of its allocated 8192 bytes.
// Logic: Place a "Magic Number" (Canary) at the very bottom of the stack (the lowest address). Before every context switch, check if that number is still there.
// Modification in user/uthread.c:

// #define STACK_MAGIC 0xDEADBEEF

// void thread_create(void (*func)()) {
//   struct thread *t;
//   // ...
  
//   // Place the guard at the lowest address (bottom of stack)
//   *(uint64*)(t->stack) = STACK_MAGIC;
  
//   t->context.sp = (uint64)(t->stack + STACK_SIZE);
//   t->context.ra = (uint64)func;
// }

// void thread_yield(void) {
//   // CHECK: Is our magic number still there?
//   if (*(uint64*)(current_thread->stack) != STACK_MAGIC) {
//     printf("STACK OVERFLOW DETECTED in thread!\n");
//     exit(-1);
//   }
  
//   current_thread->state = RUNNABLE;
//   thread_schedule();
// }



// Modification	                   sp Initialization	                    Safety Check
// Standard (Down)	                 t->stack + STACK_SIZE                  None
// Upward-Growing	                 t->stack	                              Check top boundary
// Dynamic Size	                    malloc_ptr + dynamic_size	            Verify malloc != 0
// Overflow Guard	                  t->stack + STACK_SIZE	                if (stack[0] != MAGIC)








// 3. Scheduling and State Logic

// Priority Scheduling: Adding a priority field to struct thread and modifying the for loop in thread_schedule to pick the runnable thread with the highest priority instead of just the next one.
// Thread ID (TID) tracking: Adding a unique ID to each thread and requiring that the ID be printed every time a context switch occurs.
// "Kill" or "Exit" status: Implementing a way for a thread to transition from RUNNING to FREE automatically without the manual state setting currently used in thread_a, thread_b, and thread_c.


// 1. Priority Scheduling

// The original scheduler picks the next RUNNABLE thread in a circular loop. To implement priority, you must add a priority field to the thread structure and change the selection logic to scan for the "best" thread rather than the "next" one.
// Modify struct thread in user/uthread.c:

// struct thread {
//   char       stack[STACK_SIZE]; /*  */
//   int        state;             /*  */
//   struct thread_context context;/* [cite: 835] */
//   int        priority;          // NEW: Lower number = Higher priority
// };


// Modify thread_schedule():

// void 
// thread_schedule(void)
// {
//   struct thread *t, *next_thread = 0;
//   int highest_priority = 999; // Assume 0 is the best priority

//   // Scan ALL threads to find the RUNNABLE one with the best priority
//   for(t = all_thread; t < all_thread + MAX_THREAD; t++) {
//     if(t->state == RUNNABLE && t->priority < highest_priority) {
//       highest_priority = t->priority;
//       next_thread = t;
//     }
//   }

//   // ... (rest of the switch logic as before) ... [cite: 825, 849]
// }


// 2. Thread ID (TID) Tracking

// Adding a unique identifier (TID) helps in debugging by clearly showing which thread is active.
// Modify struct thread:


// struct thread {
//   // ... existing fields ...
//   int tid; // NEW: Unique Thread ID
// };



// Modify thread_create():
// Assign a unique ID (like the index or a global counter) when the thread is initialized.

// Modify thread_schedule() (Printing the Switch):


// if (current_thread != next_thread) {
//   // LOG: "Switching from TID X to TID Y"
//   printf("Context Switch: %d -> %d\n", current_thread->tid, next_thread->tid);
  
//   // ... perform the thread_switch() ... [cite: 849, 851]
// }


// 3. Automatic "Kill" or "Exit" Status

// In the original lab, thread_a, thread_b, and thread_c must manually set their state to FREE and call the scheduler at the end of their functions. A better way is to create a "wrapper" or "stunt" function that the thread always returns to.
// The Logic: When you create a thread, you point its ra (Return Address) to a special thread_exit() function instead of the user function itself.
// Modify user/uthread.c:


// void thread_exit(void) {
//   current_thread->state = FREE; // Automatically clean up
//   thread_schedule();           // Pick next thread
// }

// void thread_create(void (*func)()) {
//   struct thread *t;
//   // ... find FREE thread ...
  
//   // Instead of jumping directly to 'func', we can set up the stack 
//   // so that when 'func' finishes, it "returns" into thread_exit.
//   // This usually requires pushing the address of thread_exit onto 
//   // the thread's new stack before starting.
  
//   t->context.ra = (uint64)func; 
//   t->context.sp = (uint64)&t->stack[STACK_SIZE];
//   t->state = RUNNABLE;
// }


// Quick Quiz Reference for Scheduling
// Modification	          Purpose                       Code Logic
// Priority	              Non-circular choice	          if(t->priority < best)
// TID Tracking	          Debugging visibility	        printf("Switch %d -> %d")
// Auto-Exit	              Clean lifecycle	              Set ra or use a wrapper function.
// Pre-emption	            Force yields	                Call thread_yield() after X ticks.









// 4. Output and Formatting (Quiz 1 Style)

// Context Switch Logging: Just like the Monitor task, you might be asked to print which thread is being switched out and which is being switched in.
// Hexadecimal Register Dumps: If a thread crashes or exits, the quiz could ask for a dump of its saved ra and sp values in hexadecimal format.
// Timing Metrics: Using uptime to calculate and print how many ticks a thread spent in the RUNNING state before yielding.
// Look for: "Print the old and new thread addresses during a context switch. Optionally, include their TIDs if you implemented that feature."

// 1. Context Switch Logging

// This is a high-probability modification. It tests if you can correctly identify the "Old" vs. "New" thread during a swap.
// Where to modify: user/uthread.c inside thread_schedule().

// // Inside thread_schedule()
// if (current_thread != next_thread) {
//   next_thread->state = RUNNING;
//   t = current_thread; // The "Old" thread being switched out
//   current_thread = next_thread; // The "New" thread being switched in

//   // LOGGING: Explicitly show the switch
//   printf("Switching out thread at %p, switching in thread at %p\n", t, current_thread);

//   thread_switch((uint64)&t->context, (uint64)&current_thread->context);
// }

// 2. Hexadecimal Register Dumps

// This modification mirrors the memdump task. If a thread is being freed or crashes, you might be asked to print its saved ra and sp.
// Where to modify: You can add a helper function in user/uthread.c or add it to the exit logic.

// void dump_context(struct thread *t) {
//   // Use %x or %p for hexadecimal formatting
//   printf("Thread Context Dump:\n");
//   printf("  ra: 0x%p\n", t->context.ra); // Return Address 
//   printf("  sp: 0x%p\n", t->context.sp); // Stack Pointer 
// }

// // Example usage when a thread is being freed
// if (t->state == FREE) {
//   dump_context(t);
// }


// 3. Timing Metrics (Using uptime)

// Just like Task 1: sleep used ticks for timing, this modification tracks how long a thread actually held the CPU.
// Step 1: Update struct thread in user/uthread.c
// Add a field to store when the thread started its current run.

// struct thread {
//   // ... existing fields ...
//   uint64 start_time; 
// };

// Step 2: Update thread_schedule() to calculate duration

// // When starting a thread
// current_thread->start_time = uptime(); // 

// // When yielding or switching out
// uint64 end_time = uptime();
// uint64 duration = end_time - current_thread->start_time;
// printf("Thread ran for %d ticks\n", duration);



// Quick Checklist for Formatting/Timing
// Modification	                      Required Data	                  Tool to Use
// Switch Log	                        t and next_thread pointers	    printf("%p")
// Register Dump	                      t->context.ra and t->context.sp	printf("0x%x")
// Timing	                            uptime()                        end_ticks - start_ticks








// 5. Initialization and Limit Tweak

// Maximum Thread expansion: Increasing MAX_THREAD and modifying the initialization loop to handle a larger pool of potential threads.
// Pre-emption Simulation: Modifying thread_yield to only allow a yield if the thread has performed a certain number of "work units" (e.g., iterations in the loop).


// 1. Maximum Thread Expansion

// The original code is hardcoded to support 4 threads. Expanding this requires updating the constant and ensuring all loops that iterate over threads use that constant.
// Modify user/uthread.c:
// Update the constant: Increase #define MAX_THREAD to a larger number (e.g., 16 or 32).
// Update loops: Ensure thread_schedule and thread_create use MAX_THREAD to scan the expanded all_thread array.

// #define MAX_THREAD  16 // Increased from 4

// struct thread all_thread[MAX_THREAD]; // Array automatically expands

// void thread_schedule(void) {
//   // ...
//   for(int i = 0; i < MAX_THREAD; i++){ // Loops now cover all 16 slots
//     // ... search logic ...
//   }
// }

// 2. Pre-emption Simulation (Work-Unit Yielding)

// In a cooperative threading system, a thread yields voluntarily. A "Pre-emption Simulation" adds a rule: a thread cannot yield until it has completed a specific amount of work.

// Step 1: Update struct thread
// Add a counter to track "work units" (e.g., how many times a loop has run).

// struct thread {
//   char       stack[STACK_SIZE]; 
//   int        state;             
//   struct thread_context context;
//   int        work_done; // NEW: Track progress
// };


// Step 2: Modify thread_yield()
// Change the yield logic so it only triggers the scheduler if the work_done exceeds a certain threshold.

// void thread_yield(void) {
//   // Only yield if the thread has done at least 10 "units" of work
//   if (current_thread->work_done >= 10) {
//     current_thread->work_done = 0; // Reset counter for next time
//     current_thread->state = RUNNABLE;
//     thread_schedule();
//   } else {
//     // If not enough work is done, just increment and keep running
//     current_thread->work_done++; 
//     // We return immediately to the thread function instead of scheduling
//   }
// }


// Quick Quiz Reference for Initialization
// Modification	            File to Change	          Key Logic
// Expand Thread Pool	      uthread.c	                Update #define MAX_THREAD. 
// Conditional Yield	        uthread.c	                Wrap thread_schedule() inside an if check. 
// Initial Context	          uthread.c	                Ensure thread_init() sets current_thread correctly.







// Original Code

// #include "kernel/types.h"
// #include "kernel/stat.h"
// #include "user/user.h"

// /* Possible states of a thread: */
// #define FREE        0x0
// #define RUNNING     0x1
// #define RUNNABLE    0x2

// #define STACK_SIZE  8192
// #define MAX_THREAD  4

// struct thread_context {
//   uint64 ra;
//   uint64 sp;
//   uint64 s0;
//   uint64 s1;
//   uint64 s2;
//   uint64 s3;
//   uint64 s4;
//   uint64 s5;
//   uint64 s6;
//   uint64 s7;
//   uint64 s8;
//   uint64 s9;
//   uint64 s10;
//   uint64 s11;
// };

// struct thread {
//   char       stack[STACK_SIZE]; /* the thread's stack */
//   int        state;             /* FREE, RUNNING, RUNNABLE */
//   struct thread_context context;
// };
// struct thread all_thread[MAX_THREAD];
// struct thread *current_thread;
// extern void thread_switch(uint64, uint64);
              
// void 
// thread_init(void)
// {
//   // main() is thread 0, which will make the first invocation to
//   // thread_schedule(). It needs a stack so that the first thread_switch() can
//   // save thread 0's state.
//   current_thread = &all_thread[0];
//   current_thread->state = RUNNING;
// }

// void 
// thread_schedule(void)
// {
//   struct thread *t, *next_thread;

//   /* Find another runnable thread. */
//   next_thread = 0;
//   t = current_thread + 1;
//   for(int i = 0; i < MAX_THREAD; i++){
//     if(t >= all_thread + MAX_THREAD)
//       t = all_thread;
//     if(t->state == RUNNABLE) {
//       next_thread = t;
//       break;
//     }
//     t = t + 1;
//   }

//   if (next_thread == 0) {
//     printf("thread_schedule: no runnable threads\n");
//     exit(-1);
//   }

//   if (current_thread != next_thread) {         /* switch threads?  */
//     next_thread->state = RUNNING;
//     t = current_thread;
//     current_thread = next_thread;
//     /* YOUR CODE HERE
//      * Invoke thread_switch to switch from t to next_thread:
//      * thread_switch(??, ??);
//      */

//     thread_switch((uint64)&t->context,(uint64)&next_thread->context);
//   } else
//     next_thread = 0;
// }

// void 
// thread_create(void (*func)())
// {
//   struct thread *t;

//   for (t = all_thread; t < all_thread + MAX_THREAD; t++) {
//     if (t->state == FREE) break;
//   }
//   t->state = RUNNABLE;
//   // YOUR CODE HERE

//   // Set stack pointer to top of stack
//   t->context.sp = (uint64)(t->stack + STACK_SIZE);

//   // Set return address to the thread function
//   t->context.ra = (uint64) func;
// }

// void 
// thread_yield(void)
// {
//   current_thread->state = RUNNABLE;
//   thread_schedule();
// }

// volatile int a_started, b_started, c_started;
// volatile int a_n, b_n, c_n;

// void 
// thread_a(void)
// {
//   int i;
//   printf("thread_a started\n");
//   a_started = 1;
//   while(b_started == 0 || c_started == 0)
//     thread_yield();
  
//   for (i = 0; i < 100; i++) {
//     printf("thread_a %d\n", i);
//     a_n += 1;
//     thread_yield();
//   }
//   printf("thread_a: exit after %d\n", a_n);

//   current_thread->state = FREE;
//   thread_schedule();
// }

// void 
// thread_b(void)
// {
//   int i;
//   printf("thread_b started\n");
//   b_started = 1;
//   while(a_started == 0 || c_started == 0)
//     thread_yield();
  
//   for (i = 0; i < 100; i++) {
//     printf("thread_b %d\n", i);
//     b_n += 1;
//     thread_yield();
//   }
//   printf("thread_b: exit after %d\n", b_n);

//   current_thread->state = FREE;
//   thread_schedule();
// }

// void 
// thread_c(void)
// {
//   int i;
//   printf("thread_c started\n");
//   c_started = 1;
//   while(a_started == 0 || b_started == 0)
//     thread_yield();
  
//   for (i = 0; i < 100; i++) {
//     printf("thread_c %d\n", i);
//     c_n += 1;
//     thread_yield();
//   }
//   printf("thread_c: exit after %d\n", c_n);

//   current_thread->state = FREE;
//   thread_schedule();
// }

// int 
// main(int argc, char *argv[]) 
// {
//   a_started = b_started = c_started = 0;
//   a_n = b_n = c_n = 0;
//   thread_init();
//   thread_create(thread_a);
//   thread_create(thread_b);
//   thread_create(thread_c);
//   current_thread->state = FREE;
//   thread_schedule();
//   exit(0);
// }





















// HOW TO DO

// Step 1: Define the thread_context Structure

// First, you need a way to store the registers for each thread.
// File: user/uthread.c
// What to do: Define a structure that holds all the "callee-saved" registers. These include the return address (ra), stack pointer (sp), and saved registers (s0-s11).

// struct thread_context {
//   uint64 ra;
//   uint64 sp;
//   uint64 s0;
//   uint64 s1;
//   uint64 s2;
//   uint64 s3;
//   uint64 s4;
//   uint64 s5;
//   uint64 s6;
//   uint64 s7;
//   uint64 s8;
//   uint64 s9;
//   uint64 s10;
//   uint64 s11;
// };

// struct thread {
//   char stack[STACK_SIZE];    /* the thread's stack */
//   int state;                /* FREE, RUNNING, RUNNABLE */
//   struct thread_context context; /* ADD THIS LINE: to store saved state */
// };


// Step 2: Initialize a New Thread in thread_create

// When you create a thread, you must set up its initial "state" so that when it runs for the first time, it knows where to start and which stack to use.
// File: user/uthread.c
// Function: thread_create
// What to do:
// Set the ra (return address) to the function the thread should run.
// Set the sp (stack pointer) to the top (end) of its private stack.

// void 
// thread_create(void (*func)())
// {
//   struct thread *t;

//   for (t = all_thread; t < all_thread + MAX_THREAD; t++) {
//     if (t->state == FREE) break;
//   }
//   t->state = RUNNABLE;

//   // Set initial return address to the function provided
//   t->context.ra = (uint64)func;
  
//   // Set stack pointer to the TOP of the allocated stack 
//   // (Stack grows downwards, so we point to the very end of the array)
//   t->context.sp = (uint64)t->stack + STACK_SIZE;
// }



// Step 3: Trigger the Switch in thread_schedule

// When the scheduler finds a new thread to run, it needs to call the assembly function thread_switch to swap the current CPU registers with the new thread's saved ones.
// File: user/uthread.c
// Function: thread_schedule
// What to do: Pass the addresses of the current thread's context and the next thread's context to the assembly function.

// void 
// thread_schedule(void)
// {
//   // ... existing code to find next_thread ...

//   if (current_thread != next_thread) {
//     next_thread->state = RUNNING;
//     t = current_thread;
//     current_thread = next_thread;

//     /* YOUR CODE HERE: Invoke thread_switch  */
//     // a0 = address of old thread's context
//     // a1 = address of next thread's context
//     thread_switch((uint64)&t->context, (uint64)&current_thread->context);
//   }
// }

// Step 4: Implement Assembly thread_switch

// Now you must write the low-level assembly to move data between the physical CPU registers and the RAM structures you created.
// File: user/uthread_switch.S
// What to do: Use sd (store) to save the current registers and ld (load) to restore the new ones.


// .globl thread_switch
// thread_switch:
//     /* a0 contains &old_thread->context */
//     /* a1 contains &new_thread->context */

//     /* 1. Save old thread's registers into its context (sd = Store Doubleword) [cite: 855] */
//     sd ra, 0(a0)
//     sd sp, 8(a0)
//     sd s0, 16(a0)
//     sd s1, 24(a0)
//     /* ... continue for s2 through s11 with 8-byte offsets ... */
//     sd s11, 104(a0)

//     /* 2. Load new thread's registers from its context (ld = Load Doubleword) [cite: 862] */
//     ld ra, 0(a1)
//     ld sp, 8(a1)
//     ld s0, 16(a1)
//     ld s1, 24(a1)
//     /* ... continue for s2 through s11 ... */
//     ld s11, 104(a1)

//     /* 3. Return. This will jump to the NEW ra we just loaded [cite: 867, 871] */
//     ret