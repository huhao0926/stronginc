#ifndef CPP_UTIL_H_
#define CPP_UTIL_H_

#include <gflags/gflags.h>
#include <glog/logging.h>
#include<boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include<time.h>

#include<iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <queue>

#include "cpp/core/global.h"
#include "cpp/core//graphapi.h"
#define random(a,b) (rand()%(b-a+1)+a)
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

template<class T>
  std::unordered_set<T> unions(const std::unordered_set<T> &a, const std::unordered_set<T>& b) {
		std::unordered_set<T> ret;
		for (auto ele : a) {
			ret.insert(ele);
		}
		for (auto ele :b){
		    ret.insert(ele);
		}
		return ret;
}

int cal_diameter_qgraph(Graph &qgraph);

void print_sim_vertex_result(Graph &qgraph,std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim);

void print_sim_edge_result(std::unordered_map<Edge,std::unordered_set<Edge>> &part);

bool  query_labl_all_notsame(Graph &qgraph);

void generate_connect_graphs_by_Dgraph(Graph &dgraph, Graph &qgraph, const int num_nodes);

void save_grape_file(Graph &qgraph, const std::string &v_file, const std::string &e_file);

bool dual_the_same(Graph &qgraph,  std::unordered_map<VertexID, std::unordered_set<VertexID>> &direct_sim,  std::unordered_map<VertexID, std::unordered_set<VertexID>> &incdsim);

void save_sim_result(Graph &qgraph,std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim,std::string filename);

void load_sim_result(std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim,std::string filename);

void LoadEdges(std::vector<std::pair<VertexID,VertexID>> &edges, const std::string efile);

void Load_bunch_edges(std::vector<std::pair<VertexID,VertexID>> &edges,const std::string basefilename,int index);

void LoadEdges(std::set<std::pair<VertexID,VertexID>> &edges, const std::string efile);

void Load_bunch_edges(std::set<std::pair<VertexID,VertexID>> &edges,const std::string basefilename,int index);

void save_edges(std::set<std::pair<VertexID,VertexID>> &edges, const std::string efile);

void genertate_random_edges(std::set<std::pair<VertexID,VertexID>> &exist_edges,const std::string basefilename,int dgraph_num_vertices,int generate_edge_num,int ci);
#endif //CPP_UTIL_H_
using namespace std;