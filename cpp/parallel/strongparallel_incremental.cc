#include "strongparallel_incremental.h"

StrongparallelInc::StrongparallelInc(){
    Vertex_MessageBuffers.init();
    Edge_MessageBuffers.init();
}

StrongparallelInc::~StrongparallelInc(){}

void StrongparallelInc::set_change_num(const int num){
    this->change_num = num;
}

void StrongparallelInc::set_base_add_file(const std::string base_file){
    this->base_add_file=base_file;
}

void StrongparallelInc::set_base_remove_file(const std::string base_file){
    this->base_remove_file=base_file;
}

std::unordered_set<VertexID> StrongparallelInc::find_dhop_nodes(Fragment& fragment, Graph& dgraph,int d_Q){
        std::unordered_set<VertexID> result_nodes;
        const std::unordered_set<VertexID> *innerVertices=(fragment.getInnerVertices());
        std::unordered_set<VertexID> roots;
        for(auto v:*innerVertices){
            if (fragment.isBorderVertex(v)){
                roots.insert(v);
            }
        }
        Bfs_Multikhop bfs_multikhop;
        bfs_multikhop.bfs_multikhopparallel(fragment, dgraph, result_nodes, roots, d_Q);
        return result_nodes;
}

void StrongparallelInc::LoadEdges(std::vector<std::pair<VertexID,VertexID>> &edges, const std::string efile) {
    std::string line;
    std::ifstream e_infile(efile,std::ios::in);
    VertexID u,v;
    while(getline(e_infile,line,'\n')){
        std::stringstream ss(line);
        ss>> u >> v;
        edges.push_back(std::make_pair(u,v));
    }
}

void StrongparallelInc::Load_bunch_edges(std::vector<std::pair<VertexID,VertexID>> &edges,const std::string basefilename,int index){
    int i=1;
    while (i<=index){
         LoadEdges(edges,basefilename+std::to_string(i));
         i+=1;
     }
}

void StrongparallelInc::LoadEdges(std::set<std::pair<VertexID,VertexID>> &edges, const std::string efile) {
    std::string line;
    std::ifstream e_infile(efile,std::ios::in);
    VertexID u,v;
    while(getline(e_infile,line,'\n')){
        std::stringstream ss(line);
        ss>> u >> v;
        edges.insert(std::make_pair(u,v));
    }
}

void StrongparallelInc::Load_bunch_edges(std::set<std::pair<VertexID,VertexID>> &edges,const std::string basefilename,int index){
    int i=1;
    while (i<=index){
         LoadEdges(edges,basefilename+std::to_string(i));
         i+=1;
     }
}

std::unordered_set<VertexID> StrongparallelInc::find_affected_area(Fragment& fragment, Graph& dgraph, std::set<std::pair<VertexID,VertexID>> &add_edges,std::set<std::pair<VertexID,VertexID>> &rm_edges,int d_Q){
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
        std::unordered_set<VertexID> result_nodes;
        Bfs_Multikhop bfs_multikhop;
        bfs_multikhop.bfs_multikhopparallel(fragment, dgraph, result_nodes, incedges_node, d_Q);
        return result_nodes;
}

void StrongparallelInc::update_fragment_inc(Fragment &fragment,Graph& dgraph,
                                             std::unordered_set<VertexID> &affected_nodes,
                                             std::set<std::pair<VertexID,VertexID>> &add_edges,
                                             std::set<std::pair<VertexID,VertexID>> &rm_edges,
                                             int d_Q){
         std::unordered_set<Vertex> new_add_nodes;
         std::unordered_set<Edge> new_add_edges;
         std::unordered_set<Edge> new_rm_edges;
         const std::unordered_set<VertexID> * OuterVertices = fragment.getOuterVertices();
         const std::unordered_set<VertexID> * innerVertices = fragment.getInnerVertices();
         for(auto v :*innerVertices){
             if(affected_nodes.find(v)!=affected_nodes.end()){
                     for(int i =0;i<get_num_workers();++i){
                         if(i!=get_worker_id()){
                         Vertex_MessageBuffers.add_message(i,Vertex(v,dgraph.GetVertexLabel(fragment.getLocalID(v))));
                         }
                     }
             }
         }
         Vertex_MessageBuffers.sync_messages();
         for (auto item :Vertex_MessageBuffers.get_messages()){
             new_add_nodes.insert(item);
         }
         Vertex_MessageBuffers.reset_in_messages();

         for(auto edge: dgraph.GetAllEdges()){
                 VertexID source_id = fragment.getGlobalID(edge.src());
                 VertexID target_id = fragment.getGlobalID(edge.dst());
                 if(affected_nodes.find(source_id)!=affected_nodes.end() && affected_nodes.find(target_id)!=affected_nodes.end()){
                     for(int i =0;i<get_num_workers();++i){
                         if(i!=get_worker_id()){
                             Edge_MessageBuffers.add_message(i,Edge(source_id,target_id,edge.attr()));
                         }
                     }
                 }
         }
         Edge_MessageBuffers.sync_messages();
         for (auto item :Edge_MessageBuffers.get_messages()){
             new_add_edges.insert(item);
         }
         for(auto e :add_edges){
             if(rm_edges.find(e) == rm_edges.end()){
                 new_add_edges.insert(Edge(e.first,e.second,1));
             }
         }
         for(auto e:rm_edges){
             if(add_edges.find(e)!=add_edges.end()){
                 new_rm_edges.insert(Edge(e.first,e.second,1));
             }
         }
         Edge_MessageBuffers.reset_in_messages();
         fragment.update_fragment(dgraph,new_add_edges,new_rm_edges,new_add_nodes);
         worker_barrier();
    }

