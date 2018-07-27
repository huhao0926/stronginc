#ifndef CPP_BALLVIEW_H_
#define CPP_BALLVIEW_H_
#include "cpp/core/edge.h"
#include "cpp/core/vertex.h"
#include<unordered_set>
#include<unordered_map>
#include <queue>
/*create a view graph for base graph,just fileter and select partial nodes and edges info from base graph.
 */
class Ball_View{
public:
    Ball_View();

    ~Ball_View();

    /**
     * @return vertex size in the graph
     */
    inline size_t GetNumVertices() const {
      return vertices_.size();
    }

    /**
     * @return edge size in the graph
     */
    inline size_t GetNumEdges() const {
      return edges_.size();
    }

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

    /**
     *get distance from vid to all nodes in undirected  graph
     un_reached return INT_MAX
     */
    void shortest_distance(VertexID vid,std::unordered_map<VertexID,int> &dis);

    /**
     * get distance from u to v in undirected graph
       un_reached return INT_MAX
     */
    int shortest_distance(VertexID source_id,VertexID target_id);

    /**
     *find d_hop from node u in undirectd graph
     */
    void find_hop_nodes(VertexID vid, int d_hop,std::unordered_set<VertexID> &result);

    /**
     *find connetivity nodes from u in graph
     */
    void  find_connectivity_nodes(VertexID vid, std::unordered_set<VertexID> &result);

    /**
     *find d_hop from node set in undirected graph
     */
    void find_hop_nodes(std::unordered_set<VertexID> &node_set,int d_hop,std::unordered_set<VertexID> &result);
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