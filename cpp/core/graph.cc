#include "graph.h"

Graph::Graph() {}

Graph::~Graph() {}

VertexLabel &Graph::GetVertexLabel(const VertexID vid) {
  assert(vid < num_vertices_);
  return vertices_label_.at(vid);
}

const bool Graph::ExistEdge(const VertexID src, const VertexID dst) const {
  assert(src < num_vertices_&& dst < num_vertices_);
  return boost::edge(vertex(src, *graph_),
                     vertex(dst, *graph_), *graph_).second;
}

const bool Graph::HasChild(const VertexID vid) const {
  assert(vid < num_vertices_);
  VertexDescriptor v = vertex(vid, *graph_);
  typedef typename boost::graph_traits<GraphT>::adjacency_iterator
      VertexDescripterIter;
  VertexDescripterIter iter_begin, iter_end;
  boost::tie(iter_begin, iter_end) = adjacent_vertices(v, *graph_);
  return iter_begin != iter_end;
}

IterContainer<Graph::OutgoingEdgeIterator> Graph::GetOutgoingEdges(
              const VertexID uid) {
  assert(uid < num_vertices_);
  VertexDescriptor u = vertex(uid, *graph_);
  OutEdgeDescripterIter iter_begin, iter_end;
  boost::tie(iter_begin, iter_end) = out_edges(u, *graph_);
  OutgoingEdgeIterator e_begin(*this, iter_begin, uid),
      e_end(*this, iter_end, uid);

  return IterContainer<OutgoingEdgeIterator>(e_begin, e_end);
}

IterContainer<Graph::IncomingEdgeIterator> Graph::GetIncomingEdges(
              const VertexID vid) {
  assert(vid < num_vertices_);
  VertexDescriptor v = vertex(vid, *graph_);
  InEdgeDescripterIter iter_begin, iter_end;
  boost::tie(iter_begin, iter_end) = in_edges(v, *graph_);
  IncomingEdgeIterator e_begin(*this, iter_begin, vid),
      e_end(*this, iter_end, vid);

  return IterContainer<IncomingEdgeIterator>(e_begin, e_end);
}

IterContainer<Graph::ChildrenIDIterator> Graph::GetChildrenID(
              const VertexID uid) {
  assert(uid < num_vertices_);
  VertexDescriptor u = vertex(uid, *graph_);
  typedef typename boost::graph_traits<GraphT>::adjacency_iterator
      VertexDescripterIter;
  VertexDescripterIter iter_begin, iter_end;
  boost::tie(iter_begin, iter_end) = adjacent_vertices(u, *graph_);
  return IterContainer<ChildrenIDIterator>(
      ChildrenIDIterator(*this, iter_begin),
      ChildrenIDIterator(*this, iter_end));
}

IterContainer<Graph::ParentsIDIterator> Graph::GetParentsID(
              const VertexID vid){
  assert(vid < num_vertices_);
  VertexDescriptor v = vertex(vid, *graph_);
  InEdgeDescripterIter iter_begin, iter_end;
  boost::tie(iter_begin, iter_end) = in_edges(v, *graph_);
  return IterContainer<ParentsIDIterator>(
      ParentsIDIterator(IncomingEdgeIterator(*this, iter_begin, vid)),
      ParentsIDIterator(IncomingEdgeIterator(*this, iter_end, vid)));
}

IterContainer<Graph::VertexIDIterator> Graph::GetAllVerticesID() {
  typedef typename boost::graph_traits<GraphT>::vertex_iterator
      VertexDescripterIter;
  VertexDescripterIter iter_begin, iter_end;
  boost::tie(iter_begin, iter_end) = vertices(*graph_);
  return IterContainer<VertexIDIterator>(VertexIDIterator(*this, iter_begin),
                                         VertexIDIterator(*this, iter_end));
}

