#include "common.hpp"
#include "ollama_client.hpp"
#include "server.hpp"
#include "config.hpp"
#include "commands.hpp"

int main()
{
    LOG("Reading config");
    g_config->fromFile("config.json");

    g_cmd->registerCommand("GETWEATHER", "A tool that retrieves current weather data for a specified city (By calling real-time weather data API)", helper_functions::getWeather);    

    auto comfyBaseDir = g_config->getValue<std::string>("comfy.fs_location");
    if (!std::filesystem::exists(comfyBaseDir) || !std::filesystem::is_directory(comfyBaseDir)) 
    {
        LOG("Base directory for comfy does not exist: {}", comfyBaseDir);
        LOG("Image generation will be disabled");
    }
    
    {
        g_cmd->registerCommand("GENIMG", "A tool that generates picture by given prompt (By calling imagine generation API)", helper_functions::generateImage);   
    }


    LOG("Starting backend");
    auto webserver = std::make_unique<server>(g_config->getValue<std::string>("server.bind_address"), g_config->getValue<int>("server.bind_port"));
    webserver->listen();   
    return 0;
}