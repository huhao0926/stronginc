#include <gflags/gflags.h>
#include <glog/logging.h>
#include<boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include "cpp/core/graphapi.h"
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
#include "cpp/parallel/strongparallel_incremental.h"
#include "cpp/serial/strongsimulation.h"
#include "cpp/parallel/dual_parallel_inc.h"
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

bool strong_is_the_same(Graph &qgraph, std::vector<StrongR> &direct_result,std::vector<StrongR> &parallel_result){
//    if(direct_result.size()!=parallel_result.size()){
//        cout<<"size not the same"<<endl;
//        return false;
//    }
    for(int i =0;i<direct_result.size();++i){
        for(int j=0;j<parallel_result.size();++j){
            if(direct_result[i].center() == parallel_result[j].center()){
                std::unordered_map<VertexID, std::unordered_set<VertexID>> dirctsim=direct_result[i].ballr();
                std::unordered_map<VertexID, std::unordered_set<VertexID>> incsim=parallel_result[j].ballr();
                if(!dual_the_same(qgraph,dirctsim,incsim)){
                    for(auto u :qgraph.GetAllVerticesID()){
                        cout<<u;
                        for(auto v:dirctsim[u]){
                            cout<<' '<<v;
                        }
                        cout<<endl;
                    }
                    for(auto u :qgraph.GetAllVerticesID()){
                        cout<<u;
                        for(auto v:incsim[u]){
                            cout<<' '<<v;
                        }
                        cout<<endl;
                    }
                    cout<<direct_result[i].center()<<' '<<parallel_result[j].center()<<endl;
                    cout<<"ball result not same"<<endl;
                    return false;
                }
            }
        }
    }
    for(int i =0;i<parallel_result.size();++i){
        for(int j=0;j<direct_result.size();++j){
            if(parallel_result[i].center() == direct_result[j].center()){
                std::unordered_map<VertexID, std::unordered_set<VertexID>> dirctsim=parallel_result[i].ballr();
                std::unordered_map<VertexID, std::unordered_set<VertexID>> incsim=direct_result[j].ballr();
                if(!dual_the_same(qgraph,dirctsim,incsim)){
//                    for(auto u :qgraph.GetAllVerticesID()){
//                        cout<<u;
//                        for(auto v:dirctsim[u]){
//                            cout<<' '<<v;
//                        }
//                        cout<<endl;
//                    }
//                    for(auto u :qgraph.GetAllVerticesID()){
//                        cout<<u;
//                        for(auto v:incsim[u]){
//                            cout<<' '<<v;
//                        }
//                        cout<<endl;
//                    }
//                    cout<<direct_result[i].center()<<' '<<parallel_result[j].center()<<endl;
                    cout<<"ball result not same"<<endl;
                    return false;
                }
            }
        }
    }
    return true;
}

void test_strong_parallel(int fid){
  Graph dgraph,fragmentgraph,qgraph;
  FragmentLoader fragmentloader;
  GraphLoader dgraph_loader,qgraph_loader;
  dgraph_loader.LoadGraph(dgraph,graph_vfile,graph_efile);
  Fragment fragment(fragmentgraph,graph_vfile,graph_efile,r_file);
  int index = 1;
  while(index <200){
      Graph qgraph;
      qgraph_loader.LoadGraph(qgraph,get_query_vfile(index),get_query_efile(index));
      StrongparallelInc strongparallelinc;
      std::vector<StrongR> partial_result = strongparallelinc.strong_parallel(fragment,fragmentgraph,qgraph);

      if (fid==0){
          std::vector<std::vector<StrongR>>  tmp_vec(get_num_workers());
          tmp_vec[fid] = partial_result;;
          masterGather(tmp_vec);
          std::vector<StrongR>  global_result;
          for(int i=0;i<get_num_workers();i++){
              for(auto ball:tmp_vec[i]){
                  global_result.push_back(ball);
              }
          }
          for(int i=0;i<global_result.size();++i){
              for(int j=i+1;j<global_result.size();++j){
                  if(global_result[j].center()<global_result[i].center()){
                      StrongR tmp_r=global_result[j];
                      global_result[j]=global_result[i];
                      global_result[i] = tmp_r;
                  }
              }
          }
          StrongSim strongsim;
          std::vector<StrongR> direct_result = strongsim.strong_simulation_sim(dgraph,qgraph);
          worker_barrier();
          std::cout<<"index: "<<index<<' '<<global_result.size()<<' '<<direct_result.size()<<' '<<strong_is_the_same(qgraph,direct_result,global_result)<<std::endl;
      }else{
         slaveGather(partial_result);
         worker_barrier();
      }
      index++;
  }
}

