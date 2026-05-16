
#include "pathname.h"
#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
void skip_element(char*,char*);
int pathname_lookup(struct unixfilesystem *fs, const char *pathname) {
  // remove the placeholder implementation and replace with your own
  if(strcmp(pathname,"/") ==0) return ROOT_INUMBER;
  int dirinumber = ROOT_INUMBER;
  char buf[256];
  char * p=buf;
  skip_element(pathname,buf);
  struct direntv6 dirEnt;
  struct inode in;
  inode_iget(fs,dirinumber,&in);
  while ((in.i_mode & IALLOC) && (in.i_mode & IFMT) ==IFDIR )
  {
    if(*p == '\0')
    {
      p++;
    }
    if(*p== '\0') break;

    char name[256];
    char * start = p;
    while (*p != '\0' )
    {
      p++;
    }
    int len = p-start;
    strncpy(name,start,len);
    name[len] = '\0';
    if(directory_findname(fs,name,dirinumber,&dirEnt) ==-1){
      return -1;
    }
    dirinumber = dirEnt.d_inumber;
    if(inode_iget(fs,dirinumber,&in)==-1){
      return -1;
    }



  }
  if(dirinumber != ROOT_INUMBER) return dirinumber;
  else{
    return -1;
  }


}
void skip_element(char * path,char * name){
  if(path ==NULL || *path == '\0'){
    *name ='\0';
    return ;

  }
  while (*path == '/' )
  {
    /* code */
    path ++ ;
  }
  char * start =path;
  while (*path !='/' && *path !='\0')
  {
    /* code */
    path++;
  }
  int len = path -start;
  strncpy(name,start,len);
  name[len] = '\0';
  name += len +1;
  skip_element(path,name);



}
