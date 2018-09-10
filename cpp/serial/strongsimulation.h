#ifndef CPP_STRONGSIMULATION_H_
#define CPP_STRONGSIMULATION_H_
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <string>
#include "cpp/core/graphapi.h"
#include "dualsimulation.h"
#include "cpp/core/strongr.h"
#include "cpp/core/ball_view.h"
#include "cpp/utils/util.h"
class StrongSim{
  public:
      StrongSim();

      ~StrongSim();

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

      void print_ball_info(Graph &qgraph,std::unordered_map<VertexID, std::unordered_set<VertexID>> &S_w,VertexID w);
      /**
        calculate strong simulation result
        */

     std::vector<StrongR> strong_simulation_sim(Graph &dgraph, Graph &qgraph);

     void  cal_culculate_directed_dhop_nodes(Graph& dgraph,VertexID vid, int d_hop,std::unordered_set<VertexID> &result,std::vector<int>& dis);

     std::vector<StrongR>  strong_simulation_sim_ball_dhop_info(Graph &dgraph, Graph &qgraph,std::unordered_map<VertexID, std::unordered_set<VertexID>> &ball_dhop_info,std::unordered_map<VertexID,std::vector<int>> &ball_dis_info);

};
#endif //CPP_STRONGSIMULATION_H_