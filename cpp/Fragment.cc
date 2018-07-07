#include "cpp/Fragment.h"
//typedef typename std::pair<VertexID, VertexID> EdgePair;
Fragment::Fragment(){};

Fragment::~Fragment(){}

Fragment::Fragment(Graph &graph, const std::string v_file, const std::string e_file, const std::string r_file){
  FID = get_worker_id();
  std::vector<Vertex> global_vertices;
  std::vector<Edge> global_edges;
  FragmentLoader fragmentloader;
  fragmentloader.LoadFragTable(fragTable,r_file);
  std::unordered_set<VertexID> idx;
  fragmentloader.LoadEdge(global_edges, fragTable,idx, FID, e_file);
  //std::cout<<"edges "<<global_edges.size()<<' '<<fid<<std::endl;
  fragmentloader.LoadVertex(global_vertices, fragTable,idx, FID, v_file);
 // std::cout<<"vertex "<<global_vertices.size()<<" "<<fid<<std::endl;

  numVertices = global_vertices.size();
  numEdges = global_edges.size();

  graph.vertices_label_.resize(global_vertices.size());
  for(int i = 0 ; i < global_vertices.size(); ++i){
       VertexID gvid = global_vertices[i].id();
       local2global[i] = gvid;
       global2local[gvid] = i;
       graph.vertices_label_[i] = global_vertices[i].label();
       if (fragTable.at(gvid) == FID){
           innerVertices.insert(gvid);
       }else{
        outerVertices.insert(gvid);
       }
    }
    std::vector<std::pair<VertexID,VertexID>> edges_from_to;
    std::vector<EdgeLabel> edges_attr;

    for (int i = 0; i < numEdges; ++i){
        Edge e = global_edges[i];
        VertexID src = e.src(),dst = e.dst();
        edges_from_to.emplace_back(global2local.at(src), global2local.at(dst));
        edges_attr.emplace_back(e.attr());
        if(fragTable.at(src) != FID && fragTable.at(dst) ==FID){
            msgThroughDest[dst].set(fragTable.at(src));
        }else if(fragTable.at(src) == FID && fragTable.at(dst) != FID){
            msgThroughDest[src].set(fragTable.at(dst));
        }
    }
    LOG(INFO) << "fragment_" << FID << ": innervsize = " << innerVertices.size() << ", outervsize = " << outerVertices.size() << " totalvsize = " << numVertices << std::endl;
    LOG(INFO) << "fragment_" << FID << ": edgesize = " << ": " << edges_from_to.size() << std::endl;
#ifndef MUTABLE_GRAPH
  graph.graph_ = std::shared_ptr<GraphT>(
      new GraphT(boost::edges_are_unsorted_multi_pass,
                 edges_from_to.begin(),
                 edges_from_to.end(),
                 edges_attr.begin(),
                 global_vertices.size()));
#else
  graph.graph_ = std::shared_ptr<GraphT>(
      new GraphT(edges_from_to.begin(),
                 edges_from_to.end(),
                 edges_attr.begin(),
                 global_vertices.size()));
#endif
  graph.vertex_index_map_ = boost::get(boost::vertex_index, *graph.graph_);
  graph.edge_data_map_ = std::unique_ptr<EdgeDataMap>(
          new EdgeDataMap(boost::get(boost::edge_weight, *graph.graph_)));
  graph.num_vertices_ = numVertices;
  graph.num_edges_ = numEdges;
  LOG(INFO) << "loading graph finished." << std::endl;
}

