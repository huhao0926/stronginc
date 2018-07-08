#include "dualsimulation_parallel.h"

Dual_Parallel::Dual_Parallel(){
    messageBuffers.init();
}

Dual_Parallel::Dual_Parallel(Fragment &fragment){
    messageBuffers.init();
    OuterVertices = const_cast<std::unordered_set<VertexID> *>(fragment.getOuterVertices());
    innerVertices = const_cast<std::unordered_set<VertexID> *>(fragment.getInnerVertices());
}
Dual_Parallel::~Dual_Parallel(){}


void Dual_Parallel::dual_sim_initialization(Fragment &fragment, Graph &dgraph, Graph &qgraph,
                               std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim,
                               bool &initialized_sim,
                               std::unordered_map<VertexID, std::unordered_set<VertexID>> &remove_pred,
                               std::unordered_map<VertexID, std::unordered_set<VertexID>> &remove_succ){
        std::unordered_set<VertexID> pred_dgraph_vertices,succ_dgraph_vertices;
        for (auto v : dgraph.GetAllVerticesID()) {
            if (dgraph.GetOutDegree(v)!=0){
                pred_dgraph_vertices.insert(v);
            }
            if (dgraph.GetInDegree(v)!=0){
                succ_dgraph_vertices.insert(v);
            }
        }
       //initial sim unordered_set
       for(auto u : qgraph.GetAllVerticesID()){
            sim[u] = std::unordered_set<VertexID>();
            remove_pred[u] = std::unordered_set<VertexID>();
            remove_succ[u] = std::unordered_set<VertexID>();
            if (initialized_sim==false){
                if (qgraph.GetOutDegree(u) == 0 && qgraph.GetInDegree(u) ==0){
                    for (auto v : dgraph.GetAllVerticesID()){
                        if (qgraph.GetVertexLabel(u)==dgraph.GetVertexLabel(v)){
                            sim[u].insert(v);
                        }
                    }
                }
                else if (qgraph.GetOutDegree(u) != 0 && qgraph.GetInDegree(u) ==0){
                    for (auto v : dgraph.GetAllVerticesID()){
                        if (qgraph.GetVertexLabel(u)==dgraph.GetVertexLabel(v) && dgraph.GetOutDegree(v) != 0){
                            sim[u].insert(v);
                        }
                    }
                }
                else if (qgraph.GetOutDegree(u) == 0 && qgraph.GetInDegree(u) !=0){
                    for (auto v : dgraph.GetAllVerticesID()){
                        if (qgraph.GetVertexLabel(u)==dgraph.GetVertexLabel(v) && dgraph.GetInDegree(v) != 0){
                            sim[u].insert(v);
                        }
                    }
                }
                else {
                    for (auto v : dgraph.GetAllVerticesID()){
                        if (qgraph.GetVertexLabel(u)==dgraph.GetVertexLabel(v) && dgraph.GetOutDegree(v) != 0 && dgraph.GetInDegree(v) != 0){
                            sim[u].insert(v);
                         }
                      }
                 }
                 /**
                  *
                  inital all outer node label is same to query graph is true
                 */
                for(auto v : *OuterVertices){
                    VertexID localid = fragment.getLocalID(v);
                    if(qgraph.GetVertexLabel(u)==dgraph.GetVertexLabel(localid)){
                         sim[u].insert(localid);
                    }
                }
             }

        //inital remove_pred,remove_succ
            std::unordered_set<VertexID> pt1,pt2;
            for (auto w : sim[u]){
                for(auto v : dgraph.GetParentsID(w)){
                    pt1.insert(v);
                }
                for (auto v : dgraph.GetChildrenID(w)){
                    pt2.insert(v);
                }
            }
            remove_pred[u] = diff(pred_dgraph_vertices,pt1);
            remove_succ[u] = diff(succ_dgraph_vertices,pt2);
       }
    }


void Dual_Parallel::reunordered_map_data_id(std::unordered_map<VertexID, VertexID> &t_f,Graph &dgraph){
        VertexID fid = 0;
        for (auto v : dgraph.GetAllVerticesID()){
            t_f[v] = fid;
            fid += 1;
        }
    }

