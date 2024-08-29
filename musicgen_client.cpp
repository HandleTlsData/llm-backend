#include "musicgen_client.hpp"

#include "py_handler.hpp"

musicgen_client::musicgen_client()
{
}

musicgen_client::~musicgen_client()
{
}

std::string musicgen_client::processSingleTxtToAudio(const std::string &prompt, const int &durationInSeconds)
{
    auto res = g_pyHandler->callPythonFunction<std::string>("musicGen", "generate", "(i,s)", durationInSeconds, prompt);
    std::filesystem::path cwd = std::filesystem::current_path() / res;
    return cwd.string();
}