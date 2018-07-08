#include "bfs_singlekhop.h"
Bfs_Singlekhop::Bfs_Singlekhop(){
    messageBuffers.init();
}

Bfs_Singlekhop::~Bfs_Singlekhop(){}

void Bfs_Singlekhop::bfs_singlekhopparallel(Fragment &fragment, Graph &dgraph,std::unordered_set<VertexID> &nodeset, VertexID root, int bound){
std::unordered_map<VertexID,int> DistInf;
worker_barrier();
pEval(fragment, dgraph, nodeset, DistInf, root, bound);
worker_barrier();
while (is_continue()){
    incEval(fragment, dgraph, nodeset, DistInf, root, bound);
    worker_barrier();
}
worker_barrier();
out_global_result(fragment,nodeset);
//assemble_result(nodeset);
}


void Bfs_Singlekhop::pEval(Fragment &fragment, Graph &dgraph,std::unordered_set<VertexID> &nodeset, std::unordered_map<VertexID,int> &DistInf, VertexID root, int bound){
std::queue<std::pair<VertexID,int>> que;
if (fragment.isInnerVertex(root)){
    que.emplace(fragment.getLocalID(root), bound);
 }
while (!que.empty()){
   VertexID u = que.front().first;
   int distance = que.front().second;
   que.pop();
   if (nodeset.find(u) != nodeset.end()){
       continue;
   }
   nodeset.insert(u);
   DistInf[fragment.getGlobalID(u)] = bound - distance;
   if (fragment.isBorderVertex(fragment.getGlobalID(u))){
        // std::cout<<"num_fragments "<<NUM_FRAGMENTS<<std::endl;
       std::bitset<NUM_FRAGMENTS> dest= fragment.getMsgThroughDest(fragment.getGlobalID(u));
       for (int fid = 0; fid < dest.size(); fid++){
           if (dest.test(fid) && fid != get_worker_id()){
               messageBuffers.add_message(fid,std::pair<VertexID,int>(fragment.getGlobalID(u), distance));
           }
       }
   }
   if (distance >0){
       for (auto v :dgraph.GetParentsID(u)){
            que.emplace(v,distance-1);
       }
       for(auto v :dgraph.GetChildrenID(u)){
           if(fragment.isInnerVertex(fragment.getGlobalID(v))){
              que.emplace(v,distance-1);
           }
       }
   }

}
  messageBuffers.sync_messages();
}

void Bfs_Singlekhop::incEval(Fragment &fragment, Graph &dgraph,std::unordered_set<VertexID> &nodeset, std::unordered_map<VertexID,int> &DistInf, VertexID root, int bound){
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
       if(bound - distance>DistInf[fragment.getGlobalID(u)]){
           continue;
       }
   }
   nodeset.insert(u);
   DistInf[fragment.getGlobalID(u)] = bound - distance;;
   if (fragment.isBorderVertex(fragment.getGlobalID(u))){
       std::bitset<NUM_FRAGMENTS> dest = fragment.getMsgThroughDest(fragment.getGlobalID(u));
       for (int fid = 0; fid < dest.size(); fid++){
           if (dest.test(fid) && fid != get_worker_id()){
               messageBuffers.add_message(fid,std::pair<VertexID,int>(fragment.getGlobalID(u), distance));
           }
       }
   }
   if (distance >0){
       for (auto v :dgraph.GetParentsID(u)){
            que.emplace(v,distance-1);
       }
       for(auto v :dgraph.GetChildrenID(u)){
           if(fragment.isInnerVertex(fragment.getGlobalID(v))){
              que.emplace(v,distance-1);
           }
       }
   }
}
messageBuffers.sync_messages();
}

bool Bfs_Singlekhop::is_continue(){
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


void Bfs_Singlekhop::out_global_result(Fragment &fragment, std::unordered_set<VertexID> &nodeset){
    std::unordered_set<VertexID> tmp_set;
    for(auto v :nodeset){
        tmp_set.insert(v);
    }
    nodeset.clear();
    for(auto v : tmp_set){
        nodeset.insert(fragment.getGlobalID(v));
    }
}

std::unordered_set<VertexID> Bfs_Singlekhop::assemble_result(std::unordered_set<VertexID> &nodeset){
    int fid = get_worker_id();
    std::unordered_set<VertexID> global_node;
    if(fid ==0){
        std::vector<std::unordered_set<VertexID>>  tmp_vec(get_num_workers());
        tmp_vec[fid] = nodeset;
        masterGather(tmp_vec);
        for(int i =0; i< get_num_workers(); ++i){
            for(auto v :tmp_vec[i]){
                global_node.insert(v);
            }
        }
        worker_barrier();
    }else{
       slaveGather(nodeset);
       worker_barrier();
    }
}