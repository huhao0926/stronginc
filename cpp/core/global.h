#ifndef UTILS_GLOBAL_H_
#define UTILS_GLOBAL_H_
#include <bsd/stdio.h>
#include <mpi.h>
#include <stddef.h>
#include <limits.h>
#include <assert.h> //for ease of debug
#include <glog/logging.h>
#include <gflags/gflags.h>

#include "cpp/core/config.h"
//============================
///worker info
#define MASTER_RANK 0

extern int _my_rank;
extern int _num_workers;
inline int get_worker_id() {
	return _my_rank;
}
inline int get_num_workers() {
	return _num_workers;
}

inline void init_workers() {
	MPI_Init(NULL, NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &_num_workers);
	MPI_Comm_rank(MPI_COMM_WORLD, &_my_rank);
}

inline void worker_finalize() {
	MPI_Finalize();///由于未执行MPI_Finalize，进程0无法感知到其他进程已退出 
}

inline void worker_barrier() {
	MPI_Barrier(MPI_COMM_WORLD);//该函数用于进程的同步。调用该函数后进程将等待直到通信器comm中的所有进程都调用了该函数才返回
}

//------------------------


DECLARE_string(vfile);
DECLARE_string(efile);
DECLARE_string(query_dir);
DECLARE_string(result_file);
DECLARE_int32(file_location);



#endif /* UTILS_GLOBAL_H_ */

