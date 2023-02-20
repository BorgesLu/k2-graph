#include "K2Vertex.h"

void K2Vertex::addOutEdge(int32_t edgeType, int secondVID)
{
    auto it = m_outVertexIds.find(edgeType);
    //新的类型的边，
    if(it == m_outVertexIds.end()){
        std::vector<int> vec;
        vec.push_back(secondVID);
        m_outVertexIds.insert(make_pair(edgeType,vec));
    }else {
        it->second.push_back(secondVID);
    }

}