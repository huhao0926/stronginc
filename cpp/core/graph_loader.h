#ifndef CPP_GRAPH_LOADER_H_
#define CPP_GRAPH_LOADER_H_

#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <string>
#include <vector>
#include <utility>
#include <map>

#include "cpp/core/global.h"
#include "cpp/core/vertex.h"
#include "cpp/core/edge.h"
#include "cpp/core/graph.h"
#include "cpp/io/io_local.h"
/**
  *load graph
  */
class GraphLoader {
 public:
  GraphLoader();
  ~GraphLoader();

  /**
   * assign exactly io handler according to file location. (e.g., hdfs, nfs.)
   * @return false if undefined
   */
  void AssignIOHandler();

  /**
   * load vertices form file
   * @param
   */
  void LoadVertices(std::vector<Vertex> &vertices, const std::string vfile);

  /**
   * load edges form file
   * @param
   */
  void LoadEdges(std::vector<Edge> &edges, const std::string efile);

  /**
   * load and build graph from vertices and edges
   * @param
   */
  void LoadGraph(Graph &graph, const std::string vfile, const std::string efile);

  void LoadGraph(Graph &graph, const std::vector<Vertex> &vertices, const std::vector<Edge> &edges);

  void LoadGraph(Graph &graph,const std::vector<Vertex> &vertices, const std::set<Edge> &edges);

 private:
  std::unique_ptr<GraphIO> graph_io_;
  int num_vertices_;
  int num_edges_;
};

#endif //CPP_GRAPE_LOADER_H_