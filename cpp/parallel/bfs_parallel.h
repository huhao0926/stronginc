#ifndef CPP_BFSPARALLEL_H_
#define CPP_BFSPARALLEL_H_

#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <bitset>

#include "cpp/core/Fragment.h"
#include "cpp/core/global.h"
#include "cpp/utils/MessageBuffer.h"
   /**
    *parallel calculate shortest distance from a node
    */
class Bfs_Parallel{
public:
    Bfs_Parallel();

    ~Bfs_Parallel();

    void bfs_parallel(Fragment &fragment, Graph &dgraph, std::unordered_map<VertexID,int> &DistInf, VertexID root);

    bool is_continue();
   /**
    *partial evaluateion record distance info in each process
    */
    void pEval(Fragment &fragment, Graph &dgraph,std::unordered_set<VertexID> &nodeset, std::unordered_map<VertexID,int> &DistInf, VertexID root);
   /**
    *incremental evaluateion find d_hop nodes in each process by messages exhange
    */
    void incEval(Fragment &fragment, Graph &dgraph,std::unordered_set<VertexID> &nodeset, std::unordered_map<VertexID,int> &DistInf, VertexID root);
   /**
    *gather each process result and result a global info for shortest distance
    */
    void gather_result(std::unordered_map<VertexID,int> &DistInf);
private:
    int continue_run = 0;
    MessageBuffer<std::pair < int, int>> messageBuffers;
};
#endif //CPP_BFSPARALLEL_H_
using namespace std;