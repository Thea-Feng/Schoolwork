#include "virtual_memory.h"
#include <cuda.h>
#include <cuda_runtime.h>
#include <stdio.h>
#include <stdlib.h>
__device__ void init_invert_page_table(VirtualMemory *vm) {

  for (int i = 0; i < vm->PAGE_ENTRIES; i++) {
    vm->invert_page_table[i] = 0x80000000; // invalid := MSB is 1
    vm->invert_page_table[i + vm->PAGE_ENTRIES] = i;
  }
}


__device__ void vm_init(VirtualMemory *vm, uchar *buffer, uchar *storage,
                        u32 *invert_page_table, int *pagefault_num_ptr,
                        int PAGESIZE, int INVERT_PAGE_TABLE_SIZE,
                        int PHYSICAL_MEM_SIZE, int STORAGE_SIZE,
                        int PAGE_ENTRIES, u32 *swap_table, URL_node *pool, int thread_id) {
  // init variables
  vm->buffer = buffer;
  vm->storage = storage;
  vm->invert_page_table = invert_page_table;
  vm->pagefault_num_ptr = pagefault_num_ptr;
  vm->swap_table = swap_table;
  vm->pool = pool;
  vm->thread_id = thread_id;

  // init constants
  vm->PAGESIZE = PAGESIZE;
  vm->INVERT_PAGE_TABLE_SIZE = INVERT_PAGE_TABLE_SIZE;
  vm->PHYSICAL_MEM_SIZE = PHYSICAL_MEM_SIZE;
  vm->STORAGE_SIZE = STORAGE_SIZE;
  vm->PAGE_ENTRIES = PAGE_ENTRIES;

  // before first vm_write or vm_read
  init_invert_page_table(vm);

  struct URL_node *head, *now;
  head = (struct URL_node *) malloc(100);
  vm->tail = vm->pool;
  now = vm->tail;
  head->nxt = now;

  for(int i = 0; i < PAGE_ENTRIES; i++){
    now->idx = i;
    if(i != PAGE_ENTRIES - 1){
      now->nxt = ++(vm->tail);
      now = now->nxt;
    } else now->nxt = NULL;
  }
  vm->head = head;

  vm->disk_nxt_available = 0;
  for(int i = 0; i < 5120; i++) vm->swap_table[i] = 0x80000000;

  // struct URL_node 
}
__device__ u32 swap_in_page(VirtualMemory *vm, int pn){
  u32 location = vm->swap_table[pn];
  if(location == 0x80000000) {
    return vm->disk_nxt_available++;
  }
  vm->swap_table[pn] = 0x80000000;
  
  return location;
}
__device__ void swap_out_page(VirtualMemory *vm, int pn, u32 addr){
  if(vm->swap_table[pn] != 0x80000000) {
    printf("ERROR: the element already in disk\n");
    return;
  }
  vm->swap_table[pn] = addr;

  return;
}
__device__ u32 lookup_pt(VirtualMemory *vm, int pn){
  for(int i = 0; i < vm->PAGE_ENTRIES; i++)
    if(vm->invert_page_table[i + vm->PAGE_ENTRIES] == pn && vm->invert_page_table[i] != 0x80000000)
      return vm->invert_page_table[i];
  return 0x80000000;
}

__device__ int LRU_get(VirtualMemory *vm){
  int ret = vm->head->nxt->idx;
  URL_node *tmp = vm->head->nxt;
  vm->head->nxt = vm->head->nxt->nxt;
  vm->tail->nxt = tmp;
  vm->tail = tmp;
  return ret;
}
// __device__ void LRU_update(int idx){

// }
__device__ uchar vm_read(VirtualMemory *vm, u32 addr) {
  /* Complate vm_read function to read single element from data buffer */
  int pn = addr >> 5, offset = addr & 31;
  if(pn % 4 != vm->thread_id) return -1;

  int ret = lookup_pt(vm, pn);
  if(ret == 0x80000000){ //not found
     (*(vm->pagefault_num_ptr))++;

    int idx = LRU_get(vm); // idx of PT
    u32 replace_page = vm->invert_page_table[idx]; // frame addr

    u32 pos_in = swap_in_page(vm, pn);

    for(int i = 0; i < 32; i++) {
        uchar tmp = vm->buffer[(replace_page << 5) + i];
        vm->buffer[(replace_page << 5) + i] = vm->storage[(pos_in << 5) + i];
        vm->storage[(pos_in << 5) + i] = tmp;
      }
    
    swap_out_page(vm, vm->invert_page_table[idx + vm->PAGE_ENTRIES], pos_in);
    vm->invert_page_table[idx + vm->PAGE_ENTRIES] = pn; 
  }
  int physical_addr = (lookup_pt(vm, pn) << 5) + offset;
  uchar ans = vm->buffer[physical_addr];
  // printf("Read: page number %d page fault %d\n", (int) addr, (*(vm->pagefault_num_ptr)));
  return ans; //TODO
}

__device__ void vm_write(VirtualMemory *vm, u32 addr, uchar value) {
  /* Complete vm_write function to write value into data buffer */
  
  int pn = addr >> 5, offset = addr & 31;
  int ret = lookup_pt(vm, pn);
  if(pn % 4 != vm->thread_id && ret != 0x80000000) return;

  if(ret == 0x80000000){ //not found
    (*(vm->pagefault_num_ptr))++;
    int idx = LRU_get(vm); // idx of PT
    u32 replace_page = vm->invert_page_table[idx]; // frame addr
    if(replace_page == 0x80000000) {
      vm->invert_page_table[idx + vm->PAGE_ENTRIES] = pn; 
      vm->invert_page_table[idx] = idx;
    } else {
      u32 pos_in = swap_in_page(vm, pn);  
      for(int i = 0; i < 32; i++) {
        uchar tmp = vm->buffer[(replace_page << 5) + i];
        vm->buffer[(replace_page << 5) + i] = vm->storage[(pos_in << 5) + i];
        vm->storage[(pos_in << 5) + i] = tmp;
      }
      
      swap_out_page(vm, vm->invert_page_table[idx + vm->PAGE_ENTRIES], pos_in);
      // if(pn % 1024 == 0)
      // printf("Write: pn %d fault %d, replace idx %d and pn %d; pos_in %d, vm->buffer %x storage %x val %x\n", 
      // (int) pn, (*(vm->pagefault_num_ptr)), idx, vm->invert_page_table[idx + vm->PAGE_ENTRIES], pos_in, vm->buffer[replace_page << 5], vm->storage[pos_in << 5], value);

      vm->invert_page_table[idx + vm->PAGE_ENTRIES] = pn; 

    }
  }
  int physical_addr = (lookup_pt(vm, pn) << 5) + offset;
  vm->buffer[physical_addr] = value;
  return ; //TODO
}

__device__ void vm_snapshot(VirtualMemory *vm, uchar *results, int offset,
                            int input_size) {
  /* Complete snapshot function togther with vm_read to load elements from data
   * to result buffer */
   for(int i = 0; i < input_size; i++)
    if(((offset + i) >> 5) % 4 == vm->thread_id )
      results[i] = vm_read(vm, offset + i);
}

