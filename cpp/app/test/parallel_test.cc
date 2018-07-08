#include <gflags/gflags.h>
#include <glog/logging.h>
#include<boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
//#include "cpp/core/graphapi.h"
#include "cpp/serial/dualsimulation.h"
#include "cpp/serial/dual_incremental.h"
#include "cpp/parallel/dualsimulation_parallel.h"
#include "cpp/core/Fragment.h"
#include "cpp/core/graph_view.h"
#include "cpp/core/FragmentLoader.h"
#include "cpp/utils/util.h"
#include "cpp/io/io_local.h"
#include "cpp/core/global.h"
#include "cpp/core/strongr.h"
#include "cpp/parallel/dual_incfilter.h"
#include "cpp/parallel/bfs_parallel.h"
#include "cpp/parallel/bfs_singlekhop.h"
#include "cpp/parallel/bfs_multikhop.h"
#include "cpp/parallel/bfs_connectivity.h"
#include<iostream>
#include <fstream>
#include<ctime>
//#include <sstream>
#define random(a,b) (rand()%(b-a+1)+a)
//mpirun -np 5 ./strongexc 2>log.txt

class Parallel{
public:
std::string get_query_vfile(int index){
        return "../data/synmticquery/q"+std::to_string(index)+".v";
}

std::string get_query_efile(int index){
        return "../data/synmticquery/q"+std::to_string(index)+".e";
}
public:

void test_fragment_loader(int fid){
  Graph dgraph,fragmentgraph;
  FragmentLoader fragmentloader;
  GraphLoader dgraph_loader;
  dgraph_loader.LoadGraph(dgraph,graph_vfile,graph_efile);

  Fragment fragment(fragmentgraph,graph_vfile,graph_efile,r_file);

  for(int i=0;i<10000;i++){
      if(fragment.has_vertex(i)){
          std::cout<<i<<' '<<fragmentgraph.GetVertexLabel(fragment.getLocalID(i))<<' '<<dgraph.GetVertexLabel(i)<<std::endl;
      }
  }
 }

void test_dual_parallel(int fid){
  Graph dgraph,fragmentgraph,qgraph;
  FragmentLoader fragmentloader;
/* note off dualsimulation_parallel.cc  276
*/
  GraphLoader dgraph_loader,qgraph_loader;
  dgraph_loader.LoadGraph(dgraph,graph_vfile,graph_efile);
  Fragment fragment(fragmentgraph,graph_vfile,graph_efile,r_file);
  int index = 1;
  DualSim dualsim;
  while (index <200){
       Graph qgraph;
      qgraph_loader.LoadGraph(qgraph,get_query_vfile(index),get_query_efile(index));
      std::unordered_map<VertexID, std::unordered_set<VertexID>> sim,psim;
      Dual_Parallel dualparallel;
      dualparallel.dual_paraller(fragment,fragmentgraph,qgraph,psim);
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
            worker_barrier();
            std::cout<<"index: "<<index<<' '<<dual_the_same(qgraph,sim,globalsim)<<std::endl;
      }else{
         slaveGather(psim);
         worker_barrier();
      }
  index += 1;
  }
 }

void test_dual_filterparallel(int fid){
  Graph dgraph,fragmentgraph;
  FragmentLoader fragmentloader;
  GraphLoader dgraph_loader,qgraph_loader;
  dgraph_loader.LoadGraph(dgraph,graph_vfile,graph_efile);
  Fragment fragment(fragmentgraph,graph_vfile,graph_efile,r_file);
  int index = 1;

  DualSim dualsim;
  while (index <200){
      Graph qgraph;
      qgraph_loader.LoadGraph(qgraph,get_query_vfile(index),get_query_efile(index));
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
            worker_barrier();
            std::cout<<"index: "<<index<<' '<<dual_the_same(qgraph,sim,globalsim)<<std::endl;

      }else{
         slaveGather(psim);
         worker_barrier();
      }
  index += 1;
  }
 }

