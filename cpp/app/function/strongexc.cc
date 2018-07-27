#include <gflags/gflags.h>
#include <glog/logging.h>
#include<boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include "cpp/core/graphapi.h"
#include "cpp/serial/dualsimulation.h"
#include "cpp/serial/dual_incremental.h"
#include "cpp/serial/strongsimulation.h"
#include "cpp/serial/strong_incremental.h"
#include "cpp/utils/util.h"
#include "cpp/io/io_local.h"
#include "cpp/core/global.h"
#include "cpp/core/strongr.h"
#include "cpp/utils/generate.h"
#include "cpp/utils/util.h"
#include<iostream>
#include <fstream>
#include<ctime>
//#include <sstream>
#define random(a,b) (rand()%(b-a+1)+a)

class StrongExr{
public:

    StrongExr(){}

    StrongExr(std::string test_data_name,int query_index){
        this->test_data_name=test_data_name;
        this->graph_vfile ="../data/"+test_data_name+"/"+test_data_name+".v";
        this->graph_efile ="../data/"+test_data_name+"/"+test_data_name+".e";
        this->r_file = "../data/"+test_data_name+"/"+test_data_name+".r";
        this->base_qfile = "../data/"+test_data_name+"/query5/q";
        this->base_add_file = "../data/"+test_data_name+"/inc/add_e";
        this->base_remove_file="../data/"+test_data_name+"/inc/rm_e";
        this->query_index = query_index;
    }
    std::string get_query_vfile(int index){
        return base_qfile+std::to_string(index)+".v";
    }

    std::string get_query_efile(int index){
        return base_qfile+std::to_string(index)+".e";
    }
public:
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

  void generate_query(int generate_query_nums,int generate_query_nodes, int max_calculate_center_nodes){
      Graph dgraph;
      Generate generate;
      GraphLoader dgraph_loader;
      dgraph_loader.LoadGraph(dgraph,graph_vfile,graph_efile);
      std::cout<<dgraph.GetNumVertices()<<' '<<dgraph.GetNumEdges()<<std::endl;
      int i=1;
      while(i<=generate_query_nums){
          Graph qgraph;
          generate.generate_connect_graphs_by_Dgraph(dgraph,qgraph,generate_query_nodes);
          int d_Q=cal_diameter_qgraph(qgraph);
          if(d_Q>2){
              continue;
          }
          clock_t s0,e0;
          s0 =clock();
          std::unordered_set<VertexID> max_dual_set = generate.get_dual_node_result(dgraph,qgraph);
          e0 =clock();
          if(max_dual_set.size()<=max_calculate_center_nodes){
              generate.save_grape_file(qgraph,get_query_vfile(i),get_query_efile(i));
              std::cout<<i<<' '<<"calculate dual time"<<(float)(e0-s0)/CLOCKS_PER_SEC<<"s"<<' '<<max_dual_set.size()<<std::endl;
              i++;
          }
      }
  }

  void print_dual_and_strong_information(){
      Graph dgraph;
      Generate generate;
      GraphLoader dgraph_loader,query_loader;
      dgraph_loader.LoadGraph(dgraph,graph_vfile,graph_efile);
      std::cout<<dgraph.GetNumVertices()<<' '<<dgraph.GetNumEdges()<<std::endl;
      std::fstream info_file("../data/dbpedia/query5/query_info.txt",std::ios::out);
      if(!info_file)
	   {
	    	std::cout<<"the outfile can  not construct";
	    	exit(0);
	   }
      int i=1;
      while(i<=200){
          Graph qgraph;
          query_loader.LoadGraph(qgraph,get_query_vfile(i),get_query_efile(i));
          int d_Q = cal_diameter_qgraph(qgraph);
          clock_t s0,e0,s1,e1;
          s0 =clock();
          std::unordered_set<VertexID> max_dual_set = generate.get_dual_node_result(dgraph,qgraph);
          e0 =clock();
          StrongSim strongsim;
          s1=clock();
          std::vector<StrongR> result = strongsim.strong_simulation_sim(dgraph,qgraph);
          e1=clock();
          std::cout<<i<<' '<<max_dual_set.size()<<' '<<d_Q<<' '<<(float)(e0-s0)/CLOCKS_PER_SEC<<' '<<(float)(e1-s1)/CLOCKS_PER_SEC<<std::endl;
          info_file<<i<<' '<<max_dual_set.size()<<' '<<d_Q<<' '<<(float)(e0-s0)/CLOCKS_PER_SEC<<' '<<(float)(e1-s1)/CLOCKS_PER_SEC<<std::endl;
          i++;
      }
         info_file.close();
  }


