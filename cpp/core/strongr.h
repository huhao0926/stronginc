#ifndef CPP_STRONGR_H_
#define CPP_STRONGR_H_
#include "cpp/core/global.h"
class StrongR{
public:
  StrongR(){}

  ~StrongR(){}

   StrongR(const VertexID &center_id,const std::unordered_map<VertexID,std::unordered_set<VertexID>> &sim):center_id_(center_id),sim_(sim){

   }

  inline const VertexID center() const {
    return center_id_;
  }

  inline const std::unordered_map<VertexID,std::unordered_set<VertexID>> & ballr() const {
    return sim_;
  }

  bool operator== (const StrongR & strongr) const {
    return (this->center_id_ == strongr.center_id_);
  }
public:
   VertexID center_id_;
   std::unordered_map<VertexID,std::unordered_set<VertexID>> sim_;
};
namespace std
{
  template<>
    struct hash<StrongR>
    {
      size_t
      operator()(const StrongR & strongr) const
      {
        size_t seed = 0;
        hash<int> h;
          seed ^= h(strongr.center()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
          return seed;
      }
    };
}
#endif //CPP_STRONGR_H_