IterContainer<Graph::EdgeIterator> Graph::GetAllEdges() {
  typedef typename boost::graph_traits<GraphT>::edge_iterator
      EdgeDescripterIter;
  EdgeDescripterIter iter_begin, iter_end;
  boost::tie(iter_begin, iter_end) = edges(*graph_);
  return IterContainer<EdgeIterator>(EdgeIterator(*this, iter_begin),
                                     EdgeIterator(*this, iter_end));
}

const int Graph::GetOutDegree(const VertexID vid) const {
  VertexDescriptor v = vertex(vid, *graph_);
  return out_degree(v, *graph_);
}

const int Graph::GetInDegree(const VertexID vid) const {
  VertexDescriptor v = vertex(vid, *graph_);
  return in_degree(v, *graph_);
}

const int Graph::GetDegree(const VertexID vid) const {
  VertexDescriptor v = vertex(vid, *graph_);
  return in_degree(v, *graph_) + out_degree(v, *graph_);
}

/**
 * sub-class OutgoingEdgeIterator
 */
Graph::OutgoingEdgeIterator::OutgoingEdgeIterator
    (const Graph &graph, const OutEdgeDescripterIter &iter,
     const VertexID &uid) :
    graph_(graph), iter_(iter), uid_(uid) {}

const Edge Graph::OutgoingEdgeIterator::operator*() const {
  EdgeLabel attr = (*(graph_.edge_data_map_))[*iter_];
  VertexID vid = graph_.vertex_index_map_[target(*iter_, *(graph_.graph_))];
  return Edge(uid_, vid, attr);
}

const bool Graph::OutgoingEdgeIterator::operator!=
    (const OutgoingEdgeIterator &oeIter) const {
  return iter_ != oeIter.iter_;
}

Graph::OutgoingEdgeIterator &Graph::OutgoingEdgeIterator::operator++() {
  ++iter_;
  return *this;
}

/**
 * sub-class IncomingEdgeIterator
 */
Graph::IncomingEdgeIterator::IncomingEdgeIterator
    (const Graph &graph, const InEdgeDescripterIter &iter,
     const VertexID &vid) :
    graph_(graph), iter_(iter), vid_(vid) {}

const Edge Graph::IncomingEdgeIterator::operator*() const {
  EdgeLabel attr = (*(graph_.edge_data_map_))[*iter_];
  VertexID uid = graph_.vertex_index_map_[source(*iter_, *(graph_.graph_))];
  return Edge(uid, vid_, attr);
}

const bool Graph::IncomingEdgeIterator::operator!=
    (const IncomingEdgeIterator &ieIter) const {
  return iter_ != ieIter.iter_;
}

Graph::IncomingEdgeIterator &Graph::IncomingEdgeIterator::operator++() {
  ++iter_;
  return *this;
}

/**
 * sub-class EdgeIDIterator
 */
Graph::EdgeIterator::EdgeIterator(const Graph &graph,
                                  const EdgeDescripterIter &iter) :
    graph_(graph), iter_(iter) {
}

const Edge Graph::EdgeIterator::operator*() const {
  EdgeLabel attr = (*(graph_.edge_data_map_))[*iter_];
  VertexID uid = graph_.vertex_index_map_[source(*iter_, *(graph_.graph_))];
  VertexID vid = graph_.vertex_index_map_[target(*iter_, *(graph_.graph_))];
  return Edge(uid, vid, attr);
}

const bool Graph::EdgeIterator::operator!=(const EdgeIterator
                                                  &eIter) const {
  return iter_ != eIter.iter_;
}

Graph::EdgeIterator &Graph::EdgeIterator::operator++() {
  ++iter_;
  return *this;
}

/**
 * sub-class VertexIDIterator
 */
Graph::VertexIDIterator::VertexIDIterator(const Graph &graph,
                                             const VertexDescripterIter &iter) :
    graph_(graph), iter_(iter) {
}

const VertexID Graph::VertexIDIterator::operator*() const {
  return graph_.vertex_index_map_[*iter_];
}

