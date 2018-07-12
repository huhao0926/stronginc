#ifndef CPP_STRONGPARALLELINCREMENTAL_H_
#define CPP_STRONGPARALLELINCREMENTAL_H_
#include <gflags/gflags.h>
#include <glog/logging.h>
#include<iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include "bfs_multikhop.h"
#include "cpp/core/Fragment.h"
#include "cpp/core/global.h"
#include "cpp/utils/MessageBuffer.h"
#include "cpp/serial/dualsimulation.h"
#include "cpp/core/strongr.h"
#include "cpp/core/ball_view.h"
#include "cpp/serial/dual_incremental.h"
#include "dual_parallel_inc.h"
class StrongparallelInc{
public:
    StrongparallelInc();

    ~StrongparallelInc();

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

    void set_change_num(const int num);

    void set_base_add_file(const std::string base_file);

    void set_base_remove_file(const std::string base_file);

    void LoadEdges(std::vector<std::pair<VertexID,VertexID>> &edges, const std::string efile);

	void Load_bunch_edges(std::vector<std::pair<VertexID,VertexID>> &edges,const std::string basefilename,int index);

	void LoadEdges(std::set<std::pair<VertexID,VertexID>> &edges, const std::string efile);

	void Load_bunch_edges(std::set<std::pair<VertexID,VertexID>> &edges,const std::string basefilename,int index);

    std::unordered_set<VertexID> find_dhop_nodes(Fragment& fragment, Graph& dgraph,int d_Q);

    std::unordered_set<VertexID> find_affected_area(Fragment& fragment, Graph& dgraph, std::set<std::pair<VertexID,VertexID>> &add_edges,std::set<std::pair<VertexID,VertexID>> &rm_edges,int d_Q);

    void update_fragment_inc(Fragment &fragment,Graph& dgraph,
                                             std::unordered_set<VertexID> &affected_nodes,
                                             std::set<std::pair<VertexID,VertexID>> &add_edges,
                                             std::set<std::pair<VertexID,VertexID>> &rm_edges,
                                             int d_Q);

    void update_fragment_parallel(Fragment &fragment,Graph& dgraph,int d_Q);

    std::vector<std::pair<VertexID,VertexID>> assign_task(Fragment &fragment,Graph &qgraph,
                                                                            std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim,
                                                                            std::unordered_set<VertexID> &affected_center,
                                                                            std::unordered_set<VertexID> &affected_nodes);
      /**
       calculate diameter for qgraph
       */
    int cal_diameter_qgraph(Graph &qgraph);

      /**
       judge node w is a valid node in sim set
       */
    bool valid_sim_w(Graph &qgraph,std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim,VertexID w);

    void find_node_connectivity_nodes(Ball_View &ball_view,std::unordered_set<VertexID> &v_set, VertexID w);

    void rename_sim(Ball_View &ball_view,Graph &qgraph,
                               std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim);

    void ss_counter_initialization(Ball_View &ball_view,Graph &qgraph,
                                     std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre,
                                     std::unordered_map<VertexID, std::vector<int>> &sim_counter_post,
                                     std::unordered_map<VertexID, std::unordered_set<VertexID>> &S_w);

    void dual_filter_match(Ball_View &refined_ball_view, Graph &qgraph,
                              std::unordered_map<VertexID, std::unordered_set<VertexID>> &S_w,VertexID w,int d_Q);


    void push_phase(Ball_View &ball,Graph &qgraph,VertexID w,int d_Q,
                                  std::set<std::pair<VertexID,VertexID>> &filter_set,
                                  std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre,
                                  std::unordered_map<VertexID, std::vector<int>> &sim_counter_post,
                                  std::unordered_map<VertexID, std::unordered_set<VertexID>> &S_w);

    void update_counter(Ball_View &ball,Graph &qgraph,VertexID u,VertexID v,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_post);

    void decremental_refine(Ball_View &ball_view,Graph &qgraph,
                          std::set<std::pair<VertexID,VertexID>> &filter_set,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_post,
                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &S_w);

    void extract_max_pg(Ball_View &ball_view,Graph &dgraph,Graph &qgraph,VertexID w,
                                std::unordered_map<VertexID, std::unordered_set<VertexID>> &S_w);

    void out_global_result(Fragment &fragment,  Graph &qgraph, std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim);

    void print_ball_info(Graph &qgraph,std::unordered_map<VertexID, std::unordered_set<VertexID>> &S_w,VertexID w);

    void  recalculate_incrementl_dual(Graph &dgraph, Graph &qgraph,
                                      std::unordered_map<VertexID,std::unordered_set<VertexID>> &dsim,
                                      std::set<std::pair<VertexID,VertexID>> &add_edges,
                                      std::set<std::pair<VertexID,VertexID>> &rm_edges);

    std::vector<StrongR>  strong_parallel(Fragment &fragment,Graph &dgraph, Graph &qgraph);

    std::vector<StrongR>  strong_parallel_inc(Fragment &fragment,Graph &dgraph, Graph &qgraph,
                                                      std::unordered_map<VertexID,std::unordered_set<VertexID>> &global_dsim,
                                                      std::vector<StrongR> &global_strong_r,
                                                      std::set<std::pair<VertexID,VertexID>> &add_edges,
                                                      std::set<std::pair<VertexID,VertexID>> &rm_edges);
private:
     MessageBuffer<Vertex> Vertex_MessageBuffers;
     MessageBuffer<Edge> Edge_MessageBuffers;
     MessageBuffer<VertexID> Id_MessageBuffers;
     MessageBuffer<std::pair<VertexID,VertexID>> pair_MessageBuffers;
     std::string base_add_file="../data/incsynmtic/add_e";
     std::string base_remove_file="../data/incsynmtic/rm_e";
     int change_num=0;
};

#endif //CPP_STRONGPARALLELINCREMENTAL_H_