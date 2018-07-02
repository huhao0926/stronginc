#include "dual_incremental.h"

  DualInc::DualInc() {}

  DualInc::~DualInc() {}

  void DualInc::propagate_add(Graph &dgraph,Graph &qgraph,
                       std::set<std::pair<VertexID,VertexID>> &candidate_node,
                       std::unordered_map<VertexID, std::unordered_set<VertexID>> &aff_node,
                       std::unordered_map<VertexID, std::unordered_set<VertexID>> &dsim,
                       std::set<std::pair<VertexID,VertexID>> &already_matched){
        while (!candidate_node.empty()){
            std::pair<VertexID,VertexID> pmatch = *candidate_node.begin();
            VertexID u = pmatch.first;
            VertexID v = pmatch.second;
            candidate_node.erase(candidate_node.begin());
            if (aff_node[u].find(v)!=aff_node[u].end()){
                continue;
            }
            if (dsim[u].find(v) != dsim[u].end()){
                already_matched.insert(std::make_pair(u,v));
                aff_node[u].insert(v);
                continue;
            }
            std::set<std::pair<VertexID,VertexID>> tmp_set;
            bool is_match = true;
            for (auto u_p : qgraph.GetParentsID(u)){
                int coun=0;
                for(auto v_p : dgraph.GetParentsID(v)){
                    if (qgraph.GetVertexLabel(u_p)==dgraph.GetVertexLabel(v_p)){
                        coun+=1;
                        tmp_set.insert(std::make_pair(u_p,v_p));
                    }
                }
                if(coun==0){
                    is_match = false;
                    break;
                }
            }
            if(is_match){
                for(auto u_s : qgraph.GetChildrenID(u)){
                    int coun=0;
                    for (auto v_s : dgraph.GetChildrenID(v)){
                        if (qgraph.GetVertexLabel(u_s)==dgraph.GetVertexLabel(v_s)){
                            coun+=1;
                            tmp_set.insert(std::make_pair(u_s,v_s));
                        }

                    }
                    if (coun==0){
                        is_match = false;
                        break;
                    }
                }
            }
            if(is_match){
                aff_node[u].insert(v);
                for(auto pm: tmp_set){
                    if(candidate_node.find(pm)==candidate_node.end()){
                        candidate_node.insert(pm);
                    }
                }
            }
        }
    }


  void DualInc::update_pre_dec_counter(GraphView &graph_view,Graph &qgraph,VertexID u,VertexID v,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_post){
        for(auto vp : graph_view.GetParentsID(v)){
            if (sim_counter_post.find(vp)!=sim_counter_post.end()){
                if(sim_counter_post[vp][u]>0){
                    sim_counter_post[vp][u]-=1;
                }
            }
        }
        for (auto vs : graph_view.GetChildrenID(v)){
            if (sim_counter_pre.find(vs)!=sim_counter_pre.end()){
                if(sim_counter_pre[vs][u]>0){
                    sim_counter_pre[vs][u]-=1;
                }
            }
        }
    }

  void DualInc::propagate_remove(GraphView &graph_view,Graph &qgraph,
                          std::unordered_map<VertexID, std::unordered_set<VertexID>> &aff_node,
                          std::set<std::pair<VertexID,VertexID>> &filter_set,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_post,
                          std::set<std::pair<VertexID,VertexID>> &already_matched){
        while(!filter_set.empty()){
            std::pair<VertexID,VertexID> pmatch = *filter_set.begin();
            VertexID u = pmatch.first;
            VertexID v = pmatch.second;
            filter_set.erase(filter_set.begin());
            if (already_matched.find(pmatch)!=already_matched.end()){
                continue;
            }
            aff_node[u].erase(v);
            update_pre_dec_counter(graph_view,qgraph,u,v,sim_counter_pre,sim_counter_post);
            for (auto u_p : qgraph.GetParentsID(u)){
                for (auto v_p : graph_view.GetParentsID(v)){
                    if (aff_node[u_p].find(v_p)!=aff_node[u_p].end()){
                        if(sim_counter_post[v_p][u]==0){
                            filter_set.insert(std::make_pair(u_p,v_p));
                        }
                    }
                }
            }
            for(auto u_s :qgraph.GetChildrenID(u)){
                for(auto v_s: graph_view.GetChildrenID(v)){
                    if (aff_node[u_s].find(v_s) != aff_node[u_s].end()){
                        if(sim_counter_pre[v_s][u] == 0){
                            filter_set.insert(std::make_pair(u_s,v_s));
                        }
                    }
                }

            }
        }
    }

  void DualInc::incremental_addedges(Graph &dgraph,Graph &qgraph,
                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &dsim,
                           std::vector<std::pair<VertexID,VertexID>> &add_edges){
        std::set<std::pair<VertexID,VertexID>> candidate_node;
        std::unordered_map<VertexID, std::unordered_set<VertexID>> aff_node;
        for(auto u : qgraph.GetAllVerticesID()){
            aff_node[u] = std::unordered_set<VertexID>();
        }
        for (auto add_e : add_edges){
            for (auto e1 : qgraph.GetAllEdges()){
                VertexID node1 = e1.src();
                VertexID node2 = e1.dst();
                if (qgraph.GetVertexLabel(node1)==dgraph.GetVertexLabel(add_e.first) && qgraph.GetVertexLabel(node2)==dgraph.GetVertexLabel(add_e.second)){
                    candidate_node.insert(std::make_pair(node1,add_e.first));
                    candidate_node.insert(std::make_pair(node2,add_e.second));
                }
            }
        }
        if(candidate_node.empty()){
            return ;
        }
        std::set<std::pair<VertexID,VertexID>> already_matched;
       // std::cout<<"propagate_add_begin "<<candidate_node.size()<<std::endl;
        propagate_add(dgraph,qgraph,candidate_node,aff_node,dsim,already_matched);
       // std::cout<<"propagate_add finish"<<std::endl;
        std::unordered_set<VertexID> view_nodes;
        for(auto u : qgraph.GetAllVerticesID()){
            for(auto v :aff_node[u]){
                view_nodes.insert(v);
            }
        }
        GraphView graph_view(dgraph,&view_nodes);
        std::unordered_map<VertexID, std::vector<int>> sim_counter_post,sim_counter_pre;
        for (auto w : view_nodes){
            sim_counter_post[w] = std::vector<int>(qgraph.GetNumVertices(), 0);
            sim_counter_pre[w] = std::vector<int>(qgraph.GetNumVertices(), 0);
            for (auto u : qgraph.GetAllVerticesID()){
                int len_des=0,len_pre=0;
                for (auto des_w : graph_view.GetChildrenID(w)){
                     if(aff_node[u].find(des_w) != aff_node[u].end()){
                        len_des += 1;
                     }
                }
                for (auto pre_w : graph_view.GetParentsID(w)){
                    if (aff_node[u].find(pre_w) != aff_node[u].end()){
                        len_pre+=1;
                    }
                }
                sim_counter_post[w][u] = len_des;
                sim_counter_pre[w][u] = len_pre;
            }
        }
        std::set<std::pair<VertexID,VertexID>>  filter_set;
        for (auto u :qgraph.GetAllVerticesID()){
            for (auto v : aff_node[u]){
                for (auto u_s : qgraph.GetChildrenID(u)){
                    if (sim_counter_post[v][u_s]==0){
                        filter_set.insert(std::make_pair(u,v));
                        break;
                    }
                }
                for(auto u_p : qgraph.GetParentsID(u)){
                    if(sim_counter_pre[v][u_p]==0){
                        filter_set.insert(std::make_pair(u,v));
                        break;
                    }
                }
            }
        }

        propagate_remove(graph_view,qgraph,aff_node,filter_set,sim_counter_pre,sim_counter_post,already_matched);
//        bool has_inc_result=true;
//        for(auto u :qgraph.GetAllVerticesID()){
//            if(aff_node[u].empty()){
//               has_inc_result=false;
//               return;
//            }
//        }
        for (auto u :qgraph.GetAllVerticesID()){
            for (auto v1 : aff_node[u]){
                if (dsim[u].find(v1)==dsim[u].end()){
                    dsim[u].insert(v1);
                }
            }
        }
    }

 void DualInc::update_counter(Graph &dgraph,Graph &qgraph,VertexID u,VertexID v,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_post){
        for(auto vp : dgraph.GetParentsID(v)){
            if (sim_counter_post.find(vp)!=sim_counter_post.end()){
                if(sim_counter_post[vp][u]>0){
                    sim_counter_post[vp][u]-=1;
                }
            }
        }
        for (auto vs : dgraph.GetChildrenID(v)){
            if (sim_counter_pre.find(vs)!=sim_counter_pre.end()){
                if(sim_counter_pre[vs][u]>0){
                    sim_counter_pre[vs][u]-=1;
                }
            }
        }
     }


  void DualInc::decremental_rmove(Graph &dgraph,Graph &qgraph,
                          std::set<std::pair<VertexID,VertexID>> &filter_set,
                          std::unordered_map<VertexID, std::unordered_set<VertexID>> &dsim,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_post){
        while(!filter_set.empty()){
            std::pair<VertexID,VertexID> pmatch = *filter_set.begin();
            VertexID u = pmatch.first;
            VertexID v = pmatch.second;
            filter_set.erase(filter_set.begin());
            dsim[u].erase(v);
            update_counter(dgraph,qgraph,u,v,sim_counter_pre,sim_counter_post);
            for (auto u_p : qgraph.GetParentsID(u)){
                for (auto v_p : dgraph.GetParentsID(v)){
                    if (dsim[u_p].find(v_p)!=dsim[u_p].end()){
                        if(sim_counter_post[v_p][u]==0){
                            filter_set.insert(std::make_pair(u_p,v_p));
                        }
                    }
                }
            }
            for(auto u_s :qgraph.GetChildrenID(u)){
                for(auto v_s: dgraph.GetChildrenID(v)){
                    if (dsim[u_s].find(v_s) != dsim[u_s].end()){
                        if(sim_counter_pre[v_s][u] == 0){
                            filter_set.insert(std::make_pair(u_s,v_s));
                        }
                    }
                }

            }
        }
  }

  void DualInc::incremental_removeedgs(Graph &dgraph,Graph &qgraph,
                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &dsim,
                           std::vector<std::pair<VertexID,VertexID>> &rm_edges){
        std::unordered_set<VertexID> view_nodes;
        for (auto u :qgraph.GetAllVerticesID()){
            for(auto v :dsim[u]){
                view_nodes.insert(v);
            }
        }
        GraphView graph_view(dgraph,&view_nodes);
        std::unordered_map<VertexID, std::vector<int>> sim_counter_post,sim_counter_pre;
        for (auto w : view_nodes){
            sim_counter_post[w] = std::vector<int>(qgraph.GetNumVertices(), 0);
            sim_counter_pre[w] = std::vector<int>(qgraph.GetNumVertices(), 0);
            for (auto u : qgraph.GetAllVerticesID()){
                int len_des=0,len_pre=0;
                for (auto des_w : graph_view.GetChildrenID(w)){
                     if(dsim[u].find(des_w) != dsim[u].end()){
                        len_des += 1;
                     }
                }
                for (auto pre_w : graph_view.GetParentsID(w)){
                    if (dsim[u].find(pre_w) != dsim[u].end()){
                        len_pre+=1;
                    }
                }
                sim_counter_post[w][u] = len_des;
                sim_counter_pre[w][u] = len_pre;
            }
        }
        std::set<std::pair<VertexID,VertexID>>  filter_set,already_matched;
        for (auto u :qgraph.GetAllVerticesID()){
            for (auto v : dsim[u]){
                for (auto u_s : qgraph.GetChildrenID(u)){
                    if (sim_counter_post[v][u_s]==0){
                        filter_set.insert(std::make_pair(u,v));
                        break;
                    }
                }
                for(auto u_p : qgraph.GetParentsID(u)){
                    if(sim_counter_pre[v][u_p]==0){
                        filter_set.insert(std::make_pair(u,v));
                        break;
                    }
                }
            }
        }
        propagate_remove(graph_view,qgraph,dsim,filter_set,sim_counter_pre,sim_counter_post,already_matched);
        bool valid_result = true;
        for (auto u:qgraph.GetAllVerticesID()){
            if(dsim[u].size()==0){
                valid_result=false;
                break;
            }
        }
        if (!valid_result){
           for (auto u:qgraph.GetAllVerticesID()){
               dsim[u].clear();
           }
        }
    }



  void DualInc::dual_incremental(Graph &dgraph,Graph &qgraph,
                          std::unordered_map<VertexID, std::unordered_set<VertexID>> &dsim,
                          std::vector<std::pair<VertexID,VertexID>> &add_edges,std::vector<std::pair<VertexID,VertexID>> &rm_edges){
          for(auto e :rm_edges){
              dgraph.RemoveEdge(Edge(e.first,e.second,1));
          }
          incremental_removeedgs(dgraph,qgraph,dsim,rm_edges);


          for (auto e:add_edges){
             dgraph.AddEdge(Edge(e.first,e.second,1));
          }

          incremental_addedges(dgraph,qgraph,dsim,add_edges);

          for (auto e:add_edges){
             dgraph.RemoveEdge(Edge(e.first,e.second,1));
          }

          for(auto e:rm_edges){
              dgraph.AddEdge(Edge(e.first,e.second,1));
          }
    }


using namespace std;
