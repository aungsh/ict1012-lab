#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <sys/time.h>

#define NBUCKET 5
#define NKEYS 100000

struct entry {
  int key;
  int value;
  struct entry *next;
};
struct entry *table[NBUCKET];
int keys[NKEYS];
int nthread = 1;
pthread_mutex_t locks[NBUCKET];

double
now()
{
 struct timeval tv;
 gettimeofday(&tv, 0);
 return tv.tv_sec + tv.tv_usec / 1000000.0;
}


static void 
insert(int key, int value, struct entry **p, struct entry *n)
{
  struct entry *e = malloc(sizeof(struct entry));
  e->key = key;
  e->value = value;
  e->next = n;
  *p = e;
}

static 
void put(int key, int value)
{
  int i = key % NBUCKET;

  // acquire lock for this bucket
  pthread_mutex_lock(&locks[i]);
  
  // is the key already present?
  struct entry *e = 0;
  for (e = table[i]; e != 0; e = e->next) {
    if (e->key == key)
      break;
  }
  if(e){
    // update the existing key.
    e->value = value;
  } else {
    // the new is new.
    insert(key, value, &table[i], table[i]);
  }

  // release lock for this bucket
  pthread_mutex_unlock(&locks[i]);
}

static struct entry*
get(int key)
{
  int i = key % NBUCKET;
  // Note: For this lab, get() is typically called after all puts are finished,
  // so additional locking in get() is often not required for the test to pass.
  struct entry *e = 0;
  for (e = table[i]; e != 0; e = e->next) {
    if (e->key == key) break;
  }

  return e;
}

static void *
put_thread(void *xa)
{
  int n = (int) (long) xa; // thread number
  int b = NKEYS/nthread;

  for (int i = 0; i < b; i++) {
    put(keys[b*n + i], n);
  }

  return NULL;
}

static void *
get_thread(void *xa)
{
  int n = (int) (long) xa; // thread number
  int missing = 0;

  for (int i = 0; i < NKEYS; i++) {
    struct entry *e = get(keys[i]);
    if (e == 0) missing++;
  }
  printf("%d: %d keys missing\n", n, missing);
  return NULL;
}

int
main(int argc, char *argv[])
{
  pthread_t *tha;
  void *value;
  double t1, t0;

  
  if (argc < 2) {
    fprintf(stderr, "Usage: %s nthreads\n", argv[0]);
    exit(-1);
  }
  nthread = atoi(argv[1]);
  tha = malloc(sizeof(pthread_t) * nthread);
  srandom(0);
  assert(NKEYS % nthread == 0);
  for (int i = 0; i < NKEYS; i++) {
    keys[i] = random();
  }

  for (int i = 0; i < NBUCKET; i++) {
    pthread_mutex_init(&locks[i], NULL);
  }

  //
  // first the puts
  //
  t0 = now();
  for(int i = 0; i < nthread; i++) {
    assert(pthread_create(&tha[i], NULL, put_thread, (void *) (long) i) == 0);
  }
  for(int i = 0; i < nthread; i++) {
    assert(pthread_join(tha[i], &value) == 0);
  }
  t1 = now();

  printf("%d puts, %.3f seconds, %.0f puts/second\n",
         NKEYS, t1 - t0, NKEYS / (t1 - t0));

  //
  // now the gets
  //
  t0 = now();
  for(int i = 0; i < nthread; i++) {
    assert(pthread_create(&tha[i], NULL, get_thread, (void *) (long) i) == 0);
  }
  for(int i = 0; i < nthread; i++) {
    assert(pthread_join(tha[i], &value) == 0);
  }
  t1 = now();

  printf("%d gets, %.3f seconds, %.0f gets/second\n",
         NKEYS*nthread, t1 - t0, (NKEYS*nthread) / (t1 - t0));
  
  for (int i = 0; i < NBUCKET; i++) {
    pthread_mutex_destroy(&locks[i]);
  }

  return 0;       
}








