#include <gflags/gflags.h>
#include <glog/logging.h>
#include<boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
//#include "cpp/core/graphapi.h"
#include "cpp/serial/dualsimulation.h"
#include "cpp/serial/dual_incremental.h"
#include "cpp/serial/strongsimulation.h"
#include "cpp/serial/strong_incremental.h"
#include "cpp/utils/util.h"
#include "cpp/io/io_local.h"
#include "cpp/core/global.h"
#include "cpp/core/strongr.h"
#include "cpp/core/view.h"
#include "cpp/utils/generate.h"
#include<iostream>
#include <fstream>
#include<ctime>
#include <sys/time.h>
#include<boost/filesystem.hpp>
#define random(a,b) (rand()%(b-a+1)+a)


class ViewExpr{
public:
   double get_current_time() {
	timeval t;
	gettimeofday(&t, 0);
	return (double) t.tv_sec + (double) t.tv_usec / 1000000;
    }

    ViewExpr(){}

    Serial(std::string test_data_name,int query_index){
        this->query_index = query_index;
        this->test_data_name=test_data_name;
        this->graph_vfile ="../data/"+test_data_name+"/"+test_data_name+".v";
        this->graph_efile ="../data/"+test_data_name+"/"+test_data_name+".e";
        this->view_file = "../data/"+test_data_name+"/views/view";
        this->r_file = "../data/"+test_data_name+"/"+test_data_name+".r";
        this->base_qfile = "../data/"+test_data_name+"/query/q";
        this->base_add_file = "../data/"+test_data_name+"/inc/add_e";
        this->base_remove_file="../data/"+test_data_name+"/inc/rm_e";
    }

    std::string get_query_vfile(int index){
        return base_qfile+std::to_string(index)+".v";
    }

    std::string get_query_efile(int index){
        return base_qfile+std::to_string(index)+".e";
    }

    std::string get_view_vfile(int query_index,int view_index){
        return view_file+std::to_string(query_index)+"/view"+std::to_string(view_index)+".v";
    }
    std::string get_view_efile(int query_index,int view_index){
       return view_file+std::to_string(query_index)+"/view"+std::to_string(view_index)+".e";
    }

    bool is_exist_file(std::string file_name){
        return boost::filesystem::is_directory(file_name);
    }

    void make_dir(std::string file_name){
        if(is_exist_file(file_name)){
             return ;
        }
        boost::filesystem::create_directory(file_name);
    }

public:

    vector<float> compare_direct_and_view_strongresult(Graph &qgraph, std::vector<StrongR> &direct_strong_result,std::vector<StrongR> &view_strong_result){
        vector<float> result;
//        if(direct_strong_result.size()!=view_strong_result.size()){
//            cout<<"size not the same "<<direct_strong_result.size()<<' '<<view_strong_result.size()<<endl;
//            return result;
//        }
//        for(int i=0;i<direct_strong_result.size();++i){
//            if(direct_strong_result[i].center() != view_strong_result[i].center()){
//                std::cerr<<"have different center result"<<endl;
//                return result;
//            }
//        }
        for(int i=0;i<direct_strong_result.size();++i){
            bool find=false;
            for(int j=0;j<view_strong_result.size();++j){
                if(direct_strong_result[i].center() == view_strong_result[j].center()){
                   std::unordered_map<VertexID, std::unordered_set<VertexID>> dirctsim=direct_strong_result[i].ballr();
                   std::unordered_map<VertexID, std::unordered_set<VertexID>> viewsim=view_strong_result[j].ballr();
                   if(dual_the_same(qgraph,dirctsim,viewsim)){
                      result.push_back(1.0);
                   }else{
                       std::unordered_set<VertexID> direct_result_node;
                       std::unordered_set<VertexID> view_result_node;
                       for(auto u:qgraph.GetAllVerticesID()){
                          for(auto v:dirctsim[u]){
                             direct_result_node.insert(v);
                          }
                          for(auto v:viewsim[u]){
                              view_result_node.insert(v);
                           }
                       }
                       result.push_back(float(view_result_node.size()*1.0/direct_result_node.size()));
                }
                find=true;
                break;
            }
        }
        if(!find){
            //result.push_back(0.0);
        }

        }
        return result;
    }



