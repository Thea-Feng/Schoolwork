#include "file_system.h"
#include <cuda.h>
#include <cuda_runtime.h>
#include <stdio.h>
#include <stdlib.h>

__device__ __managed__ u32 gtime = 0;
// __device__ __managed__ FCB root[1];
// __device__ __managed__ FCB *gparent;
__device__ u16 gparent;
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
  gparent = 2025;
  fs->current_dir = 2024;
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
__device__ int get_lenght(char *A){
  int ret = 0;
  for(int i = 0; i < 20 && A[i] != '\0'; i++, ret++);
  return ret+1;
}
__device__ u32 fs_open(FileSystem *fs, char *s, int op)
{
	/* Implement open operation here */
  // int create_idx = gtime;
  for(int i = 0; i < fs->VCB->tot_file_num; i++){
    if(cmp(fs->FCB[i].file_name, s) && (fs->FCB[i].parent == fs->current_dir))
      return i;
  }
  if(op == G_WRITE){
    //update vcb
    int fp = fs->VCB->tot_file_num++;
    fs->FCB[fp].size = 0;
    fs->FCB[fp].size2 = 0;
    fs->FCB[fp].dir = false;   
    fs->FCB[fp].create = gtime;
    if (fs->current_dir != 2024) 
      fs->FCB[fs->current_dir].modified = gtime;
    fs->FCB[fp].modified = gtime++;
    fs->FCB[fp].st_block = fs->VCB->empty_st++;

    cpy(fs->FCB[fp].file_name, s);
    fs->FCB[fp].parent = fs->current_dir;
    if(fs->current_dir != 2024)
      fs->FCB[fs->current_dir].size += get_lenght(s);
    // fs->FCB_now->parent->size += get_lenght(s);

    return fp;
  }
  printf("Read file DNE\n");
  return 2024;
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
  int pre_size =  (fs->FCB[fp].size + fs->FCB[fp].size2 / 60000);
  int block_num = (size + fs->STORAGE_BLOCK_SIZE  - 1) / fs->STORAGE_BLOCK_SIZE;
  int pre_block_num = pre_size == 0 ? 1 : (pre_size + fs->STORAGE_BLOCK_SIZE  - 1) / fs->STORAGE_BLOCK_SIZE;
  // overwrite, count previous block;
  int change_num = (block_num - pre_block_num);
  // printf("Write: %s %d %d %d\n", fs->FCB[fp].file_name, change_num, size, pre_block_num);

  // move sb
  if(fp < fs->VCB->tot_file_num - 1)
  memcpy(fs->SB + (fs->FCB[fp + 1].st_block + change_num) * fs->STORAGE_BLOCK_SIZE,
  fs->SB + fs->FCB[fp + 1].st_block * fs->STORAGE_BLOCK_SIZE,  (fs->VCB->empty_st - fs->FCB[fp + 1].st_block) * fs->STORAGE_BLOCK_SIZE);

  // update vcb
  fs->VCB->empty_st += change_num;
  //update fcb
  fs->FCB[fp].modified = gtime++;
  fs->FCB[fp].size = size % 60000;
  fs->FCB[fp].size2 = size / 60000;

  for(int i = fp + 1; i < fs->VCB->tot_file_num; i++){
    fs->FCB[i].st_block += change_num;
  }
  //update sb
  int st_addr = fs->FCB[fp].st_block * fs->STORAGE_BLOCK_SIZE;
  for(int i = st_addr; i < st_addr + size; i++){
    fs->SB[i] = input[i - st_addr];
  }
  return fp; // return ?
}
__device__ void fs_gsys(FileSystem *fs, int op){
	/* Implement LS_D and LS_S operation here */
 if(op == LS_D) {
    printf("===sort by modified time===\n");
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
        if(fs->FCB[i].priority == j && fs->FCB[i].parent == fs->current_dir){
          if(fs->FCB[i].dir) 
            printf("%s d\n", fs->FCB[i].file_name);
          else printf("%s\n", fs->FCB[i].file_name);
        }
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
        int idx_size = fs->FCB[idx].size + fs->FCB[idx].size2 * 60000;
        int j_size = fs->FCB[j].size + fs->FCB[j].size2 * 60000;
        if(fs->FCB[j].priority == 2024 && (idx == -1 || idx_size < j_size || 
        (idx_size == j_size && fs->FCB[idx].create > fs->FCB[j].create)))
          idx = j;
      }
      fs->FCB[idx].priority = i;
    }
      
    for(int j = 0; j < fs->VCB->tot_file_num; j++)
      for(int i = 0; i < fs->VCB->tot_file_num; i++) {
        if(fs->FCB[i].priority == j && fs->FCB[i].parent == fs->current_dir){
          if(fs->FCB[i].dir) printf("%s %u d\n", fs->FCB[i].file_name, fs->FCB[i].size);
          else printf("%s %u\n", fs->FCB[i].file_name, fs->FCB[i].size);
        }
      }
    // dump(fs);
  } 
  else if(op == CD_P) {
    fs->current_dir = fs->FCB[fs->current_dir].parent;
  } else if(op == PWD) {
    char name_string[60];
    name_string[0] = name_string[20] = name_string[40] = '\0';
    int dep = 40;
    int tmp = fs->current_dir;
    while(tmp != 2024) {
      cpy(name_string + dep, fs->FCB[tmp].file_name);
      dep -= 20;
      tmp = fs->FCB[tmp].parent;
    }
    for(int i = 0; i < 60; i += 20){
      if(name_string[i] != '\0') {
        char sub_name[20];
        cpy(sub_name, name_string + i);
        printf("/%s", sub_name);
      }
    }  
    if(name_string[0] == '\0' && name_string[20] == '\0' && name_string[40] == '\0') printf("/");                                                                                                                                                                                                                                                                                                   
    printf("\n");
  }
  else 
    printf("Wrong operation\n");
  return ;
}

