#include <gflags/gflags.h>
#include <glog/logging.h>
#include<boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include "cpp/graphapi.h"
#include "cpp/dualsimulation.h"
#include "cpp/dual_incremental.h"
#include "cpp/strongsimulation.h"
#include "cpp/strong_incremental.h"
#include "cpp/util.h"
#include "cpp/io/io_local.h"
#include "cpp/global.h"
#include "cpp/strongr.h"
#include "cpp/strongparallel_incremental.h"
#include "cpp/bfs_multikhop.h"
#include<iostream>
#include <fstream>
#include<ctime>
//#include <sstream>
#define random(a,b) (rand()%(b-a+1)+a)

class Generate_Query{
public:
template<class T>
    std::unordered_set<T> diff(const std::unordered_set<T> &a, const std::unordered_set<T>& b) {
		std::unordered_set<T> ret;
		for (auto ele : a) {
			if (b.find(ele) == b.end()) {
				ret.insert(ele);
			}
		}
		return ret;
	}

template<class T>
  std::unordered_set<T> intersection(const std::unordered_set<T> &a, const std::unordered_set<T>& b) {
		std::unordered_set<T> ret;
		for (auto ele : a) {
			if (b.find(ele) != b.end()) {
				ret.insert(ele);
			}
		}
		return ret;
	}

  void LoadEdges(std::vector<std::pair<VertexID,VertexID>> &edges, const std::string efile) {
    std::string line;
    std::ifstream e_infile(efile,std::ios::in);
    VertexID u,v;
    while(getline(e_infile,line,'\n')){
        std::stringstream ss(line);
        ss>> u >> v;
        edges.push_back(std::make_pair(u,v));
    }
}

  void Load_bunch_edges(std::vector<std::pair<VertexID,VertexID>> &edges,const std::string basefilename,int index){
    int i=1;
    while (i<=index){
         LoadEdges(edges,basefilename+std::to_string(i));
         i+=1;
     }
}

  void LoadEdges(std::set<std::pair<VertexID,VertexID>> &edges, const std::string efile) {
    std::string line;
    std::ifstream e_infile(efile,std::ios::in);
    VertexID u,v;
    while(getline(e_infile,line,'\n')){
        std::stringstream ss(line);
        ss>> u >> v;
        edges.insert(std::make_pair(u,v));
    }
}

  void Load_bunch_edges(std::set<std::pair<VertexID,VertexID>> &edges,const std::string basefilename,int index){
    int i=1;
    while (i<=index){
         LoadEdges(edges,basefilename+std::to_string(i));
         i+=1;
     }
}

  std::unordered_set<VertexID> get_dual_node_result(Graph &dgraph,Graph &qgraph){
    std::unordered_map<VertexID, std::unordered_set<VertexID>> sim;
    DualSim dualsim;
    bool initialized_sim=false;
    dualsim.dual_simulation(dgraph,qgraph,sim,initialized_sim);
    std::unordered_set<VertexID> dual_noderesult;
    for(auto u :qgraph.GetAllVerticesID()){
        for(auto v:sim[u]){
            dual_noderesult.insert(v);
        }
    }
    return dual_noderesult;
}

  bool add_is_valid(Graph &dgraph,Graph &qgraph,std::unordered_set<VertexID> &max_dual_set,std::set<std::pair<VertexID,VertexID>> &add_edges){
      for(auto e : add_edges){
          dgraph.AddEdge(Edge(e.first,e.second,1));
      }
    std::unordered_map<VertexID, std::unordered_set<VertexID>> sim;
    DualSim dualsim;
    bool initialized_sim=false;
    dualsim.dual_simulation(dgraph,qgraph,sim,initialized_sim);
    std::unordered_set<VertexID> dual_noderesult;
    for(auto u :qgraph.GetAllVerticesID()){
        for(auto v:sim[u]){
            dual_noderesult.insert(v);
        }
    }
    int n1=dual_noderesult.size();
    int n2=max_dual_set.size();

    for(auto e : add_edges){
        dgraph.RemoveEdge(Edge(e.first,e.second,1));
    }
//     return true;
   // std::cout<<n1<<' '<<n2<<std::endl;
    if(n1<n2){
        return (n2-n1)<=2;
    }else{
        return (n1-n2)<=2;
    }

  }

