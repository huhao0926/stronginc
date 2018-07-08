#ifndef CPP_VERTEX_H_
#define CPP_VERTEX_H_

#include "cpp/core/global.h"

class Vertex {
 public:
  /**
   * constructor
   */
  Vertex() {}

  Vertex(const VertexID id, const VertexLabel label):
          id_(id), label_(label) {}

  /**
   * @return vertex id
   */
  inline const VertexID id() const {
    return id_;
  }
  /**
   * @return vertex label
   */
  inline const VertexLabel label() const {
    return label_;
  }

  bool operator<(const Vertex &v) const {
    return id_ < v.id_;
  }

  bool operator== (const Vertex & v) const {
    return (this->id_ == v.id_);
  }

 public:
  VertexID id_;
  VertexLabel label_;
};

namespace std
{
  template<>
    struct hash<Vertex>
    {
      size_t
      operator()(const Vertex & v) const
      {
        size_t seed = 0;
        hash<int> h;
          seed ^= h(v.id()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
          return seed;
      }
    };
}

#endif //CPP_EDGE_H_