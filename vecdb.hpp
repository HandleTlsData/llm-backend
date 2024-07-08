#include "common.hpp"
#include "hnswlib/hnswlib/hnswlib.h"

#define L2SZ 16000

class vecdb
{
private:
    std::string internalName = {};
    std::unique_ptr<hnswlib::L2Space> hnswSpace;
    std::unique_ptr<hnswlib::HierarchicalNSW<float>> hnswIndex;
    /* data */
public:
    //load from fs and deserialize index
    vecdb(const std::string& collectionName);
    //generate new index, serialize, save to fs 
    vecdb(const std::string& collectionName, const std::vector<std::vector<float>>& vecDocs);
    ~vecdb();

    std::vector<int> findNearestIds(const std::vector<float> vectoredQuestion);
};

