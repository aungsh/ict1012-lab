// Include kernel-level type definitions (e.g. uint32, uint64)
#include "kernel/types.h"

// Include user-space system calls and utilities (printf, read, exit, etc.)
#include "user/user.h"

// Include file control definitions (e.g. flags for open)
#include "kernel/fcntl.h"

// Function prototype: prints memory contents according to a format string
void memdump(char *fmt, char *data);

int
main(int argc, char *argv[])
{
  // If no command-line arguments are provided
  if(argc == 1){

    // ---------------- Example 1 ----------------
    // Demonstrates dumping raw integers from memory
    printf("Example 1:\n");

    // An array of two integers stored contiguously in memory
    int a[2] = { 61810, 2025 };

    // Cast the int array to char* so memdump can read raw bytes
    // Format "ii" means: read two integers sequentially
    memdump("ii", (char*) a);

    // ---------------- Example 2 ----------------
    // Demonstrates dumping a C string directly from memory
    printf("Example 2:\n");

    // Format "S" means: treat data as a null-terminated string
    memdump("S", "a string");

    // ---------------- Example 3 ----------------
    // Demonstrates dumping a pointer to a string
    printf("Example 3:\n");

    // s is a pointer that points to a string literal
    char *s = "another";

    // Pass the ADDRESS of the pointer (char ** in memory)
    // Format "s" means: read a char* and print the string it points to
    memdump("s", (char *) &s);

    // ---------------- Example 4 ----------------
    // Demonstrates dumping a struct field-by-field
    struct sss {
      char *ptr;      // pointer (8 bytes on 64-bit)
      int num1;       // integer (4 bytes)
      short num2;     // short integer (2 bytes)
      char byte;      // single character (1 byte)
      char bytes[8];  // array of 8 characters
    } example;

    // Initialize struct fields
    example.ptr = "hello";
    example.num1 = 1819438967;
    example.num2 = 100;
    example.byte = 'z';

    // Copy a string into the character array
    strcpy(example.bytes, "xyzzy");

    printf("Example 4:\n");

    // Format string explains how to interpret the struct memory:
    // p → pointer (char*)
    // i → int
    // h → short
    // c → char
    // S → string starting at current memory location
    memdump("pihcS", (char*) &example);

    // ---------------- Example 5 ----------------
    // Demonstrates reading struct memory byte-by-byte
    printf("Example 5:\n");

    // s → pointer
    // c c c c c → individual bytes after the pointer
    memdump("sccccc", (char*) &example);

  }
  // If a format string is provided via command-line argument
  else if(argc == 2){

    // Buffer to store up to 512 bytes read from standard input
    char data[512];
    int n = 0;

    // Clear buffer to avoid garbage values
    memset(data, '\0', sizeof(data));

    // Read raw bytes from stdin until buffer is full or EOF
    while(n < sizeof(data)){
      int nn = read(0, data + n, sizeof(data) - n);
      if(nn <= 0)
        break;
      n += nn;
    }

    // Dump memory using the provided format string
    memdump(argv[1], data);
  }
  // Incorrect usage
  else {
    printf("Usage: memdump [format]\n");
    exit(1);
  }

  exit(0);
}


