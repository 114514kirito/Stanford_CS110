#include <vector>
#include<iostream>
#include"imdb-utils.h"
#include"path.h"
#include<set>
#include <list>
#include"imdb.h"
using namespace std;

int main(int argc, char *argv[]) {
  if(argc <3 ){
    cout << "用法: " << argv[0] << " <演员1> <演员2>" << endl;
    cout << "示例: " << argv[0] << " \"Tom Hanks\" \"Leonardo DiCaprio\"" << endl;
    return 0;
  }
  imdb imdb {kIMDBDataDirectory};
  const string & player1 = argv[1];
  const string & player2 = argv[2];
  string start_player;
  string end_player;
  vector <film> player1_movie{};
  vector<film> player2_movie{};
  imdb.getCredits(player1,player1_movie);
  imdb.getCredits(player2,player2_movie);
  if(player1_movie.size() > player2_movie.size()){
     start_player = player2;
     end_player = player1;
  }else{
    start_player = player1;
    end_player = player2;
  }
  path start_path {start_player};
  // 2. 创建 BFS 队列，并把起点放进去
  list<path> partialPaths;
  partialPaths.push_back(start_path);

// 3. 记录已访问的演员和电影
  set<string> previouslySeenActors;
  set<film> previouslySeenFilms;

// 千万别忘了把起点设为“已访问”！
  previouslySeenActors.insert(start_player);
  while (!partialPaths.empty()){
    path current_path = partialPaths.front();
    partialPaths.pop_front();
    if(current_path.getLength() > 6) continue;
    string last_player = current_path.getLastPlayer();
    vector<film> films;
    imdb.getCredits(last_player,films);
    for(auto & movie : films){
      if(previouslySeenFilms.count(movie)){
        continue;
      }
      previouslySeenFilms.insert(movie);
      vector<string> players;
      imdb.getCast(movie,players);
      for(auto & player : players){
        if(previouslySeenActors.count(player))continue;
        previouslySeenActors.insert(player);
        path new_path = current_path;
        new_path.addConnection(movie,player);
        if(player == end_player) {
          if(start_player == player1){
            cout << new_path << "\n";
            return 0;
          }else{
            new_path.reverse();
            cout << new_path << "\n";
            return 0;
          }
        }
      partialPaths.push_back(new_path);
    }



  }




  }
  cout << "未能在6步之内找到从 " << player1 << " 到 " << player2 << " 的连接路径。" << endl;
}
