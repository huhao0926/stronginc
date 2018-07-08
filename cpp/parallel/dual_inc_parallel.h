#ifndef CPP_DUALINCPARALLEL_H_
#define CPP_DUALINCPARALLEL_H_
#include "cpp/core/graphapi.h"
#include "cpp/utils/MessageBuffer.h"
class Dual_Incparallel{
public:
    Dual_Incparallel();

    ~Dual_Incparallel();
private:
    int continue_run = 0;

    MessageBuffer<VertexID> messageBuffers;
};
#endif //CPP_DUALINCPARALLEL_H_
using namespace std;