#ifndef CPP_BFSCONNECTIVITY_H_
#define CPP_BFSCONNECTIVITY_H_

#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <bitset>

#include "cpp/core/Fragment.h"
#include "cpp/core/global.h"
#include "cpp/utils/MessageBuffer.h"
#include "cpp/core/graph_view.h"
/**
    *parallel find connectivity nodes for node;
*/
//template<class T>
class Bfs_Connectivity{
public:
    Bfs_Connectivity();

    ~Bfs_Connectivity();

template<class T>
    void bfs_parallel(Fragment &fragment, T &dgraph, std::unordered_set<VertexID> &nodeset, VertexID root){
        worker_barrier();
        pEval(fragment, dgraph, nodeset, root);
        worker_barrier();
        while (is_continue()){
            incEval(fragment, dgraph, nodeset, root);
            worker_barrier();
        }
//        out_global_result(fragment,nodeset);
    }

    bool is_continue();
   /**
    *partial evaluateion find connectivity nodes in each process
    */
    template<class T>
    void pEval(Fragment &fragment, T &dgraph,std::unordered_set<VertexID> &nodeset, VertexID root){
        std::queue<VertexID> que;
        if (fragment.isInnerVertex(root)){
           que.emplace(fragment.getLocalID(root));
        }
        while (!que.empty()){
           VertexID u = que.front();
           que.pop();
           if (nodeset.find(u) != nodeset.end()){
               continue;
           }
           nodeset.insert(u);
           if (fragment.isBorderVertex(fragment.getGlobalID(u))){
                // std::cout<<"num_fragments "<<NUM_FRAGMENTS<<std::endl;
               std::bitset<NUM_FRAGMENTS> dest= fragment.getMsgThroughDest(fragment.getGlobalID(u));
                for (int fid = 0; fid < dest.size(); fid++){
                   if (dest.test(fid) && fid != get_worker_id()){
                       messageBuffers.add_message(fid,fragment.getGlobalID(u));
                   }
               }
           }
           for (auto v :dgraph.GetParentsID(u)){
                    que.emplace(v);
           }
           for(auto v :dgraph.GetChildrenID(u)){
                 if(fragment.isInnerVertex(fragment.getGlobalID(v))){
                    que.emplace(v);
                 }
           }
        }
        messageBuffers.sync_messages();
    }
   /**
    *incremental evaluateion find connectivity nodes in each process by messages exchange.
    */
   template<class T>
    void incEval(Fragment &fragment, T &dgraph,std::unordered_set<VertexID> &nodeset, VertexID root){
        std::queue<VertexID> que;
        for (auto u :messageBuffers.get_messages()){
            que.emplace(fragment.getLocalID(u));
        }
        messageBuffers.reset_in_messages();
        while(!que.empty()){
           VertexID u = que.front();
           que.pop();
           if (nodeset.find(u) != nodeset.end()){
                   continue;
           }
           nodeset.insert(u);
           if (fragment.isBorderVertex(fragment.getGlobalID(u))){
               std::bitset<NUM_FRAGMENTS> dest = fragment.getMsgThroughDest(fragment.getGlobalID(u));
               for (int fid = 0; fid < dest.size(); fid++){
                   if (dest.test(fid) && fid != get_worker_id()){
                       messageBuffers.add_message(fid,fragment.getGlobalID(u));
                   }
               }
           }
           for (auto v :dgraph.GetParentsID(u)){
                    que.emplace(v);
           }
           for(auto v :dgraph.GetChildrenID(u)){
//                 if(fragment.isInnerVertex(fragment.getGlobalID(v))){
                    que.emplace(v);
                 }
        }
        messageBuffers.sync_messages();
    }

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