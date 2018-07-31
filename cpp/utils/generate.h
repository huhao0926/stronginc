#ifndef UTILS_GENERATE_H_
#define UTILS_GENERATE_H_
#include<ctime>
#include<unordered_set>
#include<unordered_map>
#include<iostream>
#include <fstream>
#include "cpp/core/graphapi.h"
#include "cpp/core/global.h"
#include "cpp/core/strongr.h"
#include "cpp/serial/dualsimulation.h"
#define random(a,b) (rand()%(b-a+1)+a)
class Generate{
public:
    std::string get_query_vfile(int index){
        return base_qfile+std::to_string(index)+".v";
    }

    std::string get_query_efile(int index){
        return base_qfile+std::to_string(index)+".e";
    }
public:
    Generate();

    Generate(std::string graph_vf,std::string graph_ef,std::string graph_rf,
             std::string base_qf,std::string base_add_f,std::string base_remove_f,int q_index);

    ~Generate();

    std::vector<VertexLabel> get_graph_label_vec(Graph &graph);

    void generate_random_dgraph(Graph &graph,int num_nodes=2000,double a = 1.20,int l = 200);

    void generate_random_connectivity_graph(Graph &graph,int num_nodes=3,int num_edges=14,int l=200);

    void generate_random_connectivity_graph(Graph &graph,int num_nodes,int num_edges,std::vector<VertexLabel> &labels);

    void generate_connect_graphs_by_Dgraph(Graph &dgraph, Graph &qgraph,const  int num_nodes);

    void save_grape_file(Graph &qgraph, const std::string &v_file, const std::string &e_file);

    std::unordered_set<VertexID> get_dual_node_result(Graph &dgraph,Graph &qgraph);

    std::pair<VertexID,VertexID> generate_one_add_edge_by_nodelist(std::vector<VertexID> &node_list,std::set<std::pair<VertexID,VertexID>> &exist_edges,std::set<std::pair<VertexID,VertexID>> &add_edges);

    std::pair<VertexID,VertexID> generate_one_add_edge_random(int node_num, std::set<std::pair<VertexID,VertexID>> &exist_edges,std::set<std::pair<VertexID,VertexID>> &add_edges);

    std::pair<VertexID,VertexID> generate_one_remove_edge_by_list(std::vector<std::pair<VertexID,VertexID>> &edge_list, std::set<std::pair<VertexID,VertexID>> &already_rm_edges,std::set<std::pair<VertexID,VertexID>> &remove_edges);

    std::set<std::pair<VertexID,VertexID>> generate_bunch_add_edges_random(int node_num, std::set<std::pair<VertexID,VertexID>> &exist_edges,int num_edges);

    std::set<std::pair<VertexID,VertexID>> generate_bunch_add_edges_by_nodelist(std::vector<VertexID> &node_list,std::set<std::pair<VertexID,VertexID>> &exist_edges,int num_edges);

    std::set<std::pair<VertexID,VertexID>> generate_bunch_remove_edge_by_list(std::vector<std::pair<VertexID,VertexID>> &edge_list, std::set<std::pair<VertexID,VertexID>> &already_rm_edges,int num_edges);

    std::pair<VertexID,VertexID> generate_one_remove_edge_by_set(std::set<std::pair<VertexID,VertexID>> &edge_set, std::set<std::pair<VertexID,VertexID>> &already_rm_edges,std::set<std::pair<VertexID,VertexID>> &remove_edges);

    std::set<std::pair<VertexID,VertexID>> generate_bunch_remove_edge_by_set(std::set<std::pair<VertexID,VertexID>> &edge_set, std::set<std::pair<VertexID,VertexID>> &already_rm_edges,int num_edges);
private:
    std::string graph_vfile ="../data/synmtic.v";
    std::string graph_efile ="../data/synmtic.e";
    std::string r_file = "../data/synmtic.r";
    std::string base_qfile = "../data/synmticquery/q";
    std::string base_add_file = "../data/incsynmtic/add_e";
    std::string base_remove_file="../data/incsynmtic/rm_e";
    int query_index = 1;
};
#endif//UTILS_GENERATE_H_
using namespace std;