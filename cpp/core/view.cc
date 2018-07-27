#include "view.h"

View::View(){}

View::~View(){}

void View::add_ViewGraph(Graph* view){
    ViewGraph_list.push_back(view);
}


std::vector<Graph*> View::get_ViewGraph_list(){
    return ViewGraph_list;
}

void View::traverse_ViewGraph(){
    for(int i=0;i<ViewGraph_list.size();i++){
        cout<<"view "<<i+1<<endl;
        for(auto v:ViewGraph_list[i]->GetAllVerticesID()){
            std::cout<<"vertex: "<<v<<' '<<ViewGraph_list[i]->GetVertexLabel(v)<<' '<<std::endl;
        }
        for(auto e:ViewGraph_list[i]->GetAllEdges()){
           std::cout<<"edge: "<<e.src()<<' '<<e.dst()<<std::endl;
        }
    }
}

bool View::vector_find(std::vector<int> &nums,int target){
    for(auto &num:nums){
        if(num==target){
            return true;
        }
    }
    return false;
}

std::unordered_map<Edge,std::unordered_set<Edge>> View::simTran(Graph &dgraph,Graph &qgraph, std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim){
    std::unordered_map<Edge,std::unordered_set<Edge>> edge2edge;
    for(auto e:qgraph.GetAllEdges()){
        std::unordered_set<Edge> eSet;
        VertexID source = e.src();
        VertexID target = e.dst();
        std::unordered_set<VertexID> simsource = sim[source];
        std::unordered_set<VertexID> simtarget = sim[target];
        if(simsource.size()==0 || simtarget.size() == 0){
            eSet.clear();
            edge2edge.clear();
        }
        for(auto source_node : simsource){
            for(auto target_node : simtarget){
                if(dgraph.ExistEdge(source_node,target_node)){
                    eSet.insert(Edge(source_node,target_node,1));
                }
            }
        }
        edge2edge[e]=eSet;
    }
    return edge2edge;
}

bool View::containCheck(Graph &qgraph){
    DualSim dualsim;
    bool ans = false;
    std::unordered_set<Edge> eSet;
    int counter = ViewGraph_list.size();
    for(int i=0;i<counter;++i){
        std::unordered_map<VertexID, std::unordered_set<VertexID>>  sim;
        bool initialized_sim = false;
        dualsim.dual_simulation(qgraph,*ViewGraph_list[i],sim,initialized_sim);
        std::unordered_map<Edge,std::unordered_set<Edge>> part = simTran(qgraph,*ViewGraph_list[i],sim);
//        std::cout<<"view "<<i+1<<std::endl;
//        std::cout<<"vertex:"<<std::endl;
//        print_sim_vertex_result(*ViewGraph_list[i],sim);
//        std::cout<<"edge:"<<std::endl;
//        print_sim_edge_result(part);
        std::unordered_set<Edge> MGEdge;
        if(!part.empty()){
           std::unordered_map<Edge,std::unordered_set<Edge>>::iterator iter;
           for(iter=part.begin();iter !=part.end(); ++iter){
               for(auto e:iter->second){
                   MGEdge.insert(e);
               }
           }
           contain.push_back(i);
        }
        if(!MGEdge.empty()){
            for(auto e:MGEdge){
                eSet.insert(e);
            }
        }
        MGSet.push_back(MGEdge);
    }
    std::unordered_set<Edge> qgraph_edges_set;
    for(auto e:qgraph.GetAllEdges()){
        qgraph_edges_set.insert(e);
    }
    for(auto e:qgraph_edges_set){
        if(eSet.find(e) == eSet.end()){
            return false;
        }
    }
    return true;
}

std::vector<int> View::minContain(Graph &qgraph){
    std::unordered_set<Edge> tmp;//current match graph
    std::unordered_set<Edge> rem;//uncovered edge set
    std::vector<int> ans;
    for(auto e:qgraph.GetAllEdges()){
        rem.insert(e);
    }
    while(!rem.empty()){
       int max =0;
       int pos_index=-1;
       std::unordered_set<Edge> pos;//represent the best choice
       for(int i=0;i<MGSet.size();++i){
           std::unordered_set<Edge> eset = MGSet[i];
           if(!eset.empty() && !vector_find(ans,i)){
               for(auto e:eset){
                   tmp.insert(e);
               }
               tmp = intersection(tmp,rem);
               if(max<tmp.size()){
                   max=tmp.size();
                   pos = eset;
                   pos_index=i;
               }
               tmp.clear();
           }
       }
       if(!pos.empty()){
           rem=diff(rem,pos);
           ans.push_back(pos_index);
       }
       tmp.clear();
    }
    return ans;
}

