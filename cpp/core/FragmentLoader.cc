#include "FragmentLoader.h"

FragmentLoader::FragmentLoader(){}

FragmentLoader::~FragmentLoader(){}

void FragmentLoader::LoadFragTable(std::unordered_map<VertexID , int> &fragTable, const std::string fragTablePath){
      FILE *file = fopen(fragTablePath.c_str(),"r");
      if (file == NULL){
          printf("Cannot open the file\n");
          exit(0);
      }
      VertexID vid;
      int fid;
      while(fscanf(file,"%d %d",&vid,&fid) !=EOF){
           fragTable[vid] = fid;
      }
      fclose(file);
      return ;
}


bool FragmentLoader::LoadEdge(std::vector<Edge> &global_edges, const std::unordered_map<VertexID,int> &fragTable,
                                        std::unordered_set<VertexID> &idx,  int fid, const std::string e_file){
	 try {
		 FILE *file = fopen(e_file.c_str(), "r");
		 if(file==NULL){
		  std::cout<<"not found"<<std::endl;
		  exit(0);
		  return false;
		 }
         VertexID u,v;
         EdgeLabel attr;
         while (fscanf(file,"%d %d %d",&u, &v, &attr) != EOF){
            if (fragTable.at(u) == fid || fragTable.at(v) == fid){
                  global_edges.emplace_back(u, v, attr);
                  idx.insert(u);
                  idx.insert(v);
            }
           }
           fclose(file);
	       } catch (...) {
		   return false;
	}
	return true;
 }

bool FragmentLoader::LoadVertex(std::vector<Vertex> &global_vertices, const std::unordered_map<VertexID,int> &fragTable,
                                        std::unordered_set<VertexID> &idx,  int fid, const std::string v_file){
     try {
         FILE *file = fopen(v_file.c_str(),"r");
       	 if(file==NULL){
		  std::cout<<"not found"<<std::endl;
		  exit(0);
		  return false;
		 }
         VertexID v;
         int attr;
         while (fscanf(file,"%d %d", &v, &attr) != EOF) {
             if(fragTable.at(v) == fid || idx.find(v) != idx.end()){
                global_vertices.emplace_back(v, attr);
             }
             char *line = NULL;
             size_t len = 0;
             getline(&line, &len, file);
             free(line);
         }
         fclose(file);
       } catch (...) {
		return false;
	}
   return true;
}
//using namespace std;