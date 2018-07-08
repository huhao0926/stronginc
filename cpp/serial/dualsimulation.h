#ifndef CPP_DUALSIMULATION_H_
#define CPP_DUALSIMULATION_H_
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <unordered_map>
#include <unordered_set>
#include "cpp/core/graphapi.h"
 /**
  * algorithm for dualsimulation
  */
class DualSim {
 public:
  DualSim();
  ~DualSim();
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


    void dual_sim_initialization(Graph &dgraph, Graph &qgraph,
                               std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim,
                               bool &initialized_sim,
                               std::unordered_map<VertexID, std::unordered_set<VertexID>> &remove_pred,
                               std::unordered_map<VertexID, std::unordered_set<VertexID>> &remove_succ);


    void reunordered_map_data_id(std::unordered_map<VertexID, VertexID> &t_f,Graph &dgraph);

    void dual_counter_initialization(Graph &dgraph, Graph &qgraph,
                                     std::unordered_map<VertexID, std::vector<int>> &sim_counter_post,
                                     std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre,
                                     std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim,
                                     std::unordered_map<VertexID, VertexID> &t_f);

    VertexID find_nonempty_remove(Graph &qgraph,
                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &remove_pred,
                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &remove_succ);

    void update_sim_counter(Graph &dgraph,
                            std::unordered_map<VertexID, std::vector<int>> &sim_counter_post,
                            std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre,
                            VertexID u,VertexID w,
                            std::unordered_map<VertexID, VertexID> &t_f);

    bool match_check(Graph &qgraph,std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim);
	

    bool dual_sim_output(Graph &qgraph,std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim);


    void dual_sim_refinement(Graph &dgraph, Graph &qgraph,
                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim,
                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &remove_pred,
                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &remove_succ);

    void dual_simulation(Graph &dgraph, Graph &qgraph, std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim, bool &initialized_sim);
 private:
   //int n;
  //Graph qgraph;
  //Graph dgraph;
  //GraphLoader testgraph_loader_;
};

#endif //CPP_DUALSIMULATION_H_