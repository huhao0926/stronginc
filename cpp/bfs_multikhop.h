#ifndef CPP_BFSMULTIKHOP_H_
#define CPP_BFSMULTIKHOP_H_

#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <bitset>

#include "cpp/Fragment.h"
#include "cpp/global.h"
#include "cpp/MessageBuffer.h"
class Bfs_Multikhop{
public:
    Bfs_Multikhop();

    ~Bfs_Multikhop();

    void bfs_multikhopparallel(Fragment &fragment, Graph &dgraph,std::unordered_set<VertexID> &nodeset, std::unordered_set<VertexID> &root, int bound);

    bool is_continue();

    void pEval(Fragment &fragment, Graph &dgraph,std::unordered_set<VertexID> &nodeset, std::unordered_map<VertexID,int> &DistInf, std::unordered_set<VertexID> &root, int bound);

    void incEval(Fragment &fragment, Graph &dgraph,std::unordered_set<VertexID> &nodeset, std::unordered_map<VertexID,int> &DistInf, std::unordered_set<VertexID> &root, int bound);

    void out_global_result(Fragment &fragment, std::unordered_set<VertexID> &nodeset);

    std::unordered_set<VertexID> assemble_result(std::unordered_set<VertexID> &nodeset);
private:
    int continue_run = 0;
    MessageBuffer<std::pair < int, int>> messageBuffers;
};
#endif //CPP_BFSMULTIKHOP_H_
using namespace std;