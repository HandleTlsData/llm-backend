#include "common.hpp"
#include "ai_client.hpp"

class ollama_client : public ai_client
{
private:
    std::string backURL = {};
    std::string backURL_img = {};
    std::string backURL_emb = {};
public:
    //backendUrl: "http://address:port", modelName: "model:tag"
    ollama_client(const std::string& backendUrl, const std::string& imageBackendURL, const std::string& embedBackendURL);
    ~ollama_client();

public:
    bool isConnected();
    bool loadModel();
    void pingModel();
    
public:
    std::string processSingleMessage(const std::string& requestMsg);
    std::string processChatMessage(const std::string& requestMsg, const std::vector<std::pair<std::string, bool>>& chatHistory);
    std::string processChatMessageWithImage(const std::string& requestMsg, const std::string& imageB64);
    std::string processChatMessageWithEmbed(const std::string& requestMsg, const std::string& embedData, const std::vector<std::pair<std::string, bool>>& chatHistory);
    std::string processMessageWithCommandHandler(const std::string& requestMsg);

    std::vector<float> processNewEmbedding(const std::string& embedding);
    std::string processListModels();
};

