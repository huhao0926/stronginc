#ifndef CPP_GRAPHVIEW_H_
#define CPP_GRAPHVIEW_H_

#include <boost/graph/filtered_graph.hpp>

#include <unordered_set>

#include "cpp/core/graph.h"

/**
 * Filte predicator for edges
 */
struct EdgePredicate {
  EdgePredicate() = default;
  EdgePredicate(std::unordered_set<Edge>* eSet, std::shared_ptr<GraphT> &graph):
                eSet_(eSet), graph_(graph) {}
  EdgePredicate(std::unordered_set<VertexID>* vSet, std::shared_ptr<GraphT> &graph):
                vSet_(vSet), graph_(graph) {}

  bool operator()(const EdgeDescriptor& e) const{
    if (eSet_ != NULL) {
      VertexID src = get(boost::vertex_index, *graph_)[source(e, *graph_)];
      VertexID dst = get(boost::vertex_index, *graph_)[target(e, *graph_)];
      EdgeLabel attr = get(boost::edge_weight, *graph_)[e];
      Edge edge(src, dst, attr);
      return (*eSet_).find(edge) != (*eSet_).end();
    } else {
      VertexID src = get(boost::vertex_index, *graph_)[source(e, *graph_)];
      VertexID dst = get(boost::vertex_index, *graph_)[target(e, *graph_)];
      return ((*vSet_).find(src) != (*vSet_).end()) &&
             ((*vSet_).find(dst) != (*vSet_).end());
    }
  }

  std::unordered_set<Edge>* eSet_;
  std::unordered_set<VertexID>* vSet_;
  std::shared_ptr<GraphT> graph_;
};

/**
 * Filte predicatot for vertices
 */
struct VertexPredicate {
  VertexPredicate() = default;
  VertexPredicate(std::unordered_set<VertexID>* vSet, std::shared_ptr<GraphT> &graph):
                  vSet_(vSet), graph_(graph) {

  }
  bool operator()(const VertexDescriptor& v) const{
    VertexID vid = get(boost::vertex_index, *graph_)[v];
    return (*vSet_).find(vid) != (*vSet_).end();
  }

  std::unordered_set<VertexID>* vSet_;
  std::shared_ptr<GraphT> graph_;
};

typedef boost::filtered_graph<GraphT, EdgePredicate, VertexPredicate>
    SubGraphT;

/**
* typename of vertex index map
*/
typedef typename boost::property_map<SubGraphT, boost::vertex_index_t>::type
    ViewVertexIndexMap;
/**
* typename of edge data property map
*/
typedef typename boost::property_map<SubGraphT, boost::edge_weight_t>::type
    ViewEdgeDataMap;
/**
* type name of vertex descriptor
*/
typedef typename boost::graph_traits<SubGraphT>::vertex_descriptor
    ViewVertexDescriptor;
/**
* tpyename of all edge descriptor
*/
typedef typename boost::graph_traits<SubGraphT>::edge_descriptor
    ViewEdgeDescriptor;
/**
* typename of in-edge descriptor
*/
typedef typename SubGraphT::in_edge_iterator ViewInEdgeDescripterIter;
/**
* typename of out-edge descriptor
*/
typedef typename SubGraphT::out_edge_iterator ViewOutEdgeDescripterIter;


class GraphView {
 friend class Graph;
 public:
  GraphView();
  GraphView(Graph &graph, std::unordered_set<Edge>* eSet,
                          std::unordered_set<VertexID>* vSet);
  GraphView(Graph &graph, std::unordered_set<VertexID>* vSet);
  ~GraphView();

  /**
   * iterator of outgoing edges for a certain vertex.
   * for children vertices iteration.
   */
  class ViewOutgoingEdgeIterator {
   public:
    ViewOutgoingEdgeIterator(const GraphView &graph,
                         const ViewOutEdgeDescripterIter &iter,
                         const VertexID &uid);
    friend class GraphView;
    const Edge operator*() const;
    const bool operator!=(const ViewOutgoingEdgeIterator &oeIter) const;
    ViewOutgoingEdgeIterator &operator++();

   private:
    const GraphView &graph_;
    const VertexID uid_;
    ViewOutEdgeDescripterIter iter_;
  };

