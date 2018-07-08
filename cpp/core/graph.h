#ifndef CPP_GRAPH_H_
#define CPP_GRAPH_H_

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/compressed_sparse_row_graph.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <boost/graph/properties.hpp>

#include <vector>
#include <memory>
#include <unordered_set>
#include <queue>
#include <iostream>

#include "cpp/core/config.h"
#include "cpp/core/vertex.h"
#include "cpp/core/edge.h"
#include "cpp/core/iterator_container.h"

typedef boost::adjacency_list<boost::vecS, boost::vecS,
                              boost::bidirectionalS,
                              boost::no_property,
                              boost::property<boost::edge_weight_t, int> >
    AdjGraph;

typedef boost::compressed_sparse_row_graph
        <boost::bidirectionalS, boost::vertex_index_t,
         boost::property<boost::edge_weight_t, int> >
    CsrGraph;
/**
 * csrGraph: more effective, not mutable
 * adjGraph: mutable
 */
#ifndef MUTABLE_GRAPH
typedef CsrGraph GraphT;
#else
typedef AdjGraph GraphT;
#endif

/**
 * typename of vertex index map
 */
typedef typename boost::property_map<GraphT, boost::vertex_index_t>::type
    VertexIndexMap;
/**
 * typename of edge data property map
 */
typedef typename boost::property_map<GraphT, boost::edge_weight_t>::type
    EdgeDataMap;
/**
 * type name of vertex descriptor
 */
typedef typename boost::graph_traits<GraphT>::vertex_descriptor
    VertexDescriptor;
/**
 * tpyename of all edge descriptor
 */
typedef typename boost::graph_traits<GraphT>::edge_descriptor
    EdgeDescriptor;
/**
 * typename of in-edge descriptor
 */
typedef typename GraphT::in_edge_iterator InEdgeDescripterIter;
/**
 * typename of out-edge descriptor
 */
typedef typename GraphT::out_edge_iterator OutEdgeDescripterIter;

class Graph {
 friend class GraphLoader;
 friend class GraphView;
 friend class Fragment;
 public:
  Graph();
  ~Graph();

 public:
  /**
   * iterator of outgoing edges for a certain vertex.
   * for children vertices iteration.
   */
  class OutgoingEdgeIterator {
   public:
    OutgoingEdgeIterator(const Graph &graph,
                         const OutEdgeDescripterIter &iter,
                         const VertexID &uid);
    friend class Graph;
    const Edge operator*() const;
    const bool operator!=(const OutgoingEdgeIterator &oeIter) const;
    OutgoingEdgeIterator &operator++();

   private:
    const Graph &graph_;
    const VertexID uid_;
    OutEdgeDescripterIter iter_;
  };

  /**
   * iterator of incoming edges for a certain vertex.
   * for parent vertices iteration.
   */
  class IncomingEdgeIterator {
   public:
    IncomingEdgeIterator(const Graph &graph,
                         const InEdgeDescripterIter &iter,
                         const VertexID &vid);
    friend class graph;
    const Edge operator*() const;
    const bool operator!=(const IncomingEdgeIterator &ieIter) const;
    IncomingEdgeIterator &operator++();

   private:
    const Graph &graph_;
    const VertexID vid_;
    InEdgeDescripterIter iter_;
  };

  class EdgeIterator {
    typedef typename boost::graph_traits<GraphT>::edge_iterator
        EdgeDescripterIter;
   public:
    friend class Graph;
    EdgeIterator(const Graph &graph, const EdgeDescripterIter &iter);
    const Edge operator*() const;
    const bool operator!=(const EdgeIterator &eIter) const;
    EdgeIterator &operator++();

   private:
    const Graph &graph_;
    EdgeDescripterIter iter_;
  };

   /**
   * iterator of vertexID for the graph;
   */
  class VertexIDIterator {
    typedef typename boost::graph_traits<GraphT>::vertex_iterator
        VertexDescripterIter;

   public:
    friend class Graph;
    VertexIDIterator(const Graph &graph,
                     const VertexDescripterIter &iter);
    const VertexID operator*() const;
    const bool operator!=(const VertexIDIterator &cvIter) const;
    VertexIDIterator &operator++();

   private:
    const Graph &graph_;
    VertexDescripterIter iter_;
  };

