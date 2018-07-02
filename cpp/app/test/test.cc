#include <gflags/gflags.h>
#include <glog/logging.h>
#include<boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include "cpp/graphapi.h"
#include "cpp/dualsimulation.h"
#include "cpp/strongsimulation.h"
#include "cpp/dual_incremental.h"
#include "cpp/io/io_local.h"
#include "cpp/global.h"

#include "cpp/strongr.h"
#include<iostream>
#include <fstream>
#include<ctime>
//#include <sstream>
#define random(a,b) (rand()%(b-a+1)+a)
class Example {
 public:
  Example() {}
  void test_strongsimulation(){
  StrongSim strongs;
  Graph qgraph;
  Graph dgraph;
  GraphLoader dgraph_loader,qgraph_loader;
  FLAGS_vfile =  "../data/synmtic.v";
  FLAGS_efile = "../data/synmtic.e";
//  FLAGS_vfile =  "../data/yago.v";
//  FLAGS_efile = "../data/yago.e";
  dgraph_loader.LoadGraph(dgraph,FLAGS_vfile,FLAGS_efile);
//  FLAGS_vfile =  "../data/synmticquery/q1.v";
//  FLAGS_efile = "../data/synmticquery/q1.e";


//  qgraph_loader.LoadGraph(qgraph,FLAGS_vfile,FLAGS_efile);
  int index = 1;
  while (index <10){
      FLAGS_vfile =  "../data/synmticquery/q"+std::to_string(index)+".v";
      FLAGS_efile = "../data/synmticquery/q"+std::to_string(index)+".e";
//      FLAGS_vfile =  "../data/yagoquery/q"+std::to_string(index)+".v";
//      FLAGS_efile = "../data/yagoquery/q"+std::to_string(index)+".e";
      qgraph_loader.LoadGraph(qgraph,FLAGS_vfile,FLAGS_efile);
      clock_t start,end;
      start =clock();
      std::vector<StrongR> result = strongs.strong_simulation_sim(dgraph,qgraph);
//      std::cout << result.size() << std::endl;
      for(int i=0;i<result.size();++i){
          //StrongR *ball=&result[i];
          int w = result[i].center();
          std::unordered_map<VertexID,std::unordered_set<VertexID>> sim = result[i].ballr();
          for(auto u :qgraph.GetAllVerticesID()){
              std::cout<<w<<' '<<u;
              for(auto v :sim[u]){
                 std::cout<<' '<<v;
              }
              std::cout<<std::endl;
          }
      }
      end = clock();
     // std::cout<<"strong simulation time "<<(float)(end-start)/CLOCKS_PER_SEC<<"s"<<std::endl;
      index +=1;
   }
  }
 private:
  Graph testgraph_;
  GraphLoader graph_loader_;
};

int main(int argc, char *argv[]) {
  google::SetUsageMessage("Usage: test [gflags_opt]");
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::ShutDownCommandLineFlags();
  google::InitGoogleLogging("test for working");
  google::ShutdownGoogleLogging();

  Example example;
  example.test_strongsimulation();
  return 0;
}

using namespace std;