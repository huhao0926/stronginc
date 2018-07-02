#include <gflags/gflags.h>
#include <glog/logging.h>
#include<boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include "cpp/global.h"
#include "cpp/FragmentLoader.h"
#include "cpp/Fragment.h"
#include "cpp/graphapi.h"
#include "cpp/io/io_local.h"
#include "cpp/bfs_parallel.h"
#include "cpp/dualsimulation_parallel.h"
#include "cpp/dualsimulation.h"
#include "cpp/bfs_singlekhop.h"
#include "cpp/bfs_multikhop.h"
#include "cpp/bfs_connectivity.h"
#include "cpp/dual_incfilter.h"
#include "cpp/MessageBuffer.h"
#include "cpp/strongsim_parallel.h"
#include<iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include<ctime>
//#include <sstream>
#define random(a,b) (rand()%(b-a+1)+a)
//mpirun -np 5 ./example 2> 1.txt
 bool dual_the_same(Graph &qgraph, std::unordered_map<VertexID, std::unordered_set<VertexID>> &direct_sim, std::unordered_map<VertexID, std::unordered_set<VertexID>> &incdsim){
   for (auto u:qgraph.GetAllVerticesID()){
    if(direct_sim[u].size()!=incdsim[u].size()){
    return false;
    }
    for (auto v : direct_sim[u]){
        if(incdsim[u].find(v) == incdsim[u].end()){
           return false;
        }
    }

   }
   return true;

 }

  void test_load(int fid){
  FragmentLoader fragmentloader;
  std::string v_file  ="../data/twitter.v";
  std::string e_file = "../data/twitter.e";
  std::string r_file = "../data/twitter.r";
  std::vector<Vertex> global_vertices;
  std::vector<Edge> global_edges;
  std::unordered_map<VertexID,int> fragTable;
  fragmentloader.LoadFragTable(fragTable,r_file);
  std::unordered_set<VertexID> idx;
  fragmentloader.LoadEdge(global_edges, fragTable,idx, fid, e_file);
  std::cout<<"edges "<<global_edges.size()<<' '<<fid<<std::endl;
  fragmentloader.LoadVertex(global_vertices, fragTable,idx, fid, v_file);
  std::cout<<"vertex "<<global_vertices.size()<<" "<<fid<<std::endl;
  }

  void test_fragment_loader(int fid){
  Graph dgraph,fragmentgraph;
  FragmentLoader fragmentloader;
  std::string v_file  ="../data/synmtic.v";
  std::string e_file = "../data/synmtic.e";
  std::string r_file = "../data/synmtic.r";

  GraphLoader dgraph_loader;
  dgraph_loader.LoadGraph(dgraph,v_file,e_file);

  Fragment fragment(fragmentgraph,v_file,e_file,r_file);

  for(int i=0;i<10000;i++){
      if(fragment.has_vertex(i)){
          std::cout<<i<<' '<<fragmentgraph.GetVertexLabel(fragment.getLocalID(i))<<std::endl;
      }

  }
//  for(auto v :fragmentgraph.GetAllVerticesID()){
//      std::cout<<fragment.getGlobalID(v)<<' '<<fragmentgraph.GetVertexLabel(v)<<std::endl;
//  }
 }

 void test_bfs(int fid){
  Graph dgraph,fragmentgraph;
 // FragmentLoader fragmentloader;
  std::string v_file  ="../data/synmtic.v";
  std::string e_file = "../data/synmtic.e";
  std::string r_file = "../data/synmtic.r";

  GraphLoader dgraph_loader;
  dgraph_loader.LoadGraph(dgraph,v_file,e_file);

  Fragment fragment(fragmentgraph,v_file,e_file,r_file);
  for(int i=0;i<1000;i++){
      Bfs_Parallel bfs_parallel;
     std::unordered_map<VertexID,int> DistInf;
     bfs_parallel.bfs_parallel(fragment,fragmentgraph,DistInf,i);
     std::unordered_map<VertexID,int> dis = dgraph.shortest_distance(i);
     const std::unordered_set<VertexID> p = *fragment.getInnerVertices();
     if (fid ==0){
     for(auto v : p){
      if (DistInf.find(v) == DistInf.end()){
         std::cout<<INT_MAX<<' '<<dis[v]<<' '<<std::endl;
//      continue;
      }
//      std::cout<<DistInf[v]<<' '<<dis[v]<<std::endl;
      if(DistInf[v] != dis[v] ){
          std::cout<<"root: "<<i<<"target: "<<v<<' '<<DistInf[v]<<' '<<dis[v]<<std::endl;
//          return;
      }
  }
  if(fid==0){
  std::cout<<i<<std::endl;
  }

  }
  }
 }

 void test_dual_parallel(int fid){
  Graph dgraph,fragmentgraph,qgraph;
  FragmentLoader fragmentloader;
  std::string v_file  ="../data/synmtic.v";
  std::string e_file = "../data/synmtic.e";
  std::string r_file = "../data/synmtic.r";

  GraphLoader dgraph_loader,qgraph_loader;
  dgraph_loader.LoadGraph(dgraph,v_file,e_file);

  Fragment fragment(fragmentgraph,v_file,e_file,r_file);
  int index = 1;

  DualSim dualsim;
  while (index <200){
      std::string qv_file =  "../data/synmticquery/q"+std::to_string(index)+".v";
      std::string qe_file = "../data/synmticquery/q"+std::to_string(index)+".e";
      qgraph_loader.LoadGraph(qgraph,qv_file,qe_file);

      std::unordered_map<VertexID, std::unordered_set<VertexID>> sim,psim;

      Dual_Parallel dualparallel;
      dualparallel.dual_paraller(fragment,fragmentgraph,qgraph,psim);

//      for(auto u :qgraph.GetAllVerticesID()){
//          std::cout<<fid<<" "<<u<<' '<<psim[u].size()<<std::endl;
//      }

      if (fid==0){
            std::vector<std::unordered_map<VertexID, std::unordered_set<VertexID>>>  tmp_vec(get_num_workers());
            tmp_vec[fid] = psim;
            masterGather(tmp_vec);
            std::unordered_map<VertexID, std::unordered_set<VertexID>>  globalsim;
            for(auto u :qgraph.GetAllVerticesID()){
                 globalsim[u] = std::unordered_set<VertexID>();
             }
            for(int i=0;i<get_num_workers();i++){
                for(auto u :qgraph.GetAllVerticesID()){
                     for(auto v : tmp_vec[i][u])
                     {globalsim[u].insert(v);}
                }
            }

            DualSim dualsim;
            bool initialized_sim = false;
            dualsim.dual_simulation(dgraph,qgraph,sim,initialized_sim);

//            for(auto u :qgraph.GetAllVerticesID()){
//                std::cout<<"direct "<<sim[u].size()<<' '<<globalsim[u].size()<<std::endl;
//             }
            worker_barrier();
            std::cout<<"index: "<<index<<' '<<dual_the_same(qgraph,sim,globalsim)<<std::endl;

      }else{
         slaveGather(psim);
         worker_barrier();
      }

  index += 1;
  }
 }