// 1. Lock Granularity Tweak
// The lab asks for "Per-Bucket" locking. A quiz might ask you to move in either direction:
// Global Locking: Instead of 5 locks, use exactly one mutex for the entire hash table. This tests if you can identify that while safe, it destroys parallel speedup.
// Per-Entry Locking: Implement a lock for every individual struct entry in the linked list. This is much more complex and usually involves locking the "previous" node while updating the "next" pointer.
// Odd/Even Locking: Use only two locks—one for all even-numbered buckets and one for all odd-numbered buckets.


// 1. Global Locking (One Lock for Everything)

// In this version, every thread must wait for a single global lock before it can touch any part of the hash table. While this is 100% safe, it forces the threads to run one after another, effectively making the program sequential.
// How to modify:

//     Declare: Create a single mutex at the top of the file.
//     pthread_mutex_t global_lock;

//     Initialize: Initialize it once in main().
//     pthread_mutex_init(&global_lock, NULL);

//     Apply: In the put() function, lock at the very start and unlock at the very end.


// static void put(int key, int value) {
//   pthread_mutex_lock(&global_lock); // Every thread queues up here
  
//   int i = key % NBUCKET;
//   // ... (Search and Insert logic) ...
  
//   pthread_mutex_unlock(&global_lock);
// }



// 2. Odd/Even Locking (Two-Lock Strategy)

// This is a middle ground between Global and Per-Bucket locking. You use one lock for buckets 0, 2, 4 (Even) and another for buckets 1, 3 (Odd).

// How to modify:

//     Declare: Create an array of exactly two locks.
//     pthread_mutex_t oe_locks[2];

//     Initialize: Initialize both in main().
//     for(int i=0; i<2; i++) pthread_mutex_init(&oe_locks[i], NULL);

//     Apply: Use the bucket index to determine which lock to grab.



// static void put(int key, int value) {
//   int i = key % NBUCKET;
//   int lock_index = i % 2; // 0 for even buckets, 1 for odd buckets

//   pthread_mutex_lock(&oe_locks[lock_index]); 
  
//   // ... (Search and Insert logic) ...
  
//   pthread_mutex_unlock(&oe_locks[lock_index]);
// }


// 3. Per-Entry Locking (Extreme Granularity)

// This is the most complex version. Each struct entry gets its own mutex. This allows maximum parallelism but is very difficult to implement because you must lock the "previous" node while inserting a "new" node to prevent another thread from cutting in.

//     Structure Change: You must add pthread_mutex_t lock directly inside the struct entry.

//     Challenge: You must also have a lock for the table[i] head pointer itself to prevent race conditions during the very first insertion into an empty bucket.










// 2. Synchronized "get()" Operations

// The lab states get() doesn't require a lock because it runs after all put() operations are finished.

//     Concurrent Read/Write: The quiz might change the workload so that threads are calling put() and get() at the same time.

//     Modification: You would be required to add pthread_mutex_lock and unlock inside the get() function to prevent reading a linked list while it is being rewired by a put().


// In a real-world scenario, data is rarely updated and read in two distinct, non-overlapping phases. If the quiz modifies the workload so that put() and get() occur simultaneously, failing to lock the get() function will lead to a race condition. A thread might attempt to read a linked list at the exact moment another thread is modifying a pointer, leading to a crash or incorrect results.
// The "Concurrent" Modification

// To make the hash table safe for simultaneous reads and writes, you must apply the same locking logic used in put() to the get() function.
// Updating get() in ph-with-mutex-locks.c

// You must acquire the lock for the specific bucket before traversing the linked list and release it before the function returns.


// static struct entry*
// get(int key)
// {
//   int i = key % NBUCKET;
//   struct entry *e = 0;

//   // 1. Acquire lock for the specific bucket being read
//   pthread_mutex_lock(&locks[i]); 

//   for (e = table[i]; e != 0; e = e->next) {
//     if (e->key == key) {
//         // 2. We found it, but we MUST unlock before returning
//         pthread_mutex_unlock(&locks[i]); 
//         return e;
//     }
//   }

//   // 3. Unlock if the key was not found
//   pthread_mutex_unlock(&locks[i]); 
//   return 0;
// }

