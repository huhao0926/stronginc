#include "bfs_parallel.h"
Bfs_Parallel::Bfs_Parallel(){
    messageBuffers.init();
}

Bfs_Parallel::~Bfs_Parallel(){}

void Bfs_Parallel::bfs_parallel(Fragment &fragment, Graph &dgraph, std::unordered_map<VertexID,int> &DistInf, VertexID root){
std::unordered_set<VertexID> nodeset;
worker_barrier();
pEval(fragment, dgraph, nodeset, DistInf, root);
worker_barrier();
while (is_continue()){
    incEval(fragment, dgraph, nodeset, DistInf, root);
    worker_barrier();
}
for (auto v :dgraph.GetAllVerticesID()){
    if (nodeset.find(v) == nodeset.end()){
        DistInf[fragment.getGlobalID(v)] = INT_MAX;
    }
}
}

void Bfs_Parallel::pEval(Fragment &fragment, Graph &dgraph,std::unordered_set<VertexID> &nodeset, std::unordered_map<VertexID,int> &DistInf, VertexID root){
std::queue<std::pair<VertexID,int>> que;
if (fragment.isInnerVertex(root)){
    que.emplace(fragment.getLocalID(root),0);
 }
while (!que.empty()){
   VertexID u = que.front().first;
   int distance = que.front().second;
   que.pop();
   if (nodeset.find(u) != nodeset.end()){
       continue;
   }
   nodeset.insert(u);
   DistInf[fragment.getGlobalID(u)] = distance;
   if (fragment.isBorderVertex(fragment.getGlobalID(u))){
        // std::cout<<"num_fragments "<<NUM_FRAGMENTS<<std::endl;
       std::bitset<NUM_FRAGMENTS> dest= fragment.getMsgThroughDest(fragment.getGlobalID(u));
       for (int fid = 0; fid < dest.size(); fid++){
           if (dest.test(fid) && fid != get_worker_id()){
               messageBuffers.add_message(fid,std::pair<VertexID,int>(fragment.getGlobalID(u), distance));
           }
       }
   }
   for (auto v :dgraph.GetParentsID(u)){
            que.emplace(v,distance+1);
   }
   for(auto v :dgraph.GetChildrenID(u)){
         if(fragment.isInnerVertex(fragment.getGlobalID(v))){
            que.emplace(v,distance+1);
         }
   }
}
  messageBuffers.sync_messages();
}

void Bfs_Parallel::incEval(Fragment &fragment, Graph &dgraph,std::unordered_set<VertexID> &nodeset, std::unordered_map<VertexID,int> &DistInf, VertexID root){
std::queue<std::pair<VertexID,int>> que;
for (auto item :messageBuffers.get_messages()){
    VertexID u = item.first;
    int distance = item.second;
    que.emplace(fragment.getLocalID(u),distance);
}
messageBuffers.reset_in_messages();
while(!que.empty()){
   VertexID u = que.front().first;
   int distance = que.front().second;
   que.pop();
   if (nodeset.find(u) != nodeset.end()){
       if(distance>DistInf[fragment.getGlobalID(u)]){
           continue;
       }
   }
   nodeset.insert(u);
   DistInf[fragment.getGlobalID(u)] = distance;
   if (fragment.isBorderVertex(fragment.getGlobalID(u))){
       std::bitset<NUM_FRAGMENTS> dest = fragment.getMsgThroughDest(fragment.getGlobalID(u));
       for (int fid = 0; fid < dest.size(); fid++){
           if (dest.test(fid) && fid != get_worker_id()){
               messageBuffers.add_message(fid,std::pair<VertexID,int>(fragment.getGlobalID(u), distance));
           }
       }
   }
   for (auto v :dgraph.GetParentsID(u)){
            que.emplace(v,distance+1);
   }
   for(auto v :dgraph.GetChildrenID(u)){
//         if(fragment.isInnerVertex(fragment.getGlobalID(v))){
            que.emplace(v,distance+1);
         }
}
messageBuffers.sync_messages();
}

bool Bfs_Parallel::is_continue(){
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