void test_bfs_khop(int fid){
  Graph dgraph,fragmentgraph;
  FragmentLoader fragmentloader;
  std::string v_file  ="../data/synmtic.v";
  std::string e_file = "../data/synmtic.e";
  std::string r_file = "../data/synmtic.r";

  GraphLoader dgraph_loader;
  dgraph_loader.LoadGraph(dgraph,v_file,e_file);

  Fragment fragment(fragmentgraph,v_file,e_file,r_file);

  int index = 0 ;
  while (index <10000){
      int root = random(0,5000);
      int bound = random(1,7);
//      int root = 3598;
//      int bound = 5;
      Bfs_Singlekhop bfs_singlekhop;
      std::unordered_set<VertexID> nodeset;
      bfs_singlekhop.bfs_singlekhopparallel(fragment,fragmentgraph,nodeset,root,bound);
      if (fid == 0){
        std::unordered_set<VertexID> global_node;
         std::vector<std::unordered_set<VertexID>>  tmp_vec(get_num_workers());
        tmp_vec[fid] = nodeset;
        masterGather(tmp_vec);
        for(int i =0; i< get_num_workers(); ++i){
            for(auto v :tmp_vec[i]){
                global_node.insert(v);
            }
        }
        worker_barrier();
        std::unordered_set<VertexID> direct_resutl = dgraph.find_hop_nodes(root,bound);
//        std::cout<<"false "<<root<<' '<<bound<<' '<<global_node.size()<<' '<<direct_resutl.size()<<std::endl;
        if (global_node.size() != direct_resutl.size()){
            std::cout<<"false "<<root<<' '<<bound<<' '<<global_node.size()<<' '<<direct_resutl.size()<<std::endl;
        }else{
            for(auto v :global_node){
                if(direct_resutl.find(v) == direct_resutl.end()){
                    std::cout<<"flas "<<std::endl;
                }
            }
        }
      }else{
       slaveGather(nodeset);
       worker_barrier();
    }
    ++index;
  }
}