const bool Graph::VertexIDIterator::operator!=(const VertexIDIterator
                                                  &cvIter) const {
  return iter_ != cvIter.iter_;
}

Graph::VertexIDIterator &Graph::VertexIDIterator::operator++() {
  ++iter_;
  return *this;
}

/**
 * sub-class ChildrenIdIterator
 */
Graph::ChildrenIDIterator::ChildrenIDIterator
    (const Graph &graph, const VertexDescripterIter &iter) :
    graph_(graph), iter_(iter) {
}

const VertexID Graph::ChildrenIDIterator::operator*() const {
  return graph_.vertex_index_map_[*iter_];
}

const bool Graph::ChildrenIDIterator::operator!=(const ChildrenIDIterator
                                                    &cvIter) const {
  return iter_ != cvIter.iter_;
}

Graph::ChildrenIDIterator &Graph::ChildrenIDIterator::operator++() {
  ++iter_;
  return *this;
}

/**
 * sub-class ParentsIDIterator
 */
Graph::ParentsIDIterator::ParentsIDIterator(
    const IncomingEdgeIterator &iter) : iter_(iter) {
}

const VertexID Graph::ParentsIDIterator::operator*() const {
  return (*iter_).src();
}

const bool Graph::ParentsIDIterator::operator!=(const ParentsIDIterator &
pvIter) const {
  return iter_ != pvIter.iter_;
}

Graph::ParentsIDIterator &Graph::ParentsIDIterator::operator++() {
  ++iter_;
  return *this;
}

void Graph::shortest_distance(VertexID vid,std::unordered_map<VertexID,int> &dis){
    VertexDescriptor u = vertex(vid, *graph_);
    std::vector<int> color(num_vertices_,0);
    //std::unordered_map<VertexID,int> dis;
    std::queue<VertexDescriptor> q;
    //vertex_index_map_
    q.push(u);
    color[vid] = 1;
    dis[vid] = 0;
    while(!q.empty()){
        VertexDescriptor root = q.front();
        VertexID root_id = vertex_index_map_[root];
        q.pop();
        InEdgeDescripterIter in_iter_begin, in_iter_end;
        boost::tie(in_iter_begin, in_iter_end) = in_edges(root, *graph_);
        for (;in_iter_begin != in_iter_end;++in_iter_begin){
            VertexDescriptor in_node =  source(*in_iter_begin,*graph_);
            VertexID in_node_id = vertex_index_map_[in_node];
            if (color[in_node_id] == 0){
                q.push(in_node);
                color[in_node_id] = 1;
                dis[in_node_id] = dis[root_id]+1;
            }
        }

        OutEdgeDescripterIter out_iter_begin, out_iter_end;
        boost::tie(out_iter_begin, out_iter_end) = out_edges(root, *graph_);
        for(; out_iter_begin != out_iter_end; ++out_iter_begin){
            VertexDescriptor out_node =  target(*out_iter_begin,*graph_);
            VertexID out_node_id = vertex_index_map_[out_node];
            if (color[out_node_id] == 0){
                q.push(out_node_id);
                color[out_node_id] = 1;
                dis[out_node_id] = dis[root_id] + 1;
            }
        }
    }
    for (VertexID i = 0;i < num_vertices_; ++i){
        if (dis.find(i) == dis.end()){
            dis[i] = INT_MAX;
        }
    }

   // return dis;
}

