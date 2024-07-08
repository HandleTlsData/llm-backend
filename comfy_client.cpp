#include "comfy_client.hpp"
#include "comfy_worklows.hpp"

#define SIMPLE_TXT2IMG_URI "/prompt"
#define SIMPLE_TXT2IMG_METHOD "POST"

#define HISTORY_URI "/history/"
#define HISTORY_METHOD "GET"

std::string comfy_client::getHistory(const std::string &promptID)
{
    std::string targetURL = backURL + HISTORY_URI + promptID;
    std::string responseEncoded = makeHttpRequest(targetURL, "", HISTORY_METHOD);
    return responseEncoded;
}

comfy_client::comfy_client(const std::string &backendUrl)
{
    this->accessed();
    this->backURL = backendUrl;
    this->model = "sd3_medium.safetensors";
}

comfy_client::~comfy_client()
{
}

std::string comfy_client::processSingleTxt2Img(const std::string &positivePrompt, const std::string &negativePrompt)
{
    auto workflow = comfy_workflows::defaultWorkflow;
    auto clientID = generateString(24);

    workflow["client_id"] = clientID;
    workflow["prompt"]["4"]["inputs"]["ckpt_name"] = this->model;
    workflow["prompt"]["6"]["inputs"]["text"] = positivePrompt;
    workflow["prompt"]["7"]["inputs"]["text"] = negativePrompt;

    std::string targetURL = backURL + SIMPLE_TXT2IMG_URI;
    std::string responseEncoded = makeHttpRequest(targetURL, workflow.dump(), SIMPLE_TXT2IMG_METHOD);

    json responseBody = json::parse(responseEncoded);
    std::string promptID = responseBody["prompt_id"];

    bool imageGenerated = false;
    while(!imageGenerated)
    {
        auto strResponse = getHistory(promptID);
        auto jResponse = json::parse(strResponse);
        if(!jResponse.contains(promptID))
            continue;
        
        imageGenerated = true;
        std::string filename = jResponse[promptID]["outputs"]["9"]["images"][0]["filename"];
        LOG("comfy generated: {}", filename);
        break;
    }
    
    return std::string();
}
