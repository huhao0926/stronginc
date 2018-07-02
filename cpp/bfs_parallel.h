#ifndef CPP_BFSPARALLEL_H_
#define CPP_BFSPARALLEL_H_

#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <bitset>

#include "cpp/Fragment.h"
#include "cpp/global.h"
#include "cpp/MessageBuffer.h"
class Bfs_Parallel{
public:
    Bfs_Parallel();

    ~Bfs_Parallel();

    void bfs_parallel(Fragment &fragment, Graph &dgraph, std::unordered_map<VertexID,int> &DistInf, VertexID root);

    bool is_continue();

    void pEval(Fragment &fragment, Graph &dgraph,std::unordered_set<VertexID> &nodeset, std::unordered_map<VertexID,int> &DistInf, VertexID root);

    void incEval(Fragment &fragment, Graph &dgraph,std::unordered_set<VertexID> &nodeset, std::unordered_map<VertexID,int> &DistInf, VertexID root);

    void gather_result(std::unordered_map<VertexID,int> &DistInf);
private:
    int continue_run = 0;

    MessageBuffer<std::pair < int, int>> messageBuffers;




};
#endif //CPP_BFSPARALLEL_H_
using namespace std;