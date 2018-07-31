#include "generate.h"

Generate::Generate(){srand( (unsigned)time(0));}

Generate::Generate(std::string graph_vf,std::string graph_ef,std::string graph_rf,
             std::string base_qf,std::string base_add_f,std::string base_remove_f,int q_index){
    std::string graph_vfile =graph_vf;
    std::string graph_efile =graph_ef;
    std::string r_file = graph_rf;
    std::string base_qfile = base_qf;
    std::string base_add_file = base_add_f;
    std::string base_remove_file=base_remove_f;
    query_index = q_index;
 }

Generate::~Generate(){}

std::vector<VertexLabel> Generate::get_graph_label_vec(Graph &graph){
      std::unordered_set<VertexLabel> label_set;
      for(auto u :graph.GetAllVerticesID()){
           label_set.insert(graph.GetVertexLabel(u));
      }
      std::vector<VertexLabel> label_vec;
      for(auto label:label_set){
          label_vec.push_back(label);
      }
      return label_vec;
    }

void Generate::generate_random_connectivity_graph(Graph &graph,int num_nodes,int num_edges,std::vector<VertexLabel> &labels){
    std::vector<Vertex> vertices;
    std::vector<Edge> edges;
    std::set<std::pair<VertexID,VertexID>> exist_edges;
    int i = 0;
    std::unordered_set<VertexID> connectivity_nodes;
    while(i<num_edges){
        int n1 = random(0,num_nodes-1);
        int n2 = random(0,num_nodes-1);
        if(n1==n2 ||exist_edges.find(std::pair<VertexID,VertexID>(n1,n2)) != exist_edges.end()){
            continue;
        }
        if(connectivity_nodes.size()==0){
            connectivity_nodes.insert(n1);
            connectivity_nodes.insert(n2);
            edges.emplace_back(n1,n2,1);
            exist_edges.insert(std::make_pair(n1,n2));
            ++i;
        }else if(connectivity_nodes.size()>0 && connectivity_nodes.size()<num_nodes){
            if((connectivity_nodes.find(n1)!=connectivity_nodes.end() && connectivity_nodes.find(n2)==connectivity_nodes.end()) || (connectivity_nodes.find(n2)!=connectivity_nodes.end() && connectivity_nodes.find(n1)==connectivity_nodes.end())){
                connectivity_nodes.insert(n1);
                connectivity_nodes.insert(n2);
                edges.emplace_back(n1,n2,1);
                exist_edges.insert(std::make_pair(n1,n2));
                ++i;
            }
        }else{
            edges.emplace_back(n1,n2,1);
            exist_edges.insert(std::make_pair(n1,n2));
            ++i;
        }
    }
    int labels_len = labels.size();
    for(int j=0;j<num_nodes;j++){
        int label_index= random(0,labels_len-1);
        vertices.emplace_back(j,labels[label_index]);
    }
    GraphLoader graph_loader;
    graph_loader.LoadGraph(graph,vertices,edges);

}

void Generate::generate_random_connectivity_graph(Graph &graph,int num_nodes,int num_edges,int l){
    std::vector<Vertex> vertices;
    std::vector<Edge> edges;
    std::set<std::pair<VertexID,VertexID>> exist_edges;
    int i = 0;
    std::unordered_set<VertexID> connectivity_nodes;
    while(i<num_edges){
        int n1 = random(0,num_nodes-1);
        int n2 = random(0,num_nodes-1);
        if(n1==n2 ||exist_edges.find(std::pair<VertexID,VertexID>(n1,n2)) != exist_edges.end()){
            continue;
        }
        if(connectivity_nodes.size()==0){
            connectivity_nodes.insert(n1);
            connectivity_nodes.insert(n2);
            edges.emplace_back(n1,n2,1);
            exist_edges.insert(std::make_pair(n1,n2));
            ++i;
        }else if(connectivity_nodes.size()>0 && connectivity_nodes.size()<num_nodes){
            if((connectivity_nodes.find(n1)!=connectivity_nodes.end() && connectivity_nodes.find(n2)==connectivity_nodes.end()) || (connectivity_nodes.find(n2)!=connectivity_nodes.end() && connectivity_nodes.find(n1)==connectivity_nodes.end())){
                connectivity_nodes.insert(n1);
                connectivity_nodes.insert(n2);
                edges.emplace_back(n1,n2,1);
                exist_edges.insert(std::make_pair(n1,n2));
                ++i;
            }
        }else{
            edges.emplace_back(n1,n2,1);
            exist_edges.insert(std::make_pair(n1,n2));
            ++i;
        }
    }
    for(int j=0;j<num_nodes;j++){
        int label= random(0,l);
        vertices.emplace_back(j,label);
    }
    GraphLoader graph_loader;
    graph_loader.LoadGraph(graph,vertices,edges);
}

void Generate::generate_random_dgraph(Graph &graph,int num_nodes,double a,int l){
    std::vector<Vertex> vertices;
    std::vector<Edge> edges;
    std::set<std::pair<VertexID,VertexID>> exist_edges;
    int num_edges = pow(num_nodes,a);
    int i = 0;

    while(i<num_edges){
        int n1 = random(0,num_nodes-1);
        int n2 = random(0,num_nodes-1);
        if(n1!=n2 && exist_edges.find(std::pair<VertexID,VertexID>(n1,n2)) == exist_edges.end()){
            edges.emplace_back(n1,n2,1);
            exist_edges.insert(std::make_pair(n1,n2));
            ++i;
        }
    }
    for(int j=0;j<num_nodes;j++){
        int label= random(0,l);
        vertices.emplace_back(j,label);
    }
    GraphLoader graph_loader;
    graph_loader.LoadGraph(graph,vertices,edges);
}

