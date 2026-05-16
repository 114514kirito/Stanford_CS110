#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include "file.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

// remove the placeholder implementation and replace with your own
int directory_findname(struct unixfilesystem *fs, const char *name,
		       int dirinumber, struct direntv6 *dirEnt) {
  struct inode in;
  if(inode_iget(fs,dirinumber,&in) == -1)return -1;
  if(!(in.i_mode & IALLOC && in.i_mode & IFDIR)) return -1;
  int fileSize = inode_getsize(&in);
  int numberSize;
  if(fileSize % DISKIMG_SECTOR_SIZE ==0){
    numberSize = fileSize / DISKIMG_SECTOR_SIZE ;
  }else{
    numberSize = fileSize /DISKIMG_SECTOR_SIZE +1;
  }
  if(numberSize <0)return -1;
  struct direntv6 buf[32];
  for(int i=0;i< numberSize;i++){
    int blockSize = file_getblock(fs,dirinumber,i,buf);
    if(blockSize == -1)return -1;
    for(int j =0; j< blockSize/16;j++){
      if(strncmp(buf[j].d_name,name,sizeof(buf[j].d_name))==0)
      {
        *dirEnt =buf[j];
        return 0;

      }
    }
  }
  return -1;




}
