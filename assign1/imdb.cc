#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "imdb.h"
#include <algorithm>
using namespace std;

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";
imdb::imdb(const string& directory) {
  const string actorFileName = directory + "/" + kActorFileName;
  const string movieFileName = directory + "/" + kMovieFileName;
  actorFile = acquireFileMap(actorFileName, actorInfo);
  movieFile = acquireFileMap(movieFileName, movieInfo);
}

bool imdb::good() const {
  return !( (actorInfo.fd == -1) ||
	    (movieInfo.fd == -1) );
}

imdb::~imdb() {
  releaseFileMap(actorInfo);
  releaseFileMap(movieInfo);
}

bool imdb::getCredits(const string& player, vector<film>& films) const {
   const int & size = *(const int *)actorFile;
   const int * begin = (const int *) actorFile +1;
   const int * end = begin + size;
   const int * offset_ptr = std::lower_bound(begin,end,player,[this](int offset,std::string name){
      const char * ptr = (const char *)(actorFile) +offset;
      return std::string (ptr) < name;
   });
    // 角色信息的位置 1.名字 2.电影数量 3.电影的具体的offset
   if( offset_ptr != end){
      const char * ptr = (const char *)(actorFile) + (*offset_ptr);
      string found_name {ptr};
      if(found_name == player){

      }else{
        return false;
      }

   }else{
    return false;
   }
   const char * ptr = (const char *)(actorFile) + (*offset_ptr);
   int size_name = player.length()+1;
   if(size_name % 2 !=0){
      size_name +=1;
   }
   short num_movie = *(const short*)(ptr + size_name);
   int movie_offset;
   if((size_name +2 ) % 4 !=0){
      movie_offset = size_name + 4;
   }else{
      movie_offset = size_name + 2;
   }
   const int * movie_offsets = (const int *) (ptr + movie_offset);
   for(int i=0 ;i< num_movie;i++){
      int current_movie_offset = movie_offsets[i];
      const char * movie_record_ptr = (const char *) movieFile + current_movie_offset;
      string film_title {movie_record_ptr};
      int title_len = film_title .length() +1;
      char year_byte = *(movie_record_ptr + title_len);
      int film_year{1900 + year_byte};
      film f {film_title , film_year};
      films.push_back(f);


   }
   return true;






}

bool imdb::getCast(const film& movie, vector<string>& players) const {
   const int & size = * (const int *)(movieFile);
   const int * begin = (const int*)movieFile +1 ;
   const int * end = begin + size;
   const int * offset_ptr = std::lower_bound(begin,end,movie,[this](int offset ,const film & movie ){
      const char * ptr = (const char *) movieFile +offset;
      film current_movie ;
      current_movie.title = string {ptr};
      int movie_offset = current_movie.title.length() +1;
      current_movie.year = *((const char*) ptr +movie_offset) + 1900;
      return current_movie < movie;

   });
   if(offset_ptr < end){
      const char * ptr =  (const char*) movieFile + (* offset_ptr);
      film current_movie ;
      current_movie.title = string {ptr};
      int movie_offset = current_movie.title.length() +1;
      current_movie.year = *((const char*) ptr +movie_offset) + 1900;
      if (current_movie == movie){

      }else{
         return false;
      }
   }else{
      return false;
   }
    const char * ptr =  (const char*) movieFile + (* offset_ptr);
    film current_movie ;
    current_movie.title = string {ptr};
    int movie_offset = current_movie.title.length() +2;
    if(movie_offset % 2 !=0){
      movie_offset +=1;
    }
    const short & size_player = * (const short *) (ptr + movie_offset);
    if((movie_offset+2)% 4 !=0){
      movie_offset +=4;
    }else{
      movie_offset +=2;
    }
    const int * players_offset = (const int *) (ptr +movie_offset);
    for(int i=0 ;i<size_player ;i++){
      int offset = players_offset[i];
      const char * ptr_players = (const char*) actorFile +offset;
      string name {ptr_players};
      players.push_back(name);
    }
    return true;




}

const void *imdb::acquireFileMap(const string& fileName, struct fileInfo& info) {
  struct stat stats;
  stat(fileName.c_str(), &stats);
  info.fileSize = stats.st_size;
  info.fd = open(fileName.c_str(), O_RDONLY);
  return info.fileMap = mmap(0, info.fileSize, PROT_READ, MAP_SHARED, info.fd, 0);
}

void imdb::releaseFileMap(struct fileInfo& info) {
  if (info.fileMap != NULL) munmap((char *) info.fileMap, info.fileSize);
  if (info.fd != -1) close(info.fd);
}