    void print_view_info(int circle_num,std::string view_info_text="view_result_number.txt"){
        Graph dgraph;
        GraphLoader dgraph_loader,qgraph_loader;
        dgraph_loader.LoadGraph(dgraph,graph_vfile,graph_efile);
        cout<<dgraph.GetNumVertices()<<' '<<dgraph.GetNumEdges()<<endl;

        int index=1;
        std::fstream outfile(view_info_text,std::ios::out);
        while(index<=circle_num){
            Graph qgraph;
            qgraph_loader.LoadGraph(qgraph,get_query_vfile(index),get_query_efile(index));
            std::fstream outfile(view_info_text,std::ios::app);
            int n=get_dual_result_num(dgraph,qgraph);
            outfile<<n;
            View vie;
            for(int i=1;i<6;++i){
                GraphLoader view_loader;
                Graph* new_view = new Graph();
                view_loader.LoadGraph(*new_view,get_view_vfile(index,i),get_view_efile(index,i));
                vie.add_ViewGraph(new_view);
                n=get_dual_result_num(dgraph,*new_view);
                outfile<<'\t'<<n;
            }
            outfile<<endl;
            outfile.close();
        std::cout<<"query view combination: "<<index<<endl;
        index+=1;
        }
    }

  void generate_query(int generate_query_nums,int generate_query_nodes, int generate_query_edges, int max_calculate_center_nodes){
      Graph dgraph;
      Generate generate;
      GraphLoader dgraph_loader;
      dgraph_loader.LoadGraph(dgraph,graph_vfile,graph_efile);
      std::cout<<dgraph.GetNumVertices()<<' '<<dgraph.GetNumEdges()<<std::endl;
      int i=1;
      while(i<=generate_query_nums){
          Graph qgraph;
          //generate.generate_connect_graphs_by_Dgraph(dgraph,qgraph,generate_query_nodes);
          generate.generate_view_by_Qgraph(dgraph,qgraph,generate_query_nodes);
          int d_Q=cal_diameter_qgraph(qgraph);
          if(d_Q!=4 || qgraph.GetNumEdges()!=generate_query_edges){
              continue;
          }
          double s0=get_current_time();
          std::unordered_set<VertexID> max_dual_set = generate.get_dual_node_result(dgraph,qgraph);
          double e0=get_current_time();

          if(max_dual_set.size()<=max_calculate_center_nodes){
              generate.save_grape_file(qgraph,get_query_vfile(i),get_query_efile(i));
              std::cout<<i<<" query nodes: "<<qgraph.GetNumVertices()<<' '<<" query edges: "<<qgraph.GetNumEdges()<<" "<<"d_Q: "<<d_Q<<" max_dual_set: "<<max_dual_set.size()<<" times: "<<e0-s0<<std::endl;
              i++;
          }
      }
  }

    void generate_query_view(int generate_view_nodes){
        int index = 1;
        Generate generate;
        while(index<=199){
            Graph qgraph;
            View vie;
            GraphLoader qgraph_loader;
           // cout<<is_exist_file(view_file+std::to_string(index));
            string tmp_path = view_file+std::to_string(index);
            if(!is_exist_file(tmp_path)){
                 //cout<<"file does not exist,prepare to mkdir"<<endl;
                 make_dir(tmp_path);
            }
            qgraph_loader.LoadGraph(qgraph,get_query_vfile(index),get_query_efile(index));
            for(int i=1;i<6;++i){
                GraphLoader view_loader;
                Graph* new_view = new Graph();
                generate.generate_view_by_Qgraph(qgraph,*new_view,generate_view_nodes);
               // generate.generate_connect_graphs_by_Dgraph(qgraph,*new_view,generate_view_nodes);
                vie.add_ViewGraph(new_view);
            }

            bool is_contain = vie.containCheck(qgraph);
            if(is_contain){
                std::vector<int> result=vie.minimumContain(qgraph);
//                if(result.size()==3){
//                    continue;
//                }
                cout<<index<<' '<<"minicontain nums ";
                for(auto num:result){
                   std::cout<<num<<' ';
                }
                std::cout<<endl;
                std::vector<Graph*> tmp_view_list= vie.get_ViewGraph_list();
                for(int i=0;i<tmp_view_list.size();++i){
                    generate.save_grape_file(*tmp_view_list[i],get_view_vfile(index,i+1),get_view_efile(index,i+1));
                }
                index++;
            }
    }
}


