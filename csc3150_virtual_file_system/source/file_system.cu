
#include "file_system.h"
#include <cuda.h>
#include <cuda_runtime.h>
#include <stdio.h>
#include <stdlib.h>

__device__ __managed__ u32 gtime = 0;


__device__ void fs_init(FileSystem *fs, uchar *volume, int SUPERBLOCK_SIZE,
							int FCB_SIZE, int FCB_ENTRIES, int VOLUME_SIZE,
							int STORAGE_BLOCK_SIZE, int MAX_FILENAME_SIZE, 
							int MAX_FILE_NUM, int MAX_FILE_SIZE, int FILE_BASE_ADDRESS)
{
  // init variables
  fs->volume = volume;

  // init constants
  fs->SUPERBLOCK_SIZE = SUPERBLOCK_SIZE;
  fs->FCB_SIZE = FCB_SIZE;
  fs->FCB_ENTRIES = FCB_ENTRIES;
  fs->STORAGE_SIZE = VOLUME_SIZE;
  fs->STORAGE_BLOCK_SIZE = STORAGE_BLOCK_SIZE;
  fs->MAX_FILENAME_SIZE = MAX_FILENAME_SIZE;
  fs->MAX_FILE_NUM = MAX_FILE_NUM;
  fs->MAX_FILE_SIZE = MAX_FILE_SIZE;
  fs->FILE_BASE_ADDRESS = FILE_BASE_ADDRESS;
  fs->VCB = reinterpret_cast<VCB *> (fs->volume);
  fs->FCB = reinterpret_cast<FCB *> (fs->volume + 50);
  fs->SB = reinterpret_cast<uchar *> (fs->volume + fs->FILE_BASE_ADDRESS);
}

__device__ bool cmp(char *A, char *B){
  for(int i = 0; i < 20; i++)
    if(A[i] != B[i]) return 0;
    else if(A[i] == '\0' && B[i] == '\0') return 1;
    return 0;
}
__device__ void cpy(char *A, char *B){
  for(int i = 0; i < 20; i++){
    A[i] = B[i];
    if(B[i] == '\0') return;
  }
}
__device__ u32 fs_open(FileSystem *fs, char *s, int op)
{
	/* Implement open operation here */
  u32 tot_file_num = fs->VCB->tot_file_num;
  for(int i = 0; i < tot_file_num; i++){
    if(cmp(fs->FCB[i].file_name, s))
      return i;
  }
  if(op == G_WRITE){
    //update vcb
    int fp = fs->VCB->tot_file_num++;
    //update fcb
    fs->FCB[fp].create = gtime;
    fs->FCB[fp].modified = gtime++;
    fs->FCB[fp].size = 0;
    fs->FCB[fp].st_block = fs->VCB->empty_st++;
    cpy(fs->FCB[fp].file_name, s);
    return fp;
  }
  printf("Read file DNE\n");
  return -1;
}


__device__ void fs_read(FileSystem *fs, uchar *output, u32 size, u32 fp)
{
	/* Implement read operation here */
  int st_addr = fs->FCB[fp].st_block * fs->STORAGE_BLOCK_SIZE;
  for(int i = st_addr; i < st_addr + size; i++){
    output[i - st_addr] = fs->SB[i];
  }
}

