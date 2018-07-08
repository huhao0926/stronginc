#include "strongsim_parallel.h"

Strong_Parallel::Strong_Parallel(){messageBuffers.init();}

Strong_Parallel::~Strong_Parallel(){}

int Strong_Parallel::cal_diameter_qgraph(Graph &qgraph){
          int temp_dia = 0;
          int max_dia = qgraph.GetNumVertices()-1;
          for(auto u : qgraph.GetAllVerticesID()){
              std::unordered_map<VertexID,int> dis;
              qgraph.shortest_distance(u,dis);
              for (int i=0; i<qgraph.GetNumVertices(); i++){
                if (dis[i] <= max_dia && temp_dia < dis[i]){
                    temp_dia = dis[i];
                }
              }
          }
          return temp_dia;
      }

void Strong_Parallel::rename_sim(GraphView &ball_view,Graph &qgraph,
                               std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim){
              //std::cout<<w<<std::endl;
       for(auto u : qgraph.GetAllVerticesID()){
           std::unordered_set<VertexID> tmp_set;
           for(auto v:ball_view.GetAllVerticesID()){
               if (sim[u].find(v) != sim[u].end()){
                 tmp_set.insert(v);
               }
           }
           sim[u].clear();
           sim[u]=tmp_set;
       }
     }

void Strong_Parallel::broad_all_dual_node(Fragment &fragment,Graph &qgraph,
                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim,
                           std::unordered_set<VertexID> &center_node){
   //std::unordered_set<VertexID> local_sim_node;
   for(auto u:qgraph.GetAllVerticesID()){
       for(auto v:sim[u]){
           //local_sim_node.insert(fragment.getGlobalID(v));
           for(int fid=0; fid<get_num_workers(); ++fid){
              messageBuffers.add_message(fid,fragment.getGlobalID(v));
           }
       }
   }
   messageBuffers.sync_messages();
   for (auto item : messageBuffers.get_messages()){
        center_node.insert(item);
   }
   messageBuffers.reset_in_messages();
 }


void Strong_Parallel::get_ball_nodes(Fragment &fragment, Graph &dgraph, std::unordered_set<VertexID> &ball_node, int w,int d_Q){
      Bfs_Singlekhop bfs_singlekhop;
      bfs_singlekhop.bfs_singlekhopparallel(fragment,dgraph,ball_node,w,d_Q);
}


void Strong_Parallel::connectivity_prune(Fragment fragment,Graph &dgraph,Graph &qgraph,std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim,int w,
                            std::unordered_map<VertexID, std::unordered_set<VertexID>> &S_w,
                            std::unordered_set<VertexID> &ball_node,
                            std::unordered_set<VertexID> &prune_node){
        std::unordered_set<VertexID> vertexset;
        for(auto u : qgraph.GetAllVerticesID()){
             for (auto v : sim[u]){
                 if(ball_node.find(v) != ball_node.end()){
                     S_w[u].insert(v);
                     vertexset.insert(v);
                 }
             }
         }

        std::unordered_set<Edge> edge_match_set;
        for(auto e: qgraph.GetAllEdges()){
            VertexID sourceid=e.src();
            VertexID targetid=e.dst();
            for (auto sim_v1 : S_w[sourceid]){
                for(auto sim_v2 : S_w[targetid]){
                    if (dgraph.ExistEdge(sim_v1,sim_v2)){
                        edge_match_set.insert(Edge(sim_v1,sim_v2,1));
                    }
                }
            }
        }

        GraphView ball_view(dgraph, &edge_match_set, &vertexset);
        Bfs_Connectivity bfs_connectivity;
        bfs_connectivity.bfs_parallel(fragment,ball_view,prune_node,w);
  }

void Strong_Parallel::print_ball_info(Fragment &fragment, Graph &qgraph, std::unordered_map<VertexID, std::unordered_set<VertexID>> &S_w){
      int fid = get_worker_id();
      std::unordered_map<VertexID, std::unordered_set<VertexID>>  ballsim;
      for(auto u :qgraph.GetAllVerticesID()){
          ballsim[u] = std::unordered_set<VertexID>();
      }
      if (fid==0){
            std::vector<std::unordered_map<VertexID, std::unordered_set<VertexID>>>  tmp_vec(get_num_workers());
            tmp_vec[fid] = S_w;
            masterGather(tmp_vec);
            for(int i=0;i<get_num_workers();i++){
                for(auto u :qgraph.GetAllVerticesID()){
                     for(auto v : tmp_vec[i][u])
                     {
                        ballsim[u].insert(v);
                     }
                }
            }
            worker_barrier();
           // std::cout<<"index: "<<index<<' '<<dual_the_same(qgraph,sim,ballsim)<<std::endl;
           for(auto u :qgraph.GetAllVerticesID()){
               std::cout<<u;
               for(auto v : ballsim[u]){
                   std::cout<<' '<<v;
               }
               std::cout<<endl;
           }

      }else{
         slaveGather(S_w);
         worker_barrier();
      }
//     return ballsim;
}


void Strong_Parallel::out_global_result(Fragment &fragment, Graph &qgraph, std::unordered_map<VertexID, std::unordered_set<VertexID>> &S_w){
    std::unordered_map<VertexID, std::unordered_set<VertexID>> tmp_sim;
    for(auto u :qgraph.GetAllVerticesID()){
        tmp_sim[u] = std::unordered_set<VertexID>();
        for(auto v :S_w[u]){
            tmp_sim[u].insert(v);
        }
    }
    for(auto u : qgraph.GetAllVerticesID()){
        S_w[u].clear();
        for(auto v :tmp_sim[u]){
            S_w[u].insert(fragment.getGlobalID(v));
        }
    }

}

void Strong_Parallel::strongsim_parallel(Fragment &fragment, Graph &dgraph,Graph &qgraph){
    int d_Q = cal_diameter_qgraph(qgraph);
    Dual_Parallel dualparallel;
    std::unordered_map<VertexID, std::unordered_set<VertexID>> sim;
    dualparallel.dual_paraller(fragment,dgraph,qgraph,sim);
    worker_barrier();
    std::unordered_set<VertexID> center_node;
    broad_all_dual_node(fragment, qgraph, sim, center_node);
    for(auto w : center_node){
        worker_barrier();
        std::unordered_set<VertexID> ball_node,prune_node;
        get_ball_nodes(fragment,dgraph,ball_node,w,d_Q);
        std::unordered_map<VertexID, std::unordered_set<VertexID>> S_w;
        connectivity_prune(fragment,dgraph,qgraph,sim, w,S_w,ball_node,prune_node);
        GraphView refined_ball_view(dgraph,&prune_node);

        rename_sim(refined_ball_view,qgraph,S_w);
        Dual_Incfilter dual_incfilter;
        dual_incfilter.dual_incfilter(fragment,refined_ball_view,qgraph,S_w);
        print_ball_info(fragment,qgraph,S_w);

    }
}