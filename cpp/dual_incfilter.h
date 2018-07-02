#ifndef CPP_DUALINCFILTER_H_
#define CPP_DUALINCFILTER_H_

#include <unordered_map>
#include <unordered_set>
#include <bitset>

#include "cpp/Fragment.h"
#include "cpp/global.h"
#include "cpp/MessageBuffer.h"
#include "cpp/graph_view.h"

class Dual_Incfilter{
public:
    Dual_Incfilter();

    ~Dual_Incfilter();

    void dual_counter_initialization(GraphView &graph_view, Graph &qgraph,
                                     std::unordered_map<VertexID, std::vector<int>> &sim_counter_post,
                                     std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre,
                                     std::unordered_map<VertexID, std::unordered_set<VertexID>> &dsim);

    void update_counter(GraphView &graph_view,Graph &qgraph,VertexID u,VertexID v,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_post);

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