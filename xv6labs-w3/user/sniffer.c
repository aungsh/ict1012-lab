#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h" // Needed for PGSIZE (4096)

int
main(int argc, char *argv[])
{
  // 1. ALLOCATION:
  // We need to request a large enough chunk of memory to "catch" 
  // the pages previously used by the secret program. 
  // 100 pages (400KB) is usually more than enough.
  int pages = 100;
  int size = pages * 4096; 
  
  // sbrk(size) expands the process's memory by 'size' bytes.
  // 'p' points to the START of this new, "dirty" memory.
  char *p = sbrk(size);
  
  if(p == (char*)-1) {
    printf("sbrk failed\n");
    exit(1);
  }

  // 2. THE SEARCH:
  // We are looking for the "breadcrumb" left by secret.c
  char *marker = "This may help.";

  // We loop through every single byte of the newly allocated memory.
  // We stop at (size - 64) to ensure we don't read past the end of our memory 
  // while checking the marker or the secret.
  for (int i = 0; i < size - 64; i++) {
    
    // Efficiency optimization: Only do a full 'memcmp' if the first char matches 'T'
    if (p[i] == 'T' && memcmp(&p[i], marker, 14) == 0) {
      
      // 3. THE EXFILTRATION:
      // secret.c used: strcpy(data + 16, argv[1]);
      // This means the secret starts exactly 16 bytes after the start of the marker.
      // We print the string starting at that location.
      printf("%s\n", &p[i] + 16);
      
      // We found it! Exit successfully.
      exit(0);
    }
  }
  
  // If we reach here, we didn't find the secret.
  exit(1);
}

// PREDICTIONS

// 1. Changing the "Marker" and "Offset"

// The original lab uses "This may help." as a marker and looks 16 bytes ahead for the secret.
// Quiz Change: They may change the marker string (e.g., to "SECRET_START") or change the distance to the secret (e.g., to +32 or +0).
// Modification needed: Update the marker variable and the pointer math in your printf or memcmp. 
// Look for: "The marker string has changed" or "The secret is located at a different offset from the marker."

// #include "kernel/types.h"
// #include "kernel/fcntl.h"
// #include "user/user.h"
// #include "kernel/riscv.h" // Needed for the 4096 page size constant [cite: 56]

// int
// main(int argc, char *argv[])
// {
//   int pages = 100; // You can increase this if the secret is "hidden" deeper in memory [cite: 59]
//   int size = pages * 4096;
//   char *p = sbrk(size); // Request the "dirty" memory from the kernel [cite: 59]
  
//   if(p == (char*)-1) {
//     printf("sbrk failed\n");
//     exit(1);
//   }

//   // --- MODIFICATION AREA 1: THE MARKER ---
//   // If the quiz says the marker is "SECRET_START", change this string.
//   char *marker = "SECRET_START"; 
  
//   // You must update the length for memcmp to match the new marker.
//   // "SECRET_START" is 12 characters long.
//   int marker_len = 12; 

//   // --- MODIFICATION AREA 2: THE OFFSET ---
//   // If the quiz says the secret is 32 bytes after the marker, set this to 32.
//   // If it is immediately after, set it to the length of the marker.
//   int offset = 32; 

//   for (int i = 0; i < size - 64; i++) {
//     // Check if the current byte matches the first character of our NEW marker ('S')
//     // and then check if the rest of the string matches.
//     if (p[i] == marker[0] && memcmp(&p[i], marker, marker_len) == 0) {
      
//       // --- MODIFICATION AREA 3: THE PRINT ---
//       // We take the address of the marker (&p[i]) and add the NEW offset.
//       printf("%s\n", &p[i] + offset);
      
//       exit(0); // Found it, we are done 
//     }
//   }
  
//   exit(1);
// }



// 2. Searching for Multiple Secrets

// The original code exits after finding the first secret.
// Quiz Change: The quiz might ask you to find all occurrences of the secret in the allocated memory or find two different secrets with different markers. 
// Look for: "Find all occurrences of the secret" or "There are two different secrets with different markers."

