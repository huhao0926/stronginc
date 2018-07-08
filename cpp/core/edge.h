#ifndef CPP_EDGE_H_
#define CPP_EDGE_H_

#include "cpp/core/global.h"

class Edge {
 public:
  /**
   * constructor
   */
  Edge() {}

  Edge(const VertexID src, const VertexID dst, const EdgeLabel attr):
              src_(src), dst_(dst), attr_(attr) {}

  /**
   * @return source vertex id
   */
  inline const VertexID src() const {
    return src_;
  }
  /**
   * @return destination vertex id
   */
  inline const VertexID dst() const {
    return dst_;
  }

  /**
   * @return the weight of edge
   */
  inline const EdgeLabel attr() const {
    return attr_;
  }

  bool operator== (const Edge & edge) const {
    return (this->src_ == edge.src_) && (this->dst_ == edge.dst_);
  }

 public:
  VertexID src_, dst_;
  EdgeLabel attr_;
};

namespace std
{
  template<>
    struct hash<Edge>
    {
      size_t
      operator()(const Edge & edge) const
      {
        size_t seed = 0;
        hash<int> h;
          seed ^= h(edge.src()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
          seed ^= h(edge.dst()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
          return seed;
      }
    };
}

#endif //CPP_EDGE_H_