#include "ollama_client.hpp"
#include "rag.hpp"

//definition of ollama methods
#define LIST_MODELS_URI "/api/tags"
#define LIST_MODELS_METHOD "GET"

#define SIMPLE_COMPLETION_URI "/api/generate"
#define SIMPLE_COMPLETION_METHOD "POST"

#define CHAT_COMPLETION_URI "/api/chat"
#define CHAT_COMPLETION_METHOD "POST"

#define GENERATE_EMBED_URI "/api/embeddings"
#define GENERATE_EMBED_METHOD "POST"

#define LOAD_MODEL_URI "/api/generate"
#define LOAD_MODEL_METHOD "POST"

ollama_client::ollama_client(const std::string &backendUrl, const std::string &imageBackendURL, const std::string &embedBackendURL)
{
    this->accessed();
    this->backURL = backendUrl;
    this->model = "llama3:latest";
    this->backURL_img = imageBackendURL;
    this->backURL_emb = embedBackendURL;
}

ollama_client::~ollama_client()
{
}

bool ollama_client::isConnected()
{
    //getting list of models to test connection
    std::string targetURL = backURL + LIST_MODELS_URI;
    std::string response = makeHttpRequest(targetURL, {}, LIST_MODELS_METHOD);
    return response.length() > 1;
}

bool ollama_client::loadModel()
{
    json requestBody = {
        {"model", this->model}
    };

    std::string targetURL = backURL + LOAD_MODEL_URI;
    std::string responseEncoded = makeHttpRequest(targetURL, requestBody, LOAD_MODEL_METHOD);
    json responseBody = json::parse(responseEncoded);
    bool result = responseBody["done"];
    return result;
}

void ollama_client::pingModel()
{
    //this->processSingleMessage("Ping!");
    this->loadModel();
}

std::string ollama_client::processSingleMessage(const std::string& requestMsg)
{
    std::string responseText = {};

    this->lastRequestMessage = requestMsg;
    json requestBody = {
        {"model", this->model},
        {"prompt", this->lastRequestMessage},
        {"stream", this->supportStreaming},
        {"keep_alive", "20m"}
    };

    std::string targetURL = backURL + SIMPLE_COMPLETION_URI;

    //no streaming logic, full response returned in a single blocking request
    std::string responseEncoded = makeHttpRequest(targetURL, requestBody, SIMPLE_COMPLETION_METHOD);
    
    json responseBody = json::parse(responseEncoded);
    responseText = responseBody["response"];      
    this->lastResponseMessage = responseText;
    return responseText;
}

std::string ollama_client::processChatMessage(const std::string& requestMsg, const std::vector<std::pair<std::string, bool>>& chatHistory)
{
    std::string responseText = {};

    this->lastRequestMessage = requestMsg;
    json requestBody = {
        {"model", this->model},
        {"stream", this->supportStreaming},
        {"keep_alive", "20m"}
    };

    requestBody["messages"] = nlohmann::json::array();

    for (const auto& [content, role] : chatHistory) {
        nlohmann::json message;
        message["content"] = content;
        message["role"] = role ? "user" : "assistant";
        requestBody["messages"].push_back(message);
    }

    //process latest one
    {
        nlohmann::json message;
        message["content"] = requestMsg;
        message["role"] = "user";
        requestBody["messages"].push_back(message);
    }

    std::string targetURL = backURL + CHAT_COMPLETION_URI;
    
    //no streaming logic, full response returned in a single blocking request
    std::string responseEncoded = makeHttpRequest(targetURL, requestBody, CHAT_COMPLETION_METHOD);
    LOG("received response: {}", responseEncoded);
    
    json responseBody = json::parse(responseEncoded);
    responseText = responseBody["message"]["content"];      
    this->lastResponseMessage = responseText;
    return responseText;
}

std::string ollama_client::processChatMessageWithImage(const std::string &requestMsg, const std::string &imageB64)
{
    std::string responseText = {};

    this->lastRequestMessage = requestMsg;
    json requestBody = {
        {"model", "llava:latest"},
        {"stream", this->supportStreaming},
        {"keep_alive", "20m"}
    };

    requestBody["messages"] = nlohmann::json::array();

    //process latest one
    {
        nlohmann::json message;
        message["content"] = requestMsg;
        message["role"] = "user";
        message["images"].push_back(imageB64);
        requestBody["messages"].push_back(message);
    }

    std::string targetURL = this->backURL_img + CHAT_COMPLETION_URI;
    
    //no streaming logic, full response returned in a single blocking request
    std::string responseEncoded = makeHttpRequest(targetURL, requestBody, CHAT_COMPLETION_METHOD);

    json responseBody = json::parse(responseEncoded);
    responseText = responseBody["message"]["content"];      
    this->lastResponseMessage = responseText;
    return responseText;
}

std::string ollama_client::processChatMessageWithEmbed(const std::string &requestMsg, const std::string &embedData, const std::vector<std::pair<std::string, bool>> &chatHistory)
{
    std::string responseText = {};

    std::string prompt = "Using this data: ";
    prompt += embedData + ". Respond to this prompt: " + requestMsg;
    LOG("prompt: {}", prompt.c_str());


    this->lastRequestMessage = requestMsg;
    json requestBody = {
        {"model", this->model},
        {"stream", this->supportStreaming},
        {"keep_alive", "20m"}
    };

    requestBody["messages"] = nlohmann::json::array();

    {
        nlohmann::json message;
        message["content"] = rag::getSystemMessage();
        message["role"] = "system";
        requestBody["messages"].push_back(message);
    }

    for (const auto& [content, role] : chatHistory) {
        nlohmann::json message;
        message["content"] = content;
        message["role"] = role ? "user" : "assistant";
        requestBody["messages"].push_back(message);
    }

    {
        nlohmann::json message;
        message["content"] = prompt;
        message["role"] = "user";
        requestBody["messages"].push_back(message);
    }

    std::string targetURL = this->backURL_emb + CHAT_COMPLETION_URI;
    
    //no streaming logic, full response returned in a single blocking request
    std::string responseEncoded = makeHttpRequest(targetURL, requestBody, CHAT_COMPLETION_METHOD);
    
    json responseBody = json::parse(responseEncoded);
    responseText = responseBody["message"]["content"];      
    this->lastResponseMessage = responseText;
    return responseText;
}


std::vector<float> ollama_client::processNewEmbedding(const std::string& embedding)
{
    std::string responseText = {};

    json requestBody = {
        {"model", "mxbai-embed-large:latest"},
        {"prompt", embedding}
    };

    std::string targetURL = this->backURL_emb + GENERATE_EMBED_URI;
    
    std::string responseEncoded = makeHttpRequest(targetURL, requestBody, GENERATE_EMBED_METHOD);

    json responseBody = json::parse(responseEncoded);
    // printf("\nresponse: %.50s\n", responseBody.dump(2).c_str());
    std::vector<float> newEmbs = responseBody["embedding"]; 
    return newEmbs;
}

std::string ollama_client::processListModels()
{
    std::string targetURL = backURL + LIST_MODELS_URI;
    std::string responseEncoded = makeHttpRequest(targetURL, {}, LIST_MODELS_METHOD);
    json responseBody = json::parse(responseEncoded);
    std::vector<nlohmann::json> models = responseBody["models"];
    std::vector<std::string> modelNames = {};
    for (const auto& model : models) 
    {
        modelNames.push_back(model["name"]);
    }
    json output;
    output["models"] = modelNames;
    return output.dump();
}
