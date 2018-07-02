#ifndef CPP_STRONGR_H_
#define CPP_STRONGR_H_
#include "cpp/global.h"
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


private:
   VertexID center_id_;
   std::unordered_map<VertexID,std::unordered_set<VertexID>> sim_;
};

#endif //CPP_STRONGR_H_