  bool rm_is_valiad(Graph &dgraph,Graph &qgraph,std::unordered_set<VertexID> &max_dual_set,std::set<std::pair<VertexID,VertexID>> &remove_edges){
      for(auto e : remove_edges){
          dgraph.RemoveEdge(Edge(e.first,e.second,1));
      }
    std::unordered_map<VertexID, std::unordered_set<VertexID>> sim;
    DualSim dualsim;
    bool initialized_sim=false;
    dualsim.dual_simulation(dgraph,qgraph,sim,initialized_sim);
    std::unordered_set<VertexID> dual_noderesult;
    for(auto u :qgraph.GetAllVerticesID()){
        for(auto v:sim[u]){
            dual_noderesult.insert(v);
        }
    }
    int n1=dual_noderesult.size();
    int n2=max_dual_set.size();
    for(auto e : remove_edges){
          dgraph.AddEdge(Edge(e.first,e.second,1));
    }
//    return true;
    if(n1<n2){
        return (n2-n1)<=2;
    }else{
        return (n1-n2)<=2;
    }
  }

 void load_graph(Graph &dgraph,std::string v_file,std::string e_file, std::string basefilename1,std::string basefilename2,std::set<std::pair<VertexID,VertexID>> &exist_edges,int j)
 {
    GraphLoader dgraph_loader;
    std::vector<Vertex> vertices;
    dgraph_loader.LoadVertices(vertices,v_file);
    std::set<std::pair<VertexID,VertexID>> base_edges,add_edges,rm_edges;
    LoadEdges(base_edges,e_file);
    Load_bunch_edges(add_edges,basefilename1,j);
    Load_bunch_edges(rm_edges,basefilename2,j);
    std::vector<Edge> graph_edges;
    for(auto e :base_edges){
        if(rm_edges.find(e) == rm_edges.end()){
           graph_edges.push_back(Edge(e.first,e.second,1));
           exist_edges.insert(e);
        }
    }
    for(auto e: add_edges){
        if(rm_edges.find(e) == rm_edges.end()){
           graph_edges.push_back(Edge(e.first,e.second,1));
           exist_edges.insert(e);
        }
    }
    dgraph_loader.LoadGraph(dgraph,vertices,graph_edges);
 }

  std::pair<VertexID,VertexID>  generate_one_add_edge_by_nodelist(std::vector<VertexID> &node_list,std::set<std::pair<VertexID,VertexID>> &exist_edges,std::set<std::pair<VertexID,VertexID>> &add_edges){
       while(true){
          VertexID n1 = random(0,node_list.size()-1);
          VertexID n2 = random(0,node_list.size()-1);
          if (n1!=n2){
              if (exist_edges.find(std::pair<VertexID,VertexID>(node_list[n1],node_list[n2])) == exist_edges.end() && add_edges.find(std::pair<VertexID,VertexID>(node_list[n1],node_list[n2])) == add_edges.end()){
                  return std::pair<VertexID,VertexID>(node_list[n1],node_list[n2]);
              }
          }
      }

  }