const int Graph::shortest_distance(VertexID source_id,VertexID target_id)const{
    VertexDescriptor source_node = vertex(source_id, *graph_);
    VertexDescriptor target_node = vertex(target_id, *graph_);
    std::vector<int> color(num_vertices_,0);
    std::unordered_map<VertexID,int> dis;
    std::queue<VertexDescriptor> q;
    //vertex_index_map_
    q.push(source_node);
    color[source_id] = 1;
    dis[source_id] = 0;
    while(!q.empty()){
        VertexDescriptor root = q.front();
        VertexID root_id = vertex_index_map_[root];
        q.pop();
        InEdgeDescripterIter in_iter_begin, in_iter_end;
        boost::tie(in_iter_begin, in_iter_end) = in_edges(root, *graph_);
        for (;in_iter_begin != in_iter_end;++in_iter_begin){
            VertexDescriptor in_node =  source(*in_iter_begin,*graph_);
            if(in_node == target_node){
                return dis[root_id]+1;
            }
            VertexID in_node_id = vertex_index_map_[in_node];
            if (color[in_node_id] == 0){
                q.push(in_node);
                color[in_node_id] = 1;
                dis[in_node_id] = dis[root_id]+1;
            }
        }

        OutEdgeDescripterIter out_iter_begin, out_iter_end;
        boost::tie(out_iter_begin, out_iter_end) = out_edges(root, *graph_);
        for(; out_iter_begin != out_iter_end; ++out_iter_begin){
            VertexDescriptor out_node =  target(*out_iter_begin,*graph_);
            if(target_node == out_node){
                return dis[root_id]+1;
            }
            VertexID out_node_id = vertex_index_map_[out_node];
            if (color[out_node_id] == 0){
                q.push(out_node_id);
                color[out_node_id] = 1;
                dis[out_node_id] = dis[root_id] + 1;
            }
        }
    }
    return INT_MAX;
}

void Graph::find_hop_nodes(VertexID vid, int d_hop,std::unordered_set<VertexID> &result){
    VertexDescriptor u = vertex(vid, *graph_);
    std::vector<int> color(num_vertices_,0);
    std::unordered_map<VertexID,int> dis;
    std::queue<VertexDescriptor> q;
   // std::unordered_set<int> result;
    //vertex_index_map_
    q.push(u);
    color[vid] = 1;
    dis[vid] = 0;
    result.insert(vid);
    while(!q.empty()){
        VertexDescriptor root = q.front();
        VertexID root_id = vertex_index_map_[root];
        if(dis[root_id]==d_hop){
            return ;
        }
        q.pop();
        InEdgeDescripterIter in_iter_begin, in_iter_end;
        boost::tie(in_iter_begin, in_iter_end) = in_edges(root, *graph_);
        for (;in_iter_begin != in_iter_end;++in_iter_begin){
            VertexDescriptor in_node =  source(*in_iter_begin,*graph_);
            VertexID in_node_id = vertex_index_map_[in_node];
            if (color[in_node_id] == 0){
                q.push(in_node);
                color[in_node_id] = 1;
                dis[in_node_id] = dis[root_id]+1;
                result.insert(in_node_id);
            }
        }

        OutEdgeDescripterIter out_iter_begin, out_iter_end;
        boost::tie(out_iter_begin, out_iter_end) = out_edges(root, *graph_);
        for(; out_iter_begin != out_iter_end; ++out_iter_begin){
            VertexDescriptor out_node =  target(*out_iter_begin,*graph_);
            VertexID out_node_id = vertex_index_map_[out_node];
            if (color[out_node_id] == 0){
                q.push(out_node_id);
                color[out_node_id] = 1;
                dis[out_node_id] = dis[root_id] + 1;
                result.insert(out_node_id);
            }
        }
    }
   // return result;
}

