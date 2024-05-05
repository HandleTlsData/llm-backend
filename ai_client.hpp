#include "common.hpp"

class ai_client
{
protected:
    bool supportStreaming = false;
    //1 client = 1 chat interaction
    //for multithreading just create more clients
    bool isProcessingResponse = false;
    std::string model = {};
    std::string lastRequestMessage = {};
    std::string lastResponseMessage = {};
    std::chrono::steady_clock::time_point lastAccessed = {};
public:
    ai_client(/* args */);
    ~ai_client();
    void accessed();
};