std::vector<StrongR> calculate_direct_strong_inc(Graph &dgraph,Graph &qgraph,
                                      std::set<std::pair<VertexID,VertexID>> &add_edges,
                                      std::set<std::pair<VertexID,VertexID>> &rm_edges){
          StrongSim strongsim;
          for (auto e:add_edges){
             dgraph.AddEdge(Edge(e.first,e.second,1));
          }
         for(auto e :rm_edges){
              dgraph.RemoveEdge(Edge(e.first,e.second,1));
          }
          std::vector<StrongR> result = strongsim.strong_simulation_sim(dgraph,qgraph);

          return result;
 }

void test_strong_parallel_inc(int fid){
  Graph dgraph,fragmentgraph;
 // FragmentLoader fragmentloader;
  GraphLoader dgraph_loader,qgraph_loader;
  dgraph_loader.LoadGraph(dgraph,graph_vfile,graph_efile);
 Fragment fragment(fragmentgraph,graph_vfile,graph_efile,r_file);
  int index = 1;
  while(index <20){
      Graph qgraph;
      qgraph_loader.LoadGraph(qgraph,get_query_vfile(index),get_query_efile(index));
//      Fragment fragment(fragmentgraph,graph_vfile,graph_efile,r_file);
      std::unordered_map<VertexID, std::unordered_set<VertexID>> partial_sim;
      Dual_Parallel dualparallel;
      dualparallel.dual_paraller(fragment,fragmentgraph,qgraph,partial_sim);
      StrongSim strongsim;
      std::vector<StrongR> partial_strong = strongsim.strong_simulation_sim(dgraph,qgraph);
      int j=1;
      while(j<10){
            Graph inc_dgraph;
            Graph inc_fragmentgraph;
            dgraph_loader.LoadGraph(inc_dgraph,graph_vfile,graph_efile);
            Fragment inc_fragment(inc_fragmentgraph,graph_vfile,graph_efile,r_file);
            std::set<std::pair<VertexID,VertexID>> add_edges,rm_edges;
            Load_bunch_edges(add_edges,base_add_file,j);
           // int d_Q=cal_diameter_qgraph(qgraph);
            //std::unordered_set<VertexID> aa=find_affected_area(inc_dgraph,add_edges,rm_edges,2*d_Q);
           // cout<<(aa.find(8762)!=aa.end())<<' '<<(aa.find(3585)!=aa.end())<<' '<<inc_dgraph.shortest_distance(3585,8762)<<endl;
//            Load_bunch_edges(rm_edges,base_remove_file,j);
            std::unordered_map<VertexID, std::unordered_set<VertexID>> inc_parallel_dual;
            std::vector<StrongR> inc_parallel_strong;
            for(auto u :qgraph.GetAllVerticesID()){
                inc_parallel_dual[u] = std::unordered_set<VertexID>();
                for(auto v : partial_sim[u]){
                    inc_parallel_dual[u].insert(v);
                }
            }
            for(auto ball:partial_strong){
                inc_parallel_strong.push_back(ball);
            }
            StrongparallelInc strongparallelinc;
            std::vector<StrongR> partial_result=strongparallelinc.strong_parallel_inc(inc_fragment,inc_fragmentgraph,qgraph,inc_parallel_dual,inc_parallel_strong,add_edges,rm_edges);
      if (fid==0){
          std::vector<std::vector<StrongR>>  tmp_vec(get_num_workers());
          tmp_vec[fid] = partial_result;;
          masterGather(tmp_vec);
          std::vector<StrongR>  global_result;
          for(int i=0;i<get_num_workers();i++){
              for(auto ball:tmp_vec[i]){
                  global_result.push_back(ball);
              }
          }
          std::unordered_set<VertexID> global_center,inc_center;
          std::vector<StrongR> direct_strong = calculate_direct_strong_inc(inc_dgraph,qgraph,add_edges,rm_edges);
          for(auto ball:global_result){
             global_center.insert(ball.center());
          }
          for(auto ball:direct_strong){
              inc_center.insert(ball.center());
          }          
          worker_barrier();
          std::cout<<index<<' '<<j<<' '<<global_center.size()<<' '<<inc_center.size()<<' '<<intersection(global_center,inc_center).size()<<"=="<<global_result.size()<<' '<<direct_strong.size()<<' '<<strong_is_the_same(qgraph,direct_strong,global_result)<<std::endl;
      }else{
         slaveGather(partial_result);
         worker_barrier();
      }
            j++;
      }
      index++;
  }

}


