#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "inode.h"
#include "diskimg.h"

// remove the placeholder implementation and replace with your own
int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp) {
  char buf[512];
  int fd = fs->dfd;
  int sectorNum = INODE_START_SECTOR;
  sectorNum = sectorNum + ((inumber-1)/16);
  if(diskimg_readsector(fd,sectorNum,buf)==-1) {
    return -1;
  }
  int offset = (inumber -1)% 16;
  memcpy(inp, buf + (offset * sizeof(struct inode)), sizeof(struct inode));

  return 0;
}

// remove the placeholder implementation and replace with your own
int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp, int blockNum) {
  if(blockNum <0 || blockNum > 67238){
    return -1;
  }
  int fd =fs->dfd;
  int fileSize = inode_getsize(inp);
  if (fileSize <0) return -1;
  int isLarge = inp->i_mode & ILARG;
  if(! isLarge){
    if(blockNum >=8)return -1;
    return inp->i_addr[blockNum];
  }
  uint16_t buffer[256];
  if(blockNum < 7*256){
     int indirect_idx = blockNum / 256;
     int offset = blockNum %256;
     if(diskimg_readsector(fd,inp->i_addr[indirect_idx],buffer) == -1)return-1;
     return buffer[offset];
  }else{
    int adjusted_num = blockNum - (7 * 256);
    int dict1_index= adjusted_num /256;
    int dict2_index =adjusted_num %256;
    if(diskimg_readsector(fd,inp->i_addr[7],buffer)==-1)return -1;
    uint16_t dict1 = buffer[dict1_index];
    if(diskimg_readsector(fd,dict1,buffer) ==-1)return -1;
    uint16_t dict2 = buffer[dict2_index];
    return dict2;
  }


}

int inode_getsize(struct inode *inp) {
  return ((inp->i_size0 << 16) | inp->i_size1);
}
