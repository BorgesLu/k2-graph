#pragma once

#include<map>
#include<vector>

class K2Vertex
{
private:
    /* data */
public:
    K2Vertex(){};
    ~K2Vertex();
    void addOutEdge(int32_t edgeType, int secondVID);
    int getOutVertexId(int schemaID,int index);
	int getInEdgeId(int schemaID,int index);

protected:  
   std::map<int32_t,std::vector<int>>   m_outVertexIds;
   std::map<int32_t,std::vector<int>>   m_inVertexIds;
   

};