// To handle multiple secrets or different markers, the primary change involves removing the immediate exit(0) and potentially adding secondary marker logic. This modification ensures the program scans the entire range of newly allocated memory provided by sbrk(). 

// Scenario A: Finding ALL Occurrences of the Same Secret
// In this version, we remove the exit(0) so the loop continues through the entire buffer. This is useful if the secret program was run multiple times with different values.

// #include "kernel/types.h"
// #include "kernel/fcntl.h"
// #include "user/user.h"
// #include "kernel/riscv.h"

// int
// main(int argc, char *argv[])
// {
//   int pages = 100; // Increase this if you suspect many secrets exist [cite: 59]
//   int size = pages * 4096;
//   char *p = sbrk(size);
  
//   if(p == (char*)-1) {
//     printf("sbrk failed\n");
//     exit(1);
//   }

//   char *marker = "This may help."; // Standard marker 
//   int found_count = 0;

//   // Scan the entire size allocated 
//   for (int i = 0; i < size - 64; i++) {
//     if (p[i] == 'T' && memcmp(&p[i], marker, 14) == 0) {
//       // PRINT but do NOT exit yet
//       printf("Found Secret %d: %s\n", ++found_count, &p[i] + 16); [cite: 62]
      
//       // OPTIONAL: Skip the marker length to avoid re-finding the same 'T'
//       i += 14; 
//     }
//   }

//   if (found_count == 0) {
//     printf("No secrets found.\n");
//     exit(1);
//   }
  
//   exit(0);
// }


// Scenario B: Finding Two Different Secrets (Two Different Markers)
// The quiz might specify that "Secret A" starts with marker "HELP1" and "Secret B" starts with marker "HELP2".

// #include "kernel/types.h"
// #include "kernel/fcntl.h"
// #include "user/user.h"
// #include "kernel/riscv.h"

// int
// main(int argc, char *argv[])
// {
//   int size = 100 * 4096;
//   char *p = sbrk(size);
  
//   if(p == (char*)-1) exit(1);

//   char *m1 = "HELP1";
//   char *m2 = "HELP2";

//   for (int i = 0; i < size - 64; i++) {
//     // Check for Marker 1
//     if (p[i] == 'H' && memcmp(&p[i], m1, 5) == 0) {
//       printf("Secret 1: %s\n", &p[i] + 16); // Assuming same +16 offset 
//     }
//     // Check for Marker 2
//     else if (p[i] == 'H' && memcmp(&p[i], m2, 5) == 0) {
//       printf("Secret 2: %s\n", &p[i] + 16);
//     }
//   }
  
//   exit(0);
// }

// Why the loop boundary matters

// In both cases, we use size - 64.  This "safety buffer" is critical because:
//     Access Violations: If you check for a 14-character marker at the very last byte of your allocated memory (p[size-1]), the memcmp will try to read memory outside your allocated space, causing a trap/crash. 
//     Secret Length: Since the secret follows the marker, we need enough room to read the secret string after the marker is found. 

// Key Quiz Strategy
//     Check if "All" or "First": If the question says "find all," replace exit(0) with a counter or just let the loop finish. 
//     Reset Memory: If you are testing multiple secrets, remember to run make clean && make qemu to ensure you are starting with a known state, as files and memory persistence can be tricky across QEMU runs.



// 3. Filtering the Secret (The "sixfive" Style)

// Similar to how the sixfive quiz task added a digit-count constraint.
// Quiz Change: "Only print the secret if it is exactly 8 characters long" or "Only print secrets that start with a specific character."
// Modification needed: Add a condition before the printf to check the length or content of the found string. 
// Look for: "Add a condition to filter which secrets are printed based on their content or length."

// #include "kernel/types.h"
// #include "kernel/fcntl.h"
// #include "user/user.h"
// #include "kernel/riscv.h"

// int
// main(int argc, char *argv[])
// {
//   int pages = 100;
//   int size = pages * 4096;
//   char *p = sbrk(size);
  
