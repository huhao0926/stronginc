#include "view.h"

View::View(){}

View::~View(){}

void View::add_ViewGraph(Graph* view){
    ViewGraph_list.push_back(view);
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

void View::clear_parameter(){
}