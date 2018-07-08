#include "util.h"
int cal_diameter_qgraph(Graph &qgraph){
          int temp_dia = 0;
          int max_dia = qgraph.GetNumVertices()-1;
          for(auto u : qgraph.GetAllVerticesID()){
              std::unordered_map<VertexID,int> dis;
              qgraph.shortest_distance(u,dis);
              for (int i=0; i<qgraph.GetNumVertices(); i++){
                if (dis[i] <= max_dia && temp_dia < dis[i]){
                    temp_dia = dis[i];
                }
              }
          }
          return temp_dia;
}

bool  query_labl_all_notsame(Graph &qgraph){
     std::unordered_set<VertexLabel> labl_set;
     for (auto u:qgraph.GetAllVerticesID()){
         if (labl_set.find(qgraph.GetVertexLabel(u)) != labl_set.end()){
             return false;
         }else{
             labl_set.insert(qgraph.GetVertexLabel(u));
         }
     }
     return true;
}

void generate_connect_graphs_by_Dgraph(Graph &dgraph, Graph &qgraph,const  int num_nodes){
    std::vector<Vertex> vertices;
    std::vector<Edge> edges;
//    if(num_nodes> dgraph.GetNumVertices()){
//        for(auto u:dgraph.GetAllVerticesID()){
//            vertices.emplace_back(u, dgraph.GetVertexLabel(u));
//        }
//        for(auto edge:dgraph.GetAllEdges()){
//            edges.emplace_back(edge.src(),edge.dst(),1);
//        }
//        GraphLoader qgraph_loader;
//        qgraph_loader.LoadGraph(qgraph,vertices,edges);
//        return ;
//    }
    bool continue_select_root = true;

    std::vector<VertexID> vertex_list;
    while (continue_select_root){
        vertex_list.clear();
        std::queue<VertexID> que;
        VertexID root_id = random(0,dgraph.GetNumVertices()-1);

        que.push(root_id);
        vertex_list.push_back(root_id);

        while(!que.empty()){
            VertexID u = que.front();
            que.pop();
            int tmp_num = 0;

            //for (auto pre_w : dgraph.GetParentsID(w))
            for (auto des_w : dgraph.GetChildrenID(u)){
                if (std::find(vertex_list.begin(),vertex_list.end(),des_w) == vertex_list.end() && vertex_list.size()<num_nodes){
                    vertex_list.push_back(des_w);
                    que.push(des_w);
                    tmp_num+=1;
                }
            }

            for(auto pre_w :dgraph.GetParentsID(u)){
                if (std::find(vertex_list.begin(),vertex_list.end(),pre_w) == vertex_list.end() && vertex_list.size()<num_nodes){
                    vertex_list.push_back(pre_w);
                    que.push(pre_w);
                    tmp_num+=1;
                }

            }
        }
        if (vertex_list.size() ==num_nodes){
            continue_select_root = false;
        }
    }
    for (int i= 0 ;i < num_nodes ;i++){
        vertices.emplace_back(i, dgraph.GetVertexLabel(vertex_list[i]));
    }
    for(int i = 0 ;i<num_nodes;i++){
        for(int j =0;j<num_nodes;j++){
            if (dgraph.ExistEdge(vertex_list[i],vertex_list[j])){
                edges.emplace_back(i,j,1);
            }
        }
    }
    GraphLoader qgraph_loader;
    qgraph_loader.LoadGraph(qgraph,vertices,edges);
    return ;
}

void save_grape_file(Graph &qgraph, const std::string &v_file, const std::string &e_file){
   std::fstream out_vfile(v_file,std::ios::out);
   if(!out_vfile)
	{
		std::cout<<"the outfile can  not construct";
		exit(0);
	}
	for(auto u:qgraph.GetAllVerticesID()){
	 out_vfile<<u<<' '<<qgraph.GetVertexLabel(u)<<std::endl;
	}
	out_vfile.close();

   std::fstream out_efile(e_file,std::ios::out);
   if(!out_efile)
	{
		std::cout<<"the outfile can  not construct";
		exit(0);
	}
	for(auto edge:qgraph.GetAllEdges()){
	    out_efile<<edge.src()<<' '<<edge.dst()<<' '<<'1'<<std::endl;
	}
	out_efile.close();
}

