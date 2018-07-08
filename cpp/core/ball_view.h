#ifndef CPP_BALLVIEW_H_
#define CPP_BALLVIEW_H_
#include "cpp/core/edge.h"
#include "cpp/core/vertex.h"
#include<unordered_set>
#include<unordered_map>
/*create a view graph for base graph,just fileter and select partial nodes and edges info from base graph.
 */
class Ball_View{
public:
    Ball_View();

    ~Ball_View();

    Ball_View(std::unordered_set<VertexID> &vertices,std::unordered_set<Edge>  &edges);

    std::unordered_set<VertexID>&  GetAllVerticesID();

    std::unordered_set<Edge>& GetAllEdges();
   /**
    *get all parents id for a node
    */
    std::unordered_set<VertexID>&  GetParentsID(const VertexID w);
   /**
    *get all childrens id for a node
    */
    std::unordered_set<VertexID>&  GetChildrenID(const VertexID w);
   /**
    *if view exsist a edge
    */
    bool ExistEdge(VertexID src, VertexID dst);

private:
/*
   **vertices_ partial nodes
   **edges_ partial edges
   **childrens_ recored children nodes info for a node
   **parents_  recored parent nodes info for a node
   */
     std::unordered_set<VertexID> vertices_;
     std::unordered_set<Edge>  edges_;
     std::unordered_map<VertexID,std::unordered_set<VertexID>> childrens_;
     std::unordered_map<VertexID,std::unordered_set<VertexID>> parents_;

};
#endif //CPP_BALLVIEW_H_
using namespace std;