void test_dual_parallelinc(int fid){
  int index = 1;
  DualSim dualsim;
  while (index <20){
      Graph dgraph,fragmentgraph,qgraph;
      FragmentLoader fragmentloader;
        /* note off dualsimulation_parallel.cc  276
       */
      GraphLoader dgraph_loader,qgraph_loader;
      dgraph_loader.LoadGraph(dgraph,graph_vfile,graph_efile);
      Fragment fragment(fragmentgraph,graph_vfile,graph_efile,r_file);
      qgraph_loader.LoadGraph(qgraph,get_query_vfile(index),get_query_efile(index));
      std::unordered_map<VertexID, std::unordered_set<VertexID>> sim,psim;
      Dual_Parallel dualparallel;
      dualparallel.dual_paraller(fragment,fragmentgraph,qgraph,psim);
      int j=1;
       while (j<=10){
            Graph inc_graph,inc_fragmentgraph;
            dgraph_loader.LoadGraph(inc_graph,graph_vfile,graph_efile);
            Fragment fragment1(inc_fragmentgraph,graph_vfile,graph_efile,r_file);
            std::set<std::pair<VertexID,VertexID>> add_edges,rm_edges;
            Load_bunch_edges(add_edges,base_add_file,j);
//            Load_bunch_edges(rm_edges,base_remove_file,j);
            std::unordered_map<VertexID, std::unordered_set<VertexID>> inc_serial,inc_parallel;
            for(auto u :qgraph.GetAllVerticesID()){
                inc_parallel[u] = std::unordered_set<VertexID>();
                for(auto v : psim[u]){
                    inc_parallel[u].insert(v);
                }
            }
            for (auto e:add_edges){
               inc_graph.AddEdge(Edge(e.first,e.second,1));
            }

            Dual_parallelInc dual_parallel_inc;
            dual_parallel_inc.update_by_add_edges(fragment1,inc_fragmentgraph,add_edges,true);
            dual_parallel_inc.incremental_add_edges(fragment1, inc_fragmentgraph, qgraph,inc_parallel,add_edges);
            if (fid==0){
                std::vector<std::unordered_map<VertexID, std::unordered_set<VertexID>>>  tmp_vec(get_num_workers());
                tmp_vec[fid] = inc_parallel;
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
                sim.clear();
                dualsim.dual_simulation(inc_graph,qgraph,sim,initialized_sim);
                worker_barrier();
                std::cout<<index<<' '<<j<<' '<<dual_the_same(qgraph,sim,globalsim)<<std::endl;
            }else{
               slaveGather(inc_parallel);
               worker_barrier();
            }
            j+=1;
      }
  index += 1;
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
//  parallel.test_bfs_connectivity(rank);
//  parallel.test_strong_parallel(rank);
//  parallel.test_dual_parallelinc(rank);
  parallel.test_strong_parallel_inc(rank);
  worker_finalize();
  return 0;
}

using namespace std;