#include "common.hpp"
#include "ai_client.hpp"

class musicgen_client : public ai_client
{
private:

public:
    musicgen_client();
    ~musicgen_client();

public:
    std::string processSingleTxtToAudio(const std::string& prompt, const int& durationInSeconds);
};