void Generate::generate_connect_graphs_by_Dgraph(Graph &dgraph, Graph &qgraph,const  int num_nodes){
    std::vector<Vertex> vertices;
    std::vector<Edge> edges;
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

void Generate::save_grape_file(Graph &qgraph, const std::string &v_file, const std::string &e_file){
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

std::unordered_set<VertexID> Generate::get_dual_node_result(Graph &dgraph,Graph &qgraph){
        std::unordered_map<VertexID, std::unordered_set<VertexID>> sim;
        DualSim dualsim;
        bool initialized_sim=false;
        dualsim.dual_simulation(dgraph,qgraph,sim,initialized_sim);
        std::unordered_set<VertexID> dual_noderesult;
        for(auto u :qgraph.GetAllVerticesID()){
            for(auto v:sim[u]){
                dual_noderesult.insert(v);
            }
        }
    return dual_noderesult;
}

std::pair<VertexID,VertexID>  Generate::generate_one_add_edge_by_nodelist(std::vector<VertexID> &node_list,std::set<std::pair<VertexID,VertexID>> &exist_edges,std::set<std::pair<VertexID,VertexID>> &add_edges){
    while(true){
        VertexID n1 = random(0,node_list.size()-1);
        VertexID n2 = random(0,node_list.size()-1);
        if (n1!=n2){
            if (exist_edges.find(std::pair<VertexID,VertexID>(node_list[n1],node_list[n2])) == exist_edges.end() && add_edges.find(std::pair<VertexID,VertexID>(node_list[n1],node_list[n2])) == add_edges.end()){
                return std::pair<VertexID,VertexID>(node_list[n1],node_list[n2]);
            }
        }
    }
 }

 std::pair<VertexID,VertexID> Generate::generate_one_add_edge_random(int node_num, std::set<std::pair<VertexID,VertexID>> &exist_edges,std::set<std::pair<VertexID,VertexID>> &add_edges){
    while(true){
        VertexID n1 = random(0,node_num-1);
        VertexID n2 = random(0,node_num-1);
        if (n1!=n2){
            if (exist_edges.find(std::pair<VertexID,VertexID>(n1,n2)) == exist_edges.end() && add_edges.find(std::pair<VertexID,VertexID>(n1,n2)) == add_edges.end()){
                return std::pair<VertexID,VertexID>(n1,n2);
            }
        }
    }


 }

 std::pair<VertexID,VertexID> Generate::generate_one_remove_edge_by_list(std::vector<std::pair<VertexID,VertexID>> &edge_list, std::set<std::pair<VertexID,VertexID>> &already_rm_edges,std::set<std::pair<VertexID,VertexID>> &remove_edges){
      while (true){
          int n1 = random(0,edge_list.size()-1);
          if (already_rm_edges.find(edge_list[n1]) == already_rm_edges.end() && remove_edges.find(edge_list[n1]) == remove_edges.end()){
              return edge_list[n1];
          }
      }
 }

std::set<std::pair<VertexID,VertexID>> Generate::generate_bunch_add_edges_random(int node_num, std::set<std::pair<VertexID,VertexID>> &exist_edges,int num_edges){
     std::set<std::pair<VertexID,VertexID>> add_e_set;
     int j =0;
     while(j<num_edges){
         std::pair<VertexID,VertexID> e = generate_one_add_edge_random(node_num,exist_edges,add_e_set);
         add_e_set.insert(e);
         j++;
     }
     return add_e_set;
}

std::set<std::pair<VertexID,VertexID>> Generate::generate_bunch_add_edges_by_nodelist(std::vector<VertexID> &node_list,std::set<std::pair<VertexID,VertexID>> &exist_edges,int num_edges){
     std::set<std::pair<VertexID,VertexID>> add_e_set;
     int j =0;
     while(j<num_edges){
         std::pair<VertexID,VertexID> e = generate_one_add_edge_by_nodelist(node_list,exist_edges,add_e_set);
         add_e_set.insert(e);
         j++;
     }
     return add_e_set;
}

std::set<std::pair<VertexID,VertexID>> Generate::generate_bunch_remove_edge_by_list(std::vector<std::pair<VertexID,VertexID>> &edge_list, std::set<std::pair<VertexID,VertexID>> &already_rm_edges,int num_edges){
    std::set<std::pair<VertexID,VertexID>> remove_e_set;
    int j=0;
    while(j<num_edges){
        std::pair<VertexID,VertexID> e = generate_one_remove_edge_by_list(edge_list,already_rm_edges,remove_e_set);
        remove_e_set.insert(e);
        j++;
    }
    return remove_e_set;
}

std::pair<VertexID,VertexID> Generate::generate_one_remove_edge_by_set(std::set<std::pair<VertexID,VertexID>> &edge_set, std::set<std::pair<VertexID,VertexID>> &already_rm_edges,std::set<std::pair<VertexID,VertexID>> &remove_edges){
    std::vector<std::pair<VertexID,VertexID>> edge_list;
    for(auto e :edge_set){
        edge_list.push_back(e);
    }
    return generate_one_remove_edge_by_list(edge_list,already_rm_edges,remove_edges);
}

std::set<std::pair<VertexID,VertexID>> Generate::generate_bunch_remove_edge_by_set(std::set<std::pair<VertexID,VertexID>> &edge_set, std::set<std::pair<VertexID,VertexID>> &already_rm_edges,int num_edges){
    std::vector<std::pair<VertexID,VertexID>> edge_list;
    for(auto e :edge_set){
        edge_list.push_back(e);
    }
    return generate_bunch_remove_edge_by_list(edge_list,already_rm_edges,num_edges);
}