//   if(p == (char*)-1) {
//     printf("sbrk failed\n");
//     exit(1);
//   }

//   char *marker = "This may help.";

//   for (int i = 0; i < size - 64; i++) {
//     // 1. Identify the marker
//     if (p[i] == 'T' && memcmp(&p[i], marker, 14) == 0) {
      
//       // 2. Identify the secret location
//       char *secret = &p[i] + 16; 
      
//       // --- FILTER OPTION A: Check for specific Length ---
//       // Quiz: "Only print if the secret is exactly 8 characters long"
//       /*
//       if (strlen(secret) == 8) {
//           printf("%s\n", secret);
//           exit(0);
//       }
//       */

//       // --- FILTER OPTION B: Check for specific Starting Character ---
//       // Quiz: "Only print if the secret starts with 'A'"
//       /*
//       if (secret[0] == 'A') {
//           printf("%s\n", secret);
//           exit(0);
//       }
//       */

//       // --- FILTER OPTION C: Check for numeric content (sixfive style) ---
//       // Quiz: "Only print if the secret contains only digits"
//       int only_digits = 1;
//       for(int j = 0; secret[j] != '\0'; j++) {
//           if(secret[j] < '0' || secret[j] > '9') {
//               only_digits = 0;
//               break;
//           }
//       }
//       if(only_digits) {
//           printf("%s\n", secret);
//           exit(0);
//       }
//     }
//   }
  
//   exit(1);
// }

// Quick Quiz Reference Table
// Quiz Requirement	Code Logic to Add
// "Exactly N characters"	if (strlen(secret) == N)
// "Starts with character X"	if (secret[0] == 'X')
// "Contains only digits"	Use a for loop with secret[j] >= '0' && secret[j] <= '9'
// "Is a sequence of 5s and 6s"	Use a for loop with `secret[j] == '5'



// 4. Handling Binary/Non-String Data

// The original lab treats the secret as a null-terminated string.
// Quiz Change: The secret could be a 4-byte integer or a specific hex value.
// Modification needed: Instead of %s, you would use a typecast to read an integer: printf("%d\n", *(int*)(&p[i] + offset));. 
// Look fot: "The secret is a binary value. Use a typecast to read it as an integer instead of a string."

// This modification shifts the task from string manipulation to binary data extraction. In Quiz 1, you had to handle hexadecimal conversions in memdump and implement a 32-bit swap. This "Binary Secret" variant tests that same ability to treat a memory address as a specific data type (like a uint or int) rather than just a sequence of characters.
// Modified Code: Sniffer for Binary/Integer Data

// In this version, we replace the string-based printf with a typecast. This allows the program to read the 4 bytes starting at the offset as a single integer.

// #include "kernel/types.h"
// #include "kernel/fcntl.h"
// #include "user/user.h"
// #include "kernel/riscv.h"

// int
// main(int argc, char *argv[])
// {
//   int pages = 100;
//   int size = pages * 4096;
//   char *p = sbrk(size);
  
//   if(p == (char*)-1) {
//     printf("sbrk failed\n");
//     exit(1);
//   }

//   char *marker = "This may help.";

//   for (int i = 0; i < size - 64; i++) {
//     // 1. Find the marker (same as before)
//     if (p[i] == 'T' && memcmp(&p[i], marker, 14) == 0) {
      
//       // 2. Identify the secret's memory address
//       char *secret_addr = &p[i] + 16; 

//       // 3. TYPECASTING: Treat the address as a pointer to an integer
//       // (int*) tells the compiler: "The data at this address is a 4-byte integer."
//       // *(int*) follows that pointer to get the actual value.
//       int secret_val = *(int*)secret_addr;

//       // 4. PRINTING: Use %d for decimal or %x for hex
//       // Quiz requirement: "Print the secret as an integer."
//       printf("%d\n", secret_val);

//       // Alternative: "Print the secret as a Hex value" (like Quiz 1 Challenge)
//       // printf("0x%x\n", (uint)secret_val);

//       exit(0);
//     }
//   }
  
//   exit(1);
// }

// Key Concepts for Binary Extraction

