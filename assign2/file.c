#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "file.h"
#include "inode.h"
#include "diskimg.h"

// remove the placeholder implementation and replace with your own
int file_getblock(struct unixfilesystem *fs, int inumber, int blockNum, void *buf) {
  struct inode in;
  if(inode_iget(fs,inumber,&in) ==-1){
    fprintf(stderr, "file_getblock(inumber = %d, blockNum = %d) unimplemented. returning -1\n", inumber, blockNum);
    return -1;
  }
  int physciBlockNum = inode_indexlookup(fs,&in,blockNum);
  if(physciBlockNum == -1){
    fprintf(stderr, "file_getblock(inumber = %d, blockNum = %d) unimplemented. returning -1\n", inumber, blockNum);
    return -1;
  }
  if(diskimg_readsector(fs->dfd,physciBlockNum,buf) == -1){
    return -1;
  }
  int bytesLeft = inode_getsize(&in) - blockNum * DISKIMG_SECTOR_SIZE;
  return bytesLeft > DISKIMG_SECTOR_SIZE ? DISKIMG_SECTOR_SIZE : bytesLeft;

}
