#include "vecdb.hpp"

vecdb::vecdb(const std::string &collectionName)
{
    std::string path = collectionName;
    this->hnswSpace = std::make_unique<hnswlib::L2Space>(L2SZ);
    this->hnswIndex = std::make_unique<hnswlib::HierarchicalNSW<float>>(this->hnswSpace.get(), path);
}

vecdb::vecdb(const std::string &collectionName, const std::vector<std::vector<float>>& vecDocs)
{
    std::string path = collectionName;
    this->hnswSpace = std::make_unique<hnswlib::L2Space>(L2SZ);
    int max_elements = 10000;   
    int M = 16;             
    int ef_construction = 200;  
    this->hnswIndex = std::make_unique<hnswlib::HierarchicalNSW<float>>(this->hnswSpace.get(), max_elements, M, ef_construction);

    for (size_t i = 0; i < vecDocs.size(); ++i) {
        this->hnswIndex->addPoint(vecDocs[i].data(), i);
    }

    this->hnswIndex->saveIndex(path);
}

vecdb::~vecdb()
{
}

std::vector<int> vecdb::findNearestIds(const std::vector<float> vectoredQuestion)
{
    std::vector<int> result = {};
    auto searchResult = this->hnswIndex->searchKnnCloserFirst(vectoredQuestion.data(), 3);
    for(auto& x : searchResult)
    {
        hnswlib::labeltype label = x.second;
        result.push_back(label);
    }
    return result;
}