//void test_bfs_connectivity(int fid){
//  Graph dgraph,fragmentgraph;
//  FragmentLoader fragmentloader;
//  std::string v_file  ="../data/synmtic.v";
//  std::string e_file = "../data/synmtic.e";
//  std::string r_file = "../data/synmtic.r";
//
//  GraphLoader dgraph_loader;
//  dgraph_loader.LoadGraph(dgraph,v_file,e_file);
//
//  Fragment fragment(fragmentgraph,v_file,e_file,r_file);
//
// int index = 0 ;
//  while (index <10000){
//      int root = random(0,9999);
//      Bfs_Connectivity bfs_connectivity;
//      std::unordered_set<VertexID> nodeset;
//      bfs_connectivity.bfs_parallel(fragment,fragmentgraph,nodeset,root);
//      if (fid == 0){
//        std::unordered_set<VertexID> global_node;
//         std::vector<std::unordered_set<VertexID>>  tmp_vec(get_num_workers());
//        tmp_vec[fid] = nodeset;
//        masterGather(tmp_vec);
//        for(int i =0; i< get_num_workers(); ++i){
//            for(auto v :tmp_vec[i]){
//                global_node.insert(v);
//            }
//        }
//        worker_barrier();
//        std::unordered_set<VertexID> direct_resutl = dgraph.find_connectivity_nodes(root);
//        if (global_node.size() != direct_resutl.size()){
//            std::cout<<"false "<<root<<' '<<global_node.size()<<' '<<direct_resutl.size()<<std::endl;
//        }else{
//            for(auto v :global_node){
//                if(direct_resutl.find(v) == direct_resutl.end()){
//                    std::cout<<"flas "<<std::endl;
//                }
//            }
//        }
//      }else{
//       slaveGather(nodeset);
//       worker_barrier();
//    }
//    ++index;
//  }
//
//
//}

 void test_dual_filterparallel(int fid){
  Graph dgraph,fragmentgraph,qgraph;
  FragmentLoader fragmentloader;
  std::string v_file  ="../data/synmtic.v";
  std::string e_file = "../data/synmtic.e";
  std::string r_file = "../data/synmtic.r";

  GraphLoader dgraph_loader,qgraph_loader;
  dgraph_loader.LoadGraph(dgraph,v_file,e_file);

  Fragment fragment(fragmentgraph,v_file,e_file,r_file);
  int index = 1;

  DualSim dualsim;
  while (index <200){
      std::string qv_file =  "../data/synmticquery/q"+std::to_string(index)+".v";
      std::string qe_file = "../data/synmticquery/q"+std::to_string(index)+".e";
      qgraph_loader.LoadGraph(qgraph,qv_file,qe_file);

      std::unordered_map<VertexID, std::unordered_set<VertexID>> sim,psim;

      Dual_Parallel dualparallel(fragment);
      bool initialized_sim = false;
      std::unordered_map<VertexID, std::unordered_set<VertexID>> remove_pred;
      std::unordered_map<VertexID, std::unordered_set<VertexID>> remove_succ;

      dualparallel.dual_sim_initialization(fragment, fragmentgraph, qgraph,psim, initialized_sim,remove_pred,remove_succ);
    MessageBuffer<std::pair < VertexID, int>> messageBuffers;
    messageBuffers.init();
    for(auto u:qgraph.GetAllVerticesID()){
        for(auto v:*fragment.getInnerVertices()){
            if (qgraph.GetVertexLabel(u)==fragmentgraph.GetVertexLabel(fragment.getLocalID(v)) && psim[u].find(fragment.getLocalID(v)) == psim[u].end()){
                if (fragment.isBorderVertex(v)){
                    std::bitset<NUM_FRAGMENTS> dest= fragment.getMsgThroughDest(v);
                    for (int fid = 0; fid < dest.size(); fid++){
                        if (dest.test(fid) && fid != get_worker_id()){
                            messageBuffers.add_message(fid,std::pair<VertexID,int>(u, v));
                            }
                     }
                 }
            }
        }
    }
    messageBuffers.sync_messages();
    for (auto item:messageBuffers.get_messages()){
        VertexID u = item.first;
        VertexID v = item.second;
        if(psim[u].find(fragment.getLocalID(v)) != psim[u].end()){
           psim[u].erase(fragment.getLocalID(v));
        }
    }
    messageBuffers.reset_in_messages();

      Dual_Incfilter dual_incfilter;
      std::unordered_set<VertexID> view_node;
      for(auto u : qgraph.GetAllVerticesID()){
          for(auto v :psim[u]){
              view_node.insert(v);
          }
      }
      GraphView graph_view(fragmentgraph,&view_node);
      dual_incfilter.dual_incfilter(fragment,graph_view,qgraph,psim);

//      for(auto u :qgraph.GetAllVerticesID()){
//          std::cout<<fid<<" "<<u<<' '<<psim[u].size()<<std::endl;
//      }

      if (fid==0){
            std::vector<std::unordered_map<VertexID, std::unordered_set<VertexID>>>  tmp_vec(get_num_workers());
            tmp_vec[fid] = psim;
            masterGather(tmp_vec);
            std::unordered_map<VertexID, std::unordered_set<VertexID>>  globalsim;
            for(auto u :qgraph.GetAllVerticesID()){
                 globalsim[u] = std::unordered_set<VertexID>();
             }
            for(int i=0;i<get_num_workers();i++){
                for(auto u :qgraph.GetAllVerticesID()){
                     for(auto v : tmp_vec[i][u])
                     {globalsim[u].insert(v);}
                }
            }

            DualSim dualsim;
            bool initialized_sim = false;
            dualsim.dual_simulation(dgraph,qgraph,sim,initialized_sim);

//            for(auto u :qgraph.GetAllVerticesID()){
//                std::cout<<"direct "<<sim[u].size()<<' '<<globalsim[u].size()<<std::endl;
//             }
            worker_barrier();
            std::cout<<"index: "<<index<<' '<<dual_the_same(qgraph,sim,globalsim)<<std::endl;

      }else{
         slaveGather(psim);
         worker_barrier();
      }

  index += 1;
  }
 }

 void test_strong_parallel(int fid){
  Graph dgraph,fragmentgraph,qgraph;
  FragmentLoader fragmentloader;
  std::string v_file  ="../data/synmtic.v";
  std::string e_file = "../data/synmtic.e";
  std::string r_file = "../data/synmtic.r";

  GraphLoader dgraph_loader,qgraph_loader;
  dgraph_loader.LoadGraph(dgraph,v_file,e_file);

  Fragment fragment(fragmentgraph,v_file,e_file,r_file);
  int index = 1;

  DualSim dualsim;
  while (index <10){
      std::string qv_file =  "../data/synmticquery/q"+std::to_string(index)+".v";
      std::string qe_file = "../data/synmticquery/q"+std::to_string(index)+".e";
      qgraph_loader.LoadGraph(qgraph,qv_file,qe_file);
      Strong_Parallel strong_parallel;
      strong_parallel.strongsim_parallel(fragment,fragmentgraph,qgraph);
      worker_barrier();
//      if (fid ==0){
//      std::cout<<index<<std::endl;
//      }


  index += 1;
  }
 }

int main(int argc, char *argv[]) {
  google::SetUsageMessage("Usage: test [gflags_opt]");
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::ShutDownCommandLineFlags();
  google::InitGoogleLogging("test for working");
  google::ShutdownGoogleLogging();

  //worker_barrier();
  init_workers();
   int rank =get_worker_id();
  int num_workers =get_num_workers();
  worker_barrier();

 // std::cout<<"process:"<<rank<<' '<<num_workers<<std::endl;
  test_strong_parallel(rank);
  worker_finalize();

//  Example example;
//  example.test_load();
  return 0;
}

using namespace std;