  /**
   * iterator of incoming edges for a certain vertex.
   * for parent vertices iteration.
   */
  class ViewIncomingEdgeIterator {
   public:
    ViewIncomingEdgeIterator(const GraphView &graph,
                         const ViewInEdgeDescripterIter &iter,
                         const VertexID &vid);
    friend class GraphView;
    const Edge operator*() const;
    const bool operator!=(const ViewIncomingEdgeIterator &ieIter) const;
    ViewIncomingEdgeIterator &operator++();

   private:
    const GraphView &graph_;
    const VertexID vid_;
    ViewInEdgeDescripterIter iter_;
  };

  class ViewEdgeIterator {
    typedef typename boost::graph_traits<SubGraphT>::edge_iterator
        ViewEdgeDescripterIter;
   public:
    friend class GraphView;
    ViewEdgeIterator(const GraphView &graph, const ViewEdgeDescripterIter &iter);
    const Edge operator*() const;
    const bool operator!=(const ViewEdgeIterator &eIter) const;
    ViewEdgeIterator &operator++();

   private:
    const GraphView &graph_;
    ViewEdgeDescripterIter iter_;
  };

   /**
   * iterator of vertexID for the graph;
   */
  class ViewVertexIDIterator {
    typedef typename boost::graph_traits<SubGraphT>::vertex_iterator
        ViewVertexDescripterIter;

   public:
    friend class GraphView;
    ViewVertexIDIterator(const GraphView &graph,
                     const ViewVertexDescripterIter &iter);
    const VertexID operator*() const;
    const bool operator!=(const ViewVertexIDIterator &cvIter) const;
    ViewVertexIDIterator &operator++();

   private:
    const GraphView &graph_;
    ViewVertexDescripterIter iter_;
  };

  /**
   * iterator of children vertices for a certain vertex
   */
  class ViewChildrenIDIterator {
    typedef typename boost::graph_traits<SubGraphT>::adjacency_iterator
        ViewVertexDescripterIter;

   public:
    friend class GraphView;
    ViewChildrenIDIterator(const GraphView &graph,
                       const ViewVertexDescripterIter &iter);
    const VertexID operator*() const;
    const bool operator!=(const ViewChildrenIDIterator &cvIter) const;
    ViewChildrenIDIterator &operator++();

   private:
    const GraphView &graph_;
    ViewVertexDescripterIter iter_;
  };

  /**
   * iterator of parent vertices for a certain vertex
   */
  class ViewParentsIDIterator {
   public:
    explicit ViewParentsIDIterator(const ViewIncomingEdgeIterator &iter);
    const VertexID operator*() const;
    const bool operator!=(const ViewParentsIDIterator &pvIter) const;
    ViewParentsIDIterator &operator++();

   private:
    ViewIncomingEdgeIterator iter_;
  };

  /**
   * @return vertex size in the graph
   */
  inline size_t GetNumVertices() const {
    return num_vertices_;
  }

  /**
   * @return edge size in the graph
   */
  inline size_t GetNumEdges() const {
    return num_edges_;
  }

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
  IterContainer<ViewOutgoingEdgeIterator> GetOutgoingEdges(const VertexID uid);

  /**
   * get incoming edges for a certain vertex.
   * @return a pair, containing the first and last iterator.
   */
  IterContainer<ViewIncomingEdgeIterator> GetIncomingEdges(const VertexID vid);
  /**
   * get children vertices for a certain vertex.
   * @return a pair, containing the first and last iterator.
   */
  IterContainer<ViewChildrenIDIterator> GetChildrenID(const VertexID uid);

  /**
   * get parent vertices for a certain vertex.
   * @return a pair, containing the first and last iterator.
   */
  IterContainer<ViewParentsIDIterator> GetParentsID(const VertexID vid);

  /**
   * get all vertices in the current graph.
   * @return a pair, containing the first and last iterator.
   */
  IterContainer<ViewVertexIDIterator> GetAllVerticesID();


  /**
   * get all edges in the current graph.
   * @return a pair, containing the first and last iterator.
   */
  IterContainer<ViewEdgeIterator> GetAllEdges();

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

 private:
  std::unique_ptr<SubGraphT> graphview_;
  std::shared_ptr<GraphT> graph_;
  VertexIndexMap vertex_index_map_;
  std::unique_ptr<EdgeDataMap> edge_data_map_;

  size_t num_vertices_;
  size_t num_edges_;
};

#endif //CPP_GRAPHVIEW_H_