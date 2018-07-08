#include "graph_view.h"

GraphView::GraphView() {}

GraphView::GraphView(Graph &graph, std::unordered_set<Edge>* eSet,
                                   std::unordered_set<VertexID>* vSet) {
  LOG(INFO) << "Start building GraphView "
            << "according to given edges and vertices..." << std::endl;
  num_vertices_ = (*vSet).size();
  num_edges_ = (*eSet).size();
  graph_ = graph.graph_;
  EdgePredicate edgeP(eSet, graph_);
  VertexPredicate vertexP(vSet, graph_);
  graphview_ = std::unique_ptr<SubGraphT>(
      new SubGraphT(*graph_, edgeP, vertexP));

  vertex_index_map_ = boost::get(boost::vertex_index, *graphview_);
  edge_data_map_ = std::unique_ptr<EdgeDataMap>(
          new EdgeDataMap(boost::get(boost::edge_weight, *graphview_)));
  LOG(INFO) << "Build GraphView finished." << std::endl;
}

GraphView::GraphView(Graph &graph, std::unordered_set<VertexID>* vSet) {
  LOG(INFO) << "Start building GraphView "
            << "according to given verties..." << std::endl;
  num_vertices_ = (*vSet).size();
  graph_ = graph.graph_;
  EdgePredicate edgeP(vSet, graph_);
  VertexPredicate vertexP(vSet, graph_);
  graphview_ = std::unique_ptr<SubGraphT>(
      new SubGraphT(*graph_, edgeP, vertexP));

  vertex_index_map_ = boost::get(boost::vertex_index, *graphview_);
  edge_data_map_ = std::unique_ptr<EdgeDataMap>(
          new EdgeDataMap(boost::get(boost::edge_weight, *graphview_)));
  // if you don't want to cal the number of edges right now,
  // just make the under codes unworking.
  num_edges_ = 0;
  for (auto e : GetAllEdges()) {num_edges_ ++;}

  LOG(INFO) << "Build GraphView finished." << std::endl;
}

GraphView::~GraphView() {}

const bool GraphView::ExistEdge(const VertexID src, const VertexID dst) const {
  assert(src < num_vertices_&& dst < num_vertices_);
  return boost::edge(vertex(src, *graph_),
                     vertex(dst, *graph_), *graphview_).second;
}

const bool GraphView::HasChild(const VertexID vid) const {
  assert(vid < num_vertices_);
  ViewVertexDescriptor v = vertex(vid, *graph_);
  typedef typename boost::graph_traits<SubGraphT>::adjacency_iterator
      ViewVertexDescripterIter;
  ViewVertexDescripterIter iter_begin, iter_end;
  boost::tie(iter_begin, iter_end) = adjacent_vertices(v, *graphview_);
  return iter_begin != iter_end;
}

IterContainer<GraphView::ViewOutgoingEdgeIterator> GraphView::GetOutgoingEdges(
              const VertexID uid) {
  assert(uid < num_vertices_);
  ViewVertexDescriptor u = vertex(uid, *graph_);
  ViewOutEdgeDescripterIter iter_begin, iter_end;
  boost::tie(iter_begin, iter_end) = out_edges(u, *graphview_);
  ViewOutgoingEdgeIterator e_begin(*this, iter_begin, uid),
      e_end(*this, iter_end, uid);

  return IterContainer<ViewOutgoingEdgeIterator>(e_begin, e_end);
}

IterContainer<GraphView::ViewIncomingEdgeIterator> GraphView::GetIncomingEdges(
              const VertexID vid) {
  assert(vid < num_vertices_);
  ViewVertexDescriptor v = vertex(vid, *graph_);
  ViewInEdgeDescripterIter iter_begin, iter_end;
  boost::tie(iter_begin, iter_end) = in_edges(v, *graphview_);
  ViewIncomingEdgeIterator e_begin(*this, iter_begin, vid),
      e_end(*this, iter_end, vid);

  return IterContainer<ViewIncomingEdgeIterator>(e_begin, e_end);
}