  /**
   * iterator of children vertices for a certain vertex
   */
  class ChildrenIDIterator {
    typedef typename boost::graph_traits<GraphT>::adjacency_iterator
        VertexDescripterIter;

   public:
    friend class Graph;
    ChildrenIDIterator(const Graph &graph,
                       const VertexDescripterIter &iter);
    const VertexID operator*() const;
    const bool operator!=(const ChildrenIDIterator &cvIter) const;
    ChildrenIDIterator &operator++();

   private:
    const Graph &graph_;
    VertexDescripterIter iter_;
  };

  /**
   * iterator of parent vertices for a certain vertex
   */
  class ParentsIDIterator {
   public:
    explicit ParentsIDIterator(const IncomingEdgeIterator &iter);
    const VertexID operator*() const;
    const bool operator!=(const ParentsIDIterator &pvIter) const;
    ParentsIDIterator &operator++();

   private:
    IncomingEdgeIterator iter_;
  };

  /**
   * @return vertex size in the graph
   */
  inline size_t GetNumVertices() const {
    return boost::num_vertices(*graph_);
  }

  /**
   * @return edge size in the graph
   */
  inline size_t GetNumEdges() const {
    return boost::num_edges(*graph_);
  }

  /**
   * get vertex label
   * @param vertex id
   * @return vertex label
   */
  VertexLabel &GetVertexLabel(const VertexID vid);

  /**
   * @return whether exists an edge in the two vertices.
   */
  const bool ExistEdge(const VertexID src, const VertexID dst) const;

  // TODO: remove me, add a func return degree.
  /**
   * has Child
   */
  const bool HasChild(const VertexID vid) const;

  /**
   * get outgoing edges for a certain vertex.
   * @return a pair, containing the first and last iterator.
   */
  IterContainer<OutgoingEdgeIterator> GetOutgoingEdges(const VertexID uid);

  /**
   * get incoming edges for a certain vertex.
   * @return a pair, containing the first and last iterator.
   */
  IterContainer<IncomingEdgeIterator> GetIncomingEdges(const VertexID vid);
  /**
   * get children vertices for a certain vertex.
   * @return a pair, containing the first and last iterator.
   */
  IterContainer<ChildrenIDIterator> GetChildrenID(const VertexID uid);

  /**
   * get parent vertices for a certain vertex.
   * @return a pair, containing the first and last iterator.
   */
  IterContainer<ParentsIDIterator> GetParentsID(const VertexID vid);

  /**
   * get all vertices in the current graph.
   * @return a pair, containing the first and last iterator.
   */
  IterContainer<VertexIDIterator> GetAllVerticesID();


  /**
   * get all edges in the current graph.
   * @return a pair, containing the first and last iterator.
   */
  IterContainer<EdgeIterator> GetAllEdges();

  /**
   * get out degree number of one vertex
   * @return out-degree number
   */
  const int GetOutDegree(const VertexID vid) const;

  /**
   * get in degree number of one vertex
   * @return in-degree number
   */
  const int GetInDegree(const VertexID vid) const;

  /**
   * get degree number of one vertex
   * @return degree number
   */
  const int GetDegree(const VertexID vid) const;

  /**
  *get distance from vid to all nodes in undirected  graph
  un_reached return INT_MAX
  */
  void shortest_distance(VertexID vid,std::unordered_map<VertexID,int> &dis);

  /**
   * get distance from u to v in undirected graph
   un_reached return INT_MAX
   */
   const int shortest_distance(VertexID source_id,VertexID target_id)const;

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
#ifdef MUTABLE_GRAPH
  /**
   * add a new vertex v to the graph
   * @param v the new vertex
   */
  void AddVertex(const Vertex &v);

  /**
   * add a new edge e to the graph
   * @param e the new edge
   */
  void AddEdge(const Edge &e);

  /**
   * delete a edge e to the graph
   * @param e the exist edge
   */
  void RemoveEdge(const Edge &e);

  /**
   * refresh the graph property after adding all vertices and edges
   */
  void RebuildGraphProperties();
#endif

 private:
  std::shared_ptr<GraphT> graph_;
  VertexIndexMap vertex_index_map_;
  std::unique_ptr<EdgeDataMap> edge_data_map_;
  std::vector<VertexLabel> vertices_label_;
  int num_vertices_;
  int num_edges_;

};
#endif //CPP_GRAPH_H_