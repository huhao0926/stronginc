#include "global.h"
#include <iostream>
using namespace std;
int _my_rank = 0;
int _num_workers = 0;

DEFINE_string(vfile, "../data/test.v", "Location of vertex file");
DEFINE_string(efile, "../data/test.e", "Location of edge file");
DEFINE_string(query_dir, "../data/query/", "Location of query files");
DEFINE_string(result_dir, "../result/test.result", "Dir of result file");
DEFINE_int32(file_location, 0,
                "File location: 0-local(default), 1-HDFS, 2-AWS S3, 5-HTTP");