// Critical Quiz Tip: The "Early Return" Trap

// A common mistake in the quiz is forgetting to unlock before a return statement.

//     The Error: If you call pthread_mutex_lock, then return e inside the loop without unlocking, that bucket will be deadlocked forever.

//     The Fix: Always ensure there is a pthread_mutex_unlock immediately before every return or at the very end of the function.













// 3. Reader-Writer Locks (pthread_rwlock_t)

// Instead of a standard Mutex, you might be asked to use Reader-Writer locks.

//     The Change: Use pthread_rwlock_rdlock for get() and pthread_rwlock_wrlock for put().

//     The Goal: This allows multiple threads to read the same bucket simultaneously but gives exclusive access to a thread that needs to write.



// The Transformation: Mutex vs. Reader-Writer Lock

// To implement this, you must swap the pthread_mutex_t type and its corresponding functions with the pthread_rwlock_t equivalent.
// 1. Declaration and Initialization

// Instead of an array of mutexes, you declare an array of read-write locks.

//     Change in Global Scope: Replace pthread_mutex_t locks[NBUCKET]; with pthread_rwlock_t rwlocks[NBUCKET];.

//     Change in main(): Use the initialization function for rwlocks.

// for (int i = 0; i < NBUCKET; i++) {
//     pthread_rwlock_init(&rwlocks[i], NULL);
// }


// 2. Writing with put()

// The put operation modifies the linked list, so it requires an exclusive (Write) lock to prevent data loss or corruption.

// static void put(int key, int value) {
//   int i = key % NBUCKET;

//   // Use WRLOCK for exclusive access during modification
//   pthread_rwlock_wrlock(&rwlocks[i]); 
  
//   // ... (Standard search/insert logic) ... [cite: 365, 366]

//   pthread_rwlock_unlock(&rwlocks[i]); 
// }


// 3. Reading with get()

// The get operation only reads data. Using a Reader lock allows other threads to also call get() on the same bucket at the same time, increasing performance.


// static struct entry* get(int key) {
//   int i = key % NBUCKET;
//   struct entry *e = 0;

//   // Use RDLOCK to allow multiple simultaneous readers
//   pthread_rwlock_rdlock(&rwlocks[i]); 

//   for (e = table[i]; e != 0; e = e->next) {
//     if (e->key == key) {
//         pthread_rwlock_unlock(&rwlocks[i]); 
//         return e;
//     }
//   }

//   pthread_rwlock_unlock(&rwlocks[i]); 
//   return 0;
// }









// 4. Dynamic Bucket Count

// Currently, NBUCKET is a hardcoded constant (5).

//     The Change: Modify the program to accept the number of buckets as a command-line argument (e.g., ./ph-with-mutex-locks 2 10 for 2 threads and 10 buckets).

//     Requirement: You would need to use malloc to create the locks array and the table array dynamically in main(), rather than using fixed sizes.



// In this modification, you move away from the static #define NBUCKET 5 and instead allow the user to define the "width" of the hash table at runtime. This tests your ability to manage dynamic memory allocation using malloc for both the data structure (the table) and the synchronization primitives (the locks).
// The Strategy: From Constants to Variables

// To implement this, you must change NBUCKET from a compile-time constant to a global variable that is initialized in main() based on argv.
// Step 1: Update Global Declarations

// Instead of fixed-size arrays, you will declare pointers that will eventually point to heap-allocated memory.
// C

// // Remove: #define NBUCKET 5
// // Add these as global variables:
// int nbucket = 5; // Default value
// struct entry **table; // Pointer to an array of pointers
// pthread_mutex_t *locks; // Pointer to an array of locks

// Step 2: Initialize Dynamically in main()

// You must parse the new command-line argument and allocate exactly enough memory for the requested number of buckets.
// C

// int main(int argc, char *argv[])
// {
//   if (argc < 3) { // Now expecting threads and bucket count
//     fprintf(stderr, "Usage: %s nthreads nbuckets\n", argv[0]);
//     exit(-1);
//   }

//   nthread = atoi(argv[1]);
//   nbucket = atoi(argv[2]); // Get dynamic bucket count

