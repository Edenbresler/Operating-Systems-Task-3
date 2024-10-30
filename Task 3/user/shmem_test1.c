#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/riscv.h"

int main()
{

    char *shared_mem = malloc(PGSIZE);
    strcpy(shared_mem, "Hello child");

    int parent_pid = getpid();
    int pid = fork();

    if (shared_mem == 0) {
        printf("malloc failed\n");
        exit(1);
    }

    if (pid < 0) {
        printf("fork failed\n");
        free(shared_mem);
        exit(1);
    }

    if (pid == 0)
    { //child process
        //map from the parent to the child
        uint64 child_va = map_shared_pages( parent_pid, getpid(), (uint64)shared_mem, PGSIZE);
        
        if (child_va < 0){
            printf("map_shared_pages failed\n");
            free(shared_mem);
            exit(1);
        }
        printf("%s\n", (char*)child_va);
        exit(0);
    }

    else 
    { //parent process
        wait(0);
        free(shared_mem);
        exit(0);
    }
    
}