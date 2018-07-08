#ifndef CPP_BFSMULTIKHOP_H_
#define CPP_BFSMULTIKHOP_H_

#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <bitset>

#include "cpp/core/Fragment.h"
#include "cpp/core/global.h"
#include "cpp/utils/MessageBuffer.h"
/**
    *parallel find d_hop nodes from a node set
*/
class Bfs_Multikhop{
public:
    Bfs_Multikhop();

    ~Bfs_Multikhop();

    void bfs_multikhopparallel(Fragment &fragment, Graph &dgraph,std::unordered_set<VertexID> &nodeset, std::unordered_set<VertexID> &root, int bound);

    bool is_continue();
   /**
    *partial evaluateion find d_hop nodes in each process
    */
    void pEval(Fragment &fragment, Graph &dgraph,std::unordered_set<VertexID> &nodeset, std::unordered_map<VertexID,int> &DistInf, std::unordered_set<VertexID> &root, int bound);
   /**
    *incremental evaluateion find d_hop nodes in each process
    */
    void incEval(Fragment &fragment, Graph &dgraph,std::unordered_set<VertexID> &nodeset, std::unordered_map<VertexID,int> &DistInf, std::unordered_set<VertexID> &root, int bound);
   /**
    *change each local result id to global id
    */
    void out_global_result(Fragment &fragment, std::unordered_set<VertexID> &nodeset);

    std::unordered_set<VertexID> assemble_result(std::unordered_set<VertexID> &nodeset);
private:
    int continue_run = 0;
    MessageBuffer<std::pair < int, int>> messageBuffers;
};
#endif //CPP_BFSMULTIKHOP_H_
using namespace std;