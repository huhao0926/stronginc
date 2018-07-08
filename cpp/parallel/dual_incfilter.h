#ifndef CPP_DUALINCFILTER_H_
#define CPP_DUALINCFILTER_H_

#include <unordered_map>
#include <unordered_set>
#include <bitset>

#include "cpp/core/Fragment.h"
#include "cpp/core/global.h"
#include "cpp/utils/MessageBuffer.h"
#include "cpp/core/graph_view.h"
 /**
  * get a dual simulation result,and parallel incremental filter invalid result
  */
class Dual_Incfilter{
public:
    Dual_Incfilter();

    ~Dual_Incfilter();
 /**
  * inital dual simulation result
  */
    void dual_counter_initialization(GraphView &graph_view, Graph &qgraph,
                                     std::unordered_map<VertexID, std::vector<int>> &sim_counter_post,
                                     std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre,
                                     std::unordered_map<VertexID, std::unordered_set<VertexID>> &dsim);
 /**
  * if one node is remove from dual result,update other result parents and child info
  */
    void update_counter(GraphView &graph_view,Graph &qgraph,VertexID u,VertexID v,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_post);
 /**
  * iteratal remove invalid result from inital results
  */
    void decremental_rmove(Fragment &fragment, GraphView &graph_view,Graph &qgraph,
                          std::set<std::pair<VertexID,VertexID>> &filter_set,
                          std::unordered_map<VertexID, std::unordered_set<VertexID>> &dsim,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_post);

    void dual_incfilter(Fragment &fragment, GraphView &graph_view, Graph &qgraph,
                                             std::unordered_map<VertexID, std::unordered_set<VertexID>> &dsim);

    void pEval(Fragment &fragment, GraphView &graph_view, Graph &qgraph,
                          std::set<std::pair<VertexID,VertexID>> &filter_set,
                          std::unordered_map<VertexID, std::unordered_set<VertexID>> &dsim,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_post);

    void incEval(Fragment &fragment, GraphView &graph_view, Graph &qgraph,
                          std::set<std::pair<VertexID,VertexID>> &filter_set,
                          std::unordered_map<VertexID, std::unordered_set<VertexID>> &dsim,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_post);

    bool is_continue();

    void out_global_result(Fragment &fragment,  Graph &qgraph, std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim);
private:
    int continue_run = 0;

    MessageBuffer<std::pair < VertexID, int>> messageBuffers;
    std::unordered_set<VertexID> *OuterVertices;
    std::unordered_set<VertexID> *innerVertices;
};
#endif //CPP_DUALINCFILTER_H_
using namespace std;