  void generate_random_add_edge(int dgraph_num_vertices, std::unordered_set<VertexID> &max_dual_set, std::unordered_set<VertexID> &affectted_center_node,std::set<std::pair<VertexID,VertexID>> &exist_edges, int n, std::set<std::pair<VertexID,VertexID>> &add_e_set){
    std::vector<VertexID> center_nodes_list,outside_center_nodes_list;
    for(int i=0;i<dgraph_num_vertices;i++){
          if(affectted_center_node.find(i) == affectted_center_node.end()){
             outside_center_nodes_list.push_back(i);
          }else if(true || max_dual_set.find(i)==max_dual_set.end()){
             center_nodes_list.push_back(i);
          }
    }
    //std::cout<<center_nodes_list.size()<<' '<<outside_center_nodes_list.size()<<std::endl;
    bool has_affectted_center_node=true;
    int count=0;
    while (count<n){
         if (!has_affectted_center_node){
             int strategy = random(0,dgraph_num_vertices-1);
             if (strategy>=0 && strategy<= max_dual_set.size()){
//                  std::cout<<"i am here"<<std::endl;
                  std::pair<VertexID,VertexID> e=generate_one_add_edge_by_nodelist(center_nodes_list,exist_edges,add_e_set);
                  add_e_set.insert(e);
                  count++;
                  has_affectted_center_node = true;
             }else{
                 std::pair<VertexID,VertexID> e=generate_one_add_edge_by_nodelist(outside_center_nodes_list,exist_edges,add_e_set);
                 add_e_set.insert(e);
                 count++;
             }
         }else{
                 std::pair<VertexID,VertexID> e=generate_one_add_edge_by_nodelist(outside_center_nodes_list,exist_edges,add_e_set);
                 add_e_set.insert(e);
                 count++;
         }
    }
 }

  std::pair<VertexID,VertexID>  generate_one_remove_edge_by_list(std::vector<std::pair<VertexID,VertexID>> &edge_list, std::set<std::pair<VertexID,VertexID>> &already_rm_edges){
      while (true){
          int n1 = random(0,edge_list.size()-1);
          if (already_rm_edges.find(edge_list[n1]) == already_rm_edges.end()){
              return edge_list[n1];
          }
      }
}

  void generate_random_remove_edge(int dgraph_num_vertices, std::unordered_set<VertexID> &max_dual_set, std::unordered_set<VertexID> &affectted_center_node,std::set<std::pair<VertexID,VertexID>> &exist_edges, int n, std::set<std::pair<VertexID,VertexID>> &remove_e_set){
      std::vector<std::pair<VertexID,VertexID>> center_edges,outside_center_edges;
      for(auto e :exist_edges){
           if(affectted_center_node.find(e.first)!=affectted_center_node.end() && affectted_center_node.find(e.second) != affectted_center_node.end()){
                if(max_dual_set.find(e.first) == max_dual_set.end() && max_dual_set.find(e.second)==max_dual_set.end()){
                    center_edges.push_back(e);
                }

           }else if(affectted_center_node.find(e.first)==affectted_center_node.end() && affectted_center_node.find(e.second) == affectted_center_node.end()){
               outside_center_edges.push_back(e);
           }
      }
      int count=0;
      bool has_affectted_center_node=true;
      while (count<n){
          if (!has_affectted_center_node){
              int strategy = random(0,dgraph_num_vertices-1);
               if (strategy>=0 && strategy<= max_dual_set.size()){
                   std::pair<VertexID,VertexID> e = generate_one_remove_edge_by_list(center_edges,remove_e_set);
                   remove_e_set.insert(e);
                   count++;
               }else{
                   std::pair<VertexID,VertexID> e = generate_one_remove_edge_by_list(outside_center_edges,remove_e_set);
                   remove_e_set.insert(e);
                   count++;
               }
          }else{
                std::pair<VertexID,VertexID> e = generate_one_remove_edge_by_list(outside_center_edges,remove_e_set);
                remove_e_set.insert(e);
                count++;
          }
      }
}

  std::unordered_set<VertexID> find_affected_area(Graph &dgraph,std::set<std::pair<VertexID,VertexID>> &add_edges,int d_Q){
     std::unordered_set<VertexID> affected_nodes,changenode;
     for(auto e:add_edges){
        changenode.insert(e.first);
        changenode.insert(e.second);
     }
     dgraph.find_hop_nodes(changenode,d_Q,affected_nodes);
     return affected_nodes;
  }

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

  void save_edges_app(std::set<std::pair<VertexID,VertexID>> &edges, const std::string efile){
  //std::cout<<efile<<std::endl;
   std::fstream outfile(efile,std::ios::app);
   if(!outfile)
	{
		std::cout<<"the outfile can  not construct";
		exit(0);
	}
	for(auto e:edges){
	 outfile<<e.first<<' '<<e.second<<std::endl;
	}
	outfile.close();
}