void StrongparallelInc::update_fragment_parallel(Fragment &fragment,Graph& dgraph,int d_Q){
    std::unordered_set<VertexID> affected_nodes = find_dhop_nodes(fragment,dgraph,d_Q);
    std::unordered_set<Vertex> new_add_nodes;
    std::unordered_set<Edge> new_add_edges;
    std::unordered_set<Edge> new_rm_edges;
    const std::unordered_set<VertexID> * OuterVertices = fragment.getOuterVertices();
    const std::unordered_set<VertexID> * innerVertices = fragment.getInnerVertices();
//    affected_nodes = intersection(affected_nodes,*innerVertices);
    for(auto v :*innerVertices){
         if(affected_nodes.find(v)!=affected_nodes.end()){
             for(int i =0;i<get_num_workers();++i){
                 if(i!=get_worker_id()){
                     Vertex_MessageBuffers.add_message(i,Vertex(v,dgraph.GetVertexLabel(fragment.getLocalID(v))));
                 }
             }
         }
    }
    Vertex_MessageBuffers.sync_messages();

    for (auto item :Vertex_MessageBuffers.get_messages()){
        new_add_nodes.insert(item);
    }
    Vertex_MessageBuffers.reset_in_messages();
    for(auto edge: dgraph.GetAllEdges()){
         VertexID source_id = fragment.getGlobalID(edge.src());
         VertexID target_id = fragment.getGlobalID(edge.dst());
         if(affected_nodes.find(source_id)!=affected_nodes.end() && affected_nodes.find(target_id)!=affected_nodes.end()){
             for(int i =0;i<get_num_workers();++i){
                 if(i!=get_worker_id())
                     Edge_MessageBuffers.add_message(i,Edge(source_id,target_id,edge.attr()));
             }
         }
    }
    Edge_MessageBuffers.sync_messages();
    for (auto item :Edge_MessageBuffers.get_messages()){
        new_add_edges.insert(item);
    }
    Edge_MessageBuffers.reset_in_messages();
    fragment.update_fragment(dgraph,new_add_edges,new_rm_edges,new_add_nodes);
    worker_barrier();
}

