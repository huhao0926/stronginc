#include <gflags/gflags.h>
#include <glog/logging.h>
#include<boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include "cpp/core/graphapi.h"
#include "cpp/serial/dualsimulation.h"
#include "cpp/serial/strongsimulation.h"
#include "cpp/serial/dual_incremental.h"
#include "cpp/io/io_local.h"
#include "cpp/core/global.h"
#include "cpp/utils/util.h"
#include<iostream>
#include <fstream>
#include<ctime>
//#include <sstream>
#define random(a,b) (rand()%(b-a+1)+a)

void test_query_info(){
  Graph qgraph;
  Graph dgraph;
  GraphLoader dgraph_loader,qgraph_loader;
  FLAGS_vfile =  "../data/synmtic.v";
  FLAGS_efile = "../data/synmtic.e";
  dgraph_loader.LoadGraph(dgraph,FLAGS_vfile,FLAGS_efile);
  int index = 1;
  while (index <10000){
      FLAGS_vfile =  "../data/query/q"+std::to_string(index)+".v";
      FLAGS_efile = "../data/query/q"+std::to_string(index)+".e";
      qgraph_loader.LoadGraph(qgraph,FLAGS_vfile,FLAGS_efile);
      //std::cout<<(float)(end-start)/CLOCKS_PER_SEC<<"s"<<std::endl;
      std::cout<<cal_diameter_qgraph(qgraph)<<" "<<query_labl_all_notsame(qgraph)<<std::endl;
      index +=1;
   }


}

void test_generate_query_graph(){
  Graph qgraph;
  Graph dgraph;
  GraphLoader dgraph_loader,qgraph_loader;
  FLAGS_vfile =  "../data/synmtic.v";
  FLAGS_efile = "../data/synmtic.e";
  dgraph_loader.LoadGraph(dgraph,FLAGS_vfile,FLAGS_efile);
  int index =1;
  while (index<2){
      Graph qgraph;
      generate_connect_graphs_by_Dgraph(dgraph,qgraph,3);
      FLAGS_vfile =  "../data/query/q"+std::to_string(index)+".v";
      FLAGS_efile = "../data/query/q"+std::to_string(index)+".e";
      save_grape_file(qgraph,FLAGS_vfile,FLAGS_efile);
      index+=1;
  }


}

  std::pair<VertexID,VertexID> generate_one_add_edge_center(int dgraph_num_vertices,std::unordered_set<VertexID> &max_dual_set, std::unordered_set<VertexID> &affectted_center_node, std::set<std::pair<VertexID,VertexID>> &exist_edges){
      std::vector<VertexID> node_list;
      for(auto v : affectted_center_node){
          node_list.push_back(v);
      }
      while(true){
          VertexID n1 = random(0,node_list.size());
          VertexID n2 = random(0,node_list.size());
          if (n1!=n2){
              if (exist_edges.find(std::pair<VertexID,VertexID>(n1,n2)) == exist_edges.end()){
                  return std::pair<VertexID,VertexID>(n1,n2);
              }
          }
      }
  }

  std::pair<VertexID,VertexID> generate_one_add_edge_outside_center(int dgraph_num_vertices, std::unordered_set<VertexID> &max_dual_set, std::unordered_set<VertexID> &affectted_center_node, std::set<std::pair<VertexID,VertexID>> &exist_edges){
      std::vector<VertexID> node_list;
      for(int i=0;i<dgraph_num_vertices;i++){
          if(affectted_center_node.find(i) == affectted_center_node.end()){
             node_list.push_back(i);
          }
      }
      while(true){
          VertexID n1 = random(0,node_list.size()-1);
          VertexID n2 = random(0,node_list.size()-1);
          if (n1!=n2){
              if (exist_edges.find(std::pair<VertexID,VertexID>(n1,n2)) == exist_edges.end()){
                  return std::pair<VertexID,VertexID>(n1,n2);
              }
          }
      }
  }

  std::pair<VertexID,VertexID> generate_one_remove_edge_center(int dgraph_num_vertices, std::unordered_set<VertexID> &max_dual_set, std::unordered_set<VertexID> &affectted_center_node, std::set<std::pair<VertexID,VertexID>> &exist_edges){
      std::vector<std::pair<VertexID,VertexID>> center_edges;
      for(auto e :exist_edges){
           if(affectted_center_node.find(e.first)!=affectted_center_node.end() && affectted_center_node.find(e.second) != affectted_center_node.end()){
                center_edges.push_back(e);
           }
      }
      while (true){
          int n1 = random(0,center_edges.size()-1);
          return center_edges[n1];

      }
  }

  std::pair<VertexID,VertexID> generate_one_remove_edge_outside_center(int dgraph_num_vertices, std::unordered_set<VertexID> &max_dual_set, std::unordered_set<VertexID> &affectted_center_node, std::set<std::pair<VertexID,VertexID>> &exist_edges){
      std::vector<std::pair<VertexID,VertexID>> center_edges;
      for(auto e :exist_edges){
           if(affectted_center_node.find(e.first)==affectted_center_node.end() && affectted_center_node.find(e.second) == affectted_center_node.end()){
                center_edges.push_back(e);
           }
      }
      while (true){
          int n1 = random(0,center_edges.size()-1);
          return center_edges[n1];

      }
  }


int main(int argc, char *argv[]) {
  google::SetUsageMessage("Usage: test [gflags_opt]");
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::ShutDownCommandLineFlags();
  google::InitGoogleLogging("test for working");
  google::ShutdownGoogleLogging();

  test_generate_query_graph();
  return 0;
}

using namespace std;