//     Pointer Casting: Since p is a char*, adding 16 moves you 16 bytes ahead. To read a multi-byte value, you must tell the compiler to look at those bytes as a different type, such as (int*) or (uint64*).
//     Endianness: If the quiz asks you to print a hex value, remember your Quiz 1 Challenge where you had to perform an Endian Swap. If the value looks "backwards" (e.g., 0x78563412 instead of 0x12345678), you may need to apply your swap logic.

//     Data Size:
//         int or uint = 4 bytes (use *(int*)).
//         short = 2 bytes (use *(short*)).
//         uint64 = 8 bytes (use *(uint64*)).

// Quick Quiz Reference Table
// If the secret is...	      Use this Typecast	    Use this Printf
// A 32-bit Integer	        *(int*)(addr)	        printf("%d\n", ...)
// A Hexadecimal Value	      *(uint*)(addr)	      printf("0x%x\n", ...)
// A 16-bit Short	          *(short*)(addr)	      printf("%d\n", ...)
// A 64-bit Pointer/Long	    *(uint64*)(addr)	    printf("%p\n", ...)



// Based on the patterns from Quiz 1, where tasks were combined or required more robust error handling, here are three additional high-probability modifications for the sniffer task.

// 1. The "Combined Challenge": Filter + Binary + Multiple
// The most difficult version would combine several modifications. For example, finding all secrets that are even integers.

// // Modification: Find all secrets that are EVEN integers
// for (int i = 0; i < size - 64; i++) {
//   if (p[i] == 'T' && memcmp(&p[i], marker, 14) == 0) {
//     // Treat the data at offset 16 as an integer
//     int secret_val = *(int*)(&p[i] + 16); 
    
//     // Filter: Check if it is even
//     if (secret_val % 2 == 0) {
//         printf("%d\n", secret_val);
//     }
//     // Note: No exit(0) here so we find ALL even secrets
//   }
// }


// 2. Dynamic Allocation (Command Line Arguments)

// In Quiz 1, memdump was modified to take an argument. A similar tweak for sniffer would be making the search string dynamic.
// Quiz Change: Instead of hardcoding "This may help.", make the program accept the marker as a command-line argument.
// Modification: Replace the hardcoded marker with argv[1].

// int main(int argc, char *argv[]) {
//   if(argc < 2) {
//     printf("Usage: sniffer <marker>\n");
//     exit(1);
//   }
//   char *marker = argv[1];
//   int m_len = strlen(marker);
//   // ... rest of the sbrk logic ...
//   for (int i = 0; i < size - 64; i++) {
//     if (p[i] == marker[0] && memcmp(&p[i], marker, m_len) == 0) {
//       printf("%s\n", &p[i] + 16); // Still using +16 offset
//       exit(0);
//     }
//   }
// }


// 3. Memory Scanning with sbrk(0) (The Range Tweak)

// The current sniffer simply asks for 100 new pages. The quiz might ask you to scan memory that was already allocated to your process but not used, or to scan a specific range.
// Logic: sbrk(0) returns the current break address (the end of your current memory). You can use this to find out where your newly allocated memory starts.


// char *current_break = sbrk(0); // Get current end of memory
// char *new_mem = sbrk(pages * 4096); // Allocate more
// // Now you know exactly where the 'dirty' memory starts

// Modification Type	Key Change in Code	Purpose
// User Input Marker	      Use argv[1] instead of "This may help.".        Flexibility.
// Even/Odd Filter	        if (secret_val % 2 == 0)	                      Logic similar to sixfive multiples.
// Pointer Math	           &p[i] + 32 or &p[i] + 0	                       Changing secret location.
// ASCII Validation	      if (v >= 32 && v <= 126)	                        Using the ASCII table provided in Quiz 1.





// Original Code

// #include "kernel/types.h"
// #include "kernel/fcntl.h"
// #include "user/user.h"
// #include "kernel/riscv.h"

// int
// main(int argc, char *argv[])
// {
//   int pages = 100;
//   int size = pages * 4096;
//   char *p = sbrk(size);
  