void
memdump(char *fmt, char *data)
{
  // ptr tracks the current position in the memory buffer
  char *ptr = data;

  // Iterate over each character in the format string
  for(int i = 0; fmt[i] != '\0'; i++){

    switch(fmt[i]){

      // -------- Integer --------
      case 'i': {
        int val;

        // Copy sizeof(int) bytes from memory into val
        memcpy(&val, ptr, sizeof(int));

        // Print integer value
        printf("%d\n", val);

        // Advance pointer
        ptr += sizeof(int);
        break;
      }

      // -------- Pointer (64-bit) --------
      case 'p': {
        uint64 val;

        // Read 8 bytes (64-bit pointer)
        memcpy(&val, ptr, sizeof(uint64));

        // Split pointer into high and low 32-bit halves
        uint32 low = (uint32)(val & 0xffffffff);
        uint32 high = (uint32)(val >> 32);

        // Print pointer in hexadecimal
        printf("%x%x\n", high, low);

        // Advance pointer
        ptr += sizeof(uint64);
        break;
      }

      // -------- Short --------
      case 'h': {
        short val;

        memcpy(&val, ptr, sizeof(short));
        printf("%d\n", val);
        ptr += sizeof(short);
        break;
      }

      // -------- Character --------
      case 'c': {
        char val = *ptr;

        printf("%c\n", val);

        // Move one byte forward
        ptr += 1;
        break;
      }

      // -------- String pointer --------
      case 's': {
        char *str;

        // Read a pointer to a string
        memcpy(&str, ptr, sizeof(char*));

        // Print the string it points to
        printf("%s\n", str);

        ptr += sizeof(char*);
        break;
      }

      // -------- Raw C string --------
      case 'S': {
        // Print string starting at current memory location
        printf("%s\n", ptr);

        // Stop processing further format characters
        return;
      }
    }
  }
}

// #include "kernel/types.h"
// #include "user/user.h"
// #include "kernel/fcntl.h"

// void memdump(char *fmt, char *data);

// int
// main(int argc, char *argv[])
// {
//   if(argc == 1){
//     printf("Example 1:\n");
//     int a[2] = { 61810, 2025 };
//     memdump("ii", (char*) a);
    
//     printf("Example 2:\n");
//     memdump("S", "a string");
    
//     printf("Example 3:\n");
//     char *s = "another";
//     memdump("s", (char *) &s);

//     struct sss {
//       char *ptr;
//       int num1;
//       short num2;
//       char byte;
//       char bytes[8];
//     } example;
    
//     example.ptr = "hello";
//     example.num1 = 1819438967;
//     example.num2 = 100;
//     example.byte = 'z';
//     strcpy(example.bytes, "xyzzy");
    
//     printf("Example 4:\n");
//     memdump("pihcS", (char*) &example);
    
//     printf("Example 5:\n");
//     memdump("sccccc", (char*) &example);
//   } else if(argc == 2){
//     // format in argv[1], up to 512 bytes of data from standard input.
//     char data[512];
//     int n = 0;
//     memset(data, '\0', sizeof(data));
//     while(n < sizeof(data)){
//       int nn = read(0, data + n, sizeof(data) - n);
//       if(nn <= 0)
//         break;
//       n += nn;
//     }
//     memdump(argv[1], data);
//   } else {
//     printf("Usage: memdump [format]\n");
//     exit(1);
//   }
//   exit(0);
// }

// void
// memdump(char *fmt, char *data)
// {
//   char *ptr = data;

//   for(int i = 0; fmt[i] != '\0'; i++){
//     switch(fmt[i]){
//       case 'i': {
//         int val;
//         memcpy(&val, ptr, sizeof(int));
//         printf("%d\n", val);
//         ptr += sizeof(int);
//         break;
//       }
//       case 'p': {
//         uint64 val;
//         memcpy(&val, ptr, sizeof(uint64));

//         uint32 low = (uint32)(val & 0xffffffff);
//         uint32 high = (uint32)(val >> 32);

//         printf("%x%x\n", high, low);

//         ptr += sizeof(uint64);
//         break;
//       }
//       case 'h': {
//         short val;
//         memcpy(&val, ptr, sizeof(short));
//         printf("%d\n", val);
//         ptr += sizeof(short);
//         break;
//       }
//       case 'c': {
//         char val = *ptr;
//         printf("%c\n", val);
//         ptr += 1;
//         break;
//       }
//       case 's': {
//         char *str;
//         memcpy(&str, ptr, sizeof(char*));
//         printf("%s\n", str);
//         ptr += sizeof(char*);
//         break;
//       }
//       case 'S': {
//         printf("%s\n", ptr);
//         return;
//       }
//     }
//   }
// }
