运行环境 :
cmake,mpi,boost >=1.8
gflag,glog输出日志文件


how to run

cd build
cmake .. && make

cpp/core里面的文件是自定义graph,vertex,edge核心的代码

cpp/parallel  所有的并行算法
cpp/serial  所有的串行算法
cpp/utils  一些自定义功能函数 (消息序列化,加载边，结果判断,)
cpp/app/test/parallel_test 对所有并行算法的测试
cpp/app/test/serial_test 对所有串行算法测试
cpp/app/test/test_util 对utils功能函数测试

核心算法:单源最短路径，连通性修剪，找到一个节点d_hop范围节点,找到若干节点d_hop范围节点，dualsimulation,strong simulation,dualsimulation incremental,strong incremental（并行+串行）

.pdf 是对duasimulation ,strong simulation两个图匹配算法的介绍
strong parallel 实现了两个不同算法的并行