IterContainer<GraphView::ViewChildrenIDIterator> GraphView::GetChildrenID(
              const VertexID uid) {
  assert(uid < num_vertices_);
  ViewVertexDescriptor u = vertex(uid, *graph_);
  typedef typename boost::graph_traits<SubGraphT>::adjacency_iterator
      ViewVertexDescripterIter;
  ViewVertexDescripterIter iter_begin, iter_end;
  boost::tie(iter_begin, iter_end) = adjacent_vertices(u, *graphview_);
  return IterContainer<ViewChildrenIDIterator>(
      ViewChildrenIDIterator(*this, iter_begin),
      ViewChildrenIDIterator(*this, iter_end));
}

IterContainer<GraphView::ViewParentsIDIterator> GraphView::GetParentsID(
              const VertexID vid){
  assert(vid < num_vertices_);
  ViewVertexDescriptor v = vertex(vid, *graph_);
  ViewInEdgeDescripterIter iter_begin, iter_end;
  boost::tie(iter_begin, iter_end) = in_edges(v, *graphview_);
  return IterContainer<ViewParentsIDIterator>(
      ViewParentsIDIterator(ViewIncomingEdgeIterator(*this, iter_begin, vid)),
      ViewParentsIDIterator(ViewIncomingEdgeIterator(*this, iter_end, vid)));
}

IterContainer<GraphView::ViewVertexIDIterator> GraphView::GetAllVerticesID() {
  typedef typename boost::graph_traits<SubGraphT>::vertex_iterator
      ViewVertexDescripterIter;
  ViewVertexDescripterIter iter_begin, iter_end;
  boost::tie(iter_begin, iter_end) = vertices(*graphview_);
  return IterContainer<ViewVertexIDIterator>(ViewVertexIDIterator(*this, iter_begin),
                                         ViewVertexIDIterator(*this, iter_end));
}

IterContainer<GraphView::ViewEdgeIterator> GraphView::GetAllEdges() {
  typedef typename boost::graph_traits<SubGraphT>::edge_iterator
      ViewEdgeDescripterIter;
  ViewEdgeDescripterIter iter_begin, iter_end;
  boost::tie(iter_begin, iter_end) = edges(*graphview_);
  return IterContainer<ViewEdgeIterator>(ViewEdgeIterator(*this, iter_begin),
                                     ViewEdgeIterator(*this, iter_end));
}

const int GraphView::GetOutDegree(const VertexID vid) const {
  ViewVertexDescriptor v = vertex(vid, *graph_);
  return out_degree(v, *graphview_);
}

const int GraphView::GetInDegree(const VertexID vid) const {
  ViewVertexDescriptor v = vertex(vid, *graph_);
  return in_degree(v, *graphview_);
}

const int GraphView::GetDegree(const VertexID vid) const {
  ViewVertexDescriptor v = vertex(vid, *graph_);
  return in_degree(v, *graphview_) + out_degree(v, *graphview_);
}

/**
 * sub-class OutgoingEdgeIterator
 */
GraphView::ViewOutgoingEdgeIterator::ViewOutgoingEdgeIterator
    (const GraphView &graph, const ViewOutEdgeDescripterIter &iter,
     const VertexID &uid) :
    graph_(graph), iter_(iter), uid_(uid) {}

const Edge GraphView::ViewOutgoingEdgeIterator::operator*() const {
  EdgeLabel attr = (*(graph_.edge_data_map_))[*iter_];
  VertexID vid = graph_.vertex_index_map_[target(*iter_, *(graph_.graphview_))];
  return Edge(uid_, vid, attr);
}

const bool GraphView::ViewOutgoingEdgeIterator::operator!=
    (const ViewOutgoingEdgeIterator &oeIter) const {
  return iter_ != oeIter.iter_;
}

GraphView::ViewOutgoingEdgeIterator &GraphView::ViewOutgoingEdgeIterator::operator++() {
  ++iter_;
  return *this;
}

