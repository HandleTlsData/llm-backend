#include "rag.hpp"

//ref: https://huggingface.co/nvidia/Llama3-ChatQA-1.5-70B
const std::string chatQAPersPrompt = "System: This is a chat between a user and an artificial intelligence assistant. The assistant gives helpful, detailed," \
            "and polite answers to the user's questions based on the context. The assistant should also indicate when the answer cannot be found in the context.";


rag::rag(/* args */)
{
}

rag::~rag()
{
}

std::shared_ptr<vecdb> rag::createNewDoc(const std::string &embedName, std::vector<std::vector<float>>& vecDocs)
{
    auto it = m_vecdbMap.find(embedName);
    if (it == m_vecdbMap.end()) {
        LOG("creating new vecdb");
        auto newVecDB = std::make_shared<vecdb>(embedName, vecDocs);
        m_vecdbMap[embedName] = newVecDB;
        return newVecDB;
    } 
    else 
    {
        ERRLOG("vecdb {} already exists. unexpected", embedName);
        throw;
    }

}

std::string rag::getSystemMessage()
{  
    return chatQAPersPrompt;
}