  void generate_affected_center(int query_index){
  Graph qgraph,base_graph;
  GraphLoader dgraph_loader,qgraph_loader;
  srand( (unsigned)time(0));
  std::string basefilename1 ="../data/incyago/add_e";
  std::string basefilename2 ="../data/incyago/rm_e";
  int index = query_index;
  FLAGS_vfile =  "../data/yagoquery5/q"+std::to_string(index)+".v";
  FLAGS_efile = "../data/yagoquery5/q"+std::to_string(index)+".e";
  qgraph_loader.LoadGraph(qgraph,FLAGS_vfile,FLAGS_efile);
  FLAGS_vfile =  "../data/yago.v";
  FLAGS_efile = "../data/yago.e";
  dgraph_loader.LoadGraph(base_graph,FLAGS_vfile,FLAGS_efile);
  std::unordered_set<VertexID> max_dual_set =get_dual_node_result(base_graph,qgraph);
  std::unordered_set<VertexID> affectted_center_node;
  int d_Q = cal_diameter_qgraph(qgraph);
//  std::cout<<d_Q<<std::endl;
  base_graph.find_hop_nodes(max_dual_set,d_Q,affectted_center_node);
  int dgraph_num_vertices  = base_graph.GetNumVertices();
  std::cout<<max_dual_set.size()<<' '<<affectted_center_node.size()<<' '<<dgraph_num_vertices<<' '<<intersection(affectted_center_node,max_dual_set).size()<<' '<<d_Q<<std::endl;


   std::vector<VertexID> center_nodes_list,outside_center_nodes_list;
    for(int i=0;i<dgraph_num_vertices;i++){
          if(affectted_center_node.find(i) == affectted_center_node.end()){
             outside_center_nodes_list.push_back(i);
          }else if(max_dual_set.find(i)==max_dual_set.end()){
             center_nodes_list.push_back(i);
          }
    }
    int i=1;
  std::set<pair<VertexID,VertexID>> add_edges;
   std::set<pair<VertexID,VertexID>> exist_edges;
   Graph dgraph;
  load_graph(dgraph,FLAGS_vfile,FLAGS_efile, basefilename1,basefilename2,exist_edges,0);
 std::unordered_set<VertexID> edge_affected_nodes;

 std::string center_file_name = "../data/incyago/tmp_incadd.txt";
 std::fstream outfile(center_file_name,std::ios::out);
 outfile.close();
  while(i<=200){
//     std::pair<VertexID,VertexID> e=generate_one_add_edge_by_nodelist(center_nodes_list,exist_edges,add_edges);
     VertexID node1 = random(0,dgraph_num_vertices-1);
     VertexID node2 = random(0,dgraph_num_vertices-1);
     std::pair<VertexID,VertexID> e(node1,node2);
     std::set<pair<VertexID,VertexID>> tmp_set;
     tmp_set.insert(e);
     std::unordered_set<VertexID> e_affected_nodes = find_affected_area(dgraph,tmp_set,d_Q);
     e_affected_nodes = intersection(e_affected_nodes,max_dual_set);
     e_affected_nodes = diff(e_affected_nodes,edge_affected_nodes);
//      )
     if((e_affected_nodes.size()!=0)&& (e_affected_nodes.size()*1.0/max_dual_set.size()<=0.05)){
          add_edges.insert(e);
          save_edges_app(tmp_set,center_file_name);
          edge_affected_nodes = find_affected_area(dgraph,add_edges,d_Q);
          edge_affected_nodes=intersection(edge_affected_nodes,max_dual_set);
          std::cout<<i<<' '<<e_affected_nodes.size()<<' '<<edge_affected_nodes.size()<<std::endl;
          i++;
      }
      if(edge_affected_nodes.size() ==max_dual_set.size()){
          break;
      }
  }
  }

