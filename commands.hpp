#include "common.hpp"
#include <map>
#include <functional>
#include <regex>
#include <utility>

class command_handler 
{
private:
    //string - command name, pair<string - command description, function - command function to execute>
    std::map<std::string, std::pair<std::string, std::function<std::string(const std::string&)>>> m_commands;

public:
    void registerCommand(const std::string& name, const std::string& description, std::function<std::string(const std::string&)> func);
    std::string executeCommand(const std::string& input);

    std::vector<std::pair<std::string, std::string>> listCommands();
};

inline const auto g_cmd = std::make_unique< command_handler >();

namespace helper_functions
{
    std::string getCoordinates(const std::string& city);
    std::string getWeather(const std::string& city);

    std::string generateImage(const std::string& prompt);
};