//   // Allocate the hash table (array of entry pointers)
//   table = malloc(sizeof(struct entry*) * nbucket);
//   // Initialize table to NULL
//   for(int i = 0; i < nbucket; i++) table[i] = 0;

//   // Allocate the locks array
//   locks = malloc(sizeof(pthread_mutex_t) * nbucket);

//   // Initialize each lock
//   for (int i = 0; i < nbucket; i++) {
//     pthread_mutex_init(&locks[i], NULL); [cite: 414, 444]
//   }

//   // ... rest of the put/get logic ...
// }

// Step 3: Update Indexing Logic

// Every function that previously used the constant NBUCKET must now use the variable nbucket to ensure the modulo operation stays within the new bounds.

//     In put(key, value): int i = key % nbucket; 

//     In get(key): int i = key % nbucket; 

// Step 4: Cleanup and Memory Management

// Because you used malloc, you should ideally free the memory and destroy the locks before the program exits.
// C

// // At the end of main()
// for (int i = 0; i < nbucket; i++) {
//   pthread_mutex_destroy(&locks[i]); [cite: 442, 447]
// }
// free(table);
// free(locks);












// 5. Deadlock Scenario (Transfer Task)
// A "Challenge" modification could involve moving a key from one bucket to another.

// The Change: Implement a move(key, old_bucket, new_bucket) function.
// The Risk: To do this safely, you must acquire two locks. If Thread A tries to move from Bucket 1 to 2, and Thread B tries to move from Bucket 2 to 1, they could deadlock.
// The Fix: You would need to implement a rule to always lock the lower-indexed bucket first to avoid a circular wait.

// 1. The Implementation of move()

// To move a key, you must find it in the old_bucket, remove it from that list, and then insert it into the new_bucket.
// 2. The Risk (The Deadlock)

// If you simply lock the old_bucket and then the new_bucket, you create a hazard:

//     Thread A moves from Bucket 1 to Bucket 2 (Locks 1, then waits for 2).

//     Thread B moves from Bucket 2 to Bucket 1 (Locks 2, then waits for 1).

//     Result: Both threads are stuck forever.

// The Solution: Lock Ordering

// The standard way to prevent deadlock is to enforce a strict ordering of locks. You must always acquire the lock with the lower index first, regardless of which one is the "old" or "new" bucket.
// Modifying ph-with-mutex-locks.c
// C

// static void 
// move(int key, int old_bucket, int new_bucket)
// {
//   // 1. Identify which lock index is smaller and which is larger
//   int lock1 = (old_bucket < new_bucket) ? old_bucket : new_bucket;
//   int lock2 = (old_bucket < new_bucket) ? new_bucket : old_bucket;

//   // 2. ACQUIRE LOCKS IN ORDER (Smallest index first)
//   pthread_mutex_lock(&locks[lock1]);
//   pthread_mutex_lock(&locks[lock2]);

//   // 3. REMOVE from old_bucket
//   struct entry **pp = &table[old_bucket];
//   struct entry *e = table[old_bucket];
//   while (e != 0) {
//     if (e->key == key) {
//       *pp = e->next; // Unlink the node
//       break;
//     }
//     pp = &e->next;
//     e = e->next;
//   }

//   // 4. INSERT into new_bucket
//   if (e) {
//     e->next = table[new_bucket];
//     table[new_bucket] = e;
//   }

//   // 5. RELEASE LOCKS (Order doesn't matter for unlocking, but stay consistent)
//   pthread_mutex_unlock(&locks[lock2]);
//   pthread_mutex_unlock(&locks[lock1]);
// }









// 6. Counting and Performance Metrics
// Lock Contention Counter: Add a global counter (protected by its own lock) that increments every time a thread has to "wait" for a lock.
// Successful Update Tracker: Only print the result if the put() operation successfully updated an existing key versus inserting a new one.


// 1. Lock Contention Counter

// Lock contention occurs when a thread tries to acquire a lock that is already held by another thread, forcing it to wait. In a standard pthread_mutex_lock call, the thread simply blocks until the lock is free. To count "waits," you can use pthread_mutex_trylock, which returns immediately if the lock is busy.

