#include "common.hpp"
#include "ollama_client.hpp"
#include "server.hpp"
#include "config.hpp"

int main()
{
    LOG("Reading config");
    g_config->fromFile("config.json");    

    LOG("Starting backend");
    auto webserver = std::make_unique<server>(g_config->getValue<std::string>("server.bind_address"), g_config->getValue<int>("server.bind_port"));
    webserver->listen();   
    return 0;
}