int StrongparallelInc::cal_diameter_qgraph(Graph &qgraph){
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

bool StrongparallelInc::valid_sim_w(Graph &qgraph,std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim,VertexID w){
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

void StrongparallelInc::find_node_connectivity_nodes(Ball_View &ball_view,std::unordered_set<VertexID> &v_set,VertexID w){
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

void StrongparallelInc::rename_sim(Ball_View &ball_view,Graph &qgraph,
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

void StrongparallelInc::ss_counter_initialization(Ball_View &ball_view,Graph &qgraph,
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

void StrongparallelInc::dual_filter_match(Ball_View &refined_ball, Graph &qgraph,
                      std::unordered_map<VertexID, std::unordered_set<VertexID>> &S_w,VertexID w,int d_Q){
        std::set<std::pair<VertexID,VertexID> > filter_set;
        std::unordered_map<VertexID, std::vector<int> > sim_counter_pre,sim_counter_post;
        ss_counter_initialization(refined_ball,qgraph, sim_counter_pre,sim_counter_post,S_w);
        push_phase(refined_ball,qgraph,w,d_Q, filter_set, sim_counter_pre, sim_counter_post,S_w);
        decremental_refine(refined_ball,qgraph, filter_set,sim_counter_pre,sim_counter_post,S_w);
   }

void StrongparallelInc::push_phase(Ball_View &ball,Graph &qgraph,VertexID w,int d_Q,
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


void StrongparallelInc::update_counter(Ball_View &ball,Graph &qgraph,VertexID u,VertexID v,
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

void StrongparallelInc::decremental_refine(Ball_View &ball_view,Graph &qgraph,
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

void StrongparallelInc::extract_max_pg(Ball_View &ball_view,Graph &dgraph,Graph &qgraph,VertexID w,
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



void StrongparallelInc::print_ball_info(Graph &qgraph,std::unordered_map<VertexID, std::unordered_set<VertexID>> &S_w,VertexID w){

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

void StrongparallelInc::out_global_result(Fragment &fragment,  Graph &qgraph, std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim){
    std::unordered_map<VertexID, std::unordered_set<VertexID>> tmp_sim;
    for(auto u :qgraph.GetAllVerticesID()){
        tmp_sim[u] = std::unordered_set<VertexID>();
        for(auto v :sim[u]){
            tmp_sim[u].insert(v);
        }
    }
    for(auto u : qgraph.GetAllVerticesID()){
        sim[u].clear();
        for(auto v :tmp_sim[u]){
            sim[u].insert(fragment.getGlobalID(v));
        }
    }
 }

void  StrongparallelInc::recalculate_incrementl_dual(Graph &dgraph, Graph &qgraph,
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

std::vector<StrongR>  StrongparallelInc::strong_parallel(Fragment &fragment,Graph &dgraph, Graph &qgraph){
    std::vector<StrongR> max_result;
    int d_Q = cal_diameter_qgraph(qgraph);
//    std::cout<<dgraph.GetNumVertices()<<' '<<dgraph.GetNumEdges()<<endl;
    worker_barrier();
    update_fragment_parallel(fragment,dgraph,d_Q);
//    std::cout<<dgraph.GetNumVertices()<<' '<<dgraph.GetNumEdges()<<endl;
    std::unordered_map<VertexID, std::unordered_set<VertexID>> fragment_sim;
    /**
      *calculate dual simulation for dgraph
      */
    DualSim dualsim;
    bool inital_sim =false;
    dualsim.dual_simulation(dgraph,qgraph,fragment_sim,inital_sim);
    const std::unordered_set<VertexID> * innerVertices = fragment.getInnerVertices();
    for (auto w : dgraph.GetAllVerticesID()) {
    /**
      * calculate ball for center w if w if a valid center
      */
        if ((*innerVertices).find(fragment.getGlobalID(w))!=(*innerVertices).end() && valid_sim_w(qgraph,fragment_sim,w)){
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
            for (auto v : fragment_sim[u]){
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
        out_global_result(fragment,qgraph,S_w);
        max_result.emplace_back(fragment.getGlobalID(w),S_w);
        }
    }
    worker_barrier();
    return max_result;
}

std::vector<StrongR>  StrongparallelInc::strong_parallel_inc(Fragment &fragment,Graph &dgraph, Graph &qgraph,
                                                      std::unordered_map<VertexID,std::unordered_set<VertexID>> &global_dsim,
                                                      std::vector<StrongR> &global_strong_r,
                                                      std::set<std::pair<VertexID,VertexID>> &add_edges,
                                                      std::set<std::pair<VertexID,VertexID>> &rm_edges){
    std::vector<StrongR> max_result;
    int d_Q = cal_diameter_qgraph(qgraph);
    std::unordered_set<VertexID> affected_nodes = find_affected_area(fragment,dgraph,add_edges,rm_edges,d_Q);
    update_fragment_inc(fragment,dgraph,affected_nodes, add_edges, rm_edges, d_Q);
    std::unordered_map<VertexID, std::unordered_set<VertexID>> fragment_sim;
    /**
      *load local fragment sim
      */
    for(auto u :qgraph.GetAllVerticesID()){
        fragment_sim[u] = std::unordered_set<VertexID>();
        for(auto v: global_dsim[u]){
            if(fragment.has_vertex(v)){
                fragment_sim[u].insert(fragment.getLocalID(v));
            }
        }
    }
    recalculate_incrementl_dual(dgraph,qgraph,fragment_sim,add_edges,rm_edges);
    const std::unordered_set<VertexID> * innerVertices = fragment.getInnerVertices();
    for (auto w : dgraph.GetAllVerticesID()) {
    /**
      * calculate ball for center w if w if a valid center
      */
        if ((*innerVertices).find(fragment.getGlobalID(w))!=(*innerVertices).end() && valid_sim_w(qgraph,fragment_sim,w)){
        if( affected_nodes.find(fragment.getGlobalID(w)) == affected_nodes.end()){
            for(auto ballr:global_strong_r){
                if(fragment.getLocalID(ballr.center()) == w){
                    max_result.push_back(ballr);
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
            for (auto v : fragment_sim[u]){
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
        out_global_result(fragment,qgraph,S_w);
        max_result.emplace_back(fragment.getGlobalID(w),S_w);
        }
    }
    worker_barrier();
    return max_result;
}