#include "common.hpp"
#include "ollama_client.hpp"
#include "server.hpp"
#include "config.hpp"
#include "commands.hpp"

int main()
{
    LOG("Reading config");
    g_config->fromFile("config.json");
    
    g_cmd->registerCommand("GETWEATHER", "A tool that retrieves current weather data for a specified city (By calling real-time weather data API)", 
        helper_functions::getWeather);    

    auto comfyBaseDir = g_config->getValue<std::string>("comfy.fs_location");
    if (!std::filesystem::exists(comfyBaseDir) || !std::filesystem::is_directory(comfyBaseDir)) 
    {
        LOG("Base directory for comfy does not exist: {}", comfyBaseDir);
        LOG("Image generation will be disabled");
    }
    else
    {
        g_cmd->registerCommand("GENIMG", "A tool that generates an image based on an image description", 
            helper_functions::generateImage, false);   
    }

    g_cmd->registerCommand("GENAUDIO", "A tool that generates music (as an audio file) based on a description", 
        helper_functions::generateMusic, false);    


    LOG("Starting backend");
    auto webserver = std::make_unique<server>(g_config->getValue<std::string>("server.bind_address"), g_config->getValue<int>("server.bind_port"));
    webserver->listen();   
    return 0;
}