#include "ollama_client.hpp"

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

ollama_client::ollama_client(std::string backendUrl, std::string modelName)
{
    this->backURL = backendUrl;
    this->model = modelName;
    this->accessed();
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

    //unexpected
    if(this->isProcessingResponse)
        return {};

    this->lastRequestMessage = requestMsg;
    json requestBody = {
        {"model", this->model},
        {"prompt", this->lastRequestMessage},
        {"stream", this->supportStreaming},
        {"keep_alive", "20m"}
    };

    std::string targetURL = backURL + SIMPLE_COMPLETION_URI;

    //no streaming logic, full response returned in a single blocking request
    this->isProcessingResponse = true;
    std::string responseEncoded = makeHttpRequest(targetURL, requestBody, SIMPLE_COMPLETION_METHOD);
    
    json responseBody = json::parse(responseEncoded);
    responseText = responseBody["response"];      
    this->lastResponseMessage = responseText;
    this->isProcessingResponse = false;
    return responseText;
}

std::string ollama_client::processChatMessage(const std::string& requestMsg, const std::vector<std::pair<std::string, bool>>& chatHistory)
{
    std::string responseText = {};

    if(this->isProcessingResponse)
    {
        printf("\nstill processing response\n");
        return {};
    }

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
    this->isProcessingResponse = true;
    std::string responseEncoded = makeHttpRequest(targetURL, requestBody, CHAT_COMPLETION_METHOD);
    
    json responseBody = json::parse(responseEncoded);
    responseText = responseBody["message"]["content"];      
    this->lastResponseMessage = responseText;
    this->isProcessingResponse = false;
    return responseText;
}

std::string ollama_client::processChatMessageWithImage(const std::string &requestMsg, const std::string &imageB64, const std::vector<std::pair<std::string, bool>> &chatHistory)
{
    std::string responseText = {};

    if(this->isProcessingResponse)
    {
        printf("\nstill processing response\n");
        return {};
    }

    this->lastRequestMessage = requestMsg;
    json requestBody = {
        {"model", "llava:latest"},
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
        message["images"].push_back(imageB64);
        requestBody["messages"].push_back(message);
    }

    std::string targetURL = backURL + CHAT_COMPLETION_URI;
    
    //no streaming logic, full response returned in a single blocking request
    this->isProcessingResponse = true;
    std::string responseEncoded = makeHttpRequest(targetURL, requestBody, CHAT_COMPLETION_METHOD);
    this->isProcessingResponse = false;
    json responseBody = json::parse(responseEncoded);
    responseText = responseBody["message"]["content"];      
    this->lastResponseMessage = responseText;
    return responseText;
}

std::string ollama_client::processChatMessageWithEmbed(const std::string &requestMsg, const std::string &embedData, const std::vector<std::pair<std::string, bool>> &chatHistory)
{
    std::string responseText = {};

    if(this->isProcessingResponse)
    {
        printf("\nstill processing response\n");
        return {};
    }

    std::string prompt = "Using this data: ";
    prompt += embedData + ". Respond to this prompt: " + requestMsg;
    printf("\nprompt:%s\n", prompt.c_str());


    this->lastRequestMessage = requestMsg;
    json requestBody = {
        {"model", this->model},
        {"stream", this->supportStreaming},
        {"keep_alive", "20m"}
    };

    requestBody["messages"] = nlohmann::json::array();

    {
        nlohmann::json message;
        message["content"] = "You are an assistant for answering questions.\nYou are given the extracted parts of a long document and a question. Provide a conversational answer.\nIf you don't know the answer, just say \"I do not know.\" Don't make up an answer.";
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

    std::string targetURL = backURL + CHAT_COMPLETION_URI;
    
    //no streaming logic, full response returned in a single blocking request
    this->isProcessingResponse = true;
    std::string responseEncoded = makeHttpRequest(targetURL, requestBody, CHAT_COMPLETION_METHOD);
    
    json responseBody = json::parse(responseEncoded);
    responseText = responseBody["message"]["content"];      
    this->lastResponseMessage = responseText;
    this->isProcessingResponse = false;
    return responseText;
}

std::string ollama_client::processNewEmbedding(const std::string& embedding)
{
    std::string responseText = {};

    //unexpected
    if(this->isProcessingResponse)
        return {};

    json requestBody = {
        {"model", "snowflake-arctic-embed:latest"},
        {"prompt", embedding}
    };

    std::string targetURL = backURL + GENERATE_EMBED_URI;
    
    this->isProcessingResponse = true;
    std::string responseEncoded = makeHttpRequest(targetURL, requestBody, GENERATE_EMBED_METHOD);
    this->isProcessingResponse = false;

    json responseBody = json::parse(responseEncoded);
    ;
    printf("\nresponse: %.50s\n", responseBody.dump(2).c_str());
    //std::vector<float> vecArr = responseBody["embedding"];
    if(!responseBody.contains("embedding"))
        throw;
    responseText = responseEncoded;//["embedding"];      
    return responseText;
}
