#include"dual_incfilter.h"
    Dual_Incfilter::Dual_Incfilter(){messageBuffers.init();}

    Dual_Incfilter::~Dual_Incfilter(){}


    void Dual_Incfilter::dual_counter_initialization(GraphView &graph_view, Graph &qgraph,
                                     std::unordered_map<VertexID, std::vector<int>> &sim_counter_post,
                                     std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre,
                                     std::unordered_map<VertexID, std::unordered_set<VertexID>> &dsim){
        for (auto w : graph_view.GetAllVerticesID()){
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
    }

    void Dual_Incfilter::update_counter(GraphView &graph_view,Graph &qgraph,VertexID u,VertexID v,
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

    void Dual_Incfilter::decremental_rmove(Fragment &fragment, GraphView &graph_view,Graph &qgraph,
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
            if (fragment.isBorderVertex(fragment.getGlobalID(v))){
                std::bitset<NUM_FRAGMENTS> dest= fragment.getMsgThroughDest(fragment.getGlobalID(v));
                for (int fid = 0; fid < dest.size(); fid++){
                    if (dest.test(fid) && fid != get_worker_id()){
                        messageBuffers.add_message(fid,std::make_pair(u,fragment.getGlobalID(v)));
                   }
                }
            }
            update_counter(graph_view,qgraph,u,v,sim_counter_pre,sim_counter_post);
            for (auto u_p : qgraph.GetParentsID(u)){
                for (auto v_p : graph_view.GetParentsID(v)){
                    if (dsim[u_p].find(v_p)!=dsim[u_p].end()){
                        if(sim_counter_post[v_p][u]==0 && OuterVertices->find(fragment.getGlobalID(v_p)) == OuterVertices->end()){
                            filter_set.insert(std::make_pair(u_p,v_p));
                        }
                    }
                }
            }
            for(auto u_s :qgraph.GetChildrenID(u)){
                for(auto v_s: graph_view.GetChildrenID(v)){
                    if (dsim[u_s].find(v_s) != dsim[u_s].end()){
                        if(sim_counter_pre[v_s][u] == 0 && OuterVertices->find(fragment.getGlobalID(v_s)) == OuterVertices->end()){
                            filter_set.insert(std::make_pair(u_s,v_s));
                        }
                    }
                }
            }
        }
      }

    void Dual_Incfilter::dual_incfilter(Fragment &fragment, GraphView &graph_view, Graph &qgraph,
                                             std::unordered_map<VertexID, std::unordered_set<VertexID>> &dsim){
        OuterVertices = const_cast<std::unordered_set<VertexID> *>(fragment.getOuterVertices());
        innerVertices = const_cast<std::unordered_set<VertexID> *>(fragment.getInnerVertices());
        std::set<std::pair<VertexID,VertexID>>  filter_set;
        std::unordered_map<VertexID, std::vector<int>> sim_counter_post,sim_counter_pre;
        worker_barrier();
        pEval(fragment, graph_view, qgraph,filter_set,dsim,sim_counter_pre, sim_counter_post);
        worker_barrier();
        while(is_continue()){
            incEval(fragment, graph_view, qgraph,filter_set,dsim,sim_counter_pre, sim_counter_post);
            worker_barrier();
        }
        out_global_result(fragment,qgraph,dsim);
     }


    void Dual_Incfilter::pEval(Fragment &fragment, GraphView &graph_view, Graph &qgraph,
                          std::set<std::pair<VertexID,VertexID>> &filter_set,
                          std::unordered_map<VertexID, std::unordered_set<VertexID>> &dsim,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_post){
        dual_counter_initialization(graph_view, qgraph, sim_counter_post, sim_counter_pre, dsim);
        for (auto u :qgraph.GetAllVerticesID()){
            for (auto v : dsim[u]){
                for (auto u_s : qgraph.GetChildrenID(u)){
                    if (sim_counter_post[v][u_s]==0 && OuterVertices->find(fragment.getGlobalID(v)) == OuterVertices->end()){
                        filter_set.insert(std::make_pair(u,v));
                        break;
                    }
                }
                for(auto u_p : qgraph.GetParentsID(u)){
                    if(sim_counter_pre[v][u_p]==0 && OuterVertices->find(fragment.getGlobalID(v)) == OuterVertices->end()){
                        filter_set.insert(std::make_pair(u,v));
                        break;
                    }
                }
            }
        }
        decremental_rmove(fragment, graph_view,qgraph,filter_set,dsim,sim_counter_pre,sim_counter_post);
        messageBuffers.sync_messages();
     }



    void Dual_Incfilter::incEval(Fragment &fragment, GraphView &graph_view, Graph &qgraph,
                          std::set<std::pair<VertexID,VertexID>> &filter_set,
                          std::unordered_map<VertexID, std::unordered_set<VertexID>> &dsim,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_post){
      for (auto item : messageBuffers.get_messages()){
          VertexID u = item.first;
          VertexID v = fragment.getLocalID(item.second);
          dsim[u].erase(v);
          update_counter(graph_view,qgraph,u,v,sim_counter_pre,sim_counter_post);
           for (auto u_p : qgraph.GetParentsID(u)){
                for (auto v_p : graph_view.GetParentsID(v)){
                    if (dsim[u_p].find(v_p)!=dsim[u_p].end()){
                        if(sim_counter_post[v_p][u]==0 && OuterVertices->find(fragment.getGlobalID(v_p)) == OuterVertices->end()){
                            filter_set.insert(std::make_pair(u_p,v_p));
                        }
                    }
                }
            }
            for(auto u_s :qgraph.GetChildrenID(u)){
                for(auto v_s: graph_view.GetChildrenID(v)){
                    if (dsim[u_s].find(v_s) != dsim[u_s].end()){
                        if(sim_counter_pre[v_s][u] == 0 && OuterVertices->find(fragment.getGlobalID(v_s)) == OuterVertices->end()){
                            filter_set.insert(std::make_pair(u_s,v_s));
                        }
                    }
                }

            }
//          filter_set.insert(std::make_pair(u,w));
      }
      messageBuffers.reset_in_messages();
      decremental_rmove(fragment, graph_view,qgraph,filter_set,dsim,sim_counter_pre,sim_counter_post);
      messageBuffers.sync_messages();
     }


    bool Dual_Incfilter::is_continue(){
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

    void Dual_Incfilter::out_global_result(Fragment &fragment,  Graph &qgraph, std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim){
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