bool dual_the_same(Graph &qgraph,  std::unordered_map<VertexID, std::unordered_set<VertexID>> &direct_sim,  std::unordered_map<VertexID, std::unordered_set<VertexID>> &incdsim){
   for (auto u:qgraph.GetAllVerticesID()){
    if(direct_sim[u].size()!=incdsim[u].size()){
    return false;
    }
    for (auto v : direct_sim[u]){
        if(incdsim[u].find(v) == incdsim[u].end()){
           return false;
        }
    }

   }
   return true;

 }

void save_sim_result(Graph &qgraph,std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim,std::string filename){
   std::fstream outfile(filename,std::ios::out);
   if(!outfile)
	{
		std::cout<<"the outfile can  not construct";
		exit(0);
	}
    for(auto u:qgraph.GetAllVerticesID()){
        outfile<<u;
        for (auto v:sim[u]){
            outfile<<' '<<v;
        }
        outfile<<std::endl;

    }
    outfile.close();
}

void load_sim_result(std::unordered_map<VertexID, std::unordered_set<VertexID>> &sim,std::string filename){
    std::ifstream infile(filename,std::ios::in);
    std::string s;
    while(getline(infile,s))
    {
        std::vector<std::string> vecstr;
        boost::split(vecstr,s,boost::is_any_of(" \t"));
        VertexID key;
        for(int i=0;i<vecstr.size();i++){
            VertexID v;
            std::stringstream ss;
            ss<<vecstr[i];
            ss>>v;
            //std::cout<<v<<std::endl;
            if (i ==0){
              key = v;
              sim[key] = std::unordered_set<VertexID>();
            }else{
             sim[key].insert(v);
           }
        }
    }
    infile.close();
}

void LoadEdges(std::vector<std::pair<VertexID,VertexID>> &edges, const std::string efile) {
    std::string line;
    std::ifstream e_infile(efile,std::ios::in);
    VertexID u,v;
    while(getline(e_infile,line,'\n')){
        std::stringstream ss(line);
        ss>> u >> v;
        edges.push_back(std::make_pair(u,v));
    }
}

void Load_bunch_edges(std::vector<std::pair<VertexID,VertexID>> &edges,const std::string basefilename,int index){
    int i=1;
    while (i<=index){
         LoadEdges(edges,basefilename+std::to_string(i));
         i+=1;
     }
}

void LoadEdges(std::set<std::pair<VertexID,VertexID>> &edges, const std::string efile) {
    std::string line;
    std::ifstream e_infile(efile,std::ios::in);
    VertexID u,v;
    while(getline(e_infile,line,'\n')){
        std::stringstream ss(line);
        ss>> u >> v;
        edges.insert(std::make_pair(u,v));
    }
}

void Load_bunch_edges(std::set<std::pair<VertexID,VertexID>> &edges,const std::string basefilename,int index){
    int i=1;
    while (i<=index){
         LoadEdges(edges,basefilename+std::to_string(i));
         i+=1;
     }
}

void save_edges(std::set<std::pair<VertexID,VertexID>> &edges, const std::string efile){
  //std::cout<<efile<<std::endl;
   std::fstream outfile(efile,std::ios::out);
   if(!outfile)
	{
		std::cout<<"the outfile can  not construct";
		exit(0);
	}
	for(auto e:edges){
	 outfile<<e.first<<' '<<e.second<<std::endl;
	}
	outfile.close();
}

void genertate_random_edges(std::set<std::pair<VertexID,VertexID>> &exist_edges,const std::string basefilename,int dgraph_num_vertices,int generate_edge_num,int ci){
    int j=0;
    std::set<std::pair<VertexID,VertexID>> add_edges;
    while(j<generate_edge_num){
         VertexID node1 = random(0,dgraph_num_vertices-1);
         VertexID node2 = random(0,dgraph_num_vertices-1);
         if (node1!=node2){
             //std::pair<VertexID,VertexID> p(node1,node2);
             if (exist_edges.find(std::make_pair(node1,node2)) == exist_edges.end() && add_edges.find(std::make_pair(node1,node2)) == add_edges.end()){
                 add_edges.insert(std::make_pair(node1,node2));
             }
          }
        j+=1;
    }
    save_edges(add_edges,basefilename+std::to_string(ci));
}