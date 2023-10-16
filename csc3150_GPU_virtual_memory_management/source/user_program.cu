﻿#include "virtual_memory.h"
#include <cuda.h>
#include <cuda_runtime.h>
#include <stdio.h>
__device__ void user_program(VirtualMemory *vm, uchar *input, uchar *results,
                             int input_size) {
  for (int i = 0; i < input_size; i++) 
        vm_write(vm, i, input[i]);
	printf("write page fault: %d\n",*(vm->pagefault_num_ptr));
  for (int i = input_size - 1; i >= input_size - 32769; i--) {
      int value = vm_read(vm, i);
  }
	printf("read page fault: %d\n", *(vm->pagefault_num_ptr));

	vm_snapshot(vm, results, 0, input_size);
	printf("snapshot page fault: %d\n", *(vm->pagefault_num_ptr));

  // for(int i = 0; i < input_size; i++)
  //       vm_write(vm, 32*1024+i, input[i]);
	// printf("write page fault: %d\n",*(vm->pagefault_num_ptr));
    
  //   for(int i = 0; i < 32*1023; i++)
  //       vm_write(vm, i, input[i+32*1024]);
	// printf("write page fault: %d\n",*(vm->pagefault_num_ptr));
    
  //   vm_snapshot(vm,results,32*1024,input_size);
	// printf("snapshot page fault: %d\n", *(vm->pagefault_num_ptr));
}
