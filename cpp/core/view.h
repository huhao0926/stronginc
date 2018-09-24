#ifndef CPP_VIEW_H_
#define CPP_VIEW_H_
#include "edge.h"
#include "vertex.h"
#include "graph.h"
#include "cpp/serial/dualsimulation.h"
#include "cpp/utils/util.h"
#include "cpp/core/strongr.h"
#include "ball_view.h"
#include "cpp/serial/strongsimulation.h"
#include <sys/time.h>
#include<ctime>
#include<unordered_map>
#include<unordered_set>
#include<vector>
class View{
private:
    std::vector<Graph*> ViewGraph_list;    // a list of view patterh
    std::vector<std::unordered_set<Edge>> MGSet; //a list of view patthern match query edges;
    std::vector<int> contain;
    std::string view_base_file ="../data/contain1/data";
public:
    View();

    ~View();

    void add_ViewGraph(Graph* view);

    std::vector<Graph*> get_ViewGraph_list();

    void traverse_ViewGraph();

    bool vector_find(std::vector<int> &nums,int target);

    std::unordered_map<Edge,std::unordered_set<Edge>> simTran(Graph &dgraph,Graph &qgraph, std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim);

    bool containCheck(Graph &qgraph);

    std::vector<int> minContain(Graph &qgraph);
/*
    std::unordered_map<int,std::vector<StrongR>> cache_by_strong(Graph &dgraph,std::vector<int> &min_contain_vec);

    std::unordered_map<int,std::unordered_map<int,std::unordered_set<VertexID>>> cache_by_dual(Graph &dgraph,std::vector<int> &min_contain_vec);

    void deal_dual_cache(Graph &qgraph,std::vector<int> &min_contain_vec,std::unordered_map<int,std::unordered_map<int,std::unordered_set<VertexID>>> &result,std::unordered_map<VertexID, std::unordered_set<VertexID>> &max_query_sim, std::unordered_set<VertexID> &view_nodes);

    void deal_strong_cache(Graph &qgraph,std::vector<int> &min_contain_vec,std::unordered_map<int,std::vector<StrongR>> &result,std::unordered_map<VertexID, std::unordered_set<VertexID>> &max_query_sim, std::unordered_set<VertexID> &view_nodes);
*/
    void cache_node_distance(Graph &dgraph,std::unordered_set<VertexID> &ball_node,std::unordered_map<VertexID,std::unordered_map<VertexID,int>> &dis_ball);

    double get_current_time();

    void material_view(Graph &dgraph,Graph &qgraph,
                                     std::unordered_map<int,std::unordered_map<int,std::unordered_set<VertexID>>> &dual_result,
                                     std::unordered_map<int,std::vector<StrongR>> &strong_result,
                                     std::unordered_map<VertexID,std::unordered_map<VertexID,int>> &dis_ball,
                                     int cache_strategy);

    void deal_cache_info(Graph &dgraph,Graph &qgraph,std::unordered_map<int,std::unordered_map<int,std::unordered_set<VertexID>>> &dual_result,
                                     std::unordered_map<int,std::vector<StrongR>> &strong_result,
                                     std::unordered_map<VertexID,std::unordered_map<VertexID,int>> &dis_ball,
                                     int cache_strategy,
                                     std::vector<StrongR> &max_result);

    std::vector<StrongR>  queryByViews(Graph &dgraph,Graph &qgraph, int cache_strategy);

    int cal_diameter_qgraph(Graph &qgraph);

    bool valid_sim_w(Graph &qgraph,std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim,VertexID w);

    void rename_sim(Ball_View &ball_view,Graph &qgraph,
                               std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim);

    void ss_counter_initialization(Ball_View &ball_view,Graph &qgraph,
                                     std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre,
                                     std::unordered_map<VertexID, std::vector<int>> &sim_counter_post,
                                     std::unordered_map<VertexID, std::unordered_set<VertexID>> &S_w);

    void dual_filter_match(Ball_View &refined_ball, Graph &qgraph,
                      std::unordered_map<VertexID, std::unordered_set<VertexID>> &S_w);

    void push_phase(Ball_View &ball,Graph &qgraph,
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

    void clear_parameter();

};
#endif //CPP_VIEW_H_
using namespace std;