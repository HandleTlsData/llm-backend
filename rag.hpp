#include "common.hpp"
#include "vecdb.hpp"

class rag
{
private:
    /* data */
    std::unordered_map<std::string, std::shared_ptr<vecdb>> m_vecdbMap;
public:
    rag(/* args */);
    ~rag();

    //document name = vecdb instance
    std::shared_ptr<vecdb> createNewDoc(const std::string& embedName, std::vector<std::vector<float>>& vecDocs);
    static std::string getSystemMessage();
};
