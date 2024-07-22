#include "common.hpp"
#include "ai_client.hpp"

class comfy_client : public ai_client
{
private:
    std::string m_backURL = {};
    std::string m_backDir = {};
    std::string getHistory(const std::string& promptID);
public:
    //backendUrl: "http://address:port", modelName: "model:tag"
    comfy_client(const std::string& backendUrl, const std::string& baseDir);
    ~comfy_client();

public:

public:
    std::string processSingleTxt2Img(const std::string& positivePrompt, const std::string& negativePrompt = "bad quality, disfigured, 3d, cgi");
};