   void minimal_minimum_direct_experiment(int circle_num){
        Graph dgraph;
        GraphLoader dgraph_loader,qgraph_loader;
        dgraph_loader.LoadGraph(dgraph,graph_vfile,graph_efile);
        cout<<dgraph.GetNumVertices()<<' '<<dgraph.GetNumEdges()<<endl;
        int index=1;
        while(index<=circle_num){
        Graph qgraph;
        qgraph_loader.LoadGraph(qgraph,get_query_vfile(index),get_query_efile(index));
        View vie;
        for(int i=1;i<6;++i){
                GraphLoader view_loader;
                Graph* new_view = new Graph();
                view_loader.LoadGraph(*new_view,get_view_vfile(index,i),get_view_efile(index,i));
                vie.add_ViewGraph(new_view);
        }

         std::vector<StrongR> view_strong_result = vie.queryByViews(dgraph,qgraph,1,0);
         std::vector<StrongR> view_strong_result1 = vie.queryByViews(dgraph,qgraph,1,1);

        StrongSim strongs;
        double direct_stime=get_current_time();
//        vector<StrongR> direct_strong_result;
        vector<StrongR> direct_strong_result= strongs.strong_simulation_sim(dgraph,qgraph);
        double direct_etime=get_current_time();
        std::cout<<"calculate direct strong"<<direct_etime-direct_stime<<"s"<<std::endl;
       // cout<<view_strong_result.size()<<' '<<direct_strong_result.size()<<endl;
        vector<float> compare_rate = compare_direct_and_view_strongresult(qgraph,direct_strong_result,view_strong_result);
        int same_count=0;
        int appoximate_count=0;
        int center_none_result=direct_strong_result.size()-view_strong_result.size();
        for(auto f:compare_rate){
               if(f==1){
                   same_count+=1;
               }else{
                   appoximate_count+=1;
               }
       }
       cout<<index<<" same_count: "<<same_count<<" appoxiamte_count : "<<appoximate_count<<" none result count: "<<center_none_result<<" accuracy rate : "<<(float)same_count*1.0/(same_count+appoximate_count+center_none_result)<<endl;
       index+=1;
       }
    }

    void test_view_query_all(int circle_num,int flag0,int flag1){//flag0=0 means do not cache node distance,//flag1==0 means minimum contain,flag1=1 means minimalcontain
        Graph dgraph;
        GraphLoader dgraph_loader,qgraph_loader;
        dgraph_loader.LoadGraph(dgraph,graph_vfile,graph_efile);
        cout<<dgraph.GetNumVertices()<<' '<<dgraph.GetNumEdges()<<endl;
        int index=1;
        while(index<=circle_num){
        Graph qgraph;
        qgraph_loader.LoadGraph(qgraph,get_query_vfile(index),get_query_efile(index));
        View vie;
        for(int i=1;i<6;++i){
                GraphLoader view_loader;
                Graph* new_view = new Graph();
                view_loader.LoadGraph(*new_view,get_view_vfile(index,i),get_view_efile(index,i));
                vie.add_ViewGraph(new_view);
        }
        double view_stime=get_current_time();
        std::vector<StrongR> view_strong_result = vie.queryByViews(dgraph,qgraph,flag0,flag1);//0 means answer using cache dual_simulation ,1 means answer using cache strong simulaton
        double view_etime=get_current_time();
        std::cerr<<"calculate view strong"<<view_etime-view_stime<<"s"<<std::endl;

        StrongSim strongs;
        double direct_stime=get_current_time();
//        vector<StrongR> direct_strong_result;
        vector<StrongR> direct_strong_result= strongs.strong_simulation_sim(dgraph,qgraph);
        double direct_etime=get_current_time();
        std::cout<<"calculate direct strong"<<direct_etime-direct_stime<<"s"<<std::endl;
       // cout<<view_strong_result.size()<<' '<<direct_strong_result.size()<<endl;
        vector<float> compare_rate = compare_direct_and_view_strongresult(qgraph,direct_strong_result,view_strong_result);
        int same_count=0;
        int appoximate_count=0;
        int center_none_result=direct_strong_result.size()-view_strong_result.size();
        for(auto f:compare_rate){
               if(f==1){
                   same_count+=1;
               }else{
                   appoximate_count+=1;
               }
       }
       cout<<index<<" same_count: "<<same_count<<" appoxiamte_count : "<<appoximate_count<<" none result count: "<<center_none_result<<" accuracy rate : "<<(float)same_count*1.0/(same_count+appoximate_count+center_none_result)<<endl;
       index+=1;
       }
    }
private:
    int query_index = 1;
    std::string test_data_name ="yago";
    std::string graph_vfile ="../data/yago/yago.v";
    std::string graph_efile ="../data/yago/yago.e";
    std::string view_file = "../data/yago/views/view";
    std::string r_file = "../data/yago/yago.r";
    std::string base_qfile = "../data/yago/query/q";
    std::string base_add_file = "../data/yago/inc/add_e";
    std::string base_remove_file="../data/yago/inc/rm_e";
}

int main(int argc, char *argv[]) {
  google::SetUsageMessage("Usage: test [gflags_opt]");
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::ShutDownCommandLineFlags();
  google::InitGoogleLogging("test for working");
  google::ShutdownGoogleLogging();
 // Serial serial("dbpedia",1);
 // serial.test_generate_query(200,5,7,1000);
  ViewExpr viewexpr("yago",3);


  // viewexpr.generate_query_view(3);
  //  viewexpr.test_view_query_one(49,1,0);
   //viewexpr.test_view_query_all(200);
   viewexpr.test_view_query_all(200,1,0);
  return 0;
}

using namespace std;