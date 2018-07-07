#include "cpp/strong_incremental.h"
StrongInc::StrongInc(){}

StrongInc::~StrongInc(){}

void StrongInc::find_affected_center_area(Graph &dgraph,std::set<std::pair<VertexID,VertexID>> &add_edges,
                                                 std::set<std::pair<VertexID,VertexID>> &rm_edges,
                                                 int d_hop,
                                                 std::unordered_set<VertexID> &result){
    std::unordered_set<VertexID> incedges_node;
    for(auto e:add_edges){
        if(rm_edges.find(e) == rm_edges.end()){
        incedges_node.insert(e.first);
        incedges_node.insert(e.second);
        }
    }
    for(auto e:rm_edges){
        if(add_edges.find(e) == add_edges.end()){
         incedges_node.insert(e.first);
        incedges_node.insert(e.second);
        }
    }
    dgraph.find_hop_nodes(incedges_node,d_hop,result);
}

int StrongInc::cal_diameter_qgraph(Graph &qgraph){
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


bool StrongInc::valid_sim_w(Graph &qgraph,std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim,VertexID w){
          for(auto u : qgraph.GetAllVerticesID()){
              if(sim[u].size()==0){
                  return false;
              }
           }
           int uid = -1;
           for(auto u : qgraph.GetAllVerticesID()){
               if (sim[u].find(w) != sim[u].end()){
                   uid = u;
                   return true;
               }
           }
           if (uid == -1){
           return false;
           }
      }


void StrongInc::find_node_connectivity_nodes(Ball_View &ball_view,std::unordered_set<VertexID> &v_set,VertexID w){
    std::queue<VertexID> q;
    v_set.clear();
    v_set.insert(w);
    q.push(w);
    while(!q.empty()){
        VertexID root = q.front();
        q.pop();
        for(auto v :ball_view.GetParentsID(root)){
            if(v_set.find(v) == v_set.end()){
                v_set.insert(v);
                q.push(v);
            }
        }
        for(auto v :ball_view.GetChildrenID(root)){
            if(v_set.find(v) == v_set.end()){
                v_set.insert(v);
                q.push(v);
            }
        }
    }
}

void StrongInc::rename_sim(Ball_View &ball_view,Graph &qgraph,
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

void StrongInc::ss_counter_initialization(Ball_View &ball_view,Graph &qgraph,
                                     std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre,
                                     std::unordered_map<VertexID, std::vector<int>> &sim_counter_post,
                                     std::unordered_map<VertexID, std::unordered_set<VertexID>> &S_w){
        for (auto w : ball_view.GetAllVerticesID()){
            sim_counter_post[w] = std::vector<int>(qgraph.GetNumVertices(), 0);
            sim_counter_pre[w] = std::vector<int>(qgraph.GetNumVertices(), 0);
            for (auto u : qgraph.GetAllVerticesID()){
                int len_des=0,len_pre=0;
                for (auto des_w : ball_view.GetChildrenID(w)){
                     if(S_w[u].find(des_w) != S_w[u].end()){
                        len_des += 1;
                     }
                }
                for (auto pre_w : ball_view.GetParentsID(w)){
                    if (S_w[u].find(pre_w) != S_w[u].end()){
                        len_pre+=1;
                    }
                }
                sim_counter_post[w][u] = len_des;
                sim_counter_pre[w][u] = len_pre;
            }
        }
 }

 void StrongInc::dual_filter_match(Ball_View &refined_ball, Graph &qgraph,
                      std::unordered_map<VertexID, std::unordered_set<VertexID>> &S_w,VertexID w,int d_Q){
        std::set<std::pair<VertexID,VertexID> > filter_set;
        std::unordered_map<VertexID, std::vector<int> > sim_counter_pre,sim_counter_post;
        ss_counter_initialization(refined_ball,qgraph, sim_counter_pre,sim_counter_post,S_w);
        push_phase(refined_ball,qgraph,w,d_Q, filter_set, sim_counter_pre, sim_counter_post,S_w);
        decremental_refine(refined_ball,qgraph, filter_set,sim_counter_pre,sim_counter_post,S_w);
   }

void StrongInc::push_phase(Ball_View &ball,Graph &qgraph,VertexID w,int d_Q,
                          std::set<std::pair<VertexID,VertexID>> &filter_set,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_post,
                          std::unordered_map<VertexID, std::unordered_set<VertexID>> &S_w){

         for (auto u :qgraph.GetAllVerticesID()){
            for (auto v : S_w[u]){
                for (auto u_s : qgraph.GetChildrenID(u)){
                    if (sim_counter_post[v][u_s]==0){
                       filter_set.insert(std::pair<VertexID,VertexID>(u,v));
                        break;
                    }
                }
                for(auto u_p : qgraph.GetParentsID(u)){
                    if(sim_counter_pre[v][u_p]==0){
                        filter_set.insert(std::pair<VertexID,VertexID>(u,v));
                       break;
                    }
                }
            }
        }
 }


 void StrongInc::update_counter(Ball_View &ball,Graph &qgraph,VertexID u,VertexID v,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_post){
        for(auto vp : ball.GetParentsID(v)){
            if (sim_counter_post.find(vp)!=sim_counter_post.end()){
                if(sim_counter_post[vp][u]>0){
                    sim_counter_post[vp][u]-=1;
                }
            }
        }
        for (auto vs : ball.GetChildrenID(v)){
            if (sim_counter_pre.find(vs)!=sim_counter_pre.end()){
                if(sim_counter_pre[vs][u]>0){
                    sim_counter_pre[vs][u]-=1;
                }
            }
        }
    }

void StrongInc::decremental_refine(Ball_View &ball_view,Graph &qgraph,
                          std::set<std::pair<VertexID,VertexID>> &filter_set,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_post,
                          std::unordered_map<VertexID, std::unordered_set<VertexID>> &S_w){
        while(!filter_set.empty()){
            std::pair<VertexID,VertexID> pmatch = *filter_set.begin();
            VertexID u = pmatch.first;
            VertexID v = pmatch.second;
            filter_set.erase(filter_set.begin());
            S_w[u].erase(v);
            update_counter(ball_view,qgraph,u,v,sim_counter_pre,sim_counter_post);
            for (auto u_p : qgraph.GetParentsID(u)){
                for (auto v_p : ball_view.GetParentsID(v)){
                    if (S_w[u_p].find(v_p)!=S_w[u_p].end()){
                        if(sim_counter_post[v_p][u]==0){
                            filter_set.insert(std::pair<VertexID,VertexID>(u_p,v_p));
                        }
                    }
                }
            }
            for(auto u_s :qgraph.GetChildrenID(u)){
                for(auto v_s: ball_view.GetChildrenID(v)){
                    if (S_w[u_s].find(v_s) != S_w[u_s].end()){
                        if(sim_counter_pre[v_s][u] == 0){
                            filter_set.insert(std::pair<VertexID,VertexID>(u_s,v_s));
                        }
                    }
                }

            }
        }
    }

void StrongInc::extract_max_pg(Ball_View &ball_view,Graph &dgraph,Graph &qgraph,VertexID w,
                                std::unordered_map<VertexID, std::unordered_set<VertexID>> &S_w){
    if(!valid_sim_w(qgraph,S_w,w)){
        for (auto u : qgraph.GetAllVerticesID()){
            S_w[u].clear();
        }
    }

    std::unordered_set<VertexID> vertex_match_set;
    for (auto u : qgraph.GetAllVerticesID()){
            for(auto v: S_w[u]){
             vertex_match_set.insert(v);
            }
     }

    std::unordered_set<Edge> edge_match_set;
    for(auto e: qgraph.GetAllEdges()){
        VertexID sourceid=e.src();
        VertexID targetid=e.dst();
        for (auto sim_v1 : S_w[sourceid]){
            for(auto sim_v2 : S_w[targetid]){
                 if (ball_view.ExistEdge(sim_v1,sim_v2)){
                     edge_match_set.insert(Edge(sim_v1,sim_v2,1));
                 }

             }
        }
   }
   Ball_View pg_view(vertex_match_set,edge_match_set);
   std::unordered_set<VertexID> vertex_match_set1;
   find_node_connectivity_nodes(pg_view,vertex_match_set1,w);


   for(auto u : qgraph.GetAllVerticesID()){
       std::unordered_set<VertexID> tmp_set;
       for(auto v:vertex_match_set1){
           if (S_w[u].find(v) != S_w[u].end()){
             tmp_set.insert(v);
           }
       }
       S_w[u].clear();
       S_w[u]=tmp_set;
       }
   }



void StrongInc::print_ball_info(Graph &qgraph,std::unordered_map<VertexID, std::unordered_set<VertexID>> &S_w,VertexID w){

              std::unordered_map<VertexID,std::set<VertexID>> printset;
              for(auto u :qgraph.GetAllVerticesID()){
                  printset[u]=std::set<VertexID>();
                  for(auto v:S_w[u]){
                      printset[u].insert(v);
                  }

              }
                         for(auto u :qgraph.GetAllVerticesID()){
               std::cout<<u;
               for(auto v:printset[u]){
                   std::cout<<' '<<v;
               }
              std::cout<<std::endl;
              }
}

void  StrongInc::recalculate_incrementl_dual(Graph &dgraph, Graph &qgraph,
                                      std::unordered_map<VertexID,std::unordered_set<VertexID>> &dsim,
                                      std::set<std::pair<VertexID,VertexID>> &add_edges,
                                      std::set<std::pair<VertexID,VertexID>> &rm_edges){
          DualInc dualinc;
          for (auto e:add_edges){
             dgraph.AddEdge(Edge(e.first,e.second,1));
          }
          dualinc.incremental_addedges(dgraph,qgraph,dsim,add_edges);
          for(auto e :rm_edges){
              dgraph.RemoveEdge(Edge(e.first,e.second,1));
          }
          dualinc.incremental_removeedgs(dgraph,qgraph,dsim,rm_edges);
    }

//std::vector<StrongR>
  void StrongInc::strong_simulation_inc(Graph &dgraph, Graph &qgraph,
                                      std::unordered_map<VertexID,std::unordered_set<VertexID>> &dsim,
                                      std::vector<StrongR> &strong_r,
                                      std::set<std::pair<VertexID,VertexID>> &add_edges,
                                      std::set<std::pair<VertexID,VertexID>> &rm_edges){
          /**
           *calculate qgaraph diameter
          */
          std::vector<StrongR> max_result;
          int d_Q = cal_diameter_qgraph(qgraph);
          std::unordered_map<VertexID, std::unordered_set<VertexID>> global_sim;
          recalculate_incrementl_dual(dgraph,qgraph,dsim,add_edges,rm_edges);

          std::unordered_set<VertexID> affected_center_nodes;
          find_affected_center_area(dgraph,add_edges,rm_edges,d_Q,affected_center_nodes);

          std::unordered_set<VertexID> max_dual_set;
          for(auto u:qgraph.GetAllVerticesID()){
              for(auto v :dsim[u]){
                  max_dual_set.insert(v);
              }
          }
          affected_center_nodes = intersection(affected_center_nodes,max_dual_set);

          int i=0;
           clock_t stime,etime;
             stime =clock();
          for (auto w : max_dual_set) {
              /**
               * calculate ball for center w if w if a valid center
               */
//              if (valid_sim_w(qgraph,dsim,w)){
              if (affected_center_nodes.find(w) == affected_center_nodes.end()){
                 for(auto strong_ball:strong_r){
                     if(strong_ball.center()==w){
                          max_result.push_back(strong_ball);
                     }
                 }

                  continue;
              }
               clock_t start,end;
               start =clock();
              /**
               *find d_hop_nodes for w in dgraph
               */
              std::unordered_set<VertexID> ball_node;
              dgraph.find_hop_nodes(w,d_Q,ball_node);

              std::unordered_set<VertexID> ball_filter_node;
              std::unordered_set<Edge> ball_filter_edge;
              std::unordered_map<VertexID, std::unordered_set<VertexID>> S_w;
              for(auto u : qgraph.GetAllVerticesID()){
                  for (auto v : dsim[u]){
                      if(ball_node.find(v) != ball_node.end()){
                          S_w[u].insert(v);
                          ball_filter_node.insert(v);
                      }
                  }
              }
              for(auto e: qgraph.GetAllEdges()){
                  VertexID sourceid=e.src();
                  VertexID targetid=e.dst();
                  for (auto sim_v1 : S_w[sourceid]){
                      for(auto sim_v2 : S_w[targetid]){
                          if (dgraph.ExistEdge(sim_v1,sim_v2)){
                              ball_filter_edge.insert(Edge(sim_v1,sim_v2,1));
                          }
                      }
                  }
              }

              Ball_View ball_view(ball_filter_node,ball_filter_edge);

              std::unordered_set<VertexID> refined_ball_vertex;
              std::unordered_set<Edge> refinded_ball_edge;
              find_node_connectivity_nodes(ball_view,refined_ball_vertex,w);
              for(auto e :ball_filter_edge){
                   if(refined_ball_vertex.find(e.src()) != refined_ball_vertex.end() && refined_ball_vertex.find(e.dst())!=refined_ball_vertex.end()){
                       refinded_ball_edge.insert(e);
                   }
              }
              Ball_View refined_ball_view(refined_ball_vertex,refinded_ball_edge);
              rename_sim(refined_ball_view,qgraph,S_w);
              dual_filter_match(refined_ball_view, qgraph,S_w,w,d_Q);

              extract_max_pg(refined_ball_view,dgraph,qgraph, w,S_w);

              max_result.emplace_back(w,S_w);
//              print_ball_info(qgraph,S_w,w);
//              break;
             // std::cout<<"calculate one ball time "<<(float)(end-start)/CLOCKS_PER_SEC<<"s"<<std::endl;
//              }
          }
          etime=clock();
          strong_r.clear();
          for(auto strong_ball :max_result){
              strong_r.push_back(strong_ball);
          }
         // std::cout<<"inc strong "<< (float)(etime-stime)/CLOCKS_PER_SEC<<std::endl;
//          return max_result;
      }