//   if(p == (char*)-1) {
//     printf("sbrk failed\n");
//     exit(1);
//   }

//   char *marker = "This may help.";

//   for (int i = 0; i < size - 64; i++) {
//     if (p[i] == 'T' && memcmp(&p[i], marker, 14) == 0) {
//       printf("%s\n", &p[i] + 16);
//       exit(0);
//     }
//   }
  
//   exit(1);
// }




























// The **sniffer** task is a memory exploitation exercise. It relies on a kernel vulnerability where newly allocated memory pages are **not cleared** (zeroed out), allowing a new process to see data left behind by a previous process.

// ### Step-by-Step Implementation

// #### 1. Modify the `Makefile`

// To run these programs in xv6, you must first register them so the compiler includes them in the filesystem image.

// * **File to modify:** `Makefile`
// * **What to change:** Find the `UPROGS` section and add `$U/_secret` and `$U/_sniffer` to the list.
// ```makefile
// UPROGS=\
//     $U/_handshake\
//     $U/_secret\
//     $U/_sniffer\
//     ...

// ```



// #### 2. Implement `user/sniffer.c`

// The `secret` program writes a known "marker" string (`"This may help."`) followed by the actual secret at a specific memory offset. Your job is to find that marker in memory.

// * **File to modify:** `user/sniffer.c`
// * 
// **The Logic:** 1.  **Allocate Memory:** Use `sbrk()` to request a large chunk of memory (e.g., 100 pages).
// 2.  **Scan Memory:** Iterate through the newly allocated memory looking for the first character of the marker ('T').
// 3.  **Compare:** If 'T' is found, use `memcmp` to verify if the rest of the string matches the marker.
// 4.  **Extract:** Once found, print the string located **16 bytes** after the start of the marker (where the secret was placed).



// **Full Code for `user/sniffer.c`:**

// ```c
// #include "kernel/types.h"
// #include "kernel/fcntl.h"
// #include "user/user.h"
// #include "kernel/riscv.h" // Needed for 4096 (page size) constants

// int
// main(int argc, char *argv[])
// {
//   // 1. Request a large amount of memory from the kernel.
//   // Because the kernel bug doesn't clear pages, this memory 
//   // will contain "garbage" left by previous programs (like secret).
//   int pages = 100;
//   int size = pages * 4096;
//   char *p = sbrk(size);
  
//   if(p == (char*)-1) {
//     printf("sbrk failed\n");
//     exit(1);
//   }

//   // 2. Define what we are looking for.
//   char *marker = "This may help.";

//   // 3. Scan the "dirty" memory we just received.
//   // We stop 64 bytes early so we don't read past the end of our buffer.
//   for (int i = 0; i < size - 64; i++) {
//     // Check the first character 'T' first to save time.
//     if (p[i] == 'T' && memcmp(&p[i], marker, 14) == 0) {
      
//       // 4. Extract the secret.
//       // secret.c placed the actual secret 16 bytes after the marker.
//       printf("%s\n", &p[i] + 16);
      
//       // Found it! Exit successfully.
//       exit(0);
//     }
//   }
  
//   // If we reach the end of the loop, the secret wasn't found.
//   exit(1);
// }

// ```

// #### 3. Execution and Testing

// To test if your sniffer is working correctly, you must run the programs in a specific order within the xv6 shell:

// 1. 
// **Clean and Start:** `make clean && make qemu`.


// 2. 
// **Run Secret:** `$ secret mypassword` (This places "mypassword" in memory and then exits).


// 3. 
// **Run Sniffer:** `$ sniffer`.



// If successful, the output will be exactly `mypassword`.

// ---

// ### Why this works (The Exploit)

// In a normal operating system, `uvmalloc()` (used by `sbrk`) calls `memset(mem, 0, sz)` to wipe pages before they reach a new process. Because this lab omits that line, the physical RAM still contains the bits and bytes from the `secret` process even after it has finished.

// **Would you like me to move on to Task 3: Monitor, which involves modifying the kernel code itself?**