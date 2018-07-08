#ifndef CPP_FRAGMENTLOADER_H_
#define CPP_FRAGMENTLOADER_H_

#include <cstdio>
#include <unordered_map>
#include <vector>
#include <unordered_set>
#include<iostream>
#include "cpp/core/global.h"
#include "cpp/core/vertex.h"
#include "cpp/core/edge.h"
/**
  * load each partial graph for process
  */
class FragmentLoader {
    public:
        FragmentLoader();
        ~FragmentLoader();
        void LoadFragTable(std::unordered_map<VertexID , int> &fragTable, const std::string fragTablePath) ;

        //void LoadGraph(Fragment &fragment,Graph &graph, const std::string v_file, const std::string e_file);

        bool LoadEdge(std::vector<Edge> &global_edges, const std::unordered_map<VertexID,int> &fragTable,
                                       std::unordered_set<VertexID> &idx,  int fid, const std::string e_file);

        bool LoadVertex(std::vector<Vertex> &global_vertices, const std::unordered_map<VertexID,int> &fragTable,
                                       std::unordered_set<VertexID> &idx, int fid, const std::string v_file);
//     private:
       //int num_vertices_;
//       int num_edges_;
//       std::unordered_map<VertexID,VertexID> global2local;
//       std::unordered_map<VertexID,VertexID> local2global;
//       std::unordered_set<VertexID> innerVertices;
//	   std::unordered_set<VertexID> outerVertices;
//	   int FID;
//	   std::unordered_map<VertexID, std::bitset<NUM_FRAGMENTS>> msgThroughDest;
};
#endif //CPP_FRAGMENTLOADER_H_
using namespace std;