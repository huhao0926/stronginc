#ifndef CPP_FRAGMENT_H_
#define CPP_FRAGMENT_H_
#include "cpp/core/global.h"
#include "cpp/core/vertex.h"
#include "cpp/core/edge.h"
#include "cpp/core/graph.h"
#include "cpp/core/config.h"
#include "cpp/core/FragmentLoader.h"
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <string>
#include <vector>
#include <utility>
//#include <map>
/**
  * a class record each fragment graph and processor communication information
  */
class Fragment{
public :
    Fragment();

    Fragment(Graph &graph, const std::string v_file, const std::string e_file, const std::string r_file);

    Fragment(Graph &graph,const std::vector<Vertex> &_vertices, const std::vector<Edge> &_edges,
              const std::unordered_map<VertexID,int> &_fragTable, int _FID);

    ~Fragment();

    int getVertexFragmentID(VertexID u) const;

    int getLocalID(VertexID gvid);

    int getGlobalID(VertexID localid);

    int getNumVertices() const;

    int getNumEdges() const;

    const std::unordered_set<VertexID>* getInnerVertices() const;

    const std::unordered_set<VertexID>* getOuterVertices() const;

    const bool isBorderVertex(const VertexID gvid);

    const bool isInnerVertex(const VertexID gvid);

    const bool isOuterVertex(const VertexID gvid);

	const std::bitset<NUM_FRAGMENTS> getMsgThroughDest(const VertexID gvid);

	const bool has_vertex(const VertexID gvid) const;

	void update_fragment(Graph &graph,std::vector<Edge> &add_edges, std::vector<Edge> &rm_edges, std::vector<Vertex> &vertices);

	void update_fragment(Graph &graph,std::unordered_set<Edge> &add_edges, std::unordered_set<Edge> &rm_edges, std::unordered_set<Vertex> &vertices);

private :
       int numVertices;
       int numEdges;
       std::unordered_map<VertexID,int> fragTable;
       std::unordered_map<VertexID,VertexID> global2local;
       std::unordered_map<VertexID,VertexID> local2global;
       std::unordered_set<VertexID> innerVertices;
	   std::unordered_set<VertexID> outerVertices;
	   int FID;
	   std::unordered_map<VertexID, std::bitset<NUM_FRAGMENTS>> msgThroughDest;//recored meassage should send to process's id
};
#endif //CPP_FRAGMENT_H_