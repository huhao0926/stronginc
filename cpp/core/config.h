#ifndef CPP_CONFIG_H_
#define CPP_CONFIG_H_

#include <vector>

 //general types
typedef int VertexID;
typedef int WorkerID;
typedef int FragmentID;
typedef long Signature;
typedef int VertexLabel;
typedef int EdgeLabel;

// fragmets size
#ifndef NUM_FRAGMENTS
#define NUM_FRAGMENTS 5
#endif


#define MUTABLE_GRAPH

#endif //CPP_CONFIG_H