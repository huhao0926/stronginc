#include "dual_parallel_inc.h"

Dual_parallelInc::Dual_parallelInc(){
    messageBuffers.init();
    vertexBuffers.init();
}
Dual_parallelInc::~Dual_parallelInc(){}

void Dual_parallelInc::propagate_add_PEval(Fragment &fragment, Graph &dgraph,Graph &qgraph,
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
                if (fragment.isBorderVertex(fragment.getGlobalID(v))){
                    std::bitset<NUM_FRAGMENTS> dest= fragment.getMsgThroughDest(fragment.getGlobalID(v));
                    for (int fid = 0; fid < dest.size(); fid++){
                        if (dest.test(fid) && fid != get_worker_id()){
                            messageBuffers.add_message(fid,std::pair<VertexID,int>(fragment.getGlobalID(v), u));
                            }
                        }
                    }
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
                if (fragment.isBorderVertex(fragment.getGlobalID(v))){
                    std::bitset<NUM_FRAGMENTS> dest= fragment.getMsgThroughDest(fragment.getGlobalID(v));
                    for (int fid = 0; fid < dest.size(); fid++){
                        if (dest.test(fid) && fid != get_worker_id()){
                            messageBuffers.add_message(fid,std::pair<VertexID,int>(fragment.getGlobalID(v), u));
                            }
                        }
                    }
                for(auto pm: tmp_set){
                    if(candidate_node.find(pm)==candidate_node.end()){
                        candidate_node.insert(pm);
                    }
                }
                 }
            }
        messageBuffers.sync_messages();
        }

bool Dual_parallelInc::is_continue(){
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

void Dual_parallelInc::propagate_add_IncEval(Fragment &fragment, Graph &dgraph,Graph &qgraph,
                       std::set<std::pair<VertexID,VertexID>> &candidate_node,
                       std::unordered_map<VertexID, std::unordered_set<VertexID>> &aff_node,
                       std::unordered_map<VertexID, std::unordered_set<VertexID>> &dsim,
                       std::set<std::pair<VertexID,VertexID>> &already_matched){
    for (auto item :messageBuffers.get_messages()){
        VertexID u = item.second;
        VertexID w = fragment.getLocalID(item.first);
//        if(dsim[u].find(w) != dsim[u].end()){
            aff_node[u].insert(w);
            for(auto u_p :qgraph.GetParentsID(u)){
                for(auto v_p:dgraph.GetParentsID(w)){
                    if(qgraph.GetVertexLabel(u_p)==dgraph.GetVertexLabel(v_p)){
                        candidate_node.insert(std::make_pair(u_p,v_p));
                    }
                }
            }

            for(auto u_s :qgraph.GetChildrenID(u)){
                for(auto v_s:dgraph.GetChildrenID(w)){
                    if(qgraph.GetVertexLabel(u_s)==dgraph.GetVertexLabel(v_s)){
                        candidate_node.insert(std::make_pair(u_s,v_s));
                    }
                }
            }
//        }
    }
    messageBuffers.reset_in_messages();
   propagate_add_PEval(fragment,dgraph,qgraph,candidate_node,aff_node,dsim,already_matched);
 }

void Dual_parallelInc::incremental_add_edges(Fragment &fragment, Graph &dgraph, Graph &qgraph,
                                                    std::unordered_map<VertexID, std::unordered_set<VertexID>> &dsim,
                                                    std::set<std::pair<VertexID,VertexID>> &add_edges){
        const std::unordered_set<VertexID> *OuterVertices = fragment.getOuterVertices();
        std::set<std::pair<VertexID,VertexID>> candidate_node;
        std::unordered_map<VertexID, std::unordered_set<VertexID>> aff_node;
        for(auto u : qgraph.GetAllVerticesID()){
            aff_node[u] = std::unordered_set<VertexID>();
        }
        for(auto edge:add_edges){
            if(fragment.isInnerVertex(edge.first) || fragment.isInnerVertex(edge.second)){
                std::pair<VertexID,VertexID> add_e(fragment.getLocalID(edge.first),fragment.getLocalID(edge.second));
                for (auto e1 : qgraph.GetAllEdges()){
                    VertexID node1 = e1.src();
                    VertexID node2 = e1.dst();
                    if (qgraph.GetVertexLabel(node1)==dgraph.GetVertexLabel(add_e.first) && qgraph.GetVertexLabel(node2)==dgraph.GetVertexLabel(add_e.second)){
                        candidate_node.insert(std::make_pair(node1,add_e.first));
                        candidate_node.insert(std::make_pair(node2,add_e.second));
                    }
                }
            }
        }
        std::set<std::pair<VertexID,VertexID>> already_matched;
        propagate_add_PEval(fragment,dgraph,qgraph,candidate_node,aff_node,dsim,already_matched);
        while(is_continue()){
            propagate_add_IncEval(fragment,dgraph,qgraph,candidate_node,aff_node,dsim,already_matched);
            worker_barrier();
        }
        worker_barrier();
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
                    if (sim_counter_post[v][u_s]==0 &&  OuterVertices->find(fragment.getGlobalID(v)) == OuterVertices->end()){
                        filter_set.insert(std::make_pair(u,v));
                        break;
                    }
                }
                for(auto u_p : qgraph.GetParentsID(u)){
                    if(sim_counter_pre[v][u_p]==0 &&  OuterVertices->find(fragment.getGlobalID(v)) == OuterVertices->end()){
                        filter_set.insert(std::make_pair(u,v));
                        break;
                    }
                }
            }
        }
        worker_barrier();
        propagate_remove_PEval(fragment,graph_view,qgraph,aff_node,filter_set,sim_counter_pre,sim_counter_post,already_matched);
        while(is_continue()){
            propagate_remove_IncEval(fragment,graph_view,qgraph,aff_node,filter_set,sim_counter_pre,sim_counter_post,already_matched);
            worker_barrier();
        }
        for (auto u :qgraph.GetAllVerticesID()){
            for (auto v1 : aff_node[u]){
                if (dsim[u].find(v1)==dsim[u].end()){
                    dsim[u].insert(v1);
                }
            }
        }