// Implementation Logic:

//     Global Variable: Declare a long contention_count = 0; and a dedicated pthread_mutex_t count_lock; to protect it.

//     Initialization: Initialize count_lock in main() before threads start.

//     Modification in put():

//         Instead of calling lock directly, use a while(pthread_mutex_trylock(&locks[i]) != 0) loop.

//         Inside that loop, acquire the count_lock, increment contention_count, and release count_lock.

//         Then, perform the standard pthread_mutex_lock to actually wait for the bucket.

// 2. Successful Update Tracker

// The put() function currently performs two types of actions: updating a value for an existing key or inserting a brand-new entry. A quiz modification might ask you to distinguish between these two "outcomes".

// Implementation Logic:

//     The Logic Change: Inside put(), there is an if(e) check that determines if a key was found.

//     Update Case: If e is not null, the value is updated (e->value = value;). This is a "Successful Update".

//     Insert Case: If e is null, the insert() function is called to add a new node.

// Example Modification in put():
// C

// static void put(int key, int value) {
//   int i = key % nbucket;
//   pthread_mutex_lock(&locks[i]); // [cite: 414, 446]

//   struct entry *e = 0;
//   for (e = table[i]; e != 0; e = e->next) {
//     if (e->key == key) break;
//   }

//   if(e) {
//     e->value = value;
//     // QUIZ TASK: Print only on update
//     printf("Thread %d: Successfully updated key %d\n", value, key); [cite: 263, 382]
//   } else {
//     insert(key, value, &table[i], table[i]); [cite: 263, 406]
//   }

//   pthread_mutex_unlock(&locks[i]); // [cite: 415, 446]
// }






// 7. "Delete" Operation
// The current code only has put and get.
// The Change: Implement a thread-safe delete(key) function.

// Logic: This requires careful pointer manipulation of the linked list while holding the bucket lock to ensure the "next" pointers are updated without losing data.

// Step 1: Declare the Function

// Add the delete function to ph-with-mutex-locks.c. It needs to calculate the bucket index and acquire the correct lock.
// Step 2: Traverse and Unlink

// You must maintain a pointer to the "previous" node or use a "pointer to a pointer" to bridge the gap.
// C

// static void 
// delete(int key)
// {
//   int i = key % NBUCKET; // Identify the bucket 

//   pthread_mutex_lock(&locks[i]); // Acquire the bucket-specific lock 

//   struct entry **pp = &table[i]; // Pointer to the current pointer we are looking at
//   struct entry *e = table[i];

//   while (e != 0) {
//     if (e->key == key) {
//       // FOUND: Bridge the gap by making the previous pointer point to the next node
//       *pp = e->next; 
      
//       // Free the memory of the removed entry
//       free(e); 
      
//       pthread_mutex_unlock(&locks[i]); // Always unlock before returning 
//       return;
//     }
//     // Move to the next node
//     pp = &e->next;
//     e = e->next;
//   }

//   pthread_mutex_unlock(&locks[i]); // Unlock if key was not found 
// }





// This task is a classic "Lock Ordering" challenge. To prevent deadlock (where Thread 1 holds Lock A and waits for Lock B, while Thread 2 holds Lock B and waits for Lock A), you must ensure that every thread acquires the locks in the exact same order.

// Since you have resourceA (Lock A) and resourceB (Lock B), the golden rule here is: Always lock A, then lock B.
// The "Safe" Strategy

//     Race Conditions: Wrap every access to resourceA, resourceB, total_ops, and done with the appropriate lock.

//     Deadlock Prevention: Even if a thread only needs to update B then A (like worker_BA), it must acquire lockA first, then lockB.

// Implementation Guide for notxv6/pthread_locks.c

// Here is how you should structure the synchronization logic within the three worker functions.

// 1. The worker_AB Thread

// This thread updates A then B. Since it follows the alphabetical order, it's straightforward.
// C

// // Inside worker_AB loop:
// pthread_mutex_lock(&lockA);
// pthread_mutex_lock(&lockB);

