#include "kernel/types.h"
#include "user/user.h"
#include "kernel/spinlock.h"
#include "kernel/sleeplock.h"
#include "kernel/fs.h"
#include "kernel/file.h"
#include "kernel/fcntl.h"

#include "kernel/crypto.h"

int main(void) {
  if(open("console", O_RDWR) < 0){
    mknod("console", CONSOLE, 0);
    open("console", O_RDWR);
  }
  dup(0);  // stdout
  dup(0);  // stderr

  printf("crypto_srv: starting\n");

  // TODO: implement the cryptographic server here

  if (getpid() != 2) {
    exit(1);
  }
  while(1){
    void* addr;
    uint64 size;

    // Take a shared memory request from the queue
    if (take_shared_memory_request(&addr, &size) == 0) {

      struct crypto_op* op = (struct crypto_op*)addr;

      //check state and type fields
      if(op->state != CRYPTO_OP_STATE_INIT ||(op->type != CRYPTO_OP_TYPE_ENCRYPT && op->type != CRYPTO_OP_TYPE_DECRYPT)) {
        asm volatile ("fence rw,rw" : : : "memory");
        op->state = CRYPTO_OP_STATE_ERROR;
        remove_shared_memory_request(addr, size);
      }

      else{

        char* key = (char*)(op + 1); //go to the key_array
        char* data = key + op->key_size; // go to the data_array

        //XOR operation
        for (uint64 i = 0; i < op->data_size; ++i) {
            data[i] ^= key[i % op->key_size];
        }
        //set the state to DONE
        asm volatile ("fence rw,rw" : : : "memory");
        op->state = CRYPTO_OP_STATE_DONE;

        remove_shared_memory_request(addr,size);
      }
    }

  }
  exit(0);
}