//        out_global_result(fragment,qgraph,dsim);
   }

void Dual_parallelInc::incremental_remove_edgs(Fragment &fragment, Graph &dgraph,Graph &qgraph,
                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &dsim,
                           std::set<std::pair<VertexID,VertexID>> &rm_edges){
        const std::unordered_set<VertexID> *OuterVertices = fragment.getOuterVertices();
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
                    if (sim_counter_post[v][u_s]==0 &&  OuterVertices->find(fragment.getGlobalID(v)) == OuterVertices->end()){
                        filter_set.insert(std::make_pair(u,v));
                        break;
                    }
                }
                for(auto u_p : qgraph.GetParentsID(u)){
                    if(sim_counter_pre[v][u_p]==0 &&  OuterVertices->find(fragment.getGlobalID(v)) == OuterVertices->end()){
                        filter_set.insert(std::make_pair(u,v));
                        break;
                    }
                }
            }
        }
        propagate_remove_PEval(fragment,graph_view,qgraph,dsim,filter_set,sim_counter_pre,sim_counter_post,already_matched);
        while(is_continue()){
            propagate_remove_IncEval(fragment,graph_view,qgraph,dsim,filter_set,sim_counter_pre,sim_counter_post,already_matched);
            worker_barrier();
        }
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
//        out_global_result(fragment,qgraph,dsim);
 }

