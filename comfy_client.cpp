#include "comfy_client.hpp"
#include "comfy_worklows.hpp"

#define SIMPLE_TXT2IMG_URI "/prompt"
#define SIMPLE_TXT2IMG_METHOD "POST"

#define HISTORY_URI "/history/"
#define HISTORY_METHOD "GET"

std::string comfy_client::getHistory(const std::string &promptID)
{
    std::string targetURL = m_backURL + HISTORY_URI + promptID;
    std::string responseEncoded = makeHttpRequest(targetURL, "", HISTORY_METHOD);
    return responseEncoded;
}

comfy_client::comfy_client(const std::string &backendUrl, const std::string& baseDir)
{
    this->accessed();
    this->model = "sd3_medium.safetensors";
    m_backURL = backendUrl;
    m_backDir = baseDir;
}

comfy_client::~comfy_client()
{
}

std::string comfy_client::processSingleTxt2Img(const std::string &positivePrompt, const std::string &negativePrompt)
{
    auto workflow = comfy_workflows::SD3Workflow;
    auto clientID = generateString(16);

    workflow["client_id"] = clientID;
    workflow["prompt"]["4"]["inputs"]["ckpt_name"] = this->model;
    workflow["prompt"]["6"]["inputs"]["text"] = positivePrompt;
    workflow["prompt"]["7"]["inputs"]["text"] = negativePrompt;

    std::string targetURL = m_backURL + SIMPLE_TXT2IMG_URI;
    LOG("calling {}", targetURL);
    LOG("workflow: {}", workflow.dump());
    std::string responseEncoded = makeHttpRequest(targetURL, workflow, SIMPLE_TXT2IMG_METHOD);
    LOG("responseEncoded {}", responseEncoded);
    json responseBody = json::parse(responseEncoded);
    std::string promptID = responseBody["prompt_id"];

    bool imageGenerated = false;
    std::string generatedFilename = {};
    while(!imageGenerated)
    {
        auto strResponse = getHistory(promptID);
        auto jResponse = json::parse(strResponse);
        if(!jResponse.contains(promptID))
            continue;
        
        imageGenerated = true;
        generatedFilename = jResponse[promptID]["outputs"]["9"]["images"][0]["filename"];
        LOG("comfy generated: {}", generatedFilename);
        break;
    }

    std::string sourcePath = m_backDir + "output/" + generatedFilename;
    return sourcePath;
}
