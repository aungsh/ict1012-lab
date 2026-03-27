#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

int main() {
    const char *filepath = "example.txt";
    // O_TRUNC: to start with a fresh file
    int fd = open(filepath, O_RDWR | O_CREAT | O_TRUNC, 0666);
    
    // 1. Write initial content
    const char *text = "Hello, Unix.....!\n";
    write(fd, text, strlen(text));

    // 2. Map the file
    char *map = mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    // 3. PRINT FIRST: Show the data as it exists on disk
    printf("Original content in memory: %s", map);

    // 4. MODIFY: Write the new string
    strcpy(map, "Modified by mmap!\n");

    // 5. SYNC & CLOSE
    munmap(map, 4096);
    close(fd);
    
    printf("Changes saved to disk via munmap.\n");
    return 0;
}