void Dual_parallelInc::propagate_remove_PEval(Fragment &fragment,GraphView &graph_view,Graph &qgraph,
                          std::unordered_map<VertexID, std::unordered_set<VertexID>> &aff_node,
                          std::set<std::pair<VertexID,VertexID>> &filter_set,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_post,
                          std::set<std::pair<VertexID,VertexID>> &already_matched){
        const std::unordered_set<VertexID> *OuterVertices = fragment.getOuterVertices();
        while(!filter_set.empty()){
            std::pair<VertexID,VertexID> pmatch = *filter_set.begin();
            VertexID u = pmatch.first;
            VertexID v = pmatch.second;
            filter_set.erase(filter_set.begin());
            if (already_matched.find(pmatch)!=already_matched.end()){
                continue;
            }
            aff_node[u].erase(v);
            if (fragment.isBorderVertex(fragment.getGlobalID(v))){
                std::bitset<NUM_FRAGMENTS> dest= fragment.getMsgThroughDest(fragment.getGlobalID(v));
                for (int fid = 0; fid < dest.size(); fid++){
                    if (dest.test(fid) && fid != get_worker_id()){
                        messageBuffers.add_message(fid,std::pair<VertexID,int>(fragment.getGlobalID(v), u));
                        }
                    }
            }
            update_counter(graph_view,sim_counter_pre,sim_counter_post,u,v);
            for (auto u_p : qgraph.GetParentsID(u)){
                for (auto v_p : graph_view.GetParentsID(v)){
                    if (aff_node[u_p].find(v_p)!=aff_node[u_p].end()){
                        if(sim_counter_post[v_p][u]==0 && OuterVertices->find(fragment.getGlobalID(v_p)) == OuterVertices->end()){
                            filter_set.insert(std::make_pair(u_p,v_p));
                        }
                    }
                }
            }
            for(auto u_s :qgraph.GetChildrenID(u)){
                for(auto v_s: graph_view.GetChildrenID(v)){
                    if (aff_node[u_s].find(v_s) != aff_node[u_s].end()){
                        if(sim_counter_pre[v_s][u] == 0 &&OuterVertices->find(fragment.getGlobalID(v_s)) == OuterVertices->end()){
                            filter_set.insert(std::make_pair(u_s,v_s));
                        }
                    }
                }
            }
        }
        messageBuffers.sync_messages();
}

void Dual_parallelInc::propagate_remove_IncEval(Fragment &fragment,GraphView &graph_view,Graph &qgraph,
                          std::unordered_map<VertexID, std::unordered_set<VertexID>> &aff_node,
                          std::set<std::pair<VertexID,VertexID>> &filter_set,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre,
                          std::unordered_map<VertexID, std::vector<int>> &sim_counter_post,
                          std::set<std::pair<VertexID,VertexID>> &already_matched){
    const std::unordered_set<VertexID> *OuterVertices = fragment.getOuterVertices();
    for (auto item :messageBuffers.get_messages()){
        VertexID u = item.second;
        VertexID v = fragment.getLocalID(item.first);
        if(aff_node[u].find(v) != aff_node[u].end()){
            aff_node[u].erase(v);
            update_counter(graph_view,sim_counter_post, sim_counter_pre, u, v);
            for (auto u_p : qgraph.GetParentsID(u)){
                for (auto v_p : graph_view.GetParentsID(v)){
                    if (aff_node[u_p].find(v_p)!=aff_node[u_p].end()){
                        if(sim_counter_post[v_p][u]==0 && OuterVertices->find(fragment.getGlobalID(v_p)) == OuterVertices->end()){
                            filter_set.insert(std::make_pair(u_p,v_p));
                        }
                    }
                }
            }
            for(auto u_s :qgraph.GetChildrenID(u)){
                for(auto v_s: graph_view.GetChildrenID(v)){
                    if (aff_node[u_s].find(v_s) != aff_node[u_s].end()){
                        if(sim_counter_pre[v_s][u] == 0 &&OuterVertices->find(fragment.getGlobalID(v_s)) == OuterVertices->end()){
                            filter_set.insert(std::make_pair(u_s,v_s));
                        }
                    }
                }
            }
        }
    }
    messageBuffers.reset_in_messages();
    propagate_remove_PEval(fragment, graph_view,qgraph,aff_node,filter_set,sim_counter_pre,sim_counter_post,already_matched);
 }

