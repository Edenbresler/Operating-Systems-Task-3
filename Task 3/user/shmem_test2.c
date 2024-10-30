#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define PGSIZE 4096

int main(void) {
    char *shared_mem = malloc(PGSIZE);
    int parent_pid = getpid();
    int pid;

    if (shared_mem == 0) {
        printf("malloc failed\n");
        exit(1);
    }

    if ((pid = fork()) < 0) {
        printf("fork failed\n");
        free(shared_mem);
        exit(1);
    }

    //child process
    if (pid == 0) { 
        printf("Size of the child process before mapping:%d\n", sbrk(0));

        //map from the parent to the child
        uint64 child_va = map_shared_pages(parent_pid, getpid(), (uint64)shared_mem, PGSIZE);
        
        if (child_va < 0){
            printf("map_shared_pages failed\n");
            free(shared_mem);
            exit(1);
        }
        
        printf("Size of the child process after mapping:%d\n", sbrk(0));

        //Write from child to parent
        strcpy((char *)child_va, "Hello daddy");

        // Unmap the shared memory
        if (unmap_shared_pages(getpid(), child_va, PGSIZE) < 0) {
            printf("unmap_shared_pages failed\n");
            exit(1);
        }

        printf("Size of the child process unmapping:%d\n", sbrk(0));

        // Allocate new memory
        char *mem_ch = (char *)malloc(PGSIZE);
        if (mem_ch == 0) {
            printf("malloc after unmapping failed\n");
            exit(1);
        }

        printf("Size of the child process malloc:%d\n", sbrk(0));

        exit(0);

    } else { // Parent process
        // Wait for the child process to finish
        wait(0);

        // Print the string from shared memory
        printf("%s\n", (char *)shared_mem);

        // Free the allocated memory
        free(shared_mem);
        exit(0);
    }
}