// resourceA++;
// resourceB++;
// total_ops++;

// pthread_mutex_unlock(&lockB);
// pthread_mutex_unlock(&lockA);

// 2. The worker_BA Thread (The Deadlock Risk)

// This thread naturally wants to update B then A. Do not let it lock B first. You must force it to follow the global order to prevent a circular wait.
// C

// // Inside worker_BA loop:
// pthread_mutex_lock(&lockA); // Force A first!
// pthread_mutex_lock(&lockB);

// resourceB++;
// resourceA++;
// total_ops++;

// pthread_mutex_unlock(&lockB);
// pthread_mutex_unlock(&lockA);

// 3. The worker_monitor Thread

// The monitor reads the values and checks the done flag. Even reading requires locks to ensure a "consistent snapshot" (so you don't read A after it was updated but B before it was updated).
// C

// // Inside monitor loop:
// pthread_mutex_lock(&lockA);
// pthread_mutex_lock(&lockB);

// // Read/Print/Adjust resources
// printf("[monitor] A=%d B=%d total_ops=%d done=%d\n", resourceA, resourceB, total_ops, done);

// if (total_ops >= 80000) {
// done = 1;
// }

// pthread_mutex_unlock(&lockB);
// pthread_mutex_unlock(&lockA);

// Why this works

// By enforcing a strict hierarchy (LockA→LockB), you break the "Circular Wait" condition of deadlocks.

//     If worker_AB has lockA, worker_BA will be blocked at the very first line (lockA) and won't be able to grab lockB.

//     This prevents the situation where each thread holds one lock and waits forever for the other.

// Final Verification

//     Shared Variables: Ensure total_ops and done are also inside the lock blocks, as they are shared across all three threads.

//     Heap Allocation: Make sure your threads return their result structs using malloc so the memory persists after the thread joins.

//     Testing: Run ./optional_test in the notxv6 folder. If it passes 20 times without hanging, your lock order is correct.

// Would you like me to show you how to use valgrind --tool=helgrind to programmatically detect these race conditions in Ubuntu?





// STEP BY STEP
// Step 1: Copy the Base Code

// First, copy all the code from ph-without-locks.c into the dummy file ph-with-mutex-locks.c



// Step 2: Declare the Mutexes

// You need an array of 5 locks, one for each bucket.
//     Where: Near the top of the file, after the table declaration.
//     What: pthread_mutex_t locks[NBUCKET];.



// Step 3: Initialize the Locks

// Before any threads start, you must initialize every lock in the array.
//     Where: Inside main(), before the "first the puts" comment.
//     Code:

// for (int i = 0; i < NBUCKET; i++) {
//     pthread_mutex_init(&locks[i], NULL); // [cite: 414, 418]
// }


// Step 4: Protect the put Function

// This is the most critical step. You must acquire the lock for the specific bucket being updated and release it when finished.
//     Where: Inside the put() function.
//     Logic:

// static void put(int key, int value) {
//   int i = key % NBUCKET;

//   pthread_mutex_lock(&locks[i]); // Acquire lock for this bucket [cite: 414, 420]

//   // ... existing code: searching for key and updating/inserting ...

//   pthread_mutex_unlock(&locks[i]); // Release lock so other threads can use it [cite: 415, 422]
// }

// Note: You do not need locks in get() because, in this specific lab, all "puts" are finished before "gets" begin, meaning the data is no longer changing.


// Step 5: Clean Up (Destroy Locks)

// After all work is done, you should destroy the mutexes.
//     Where: At the very end of main(), before return 0.
//     Code:

// for (int i = 0; i < NBUCKET; i++) {
//     pthread_mutex_destroy(&locks[i]);
// }



// Performance Comparison
// Configuration	              Missing Keys	                Performance Note
// No Lock (2 Threads)	

// ~1,442 
	

// Fast, but data is corrupted.
// Global Lock (1 Lock)	

// 0 
	

// Safe, but slow because threads queue up.
// Per-Bucket (5 Locks)	

// 0 
	

// Safe and provides parallel speedup (target >1.25x).