Fragment::Fragment(Graph &graph,const std::vector<Vertex> &_vertices, const std::vector<Edge> &_edges,
              const std::unordered_map<VertexID,int> &_fragTable, int _FID): fragTable(_fragTable), FID(_FID){
    numVertices = _vertices.size();
    numEdges = _edges.size();

    graph.vertices_label_.resize(_vertices.size());
    for(int i = 0 ; i < _vertices.size(); ++i){
       VertexID gvid = _vertices[i].id();
       local2global[i] = gvid;
       global2local[gvid] = i;
       graph.vertices_label_[i] = _vertices[i].label();
       if (fragTable.at(gvid) == FID){
           innerVertices.insert(gvid);
       }else{
        outerVertices.insert(gvid);
       }
    }
    std::vector<std::pair<VertexID,VertexID>> edges_from_to;
    std::vector<EdgeLabel> edges_attr;

    for (int i = 0; i < numEdges; ++i){
        Edge e = _edges[i];
        VertexID src = e.src(),dst = e.dst();
        edges_from_to.emplace_back(global2local.at(src), global2local.at(dst));
        edges_attr.emplace_back(e.attr());
        if(fragTable.at(src) != FID && fragTable.at(dst) ==FID){
            msgThroughDest[dst].set(fragTable.at(src));
        }else if(fragTable.at(src) == FID && fragTable.at(dst) != FID){
            msgThroughDest[src].set(fragTable.at(dst));
        }
    }
    LOG(INFO) << "fragment_" << FID << ": innervsize = " << innerVertices.size() << ", outervsize = " << outerVertices.size() << " totalvsize = " << numVertices << std::endl;
    LOG(INFO) << "fragment_" << FID << ": edgesize = " << ": " << edges_from_to.size() << std::endl;
#ifndef MUTABLE_GRAPH
  graph.graph_ = std::shared_ptr<GraphT>(
      new GraphT(boost::edges_are_unsorted_multi_pass,
                 edges_from_to.begin(),
                 edges_from_to.end(),
                 edges_attr.begin(),
                 _vertices.size()));
#else
  graph.graph_ = std::shared_ptr<GraphT>(
      new GraphT(edges_from_to.begin(),
                 edges_from_to.end(),
                 edges_attr.begin(),
                 _vertices.size()));
#endif
  graph.vertex_index_map_ = boost::get(boost::vertex_index, *graph.graph_);
  graph.edge_data_map_ = std::unique_ptr<EdgeDataMap>(
          new EdgeDataMap(boost::get(boost::edge_weight, *graph.graph_)));
  graph.num_vertices_ = numVertices;
  graph.num_edges_ = numEdges;
  LOG(INFO) << "loading graph finished." << std::endl;
 }

int Fragment::getVertexFragmentID(VertexID u)const{
    if (fragTable.find(u) != fragTable.end()){
        return fragTable.at(u);
    }
    return -1;
}

int Fragment::getLocalID(VertexID gvid){
        return global2local[gvid];
}

int Fragment::getGlobalID(VertexID localid){
        return local2global[localid];
}

int Fragment::getNumVertices() const {
       return numVertices;
}

int Fragment::getNumEdges() const {
        return numEdges;
}

const std::unordered_set<VertexID>* Fragment::getInnerVertices() const{
        return &innerVertices;
}

const std::unordered_set<VertexID>* Fragment::getOuterVertices() const{
        return &outerVertices;
}

const bool Fragment::isBorderVertex(const VertexID gvid){
        return msgThroughDest.find(gvid) != msgThroughDest.end();
}

const bool Fragment::isInnerVertex(const VertexID gvid){
        return innerVertices.find(gvid) != innerVertices.end();
}

const bool Fragment::isOuterVertex(const VertexID gvid){
        return outerVertices.find(gvid) != outerVertices.end();
}

const std::bitset<NUM_FRAGMENTS> Fragment::getMsgThroughDest(const VertexID gvid){
		//TODO: assert in_only or both
		return msgThroughDest.at(gvid);
	}

const bool Fragment::has_vertex(const VertexID gvid) const{
    return global2local.find(gvid)!=global2local.end();
}

void Fragment::update_fragment(Graph &graph,std::vector<Edge> &add_edges,std::vector<Edge> &rm_edges, std::vector<Vertex> &add_vertices){
    int tmp_num_vertices = graph.GetNumVertices();
    for(int i=0;i<add_vertices.size(); ++i){
        VertexID gvid = add_vertices[i].id();
        if(global2local.find(gvid) == global2local.end()){
            graph.AddVertex(add_vertices[i]);
            local2global[tmp_num_vertices] = gvid;
            global2local[gvid] = tmp_num_vertices;
            tmp_num_vertices++;
        }
    }
    for(auto edge :add_edges){
        graph.AddEdge(Edge(global2local[edge.src()], global2local[edge.dst()], edge.attr()));
    }
    for(auto edge: rm_edges){
        graph.RemoveEdge(Edge(global2local[edge.src()], global2local[edge.dst()], edge.attr()));
    }
    graph.RebuildGraphProperties();
}

void Fragment::update_fragment(Graph &graph,std::unordered_set<Edge> &add_edges, std::unordered_set<Edge> &rm_edges, std::unordered_set<Vertex> &vertices){
    int tmp_num_vertices = graph.GetNumVertices();
    for(auto vertex : vertices){
        VertexID gvid = vertex.id();
        if(global2local.find(gvid) == global2local.end()){
            graph.AddVertex(vertex);
            local2global[tmp_num_vertices] = gvid;
            global2local[gvid] = tmp_num_vertices;
            outerVertices.insert(gvid);
            tmp_num_vertices++;
        }
    }
    for(auto edge : add_edges){
        graph.AddEdge(Edge(global2local[edge.src()], global2local[edge.dst()], edge.attr()));
    }
    for(auto edge: rm_edges){
        graph.RemoveEdge(Edge(global2local[edge.src()], global2local[edge.dst()], edge.attr()));
    }
    graph.RebuildGraphProperties();
}