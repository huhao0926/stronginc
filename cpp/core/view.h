#ifndef CPP_VIEW_H_
#define CPP_VIEW_H_
#include "edge.h"
#include "vertex.h"
#include "graph.h"
#include "cpp/serial/dualsimulation.h"
#include "cpp/utils/util.h"
#include<unordered_map>
#include<unordered_set>
#include<vector>
class View{
private:
    std::vector<Graph*> ViewGraph_list;    // a list of view patterh
    std::vector<std::unordered_set<Edge>> MGSet; //a list of view patthern match query edges;
    std::vector<int> contain;
public:
    View();

    ~View();

    void add_ViewGraph(Graph* view);

    void traverse_ViewGraph();

    bool vector_find(std::vector<int> &nums,int target);

    std::unordered_map<Edge,std::unordered_set<Edge>> simTran(Graph &dgraph,Graph &qgraph, std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim);

    bool containCheck(Graph &qgraph);

    std::vector<int> minContain(Graph &qgraph);

    void clear_parameter();

};
#endif //CPP_VIEW_H_
using namespace std;