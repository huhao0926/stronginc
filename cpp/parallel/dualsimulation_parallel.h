#ifndef CPP_DUALSIMULATIONPARALLEL_H_
#define CPP_DUALSIMULATIONPARALLEL_H_

#include <unordered_map>
#include <unordered_set>
#include <bitset>

#include "cpp/core/Fragment.h"
#include "cpp/core/global.h"
#include "cpp/utils/MessageBuffer.h"
 /**
  * algorithm for parallel  dual simualation
  */
class Dual_Parallel{
public:
    Dual_Parallel();

    Dual_Parallel(Fragment &fragment);

    ~Dual_Parallel();

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


    void dual_sim_initialization(Fragment &fragment, Graph &dgraph, Graph &qgraph,
                               std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim,
                               bool &initialized_sim,
                               std::unordered_map<VertexID, std::unordered_set<VertexID>> &remove_pred,
                               std::unordered_map<VertexID, std::unordered_set<VertexID>> &remove_succ);


    void reunordered_map_data_id(std::unordered_map<VertexID, VertexID> &t_f,Graph &dgraph);

    void dual_counter_initialization(Graph &dgraph, Graph &qgraph,
                                     std::unordered_map<VertexID, std::vector<int>> &sim_counter_post,
                                     std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre,
                                     std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim);

    VertexID find_nonempty_remove(Graph &qgraph,
                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &remove_pred,
                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &remove_succ);

    void update_sim_counter(Graph &dgraph,
                            std::unordered_map<VertexID, std::vector<int>> &sim_counter_post,
                            std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre,
                            VertexID u,VertexID w);

    bool match_check(Graph &qgraph,std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim);


    bool dual_sim_output(Graph &qgraph,std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim);


    void dual_sim_refinement(Fragment &fragment, Graph &dgraph, Graph &qgraph,
                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim,
                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &remove_pred,
                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &remove_succ,
                           std::unordered_map<VertexID, std::vector<int>> &sim_counter_post,
                           std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre);

    void dual_simulation(Graph &dgraph, Graph &qgraph, std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim, bool &initialized_sim);

    bool is_continue();

    void dual_paraller(Fragment &fragment, Graph &dgraph, Graph &qgraph, std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim);

    void pEval(Fragment &fragment, Graph &dgraph, Graph &qgraph,
                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim,
                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &remove_pred,
                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &remove_succ,
                           std::unordered_map<VertexID, std::vector<int>> &sim_counter_post,
                           std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre);

    void incEval(Fragment &fragment, Graph &dgraph, Graph &qgraph,
                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim,
                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &remove_pred,
                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &remove_succ,
                           std::unordered_map<VertexID, std::vector<int>> &sim_counter_post,
                           std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre);

   void out_global_result(Fragment &fragment,  Graph &qgraph, std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim);
private:
    int continue_run = 0;
    MessageBuffer<std::pair < VertexID, int>> messageBuffers;
    std::unordered_set<VertexID> *OuterVertices;
    std::unordered_set<VertexID> *innerVertices;
};

#endif //CPP_DUALSIMULATIONPARALLEL_H_