  void print_evaluate_incremental_information(int circle_num){
        Graph dgraph;
        GraphLoader dgraph_loader,qgraph_loader;
        dgraph_loader.LoadGraph(dgraph,graph_vfile,graph_efile);
        Graph qgraph;
        qgraph_loader.LoadGraph(qgraph,get_query_vfile(query_index),get_query_efile(query_index));
        int d_Q = cal_diameter_qgraph(qgraph);
        std::unordered_map<VertexID, std::unordered_set<VertexID>> sim;
        DualInc dualinc;
        DualSim dualsim;
        bool initialized_sim=false;
        dualsim.dual_simulation(dgraph,qgraph,sim,initialized_sim);
        std::unordered_set<int> max_dual_set;
        for(auto u :qgraph.GetAllVerticesID()){
            for(auto v:sim[u]){
               max_dual_set.insert(v);
            }
        }
        std::cout<<"original need calculate center node "<<max_dual_set.size()<<endl;
        int j=1;
        while(j<circle_num){
            std::set<std::pair<VertexID,VertexID>> add_edges,rm_edges;
            std::unordered_map<VertexID, std::unordered_set<VertexID>> incdsim;
            for(auto u :qgraph.GetAllVerticesID()){
                incdsim[u]=std::unordered_set<VertexID>();
                for(auto v:sim[u]){
                incdsim[u].insert(v);
                }
            }
            Load_bunch_edges(add_edges,base_add_file,j);
            Load_bunch_edges(rm_edges,base_remove_file,j);
            std::set<pair<VertexID,VertexID>> tmp_set;
            for(auto e:add_edges){
                tmp_set.insert(e);
            }
            for(auto e:rm_edges){
               tmp_set.insert(e);
            }
            std::unordered_set<VertexID> e_affected_nodes = find_affected_area(dgraph,tmp_set,d_Q);
            e_affected_nodes = intersection(e_affected_nodes,max_dual_set);
          //  LoadEdges(edges,base_add_file+std::to_string(j));
        //    LoadEdges(edges,base_remove_file+std::to_string(j));
            for (auto e:add_edges){
               dgraph.AddEdge(Edge(e.first,e.second,1));
            }
            dualinc.incremental_addedges(dgraph,qgraph,incdsim,add_edges);
            for(auto e :rm_edges){
                dgraph.RemoveEdge(Edge(e.first,e.second,1));
            }
            dualinc.incremental_removeedgs(dgraph,qgraph,incdsim,rm_edges);
            std::unordered_set<VertexID> inc_max_dual_set;
            for(auto u :qgraph.GetAllVerticesID()){
                for(auto v:incdsim[u]){
                   inc_max_dual_set.insert(v);
                }
            }
            for(auto e :rm_edges){
                dgraph.AddEdge(Edge(e.first,e.second,1));
            }
            for(auto e:add_edges){
                dgraph.RemoveEdge(Edge(e.first,e.second,1));
            }
            std::cout<<j<<' '<<"after incremental edgs all need calculate center nodes: "<<inc_max_dual_set.size()<<' '<<"      incremental edges affected center node nums: "<<e_affected_nodes.size()<<endl;
            j+=1;
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

  void generate_affected_center(int edge_num,double afa,std::string affected_center_file){
     Generate generate;
     Graph dgraph,qgraph;
     GraphLoader dgraph_loader,qgraph_loader;
     dgraph_loader.LoadGraph(dgraph,graph_vfile,graph_efile);
     qgraph_loader.LoadGraph(qgraph,get_query_vfile(query_index),get_query_efile(query_index));
     std::unordered_set<VertexID> max_dual_set =generate.get_dual_node_result(dgraph,qgraph);
     std::unordered_set<VertexID> affectted_center_node;
     int d_Q = cal_diameter_qgraph(qgraph);
     dgraph.find_hop_nodes(max_dual_set,d_Q,affectted_center_node);
     int dgraph_num_vertices  = dgraph.GetNumVertices();
     std::cout<<"generate_affected_center "<<max_dual_set.size()<<' '<<affectted_center_node.size()<<' '<<dgraph_num_vertices<<' '<<d_Q<<std::endl;
     std::vector<VertexID> center_nodes_list,outside_center_nodes_list;
     for(int i=0;i<dgraph_num_vertices;i++){
          if(affectted_center_node.find(i) == affectted_center_node.end()){
             outside_center_nodes_list.push_back(i);
          }else if(max_dual_set.find(i)==max_dual_set.end()){
             center_nodes_list.push_back(i);
          }
     }
     std::fstream outfile(affected_center_file,std::ios::out);
     outfile.close();
     std::unordered_set<VertexID> edge_affected_nodes;
     int i =0;
     std::set<pair<VertexID,VertexID>> exist_edges,add_edges;
     while(i<edge_num){
         std::pair<VertexID,VertexID> e=generate.generate_one_add_edge_by_nodelist(center_nodes_list,exist_edges,add_edges);
         std::set<pair<VertexID,VertexID>> tmp_set;
         tmp_set.insert(e);
         std::unordered_set<VertexID> e_affected_nodes = find_affected_area(dgraph,tmp_set,d_Q);
         e_affected_nodes = intersection(e_affected_nodes,max_dual_set);
         e_affected_nodes = diff(e_affected_nodes,edge_affected_nodes);
         if((e_affected_nodes.size()!=0)&& (e_affected_nodes.size()*1.0/max_dual_set.size()<=afa)){
          add_edges.insert(e);
          save_edges_app(tmp_set,affected_center_file);
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

  std::set<std::pair<VertexID,VertexID>> generate_random_remove_edge(std::unordered_set<VertexID> &max_dual_set, std::unordered_set<VertexID> &affectted_center_node,std::set<std::pair<VertexID,VertexID>> &exist_edges, int num_edges){
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
      Generate generate;
      std::set<std::pair<VertexID,VertexID>> already_remove_edges;
      return generate.generate_bunch_remove_edge_by_list(outside_center_edges,already_remove_edges,num_edges);
  }

  void generate_outside_center(int edge_num,int circle_num){
     Generate generate;
     Graph dgraph,qgraph;
     GraphLoader dgraph_loader,qgraph_loader;
     dgraph_loader.LoadGraph(dgraph,graph_vfile,graph_efile);
     qgraph_loader.LoadGraph(qgraph,get_query_vfile(query_index),get_query_efile(query_index));
     std::unordered_set<VertexID> max_dual_set =generate.get_dual_node_result(dgraph,qgraph);
     std::unordered_set<VertexID> affectted_center_node;
     int d_Q = cal_diameter_qgraph(qgraph);
     dgraph.find_hop_nodes(max_dual_set,d_Q,affectted_center_node);
     int dgraph_num_vertices  = dgraph.GetNumVertices();
     std::cout<<"generate_outside_center "<<max_dual_set.size()<<' '<<affectted_center_node.size()<<' '<<dgraph_num_vertices<<' '<<d_Q<<std::endl;
     std::vector<VertexID> center_nodes_list,outside_center_nodes_list;
     for(int i=0;i<dgraph_num_vertices;i++){
          if(affectted_center_node.find(i) == affectted_center_node.end()){
             outside_center_nodes_list.push_back(i);
          }else if(max_dual_set.find(i)==max_dual_set.end()){
             center_nodes_list.push_back(i);
          }
     }

//  std::vector<std::pair<VertexID,VertexID>> out_center_edges;
//  std::set<pair<VertexID,VertexID>> exist_edges;

  int i=1;
  while(i<circle_num){
     std::set<pair<VertexID,VertexID>> exist_edges;
     Graph dgraph;
     load_graph(dgraph,graph_vfile,graph_efile, base_add_file,base_remove_file,exist_edges,i-1);
     std::set<std::pair<VertexID,VertexID>> add_e_set,remove_e_set;
     add_e_set = generate.generate_bunch_add_edges_by_nodelist(outside_center_nodes_list,exist_edges,edge_num);
     for(auto e :add_e_set){
         exist_edges.insert(e);
     }
     save_edges(add_e_set,base_add_file+std::to_string(i));
     remove_e_set = generate_random_remove_edge(max_dual_set,affectted_center_node,exist_edges,edge_num);
    // std::cout<<base_remove_file+std::to_string(i)<<' '<<remove_e_set.size()<<endl;
     save_edges(remove_e_set,base_remove_file+std::to_string(i));
     std::cout<<i<<' '<<add_e_set.size()<<' '<<remove_e_set.size()<<std::endl;
     add_e_set.clear();
     remove_e_set.clear();
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

 void test_strongsimulation_inc(std::string affecte_center_file,int circle_num){
  StrongInc stronginc;
  StrongSim strongsim;
  DualSim dualsim;

  Graph dgraph;
  GraphLoader dgraph_loader,qgraph_loader;
  dgraph_loader.LoadGraph(dgraph,graph_vfile,graph_efile);
  int index =query_index;
  Generate generate;
      Graph qgraph;
      qgraph_loader.LoadGraph(qgraph,get_query_vfile(index),get_query_efile(index));
      std::unordered_map<VertexID, std::unordered_set<VertexID>> sim;
      clock_t s0,e0;
      s0 =clock();
//      std::vector<StrongR> strongsimr = strongsim.strong_simulation_sim(dgraph,qgraph);
      std::vector<StrongR> strongsimr;
      e0 =clock();
      std::cout<<"calculate original strong"<<(float)(e0-s0)/CLOCKS_PER_SEC<<"s"<<std::endl;
      bool initialized_sim=false;
      dualsim.dual_simulation(dgraph,qgraph,sim,initialized_sim);
      std::unordered_set<VertexID> max_dual_set =generate.get_dual_node_result(dgraph,qgraph);
      std::cout<<"maxduao "<<max_dual_set.size()<<std::endl;
      int j=1;
      std::vector<std::pair<VertexID,VertexID>> affected_center_edges;
      LoadEdges(affected_center_edges,affecte_center_file);
      std::fstream outfile("runtime.txt",std::ios::out);
      outfile.close();
      while (j<=circle_num){
          GraphLoader dgraph_loaddir,dgraph_loadinc;
          Graph dgraphdir,dgraphinc;
          dgraph_loaddir.LoadGraph(dgraphdir,graph_vfile,graph_efile);
          dgraph_loadinc.LoadGraph(dgraphinc,graph_vfile,graph_efile);
          std::set<std::pair<VertexID,VertexID>> add_edges,rm_edges;
         Load_bunch_edges(add_edges,base_add_file,j);
         Load_bunch_edges(rm_edges,base_remove_file,j);
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
 }

void generate_all_random_edges(int num_edges,int circle_num){
    Graph dgraph;
    GraphLoader dgraph_loader;
    dgraph_loader.LoadGraph(dgraph,graph_vfile,graph_efile);
    int dgraph_num_vertices  = dgraph.GetNumVertices();
    std::set<pair<VertexID,VertexID>> exist_edges;
    LoadEdges(exist_edges,graph_efile);
    cout<<"Base Graph vertices: "<<dgraph_num_vertices<<"  Base Graph edges: "<<dgraph.GetNumEdges()<<endl;
    int i=1;
    while(i<circle_num){
       // Graph dgraph;
       // load_graph(dgraph,graph_vfile,graph_efile, base_add_file,base_remove_file,exist_edges,i-1);
        std::set<std::pair<VertexID,VertexID>> add_e_set,remove_e_set;
        add_e_set = generate_all_random_add_edges(exist_edges,dgraph_num_vertices,num_edges);
        for(auto e :add_e_set){
            exist_edges.insert(e);
        }
        save_edges(add_e_set,base_add_file+std::to_string(i));
        remove_e_set = generate_all_random_remove_edges(exist_edges,dgraph_num_vertices,num_edges);
        for(auto e:remove_e_set){
            exist_edges.erase(e);
        }
        save_edges(remove_e_set,base_remove_file+std::to_string(i));
        std::cout<<i<<' '<<add_e_set.size()<<' '<<remove_e_set.size()<<std::endl;
        add_e_set.clear();
        remove_e_set.clear();
        i++;
    }
}

std::set<std::pair<VertexID,VertexID>> generate_all_random_add_edges(std::set<std::pair<VertexID,VertexID>> &exist_edges,int dgraph_num_vertices,int num_edges){
   srand( (unsigned)time(0));
    int j=0;
    std::set<std::pair<VertexID,VertexID>> add_edges;
    while(j<num_edges){
         VertexID node1 = random(0,dgraph_num_vertices-1);
         VertexID node2 = random(0,dgraph_num_vertices-1);
         if (node1!=node2){
             //std::pair<VertexID,VertexID> p(node1,node2);
             if (exist_edges.find(std::make_pair(node1,node2)) == exist_edges.end() && add_edges.find(std::make_pair(node1,node2)) == add_edges.end()){
                 add_edges.insert(std::make_pair(node1,node2));
                 j+=1;
             }
          }
    }
   return add_edges;
}


std::set<std::pair<VertexID,VertexID>> generate_all_random_remove_edges(std::set<std::pair<VertexID,VertexID>> &exist_edges,int dgraph_num_vertices,int num_edges){
    srand( (unsigned)time(0));
    std::vector<std::pair<VertexID,VertexID>> edge_list;
    for(auto e :exist_edges){
        edge_list.push_back(e);
    }
    std::set<std::pair<VertexID,VertexID>> remove_e_set;
    std::unordered_set<int> record;
    int j=0;
    while(j<num_edges){
          int n1 = random(0,edge_list.size()-1);
          if (record.find(n1) == record.end()){
              remove_e_set.insert(edge_list[n1]);
              record.insert(n1);
              j++;
          }
    }
    return remove_e_set;
}

void print_affected_center_info(int circle_num){
    Generate generate;
     Graph dgraph,qgraph;
     GraphLoader dgraph_loader,qgraph_loader;
     dgraph_loader.LoadGraph(dgraph,graph_vfile,graph_efile);
     qgraph_loader.LoadGraph(qgraph,get_query_vfile(query_index),get_query_efile(query_index));
     std::unordered_set<VertexID> max_dual_set =generate.get_dual_node_result(dgraph,qgraph);
     int d_Q = cal_diameter_qgraph(qgraph);
     cout<<"Base Graph vertices: "<<dgraph.GetNumVertices()<<"  Base Graph edges: "<<dgraph.GetNumEdges()<<endl;
     int j =1;
     while(j<circle_num){
         std::set<std::pair<VertexID,VertexID>> add_edges,rm_edges;
         Load_bunch_edges(add_edges,base_add_file,j);
         Load_bunch_edges(rm_edges,base_remove_file,j);
         std::set<pair<VertexID,VertexID>> tmp_set;
         for(auto e:add_edges){
             tmp_set.insert(e);
         }
         for(auto e:rm_edges){
             tmp_set.insert(e);
         }
         std::unordered_set<VertexID> e_affected_nodes = find_affected_area(dgraph,tmp_set,d_Q);
         e_affected_nodes = intersection(e_affected_nodes,max_dual_set);
         std::cout<<"affected center rate "<<j<<' '<<(e_affected_nodes.size()*1.0/max_dual_set.size())<<endl;
         j++;
     }

}

 void first_strong_strongInc(int circle_num){
  StrongInc stronginc;
  StrongSim strongsim;
  DualSim dualsim;

  Graph dgraph;
  GraphLoader dgraph_loader,qgraph_loader;
  dgraph_loader.LoadGraph(dgraph,graph_vfile,graph_efile);
  int index =query_index;
  Generate generate;
      Graph qgraph;
      qgraph_loader.LoadGraph(qgraph,get_query_vfile(index),get_query_efile(index));
      std::unordered_map<VertexID, std::unordered_set<VertexID>> sim;
      clock_t s0,e0;
      s0 =clock();
      std::vector<StrongR> strongsimr = strongsim.strong_simulation_sim(dgraph,qgraph);
  //    std::vector<StrongR> strongsimr;
      e0 =clock();
      std::cout<<"calculate original strong"<<(float)(e0-s0)/CLOCKS_PER_SEC<<"s"<<std::endl;
      bool initialized_sim=false;
      dualsim.dual_simulation(dgraph,qgraph,sim,initialized_sim);
      std::unordered_set<VertexID> max_dual_set =generate.get_dual_node_result(dgraph,qgraph);
      std::cout<<"max dual size "<<max_dual_set.size()<<std::endl;
      int j=1;
     // std::vector<std::pair<VertexID,VertexID>> affected_center_edges;
     // LoadEdges(affected_center_edges,affecte_center_file);
      std::fstream outfile("runtime.txt",std::ios::out);
      outfile.close();
      while (j<=circle_num){
          GraphLoader dgraph_loaddir,dgraph_loadinc;
          Graph dgraphdir,dgraphinc;
          dgraph_loaddir.LoadGraph(dgraphdir,graph_vfile,graph_efile);
          dgraph_loadinc.LoadGraph(dgraphinc,graph_vfile,graph_efile);
          std::set<std::pair<VertexID,VertexID>> add_edges,rm_edges;
         Load_bunch_edges(add_edges,base_add_file,j);
         Load_bunch_edges(rm_edges,base_remove_file,j);
         // for(int i=0;i<affected_center_edges.size();i++){
        //      if(i<(affected_center_edges.size()/circle_num+1)*j){
        //          add_edges.insert(affected_center_edges[i]);
         //     }
         // }
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
 }
private:
    std::string test_data_name ="yago";
    std::string graph_vfile ="../data/yago/yago.v";
    std::string graph_efile ="../data/yago/yago.e";
    std::string r_file = "../data/yago/yago.r";
    std::string base_qfile = "../data/yago/query5/q";
    std::string base_add_file = "../data/yago/inc/add_e";
    std::string base_remove_file="../data/yago/inc/rm_e";
    int query_index = 1;

};


int main(int argc, char *argv[]) {
  google::SetUsageMessage("Usage: test [gflags_opt]");
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::ShutDownCommandLineFlags();
  google::InitGoogleLogging("test for working");
  google::ShutdownGoogleLogging();
  init_workers();
//  int query_index = 1;
//  StrongExr strongexr("yago",1);
//  strongexr.generate_affected_center(200,0.06,"../data/affected_center.txt");
//  strongexr.generate_outside_center(50000,30);
  StrongExr strongexr("yago",3);
 // strongexr.print_dual_and_strong_information();
//  strongexr.generate_query();
//  strongexr.generate_affected_center(200,0.04,"../data/dbpedia/affected_center.txt");
  //strongexr.generate_outside_center(50000,10);
//  strongexr.test_strongsimulation_inc("../data/dbpedia/affected_center.txt",30);
//strongexr.first_strong_strongInc(30);
 // strongexr.print_affected_center_info(10);
 strongexr.generate_query(200,5,1000);
 //strongexr.generate_all_random_edges(100000,20);
  strongexr.print_evaluate_incremental_information(10);
  worker_finalize();
  return 0;
}

using namespace std;