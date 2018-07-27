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

    std::vector<StrongR>  queryByViews(Graph &dgraph,Graph &qgraph);

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