void Dual_parallelInc::dual_parallel_incremental(Fragment &fragment, Graph &dgraph, Graph &qgraph,
                                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &dsim,
                                           std::set<std::pair<VertexID,VertexID>> &add_edges,
                                           std::set<std::pair<VertexID,VertexID>> &rm_edges){

   }

void Dual_parallelInc::update_by_add_edges(Fragment &fragment,Graph &dgraph,std::set<std::pair<VertexID,VertexID>> &add_edges,bool communication_next){
      //  int tmp_num_vertices = graph.GetNumVertices();
        std::unordered_set<Vertex> vertices_;
        int fid = get_worker_id();
        std::unordered_set<Edge> edges_;
        for(auto e:add_edges){
            edges_.insert(Edge(e.first,e.second,1));
            if(fragment.isInnerVertex(e.first) && !fragment.isInnerVertex(e.second)){
                const int dst = fragment.getVertexFragmentID(e.second);
                vertexBuffers.add_message(dst,Vertex(e.first,dgraph.GetVertexLabel(fragment.getLocalID(e.first))));
            }else if(fragment.isInnerVertex(e.second) && !fragment.isInnerVertex(e.first)){
                const int dst = fragment.getVertexFragmentID(e.first);
                vertexBuffers.add_message(dst,Vertex(e.second,dgraph.GetVertexLabel(fragment.getLocalID(e.second))));
            }
        }
        vertexBuffers.sync_messages();

        for (auto item :vertexBuffers.get_messages()){
            if(!fragment.isOuterVertex(item.id())){
                vertices_.insert(item);
            }
        }
        vertexBuffers.reset_in_messages();
        fragment.update_fragment_add_edges(dgraph,edges_,vertices_,communication_next);
    }

void Dual_parallelInc::update_by_remove_edges(Fragment &fragment,Graph &dgraph,std::set<std::pair<VertexID,VertexID>> &rm_edges,bool communication_next){
        std::vector<Edge> rm_edges_;
        for(auto e :rm_edges){
            rm_edges_.emplace_back(e.first,e.second,1);
        }
        fragment.update_fragment_remove_edges(dgraph,rm_edges_,communication_next);
    }

 void Dual_parallelInc::out_global_result(Fragment &fragment,  Graph &qgraph, std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim){
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


void Dual_parallelInc::print_global_info(Fragment &fragment, Graph &qgraph, std::unordered_map<VertexID, std::unordered_set<VertexID>> &dsim){
       std::unordered_map<VertexID, std::unordered_set<VertexID>>  sim;
      for(auto u:qgraph.GetAllVerticesID()){
          sim[u] = std::unordered_set<VertexID>();
          for(auto v :dsim[u]){
              sim[u].insert(v);
          }
      }
      out_global_result(fragment,qgraph,sim);
      int fid =get_worker_id();
      if (fid==0){
            std::vector<std::unordered_map<VertexID, std::unordered_set<VertexID>>>  tmp_vec(get_num_workers());
            tmp_vec[fid] = sim;
            masterGather(tmp_vec);
            std::unordered_map<VertexID, std::unordered_set<VertexID>>  globalsim;
            for(auto u :qgraph.GetAllVerticesID()){
                 globalsim[u] = std::unordered_set<VertexID>();
             }
            for(int i=0;i<get_num_workers();i++){
                for(auto u :qgraph.GetAllVerticesID()){
                     for(auto v : tmp_vec[i][u])
                     {globalsim[u].insert(v);}
                }
            }
            worker_barrier();
            for(auto u :qgraph.GetAllVerticesID()){
                cout<<globalsim[u].size()<<endl;
            }
      }else{
         slaveGather(sim);
         worker_barrier();
      }


}