__device__ void fs_gsys(FileSystem *fs, int op, char *s)
{
	/* Implement rm operation here */
  if(op == RM) {
    bool in_dir = false;
    if(gparent == 2025) {gparent = fs->current_dir; in_dir = true;}
    for(int i = 0; i < fs->VCB->tot_file_num; i++)
      if(cmp(fs->FCB[i].file_name, s) && (fs->FCB[i].parent == gparent)){
          int st_block = fs->FCB[i].st_block;
          int size = (fs->FCB[i].size + fs->FCB[i].size2 / 60000);
          int block_num = fs->FCB[i].size == 0 ? 1 : (fs->FCB[i].size + fs->STORAGE_BLOCK_SIZE - 1) / fs->STORAGE_BLOCK_SIZE;
          // dump(fs);
          if(gparent != 2024) {fs->FCB[gparent].size -= get_lenght(fs->FCB[i].file_name); fs->FCB[gparent].modified = gtime++;}
          fs->VCB->tot_file_num--;
          fs->VCB->empty_st -= block_num;
          for(int j = i; j < fs->VCB->tot_file_num; j++){
            fs->FCB[j] = fs->FCB[j + 1];
            fs->FCB[j].st_block -= block_num;
          
          }

          if(i < fs->VCB->tot_file_num)
          memcpy(fs->SB + st_block * fs->STORAGE_BLOCK_SIZE, fs->SB + (st_block + block_num) * 
          fs->STORAGE_BLOCK_SIZE, (fs->VCB->empty_st - st_block) * fs->STORAGE_BLOCK_SIZE);
          else memset(fs->SB + st_block * fs->STORAGE_BLOCK_SIZE, 0, size);
          gparent = 2025;
          return;
        }
      // else {
      //   printf("%s %s %s\n", fs->FCB[i].file_name, fs->FCB[i].parent->file_name, gparent->file_name);
      // } 
    
    gparent = 2025;
    printf("Delete file DNE\n");
    
    
  
  } 
  else if(op == CD) {
    for(int i = 0; i < fs->VCB->tot_file_num; i++){
      if(fs->FCB[i].parent == fs->current_dir && cmp(fs->FCB[i].file_name, s)){
        fs->current_dir = i;
        return;
      }
    }
      printf("No such subdirectory\n");
  }
  else if(op == MKDIR) {
    for(int i = 0; i < fs->VCB->tot_file_num; i++)
      if(cmp(s, fs->FCB[i].file_name) && fs->FCB[i].parent == fs->current_dir) return;

    int fp = fs->VCB->tot_file_num++;
    //update fcb
    fs->FCB[fp].create = gtime;
    if (fs->current_dir != 2024) 
      fs->FCB[fs->current_dir].modified = gtime;
    fs->FCB[fp].modified = gtime++;
    fs->FCB[fp].size = 0;
    fs->FCB[fp].size2 = 0;

    fs->FCB[fp].dir = true;
    fs->FCB[fp].st_block = fs->VCB->empty_st++;
    cpy(fs->FCB[fp].file_name, s);
    fs->FCB[fp].parent = fs->current_dir;
    if(fs->current_dir != 2024)
      fs->FCB[fs->current_dir].size += get_lenght(s);
  } 
  else if(op == RM_RF) {
    if(gparent == 2025) gparent = fs->current_dir;
    for(int i = 0; i < fs->VCB->tot_file_num; i++)
      if((fs->FCB[i].parent == gparent) && cmp(fs->FCB[i].file_name, s)){
         int idx = 0;
        // printf("%d\n", i);
        for(int j = i + 1; j < fs->VCB->tot_file_num; ){
          if(fs->FCB[j].parent == i) {
              // printf("%d\n", j);
              gparent = i;
              if(fs->FCB[j].dir) fs_gsys(fs, RM_RF, fs->FCB[j].file_name);
              else fs_gsys(fs, RM, fs->FCB[j].file_name);
              idx++;
          }
          else j++;
          // if(idx > 10) {printf("Loop!\n"); break;}
        }
        gparent = fs->current_dir;
        fs_gsys(fs, RM, fs->FCB[i].file_name);  
        gparent = 2025;
        return; 
      }
    gparent = 2025;
    return;
      }
}
__device__ void dump(FileSystem *fs){
  printf("--------------------------------------------------\n");
  for(int i = 0; i < fs->VCB->tot_file_num; i++){
    printf("%d %d %d %s %d ", i, fs->FCB[i].st_block, fs->FCB[i].size, fs->FCB[i].file_name,  fs->FCB[i].dir);
    if(fs->FCB[i].parent == 2024) printf("parent: root\n");
    else printf("parent: %s\n", fs->FCB[fs->FCB[i].parent].file_name);
    // if( fs->FCB[i].first_ch != NULL) printf(" child: %s",  fs->FCB[i].first_ch->file_name);
    // if(  fs->FCB[i].peer != NULL ) printf(" peer: %s",  fs->FCB[i].peer->file_name);
    // printf("\n");
  }
  printf("empty_st: %d", fs->VCB->empty_st);
  if(fs->current_dir == 2024) printf(" root\n");
  else printf(" %s\n", fs->FCB[fs->current_dir].file_name);
  printf("--------------------------------------------------\n");

}


