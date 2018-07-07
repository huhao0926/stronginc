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

  void generate_dual_query(int num_nodes){
      DualSim dualsim;

      Graph dgraph;
      GraphLoader dgraph_loader,qgraph_loader;
      FLAGS_vfile =  "../data/yago.v";
      FLAGS_efile = "../data/yago.e";
//      FLAGS_vfile =  "../data/synmtic.v";
//      FLAGS_efile = "../data/synmtic.e";
      dgraph_loader.LoadGraph(dgraph,FLAGS_vfile,FLAGS_efile);
      srand( (unsigned)time(0));
      int index = 1;
      while (index <10){
          FLAGS_vfile =  "../data/yagoquery/"+std::to_string(num_nodes)+"/q"+std::to_string(index)+".v";
          FLAGS_efile = "../data/yagoquery/"+std::to_string(num_nodes)+"/q"+std::to_string(index)+".e";
//          FLAGS_vfile =  "../data/query"+std::to_string(num_nodes)+"/q"+std::to_string(index)+".v";
//          FLAGS_efile = "../data/query"+std::to_string(num_nodes)+"/q"+std::to_string(index)+".e";
//          std::cout<<FLAGS_vfile<<std::endl;
          Graph qgraph;
          generate_connect_graphs_by_Dgraph(dgraph,qgraph,num_nodes);
          std::unordered_set<VertexID> center_node;
          if (cal_diameter_qgraph(qgraph)<=2 && query_labl_all_notsame(qgraph)){
              std::unordered_map<VertexID, std::unordered_set<VertexID>> sim;
              bool initialized_sim=false;
              clock_t start,end;
              start =clock();
              dualsim.dual_simulation(dgraph,qgraph,sim,initialized_sim);
              end = clock();
              center_node.clear();
              for (auto u : qgraph.GetAllVerticesID()){
                   for(auto v:sim[u]){
                       center_node.insert(v);
                   }
              }

              if(center_node.size()<=100){
                  std::cout<<index<<" "<<cal_diameter_qgraph(qgraph)<<" "<<query_labl_all_notsame(qgraph)<<" "<<center_node.size()<<std::endl;
                  save_grape_file(qgraph,FLAGS_vfile,FLAGS_efile);
                  index+=1;
              }
          }
   }
    }

  void generate_random_edge(){
  FLAGS_vfile =  "../data/yago.v";
  FLAGS_efile = "../data/yago.e";
  srand( (unsigned)time(0));
  std::string base_yago_inc_filename1 ="../data/incyago/add_e";
  std::string base_yago_inc_filename2 ="../data/incyago/rm_e";
  std::vector<std::pair<VertexID,VertexID>> synmtic_edges;
  LoadEdges(synmtic_edges,FLAGS_efile);
  std::set<std::pair<VertexID,VertexID>> exist_edges;
  int i=1;
  while(i<20){
    for(auto e:synmtic_edges){
      exist_edges.insert(e);
    }
    genertate_random_remove_edges(exist_edges,base_yago_inc_filename2,10000,500*i,i);
    genertate_random_add_edges(exist_edges,base_yago_inc_filename1,10000,500*i,i);
    exist_edges.clear();
    std::cout<<i<<std::endl;
    i+=1;
  }
  }

  void genertate_random_add_edges(std::set<std::pair<VertexID,VertexID>> &exist_edges,const std::string basefilename,int dgraph_num_vertices,int generate_edge_num,int ci){
    int j=0;
    std::set<std::pair<VertexID,VertexID>> add_edges;
    while(j<generate_edge_num){
         VertexID node1 = random(0,dgraph_num_vertices-1);
         VertexID node2 = random(0,dgraph_num_vertices-1);
         if (node1!=node2){
             //std::pair<VertexID,VertexID> p(node1,node2);
             if (exist_edges.find(std::make_pair(node1,node2)) == exist_edges.end()){
                 add_edges.insert(std::make_pair(node1,node2));
                 exist_edges.insert(std::make_pair(node1,node2));
                 j+=1;
             }
          }

    }
    save_edges(add_edges,basefilename+std::to_string(ci));
    add_edges.clear();
  }

  void genertate_random_remove_edges(std::set<std::pair<VertexID,VertexID>> &exist_edges,const std::string basefilename,int dgraph_num_vertices,int generate_edge_num,int ci){
      int j = 0;
      std::set<std::pair<VertexID,VertexID>> rm_edges;
      std::unordered_set<int> rm_index;
      std::vector<std::pair<VertexID,VertexID>> exist_edges_list;

      for(auto e :exist_edges){
          exist_edges_list.push_back(e);
      }
      while(j<generate_edge_num){
          int ind = random(0,exist_edges_list.size()-1);
          if(rm_index.find(ind) == rm_index.end()){
              rm_edges.insert(exist_edges_list[ind]);
              rm_index.insert(ind);
              exist_edges.erase(exist_edges_list[ind]);
              j+=1;
          }
      }
      save_edges(rm_edges,basefilename+std::to_string(ci));
      rm_edges.clear();
      rm_index.clear();
      exist_edges_list.clear();
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
    bool has_affectted_center_node=false;
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

  void generate_affected_center(){
  Graph qgraph,base_graph;
  GraphLoader dgraph_loader,qgraph_loader;
  srand( (unsigned)time(0));
  std::string basefilename1 ="../data/incyago/add_e";
  std::string basefilename2 ="../data/incyago/rm_e";
  int index = 1;
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
     if((e_affected_nodes.size()!=0)&& (e_affected_nodes.size()*1.0/max_dual_set.size()<=0.3)){
          add_edges.insert(e);
          save_edges_app(tmp_set,"../data/tmp_incadd.txt");
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

  void test_generate_center_outsidecenter_edges(){
  Graph qgraph,base_graph;
  GraphLoader dgraph_loader,qgraph_loader;
  srand( (unsigned)time(0));
  std::string basefilename1 ="../data/incyago/add_e";
  std::string basefilename2 ="../data/incyago/rm_e";
  int index = 1;
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
  while(i<20){
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

  void test_find_affecttedarea(){
 Graph qgraph,base_graph;
  GraphLoader dgraph_loader,qgraph_loader;
  srand( (unsigned)time(0));
  std::string basefilename1 ="../data/incyago/add_e";
  std::string basefilename2 ="../data/incyago/rm_e";
  int index = 1;
  FLAGS_vfile =  "../data/yagoquery5/q"+std::to_string(index)+".v";
  FLAGS_efile = "../data/yagoquery5/q"+std::to_string(index)+".e";
  qgraph_loader.LoadGraph(qgraph,FLAGS_vfile,FLAGS_efile);
  FLAGS_vfile =  "../data/yago.v";
  FLAGS_efile = "../data/yago.e";
  dgraph_loader.LoadGraph(base_graph,FLAGS_vfile,FLAGS_efile);
  std::unordered_set<VertexID> max_dual_set =get_dual_node_result(base_graph,qgraph);
  std::unordered_set<VertexID> affectted_center_node;
  int d_Q = cal_diameter_qgraph(qgraph);
  base_graph.find_hop_nodes(max_dual_set,d_Q,affectted_center_node);
  int dgraph_num_vertices  = base_graph.GetNumVertices();
  std::cout<<max_dual_set.size()<<' '<<affectted_center_node.size()<<' '<<dgraph_num_vertices<<' '<<d_Q<<std::endl;
  int i=1;
  while(i<10){
     std::set<std::pair<VertexID,VertexID>> add_edges,rm_edges;
     Load_bunch_edges(add_edges,basefilename1,i);
     Load_bunch_edges(rm_edges,basefilename2,i);
     std::unordered_set<VertexID> changenode;
     for(auto e:add_edges){
         changenode.insert(e.first);
         changenode.insert(e.second);
     }
     for(auto e:rm_edges){
          changenode.insert(e.first);
          changenode.insert(e.second);
     }
     std::unordered_set<VertexID> affected_nodes;
     base_graph.find_hop_nodes(changenode,d_Q,affected_nodes);
     std::cout<<i<<' '<<affected_nodes.size()<<' '<<diff(max_dual_set,affected_nodes).size()<<' '<<intersection(max_dual_set,affected_nodes).size()<<std::endl;
  i++;
  }
}

  void test_dual_incremental(){
  DualInc dualinc;
  DualSim dualsim;
  Graph qgraph;
  Graph dgraph;
  GraphLoader dgraph_loader,qgraph_loader;
  FLAGS_vfile =  "../data/yago.v";
  FLAGS_efile = "../data/yago.e";
  dgraph_loader.LoadGraph(dgraph,FLAGS_vfile,FLAGS_efile);
  std::string basefilename1 ="../data/incyago/add_e";
  std::string basefilename2 ="../data/incyago/rm_e";
//  FLAGS_vfile =  "../data/synmticquery/q1.v";
//  FLAGS_efile = "../data/synmticquery/q1.e";
//  qgraph_loader.LoadGraph(qgraph,FLAGS_vfile,FLAGS_efile);
//  FLAGS_vfile =  "../data/yago.vertex";
//  FLAGS_efile = "../data/yago.edge";

  int index =1;
  while (index <20){
      FLAGS_vfile =  "../data/yagoquery5/q"+std::to_string(index)+".v";
      FLAGS_efile = "../data/yagoquery5/q"+std::to_string(index)+".e";
      qgraph_loader.LoadGraph(qgraph,FLAGS_vfile,FLAGS_efile);
      std::unordered_map<VertexID, std::unordered_set<VertexID>> sim;
      bool initialized_sim=false;
      dualsim.dual_simulation(dgraph,qgraph,sim,initialized_sim);
      save_sim_result(qgraph,sim,"../data/yagoquery5/sim"+std::to_string(index));
      cout<<index<<endl;
      int j=1;
//      while (j<20){
//          std::set<std::pair<VertexID,VertexID>> add_edges,rm_edges;
//          LoadEdges(add_edges,basefilename1+std::to_string(j));
//          LoadEdges(rm_edges,basefilename2+std::to_string(j));
//          std::unordered_map<VertexID, std::unordered_set<VertexID>> incdsim,direct_sim;
//          for(auto u :qgraph.GetAllVerticesID()){
//              incdsim[u]=std::unordered_set<VertexID>();
//              for(auto v:sim[u]){
//                incdsim[u].insert(v);
//              }
//          }
//
//
//          for (auto e:add_edges){
//             dgraph.AddEdge(Edge(e.first,e.second,1));
//          }
//          direct_sim.clear();
//          dualsim.dual_simulation(dgraph,qgraph,direct_sim,initialized_sim);
//          dualinc.incremental_addedges(dgraph,qgraph,incdsim,add_edges);
//          bool rmcompare = dual_the_same(qgraph,direct_sim,incdsim);
//
//          for(auto e :rm_edges){
//              dgraph.RemoveEdge(Edge(e.first,e.second,1));
//          }
//          direct_sim.clear();
//          dualsim.dual_simulation(dgraph,qgraph,direct_sim,initialized_sim);
//          dualinc.incremental_removeedgs(dgraph,qgraph,incdsim,rm_edges);
//
//          //std::cout<<"add "<<dual_the_same(qgraph,direct_sim,incdsim)<<std::endl;
//          bool addcompare = dual_the_same(qgraph,direct_sim,incdsim);
//          for(auto e:rm_edges){
//              dgraph.AddEdge(Edge(e.first,e.second,1));
//          }
//          for (auto e:add_edges){
//             dgraph.RemoveEdge(Edge(e.first,e.second,1));
//          }
//          std::cout<<rmcompare<<' '<<addcompare<<std::endl;
//          j+=1;
//      }
      index +=1;
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

          for(auto e:rm_edges){
              dgraph.AddEdge(Edge(e.first,e.second,1));
          }
          for (auto e:add_edges){
             dgraph.RemoveEdge(Edge(e.first,e.second,1));
          }
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

 void test_strongsimulation_inc(){
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
//  FLAGS_vfile =  "../data/synmticquery/q1.v";
//  FLAGS_efile = "../data/synmticquery/q1.e";
//  qgraph_loader.LoadGraph(qgraph,FLAGS_vfile,FLAGS_efile);
//  FLAGS_vfile =  "../data/yago.vertex";
//  FLAGS_efile = "../data/yago.edge";

  int index =1;
  while (index <2){
      Graph qgraph;
      FLAGS_vfile =  "../data/yagoquery5/q"+std::to_string(index)+".v";
      FLAGS_efile = "../data/yagoquery5/q"+std::to_string(index)+".e";
      qgraph_loader.LoadGraph(qgraph,FLAGS_vfile,FLAGS_efile);
      std::unordered_map<VertexID, std::unordered_set<VertexID>> sim;
      clock_t s0,e0;
      s0 =clock();
      std::vector<StrongR> strongsimr = strongsim.strong_simulation_sim(dgraph,qgraph);
      e0 =clock();
      std::cout<<"calculate original strong"<<(float)(e0-s0)/CLOCKS_PER_SEC<<"s"<<std::endl;
//     std::vector<StrongR> strongsimr;
      bool initialized_sim=false;
      dualsim.dual_simulation(dgraph,qgraph,sim,initialized_sim);
      int j=1;
      std::set<std::pair<VertexID,VertexID>> add_edges,rm_edges,tmp_add_edges;
      LoadEdges(add_edges,"../data/tmp_incadd.txt");
      for(auto e:add_edges){
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
          tmp_add_edges.insert(e);
          clock_t start1,end1;
          start1 =clock();
          std::vector<StrongR> direct_strong = calculate_direct_strong_inc(dgraph,qgraph,tmp_add_edges,rm_edges);
          end1 = clock();
         std::cout<<"calculate direct strong"<<(float)(end1-start1)/CLOCKS_PER_SEC<<"s"<<std::endl;
          clock_t start2,end2;
          start2 =clock();
         stronginc.strong_simulation_inc(dgraph,qgraph,tmp_sim,tmp_r,tmp_add_edges,rm_edges);
         end2 = clock();
         std::cout<<"calculate inc strong"<<(float)(end2-start2)/CLOCKS_PER_SEC<<"s"<<std::endl;
      }
//      while (j<20){
//          std::set<std::pair<VertexID,VertexID>> add_edges,rm_edges;
//         Load_bunch_edges(add_edges,basefilename1,j);
//         Load_bunch_edges(rm_edges,basefilename2,j);
//
//          std::vector<StrongR> tmp_r;
//          std::unordered_map<VertexID, std::unordered_set<VertexID>> tmp_sim;
//          for(auto ball:strongsimr){
//              tmp_r.push_back(ball);
//          }
//          for(auto u :qgraph.GetAllVerticesID()){
//              tmp_sim[u]=std::unordered_set<VertexID>();
//              for(auto v:sim[u]){
//                tmp_sim[u].insert(v);
//              }
//          }
//          clock_t start1,end1;
//          start1 =clock();
//          std::vector<StrongR> direct_strong = calculate_direct_strong_inc(dgraph,qgraph,add_edges,rm_edges);
//          end1 = clock();
//         std::cout<<"calculate direct strong"<<(float)(end1-start1)/CLOCKS_PER_SEC<<"s"<<std::endl;
//          clock_t start2,end2;
//          start2 =clock();
//         stronginc.strong_simulation_inc(dgraph,qgraph,tmp_sim,tmp_r,add_edges,rm_edges);
//         end2 = clock();
//         std::cout<<"calculate inc strong"<<(float)(end2-start2)/CLOCKS_PER_SEC<<"s"<<std::endl;
//
//          j+=1;
//      }
//    cout<<index<<endl;
      index +=1;
   }
 }
};


int main(int argc, char *argv[]) {
  google::SetUsageMessage("Usage: test [gflags_opt]");
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::ShutDownCommandLineFlags();
  google::InitGoogleLogging("test for working");
  google::ShutdownGoogleLogging();

  Generate_Query geratequery;
//  geratequery.test_generate_center_outsidecenter_edges();
//  geratequery.test_find_affecttedarea();
//  geratequery.test_strongsimulation_inc();
  geratequery.test_strongsimulation_inc();
  return 0;
}

using namespace std;