void test_bfs(int fid){
  Graph dgraph,fragmentgraph;
  GraphLoader dgraph_loader;
  dgraph_loader.LoadGraph(dgraph,graph_vfile,graph_efile);

  Fragment fragment(fragmentgraph,graph_vfile,graph_efile,r_file);
  for(int i=0;i<1000;i++){
     Bfs_Parallel bfs_parallel;
     std::unordered_map<VertexID,int> DistInf;
     bfs_parallel.bfs_parallel(fragment,fragmentgraph,DistInf,i);
     std::unordered_map<VertexID,int> dis;
     dgraph.shortest_distance(i,dis);
     const std::unordered_set<VertexID> p = *fragment.getInnerVertices();
     if (fid ==0){
         for(auto v : p){
            if (DistInf.find(v) == DistInf.end()){
                std::cout<<INT_MAX<<' '<<dis[v]<<' '<<std::endl;
            }
             if(DistInf[v] != dis[v] ){
                std::cout<<"root: "<<i<<"target: "<<v<<' '<<DistInf[v]<<' '<<dis[v]<<std::endl;
             }
         }
     }
  }
 }

void test_bfs_khop(int fid){
  srand( (unsigned)time(0));
  Graph dgraph,fragmentgraph;
  FragmentLoader fragmentloader;
  GraphLoader dgraph_loader;
  dgraph_loader.LoadGraph(dgraph,graph_vfile,graph_efile);
 //when test note denote bfs_singlekhop.cc  18
  Fragment fragment(fragmentgraph,graph_vfile,graph_efile,r_file);
  int index = 0 ;
  while (index <10000){
      int root = random(0,5000);
      int bound = random(1,7);
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
        std::unordered_set<VertexID> direct_result;
        dgraph.find_hop_nodes(root,bound, direct_result);
//        std::cout<<"false "<<root<<' '<<bound<<' '<<global_node.size()<<' '<<direct_resutl.size()<<std::endl;
        if (global_node.size() != direct_result.size()){
            std::cout<<"false "<<root<<' '<<bound<<' '<<global_node.size()<<' '<<direct_result.size()<<std::endl;
        }else{
            for(auto v :global_node){
                if(direct_result.find(v) == direct_result.end()){
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

/**
  *serial find affected_area
  */
std::unordered_set<VertexID> find_affected_area(Graph &dgraph,std::set<std::pair<VertexID,VertexID>> &add_edges,std::set<std::pair<VertexID,VertexID>> &rm_edges,int d_Q){
       std::unordered_set<VertexID> incedges_node,affected_nodes;
        for(auto e:add_edges){
            if(rm_edges.find(e) == rm_edges.end()){
                incedges_node.insert(e.first);
                incedges_node.insert(e.second);
            }
        }
        for(auto e:rm_edges){
            if(add_edges.find(e) == add_edges.end()){
                incedges_node.insert(e.first);
                incedges_node.insert(e.second);
            }
        }
     dgraph.find_hop_nodes(incedges_node,d_Q,affected_nodes);
     return affected_nodes;
  }
/**
  *parallel find affected_area
  */
std::unordered_set<VertexID> find_affected_area(Fragment& fragment, Graph& dgraph, std::set<std::pair<VertexID,VertexID>> &add_edges,std::set<std::pair<VertexID,VertexID>> &rm_edges,int d_Q){
        std::unordered_set<VertexID> incedges_node;
        for(auto e:add_edges){
            if(rm_edges.find(e) == rm_edges.end()){
                incedges_node.insert(e.first);
                incedges_node.insert(e.second);
            }
        }
        for(auto e:rm_edges){
            if(add_edges.find(e) == add_edges.end()){
                incedges_node.insert(e.first);
                incedges_node.insert(e.second);
            }
        }
        std::unordered_set<VertexID> result_nodes;
        Bfs_Multikhop bfs_multikhop;
        bfs_multikhop.bfs_multikhopparallel(fragment, dgraph, result_nodes, incedges_node, d_Q);
        return result_nodes;
}

void test_multikhop(int fid){
//before test,note off bfs_multikhop.cc 18
  Graph dgraph,fragmentgraph,qgraph;
  FragmentLoader fragmentloader;
  GraphLoader dgraph_loader,qgraph_loader;
  dgraph_loader.LoadGraph(dgraph,graph_vfile,graph_efile);
  Fragment fragment(fragmentgraph,graph_vfile,graph_efile,r_file);
  int i =1;
  while(i<20){
         std::set<std::pair<VertexID,VertexID>> add_edges;
         std::set<std::pair<VertexID,VertexID>> rm_edges;
         Load_bunch_edges(add_edges,base_add_file,i);
         Load_bunch_edges(rm_edges,base_remove_file,i);
         std::unordered_set<VertexID> all_affected_nodes = find_affected_area(dgraph,add_edges,rm_edges,2);
         std::unordered_set<VertexID> partial_affected_nodes = find_affected_area(fragment,fragmentgraph,add_edges,rm_edges,2);
         if (fid==0){
            std::vector<std::unordered_set<VertexID>>  tmp_vec(get_num_workers());
            tmp_vec[fid] = partial_affected_nodes;
            masterGather(tmp_vec);
            std::unordered_set<VertexID>   global_affected_nodes;
            for(int i=0;i<get_num_workers();i++){
                for(auto v:tmp_vec[i]){
                    global_affected_nodes.insert(v);
                }
            }
            cout<<i<<' '<<global_affected_nodes.size()<<' '<<all_affected_nodes.size()<<endl;
            if(global_affected_nodes.size()!=all_affected_nodes.size()){
                cout<<"not same size"<<endl;
            }else{
                for(auto v :all_affected_nodes){
                    if(global_affected_nodes.find(v) == global_affected_nodes.end()){
                        cout<<"not same"<<endl;
                        break;
                    }
                }
            }
            worker_barrier();

          }else{
             slaveGather(partial_affected_nodes);
             worker_barrier();
          }
         i++;
    }
  }

void test_bfs_connectivity(int fid){
  //before test ,note off bfs_connectivity.h 32
  Graph dgraph,fragmentgraph;
  FragmentLoader fragmentloader;
  GraphLoader dgraph_loader;
  dgraph_loader.LoadGraph(dgraph,graph_vfile,graph_efile);
  Fragment fragment(fragmentgraph,graph_vfile,graph_efile,r_file);
  int index = 0 ;
  while (index <10000){
      int root = random(0,9999);
      Bfs_Connectivity bfs_connectivity;
      std::unordered_set<VertexID> nodeset;
      bfs_connectivity.bfs_parallel(fragment,fragmentgraph,nodeset,root);
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
        std::unordered_set<VertexID> direct_result;
        dgraph.find_connectivity_nodes(root,direct_result);
        if (global_node.size() != direct_result.size()){
            std::cout<<"false "<<root<<' '<<global_node.size()<<' '<<direct_result.size()<<std::endl;
        }else{
            for(auto v :global_node){
                if(direct_result.find(v) == direct_result.end()){
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
private:
    std::string graph_vfile ="../data/synmtic.v";
    std::string graph_efile ="../data/synmtic.e";
    std::string r_file = "../data/synmtic.r";
    std::string base_add_file = "../data/incsynmtic/add_e";
    std::string base_remove_file="../data/incsynmtic/rm_e";
};

int main(int argc, char *argv[]) {
  google::SetUsageMessage("Usage: test [gflags_opt]");
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::ShutDownCommandLineFlags();
  google::InitGoogleLogging("test for working");
  google::ShutdownGoogleLogging();
  init_workers();
   int rank =get_worker_id();
  int num_workers =get_num_workers();
  Parallel parallel;
//  serial.test_dualsimulation();
//  parallel.test_dual_parallel(rank);
//  parallel.test_dual_filterparallel(rank);
//  parallel.test_bfs(rank);
//  parallel.test_bfs_khop(rank);
//  parallel.test_multikhop(rank);
//  parallel.test_fragment_loader(rank);
  parallel.test_bfs_connectivity(rank);
  worker_finalize();
  return 0;
}

using namespace std;