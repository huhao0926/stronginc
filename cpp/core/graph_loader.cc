#include <cstdlib>

#include "graph_loader.h"

typedef typename std::pair<VertexID, VertexID> EdgePair;

GraphLoader::GraphLoader() {
  AssignIOHandler();
}

GraphLoader::~GraphLoader() {
  graph_io_.release();
}

void GraphLoader::AssignIOHandler() {
  graph_io_ = std::unique_ptr<GraphIOLocal>(new GraphIOLocal());
  if (graph_io_ == nullptr) {
    LOG(FATAL) << "assign io handler failed." << std::endl;
  }
}

void GraphLoader::LoadVertices(std::vector<Vertex> &vertices, const std::string vfile) {
  LOG(INFO) << "begin loading vertices." << std::endl;
  if (graph_io_->OpenLocation(vfile) != false) {
    VertexID vid;
    std::string line, vlabel;
    int cnt = 0;
    while (graph_io_->GetNextLine(line)) {
      cnt ++;
      std::stringstream ss(line);
      ss >> vid;
      getline(ss, vlabel);
      VertexLabel attr = atoi(vlabel.c_str());
      vertices.emplace_back(vid, attr);
    }
    num_vertices_ = cnt;
    LOG(INFO) << "loaded vertices size = " << cnt << std::endl;
  } else {
    LOG(FATAL) << "load vertices error" << std::endl;
  }
}

void GraphLoader::LoadEdges(std::vector<Edge> &edges, const std::string efile) {
  LOG(INFO) << "begin loading edges." << std::endl;
  if (graph_io_->OpenLocation(efile) != false) {
    VertexID u, v;
    std::string line, elabel;
    int cnt = 0;
    while (graph_io_->GetNextLine(line)) {
      cnt ++;
      std::stringstream ss(line);
      ss >> u >> v;
      getline(ss, elabel);
      EdgeLabel attr = atoi(elabel.c_str());
      edges.emplace_back(u, v, attr);
    }
    num_edges_ = cnt;
    LOG(INFO) << "loaded edges size = " << cnt << std::endl;
  } else {
    LOG(FATAL) << "load edges error" << std::endl;
  }
}

void GraphLoader::LoadGraph(Graph &graph, const std::string vfile, const std::string efile) {
  std::vector<Vertex> vertices;
  std::vector<Edge> edges;
  LoadVertices(vertices, vfile);
  LoadEdges(edges, efile);

  std::vector<EdgePair> edges_form_to;
  std::vector<EdgeLabel> edges_attr;
  graph.vertices_label_.resize(vertices.size());
  for (int i = 0; i < vertices.size(); ++i) {
    graph.vertices_label_[i] = vertices[i].label();
  }

  for (int i = 0; i < edges.size(); ++i) {
    const Edge &e = edges[i];
    edges_form_to.emplace_back(e.src(), e.dst());
    edges_attr.emplace_back(e.attr());
  }

#ifndef MUTABLE_GRAPH
  graph.graph_ = std::shared_ptr<GraphT>(
      new GraphT(boost::edges_are_unsorted_multi_pass,
                 edges_form_to.begin(),
                 edges_form_to.end(),
                 edges_attr.begin(),
                 vertices.size()));
#else
  graph.graph_ = std::shared_ptr<GraphT>(
      new GraphT(edges_form_to.begin(),
                 edges_form_to.end(),
                 edges_attr.begin(),
                 vertices.size()));
#endif
  graph.vertex_index_map_ = boost::get(boost::vertex_index, *graph.graph_);
  graph.edge_data_map_ = std::unique_ptr<EdgeDataMap>(
          new EdgeDataMap(boost::get(boost::edge_weight, *graph.graph_)));
  graph.num_vertices_ = num_vertices_;
  graph.num_edges_ = num_edges_;
  LOG(INFO) << "loading graph finished." << std::endl;
}

void GraphLoader::LoadGraph(Graph &graph, const std::vector<Vertex> &vertices, const std::vector<Edge> &edges) {
//  std::vector<Vertex> vertices;
//  std::vector<Edge> edges;
//  LoadVertices(vertices, vfile);
//  LoadEdges(edges, efile);

  std::vector<EdgePair> edges_form_to;
  std::vector<EdgeLabel> edges_attr;
  graph.vertices_label_.resize(vertices.size());
  for (int i = 0; i < vertices.size(); ++i) {
    graph.vertices_label_[i] = vertices[i].label();
  }

  for (int i = 0; i < edges.size(); ++i) {
    const Edge &e = edges[i];
    edges_form_to.emplace_back(e.src(), e.dst());
    edges_attr.emplace_back(e.attr());
  }

#ifndef MUTABLE_GRAPH
  graph.graph_ = std::shared_ptr<GraphT>(
      new GraphT(boost::edges_are_unsorted_multi_pass,
                 edges_form_to.begin(),
                 edges_form_to.end(),
                 edges_attr.begin(),
                 vertices.size()));
#else
  graph.graph_ = std::shared_ptr<GraphT>(
      new GraphT(edges_form_to.begin(),
                 edges_form_to.end(),
                 edges_attr.begin(),
                 vertices.size()));
#endif
  graph.vertex_index_map_ = boost::get(boost::vertex_index, *graph.graph_);
  graph.edge_data_map_ = std::unique_ptr<EdgeDataMap>(
          new EdgeDataMap(boost::get(boost::edge_weight, *graph.graph_)));
  graph.num_vertices_ = vertices.size();
  graph.num_edges_ = edges.size();
  LOG(INFO) << "loading graph finished." << std::endl;
}


void GraphLoader::LoadGraph(Graph &graph,const std::vector<Vertex> &vertices, const std::set<Edge> &edges){
  std::vector<EdgePair> edges_form_to;
  std::vector<EdgeLabel> edges_attr;
  graph.vertices_label_.resize(vertices.size());
  for (int i = 0; i < vertices.size(); ++i) {
    graph.vertices_label_[i] = vertices[i].label();
  }

  for(auto edge:edges){
      const Edge &e = edge;
      edges_form_to.emplace_back(e.src(), e.dst());
      edges_attr.emplace_back(e.attr());
  }

#ifndef MUTABLE_GRAPH
  graph.graph_ = std::shared_ptr<GraphT>(
      new GraphT(boost::edges_are_unsorted_multi_pass,
                 edges_form_to.begin(),
                 edges_form_to.end(),
                 edges_attr.begin(),
                 vertices.size()));
#else
  graph.graph_ = std::shared_ptr<GraphT>(
      new GraphT(edges_form_to.begin(),
                 edges_form_to.end(),
                 edges_attr.begin(),
                 vertices.size()));
#endif
  graph.vertex_index_map_ = boost::get(boost::vertex_index, *graph.graph_);
  graph.edge_data_map_ = std::unique_ptr<EdgeDataMap>(
          new EdgeDataMap(boost::get(boost::edge_weight, *graph.graph_)));
  graph.num_vertices_ = vertices.size();
  graph.num_edges_ = edges.size();
  LOG(INFO) << "loading graph finished." << std::endl;


}