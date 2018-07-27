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

void Ball_View::shortest_distance(VertexID vid,std::unordered_map<VertexID,int> &dis){
    //VertexID root =vid;
    if(vertices_.find(vid) == vertices_.end()){
        return ;
    }
    std::unordered_set<VertexID> color;
    std::queue<VertexID> q;
    q.push(vid);
    color.insert(vid);
    dis[vid] = 0;
    while(!q.empty()){
       VertexID root = q.front();
       q.pop();
       for(auto v_p : parents_[root]){
           if(color.find(v_p) == color.end()){
               q.push(v_p);
               color.insert(v_p);
               dis[v_p] = dis[root] +1;
           }
       }
       for(auto v_s : childrens_[root]){
           if(color.find(v_s) == color.end()){
               q.push(v_s);
               color.insert(v_s);
               dis[v_s] = dis[root] +1;
            }
       }
    }
   for(auto v : vertices_){
       if(color.find(v) == color.end()){
           dis[v] = INT_MAX;
       }
   }
}

int Ball_View::shortest_distance(VertexID source_id,VertexID target_id){
    if(vertices_.find(source_id) == vertices_.end() || vertices_.find(target_id) == vertices_.end()){
        return -1;
    }
    std::unordered_set<VertexID> color;
    std::unordered_map<VertexID,int> dis;
    std::queue<VertexID> q;
    q.push(source_id);
    color.insert(source_id);
    dis[source_id] = 0;
    while(!q.empty()){
       VertexID root = q.front();
       q.pop();
       for(auto v_p : parents_[root]){
           if(v_p == target_id){
               return dis[root] +1;
           }
           if(color.find(v_p) == color.end()){
               q.push(v_p);
               color.insert(v_p);
               dis[v_p] = dis[root] +1;
           }
       }
       for(auto v_s : childrens_[root]){
           if(v_s == target_id){
               return dis[root] +1;
           }
           if(color.find(v_s) == color.end()){
               q.push(v_s);
               color.insert(v_s);
               dis[v_s] = dis[root] +1;
            }
       }
    }
    return INT_MAX;
}

void Ball_View::find_hop_nodes(VertexID vid, int d_hop,std::unordered_set<VertexID> &result){
    if(vertices_.find(vid) == vertices_.end()){
        return ;
    }
    std::unordered_set<VertexID> color;
    std::unordered_map<VertexID,int> dis;
    std::queue<VertexID> q;
    q.push(vid);
    color.insert(vid);
    dis[vid] =0;
    result.insert(vid);
    while(!q.empty()){
       VertexID root = q.front();
       if(dis[root] == d_hop){
           return;
       }
       q.pop();
       for(auto v_p : parents_[root]){
           if(color.find(v_p) == color.end()){
               q.push(v_p);
               color.insert(v_p);
               dis[v_p] = dis[root] +1;
               result.insert(v_p);
           }
       }
       for(auto v_s : childrens_[root]){
           if(color.find(v_s) == color.end()){
               q.push(v_s);
               color.insert(v_s);
               dis[v_s] = dis[root] +1;
               result.insert(v_s);
            }
       }
    }
}

void  Ball_View::find_connectivity_nodes(VertexID vid, std::unordered_set<VertexID> &result){
    if(vertices_.find(vid) == vertices_.end()){
        return ;
    }
    std::unordered_set<VertexID> color;
    std::unordered_map<VertexID,int> dis;
    std::queue<VertexID> q;
    q.push(vid);
    color.insert(vid);
    dis[vid] =0;
    result.insert(vid);
    while(!q.empty()){
       VertexID root = q.front();
       q.pop();
       for(auto v_p : parents_[root]){
           if(color.find(v_p) == color.end()){
               q.push(v_p);
               color.insert(v_p);
               dis[v_p] = dis[root] +1;
               result.insert(v_p);
           }
       }
       for(auto v_s : childrens_[root]){
           if(color.find(v_s) == color.end()){
               q.push(v_s);
               color.insert(v_s);
               dis[v_s] = dis[root] +1;
               result.insert(v_s);
            }
       }
    }
}


void Ball_View::find_hop_nodes(std::unordered_set<VertexID> &node_set,int d_hop,std::unordered_set<VertexID> &result){
    std::unordered_set<VertexID> color;
    std::unordered_map<VertexID,int> dis;
    std::queue<VertexID> q;
    for(auto vid :node_set){
        if(vertices_.find(vid) == vertices_.end()){
            continue;
        }
        q.push(vid);
        color.insert(vid);
        dis[vid] =0;
        result.insert(vid);
    }
    while(!q.empty()){
       VertexID root = q.front();
       if(dis[root] == d_hop){
           return;
       }
       q.pop();
       for(auto v_p : parents_[root]){
           if(color.find(v_p) == color.end()){
               q.push(v_p);
               color.insert(v_p);
               dis[v_p] = dis[root] +1;
               result.insert(v_p);
           }
       }
       for(auto v_s : childrens_[root]){
           if(color.find(v_s) == color.end()){
               q.push(v_s);
               color.insert(v_s);
               dis[v_s] = dis[root] +1;
               result.insert(v_s);
            }
       }
    }

}