/**
 * sub-class IncomingEdgeIterator
 */
GraphView::ViewIncomingEdgeIterator::ViewIncomingEdgeIterator
    (const GraphView &graph, const ViewInEdgeDescripterIter &iter,
     const VertexID &vid) :
    graph_(graph), iter_(iter), vid_(vid) {}

const Edge GraphView::ViewIncomingEdgeIterator::operator*() const {
  EdgeLabel attr = (*(graph_.edge_data_map_))[*iter_];
  VertexID uid = graph_.vertex_index_map_[source(*iter_, *(graph_.graphview_))];
  return Edge(uid, vid_, attr);
}

const bool GraphView::ViewIncomingEdgeIterator::operator!=
    (const ViewIncomingEdgeIterator &ieIter) const {
  return iter_ != ieIter.iter_;
}

GraphView::ViewIncomingEdgeIterator &GraphView::ViewIncomingEdgeIterator::operator++() {
  ++iter_;
  return *this;
}

/**
 * sub-class EdgeIDIterator
 */
GraphView::ViewEdgeIterator::ViewEdgeIterator(const GraphView &graph,
                                  const ViewEdgeDescripterIter &iter) :
    graph_(graph), iter_(iter) {
}

const Edge GraphView::ViewEdgeIterator::operator*() const {
  EdgeLabel attr = (*(graph_.edge_data_map_))[*iter_];
  VertexID uid = graph_.vertex_index_map_[source(*iter_, *(graph_.graphview_))];
  VertexID vid = graph_.vertex_index_map_[target(*iter_, *(graph_.graphview_))];
  return Edge(uid, vid, attr);
}

const bool GraphView::ViewEdgeIterator::operator!=(const ViewEdgeIterator
                                                  &eIter) const {
  return iter_ != eIter.iter_;
}

GraphView::ViewEdgeIterator &GraphView::ViewEdgeIterator::operator++() {
  ++iter_;
  return *this;
}

/**
 * sub-class VertexIDIterator
 */
GraphView::ViewVertexIDIterator::ViewVertexIDIterator(const GraphView &graph,
                                             const ViewVertexDescripterIter &iter) :
    graph_(graph), iter_(iter) {
}

const VertexID GraphView::ViewVertexIDIterator::operator*() const {
  return graph_.vertex_index_map_[*iter_];
}

const bool GraphView::ViewVertexIDIterator::operator!=(const ViewVertexIDIterator
                                                  &cvIter) const {
  return iter_ != cvIter.iter_;
}

GraphView::ViewVertexIDIterator &GraphView::ViewVertexIDIterator::operator++() {
  ++iter_;
  return *this;
}

/**
 * sub-class ChildrenIdIterator
 */
GraphView::ViewChildrenIDIterator::ViewChildrenIDIterator
    (const GraphView &graph, const ViewVertexDescripterIter &iter) :
    graph_(graph), iter_(iter) {
}

const VertexID GraphView::ViewChildrenIDIterator::operator*() const {
  return graph_.vertex_index_map_[*iter_];
}

const bool GraphView::ViewChildrenIDIterator::operator!=(const ViewChildrenIDIterator
                                                    &cvIter) const {
  return iter_ != cvIter.iter_;
}

GraphView::ViewChildrenIDIterator &GraphView::ViewChildrenIDIterator::operator++() {
  ++iter_;
  return *this;
}

/**
 * sub-class ParentsIDIterator
 */
GraphView::ViewParentsIDIterator::ViewParentsIDIterator(
    const ViewIncomingEdgeIterator &iter) : iter_(iter) {
}

const VertexID GraphView::ViewParentsIDIterator::operator*() const {
  return (*iter_).src();
}

const bool GraphView::ViewParentsIDIterator::operator!=(const ViewParentsIDIterator &
pvIter) const {
  return iter_ != pvIter.iter_;
}

GraphView::ViewParentsIDIterator &GraphView::ViewParentsIDIterator::operator++() {
  ++iter_;
  return *this;
}