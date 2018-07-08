#include "ball_view.h"

Ball_View::Ball_View(){}

Ball_View::Ball_View(std::unordered_set<VertexID> &vertices,std::unordered_set<Edge>  &edges):vertices_(vertices),edges_(edges){
    for(auto u : vertices){
        childrens_[u] = std::unordered_set<VertexID>();
        parents_[u] = std::unordered_set<VertexID>();
    }
    for (auto e :edges){
        childrens_[e.src()].insert(e.dst());
        parents_[e.dst()].insert(e.src());
    }
}

Ball_View::~Ball_View(){}

std::unordered_set<VertexID>&  Ball_View::GetAllVerticesID(){
     return vertices_;
}

std::unordered_set<Edge>& Ball_View::GetAllEdges(){
    return edges_;
}

std::unordered_set<VertexID>&  Ball_View::GetParentsID(const VertexID w){
    return parents_[w];
}


std::unordered_set<VertexID>&  Ball_View::GetChildrenID(const VertexID w){
    return childrens_[w];
}

bool Ball_View::ExistEdge(const VertexID src, const VertexID dst){
    return edges_.find(Edge(src,dst,1)) !=edges_.end();

}
