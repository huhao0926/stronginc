#include "bfs_connectivity.h"
Bfs_Connectivity::Bfs_Connectivity(){
    messageBuffers.init();
}

Bfs_Connectivity::~Bfs_Connectivity(){}
//template<class T>
//void Bfs_Connectivity::bfs_parallel(Fragment &fragment, T &dgraph, std::unordered_set<VertexID> &nodeset, VertexID root){
////std::unordered_set<VertexID> nodeset;
//worker_barrier();
//pEval(fragment, dgraph, nodeset, root);
//worker_barrier();
//while (is_continue()){
//    incEval(fragment, dgraph, nodeset, root);
//    worker_barrier();
//}
//out_global_result(fragment,nodeset);
//}

//template<class T>
//void Bfs_Connectivity::pEval(Fragment &fragment, T &dgraph,std::unordered_set<VertexID> &nodeset, VertexID root){
//std::queue<VertexID> que;
//if (fragment.isInnerVertex(root)){
//    que.emplace(fragment.getLocalID(root));
// }
//while (!que.empty()){
//   VertexID u = que.front();
//   que.pop();
//   if (nodeset.find(u) != nodeset.end()){
//       continue;
//   }
//   nodeset.insert(u);
//   if (fragment.isBorderVertex(fragment.getGlobalID(u))){
//        // std::cout<<"num_fragments "<<NUM_FRAGMENTS<<std::endl;
//       std::bitset<NUM_FRAGMENTS> dest= fragment.getMsgThroughDest(fragment.getGlobalID(u));
//       for (int fid = 0; fid < dest.size(); fid++){
//           if (dest.test(fid) && fid != get_worker_id()){
//               messageBuffers.add_message(fid,fragment.getGlobalID(u));
//           }
//       }
//   }
//   for (auto v :dgraph.GetParentsID(u)){
//            que.emplace(v);
//   }
//   for(auto v :dgraph.GetChildrenID(u)){
//         if(fragment.isInnerVertex(fragment.getGlobalID(v))){
//            que.emplace(v);
//         }
//   }
//}
//  messageBuffers.sync_messages();
//}

//template<class T>
//void Bfs_Connectivity::incEval(Fragment &fragment, T &dgraph,std::unordered_set<VertexID> &nodeset,  VertexID root){
//std::queue<VertexID> que;
//for (auto u :messageBuffers.get_messages()){
//    que.emplace(fragment.getLocalID(u));
//}
//messageBuffers.reset_in_messages();
//while(!que.empty()){
//   VertexID u = que.front();
//   que.pop();
//   if (nodeset.find(u) != nodeset.end()){
//           continue;
//   }
//   nodeset.insert(u);
//   if (fragment.isBorderVertex(fragment.getGlobalID(u))){
//       std::bitset<NUM_FRAGMENTS> dest = fragment.getMsgThroughDest(fragment.getGlobalID(u));
//       for (int fid = 0; fid < dest.size(); fid++){
//           if (dest.test(fid) && fid != get_worker_id()){
//               messageBuffers.add_message(fid,fragment.getGlobalID(u));
//           }
//       }
//   }
//   for (auto v :dgraph.GetParentsID(u)){
//            que.emplace(v);
//   }
//   for(auto v :dgraph.GetChildrenID(u)){
////         if(fragment.isInnerVertex(fragment.getGlobalID(v))){
//            que.emplace(v);
//         }
//}
//messageBuffers.sync_messages();
//}

bool Bfs_Connectivity::is_continue(){
    continue_run = messageBuffers.exchange_message_size();
    int b=0;
    MPI_Allreduce(&continue_run, &b, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    continue_run = 0;
    if(b>0){
       return true;
    }else{
      return false;
    }
}

void Bfs_Connectivity::out_global_result(Fragment &fragment, std::unordered_set<VertexID> &nodeset){
    std::unordered_set<VertexID> tmp_set;
    for(auto v :nodeset){
        tmp_set.insert(v);
    }
    nodeset.clear();
    for(auto v : tmp_set){
        nodeset.insert(fragment.getGlobalID(v));
    }
}
