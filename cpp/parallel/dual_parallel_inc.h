#ifndef CPP_DUALINCPARALLEL_H_
#define CPP_DUALINCPARALLEL_H_
#include "cpp/core/graphapi.h"
#include "cpp/core/Fragment.h"
#include "cpp/utils/MessageBuffer.h"
class Dual_parallelInc{
public:
    Dual_parallelInc();

    ~Dual_parallelInc();

    template <class T>
    void update_counter(T &dgraph,
                            std::unordered_map<VertexID, std::vector<int>> &sim_counter_post,
                            std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre,
                            VertexID u,VertexID v){
        for(auto vp : dgraph.GetParentsID(v)){
            if (sim_counter_post.find(vp)!=sim_counter_post.end()){
                if(sim_counter_post[vp][u]>0){
                    sim_counter_post[vp][u]-=1;
                }
            }
        }
        for (auto vs : dgraph.GetChildrenID(v)){
            if (sim_counter_pre.find(vs)!=sim_counter_pre.end()){
                if(sim_counter_pre[vs][u]>0){
                    sim_counter_pre[vs][u]-=1;
                }
            }
        }
     }

    bool is_continue();

    void propagate_add_PEval(Fragment &fragment, Graph &dgraph,Graph &qgraph,
                       std::set<std::pair<VertexID,VertexID>> &candidate_node,
                       std::unordered_map<VertexID, std::unordered_set<VertexID>> &aff_node,
                       std::unordered_map<VertexID, std::unordered_set<VertexID>> &dsim,
                       std::set<std::pair<VertexID,VertexID>> &already_matched);

    void propagate_add_IncEval(Fragment &fragment, Graph &dgraph,Graph &qgraph,
                       std::set<std::pair<VertexID,VertexID>> &candidate_node,
                       std::unordered_map<VertexID, std::unordered_set<VertexID>> &aff_node,
                       std::unordered_map<VertexID, std::unordered_set<VertexID>> &dsim,
                       std::set<std::pair<VertexID,VertexID>> &already_matched);

    void incremental_add_edges(Fragment &fragment, Graph &dgraph, Graph &qgraph,
                                                    std::unordered_map<VertexID, std::unordered_set<VertexID>> &dsim,
                                                    std::set<std::pair<VertexID,VertexID>> &add_edges);

    void incremental_remove_edgs(Fragment &fragment, Graph &dgraph,Graph &qgraph,
                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &dsim,
                           std::set<std::pair<VertexID,VertexID>> &rm_edges);

    void propagate_remove_PEval(Fragment &fragment,GraphView &graph_view,Graph &qgraph,
                          std::unordered_map<VertexID, std::unordered_set<VertexID>> &aff_node,
                          std::set<std::pair<VertexID,VertexID>> &filter_set,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_post,
                          std::set<std::pair<VertexID,VertexID>> &already_matched);

    void propagate_remove_IncEval(Fragment &fragment,GraphView &graph_view,Graph &qgraph,
                          std::unordered_map<VertexID, std::unordered_set<VertexID>> &aff_node,
                          std::set<std::pair<VertexID,VertexID>> &filter_set,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_post,
                          std::set<std::pair<VertexID,VertexID>> &already_matched);

    void dual_parallel_incremental(Fragment &fragment, Graph &dgraph, Graph &qgraph,
                                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &dsim,
                                           std::set<std::pair<VertexID,VertexID>> &add_edges,
                                           std::set<std::pair<VertexID,VertexID>> &rm_edges);

    void update_by_add_edges(Fragment &fragment,Graph &dgraph,std::set<std::pair<VertexID,VertexID>> &add_edges,bool communication_next);

    void update_by_remove_edges(Fragment &fragment,Graph &dgraph,std::set<std::pair<VertexID,VertexID>> &rm_edges,bool communication_next);

    void out_global_result(Fragment &fragment,  Graph &qgraph, std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim);

    void print_global_info(Fragment &fragment, Graph &qgraph, std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim);
private:
    int continue_run = 0;
    MessageBuffer<std::pair < VertexID, int>> messageBuffers;
    MessageBuffer<Vertex>  vertexBuffers;
};
#endif //CPP_DUALINCPARALLEL_H_
using namespace std;