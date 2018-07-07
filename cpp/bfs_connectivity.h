#ifndef CPP_BFSCONNECTIVITY_H_
#define CPP_BFSCONNECTIVITY_H_

#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <bitset>

#include "cpp/Fragment.h"
#include "cpp/global.h"
#include "cpp/MessageBuffer.h"
#include "cpp/graph_view.h"
/**
    *parallel find connectivity nodes for node;
*/
class Bfs_Connectivity{
public:
    Bfs_Connectivity();

    ~Bfs_Connectivity();

    void bfs_parallel(Fragment &fragment, GraphView &dgraph, std::unordered_set<VertexID> &nodeset, VertexID root);

    bool is_continue();
   /**
    *partial evaluateion find connectivity nodes in each process
    */
    void pEval(Fragment &fragment, GraphView &dgraph,std::unordered_set<VertexID> &nodeset, VertexID root);
   /**
    *incremental evaluateion find connectivity nodes in each process by messages exchange.
    */
    void incEval(Fragment &fragment, GraphView &dgraph,std::unordered_set<VertexID> &nodeset, VertexID root);

    void out_global_result(Fragment &fragment, std::unordered_set<VertexID> &nodeset);

private:
   /**
    *continue_run to charge if continue to run;
    */
    int continue_run = 0;
    MessageBuffer<VertexID> messageBuffers;
};
#endif //CPP_BFSCONNECTIVITY_H_
using namespace std;