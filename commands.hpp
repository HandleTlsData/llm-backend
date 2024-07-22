#include "common.hpp"
#include <map>
#include <functional>
#include <regex>
#include <utility>

struct command_body
{
    std::string description;
    //with tools like getweather we can pass response back to the normal conversation for it to generate us proper reply
    //with tools like imageGeneration we cannot pass it back to LLM, since the output is binary data
    bool llmPostProcessing;
    std::function<std::string(const std::string&)> fn;
};

class command_handler 
{
private:
    std::map<std::string, command_body> m_commands;

public:
    void registerCommand(const std::string& name, const std::string& description, std::function<std::string(const std::string&)> func, bool postProcess = true);
    std::pair<std::string,std::string> executeCommand(const std::string& input, bool& llmPostProcessing);

    std::vector<std::pair<std::string, std::string>> listCommands();
};

inline const auto g_cmd = std::make_unique< command_handler >();

namespace helper_functions
{
    std::string getCoordinates(const std::string& city);
    std::string getWeather(const std::string& city);

    std::string generateImage(const std::string& prompt);
};