std::vector<StrongR>  View::queryByViews(Graph &dgraph,Graph &qgraph){
    vector<StrongR> max_result;
    /*
    int index=1;
    for(int i=1;i<6;++i){
        GraphLoader view_loader;
        Graph* new_view = new Graph();
        view_loader.LoadGraph(*new_view,"../data/contain1/data"+std::to_string(index)+"/view"+std::to_string(i)+".v","../data/contain1/data"+std::to_string(index)+"/view"+std::to_string(i)+".e");
        add_ViewGraph(new_view);
    }*/
    bool is_contain = containCheck(qgraph);
  //  cout<<is_contain<<endl;
    if(!is_contain){
        return max_result;
    }
    std::vector<int> min_contain_vec = minContain(qgraph);
   // for(auto n:min_contain_vec){
  //      cout<<n<<endl;
  //  }
    std::unordered_map<VertexID, std::unordered_set<VertexID>> max_query_sim;
    std::unordered_set<VertexID> view_nodes;
    std::unordered_set<Edge> view_edges;
    for(auto u :qgraph.GetAllVerticesID()){
        max_query_sim[u] = std::unordered_set<VertexID>();
    }
    StrongSim strongs;
    for(auto num:min_contain_vec){
        vector<StrongR> res= strongs.strong_simulation_sim(dgraph,*ViewGraph_list[num]);
        DualSim dualsim;
        std::unordered_map<VertexID, std::unordered_set<VertexID>>  sim;
        bool initialized_sim = false;
        dualsim.dual_simulation(qgraph,*ViewGraph_list[num],sim,initialized_sim);
        //print_sim_vertex_result(*ViewGraph_list[num],sim);
        //cout<<res.size()<<endl;

        for(int i=0;i<res.size();++i){
            std::unordered_map<VertexID, std::unordered_set<VertexID>> ball_sim=res[i].ballr();
            for(auto u:(*ViewGraph_list[num]).GetAllVerticesID()){
                for(auto v :ball_sim[u]){
                    for(auto q_node:sim[u]){
                        max_query_sim[q_node].insert(v);
                        view_nodes.insert(v);
                    }
                }
            }
        }
    }
    //cout<<"dd1"<<endl;
    //return max_result;
    for(auto e:qgraph.GetAllEdges()){
        std::unordered_set<Edge> eSet;
        VertexID source = e.src();
        VertexID target = e.dst();
        std::unordered_set<VertexID> simsource = max_query_sim[source];
        std::unordered_set<VertexID> simtarget = max_query_sim[target];
        for(auto source_node : simsource){
            for(auto target_node : simtarget){
                if(dgraph.ExistEdge(source_node,target_node)){
                    view_edges.insert(Edge(source_node,target_node,1));
                }
            }
        }
    }
    Ball_View graph_view(view_nodes,view_edges);
  //  cout<<graph_view.GetNumVertices()<<' '<<graph_view.GetNumEdges()<<endl;
    //return max_result;
    int d_Q = cal_diameter_qgraph(qgraph);
    dual_filter_match(graph_view,qgraph,max_query_sim);
    //print_sim_vertex_result(qgraph,max_query_sim);
    for (auto w : dgraph.GetAllVerticesID()){
        if(!valid_sim_w(qgraph,max_query_sim,w)){
            continue;
        }
       /**
         *find d_hop_nodes for w in dgraph
        */
       std::unordered_set<VertexID> ball_node;
       graph_view.find_hop_nodes(w,d_Q,ball_node);

       std::unordered_set<VertexID> ball_filter_node;
       std::unordered_set<Edge> ball_filter_edge;
       std::unordered_map<VertexID, std::unordered_set<VertexID>> S_w;
       for(auto u : qgraph.GetAllVerticesID()){
          for (auto v : max_query_sim[u]){
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
                if (graph_view.ExistEdge(sim_v1,sim_v2)){
                    ball_filter_edge.insert(Edge(sim_v1,sim_v2,1));
                }
             }
          }
       }
       //cout<<"view center "<<w<<endl;
      // print_sim_vertex_result(qgraph,S_w);
       Ball_View ball_view(ball_filter_node,ball_filter_edge);

       std::unordered_set<VertexID> refined_ball_vertex;
       std::unordered_set<Edge> refinded_ball_edge;
       ball_view.find_connectivity_nodes(w,refined_ball_vertex);
       for(auto e :ball_filter_edge){
          if(refined_ball_vertex.find(e.src()) != refined_ball_vertex.end() && refined_ball_vertex.find(e.dst())!=refined_ball_vertex.end()){
              refinded_ball_edge.insert(e);
          }
       }
       Ball_View refined_ball_view(refined_ball_vertex,refinded_ball_edge);
       rename_sim(refined_ball_view,qgraph,S_w);
       dual_filter_match(refined_ball_view, qgraph,S_w);
       extract_max_pg(refined_ball_view,dgraph,qgraph, w,S_w);
       max_result.emplace_back(w,S_w);
      // break;
    }
    return max_result;
}


int View::cal_diameter_qgraph(Graph &qgraph){
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

void View::rename_sim(Ball_View &ball_view,Graph &qgraph,
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

bool View::valid_sim_w(Graph &qgraph,std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim,VertexID w){
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

void View::ss_counter_initialization(Ball_View &ball_view,Graph &qgraph,
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

void View::dual_filter_match(Ball_View &refined_ball, Graph &qgraph,
                      std::unordered_map<VertexID, std::unordered_set<VertexID>> &S_w){
        std::set<std::pair<VertexID,VertexID> > filter_set;
        std::unordered_map<VertexID, std::vector<int> > sim_counter_pre,sim_counter_post;
        ss_counter_initialization(refined_ball,qgraph, sim_counter_pre,sim_counter_post,S_w);
        push_phase(refined_ball,qgraph,filter_set, sim_counter_pre, sim_counter_post,S_w);
        decremental_refine(refined_ball,qgraph, filter_set,sim_counter_pre,sim_counter_post,S_w);
   }

void View::push_phase(Ball_View &ball,Graph &qgraph,
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


 void View::update_counter(Ball_View &ball,Graph &qgraph,VertexID u,VertexID v,
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

void View::decremental_refine(Ball_View &ball_view,Graph &qgraph,
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

void View::extract_max_pg(Ball_View &ball_view,Graph &dgraph,Graph &qgraph,VertexID w,
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
   pg_view.find_connectivity_nodes(w,vertex_match_set1);


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

void View::clear_parameter(){
     for(int i=0;i<ViewGraph_list.size();++i){
         delete ViewGraph_list[i];
     }
     ViewGraph_list.clear();
}