  void generate_outside_center(int query_index){
  Graph qgraph,base_graph;
  GraphLoader dgraph_loader,qgraph_loader;
  srand( (unsigned)time(0));
  std::string basefilename1 ="../data/incyago/add_e";
  std::string basefilename2 ="../data/incyago/rm_e";
  int index = query_index;
  FLAGS_vfile =  "../data/yagoquery5/q"+std::to_string(index)+".v";
  FLAGS_efile = "../data/yagoquery5/q"+std::to_string(index)+".e";
  qgraph_loader.LoadGraph(qgraph,FLAGS_vfile,FLAGS_efile);
  FLAGS_vfile =  "../data/yago.v";
  FLAGS_efile = "../data/yago.e";
  dgraph_loader.LoadGraph(base_graph,FLAGS_vfile,FLAGS_efile);
  std::unordered_set<VertexID> max_dual_set =get_dual_node_result(base_graph,qgraph);
  std::unordered_set<VertexID> affectted_center_node;
  int d_Q = cal_diameter_qgraph(qgraph);
//  std::cout<<d_Q<<std::endl;
  base_graph.find_hop_nodes(max_dual_set,d_Q,affectted_center_node);
  int dgraph_num_vertices  = base_graph.GetNumVertices();
  std::cout<<max_dual_set.size()<<' '<<affectted_center_node.size()<<' '<<dgraph_num_vertices<<' '<<intersection(affectted_center_node,max_dual_set).size()<<' '<<d_Q<<std::endl;

  int i=1;
  int circle_num = 30;
  while(i<=circle_num){
     std::set<pair<VertexID,VertexID>> exist_edges;
     Graph dgraph;
     load_graph(dgraph,FLAGS_vfile,FLAGS_efile, basefilename1,basefilename2,exist_edges,i-1);
     std::set<std::pair<VertexID,VertexID>> add_e_set,remove_e_set;
     while (true){
         generate_random_add_edge(dgraph_num_vertices,max_dual_set,affectted_center_node,exist_edges,50000,add_e_set);
         if (true || add_is_valid(dgraph,qgraph,max_dual_set,add_e_set)){
             std::unordered_set<VertexID> affected_nodes = find_affected_area(base_graph,add_e_set,d_Q);
             affected_nodes = intersection(affected_nodes,max_dual_set);
//             std::cout<<affected_nodes.size()<<' '<<affected_nodes.size()*1.0/max_dual_set.size()<<std::endl;
             if(affected_nodes.size()*1.0/max_dual_set.size()<=0.1){
                save_edges(add_e_set,basefilename1+std::to_string(i));
                 for(auto e :add_e_set){
                     exist_edges.insert(e);
                 }
                 break;
             }
         }else{
             add_e_set.clear();
         }
     }
      for(auto e : add_e_set){
          dgraph.AddEdge(Edge(e.first,e.second,1));
      }
     while(true){
        generate_random_remove_edge(dgraph_num_vertices,max_dual_set,affectted_center_node,exist_edges,500,remove_e_set);
         if(true || rm_is_valiad(dgraph,qgraph,max_dual_set,remove_e_set)){
             save_edges(remove_e_set,basefilename2+std::to_string(i));
             break;
         }else{
             remove_e_set.clear();
         }
     }
     add_e_set.clear();
     remove_e_set.clear();
     std::cout<<i<<std::endl;
  i++;
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

//          for(auto e:rm_edges){
//              dgraph.AddEdge(Edge(e.first,e.second,1));
//          }
//          for (auto e:add_edges){
//             dgraph.RemoveEdge(Edge(e.first,e.second,1));
//          }
          return result;
 }

 bool strongr_issame(Graph &qgraph,std::vector<StrongR> &directr,std::vector<StrongR> &incr){
     if(directr.size() != incr.size()){
         std::cout<<"size not same"<<directr.size()<<' '<<incr.size()<<std::endl;

         return false;
     }
     for(int i=0; i < directr.size() ; i++){
         std::unordered_map<VertexID, std::unordered_set<VertexID>> dirctsim=directr[i].ballr();
         std::unordered_map<VertexID, std::unordered_set<VertexID>> incsim=incr[i].ballr();
         if (!dual_the_same(qgraph,dirctsim,incsim)){
             return false;
         }
     }
     return true;

 }

 void test_strongsimulation_inc(int query_index){
  StrongInc stronginc;
  StrongSim strongsim;
  DualSim dualsim;

  Graph dgraph;
  GraphLoader dgraph_loader,qgraph_loader;
  FLAGS_vfile =  "../data/yago.v";
  FLAGS_efile = "../data/yago.e";
  dgraph_loader.LoadGraph(dgraph,FLAGS_vfile,FLAGS_efile);
  std::string basefilename1 ="../data/incyago/add_e";
  std::string basefilename2 ="../data/incyago/rm_e";
  std::string center_file_name = "../data/incyago/tmp_incadd.txt";
  int index =query_index;
  while (index <query_index+1){
      Graph qgraph;
      std::string q_v =  "../data/yagoquery5/q"+std::to_string(index)+".v";
      std::string q_e = "../data/yagoquery5/q"+std::to_string(index)+".e";
      qgraph_loader.LoadGraph(qgraph,q_v,q_e);
      std::unordered_map<VertexID, std::unordered_set<VertexID>> sim;
      clock_t s0,e0;
      s0 =clock();
//      std::vector<StrongR> strongsimr = strongsim.strong_simulation_sim(dgraph,qgraph);
      std::vector<StrongR> strongsimr;
      e0 =clock();
      std::cout<<"calculate original strong"<<(float)(e0-s0)/CLOCKS_PER_SEC<<"s"<<std::endl;
      bool initialized_sim=false;
      dualsim.dual_simulation(dgraph,qgraph,sim,initialized_sim);
      std::unordered_set<VertexID> max_dual_set =get_dual_node_result(dgraph,qgraph);
      std::cout<<"maxduao "<<max_dual_set.size()<<std::endl;
      int j=1;
      std::vector<std::pair<VertexID,VertexID>> affected_center_edges;
      LoadEdges(affected_center_edges,center_file_name);
      std::fstream outfile("runtime.txt",std::ios::out);
      outfile.close();
      int circle_num = 30;
      while (j<=circle_num){
          GraphLoader dgraph_loaddir,dgraph_loadinc;
          Graph dgraphdir,dgraphinc;
          dgraph_loaddir.LoadGraph(dgraphdir,FLAGS_vfile,FLAGS_efile);
          dgraph_loadinc.LoadGraph(dgraphinc,FLAGS_vfile,FLAGS_efile);
          std::set<std::pair<VertexID,VertexID>> add_edges,rm_edges;
         Load_bunch_edges(add_edges,basefilename1,j);
         Load_bunch_edges(rm_edges,basefilename2,j);
          for(int i=0;i<affected_center_edges.size();i++){
              if(i<(affected_center_edges.size()/circle_num+1)*j){
                  add_edges.insert(affected_center_edges[i]);
              }
          }
          std::vector<StrongR> tmp_r;
          std::unordered_map<VertexID, std::unordered_set<VertexID>> tmp_sim;
          for(auto ball:strongsimr){
              tmp_r.push_back(ball);
          }
          for(auto u :qgraph.GetAllVerticesID()){
              tmp_sim[u]=std::unordered_set<VertexID>();
              for(auto v:sim[u]){
                tmp_sim[u].insert(v);
              }
          }
          clock_t start1,end1;
          start1 =clock();
          std::vector<StrongR> direct_strong = calculate_direct_strong_inc(dgraphdir,qgraph,add_edges,rm_edges);
          end1 = clock();
        // std::cout<<"calculate direct strong"<<(float)(end1-start1)/CLOCKS_PER_SEC<<"s"<<std::endl;
          clock_t start2,end2;
          start2 =clock();
         stronginc.strong_simulation_inc(dgraphinc,qgraph,tmp_sim,tmp_r,add_edges,rm_edges);
         end2 = clock();
         //std::cout<<"calculate inc strong"<<(float)(end2-start2)/CLOCKS_PER_SEC<<"s"<<std::endl;
         cout<<(float)(end1-start1)/CLOCKS_PER_SEC<<' '<<(float)(end2-start2)/CLOCKS_PER_SEC<<endl;
         std::fstream outfile("runtime.txt",std::ios::app);
         outfile<<j<<' '<<(float)(end1-start1)/CLOCKS_PER_SEC<<' '<<(float)(end2-start2)/CLOCKS_PER_SEC<<endl;
         outfile.close();
          j+=1;
      }
    cout<<index<<endl;
      index +=1;
   }
 }


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

 void test_multibfs(){
  Graph dgraph,fragmentgraph,qgraph;
  FragmentLoader fragmentloader;
  std::string v_file  ="../data/synmtic.v";
  std::string e_file = "../data/synmtic.e";
  std::string r_file = "../data/synmtic.r";

  GraphLoader dgraph_loader,qgraph_loader;
  dgraph_loader.LoadGraph(dgraph,v_file,e_file);

  Fragment fragment(fragmentgraph,v_file,e_file,r_file);
     std::string base_add_file="../data/incsynmtic/add_e";
     std::string base_remove_file="../data/incsynmtic/rm_e";
//  int index = 1;
    int i =1;
    while(i<20){
             std::set<std::pair<VertexID,VertexID>> add_edges;
         std::set<std::pair<VertexID,VertexID>> rm_edges;
          Load_bunch_edges(add_edges,base_add_file,i);
         Load_bunch_edges(rm_edges,base_remove_file,i);
         std::unordered_set<VertexID> all_affected_nodes;
//         std::unordered_set<VertexID> all_affected_nodes = find_affected_area(dgraph,add_edges,rm_edges,2);
         std::unordered_set<VertexID> partial_affected_nodes = find_affected_area(fragment,fragmentgraph,add_edges,rm_edges,2);
     int fid = get_worker_id();
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


//         std::unordered_set<VertexID> affected_nodes = find_affected_area(fragment,dgraph,add_edges,rm_edges,d_Q);
//  while (index <2){
//      std::string qv_file =  "../data/synmticquery/q"+std::to_string(index)+".v";
//      std::string qe_file = "../data/synmticquery/q"+std::to_string(index)+".e";
//      qgraph_loader.LoadGraph(qgraph,qv_file,qe_file);
//     int d_Q = cal_diameter_qgraph(qgraph);
//     cout<<d_Q<<endl;
//     StrongparallelInc strongparallelinc;
//     strongparallelinc.update_fragment(fragment,fragmentgraph,d_Q);
//
//  index += 1;
//  }
 }

 void test_parallel_strong(){
  Graph dgraph,fragmentgraph,qgraph;
  FragmentLoader fragmentloader;
  std::string v_file  ="../data/synmtic.v";
  std::string e_file = "../data/synmtic.e";
  std::string r_file = "../data/synmtic.r";

  GraphLoader dgraph_loader,qgraph_loader;
  dgraph_loader.LoadGraph(dgraph,v_file,e_file);

  Fragment fragment(fragmentgraph,v_file,e_file,r_file);
 int index = 1;
   while (index <2){
      std::string qv_file =  "../data/synmticquery/q"+std::to_string(index)+".v";
      std::string qe_file = "../data/synmticquery/q"+std::to_string(index)+".e";
      qgraph_loader.LoadGraph(qgraph,qv_file,qe_file);
     int d_Q = cal_diameter_qgraph(qgraph);
     StrongparallelInc strongparallelinc;
     strongparallelinc.update_fragment_inc(fragment,fragmentgraph,d_Q);
  index += 1;
  }
 }
};


int main(int argc, char *argv[]) {
  google::SetUsageMessage("Usage: test [gflags_opt]");
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::ShutDownCommandLineFlags();
  google::InitGoogleLogging("test for working");
  google::ShutdownGoogleLogging();
  init_workers();
  int query_index = 1;
  Generate_Query geratequery;
//  geratequery.test_generate_center_outsidecenter_edges();
//  geratequery.generate_affected_center(query_index);
//  geratequery.generate_outside_center(query_index);
  geratequery.test_parallel_strong();

  worker_finalize();
  return 0;
}

using namespace std;