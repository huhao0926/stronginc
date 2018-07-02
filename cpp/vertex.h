#ifndef CPP_VERTEX_H_
#define CPP_VERTEX_H_

#include "cpp/global.h"

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

 private:
  VertexID id_;
  VertexLabel label_;
};

#endif //CPP_EDGE_H_