__device__ u32 fs_write(FileSystem *fs, uchar* input, u32 size, u32 fp)
{
	/* Implement write operation here */
  int block_num = (size + fs->STORAGE_BLOCK_SIZE  - 1) / fs->STORAGE_BLOCK_SIZE;
  int pre_block_num = fs->FCB[fp].size == 0 ? 1 : (fs->FCB[fp].size + fs->STORAGE_BLOCK_SIZE  - 1) / fs->STORAGE_BLOCK_SIZE;
  // overwrite, count previous block;
  int change_num = (block_num - pre_block_num);
  // printf("Write: %s %d %d %d\n", fs->FCB[fp].file_name, change_num, size, pre_block_num);

  // make space for sb
  if(fp < fs->VCB->tot_file_num - 1)
  memcpy(fs->SB + (fs->FCB[fp + 1].st_block + change_num) * fs->STORAGE_BLOCK_SIZE,
  fs->SB + fs->FCB[fp + 1].st_block * fs->STORAGE_BLOCK_SIZE,  (fs->VCB->empty_st - fs->FCB[fp + 1].st_block) * fs->STORAGE_BLOCK_SIZE);
  //update sb
  int st_addr = fs->FCB[fp].st_block * fs->STORAGE_BLOCK_SIZE;
  for(int i = st_addr; i < st_addr + size; i++){
    fs->SB[i] = input[i - st_addr];
  }
  // update vcb
  fs->VCB->empty_st += change_num;
  //update fcb
  fs->FCB[fp].modified = gtime++;
  fs->FCB[fp].size = size;
  for(int i = fp + 1; i < fs->VCB->tot_file_num; i++){
    fs->FCB[i].st_block += change_num;
  }

  return 0; 
}
__device__ void fs_gsys(FileSystem *fs, int op){
	/* Implement LS_D and LS_S operation here */
  if(op == LS_D) {
    printf("===sort by modified time===\n");

    // version 1: use priority to list
      for(u16 i = 0; i < fs->VCB->tot_file_num; i++)
      fs->FCB[i].priority = 2024;
    for(int i = 0; i < fs->VCB->tot_file_num; i++) {
      int idx = -1;
      for(int j = 0; j < fs->VCB->tot_file_num; j++) {
        if(fs->FCB[j].priority == 2024 && (idx == -1 || fs->FCB[idx].modified < fs->FCB[j].modified))
          idx = j;
      }
      fs->FCB[idx].priority = i;
    }
      
    for(int j = 0; j < fs->VCB->tot_file_num; j++)
      for(int i = 0; i < fs->VCB->tot_file_num; i++) {
        if(fs->FCB[i].priority == j)
          printf("%s\n", fs->FCB[i].file_name);
      }
  }
  else if(op == LS_S) {
    printf("===sort by file size===\n");

    // version 1
    for(u16 i = 0; i < fs->VCB->tot_file_num; i++)
      fs->FCB[i].priority = 2024;
    for(int i = 0; i < fs->VCB->tot_file_num; i++) {
      int idx = -1;
      for(int j = 0; j < fs->VCB->tot_file_num; j++) {
        if(fs->FCB[j].priority == 2024 && (idx == -1 || fs->FCB[idx].size < fs->FCB[j].size || 
        (fs->FCB[idx].size == fs->FCB[j].size && fs->FCB[idx].create > fs->FCB[j].create)))
          idx = j;
      }
      fs->FCB[idx].priority = i;
    }
      
    for(int j = 0; j < fs->VCB->tot_file_num; j++)
      for(int i = 0; i < fs->VCB->tot_file_num; i++) {
        if(fs->FCB[i].priority == j)
          printf("%s %u\n", fs->FCB[i].file_name, fs->FCB[i].size);
      }
    // dump(fs);
  } else 
    printf("Wrong operation\n");
  return ;
}

__device__ void fs_gsys(FileSystem *fs, int op, char *s)
{
	/* Implement rm operation here */
  if(op == RM) {
    for(int i = 0; i < fs->VCB->tot_file_num; i++)
      if(cmp(fs->FCB[i].file_name, s)){
          int st_block = fs->FCB[i].st_block;
          int block_num = fs->FCB[i].size == 0 ? 1 : (fs->FCB[i].size + fs->STORAGE_BLOCK_SIZE - 1) / fs->STORAGE_BLOCK_SIZE;
          fs->VCB->tot_file_num--;
          fs->VCB->empty_st -= block_num;
          for(int j = i; j < fs->VCB->tot_file_num; j++){
            fs->FCB[j] = fs->FCB[j + 1];
            fs->FCB[j].st_block -= block_num;
          }
          // compact: move leftover to front
          if(i < fs->VCB->tot_file_num)
            memcpy(fs->SB + st_block * fs->STORAGE_BLOCK_SIZE, fs->SB + (st_block + block_num) * 
          fs->STORAGE_BLOCK_SIZE, (fs->VCB->empty_st - st_block) * fs->STORAGE_BLOCK_SIZE);
          else 
            memset(fs->SB + st_block * fs->STORAGE_BLOCK_SIZE, 0, fs->FCB[i].size);
          return;
        }
    printf("Delete file DNE\n");
  
  } else {
    printf("Wrong operation\n");

  }
}
__device__ void dump(FileSystem *fs){
  for(int i = 0; i < fs->VCB->tot_file_num; i++){
    printf("%d %d %s %d %d %d %d\n", i, fs->FCB[i].st_block, fs->FCB[i].file_name, fs->FCB[i].size, fs->FCB[i].create, fs->FCB[i].modified, fs->FCB[i].priority);
  }
  printf(" empty_st: %d\n", fs->VCB->empty_st);
}
