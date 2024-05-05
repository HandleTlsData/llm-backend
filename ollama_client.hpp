#include "common.hpp"
#include "ai_client.hpp"

class ollama_client : public ai_client
{
private:
    std::string backURL = {};
public:
    //backendUrl: "http://address:port", modelName: "model:tag"
    ollama_client(std::string backendUrl, std::string modelName);
    ~ollama_client();
    bool isConnected();

    bool loadModel();
    void pingModel();

    std::string processSingleMessage(const std::string& requestMsg);
    std::string processChatMessage(const std::string& requestMsg, const std::vector<std::pair<std::string, bool>>& chatHistory);
    std::string processChatMessageWithImage(const std::string& requestMsg, const std::string& imageB64, const std::vector<std::pair<std::string, bool>>& chatHistory);
    std::string processChatMessageWithEmbed(const std::string& requestMsg, const std::string& embedData, const std::vector<std::pair<std::string, bool>>& chatHistory);
    std::string processNewEmbedding(const std::string& embedding);
};

