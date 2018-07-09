#ifndef CPP_STRONGSIMPARALLEL_H_
#define CPP_STRONGSIMPARALLEL_H_
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <unordered_map>
#include <unordered_set>
#include "cpp/core/graphapi.h"
#include "dualsimulation_parallel.h"
#include "bfs_singlekhop.h"
#include "bfs_connectivity.h"
#include "dual_incfilter.h"
class Strong_Parallel{
public:
     Strong_Parallel();

     ~Strong_Parallel();

     int cal_diameter_qgraph(Graph &qgraph);

     void rename_sim(GraphView &ball_view,Graph &qgraph,
                               std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim);

     void broad_all_dual_node(Fragment &fragment,Graph &qgraph,
                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim,
                           std::unordered_set<VertexID> &center_node);

     void get_ball_nodes(Fragment &fragment, Graph &dgraph, std::unordered_set<VertexID> &ball_node, int w,int d_Q);


    void connectivity_prune(Fragment fragment, Graph &dgraph,Graph &qgraph,std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim,int w,
                            std::unordered_map<VertexID, std::unordered_set<VertexID>> &S_w,
                            std::unordered_set<VertexID> &ball_node,
                            std::unordered_set<VertexID> &prune_node);

    void out_global_result(Fragment &fragment, Graph &qgraph, std::unordered_map<VertexID, std::unordered_set<VertexID>> &S_w);

    void print_ball_info(Fragment &fragment, Graph &qgraph, std::unordered_map<VertexID, std::unordered_set<VertexID>> &S_w);

    void strongsim_parallel(Fragment &fragment, Graph &dgraph,Graph &qgraph);
private:
    int continue_run = 0;

    MessageBuffer<VertexID> messageBuffers;


};
#endif //CPP_STRONGSIMPARALLEL_H_
using namespace std;