void Dual_Parallel::dual_counter_initialization(Graph &dgraph, Graph &qgraph,
                                     std::unordered_map<VertexID, std::vector<int>> &sim_counter_post,
                                     std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre,
                                     std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim){
        for (auto w : dgraph.GetAllVerticesID()){
            sim_counter_post[w] = std::vector<int>(qgraph.GetNumVertices(), 0);
            sim_counter_pre[w] = std::vector<int>(qgraph.GetNumVertices(), 0);
            for (auto u : qgraph.GetAllVerticesID()){
                int len_des=0,len_pre=0;
                for (auto des_w : dgraph.GetChildrenID(w)){
                     if(sim[u].find(des_w) != sim[u].end()){
                        len_des += 1;
                     }
                }
                for (auto pre_w : dgraph.GetParentsID(w)){
                    if (sim[u].find(pre_w) != sim[u].end()){
                        len_pre+=1;
                    }
                }
                sim_counter_post[w][u] = len_des;
                sim_counter_pre[w][u] = len_pre;
            }
        }
    }

VertexID Dual_Parallel::find_nonempty_remove(Graph &qgraph,
                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &remove_pred,
                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &remove_succ){
        for (auto u : qgraph.GetAllVerticesID())
        {
            if(remove_pred[u].size() !=0){
                return u;
            }
            if (remove_succ[u].size() !=0){
                return u;
            }
        }
        return -1;
    }

void Dual_Parallel::update_sim_counter(Graph &dgraph,
                            std::unordered_map<VertexID, std::vector<int>> &sim_counter_post,
                            std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre,
                            VertexID u,VertexID w){
    for (auto wp : dgraph.GetParentsID(w)){
        if (sim_counter_post[wp][u] > 0){
            --sim_counter_post[wp][u];
        }
    }
    for (auto ws : dgraph.GetChildrenID(w)){
        if (sim_counter_pre[ws][u] > 0){
            --sim_counter_pre[ws][u];
        }

    }
    }

bool Dual_Parallel::match_check(Graph &qgraph,std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim){
        for (auto u : qgraph.GetAllVerticesID()){
            if (sim[u].size() == 0){
                return false;
            }
        }
        return true;
    }

bool Dual_Parallel::dual_sim_output(Graph &qgraph,std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim){
        if (match_check(qgraph, sim) == false){
           for(auto u : qgraph.GetAllVerticesID()){
                sim[u].clear();
           }
        }

    }


void Dual_Parallel::dual_sim_refinement(Fragment &fragment, Graph &dgraph, Graph &qgraph,
                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim,
                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &remove_pred,
                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &remove_succ,
                           std::unordered_map<VertexID, std::vector<int>> &sim_counter_post,
                           std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre){

        VertexID u= find_nonempty_remove(qgraph, remove_pred, remove_succ);
        while(u!=-1){
            if(remove_pred[u].size() !=0){
                for (auto u_p : qgraph.GetParentsID(u)){
                    for (auto w_pred : remove_pred[u]){
                        if (sim[u_p].find(w_pred)!=sim[u_p].end() && OuterVertices->find(fragment.getGlobalID(w_pred)) == OuterVertices->end()){
                            sim[u_p].erase(w_pred);
                            if (fragment.isBorderVertex(fragment.getGlobalID(w_pred))){
                                std::bitset<NUM_FRAGMENTS> dest= fragment.getMsgThroughDest(fragment.getGlobalID(w_pred));
                                for (int fid = 0; fid < dest.size(); fid++){
                                    if (dest.test(fid) && fid != get_worker_id()){
                                        messageBuffers.add_message(fid,std::pair<VertexID,int>(fragment.getGlobalID(w_pred), u_p));
                                     }
                                 }
                            }
                            update_sim_counter(dgraph,sim_counter_post, sim_counter_pre, u_p, w_pred);
                            for (auto w_pp : dgraph.GetParentsID(w_pred)){
                                if (sim_counter_post[w_pp][u_p] == 0){
                                    remove_pred[u_p].insert(w_pp);
                                }
                            }
                            for(auto w_ps : dgraph.GetChildrenID(w_pred)){
                                if (sim_counter_pre[w_ps][u_p] == 0){
                                    remove_succ[u_p].insert(w_ps);
                                }
                            }
                        }
                    }
                }
                remove_pred[u].clear();
            }

            if(remove_succ[u].size() != 0){
                for (auto u_s : qgraph.GetChildrenID(u)){
                    for (auto w_succ : remove_succ[u]){
                        if (sim[u_s].find(w_succ) != sim[u_s].end() && OuterVertices->find(fragment.getGlobalID(w_succ)) == OuterVertices->end()){
                            sim[u_s].erase(w_succ);
                           if (fragment.isBorderVertex(fragment.getGlobalID(w_succ))){
                                std::bitset<NUM_FRAGMENTS> dest= fragment.getMsgThroughDest(fragment.getGlobalID(w_succ));
                                for (int fid = 0; fid < dest.size(); fid++){
                                    if (dest.test(fid) && fid != get_worker_id()){
                                        messageBuffers.add_message(fid,std::pair<VertexID,int>(fragment.getGlobalID(w_succ), u_s));
                                     }
                                 }
                            }
                            update_sim_counter(dgraph,sim_counter_post, sim_counter_pre, u_s, w_succ);
                            for (auto w_sp : dgraph.GetParentsID(w_succ)){
                                if (sim_counter_post[w_sp][u_s] == 0){
                                    remove_pred[u_s].insert(w_sp);
                                }
                            }
                            for (auto w_ss : dgraph.GetChildrenID(w_succ)){
                                if (sim_counter_pre[w_ss][u_s] == 0){
                                    remove_succ[u_s].insert(w_ss);
                                }
                            }
                        }
                    }

                }
                remove_succ[u].clear();
            }

            u = find_nonempty_remove(qgraph, remove_pred, remove_succ);
        }
    }