void  Graph::find_connectivity_nodes(VertexID vid, std::unordered_set<VertexID> &result) {
    VertexDescriptor u = vertex(vid, *graph_);
    std::vector<int> color(num_vertices_,0);
   // std::unordered_set<VertexID> result;
    std::queue<VertexDescriptor> q;
    //vertex_index_map_
    q.push(u);
    result.insert(vid);
    color[vid] = 1;
    while(!q.empty()){
        VertexDescriptor root = q.front();
        VertexID root_id = vertex_index_map_[root];
        q.pop();
        InEdgeDescripterIter in_iter_begin, in_iter_end;
        boost::tie(in_iter_begin, in_iter_end) = in_edges(root, *graph_);
        for (;in_iter_begin != in_iter_end;++in_iter_begin){
            VertexDescriptor in_node =  source(*in_iter_begin,*graph_);
            VertexID in_node_id = vertex_index_map_[in_node];
            if (color[in_node_id] == 0){
                q.push(in_node);
                color[in_node_id] = 1;
                result.insert(in_node_id);
            }
        }

        OutEdgeDescripterIter out_iter_begin, out_iter_end;
        boost::tie(out_iter_begin, out_iter_end) = out_edges(root, *graph_);
        for(; out_iter_begin != out_iter_end; ++out_iter_begin){
            VertexDescriptor out_node =  target(*out_iter_begin,*graph_);
            VertexID out_node_id = vertex_index_map_[out_node];
            if (color[out_node_id] == 0){
                q.push(out_node_id);
                color[out_node_id] = 1;
                result.insert(out_node_id);
            }
        }
    }
  //  return result;
}

void Graph::find_hop_nodes(std::unordered_set<VertexID> &node_set,int d_hop,std::unordered_set<VertexID> &result){
    std::vector<int> color(num_vertices_,0);
    std::unordered_map<VertexID,int> dis;
    std::queue<VertexDescriptor> q;
  //  std::unordered_set<int> result;
    for(auto vid :node_set){
        VertexDescriptor u = vertex(vid, *graph_);
        q.push(u);
        color[vid] = 1;
        dis[vid] = 0;
        result.insert(vid);
    }
    while(!q.empty()){
        VertexDescriptor root = q.front();
        VertexID root_id = vertex_index_map_[root];
        if(dis[root_id]==d_hop){
            return ;
         //   return result;
        }
        q.pop();
        InEdgeDescripterIter in_iter_begin, in_iter_end;
        boost::tie(in_iter_begin, in_iter_end) = in_edges(root, *graph_);
        for (;in_iter_begin != in_iter_end;++in_iter_begin){
            VertexDescriptor in_node =  source(*in_iter_begin,*graph_);
            VertexID in_node_id = vertex_index_map_[in_node];
            if (color[in_node_id] == 0){
                q.push(in_node);
                color[in_node_id] = 1;
                dis[in_node_id] = dis[root_id]+1;
                result.insert(in_node_id);
            }
        }

        OutEdgeDescripterIter out_iter_begin, out_iter_end;
        boost::tie(out_iter_begin, out_iter_end) = out_edges(root, *graph_);
        for(; out_iter_begin != out_iter_end; ++out_iter_begin){
            VertexDescriptor out_node =  target(*out_iter_begin,*graph_);
            VertexID out_node_id = vertex_index_map_[out_node];
            if (color[out_node_id] == 0){
                q.push(out_node_id);
                color[out_node_id] = 1;
                dis[out_node_id] = dis[root_id] + 1;
                result.insert(out_node_id);
            }
        }
    }

   // return result;
}

#ifdef MUTABLE_GRAPH
// Mutable graph: adding vertices and edges
void Graph::AddVertex(const Vertex &v) {
  vertices_label_.push_back(v.label());
  add_vertex(*graph_);
}

void Graph::AddEdge(const Edge &e) {
  auto u = e.src(), v = e.dst();
  auto attr = e.attr();
  if (!edge(u, v, *graph_).second) {
    add_edge(u, v, attr, *graph_);
  }
}

void Graph::RemoveEdge(const Edge &e) {
  auto u = e.src(), v = e.dst();
  if (edge(u, v, *graph_).second) {
    remove_edge(u, v, *graph_);
  }
}

void Graph::RebuildGraphProperties() {
  num_vertices_ = num_vertices(*graph_);
  num_edges_ =  num_edges(*graph_);
  vertex_index_map_ = boost::get(boost::vertex_index, *graph_);
  edge_data_map_ = std::unique_ptr<EdgeDataMap>(new EdgeDataMap(
      boost::get(boost::edge_weight, *graph_)));
}
#endif