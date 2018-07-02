#include "cpp/dualsimulation.h"

DualSim::DualSim(){}

DualSim::~DualSim(){}

void DualSim::dual_sim_initialization(Graph &dgraph, Graph &qgraph,
                               std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim,
                               bool &initialized_sim,
                               std::unordered_map<VertexID, std::unordered_set<VertexID>> &remove_pred,
                               std::unordered_map<VertexID, std::unordered_set<VertexID>> &remove_succ){
       // std::cout<<"dual_sim_initialzation"<<std::endl;
        std::unordered_set<VertexID> pred_dgraph_vertices,succ_dgraph_vertices;
        //initial pred_dgraph_vertices ,succ_dgraph_vertices
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


void DualSim::reunordered_map_data_id(std::unordered_map<VertexID, VertexID> &t_f,Graph &dgraph){
        VertexID fid = 0;
        for (auto v : dgraph.GetAllVerticesID()){
            t_f[v] = fid;
            fid += 1;
        }
    }

void DualSim::dual_counter_initialization(Graph &dgraph, Graph &qgraph,
                                     std::unordered_map<VertexID, std::vector<int>> &sim_counter_post,
                                     std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre,
                                     std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim,
                                     std::unordered_map<VertexID, VertexID> &t_f){
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
                sim_counter_post[t_f[w]][u] = len_des;
                sim_counter_pre[t_f[w]][u] = len_pre;
            }
        }
    }

VertexID DualSim::find_nonempty_remove(Graph &qgraph,
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

void DualSim::update_sim_counter(Graph &dgraph,
                            std::unordered_map<VertexID, std::vector<int>> &sim_counter_post,
                            std::unordered_map<VertexID, std::vector<int>> &sim_counter_pre,
                            VertexID u,VertexID w,
                            std::unordered_map<VertexID, VertexID> &t_f){
    for (auto wp : dgraph.GetParentsID(w)){
        if (sim_counter_post[t_f[wp]][u] > 0){
            --sim_counter_post[t_f[wp]][u];
        }
    }
    for (auto ws : dgraph.GetChildrenID(w)){
        if (sim_counter_pre[t_f[ws]][u] > 0){
            --sim_counter_pre[t_f[ws]][u];
        }

    }
    }

bool DualSim::match_check(Graph &qgraph,std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim){
        for (auto u : qgraph.GetAllVerticesID()){
            if (sim[u].size() == 0){
                return false;
            }
        }
        return true;
    }

bool DualSim::dual_sim_output(Graph &qgraph,std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim){
        if (match_check(qgraph, sim) == false){
           for(auto u : qgraph.GetAllVerticesID()){
                sim[u].clear();
           }
        }

    }


void DualSim::dual_sim_refinement(Graph &dgraph, Graph &qgraph,
                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim,
                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &remove_pred,
                           std::unordered_map<VertexID, std::unordered_set<VertexID>> &remove_succ){
        //std::cout<<"dual_sim_refinement"<<std::endl;
        std::unordered_map<VertexID, std::vector<int>> sim_counter_post,sim_counter_pre;
        std::unordered_map<VertexID, VertexID> t_f;
        reunordered_map_data_id(t_f, dgraph);

        dual_counter_initialization(dgraph, qgraph, sim_counter_post, sim_counter_pre, sim,t_f);
        int len_pre=0,len_succ=0;
        VertexID u= find_nonempty_remove(qgraph, remove_pred, remove_succ);
        while(u!=-1){
            if(remove_pred[u].size() !=0){
                for (auto u_p : qgraph.GetParentsID(u)){
                    for (auto w_pred : remove_pred[u]){
                        if (sim[u_p].find(w_pred)!=sim[u_p].end()){
                            sim[u_p].erase(w_pred);
                            len_pre++;
                            //std::cout<<u_p<<' '<<w_pred<<std::endl;
                            update_sim_counter(dgraph,sim_counter_post, sim_counter_pre, u_p, w_pred, t_f);
                            for (auto w_pp : dgraph.GetParentsID(w_pred)){
                                if (sim_counter_post[t_f[w_pp]][u_p] == 0){
                                    remove_pred[u_p].insert(w_pp);
                                }
                            }
                            for(auto w_ps : dgraph.GetChildrenID(w_pred)){
                                if (sim_counter_pre[t_f[w_ps]][u_p] == 0){
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
                        if (sim[u_s].find(w_succ) != sim[u_s].end()){
                            sim[u_s].erase(w_succ);
                            len_succ++;
                            //std::cout<<u_s<<' '<<w_succ<<std::endl;
                            update_sim_counter(dgraph,sim_counter_post, sim_counter_pre, u_s, w_succ, t_f);
                            for (auto w_sp : dgraph.GetParentsID(w_succ)){
                                if (sim_counter_post[t_f[w_sp]][u_s] == 0){
                                    remove_pred[u_s].insert(w_sp);
                                }
                            }
                            for (auto w_ss : dgraph.GetChildrenID(w_succ)){
                                if (sim_counter_pre[t_f[w_ss]][u_s] == 0){
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

void DualSim::dual_simulation(Graph &dgraph, Graph &qgraph, std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim, bool &initialized_sim){
        //std::cout<<"begin dual"<<std::endl;
        std::unordered_map<VertexID, std::unordered_set<VertexID>> remove_pred,remove_succ;
        dual_sim_initialization(dgraph, qgraph, sim, initialized_sim, remove_pred, remove_succ);
        dual_sim_refinement(dgraph, qgraph, sim, remove_pred, remove_succ);
        dual_sim_output(qgraph, sim);
  }

using namespace std;