bool Dual_Parallel::is_continue(){
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

void Dual_Parallel::dual_paraller(Fragment &fragment, Graph &dgraph, Graph &qgraph, std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim){
    OuterVertices = const_cast<std::unordered_set<VertexID> *>(fragment.getOuterVertices());
    innerVertices = const_cast<std::unordered_set<VertexID> *>(fragment.getInnerVertices());
    worker_barrier();
    std::unordered_map<VertexID, std::unordered_set<VertexID>> remove_pred,remove_succ;
    std::unordered_map<VertexID, std::vector<int>> sim_counter_post,sim_counter_pre;
    pEval(fragment, dgraph, qgraph, sim, remove_pred, remove_succ, sim_counter_post, sim_counter_pre);
    worker_barrier();
    while(is_continue()){
        incEval(fragment, dgraph, qgraph, sim, remove_pred, remove_succ, sim_counter_post, sim_counter_pre);
        worker_barrier();
    }
    worker_barrier();
//    out_global_result(fragment,qgraph,sim);
}

void Dual_Parallel::pEval(Fragment &fragment, Graph &dgraph, Graph &qgraph,
                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim,
                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &remove_pred,
                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &remove_succ,
                           std::unordered_map<VertexID, std::vector<int>> &sim_counter_post,
                           std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre){

    bool initialized_sim = false;
    dual_sim_initialization(fragment, dgraph, qgraph, sim, initialized_sim, remove_pred, remove_succ);
    dual_counter_initialization(dgraph, qgraph, sim_counter_post, sim_counter_pre, sim);
    dual_sim_refinement(fragment, dgraph, qgraph, sim, remove_pred, remove_succ, sim_counter_post, sim_counter_pre);

    for(auto u:qgraph.GetAllVerticesID()){
        for(auto v:*innerVertices){
            if (qgraph.GetVertexLabel(u)==dgraph.GetVertexLabel(fragment.getLocalID(v)) && sim[u].find(fragment.getLocalID(v)) == sim[u].end()){
                if (fragment.isBorderVertex(v)){
                    std::bitset<NUM_FRAGMENTS> dest= fragment.getMsgThroughDest(v);
                    for (int fid = 0; fid < dest.size(); fid++){
                        if (dest.test(fid) && fid != get_worker_id()){
                            messageBuffers.add_message(fid,std::pair<VertexID,int>(v, u));
                            }
                     }
                 }
            }
        }
    }
    messageBuffers.sync_messages();

}

void Dual_Parallel::incEval(Fragment &fragment, Graph &dgraph, Graph &qgraph,
                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim,
                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &remove_pred,
                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &remove_succ,
                           std::unordered_map<VertexID, std::vector<int>> &sim_counter_post,
                           std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre){
    for (auto item :messageBuffers.get_messages()){
        VertexID u = item.second;
        VertexID w = fragment.getLocalID(item.first);
        if(sim[u].find(w) != sim[u].end()){
            sim[u].erase(w);
            update_sim_counter(dgraph,sim_counter_post, sim_counter_pre, u, w);
            for(auto w_pp : dgraph.GetParentsID(w)){
                if(sim_counter_post[w_pp][u] == 0){
                     remove_pred[u].insert(w_pp);
                }
            }
            for(auto w_ps: dgraph.GetChildrenID(w)){
                if(sim_counter_pre[w_ps][u] == 0){
                    remove_succ[u].insert(w_ps);
                }
            }
        }
    }
    messageBuffers.reset_in_messages();
    dual_sim_refinement(fragment, dgraph, qgraph, sim, remove_pred, remove_succ, sim_counter_post, sim_counter_pre);
    messageBuffers.sync_messages();
 }


 void Dual_Parallel::out_global_result(Fragment &fragment,  Graph &qgraph, std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim){
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
using namespace std;
